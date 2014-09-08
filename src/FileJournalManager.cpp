/*
 * FileJournalManager.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: psarda
 */

#include "FileJournalManager.h"
#include <stdlib.h>
#include "/usr/local/include/boost/filesystem/operations.hpp"
#include </usr/local/include/boost/regex.hpp>
#include "../util/Constants.h"
#include "../util/Logger.h"

namespace JournalServiceServer
{

FileJournalManager::~FileJournalManager()
{
    // TODO Auto-generated destructor stub
}

//vector<EditLogFile>
//FileJournalManager::getLogFiles(long fromTxId){
//    string currentDir = jnStorage.getCurrentDir();
//    List<EditLogFile> allLogFiles = matchEditLogs(currentDir);
//    List<EditLogFile> logFiles = Lists.newArrayList();
//
//    for (EditLogFile elf : allLogFiles) {
//      if (fromTxId <= elf.getFirstTxId() ||
//          elf.containsTxId(fromTxId)) {
//        logFiles.add(elf);
//      }
//    }
//
//    Collections.sort(logFiles, EditLogFile.COMPARE_BY_START_TXID);
//
//    return logFiles;
//  }
//


int
FileJournalManager::getLogFile(long startTxId, shared_ptr<EditLogFile>& ret) {
    cout << "current dir is " << jnStorage.getCurrentDir() << endl;
    return getLogFile(jnStorage.getCurrentDir(), startTxId, ret);
}

int
FileJournalManager::getLogFile(string dir, long startTxId, shared_ptr<EditLogFile>& result)
{
    vector<shared_ptr<EditLogFile> > matchedEditLogs;
    matchEditLogs(dir, matchedEditLogs);
    vector<shared_ptr<EditLogFile> > retEditLogFile;
    for (vector<shared_ptr<EditLogFile> >::iterator it = matchedEditLogs.begin(); it != matchedEditLogs.end(); ++it) {
        if ((*it)->getFirstTxId() == startTxId) {
            retEditLogFile.push_back(*it);
        }
    }
    
    if (retEditLogFile.empty()) {
        cout << "no matches :-(" << endl;
        // no matches
        //TODO : Assuming that compiler will initialize result to zero by default
        return 0;
    } else if (retEditLogFile.size() == 1) {
        cout << "yayyy, size is 1" << endl;
        cout << " start txid is " << retEditLogFile.front()->getFirstTxId() << endl;
        cout << " end txid is " << retEditLogFile.front()->getLastTxId() << endl;
        result = retEditLogFile.front();
        cout << " ++start txid is " << result->getFirstTxId() << endl;
                cout << " ++end txid is " << result->getLastTxId() << endl;
        //retEditLogFile.front();
        return 0;
    }

    LOG.error("More than one log segment in %s starting at txid %d" , dir.c_str(), startTxId);
    return -1;
}
//
void FileJournalManager::matchEditLogs(string logDir,
        vector<shared_ptr<EditLogFile> >& ret)
{
    vector<string> filenames;
    boost::filesystem::directory_iterator begin(logDir);
    boost::filesystem::directory_iterator end;
    for (; begin != end; ++begin) {
        if (boost::filesystem::is_regular_file(begin->status())) {
            std::stringstream temp;
            temp << begin->path().filename().string();
            cout << " found file " << temp.str() << endl;
            filenames.push_back(temp.str().c_str());
        }
    }
    return matchEditLogs(filenames, ret);
}

void FileJournalManager::matchEditLogs(const vector<string> filesInStorage,
        vector<shared_ptr<EditLogFile> >& ret)
{
    for (vector<string>::const_iterator it = filesInStorage.begin();
            it != filesInStorage.end(); ++it) {
        cout << " trying to match '" << (*it).c_str() <<"'" << endl;
        // Check for edits
        boost::smatch finalizedMatchResults;
        if (boost::regex_match(*it, finalizedMatchResults, FINALIZED_PATTERN)) {
            cout << "finalized pattern matched on '" << *it << "'" << endl;
            string startTxStr(finalizedMatchResults[1]);
            string endTxStr(finalizedMatchResults[2]);
            long startTxId = std::strtol(startTxStr.c_str(), 0, 10);
            long endTxId = std::strtol(endTxStr.c_str(), 0, 10);
            shared_ptr<EditLogFile> elfp(
                    new EditLogFile(*it, startTxId, endTxId, false));
            ret.push_back(elfp);
            continue;
        }
        
        // Check for in-progress edits
        boost::smatch inProgressMatchResults;
        if (boost::regex_match(*it, inProgressMatchResults,
                IN_PROGRESS_PATTERN)) {
            cout << "inprogress pattern matched on '" << *it << "'" << endl;
            string startTxStr(inProgressMatchResults[1]);
            long startTxId = std::strtol(startTxStr.c_str(), 0, 10);
            shared_ptr<EditLogFile> elfp(
                    new EditLogFile(*it, startTxId, INVALID_TXID, true));
            ret.push_back(elfp);
        }
    }
}

} /* namespace JournalServiceServer */

int main() {
    JournalServiceServer::JNStorage storage("/home/psarda/qfsbase/jn/data/sample-cluster-changed");
    JournalServiceServer::FileJournalManager fjm(storage);
    boost::shared_ptr<JournalServiceServer::EditLogFile> elf;
    int rc = fjm.getLogFile(10, elf);
    if(rc == 0){
        if(elf) {
            cout << "first txid is " << elf->getFirstTxId() << endl;
            cout << "last txid is " << elf->getLastTxId() << endl;
        } else {
            cout << "there is no segment which starts at 10" << endl;
        }
    }else {
        cout << "result code is non zero" << endl;
    }
}
