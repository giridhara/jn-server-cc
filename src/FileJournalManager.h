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
#include <boost/shared_ptr.hpp>
#include "JNClientOutputStream.h"

using std::vector;
using std::string;
using boost::shared_ptr;

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

    vector<EditLogFile> getLogFiles(long fromTxId);
    void getLogFiles(long fromTxId, vector<EditLogFile>& ret);
    void getRemoteEditLogs(long firstTxId, bool inProgressOk, vector<EditLogFile>& ret);
    int getLogFile(long startTxId, EditLogFile&);
    int getLogFile(string dir, long startTxId, EditLogFile&);

    void matchEditLogs(string logDir, vector<EditLogFile> & ret );

    void matchEditLogs(const vector<string>& filesInStorage, vector<EditLogFile>& ret);

    int finalizeLogSegment(long firstTxId, long lastTxId);
    int startLogSegment(long txid, int layoutVersion, JNClientOutputStream&);

private:

    JNStorage& jnStorage;
    string currentInProgress;
};

} /* namespace JournalServiceServer */

#endif /* FILEJOURNALMANAGER_H_ */
