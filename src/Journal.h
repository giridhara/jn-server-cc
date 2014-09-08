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

using std::string;

namespace JournalServiceServer
{

class Journal
{
public:
    Journal();
    virtual ~Journal();
    const string LAST_PROMISED_FILENAME = "last-promised-epoch";
    const string LAST_WRITER_EPOCH = "last-writer-epoch";

private:
    const string COMMITTED_TXID_FILENAME = "committed-txid";
    JNClientOutputStream curSegment;
    long curSegmentTxId = INVALID_TXID;
    long nextTxId = INVALID_TXID;
    long highestWrittenTxId = 0;

    const string journalId;

    const JNStorage storage;

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
    long currentEpochIpcSerial = -1;

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
};

} /* namespace JournalServiceServer */

#endif /* JOURNAL_H_ */
