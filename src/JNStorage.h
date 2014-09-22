/*
 * JNStorage.h
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#ifndef JNSTORAGE_H_
#define JNSTORAGE_H_

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <fstream>
#include "../util/StorageInfo.h"
#include "../util/NamespaceInfo.h"
#include "../util/Constants.h"
//#include "../common/Properties.h"

//using namespace KFS;

namespace JournalServiceServer
{

using std::ostringstream;
using std::string;
using std::ofstream;

class JNStorage : public StorageInfo
{

public:
    JNStorage(string logDir) :
        logDir(logDir),
        currentDir(logDir + "/" + "current")
    {
        analyzeStorage();
    }
    virtual ~JNStorage() {}

    const string getInProgressEditLog(long startTxId) const{
        ostringstream ostr;
        ostr << currentDir << "/" << "edits_inprogress_" << startTxId;
        return ostr.str();
    }

    const string getPaxosFile(long segmentTxId) const {
        ostringstream ostr;
        ostr << getPaxosDir() << "/" << segmentTxId;
        return ostr.str();
    }

    const string getPaxosDir() const {
        ostringstream ostr;
        ostr << currentDir << "/"  << "paxos";
        return ostr.str();
    }
    /**
       * @param segmentTxId the first txid of the segment
       * @param epoch the epoch number of the writer which is coordinating
       * recovery
       * @return the temporary path in which an edits log should be stored
       * while it is being downloaded from a remote JournalNode
       */
    string getSyncLogTemporaryFile(long segmentTxId, long epoch) {
        ostringstream ostr;
        ostr << getInProgressEditLog(segmentTxId) << ".epoch=" << epoch;
        return ostr.str();
    }

    int format(const NamespaceInfo& nsInfo);

    const string getCurrentDir() const{
        return currentDir;
    }

    const string getLogDir() const{
        return logDir;
    }

    int createPaxosDir() {
        boost::filesystem::path dir(getPaxosDir());
        try{
            boost::filesystem::create_directories(dir);
        }catch (const boost::filesystem::filesystem_error& ex){
            cout << ex.what() << '\n';
            return -1;
        }
        return 0;
    }

    /**
       * @return the path for the file which contains persisted data for the
       * paxos-like recovery process for the given log segment.
       */
    string getPaxosFile(long segmentTxId) {
        ostringstream ostr;
        ostr << getPaxosDir() << "/" << segmentTxId;
        return ostr.str();
    }

    string getVersionFile() {
        ostringstream ostr;
        ostr << currentDir  << "/" << STORAGE_FILE_VERSION;
        return ostr.str();
    }

    int clearLogDirectory();

    int writeProperties(string to);

    bool isFormatted() {
        return state == NORMAL;
    }

    int checkConsistentNamespace(NamespaceInfo nsInfo);

private:
    int findFinalizedEditsFile(long startTxId, long endTxId, string& res) {
        ostringstream ostr;
        ostr << logDir << "/" << "edits_" << startTxId << "_" << endTxId;
        bool exists = false;
        try{
            exists = boost::filesystem::exists(ostr.str().c_str());
        }catch (const boost::filesystem::filesystem_error& ex){
            cout << ex.what() << '\n';
            return -1;
        }
        if (!exists) {
            return -1;
        }

        res = ostr.str();
        return 0;
    }
    int analyzeStorage();

    string logDir;
    string currentDir;
    StorageState state;
};

} /* namespace JournalServiceServer */

#endif /* JNSTORAGE_H_ */
