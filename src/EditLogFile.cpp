/*
 * EditLogFile.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: psarda
 */

#include "EditLogFile.h"

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

} /* namespace JournalServiceServer */
