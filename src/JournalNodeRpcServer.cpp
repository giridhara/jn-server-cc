/*
 * JournalNodeRpcServer.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "JournalNodeRpcServer.h"

namespace JournalServiceServer
{

CallStatus
JournalNodeRpcServer::isFormatted(const string& journalId, bool& result){
    cout << "jid received from client is " << journalId << endl;
    result = false;
    CallStatus cs(0, "");
    return cs;
}

CallStatus
JournalNodeRpcServer::getJournalState(const string& journalId, hadoop::hdfs::GetJournalStateResponseProto& resp){
    cout << "inside getJournalState :: jid received from client is " << journalId << endl;
    resp.set_lastpromisedepoch(1);
    resp.set_httpport(123456);
    resp.set_fromurl("www.walmart.com");
    CallStatus cs(0, "");
    return cs;
}

} /* namespace JournalServiceServer */

