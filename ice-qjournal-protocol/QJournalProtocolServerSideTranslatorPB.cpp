/*
 * QJournalProtocolServerSideTranslatorPB.cpp
 *
 *  Created on: Sep 2, 2014
 *      Author: psarda
 */

#include "QJournalProtocolServerSideTranslatorPB.h"
#include "../common/commons.h"

namespace JournalServiceServer
{

void
QJournalProtocolServerSideTranslatorPB::throwExceptionOnError(int rc) const {
    if (rc != 0) {
        QJournalProtocolProtos::ServiceException e;
        throw e;
    }
}

hadoop::hdfs::IsFormattedResponseProto
QJournalProtocolServerSideTranslatorPB::isFormatted(const hadoop::hdfs::IsFormattedRequestProto& req, const ::Ice::Current& current){
    bool isFormatted = false;

    int ret = impl->isFormatted(req.jid().identifier(),isFormatted);
    throwExceptionOnError(ret);

    hadoop::hdfs::IsFormattedResponseProto resp;
    cout << "response from QJournalProtocol impl is " << isFormatted << endl;
    resp.set_isformatted(isFormatted);
    return resp;
}

hadoop::hdfs::GetJournalStateResponseProto
QJournalProtocolServerSideTranslatorPB::getJournalState(const hadoop::hdfs::GetJournalStateRequestProto& req, const ::Ice::Current& current){
    hadoop::hdfs::GetJournalStateResponseProto resp;
    int ret = impl->getJournalState(req.jid().identifier(), resp);
    throwExceptionOnError(ret);
    return resp;
}

hadoop::hdfs::NewEpochResponseProto
QJournalProtocolServerSideTranslatorPB::newEpoch(const hadoop::hdfs::NewEpochRequestProto& req, const ::Ice::Current& current){
    hadoop::hdfs::NewEpochResponseProto resp;
    NamespaceInfo nsi(req.nsinfo());
    int ret = impl->newEpoch(req.jid().identifier(), nsi, req.epoch(), resp);
    throwExceptionOnError(ret);
    return resp;
}

hadoop::hdfs::FormatResponseProto
QJournalProtocolServerSideTranslatorPB::format(const hadoop::hdfs::FormatRequestProto& req, const ::Ice::Current& current){
    NamespaceInfo nsi(req.nsinfo());
    int ret = impl->format(req.jid().identifier(), nsi);
    throwExceptionOnError(ret);
    return hadoop::hdfs::FormatResponseProto::default_instance();
}

hadoop::hdfs::JournalResponseProto
QJournalProtocolServerSideTranslatorPB::journal(const hadoop::hdfs::JournalRequestProto& req, const ::Ice::Current& current){
    RequestInfo ri(req.reqinfo());
    int ret = impl->journal(ri, req.segmenttxnid(), req.firsttxnid(), req.numtxns(), req.records());
    throwExceptionOnError(ret);
    hadoop::hdfs::JournalResponseProto resp;
    //TODO:changed return VOID_JOURNAL_RESPONSE to hadoop::hdfs::JournalResponseProto::default_instance()
    // Looks like there is nothing to worry because as per below webpage  Foo getDefaultInstance() is similar to Foo.newBuilder().build()
    //https://developers.google.com/protocol-buffers/docs/reference/java-generated
    return hadoop::hdfs::JournalResponseProto::default_instance();
}

hadoop::hdfs::StartLogSegmentResponseProto
QJournalProtocolServerSideTranslatorPB::startLogSegment(const hadoop::hdfs::StartLogSegmentRequestProto& req, const ::Ice::Current& current){
    // TODO: Assumed that request has Layoutversion. This is to avoid looking to namenode code for layoutversion.
    RequestInfo ri(req.reqinfo());
    int ret = impl->startLogSegment(ri, req.txid(), req.layoutversion());
    throwExceptionOnError(ret);
    //TODO: changed from VOID_START_LOG_SEGMENT_RESPONSE to hadoop::hdfs::StartLogSegmentResponseProto::default_instance()
    return hadoop::hdfs::StartLogSegmentResponseProto::default_instance();
}

hadoop::hdfs::FinalizeLogSegmentResponseProto
QJournalProtocolServerSideTranslatorPB::finalizeLogSegment(const hadoop::hdfs::FinalizeLogSegmentRequestProto& req, const ::Ice::Current& current){
    RequestInfo ri(req.reqinfo());
    int ret = impl->finalizeLogSegment(ri, req.starttxid(), req.endtxid());
    throwExceptionOnError(ret);
    return hadoop::hdfs::FinalizeLogSegmentResponseProto::default_instance();
}

hadoop::hdfs::GetEditLogManifestResponseProto
QJournalProtocolServerSideTranslatorPB::getEditLogManifest(const hadoop::hdfs::GetEditLogManifestRequestProto& req, const ::Ice::Current& current){
    hadoop::hdfs::GetEditLogManifestResponseProto resp;
    int ret = impl->getEditLogManifest(req.jid().identifier(), req.sincetxid(), req.inprogressok(), resp);
    throwExceptionOnError(ret);
    return resp;
}

hadoop::hdfs::PrepareRecoveryResponseProto
QJournalProtocolServerSideTranslatorPB::prepareRecovery(const hadoop::hdfs::PrepareRecoveryRequestProto& req, const ::Ice::Current& current) {
    hadoop::hdfs::PrepareRecoveryResponseProto resp;
    RequestInfo ri(req.reqinfo());
    int ret = impl->prepareRecovery(ri, req.segmenttxid(), resp);
    cout << "contents of prepareRecovery response [lastwriterepoch, lastcommittedtxid] : [" << resp.lastwriterepoch() << ", " << resp.lastcommittedtxid() << "]"<< endl;
    throwExceptionOnError(ret);
    return resp;
}

hadoop::hdfs::AcceptRecoveryResponseProto
QJournalProtocolServerSideTranslatorPB::acceptRecovery(const hadoop::hdfs::AcceptRecoveryRequestProto& req, const ::Ice::Current& current){
    RequestInfo ri(req.reqinfo());
    int ret = impl->acceptRecovery(ri, req.statetoaccept(), req.fromurl());
    throwExceptionOnError(ret);
    return hadoop::hdfs::AcceptRecoveryResponseProto::default_instance();
}

} /* namespace JournalServiceServer */
