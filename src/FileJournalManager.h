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
            jnStorage(storage)
    {}
    virtual ~FileJournalManager();

    vector<EditLogFile> getLogFiles(long fromTxId);
    int getLogFile(long startTxId, EditLogFile&);
    int getLogFile(string dir, long startTxId, EditLogFile&);

    void matchEditLogs(string logDir, vector<EditLogFile> & ret );

    void matchEditLogs(const vector<string>& filesInStorage, vector<EditLogFile>& ret);

private:

    JNStorage& jnStorage;
};

} /* namespace JournalServiceServer */

#endif /* FILEJOURNALMANAGER_H_ */
