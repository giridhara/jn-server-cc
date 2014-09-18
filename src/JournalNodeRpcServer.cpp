/*
 * JournalNodeRpcServer.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "JournalNodeRpcServer.h"

namespace JournalServiceServer
{

int
JournalNodeRpcServer::isFormatted(const string& journalId, bool& result){
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(journalId, journal);
    if(rc != 0) {
        return -1;
    }

    result = journal->isFormatted();
    return 0;
}

int
JournalNodeRpcServer::getJournalState(const string& journalId,
        hadoop::hdfs::GetJournalStateResponseProto& ret){
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(journalId, journal);
    if(rc != 0) {
        return -1;
    }

    long epoch;
    rc = journal->getLastPromisedEpoch(epoch);
    if(rc != 0) {
        return -1;
    }
    ret.set_lastpromisedepoch(epoch);
    ret.set_httpport(jn.getPort());
    ret.set_fromurl(jn.getHttpServerURI());
    return 0;
}

int
JournalNodeRpcServer::newEpoch(const string& journalId, NamespaceInfo& nsInfo,
             const uint64_t epoch, hadoop::hdfs::NewEpochResponseProto& ret){
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(journalId, journal);
    if(rc != 0) {
        return -1;
    }
    return journal->newEpoch(nsInfo, epoch, ret);
}

int
JournalNodeRpcServer::format(const string& journalId, const NamespaceInfo& nsInfo) {
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(journalId, journal);
    if(rc != 0) {
        return -1;
    }
    return journal->format(nsInfo);
}

int
JournalNodeRpcServer::journal(const RequestInfo& reqInfo, const long segmentTxId,
            const long firstTxnId, const int numTxns, const char* records){
    Journal* journalPtr = 0;
    int rc = jn.getOrCreateJournal(reqInfo.getJournalId(), journalPtr);
    if(rc != 0) {
        return -1;
    }
    return journalPtr->journal(reqInfo, segmentTxId, firstTxnId, numTxns, records);
}

int
JournalNodeRpcServer::startLogSegment(const RequestInfo& reqInfo,
      const long txid, const int layoutVersion) {
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(reqInfo.getJournalId(), journal);
    if(rc != 0) {
        return -1;
    }
    return journal->startLogSegment(reqInfo, txid, layoutVersion);
}

int
JournalNodeRpcServer::finalizeLogSegment(const RequestInfo& reqInfo,
      const long startTxId, const long endTxId) {
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(reqInfo.getJournalId(), journal);
    if(rc != 0) {
        return -1;
    }
    return journal->finalizeLogSegment(reqInfo, startTxId, endTxId );
}

int
JournalNodeRpcServer::getEditLogManifest(const string& jid, const long sinceTxId,
          const bool inProgressOk, hadoop::hdfs::GetEditLogManifestResponseProto& ret){
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(jid, journal);
    if(rc != 0) {
        return -1;
    }

    vector<EditLogFile> elfv;
    if(journal->getEditLogManifest(sinceTxId, inProgressOk, elfv) != 0) {
        return -1;
    }

    hadoop::hdfs::RemoteEditLogManifestProto* relmp = new hadoop::hdfs::RemoteEditLogManifestProto();

    for(std::vector<EditLogFile>::iterator it = elfv.begin(); it != elfv.end(); ++it) {
        hadoop::hdfs::RemoteEditLogProto* relp = new hadoop::hdfs::RemoteEditLogProto();
        relp->set_starttxid(it->getFirstTxId());
        relp->set_endtxid(it->getLastTxId());
        relp->set_isinprogress(it->isInProgress());
        relmp->mutable_logs()->AddAllocated(relp);
     }

    ret.set_allocated_manifest(relmp);
    ret.set_httpport(jn.getPort());
    ret.set_fromurl(jn.getHttpServerURI());
    return 0;
}

int
JournalNodeRpcServer::prepareRecovery(const RequestInfo& reqInfo,
      const long segmentTxId, hadoop::hdfs::PrepareRecoveryResponseProto& ret){
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(reqInfo.getJournalId(), journal);
    if(rc != 0) {
        return -1;
    }
    return journal->prepareRecovery(reqInfo, segmentTxId, ret);
}

int
JournalNodeRpcServer::acceptRecovery(const RequestInfo& reqInfo,
          const hadoop::hdfs::SegmentStateProto& stateToAccept, const string& fromUrl){
    Journal* journal = 0;
    int rc = jn.getOrCreateJournal(reqInfo.getJournalId(), journal);
    if(rc != 0) {
        return -1;
    }
    return journal->acceptRecovery(reqInfo, stateToAccept,fromUrl);
}

} /* namespace JournalServiceServer */

