/*
 * JournalNodeRpcServer.h
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#ifndef JOURNALNODERPCSERVER_H_
#define JOURNALNODERPCSERVER_H_

#include "QjournalProtocol.h"
#include "Journal.h"
#include "JournalNode.h"
#include <Ice/Properties.h>
#include <Ice/Ice.h>

//forward declaration
class JournalNode;

namespace JournalServiceServer
{
//const ObjectPtr instance, const string& protocol, const string bind_address, int port, int num_handlers

class JournalNodeRpcServer : public QJournalProtocol
{
public:
    JournalNodeRpcServer(Ice::PropertiesPtr conf, JournalNode& jn )
        :
        jn(jn),
        conf(conf)
    {}
    ~JournalNodeRpcServer() {}
    int isFormatted(const string& journalId, bool& result);
    int getJournalState(const string& journalId, hadoop::hdfs::GetJournalStateResponseProto&);
    int newEpoch(const string& journalId, NamespaceInfo& nsInfo,
            const uint64_t epoch, hadoop::hdfs::NewEpochResponseProto&);
    int format(const string& journalId, const NamespaceInfo& nsInfo);
    int journal(const RequestInfo& reqInfo, const long segmentTxId,
            const long firstTxnId, const int numTxns, const char* records);
    int startLogSegment(const RequestInfo& reqInfo,
          const long txid, const int layoutVersion);
    int finalizeLogSegment(const RequestInfo& reqInfo,
          const long startTxId, const long endTxId);
    int getEditLogManifest(const string& jid, const long sinceTxId,
          const bool inProgressOk, hadoop::hdfs::GetEditLogManifestResponseProto&);
    int prepareRecovery(const RequestInfo& reqInfo,
          const long segmentTxId, hadoop::hdfs::PrepareRecoveryResponseProto&);
    int acceptRecovery(const RequestInfo& reqInfo,
          const hadoop::hdfs::SegmentStateProto& stateToAccept, const string& fromUrl);

private:
    JournalNode& jn;
    Ice::PropertiesPtr conf;
};

} /* namespace JournalServiceServer */

#endif /* JOURNALNODERPCSERVER_H_ */
