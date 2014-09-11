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
    //TODO : storage is currently stack variable, hence should not call delete on it
    // it will get automatically cleared up
//    delete storage;
    committedTxnId.reset();
    curSegment.reset();
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

/**
   * Ensure that the given request is coming from the correct writer and in-order.
   * @param reqInfo the request info
   * @throws IOException if the request is invalid.
*/
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

int
Journal::checkWriteRequest(RequestInfo& reqInfo) {
    if(checkRequest(reqInfo)!=0) {
        return -1;
    }

    long lwe;
    if(lastWriterEpoch->get(lwe) !=0 ){
        return -1;
    }

    if (reqInfo.getEpoch() != lwe) {
      LOG.error("IPC's epoch %d is not the current writer epoch %d",
              reqInfo.getEpoch(), lwe);
      return -1;
    }
    return 0;
}

/**
   * Finalize the log segment at the given transaction ID.
   */
int
Journal::finalizeLogSegment(RequestInfo& reqInfo, long startTxId,
      long endTxId) {
    if(checkFormatted() != 0 ) {
        return -1;
    }

    if(checkRequest(reqInfo) != 0){
        return -1;
    }

    bool needsValidation = true;

    // Finalizing the log that the writer was just writing.
    if (startTxId == curSegmentTxId) {
      if (curSegment) {
        curSegment->close();
        curSegment.reset();
        curSegmentTxId = INVALID_TXID;
      }

    if (nextTxId != endTxId + 1) {
        LOG.error("Trying to finalize in-progress log segment %s "
                  "to end at txid %s but only written up to txid %s",
          startTxId, endTxId, nextTxId - 1);
        return -1;
    }
      // No need to validate the edit log if the client is finalizing
      // the log segment that it was just writing to.
      needsValidation = false;
    }

    EditLogFile elf;
    if (fjm.getLogFile(startTxId, elf) != 0) {
        return -1;
    }
    if (!elf.isInitialized()) {
      LOG.error("No log file to finalize at transaction ID %d", startTxId);
      return -1;
    }

    if (elf.isInProgress()) {
        if (needsValidation) {
            LOG.info("Validating log segment %s about to be finalized",  elf.getFile().c_str());
            if(elf.scanLog() != 0) {
                return -1;
            }
            if(elf.getLastTxId() != endTxId) {
                LOG.error("Trying to finalize in-progress log segment %s to end at txid %s but log %s on disk only contains up to txid %s",
                        startTxId, endTxId, elf.getFile().c_str(), elf.getLastTxId());
                return -1;
            }
        }
        if (fjm.finalizeLogSegment(startTxId, endTxId) != 0 ) {
          return -1;
        }
    }else {
        if(endTxId != elf.getLastTxId()) {
            LOG.error("Trying to re-finalize already finalized log [%d, %d] with different endTxId %d",
                    elf.getFirstTxId(), elf.getLastTxId(), endTxId);
        }
    }

    // Once logs are finalized, a different length will never be decided.
    // During recovery, we treat a finalized segment the same as an accepted
    // recovery. Thus, we no longer need to keep track of the previously-
    // accepted decision. The existence of the finalized log segment is enough.
    if (purgePaxosDecision(elf.getFirstTxId()) != 0 ){
        return -1;
    }
    return 0;
}

int
Journal::getSegmentInfo(long segmentTxId, hadoop::hdfs::SegmentStateProto& ssp, bool& isInitialized) {
    EditLogFile elf;
    if( fjm.getLogFile(segmentTxId, elf) != 0 ) {
        return -1;
    }
    if (!elf.isInitialized()) {
      isInitialized = false;
      return 0;
    }
    if (elf.isInProgress()) {
      if(elf.scanLog() != 0) {
          return -1;
      }
    }
    if (elf.getLastTxId() == INVALID_TXID) {
      LOG.info("Edit log file %s appears to be empty. Moving it aside...", elf.getFile().c_str());
      elf.moveAsideEmptyFile();
      isInitialized = false;
      return 0;
    }
    ssp.set_starttxid(segmentTxId);
    ssp.set_endtxid(elf.getLastTxId());
    ssp.set_isinprogress(elf.isInProgress());

    //TODO : Looks like i need to provide implementation for '<<' operator for EditLogFile,
    //so that logging as done below is possible
//    LOG.info("getSegmentInfo(" + segmentTxId + "): " + elf + " -> " +
//        TextFormat.shortDebugString(ret));
    return 0;
}

/**
   * Start a new segment at the given txid. The previous segment
   * must have already been finalized.
   */
