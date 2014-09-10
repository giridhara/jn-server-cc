/*
 * Journal.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "Journal.h"
#include "../util/JNServiceMiscUtils.h"

namespace JournalServiceServer
{

Journal::~Journal()
{
}

/**
   * Reload any data that may have been cached. This is necessary
   * when we first load the Journal, but also after any formatting
   * operation, since the cached data is no longer relevant.
   */
void
Journal::refreshCachedData() {
    //TODO : this function is not complete
    //IOUtils.closeStream(committedTxnId);
    const string currentDir(storage.getCurrentDir());
    lastPromisedEpoch.reset(new PersistentLongFile(currentDir+ "/" +  LAST_PROMISED_FILENAME, 0));
    lastWriterEpoch.reset(new PersistentLongFile(currentDir + "/" + LAST_WRITER_EPOCH, 0));
    committedTxnId.reset(new BestEffortLongFile(currentDir + "/" + COMMITTED_TXID_FILENAME, INVALID_TXID));
  }

/**
* Scan the local storage directory, and return the segment containing
* the highest transaction.
* @return the EditLogFile with the highest transactions, or null
* if no files exist.
*/
//call isInitialized function on 'ret' before using it
int
Journal::scanStorageForLatestEdits(EditLogFile& ret) {
    if (!file_exists(storage.getCurrentDir())) {
      return 0;
    }

    LOG.info("Scanning storage ");
    vector<EditLogFile> files;
    fjm.getLogFiles(0, files);

    for (unsigned i = files.size(); i-- > 0; ){
        EditLogFile& latestLog = files[i];
        int rc = latestLog.scanLog();
        if(rc != 0)
            return -1;
        if(latestLog.getLastTxId() == INVALID_TXID) {
            LOG.warn("Latest log %s has no transactions. moving it aside and looking for previous log",
                    latestLog.getFile().c_str());
            latestLog.moveAsideEmptyFile();
        }else {
            ret = latestLog;
            return 0;
        }
    }

    LOG.info("No files in %s", storage.getCurrentDir().c_str());
    return 0;
}

int
Journal::format(NamespaceInfo& nsInfo) {
    if(nsInfo.getNamespaceID() == 0) {
        LOG.error("can't format with uninitialized namespace info");
        return -1;
    }
    LOG.info("Formatting journal with namespace info");
    int rc = storage.format(nsInfo);
    if(rc != 0)
        return -1;
    refreshCachedData();
    return 0;
}

int
Journal::newEpoch(NamespaceInfo& nsInfo, long epoch, hadoop::hdfs::NewEpochResponseProto& ret) {
    if(checkFormatted() != 0) {
        return -1;
    }

    if(storage.checkConsistentNamespace(nsInfo) != 0 ){
        return -1;
    }

    long lpe;

    if(getLastPromisedEpoch(lpe) != 0) {
        return -1;
    }

    // Check that the new epoch being proposed is in fact newer than
    // any other that we've promised.
    if (epoch <= lpe) {
        LOG.error("Proposed epoch %d <= last promise %d", epoch, lpe);
        return -1;
    }

    if(updateLastPromisedEpoch(lpe, epoch) != 0 ) {
        return -1;
    }

    if (abortCurSegment() != 0 ) {
        return -1;
    }

    EditLogFile latestFile;
    if(scanStorageForLatestEdits(latestFile) != 0 ) {
        return -1;
    }

    if(latestFile.isInitialized()) {
        ret.set_lastsegmenttxid(latestFile.getFirstTxId());
    }

    return 0;
}

int
Journal::checkRequest(RequestInfo& reqInfo) {
    // Invariant 25 from ZAB paper
    long lpe;
    if(lastPromisedEpoch->get(lpe) != 0)
        return -1;
    if (reqInfo.getEpoch() < lpe) {
      LOG.error("IPC's epoch %d is less than the last promised epoch %d ",
              reqInfo.getEpoch(), lpe);
      return -1;
    } else if (reqInfo.getEpoch() > lpe) {
      // A newer client has arrived. Fence any previous writers by updating
      // the promise.
      if(updateLastPromisedEpoch(lpe, reqInfo.getEpoch()) != 0){
          return -1;
      }
    }

    // Ensure that the IPCs are arriving in-order as expected.
    if(reqInfo.getIpcSerialNumber() <= currentEpochIpcSerial) {
        LOG.error("IPC serial %s from client was not higher than prior highest IPC serial %s",
                reqInfo.getIpcSerialNumber(), currentEpochIpcSerial);
        return -1;
    }
    currentEpochIpcSerial = reqInfo.getIpcSerialNumber();

    if (reqInfo.hasCommittedTxId()) {
        long cti;
        if(committedTxnId->get(cti) != 0) {
            return -1;
        }
      if (reqInfo.getCommittedTxId() < cti) {
          LOG.error("Client trying to move committed txid backward from %d to ",
                  cti, reqInfo.getCommittedTxId());
          return -1;
      }

      return committedTxnId->set(reqInfo.getCommittedTxId());
    }

    return 0;
}

} /* namespace JournalServiceServer */
