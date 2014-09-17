/*
 * JournalNodeHttpServer.cpp
 *
 *  Created on: Sep 17, 2014
 *      Author: psarda
 */

#include "JournalNodeHttpServer.h"
#include "../util/Logger.h"
#include <sstream>

namespace JournalServiceServer
{

JournalNodeHttpServer::JournalNodeHttpServer()
{
    // TODO Auto-generated constructor stub
    
}

JournalNodeHttpServer::~JournalNodeHttpServer()
{
    // TODO Auto-generated destructor stub
}

int checkStorageInfo(JNStorage& storage, char* storageInfo) {
    int myNsId = storage.getNamespaceID();
    string myClusterId = storage.getClusterID();
    char *tok = 0;
    tok = strtok(storageInfo, ":");
    vector<string> storageInfoTokens;
    while (tok) {
        printf("Token: %s\n", tok);
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

static void send_reply(struct mg_connection *conn) {
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
        return;
    }

    char *end;
    long segmentTxId = strtol(segmentTxIdArr, &end, 10);

    FileJournalManager& fjm = journal_ptr->getFileJournalManager();
    EditLogFile elf;

    if(fjm.getLogFile(segmentTxId, elf) != 0) {
        return;
    }
    if(!elf.isInitialized()){
        return;
    }

    mg_send_file(conn, elf.getFile().c_str());
  }
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
    if (ev == MG_AUTH) {
        return MG_TRUE;   // Authorize all requests
    }else if (ev == MG_REQUEST && !strcmp(conn->uri, "/getJournal")) {
        send_reply(conn);
        return MG_MORE;   // It is important to return MG_MORE after mg_send_file!
    }else {
        return MG_FALSE;  // Rest of the events are not processed
    }
}

//int main(void) {
//  struct mg_server *server = mg_create_server(NULL, ev_handler);
//  mg_set_option(server, "listening_port", "8080");
//
//  printf("Starting on port %s\n", mg_get_option(server, "listening_port"));
//  for (;;) mg_poll_server(server, 1000);
//  mg_destroy_server(&server);
//
//  return 0;
//}

} /* namespace JournalServiceServer */