int
Journal::startLogSegment(RequestInfo& reqInfo, long txid,
      int layoutVersion) {
    //TODO : Have to implement below assert.
    //assert fjm != null;

    if (checkFormatted() != 0) {
        return -1;
    }

    if (checkRequest(reqInfo) != 0) {
        return -1;
    }

    if (curSegment) {
        ostringstream warnMsg;
        // TODO : Below msg is not entirely proper.
        warnMsg << "Client is requesting a new log segment " << txid
                << "though we are already writing " << ". "
                << "Aborting the current segment in order to begin the new one.";
        LOG.warn(warnMsg.str().c_str());
        // The writer may have lost a connection to us and is now
        // re-connecting after the connection came back.
        // We should abort our own old segment.
        if(abortCurSegment() != 0 ){
            return -1;
        }
    }

    // Paranoid sanity check: we should never overwrite a finalized log file.
    // Additionally, if it's in-progress, it should have at most 1 transaction.
    // This can happen if the writer crashes exactly at the start of a segment.
    EditLogFile existing;
    if (fjm.getLogFile(txid, existing) != 0 ) {
        return -1;
    }
    if (existing.isInitialized()) {
        if (!existing.isInProgress()) {
            LOG.warn("Already have a finalized segment %s beginning at ",
              existing.getFile().c_str(), txid);
        }

        // If it's in-progress, it should only contain one transaction,
        // because the "startLogSegment" transaction is written alone at the
        // start of each segment.
        if (existing.scanLog() != 0 ){
            return -1;
        }
        if (existing.getLastTxId() != existing.getFirstTxId()) {
            LOG.error("The log file %s seems to contain valid transactions" , existing.getFile().c_str());
            return -1;
        }
    }

    long curLastWriterEpoch;
    if (lastWriterEpoch->get(curLastWriterEpoch) != 0 ) {
        return -1;
    }
    if (curLastWriterEpoch != reqInfo.getEpoch()) {
      LOG.info("Updating lastWriterEpoch from %d to %d", reqInfo.getEpoch(), curLastWriterEpoch);
      if(lastWriterEpoch->set(reqInfo.getEpoch()) !=0 ) {
          return -1;
      }
    }

    // The fact that we are starting a segment at this txid indicates
    // that any previous recovery for this same segment was aborted.
    // Otherwise, no writer would have started writing. So, we can
    // remove the record of the older segment here.
    if (purgePaxosDecision(txid) != 0 ) {
        return -1;
    }

    if (fjm.startLogSegment(txid, layoutVersion, curSegment) != 0){
        return -1;
    }

    curSegmentTxId = txid;
    nextTxId = txid;
}

int
Journal::journal(RequestInfo& reqInfo,
      long segmentTxId, long firstTxnId,
      int numTxns, const char* records) {

    if(checkFormatted() != 0 ) {
        return -1;
    }

    if(checkWriteRequest(reqInfo) != 0) {
        return -1;
    }

    if(!curSegment){
        LOG.error("Can't write, no segment open");
    }

    if (curSegmentTxId != segmentTxId) {
      // Sanity check: it is possible that the writer will fail IPCs
      // on both the finalize() and then the start() of the next segment.
      // This could cause us to continue writing to an old segment
      // instead of rolling to a new one, which breaks one of the
      // invariants in the design. If it happens, abort the segment
      // and throw an exception.
      LOG.error("Writer out of sync: it thinks it is writing segment %d but current segment is %d",
              segmentTxId, curSegmentTxId);
      if(abortCurSegment() != 0) {
          return -1;
      }
      return -1;
    }

    if(nextTxId != firstTxnId) {
        LOG.error("Can't write txid %d expecting nextTxId=", firstTxnId, nextTxId);
        return -1;
    }

    long lastTxnId = firstTxnId + numTxns - 1;
    LOG.debug("Writing txid %d-%d", firstTxnId, lastTxnId);

    // If the edit has already been marked as committed, we know
    // it has been fsynced on a quorum of other nodes, and we are
    // "catching up" with the rest. Hence we do not need to fsync.
    long ctid;
    if(committedTxnId->get(ctid) != 0) {
        return -1;
    }
    bool isLagging = lastTxnId <= ctid;
    bool shouldFsync = !isLagging;

    curSegment->writeRaw(records, 0, static_cast<int>(strlen(records)));
    curSegment->setReadyToFlush();
    curSegment->flush(shouldFsync);

    highestWrittenTxId = lastTxnId;
    nextTxId = lastTxnId + 1;

    return 0;
  }

} /* namespace JournalServiceServer */
