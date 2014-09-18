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
#include <boost/scoped_ptr.hpp>
#include <Ice/Properties.h>
#include <Ice/Ice.h>

using std::map;

namespace JournalServiceServer
{

class JournalNodeRpcServer;

class JournalNode
{
public:
    JournalNode(Ice::PropertiesPtr conf)
    :
        journalsById(),
        conf(conf),
        rpcServer(0)
    {
        string editsDir = conf->getProperty(DFS_JOURNALNODE_EDITS_DIR_KEY);
        if(editsDir.empty()){
            localDir = DFS_JOURNALNODE_EDITS_DIR_DEFAULT;
        }else {
            localDir =  editsDir;
        }
        string httpsAddrString = conf->getProperty(DFS_JOURNALNODE_HTTPS_ADDRESS_KEY);
        if(httpsAddrString.empty()){
            httpServerURI = DFS_JOURNALNODE_HTTPS_ADDRESS_DEFAULT;
        }else{
            httpServerURI = httpsAddrString;
        }

        HostPortPair hpp(httpServerURI);
        httpPort = hpp.port;
    }
    virtual ~JournalNode();
    string getHttpServerURI() {
       return httpServerURI;
    }
    unsigned int getPort(){
        return httpPort;
    }
    void start();

    int getOrCreateJournal(const string& jid, Journal* journal);
private:
    int getLogDir(const string& jid, string& logDir);

    Ice::PropertiesPtr conf;
    JournalNodeRpcServer* rpcServer;
    map<const string, Journal*> journalsById;
    string httpServerURI;
    string localDir;
    unsigned int httpPort;
};

extern scoped_ptr<JournalNode> global_jn;

}

#endif /* JOURNALNODE_H_ */


//int main(){
//
//}
