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
#include <ice-rpc-cc/src/Server.h>
#include <ice-qjournal-protocol/QJournalProtocolServerSideTranslatorPB.h>
#include <util/JournalNodeConfigKeys.h>
#include <boost/scoped_ptr.hpp>

//forward declaration


namespace JournalServiceServer
{

class JournalNode;

class JournalNodeRpcServer : public QJournalProtocol
{
public:
    JournalNodeRpcServer(Ice::PropertiesPtr conf, JournalNode& jn )
        :
        jn(jn),
        conf(conf)
    {
        QJournalProtocolProtos::QJournalProtocolPBPtr instance =
                new JournalServiceServer::QJournalProtocolServerSideTranslatorPB(this);

        string rpcaddress;
        scoped_ptr<HostPortPair> hpp;
        hpp.reset(new HostPortPair(DFS_JOURNALNODE_RPC_ADDRESS_DEFAULT));
        if(!(rpcaddress=conf->getProperty(DFS_JOURNALNODE_RPC_ADDRESS_KEY)).empty()){
            hpp.reset(new HostPortPair(rpcaddress));
        }

        //TODO :: Giving default values for now for hostname and port address of rpc server
        server = new icerpc::Server(instance, "QJournalProtocolPB", hpp->hostname, hpp->port, 1);
    }

    ~JournalNodeRpcServer() {}
    void start();
    int isFormatted(const string& journalId, bool& result);
    int getJournalState(const string& journalId, hadoop::hdfs::GetJournalStateResponseProto&);
    int newEpoch(const string& journalId, NamespaceInfo& nsInfo,
            const uint64_t epoch, hadoop::hdfs::NewEpochResponseProto&);
    int format(const string& journalId, const NamespaceInfo& nsInfo);
    int journal(const RequestInfo& reqInfo, const long segmentTxId,
            const long firstTxnId, const int numTxns, const string& records);
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
    JournalNode&  jn;
    Ice::PropertiesPtr conf;
    icerpc::Server* server;

};

} /* namespace JournalServiceServer */

#endif /* JOURNALNODERPCSERVER_H_ */
