/*
 * JNStorage.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "JNStorage.h"
#include "util/Logger.h"
#include "util/JNServiceMiscUtils.h"

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
    versionStream << propstream.str();
    versionStream.close();

    return 0;
}

int
JNStorage::clearLogDirectory() {
    bool dir_exists = false;
    try{
        dir_exists = boost::filesystem::exists(currentDir.c_str());
        if (dir_exists){
            boost::filesystem::remove_all(currentDir.c_str());
        }
        dir_exists = boost::filesystem::exists(logDir.c_str());
        if(!dir_exists)
            boost::filesystem::create_directories(logDir);
        boost::filesystem::create_directories(currentDir);
    }catch (const boost::filesystem::filesystem_error& ex){
        LOG.error("%s", ex.what());
        return -1;
    }

    return 0;
}

int
JNStorage::format(const NamespaceInfo& nsInfo) {
    setStorageInfo(nsInfo);
    LOG.info("Formatting journal %s with nsid: %d",logDir.c_str(), getNamespaceID());
    // Unlock the directory before formatting, because we will
    // re-analyze it after format(). The analyzeStorage() call
    // below is reponsible for re-locking it. This is a no-op
    // if the storage is not currently locked.
//TODO:    unlockAll();
    if(clearLogDirectory() != 0) {
        return -1;
    }
    writeProperties(getVersionFile());
    if(createPaxosDir() != 0) {
       LOG.error("Could not create paxos dir: %s", getPaxosDir().c_str());
        return -1;
    }
    LOG.info("Created paxos dir: %s", getPaxosDir().c_str());

    if(analyzeStorage() != 0 ) {
        return -1;
    }
    if(state == NORMAL) {
        if(readProperties(getVersionFile()) != 0)
            return -1;
    }
    return 0;
}

int
JNStorage::checkConsistentNamespace(NamespaceInfo nsInfo) {
    if (nsInfo.getNamespaceID() != getNamespaceID()) {
        LOG.error("Incompatible namespaceID for journal %s : Metaserver has nsId %d  but storage has nsId %d",
          logDir.c_str(), nsInfo.getNamespaceID(),  getNamespaceID());
        return -1;
    }

    if (nsInfo.getClusterID() != getClusterID()) {
        LOG.error("Incompatible clusterID for journal %s : NameNode has clusterId '%s' but storage has clusterId '%s'",
                logDir.c_str(),nsInfo.getClusterID().c_str(), getClusterID().c_str());
      return -1;
    }

    return 0;
}


int
JNStorage::analyzeStorage() {
    bool storagedir_exists = false;
    // check that storage exists
    if(dir_exists(logDir, storagedir_exists) != 0 ) {
        return -1;
    }

    if (!storagedir_exists) {
      // storage directory does not exist
        LOG.info("Storage directory %s does not exist", logDir.c_str());
        state = NON_EXISTENT;
        return 0;
    }
//    this.lock(); // lock storage if it exists
//
    // check whether current directory is valid
    string versionFile = getVersionFile();
    bool hasCurrent = false;

    if(file_exists(versionFile, hasCurrent) !=0 ){
        return -1;
    }

    if (hasCurrent) {
        state = NORMAL;
        return 0;
    }

    state = NOT_FORMATTED;
    return 0;
}

} /* namespace JournalServiceServer */
