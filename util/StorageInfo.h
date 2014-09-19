/*
 * StorageInfo.h
 *
 *  Created on: Sep 3, 2014
 *      Author: psarda
 */

#ifndef STORAGEINFO_H_
#define STORAGEINFO_H_


#include <QJournalProtocolPB.h>

#include <stdint.h>
#include <string>
using namespace std;

namespace JournalServiceServer
{

const string STORAGE_FILE_VERSION  = "VERSION";

class StorageInfo
{
public:
    StorageInfo(uint32_t layoutV, string cid, uint32_t nsID,  uint64_t cT)
        :
        layoutVersion(layoutV),
        clusterID(cid),
        namespaceID(nsID),
        cTime(cT)
     {}

    StorageInfo() {
        StorageInfo(0, "", 0, 0);
    }

    void setStorageInfo(const StorageInfo& from) {
        layoutVersion = from.layoutVersion;
        clusterID = from.clusterID;
        namespaceID = from.namespaceID;
        cTime = from.cTime;
    }

    virtual ~StorageInfo(){}

    /**
       * Layout version of the storage data.
       */
    int getLayoutVersion(){ return layoutVersion; }

    /**
    * Namespace id of the file system.<p>
    * Assigned to the file system at formatting and never changes after that.
    * Shared by all file system components.
    */
    int getNamespaceID() const { return namespaceID; }

    /**
    * cluster id of the file system.<p>
    */
    string    getClusterID() { return clusterID; }

    /**
    * Creation time of the file system state.<p>
    * Modified during upgrades.
    */
     long getCTime() { return cTime; }

private:
    int   layoutVersion;   // layout version of the storage data
    string clusterID;      // id of the cluster
    int   namespaceID;     // id of the file system
    uint64_t  cTime;           // creation time of the file system state
};

}
#endif /* STORAGEINFO_H_ */
