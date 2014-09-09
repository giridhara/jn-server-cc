/*
 * Journal.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "Journal.h"
#include "../util/Logger.h"
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
    PersistentLongFile lpe((currentDir+ "/" +  LAST_PROMISED_FILENAME), 0);
    lastPromisedEpoch = lpe;
//    lastPromisedEpoch((currentDir+ "/" +  LAST_PROMISED_FILENAME), 0);
    PersistentLongFile lwe((currentDir, LAST_WRITER_EPOCH), 0);
    lastWriterEpoch = lwe;
//    lastWriterEpoch((currentDir, LAST_WRITER_EPOCH), 0);
    committedTxnId = INVALID_TXID;
  }

int
Journal::scanStorageForLatestEdits(EditLogFile& ret) {
    if (!file_exists(storage.getCurrentDir())) {
      return -1;
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

} /* namespace JournalServiceServer */
