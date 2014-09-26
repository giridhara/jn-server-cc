/*
 * JournalNode.h
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#ifndef JOURNALNODE_H_
#define JOURNALNODE_H_

#include "JournalNodeRpcServer.h"
#include "JournalNodeHttpServer.h"
#include "../util/JournalNodeConfigKeys.h"
#include "../util/JNServiceMiscUtils.h"
#include "Journal.h"
#include <map>
#include <boost/scoped_ptr.hpp>
#include <Ice/Properties.h>
#include <Ice/Ice.h>
#include <string>

using std::map;
using std::string;

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
        string httpsAddrString = conf->getProperty(DFS_JOURNALNODE_HTTP_ADDRESS_KEY);
        if(httpsAddrString.empty()){
            httpServerURI = DFS_JOURNALNODE_HTTP_ADDRESS_DEFAULT;
            cout << "httpServerURI being used is " << httpServerURI;
        }else{
            httpServerURI = httpsAddrString;
        }
        cout << "httpServerURI being used is " << httpServerURI;

        HostPortPair hpp(httpServerURI);
        httpPort = hpp.port;
        cout << "httpPort being used is " << httpPort;
    }
    virtual ~JournalNode() {}
    string getHttpServerURI() {
       return httpServerURI;
    }
    unsigned int getPort(){
        return httpPort;
    }
    void start();

    int getOrCreateJournal(const string& jid, Journal*& journal);

    //TODO : Added this function only for testing purpose
    JournalNodeRpcServer* getJNRPCServer() {
        return rpcServer;
    }

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
