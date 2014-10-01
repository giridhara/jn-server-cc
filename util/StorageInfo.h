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
#include <Ice/Properties.h>
#include <Ice/Ice.h>
#include "Logger.h"
#include <fstream>

using namespace std;

namespace JournalServiceServer
{

const string STORAGE_FILE_VERSION  = "VERSION";

class StorageInfo
{
public:
    StorageInfo(int32_t layoutV, string cid, int32_t nsID,  uint64_t cT)
        :
        layoutVersion(layoutV),
        clusterID(cid),
        namespaceID(nsID),
        cTime(cT)
    {}

    StorageInfo()
        :
        layoutVersion(0),
        clusterID(""),
        namespaceID(0),
        cTime(0)
    {}

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
    int32_t getLayoutVersion(){ return layoutVersion; }

    /**
    * Namespace id of the file system.<p>
    * Assigned to the file system at formatting and never changes after that.
    * Shared by all file system components.
    */
    int32_t getNamespaceID() const { return namespaceID; }

    /**
    * cluster id of the file system.<p>
    */
    string    getClusterID() { return clusterID; }

    /**
    * Creation time of the file system state.<p>
    * Modified during upgrades.
    */
     long getCTime() { return cTime; }

     int readProperties(const string& from){
         ifstream is(from.c_str());
         if(!is.is_open()){
             LOG.error("File %s is missing", from.c_str());
             return -1;
         }
         is.close();

         Ice::PropertiesPtr properties = Ice::createProperties();
         properties->load(from);
         string lv = properties->getProperty("layoutVersion");
         string cid = properties->getProperty("clusterID");
         string nsid = properties->getProperty("namespaceID");
         string ct = properties->getProperty("cTime");
         if(lv.empty() || cid.empty() || nsid.empty() || ct.empty()) {
             LOG.error("Bad VERSION file %s",from.c_str());
             return -1;
         }
         layoutVersion =  atoi(lv.c_str());
         clusterID = cid;
         namespaceID = atoi(nsid.c_str());
         cTime = strtoul(ct.c_str(), 0, 10);
         return 0;
     }



private:
    int32_t   layoutVersion;   // layout version of the storage data
    string clusterID;      // id of the cluster
    int32_t   namespaceID;     // id of the file system
    uint64_t  cTime;           // creation time of the file system state
};

}
#endif /* STORAGEINFO_H_ */
