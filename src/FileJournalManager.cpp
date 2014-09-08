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
#include <stdio.h>

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
FileJournalManager::getLogFile(long startTxId, EditLogFile& ret) {
    return getLogFile(jnStorage.getCurrentDir(), startTxId, ret);
}

int
FileJournalManager::getLogFile(string dir, long startTxId, EditLogFile& result)
{
    vector<EditLogFile> matchedEditLogs;
    matchEditLogs(dir, matchedEditLogs);
    vector<EditLogFile > retEditLogFile;
    for (vector<EditLogFile>::iterator it = matchedEditLogs.begin(); it != matchedEditLogs.end(); ++it) {
        if ((*it).getFirstTxId() == startTxId) {
            retEditLogFile.push_back(*it);
        }
    }
    
    if (retEditLogFile.empty()) {
        // no matches
        //TODO : Assuming that compiler will initialize result to zero by default
        return 0;
    } else if (retEditLogFile.size() == 1) {
        result = retEditLogFile.front();  //  copy constructor
        //retEditLogFile.front();
        return 0;
    }

    LOG.error("More than one log segment in %s starting at txid %d" , dir.c_str(), startTxId);
    return -1;
}
//
void FileJournalManager::matchEditLogs(string logDir,
        vector<EditLogFile>& ret)
{
    vector<string> filenames;
    boost::filesystem::directory_iterator begin(logDir);
    boost::filesystem::directory_iterator end;
    for (; begin != end; ++begin) {
        if (boost::filesystem::is_regular_file(begin->status())) {
            std::stringstream temp;
            temp << begin->path().filename().string();
            filenames.push_back(temp.str().c_str());
        }
    }
    return matchEditLogs(filenames, ret);
}

void FileJournalManager::matchEditLogs(const vector<string>& filesInStorage,
        vector<EditLogFile>& ret)
{
    for (vector<string>::const_iterator it = filesInStorage.begin();
            it != filesInStorage.end(); ++it) {
        // Check for edits
        boost::smatch finalizedMatchResults;
        if (boost::regex_match(*it, finalizedMatchResults, FINALIZED_PATTERN)) {
            string startTxStr(finalizedMatchResults[1]);
            string endTxStr(finalizedMatchResults[2]);
            long startTxId = std::strtol(startTxStr.c_str(), 0, 10);
            long endTxId = std::strtol(endTxStr.c_str(), 0, 10);
            EditLogFile elf(*it, startTxId, endTxId, false);
            ret.push_back(elf);
            continue;
        }
        
        // Check for in-progress edits
        boost::smatch inProgressMatchResults;
        if (boost::regex_match(*it, inProgressMatchResults,
                IN_PROGRESS_PATTERN)) {
            string startTxStr(inProgressMatchResults[1]);
            long startTxId = std::strtol(startTxStr.c_str(), 0, 10);
            EditLogFile elf(*it, startTxId, INVALID_TXID, true);
            ret.push_back(elf);
        }
    }
}

} /* namespace JournalServiceServer */

int main() {
    JournalServiceServer::JNStorage storage("/home/psarda/qfsbase/jn/data/sample-cluster-changed");
    JournalServiceServer::FileJournalManager fjm(storage);
    JournalServiceServer::EditLogFile elf;
    int rc = fjm.getLogFile(10, elf);
    if(rc == 0){
        if(elf.getFirstTxId() != -1) {
            cout << "first txid is " << elf.getFirstTxId() << endl;
            cout << "last txid is " << elf.getLastTxId() << endl;
        } else {
            cout << "there is no segment which starts at 10" << endl;
        }
    }else {
        cout << "result code is non zero" << endl;
    }
}
