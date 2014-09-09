/*
 * Journal.h
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#ifndef JOURNAL_H_
#define JOURNAL_H_

#include <string>
#include "FileJournalManager.h"
#include "JNStorage.h"
//#include "../common/Properties.h"
#include "../util/Constants.h"
#include "../util/PersistentLongFile.h"

using std::string;

//using namespace KFS;

namespace JournalServiceServer
{

const string LAST_PROMISED_FILENAME = "last-promised-epoch";
const string LAST_WRITER_EPOCH = "last-writer-epoch";
const string COMMITTED_TXID_FILENAME = "committed-txid";

class Journal
{
public:
    Journal();
    virtual ~Journal();


private:
    JNClientOutputStream curSegment;
    long curSegmentTxId;
    long nextTxId;
    long highestWrittenTxId;

    const string journalId;

    JNStorage storage;

    Journal(string conf, string logDir, string jid)
        : curSegment(0),
          journalId(jid),
          storage(conf, logDir),
          curSegmentTxId(INVALID_TXID),
          nextTxId(INVALID_TXID),
          highestWrittenTxId(0),
          currentEpochIpcSerial(-1),
          lastPromisedEpoch(),
          lastWriterEpoch(),
          committedTxnId(INVALID_TXID),
          fjm(storage)
    {
            refreshCachedData();

    //        EditLogFile latest = scanStorageForLatestEdits();
    //        if (latest != null) {
    //          highestWrittenTxId = latest.getLastTxId();
    //        }

    }

    /**
     * When a new writer comes along, it asks each node to promise
     * to ignore requests from any previous writer, as identified
     * by epoch number. In order to make such a promise, the epoch
     * number of that writer is stored persistently on disk.
     */
    PersistentLongFile lastPromisedEpoch;

    /**
     * Each IPC that comes from a given client contains a serial number
     * which only increases from the client's perspective. Whenever
     * we switch epochs, we reset this back to -1. Whenever an IPC
     * comes from a client, we ensure that it is strictly higher
     * than any previous IPC. This guards against any bugs in the IPC
     * layer that would re-order IPCs or cause a stale retry from an old
     * request to resurface and confuse things.
     */
    long currentEpochIpcSerial;

    /**
     * The epoch number of the last writer to actually write a transaction.
     * This is used to differentiate log segments after a crash at the very
     * beginning of a segment. See the the 'testNewerVersionOfSegmentWins'
     * test case.
     */
    PersistentLongFile lastWriterEpoch;

    /**
     * Lower-bound on the last committed transaction ID. This is not
     * depended upon for correctness, but acts as a sanity check
     * during the recovery procedures, and as a visibility mark
     * for clients reading in-progress logs.
     */
    long committedTxnId;

    FileJournalManager fjm;

    void refreshCachedData();


};

} /* namespace JournalServiceServer */

#endif /* JOURNAL_H_ */
