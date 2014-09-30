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
#include "../util/Constants.h"
#include "../util/PersistentLongFile.h"
#include "../util/BestEffortLongFile.h"
#include "../util/NamespaceInfo.h"
#include "../util/RequestInfo.h"
#include <boost/scoped_ptr.hpp>
#include "../util/Logger.h"
#include <QJournalProtocolPB.h>
#include <Ice/Ice.h>
#include <ice-rpc-cc/src/Server.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

//#include <fstream>

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
    Journal(Ice::PropertiesPtr conf, string logDir, string jid)
        :
          journalId(jid),
          storage(logDir),
          curSegmentTxId(INVALID_TXID),
          nextTxId(INVALID_TXID),
          highestWrittenTxId(0),
          currentEpochIpcSerial(-1),
          lastPromisedEpoch(),
          lastWriterEpoch(),
          committedTxnId(),
          fjm(storage),
          conf(conf)
    {
        curSegment.reset(0);
        lastPromisedEpoch.reset(0);
        lastWriterEpoch.reset(0);
        committedTxnId.reset(0);
 //     TODO :: have to read about how to handle failures in constructors in c++
        refreshCachedData();

        EditLogFile latest;
        LOG.info("calling scanStorageLatestEdits from inside constructor of Journal");
        if(scanStorageForLatestEdits(latest) != 0) {
            if (latest.isInitialized()) {
                highestWrittenTxId = latest.getLastTxId();
            }
        }
    }
    virtual ~Journal();

    int getLastPromisedEpoch(long& ret){
        boost::recursive_mutex::scoped_lock lock(mMutex);
        checkFormatted();
        // get function below modifies argument only on successful execution
        return lastPromisedEpoch->get(ret);
      }

    int getLastWriterEpoch(long& ret) {
        boost::recursive_mutex::scoped_lock lock(mMutex);
        checkFormatted();
        return lastWriterEpoch->get(ret);
    }

    long getHighestWrittenTxId() {
        boost::recursive_mutex::scoped_lock lock(mMutex);
        return highestWrittenTxId;
    }

    int newEpoch(NamespaceInfo& nsInfo, long epoch, hadoop::hdfs::NewEpochResponseProto& ret);
    int format(const NamespaceInfo& nsInfo);
    int startLogSegment(const RequestInfo& reqInfo, const long txid, const int layoutVersion);
    int finalizeLogSegment(const RequestInfo& reqInfo, const long startTxId, const long endTxId);
    int journal(const RequestInfo& reqInfo,
          long segmentTxId, long firstTxnId,
          int numTxns, const string& records);
    int prepareRecovery(const RequestInfo& reqInfo, const long segmentTxId, hadoop::hdfs::PrepareRecoveryResponseProto& ret);
    int acceptRecovery(const RequestInfo& reqInfo, const hadoop::hdfs::SegmentStateProto& segment, const string& fromUrl);
    int getEditLogManifest(const long sinceTxId, const bool inProgressOk, vector<EditLogFile>& ret);
    bool isFormatted() {
        boost::recursive_mutex::scoped_lock lock(mMutex);
        return storage.isFormatted();
    }
    JNStorage& getStorage() {
        return storage;
    }

    FileJournalManager& getFileJournalManager(){
        return fjm;
    }
    int close() {
//TODO:        storage.close();
        if(committedTxnId)
            committedTxnId->close();
        if(curSegment)
            curSegment->close();
        return 0;
    }

    // TODO : Making getSegmentInfo public just for testing purpose
    int getSegmentInfo(long segmentTxId, hadoop::hdfs::SegmentStateProto& ssp, bool& isInitialized);

private:
    void refreshCachedData();

    int checkFormatted() {
       if (!isFormatted()) {
           LOG.error("Journal %s  is not formatted", storage.getLogDir().c_str());
           return -1;
       }
       return 0;
     }

    int scanStorageForLatestEdits(EditLogFile& ret);

    int checkRequest(const RequestInfo& reqInfo);
    int checkWriteRequest(const RequestInfo& reqInfo);

//    int getSegmentInfo(long segmentTxId, hadoop::hdfs::SegmentStateProto& ssp, bool& isInitialized);

    int purgePaxosDecision(long segmentTxId);
    int getPersistedPaxosData(long segmentTxId, hadoop::hdfs::PersistedRecoveryPaxosData& ret, bool& isInitialized);
    int completeHalfDoneAcceptRecovery(hadoop::hdfs::PersistedRecoveryPaxosData& paxosData, bool isInitialized);
    int persistPaxosData(long segmentTxId, hadoop::hdfs::PersistedRecoveryPaxosData& newData);
    int syncLog(const RequestInfo& reqInfo, const hadoop::hdfs::SegmentStateProto& segment, const string& url, string& ret);

    int updateLastPromisedEpoch (long oldEpoch, long newEpoch) {
       LOG.info("Updating lastPromisedEpoch from %d to %d", oldEpoch, newEpoch);
       if(lastPromisedEpoch->set(newEpoch) != 0 ){
           return -1;
       }

       // Since we have a new writer, reset the IPC serial - it will start
       // counting again from 0 for this writer.
       currentEpochIpcSerial = -1;
       return 0;
     }

    int abortCurSegment() {
        if (!curSegment) {
          return 0;
        }

        if(curSegment->abort() != 0)
            return -1;
        curSegment.reset(0);
        curSegmentTxId = INVALID_TXID;

        return 0;
    }

    int checkIfLogsInSequence(const vector<EditLogFile>& vec);


    boost::scoped_ptr<JNClientOutputStream> curSegment;
    long curSegmentTxId;
    long nextTxId;
    long highestWrittenTxId;

    const string journalId;

    JNStorage storage;

    /**
     * When a new writer comes along, it asks each node to promise
     * to ignore requests from any previous writer, as identified
     * by epoch number. In order to make such a promise, the epoch
     * number of that writer is stored persistently on disk.
     */
    boost::scoped_ptr<PersistentLongFile> lastPromisedEpoch;

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
    boost::scoped_ptr<PersistentLongFile> lastWriterEpoch;

    /**
     * Lower-bound on the last committed transaction ID. This is not
     * depended upon for correctness, but acts as a sanity check
     * during the recovery procedures, and as a visibility mark
     * for clients reading in-progress logs.
     */
    boost::scoped_ptr<BestEffortLongFile> committedTxnId;

    FileJournalManager fjm;
    Ice::PropertiesPtr conf;

    boost::recursive_mutex mMutex;

};

} /* namespace JournalServiceServer */

#endif /* JOURNAL_H_ */
