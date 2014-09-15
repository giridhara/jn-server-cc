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

using std::vector;
using std::string;
using boost::scoped_ptr;

namespace JournalServiceServer
{

static const boost::regex IN_PROGRESS_PATTERN("^edits_inprogress_(\\d+)$");

static const boost::regex FINALIZED_PATTERN("^edits_(\\d+)_(\\d+)$");

class FileJournalManager
{
public:
    FileJournalManager(JNStorage& storage)
        :
            jnStorage(storage),
            currentInProgress()
    {}
    virtual ~FileJournalManager();

    void getLogFiles(long fromTxId, vector<EditLogFile>& ret);
    void getRemoteEditLogs(const long firstTxId, const bool inProgressOk, vector<EditLogFile>& ret);
    int getLogFile(long startTxId, EditLogFile&);
    int getLogFile(string dir, long startTxId, EditLogFile&);

    void matchEditLogs(string logDir, vector<EditLogFile> & ret );

    void matchEditLogs(const vector<string>& filesInStorage, vector<EditLogFile>& ret);

    int finalizeLogSegment(long firstTxId, long lastTxId);
    int startLogSegment(long txid, int layoutVersion, scoped_ptr<JNClientOutputStream>&);

    void GetFilesInDirectory(std::vector<string>& out, const string& directory);

private:

    JNStorage& jnStorage;
    string currentInProgress;
};

} /* namespace JournalServiceServer */

#endif /* FILEJOURNALMANAGER_H_ */
