/*
 * Server.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "Server.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <util/JNServiceMiscUtils.h>

//new lines
//#include "../../ice-qjournal-protocol/QJournalProtocolServerSideTranslatorPB.h"
#include "/home/psarda/workspace/jn-server-cc/ice-qjournal-protocol/QJournalProtocolServerSideTranslatorPB.h"
#include "/home/psarda/workspace/jn-server-cc/src/JournalNodeRpcServer.h"

namespace icerpc
{
const string Server::server_config = "/tmp/config.server";

Server::~Server()
{
}

int
Server::createConfigFile(){
    ofstream myfile;
    myfile.open(server_config.c_str());
    if (!myfile.is_open()){
        return 1;
    }
    myfile << protocol << ".Endpoints=tcp -h " << bind_address << " -p " << port <<"\n";
    myfile << "Ice.ThreadPool.Server.Size=" << num_handlers << "\n";
//    Ice.Trace.Network=3
//
//    #
//    # Protocol Tracing
//    #
//    # 0 = no protocol tracing
//    # 1 = trace protocol messages
//    #
//    Ice.Trace.Protocol=1
//    myfile << "Ice.Trace.Network=3" << "\n";
//    myfile << "Ice.Trace.Protocol=1" << "\n";

    myfile.close();
    return 0;
}

bool
Server::deleteConfigFile() {
    return JournalServiceServer::file_delete(server_config);
//    return remove(server_config.c_str()) == 0;
}

void
Server::start(){
    int status = createConfigFile();
    if (status == 0) {
      StringSeq vec;
      vec.push_back("Server");
      status = this->main(vec, server_config.c_str());
    }
    exit(status);
}

void
Server::stop() {
    if (running) {
        assert (adapter != 0);
        adapter->getCommunicator()->shutdown();
        running = false;
        cout << "Stopping journal node running at " << bind_address  << ":" << port << endl;
        deleteConfigFile();
    }
}

void
Server::join(){
    while (running) {
      communicator()->waitForShutdown();
    }
}

int
Server::run(int argc, char*[]){
    adapter = communicator()->createObjectAdapter(protocol);
    adapter->add(instance, communicator()->stringToIdentity(protocol));
    adapter->activate();

    running = true;
    cout << "Started ICE based journal node at " + bind_address + ":" << port << endl;

    shutdownOnInterrupt();

    communicator()->waitForShutdown();

    stop();

    return 0;
}

} /* namespace icerpc */

//int main(int argc, char *argv[]){
//    JournalServiceServer::QJournalProtocol* qjp= new JournalServiceServer::JournalNodeRpcServer();
//    QJournalProtocolProtos::QJournalProtocolPBPtr instance = new JournalServiceServer::QJournalProtocolServerSideTranslatorPB(qjp);
//    icerpc::Server server(instance, "QJournalProtocolPB", "localhost", 10000, 1);
//    server.start();
//    return 0;
//}
