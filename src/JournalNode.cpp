/*
 * JournalNode.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "JournalNode.h"

namespace JournalServiceServer
{

int
JournalNode::getLogDir(string& jid, string& logDir) {
    string dir = conf.getValue(DFS_JOURNALNODE_EDITS_DIR_KEY,DFS_JOURNALNODE_EDITS_DIR_DEFAULT);
    if(jid.empty()) {
        return -1;
    }

    logDir = dir + "/" +jid;
    return 0;
  }

int
JournalNode::getOrCreateJournal(const string& jid, Journal* journal) {
    map<string, Journal*>::const_iterator pos = journalsById.find(jid);
    if (pos == map.end()) {
        string logDir;
        if(getLogDir(jid, logDir) != 0)
            return -1;
        cout << "Initializing journal in directory " + logDir;
        journal = new Journal(conf, logDir, jid);
        journalsById.put(jid, journal);
    } else {
        journal = pos->second;
    }
    return 0;
  }

}
