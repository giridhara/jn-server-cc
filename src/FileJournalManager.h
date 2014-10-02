/*
 * FileJournalManager.h
 *
 *  Created on: Sep 7, 2014
 *      Author: psarda
 */

#ifndef FILEJOURNALMANAGER_H_
#define FILEJOURNALMANAGER_H_

#include "JNStorage.h"
#include "EditLogFile.h"
#include <vector>
#include <string>
#include <iostream>
#include "JNClientOutputStream.h"
#include "boost/scoped_ptr.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

using std::vector;
using std::string;
using boost::scoped_ptr;

namespace JournalServiceServer
{

static const boost::regex IN_PROGRESS_PATTERN("^edits_inprogress_(\\d+)$");
static const boost::regex FINALIZED_PATTERN("^edits_(\\d+)-(\\d+)$");

class FileJournalManager
{
public:
    FileJournalManager(JNStorage& storage)
        :
            jnStorage(storage),
            currentInProgress()
    {}
    virtual ~FileJournalManager() {}

    int getLogFiles(const long fromTxId, vector<EditLogFile>& ret);
    int getRemoteEditLogs(const long firstTxId, const bool inProgressOk, vector<EditLogFile>& ret);
    int getLogFile(const long startTxId, EditLogFile&);
    int finalizeLogSegment(const long firstTxId, const long lastTxId);
    int startLogSegment(const long txid, const int layoutVersion, scoped_ptr<JNClientOutputStream>&);

private:
    int getLogFile(const string& dir, long startTxId, EditLogFile&);
    int GetFilesInDirectory(const string& directory, vector<string> &out);
    int matchEditLogs(const string& dir, vector<EditLogFile> & ret );
    int matchEditLogs(const string& dir, const vector<string>& filesInStorage, vector<EditLogFile>& ret);

    JNStorage& jnStorage;
    string currentInProgress;
    boost::recursive_mutex mMutex;
};

} /* namespace JournalServiceServer */

#endif /* FILEJOURNALMANAGER_H_ */
