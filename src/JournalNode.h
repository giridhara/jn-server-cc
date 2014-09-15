/*
 * JournalNode.h
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#ifndef JOURNALNODE_H_
#define JOURNALNODE_H_

#include "JournalNodeRpcServer.h"
#include "../util/JournalNodeConfigKeys.h"
#include "Journal.h"
#include <map>

using std::map;

namespace JournalServiceServer
{

class JournalNodeRpcServer;

class JournalNode
{
public:
    JournalNode()
    :
 //       journalsById(),
 //       conf(0),
        rpcServer(0)
    {}
    virtual ~JournalNode();
    string getHttpServerURI() {
       return httpServerURI;
    }
    unsigned int getPort(){
        return port;
    }
    int getOrCreateJournal(const string& jid, Journal* journal);
private:
    int getLogDir(const string& jid, string& logDir);

//    map<string, string> conf;
    JournalNodeRpcServer* rpcServer;
//    JournalNodeHttpServer httpServer;
//    map<const string, Journal*> journalsById;
    string httpServerURI;
    string localDir;
    unsigned int port;
};

}


#endif /* JOURNALNODE_H_ */
