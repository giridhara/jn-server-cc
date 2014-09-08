/*
 * JournalNode.h
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#ifndef JOURNALNODE_H_
#define JOURNALNODE_H_

#include "JournalNodeRpcServer.h"
#include "util/JournalNodeConfigKeys.h"
#include "Journal.h"
#include "../common/Properties.h"
#include <map>

using std::map;
using namespace KFS;

namespace JournalServiceServer
{

class JournalNode
{
public:
    JournalNode()
    :
        conf(0),
        rpcServer(0),
        journalsById()
    {}
    virtual ~JournalNode();
private:
    int getLogDir(string& jid, string& logDir);
    int getOrCreateJournal(const string& jid, Journal* journal);

    Properties conf;
    JournalNodeRpcServer rpcServer;
//    JournalNodeHttpServer httpServer;
    map<string, Journal*> journalsById;
    string httpServerURI;
    string localDir;
};

}


#endif /* JOURNALNODE_H_ */
