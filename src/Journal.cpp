/*
 * Journal.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "Journal.h"

namespace JournalServiceServer
{

Journal::~Journal()
{
    // TODO Auto-generated destructor stub
}

/**
   * Reload any data that may have been cached. This is necessary
   * when we first load the Journal, but also after any formatting
   * operation, since the cached data is no longer relevant.
   */
void
Journal::refreshCachedData() {
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

} /* namespace JournalServiceServer */
