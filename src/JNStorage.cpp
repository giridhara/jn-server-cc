/*
 * JNStorage.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "JNStorage.h"
#include "util/Logger.h"

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
    bool dir_exists = false;
    try{
        dir_exists = boost::filesystem::exists(currentDir.c_str());
        if (dir_exists){
            boost::filesystem::remove_all(currentDir.c_str());
        }
        boost::filesystem::create_directory(currentDir);
    }catch (const boost::filesystem::filesystem_error& ex){
        cout << ex.what() << '\n';
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


int
JNStorage::analyzeStorage() {
    bool storagedir_exists = false;

    try { // check that storage exists
        storagedir_exists = boost::filesystem::exists(logDir.c_str());
    }catch (const boost::filesystem::filesystem_error& ex){
        LOG.error("%s \n", ex.what());
        return -1;
    }
    if (!storagedir_exists) {
      // storage directory does not exist
     // if (startOpt != StartupOption.FORMAT) {
        LOG.info("Storage directory %s does not exist", logDir.c_str());
        state = NON_EXISTENT;
        return 0;
     // }
//      LOG.info(rootPath + " does not exist. Creating ...");
//      if (!root.mkdirs())
//        throw new IOException("Cannot create directory " + rootPath);
    }
    return 0;
    // or is inaccessible
//    if (!root.isDirectory()) {
//      LOG.warn(rootPath + "is not a directory");
//      return StorageState.NON_EXISTENT;
//    }
//    if (!FileUtil.canWrite(root)) {
//      LOG.warn("Cannot access storage directory " + rootPath);
//      return StorageState.NON_EXISTENT;
//    }
//    } catch(SecurityException ex) {
//    LOG.warn("Cannot access storage directory " + rootPath, ex);
//    return StorageState.NON_EXISTENT;
//    }

//    this.lock(); // lock storage if it exists
//
//    if (startOpt == HdfsServerConstants.StartupOption.FORMAT)
//    return StorageState.NOT_FORMATTED;
//
//    if (startOpt != HdfsServerConstants.StartupOption.IMPORT) {
//    storage.checkOldLayoutStorage(this);
//    }
//
//    // check whether current directory is valid
//    File versionFile = getVersionFile();
//    boolean hasCurrent = versionFile.exists();
//
//    // check which directories exist
//    boolean hasPrevious = getPreviousDir().exists();
//    boolean hasPreviousTmp = getPreviousTmp().exists();
//    boolean hasRemovedTmp = getRemovedTmp().exists();
//    boolean hasFinalizedTmp = getFinalizedTmp().exists();
//    boolean hasCheckpointTmp = getLastCheckpointTmp().exists();
//
//    if (!(hasPreviousTmp || hasRemovedTmp
//      || hasFinalizedTmp || hasCheckpointTmp)) {
//    // no temp dirs - no recovery
//    if (hasCurrent)
//      return StorageState.NORMAL;
//    if (hasPrevious)
//      throw new InconsistentFSStateException(root,
//                          "version file in current directory is missing.");
//    return StorageState.NOT_FORMATTED;
//    }
//
//    if ((hasPreviousTmp?1:0) + (hasRemovedTmp?1:0)
//      + (hasFinalizedTmp?1:0) + (hasCheckpointTmp?1:0) > 1)
//    // more than one temp dirs
//    throw new InconsistentFSStateException(root,
//                                           "too many temporary directories.");
//
//    // # of temp dirs == 1 should either recover or complete a transition
//    if (hasCheckpointTmp) {
//    return hasCurrent ? StorageState.COMPLETE_CHECKPOINT
//                      : StorageState.RECOVER_CHECKPOINT;
//    }
//
//    if (hasFinalizedTmp) {
//    if (hasPrevious)
//      throw new InconsistentFSStateException(root,
//                                             STORAGE_DIR_PREVIOUS + " and " + STORAGE_TMP_FINALIZED
//                                             + "cannot exist together.");
//    return StorageState.COMPLETE_FINALIZE;
//    }
//
//    if (hasPreviousTmp) {
//    if (hasPrevious)
//      throw new InconsistentFSStateException(root,
//                                             STORAGE_DIR_PREVIOUS + " and " + STORAGE_TMP_PREVIOUS
//                                             + " cannot exist together.");
//    if (hasCurrent)
//      return StorageState.COMPLETE_UPGRADE;
//    return StorageState.RECOVER_UPGRADE;
//    }
//
//    assert hasRemovedTmp : "hasRemovedTmp must be true";
//    if (!(hasCurrent ^ hasPrevious))
//    throw new InconsistentFSStateException(root,
//                                           "one and only one directory " + STORAGE_DIR_CURRENT
//                                           + " or " + STORAGE_DIR_PREVIOUS
//                                           + " must be present when " + STORAGE_TMP_REMOVED
//                                           + " exists.");
//    if (hasCurrent)
//    return StorageState.COMPLETE_ROLLBACK;
//    return StorageState.RECOVER_ROLLBACK;
}

} /* namespace JournalServiceServer */
