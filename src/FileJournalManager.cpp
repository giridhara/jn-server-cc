/*
 * FileJournalManager.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: psarda
 */

#include "FileJournalManager.h"
#include "../util/JNServiceMiscUtils.h"
#include <stdlib.h>
#include <boost/regex.hpp>
#include "../util/Constants.h"
#include "../util/Logger.h"
#include <algorithm>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

namespace JournalServiceServer
{

FileJournalManager::~FileJournalManager()
{
}

/**
 * Find all editlog segments starting at or above the given txid.
 * @param fromTxId the txnid which to start looking
 * @param inProgressOk whether or not to include the in-progress edit log
 *        segment
 * @return a list of remote edit logs
 * @throws IOException if edit logs cannot be listed.
 */
int
FileJournalManager::getRemoteEditLogs(const long firstTxId, const bool inProgressOk, vector<EditLogFile>& ret) {
    cout << "got request to find all edit log segments after " << firstTxId << " in progress segments are also fine ?" << inProgressOk << endl;
    string currentDir = jnStorage.getCurrentDir();
    vector<EditLogFile> allLogFiles;
    if (matchEditLogs(currentDir, allLogFiles) != 0) {
        return -1;
    }

    for (vector<EditLogFile>::iterator it = allLogFiles.begin(); it != allLogFiles.end(); ++it) {
        if((*it).hasCorruptHeader() || (!inProgressOk && (*it).isInProgress())) {
            continue;
        }
        EditLogFile elf(" ", (*it).getFirstTxId(), (*it).getLastTxId(), false);
        if((*it).getFirstTxId() >= firstTxId) {
            ret.push_back(elf);
        } else if((*it).getFirstTxId() < firstTxId && firstTxId <= (*it).getLastTxId()) {
            // If the firstTxId is in the middle of an edit log segment. Return this
            // anyway and let the caller figure out whether it wants to use it.
            ret.push_back(elf);
        }
    }

    sort(ret.begin(), ret.end());
    return 0;
}

int
FileJournalManager::startLogSegment(long txid, int layoutVersion, scoped_ptr<JNClientOutputStream>& ret) {
    currentInProgress = getInProgressEditsFile(jnStorage.getCurrentDir(), txid);
    ret.reset(new JNClientOutputStream(currentInProgress));

    if(!ret->create(layoutVersion)) {
        return -1;
    }

    return 0;
}

int
FileJournalManager::finalizeLogSegment(long firstTxId, long lastTxId) {
    string inProgressFile = getInProgressEditsFile(jnStorage.getCurrentDir(), firstTxId);
    string dstFile = getFinalizedEditsFile(jnStorage.getCurrentDir(), firstTxId, lastTxId);
    LOG.info("Finalizing edits file %s -> %s" , inProgressFile.c_str() , dstFile.c_str() );
    if (file_exists(dstFile)) {
        LOG.error("Can't finalize edits file %s since finalized file already exists", dstFile.c_str());
        return -1;
    }

    int rc = file_rename(inProgressFile, dstFile);

    if(rc != 0 ) {
        LOG.error("Can't rename file %s to %s" , inProgressFile.c_str() , dstFile.c_str());
        return -1;
    }

    if(inProgressFile ==  currentInProgress)
        currentInProgress="";

    return 0;
}

int
FileJournalManager::getLogFiles(long fromTxId, vector<EditLogFile>& ret){
    cout << "getting all logs from txid : " << fromTxId << endl;
    string currentDir = jnStorage.getCurrentDir();
    vector<EditLogFile> allLogFiles;
    if(matchEditLogs(currentDir, allLogFiles) != 0){
        return -1;
    }

    for (vector<EditLogFile>::iterator it = allLogFiles.begin(); it != allLogFiles.end(); ++it) {
        if (fromTxId <= (*it).getFirstTxId() ||
                (*it).containsTxId(fromTxId)) {
            cout << "pushing elf[ " << it->getFirstTxId() << "," << it->getLastTxId() << "]" << endl;
            ret.push_back(*it);
        }
    }

    sort(ret.begin(), ret.end());
    return 0;
}

int
FileJournalManager::getLogFile(long startTxId, EditLogFile& ret) {
    return getLogFile(jnStorage.getCurrentDir(), startTxId, ret);
}

int
FileJournalManager::getLogFile(string dir, long startTxId, EditLogFile& result) {
    vector<EditLogFile> matchedEditLogs;
    if(matchEditLogs(dir, matchedEditLogs) != 0) {
        return -1;
    }
    vector<EditLogFile > retEditLogFile;
    for (vector<EditLogFile>::iterator it = matchedEditLogs.begin(); it != matchedEditLogs.end(); ++it) {
        if ((*it).getFirstTxId() == startTxId) {
            cout << "pushing elf[ " << it->getFirstTxId() << "," << it->getLastTxId() << "]" << endl;
            retEditLogFile.push_back(*it);
        }
    }
    
    if (retEditLogFile.empty()) {
        // no matches
        return 0;
    } else if (retEditLogFile.size() == 1) {
        result = retEditLogFile.front();  //  assignment operator of EditLogFile function is called
        //retEditLogFile.front();
        return 0;
    }

    LOG.error("More than one log segment in %s starting at txid %d" , dir.c_str(), startTxId);
    return -1;
}

int FileJournalManager::GetFilesInDirectory(const string& directory, vector<string> &out) {
    try{
        boost::filesystem::directory_iterator begin(directory);
        boost::filesystem::directory_iterator end;
        for (; begin != end; ++begin) {
            if (boost::filesystem::is_regular_file(begin->status())) {
                std::stringstream temp;
                temp << begin->path().leaf();
                out.push_back(temp.str().c_str());
            }
        }
    }catch(const boost::filesystem::filesystem_error& ex){
        LOG.error("%s", ex.what());
        return -1;
    }
    return 0;
}

int FileJournalManager::matchEditLogs(const string& dir,
        vector<EditLogFile>& ret) {
    vector<string> filenames;
    if (GetFilesInDirectory(dir, filenames) != 0) {
        return -1;
    }
    return matchEditLogs(dir, filenames, ret);
}

int FileJournalManager::matchEditLogs(const string& dir,  const vector<string>& filesInStorage,
        vector<EditLogFile>& ret)
{
    for (vector<string>::const_iterator it = filesInStorage.begin();
            it != filesInStorage.end(); ++it) {
        // Check for edits
        boost::smatch finalizedMatchResults;
        try{
            if (boost::regex_match(*it, finalizedMatchResults, FINALIZED_PATTERN)) {
                string startTxStr(finalizedMatchResults[1]);
                string endTxStr(finalizedMatchResults[2]);
                long startTxId = std::strtol(startTxStr.c_str(), 0, 10);
                long endTxId = std::strtol(endTxStr.c_str(), 0, 10);
                EditLogFile elf(dir + "/" + (*it), startTxId, endTxId, false);
                ret.push_back(elf);
                continue;
            }

            // Check for in-progress edits
            boost::smatch inProgressMatchResults;
            if (boost::regex_match(*it, inProgressMatchResults,
                    IN_PROGRESS_PATTERN)) {
                string startTxStr(inProgressMatchResults[1]);
                long startTxId = std::strtol(startTxStr.c_str(), 0, 10);
                EditLogFile elf(dir + "/" + (*it), startTxId, INVALID_TXID, true);
                ret.push_back(elf);
            }
        }catch(const std::runtime_error& ex){
            LOG.error("%s", ex.what());
            return -1;
        }
    }
    return 0;
}

} /* namespace JournalServiceServer */

//int main() {
//    JournalServiceServer::JNStorage storage("/home/psarda/qfsbase/jn/data/sample-cluster-changed");
//    JournalServiceServer::FileJournalManager fjm(storage);
//    JournalServiceServer::EditLogFile elf;
//    int rc = fjm.getLogFile(10, elf);
//    if(rc == 0){
//        if(elf.getFirstTxId() != -1) {
//            cout << "first txid is " << elf.getFirstTxId() << endl;
//            cout << "last txid is " << elf.getLastTxId() << endl;
//        } else {
//            cout << "there is no segment which starts at 10" << endl;
//        }
//    }else {
//        cout << "result code is non zero" << endl;
//    }
//
//    vector<JournalServiceServer::EditLogFile> elfv;
//    fjm.getLogFiles(1, elfv);
//
//    for (vector<JournalServiceServer::EditLogFile>::iterator it = elfv.begin(); it != elfv.end(); ++it) {
//        cout << "edit log file name is '" << (*it).getFile() << "'" << endl;
//    }
//
//}
