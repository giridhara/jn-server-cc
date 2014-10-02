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
#include <ice-qjournal-protocol/QJournalProtocolServerSideTranslatorPB.h>
#include <src/JournalNodeRpcServer.h>
#include <util/Logger.h>

namespace icerpc
{
const string Server::server_config = "/tmp/config.server";

int
Server::createConfigFile(){
    ofstream myfile;
    myfile.open(server_config.c_str());
    if (!myfile.is_open()){
        return 1;
    }
    myfile << protocol << ".Endpoints=tcp -h " << bind_address << " -p " << port <<"\n";
    myfile << "Ice.ThreadPool.Server.Size=" << num_handlers << "\n";
    myfile.close();
    return 0;
}

bool
Server::deleteConfigFile() {
    return JournalServiceServer::file_delete(server_config);
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
        JournalServiceServer::LOG.info("Stopping journal node running at %s:%d", bind_address.c_str(), port);
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
    JournalServiceServer::LOG.info("Started ICE based journal node at %s:%d",bind_address.c_str(), port);

    shutdownOnInterrupt();

    communicator()->waitForShutdown();

    stop();

    return 0;
}

} /* namespace icerpc */
