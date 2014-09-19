/*
 * QJournalProtocolServerSideTranslatorPB.h
 *
 *  Created on: Sep 2, 2014
 *      Author: psarda
 */
#ifndef QJOURNALPROTOCOLSERVERSIDETRANSLATORPB_H_
#define QJOURNALPROTOCOLSERVERSIDETRANSLATORPB_H_

#include <Ice/Ice.h>
#include "QJournalProtocolPB.h"
#include <string>
//#include "../src/QjournalProtocol.h"
#include "/home/psarda/workspace/jn-server-cc/src/QjournalProtocol.h"

using std::string;

namespace JournalServiceServer
{

class QJournalProtocolServerSideTranslatorPB : public QJournalProtocolProtos::QJournalProtocolPB{
public:
    QJournalProtocolServerSideTranslatorPB(QJournalProtocol* impl)
        :impl(impl)
    {}

    virtual ~QJournalProtocolServerSideTranslatorPB() {}
    virtual hadoop::hdfs::IsFormattedResponseProto isFormatted(const hadoop::hdfs::IsFormattedRequestProto&, const ::Ice::Current& current);
//    virtual DiscardSegmentsResponseProto discardSegments(const DiscardSegmentsRequestProto&, const ::Ice::Current& current);

//    virtual GetJournalCTimeResponseProto getJournalCTime(const GetJournalCTimeRequestProto&, const ::Ice::Current& current);

//    virtual DoPreUpgradeResponseProto doPreUpgrade(const DoPreUpgradeRequestProto&, const ::Ice::Current& current);

//    virtual DoUpgradeResponseProto doUpgrade(const DoUpgradeRequestProto&, const ::Ice::Current& current);

//    virtual DoFinalizeResponseProto doFinalize(const DoFinalizeRequestProto&, const ::Ice::Current& current);

//    virtual CanRollBackResponseProto canRollBack(const CanRollBackRequestProto&, const ::Ice::Current& current);

//   virtual DoRollbackResponseProto doRollback(const DoRollbackRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::GetJournalStateResponseProto getJournalState(const hadoop::hdfs::GetJournalStateRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::NewEpochResponseProto newEpoch(const hadoop::hdfs::NewEpochRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::FormatResponseProto format(const hadoop::hdfs::FormatRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::JournalResponseProto journal(const hadoop::hdfs::JournalRequestProto&, const ::Ice::Current& current);

//    virtual HeartbeatResponseProto heartbeat(const HeartbeatRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::StartLogSegmentResponseProto startLogSegment(const hadoop::hdfs::StartLogSegmentRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::FinalizeLogSegmentResponseProto finalizeLogSegment(const hadoop::hdfs::FinalizeLogSegmentRequestProto&, const ::Ice::Current& current);

//    virtual PurgeLogsResponseProto purgeLogs(const PurgeLogsRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::GetEditLogManifestResponseProto getEditLogManifest(const hadoop::hdfs::GetEditLogManifestRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::PrepareRecoveryResponseProto prepareRecovery(const hadoop::hdfs::PrepareRecoveryRequestProto&, const ::Ice::Current& current);

    virtual hadoop::hdfs::AcceptRecoveryResponseProto acceptRecovery(const hadoop::hdfs::AcceptRecoveryRequestProto&, const ::Ice::Current& current);

private:
    void throwExceptionOnError(int rc) const;
    QJournalProtocol* impl;
};


} /* namespace JournalServiceServer */

#endif /* QJOURNALPROTOCOLSERVERSIDETRANSLATORPB_H_ */
