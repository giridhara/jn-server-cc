/*
 * JNStorage.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "JNStorage.h"

using namespace std;

namespace JournalServiceServer
{

int
JNStorage::writeProperties(string to) {
    ostringstream propstream;
    propstream << "layoutVersion=" << getLayoutVersion() << "\n";
    propstream << "storageType=" << "JOURNAL_NODE" << "\n";
    propstream << "namespaceID=" << getNamespaceID() << "\n";
    propstream << "clusterID=" << getClusterID() << "\n";
    propstream << "cTime=" << getCTime() << "\n";

    ofstream versionStream(to.c_str());
    if(!versionStream.is_open())
      return -1;
    versionStream << propstream;
    versionStream.close();

    return 0;
}

int
JNStorage::clearLogDirectory() {
    boost::system::error_code error;
    boost::filesystem::exists(currentDir.c_str(), error);
    if (!error){
        error.clear();
        boost::filesystem::remove_all(currentDir, error);
        if(error){
            cout << "Cannot remove current directory: " << currentDir;
            return -1;
        }
    }
    error.clear();
    boost::filesystem::create_directories(currentDir, error);
    if(error){
        cout << "could not create current dir " << currentDir;
        return -1;
    }
    return 0;
}

int
JNStorage::format(const NamespaceInfo& nsInfo) {
    setStorageInfo(nsInfo);
    cout << "Formatting journal " << logDir  << " with nsid: "  << getNamespaceID();
    // Unlock the directory before formatting, because we will
    // re-analyze it after format(). The analyzeStorage() call
    // below is reponsible for re-locking it. This is a no-op
    // if the storage is not currently locked.
//TODO:    unlockAll();
    clearLogDirectory();
    writeProperties(getVersionFile());
    int cs = createPaxosDir();
    if(cs != 0) {
        cout << "Could not create paxos dir: " << getPaxosDir();
    }else {
        cout << "Created paxos dir: " << getPaxosDir();
    }
    // TODO :As part of analyze storage , once the storagestate is normal , they are reading back again properties from VERSION file
    // To me it looks redundant. Hence skipping analyzeStorage for now
    // Will revisit this decision
    //    analyzeStorage();
    return 0;
}

int
JNStorage::checkConsistentNamespace(NamespaceInfo nsInfo) {
    if (nsInfo.getNamespaceID() != getNamespaceID()) {
        cout << "Incompatible namespaceID for journal " <<
          logDir << ": Metaserver has nsId " << nsInfo.getNamespaceID() <<
          " but storage has nsId " << getNamespaceID();
        return -1;
    }

    if (nsInfo.getClusterID() != getClusterID()) {
      cout << "Incompatible clusterID for journal " <<
          logDir << ": NameNode has clusterId '" << nsInfo.getClusterID() <<
          "' but storage has clusterId '" << getClusterID() << "'";
      return -1;
    }

    return 0;
  }

} /* namespace JournalServiceServer */
