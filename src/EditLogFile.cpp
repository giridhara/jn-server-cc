/*
 * EditLogFile.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: psarda
 */

#include "EditLogFile.h"
#include "../util/Logger.h"

namespace JournalServiceServer
{

const bool
EditLogFile::operator<(const EditLogFile &other) const {
    if(firstTxId < other.firstTxId)
        return true;
    if(firstTxId > other.firstTxId)
        return false;

    if(lastTxId < other.lastTxId)
        return true;
    if(lastTxId > other.lastTxId)
        return false;

    return true;
}

const bool
EditLogFile::operator>(const EditLogFile &other) const {
    if(firstTxId > other.firstTxId)
        return true;
    if(firstTxId < other.firstTxId)
        return false;

    if(lastTxId > other.lastTxId)
        return true;
    if(lastTxId < other.lastTxId)
        return false;

    return true;
}

const bool
EditLogFile::operator==(const EditLogFile &other) const {
    return (firstTxId == other.firstTxId) && (lastTxId == other.lastTxId);
}

int
EditLogFile::renameSelf(string newSuffix) {
     string to = (file+newSuffix);
     int rc = rename(file.c_str(), to.c_str());
     if(rc != 0){
         LOG.error("Couldn't rename log %s to %s", file.c_str(), to.c_str());
         return -1;
     }

     file = to;
     return 0;
}

} /* namespace JournalServiceServer */
