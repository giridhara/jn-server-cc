/*
 * JournalNode.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "JournalNode.h"

namespace JournalServiceServer
{

scoped_ptr<JournalNode> global_jn;

JournalNode::~JournalNode() {
    for(map<string, Journal*>::const_iterator iter = journalsById.begin(); iter != journalsById.end(); iter++) {
        LOG.info("Closing journal %s", iter->first.c_str());
        iter->second->close();
        delete (iter->second);
    }
}

int
JournalNode::getLogDir(const string& jid, string& logDir) {
    if(jid.empty()) {
        LOG.error("journal identifier is empty");
        abort();
    }
    string dir;
    dir = conf->getProperty(DFS_JOURNALNODE_EDITS_DIR_KEY);
    if(dir.empty()) {
        dir = DFS_JOURNALNODE_EDITS_DIR_DEFAULT;
    }

    logDir = dir + "/" +jid;
    return 0;
}

int
JournalNode::getOrCreateJournal(const string& jid, Journal*& journal) {
    map<string, Journal*>::const_iterator pos = journalsById.find(jid);
    if (pos == journalsById.end()) {
        string logDir;
        if(getLogDir(jid, logDir) != 0)
            return -1;
        LOG.info("Initializing journal in directory '%s'", logDir.c_str());
        journal = new Journal(conf, logDir, jid);
        if (!journal->isInitialized()) {
            delete journal;
            return -1;
        }
        journalsById[jid] = journal;
    } else {
        journal = pos->second;
    }
    return 0;
  }

void
JournalNode::validateAndCreateJournalDir(const string& dir) {
    if (!is_absolute_path(dir)) {
      LOG.error("Journal dir '%s' should be an absolute path", dir.c_str());
      abort();
    }

    bool journal_dir_exists_flag = false;
    if(dir_exists(dir, journal_dir_exists_flag) != 0 ) {
      abort();
    }

    if (!journal_dir_exists_flag) {
      bool is_created = false;
      try {
          is_created = boost::filesystem::create_directories(dir);
      }catch(const boost::filesystem::filesystem_error& ex){
          LOG.error("%s", ex.what());
          abort();
      }
      if(!is_created) {
          LOG.error("Could not create Journal dir '%s'", dir.c_str());
          abort();
      }
    }
}

void
JournalNode::start() {
    validateAndCreateJournalDir(localDir);

    ostringstream ostr;
    ostr << httpPort;

    JournalNodeHttpServer::start_httpserver(ostr.str());

    rpcServer = new JournalNodeRpcServer(conf, *this);
    rpcServer->start();
}
}

int main(int argc, char** argv){
    string progname = "journalnode";
    if(argc !=2) {
        cerr << "Usage: " << progname <<
                       "  <properties file> \n";
                   return 0;
    }

    Ice::PropertiesPtr properties = Ice::createProperties();
    properties->load(argv[1]);

    (JournalServiceServer::global_jn).reset(new JournalServiceServer::JournalNode(properties));
    JournalServiceServer::global_jn->start();
}
