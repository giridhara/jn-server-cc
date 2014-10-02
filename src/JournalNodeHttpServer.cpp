/*
 * JournalNodeHttpServer.cpp
 *
 *  Created on: Sep 17, 2014
 *      Author: psarda
 */

#include "JournalNodeHttpServer.h"
#include "../util/Logger.h"
#include <sstream>
#include <signal.h>

namespace JournalServiceServer
{

string JournalNodeHttpServer::listening_port = "";
volatile int JournalNodeHttpServer::signal_received = 0;

JournalNodeHttpServer::JournalNodeHttpServer() {}

JournalNodeHttpServer::~JournalNodeHttpServer() {}

void JournalNodeHttpServer::signal_handler(int sig_num) {
  signal_received = sig_num;
}

static int checkStorageInfo(JNStorage& storage, char* storageInfo) {
    int myNsId = storage.getNamespaceID();
    string myClusterId = storage.getClusterID();
    char *tok = 0;
    tok = strtok(storageInfo, ":");
    vector<string> storageInfoTokens;
    while (tok) {
        storageInfoTokens.push_back(tok);
        tok = strtok(NULL, ":");
    }

    if(storageInfoTokens.size() != 4 ) {
        return -1;
    }

    if (myNsId != atoi(storageInfoTokens[1].c_str()) || myClusterId != storageInfoTokens[3]) {
        ostringstream ostr;
        ostr << "This node has namespaceId '" << myNsId <<" and clusterId '"
                << myClusterId << "' but the requesting node expected '" << storageInfoTokens[1]
                << "' and '" << storageInfoTokens[3] << "'";
        LOG.warn("Received an invalid file transfer request %s ", ostr.str().c_str());
        return -1;
    }

    return 0;
}

static int send_reply(struct mg_connection *conn) {
  char jid[100], segmentTxIdArr[100], storageInfo[500];
  if (strcmp(conn->uri, "/getJournal") == 0) {
    // Parse form data. var1 and var2 are guaranteed to be NUL-terminated
    mg_get_var(conn, "jid", jid, sizeof(jid));
    mg_get_var(conn, "segmentTxId", segmentTxIdArr, sizeof(segmentTxIdArr));
    mg_get_var(conn, "storageInfo", storageInfo, sizeof(storageInfo));

    Journal* journal_ptr;
    const string jidname(jid);
    global_jn->getOrCreateJournal(jidname, journal_ptr);

    JNStorage& storage = journal_ptr->getStorage();

    int rc = checkStorageInfo(storage, storageInfo);
    if(rc != 0) {
        // 412 - Precondition failed
        mg_send_status(conn, 412);
        mg_send_data(conn, "", 0);
        return MG_TRUE;
    }

    char *end;
    long segmentTxId = strtol(segmentTxIdArr, &end, 10);

    FileJournalManager& fjm = journal_ptr->getFileJournalManager();
    EditLogFile elf;

    if(fjm.getLogFile(segmentTxId, elf) != 0) {
        // 404- resource not found
        mg_send_status(conn, 404);
        mg_send_data(conn, "", 0);
        return MG_TRUE;
    }
    if(!elf.isInitialized()){
        // 404- resource not found
        mg_send_status(conn, 404);
        mg_send_data(conn, "", 0);
        return MG_TRUE;
    }

//    std::ifstream t(elf.getFile().c_str());
//
//    std::stringstream buffer;
//    buffer << t.rdbuf();
//    t.close();
////    LOG.info("HTTP SERVER : data read from file is '%s'", buffer.str().c_str());
//    LOG.info("HTTP SERVER : length of data read from file is %d ",buffer.str().length());
//    if(elf.isInProgress())
//        LOG.info("HTTP SERVER : elf is in progress");
//    else
//        LOG.info("HTTP SERVER : elf is finalized");

    mg_send_file(conn, elf.getFile().c_str());
    return MG_MORE;    // It is important to return MG_MORE after mg_send_file!
  }
}

static int
ev_handler(struct mg_connection *conn, enum mg_event ev) {
    if (ev == MG_AUTH) {
        return MG_TRUE;   // Authorize all requests
    }else if (ev == MG_REQUEST && !strcmp(conn->uri, "/getJournal")) {
        LOG.info("HTTP SERVER : received request to server following file '%s'", conn->query_string);
        return send_reply(conn);
    }else {
        return MG_FALSE;  // Rest of the events are not processed
    }
}

void* JournalNodeHttpServer::run_httpserver(void* arg) {
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    struct mg_server *server = mg_create_server(NULL, ev_handler);
    mg_set_option(server, "listening_port", listening_port.c_str());

    LOG.info("Starting http server on port %s ", mg_get_option(server, "listening_port"));
    while(signal_received == 0) {
        mg_poll_server(server, 1000);
    }
    mg_destroy_server(&server);
}

int
JournalNodeHttpServer::start_httpserver(string port) {
  listening_port = port;
  pthread_t thread;
  pthread_create(&thread, 0, &run_httpserver, 0);

  return 0;
}

} /* namespace JournalServiceServer */
