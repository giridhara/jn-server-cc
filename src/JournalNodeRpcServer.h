/*
 * JournalNodeRpcServer.h
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#ifndef JOURNALNODERPCSERVER_H_
#define JOURNALNODERPCSERVER_H_

#include "QjournalProtocol.h"

namespace JournalServiceServer
{

class JournalNodeRpcServer : public QJournalProtocol
{
public:
    JournalNodeRpcServer() {}
    ~JournalNodeRpcServer() {}
    CallStatus isFormatted(const string& journalId, bool& result);
    CallStatus getJournalState(const string& journalId, hadoop::hdfs::GetJournalStateResponseProto&);
};

} /* namespace JournalServiceServer */

#endif /* JOURNALNODERPCSERVER_H_ */
