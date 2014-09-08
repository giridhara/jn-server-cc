/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef QJOURNALPROTOCOLPB_ICE
#define QJOURNALPROTOCOLPB_ICE

[["cpp:include:QJournalProtocol.pb.h"]]
[["cpp:include:src/StreamProtobuf.h"]]

module QJournalProtocolProtos
{
#ifdef __SLICE2CPP__
["cpp:type:hadoop::hdfs::AcceptRecoveryRequestProto"] sequence<byte> AcceptRecoveryRequestProto;
["cpp:type:hadoop::hdfs::AcceptRecoveryResponseProto"] sequence<byte> AcceptRecoveryResponseProto;
["cpp:type:hadoop::hdfs::CanRollBackRequestProto"] sequence<byte> CanRollBackRequestProto;
["cpp:type:hadoop::hdfs::CanRollBackResponseProto"] sequence<byte> CanRollBackResponseProto;
["cpp:type:hadoop::hdfs::DiscardSegmentsRequestProto"] sequence<byte> DiscardSegmentsRequestProto;
["cpp:type:hadoop::hdfs::DiscardSegmentsResponseProto"] sequence<byte> DiscardSegmentsResponseProto;
["cpp:type:hadoop::hdfs::DoFinalizeRequestProto"] sequence<byte> DoFinalizeRequestProto;
["cpp:type:hadoop::hdfs::DoFinalizeResponseProto"] sequence<byte> DoFinalizeResponseProto;
["cpp:type:hadoop::hdfs::DoPreUpgradeRequestProto"] sequence<byte> DoPreUpgradeRequestProto;
["cpp:type:hadoop::hdfs::DoPreUpgradeResponseProto"] sequence<byte> DoPreUpgradeResponseProto;
["cpp:type:hadoop::hdfs::DoRollbackRequestProto"] sequence<byte> DoRollbackRequestProto;
["cpp:type:hadoop::hdfs::DoRollbackResponseProto"] sequence<byte> DoRollbackResponseProto;
["cpp:type:hadoop::hdfs::DoUpgradeRequestProto"] sequence<byte> DoUpgradeRequestProto;
["cpp:type:hadoop::hdfs::DoUpgradeResponseProto"] sequence<byte> DoUpgradeResponseProto;
["cpp:type:hadoop::hdfs::FinalizeLogSegmentRequestProto"] sequence<byte> FinalizeLogSegmentRequestProto;
["cpp:type:hadoop::hdfs::FinalizeLogSegmentResponseProto"] sequence<byte> FinalizeLogSegmentResponseProto;
["cpp:type:hadoop::hdfs::FormatRequestProto"] sequence<byte> FormatRequestProto;
["cpp:type:hadoop::hdfs::FormatResponseProto"] sequence<byte> FormatResponseProto;
["cpp:type:hadoop::hdfs::GetEditLogManifestRequestProto"] sequence<byte> GetEditLogManifestRequestProto;
["cpp:type:hadoop::hdfs::GetEditLogManifestResponseProto"] sequence<byte> GetEditLogManifestResponseProto;
["cpp:type:hadoop::hdfs::GetJournalCTimeRequestProto"] sequence<byte> GetJournalCTimeRequestProto;
["cpp:type:hadoop::hdfs::GetJournalCTimeResponseProto"] sequence<byte> GetJournalCTimeResponseProto;
["cpp:type:hadoop::hdfs::GetJournalStateRequestProto"] sequence<byte> GetJournalStateRequestProto;
["cpp:type:hadoop::hdfs::GetJournalStateResponseProto"] sequence<byte> GetJournalStateResponseProto;
["cpp:type:hadoop::hdfs::HeartbeatRequestProto"] sequence<byte> HeartbeatRequestProto;
["cpp:type:hadoop::hdfs::HeartbeatResponseProto"] sequence<byte> HeartbeatResponseProto;
["cpp:type:hadoop::hdfs::IsFormattedRequestProto"] sequence<byte> IsFormattedRequestProto;
["cpp:type:hadoop::hdfs::IsFormattedResponseProto"] sequence<byte> IsFormattedResponseProto;
["cpp:type:hadoop::hdfs::JournalRequestProto"] sequence<byte> JournalRequestProto;
["cpp:type:hadoop::hdfs::JournalResponseProto"] sequence<byte> JournalResponseProto;
["cpp:type:hadoop::hdfs::NewEpochRequestProto"] sequence<byte> NewEpochRequestProto;
["cpp:type:hadoop::hdfs::NewEpochResponseProto"] sequence<byte> NewEpochResponseProto;
["cpp:type:hadoop::hdfs::PrepareRecoveryRequestProto"] sequence<byte> PrepareRecoveryRequestProto;
["cpp:type:hadoop::hdfs::PrepareRecoveryResponseProto"] sequence<byte> PrepareRecoveryResponseProto;
["cpp:type:hadoop::hdfs::PurgeLogsRequestProto"] sequence<byte> PurgeLogsRequestProto;
["cpp:type:hadoop::hdfs::PurgeLogsResponseProto"] sequence<byte> PurgeLogsResponseProto;
["cpp:type:hadoop::hdfs::RemoteEditLogManifestProto"] sequence<byte> RemoteEditLogManifestProto;
["cpp:type:hadoop::hdfs::RemoteEditLogProto"] sequence<byte> RemoteEditLogProto;
["cpp:type:hadoop::hdfs::RequestInfoProto"] sequence<byte> RequestInfoProto;
["cpp:type:hadoop::hdfs::SegmentStateProto"] sequence<byte> SegmentStateProto;
["cpp:type:hadoop::hdfs::StartLogSegmentRequestProto"] sequence<byte> StartLogSegmentRequestProto;
["cpp:type:hadoop::hdfs::StartLogSegmentResponseProto"] sequence<byte> StartLogSegmentResponseProto;
#endif

exception ServiceException
{
  string reason;
};
/**
 * Protocol used to journal edits to a JournalNode.
 * This file provides similar interface as QJournalProtocolService in QJournalProtocol.proto for ice to work with.
 */

//service QJournalProtocolService
interface QJournalProtocolPB
{
  //  rpc isFormatted(IsFormattedRequestProto) returns (IsFormattedResponseProto);
  IsFormattedResponseProto isFormatted(IsFormattedRequestProto request) throws ServiceException;

  //  rpc discardSegments(DiscardSegmentsRequestProto) returns (DiscardSegmentsResponseProto);
  DiscardSegmentsResponseProto discardSegments(DiscardSegmentsRequestProto request) throws ServiceException;

  //  rpc getJournalCTime(GetJournalCTimeRequestProto) returns (GetJournalCTimeResponseProto);
  GetJournalCTimeResponseProto getJournalCTime(GetJournalCTimeRequestProto request) throws ServiceException;

  //  rpc doPreUpgrade(DoPreUpgradeRequestProto) returns (DoPreUpgradeResponseProto);
  DoPreUpgradeResponseProto doPreUpgrade(DoPreUpgradeRequestProto request) throws ServiceException;
  //  rpc doUpgrade(DoUpgradeRequestProto) returns (DoUpgradeResponseProto);
  DoUpgradeResponseProto doUpgrade(DoUpgradeRequestProto request) throws ServiceException;

  //  rpc doFinalize(DoFinalizeRequestProto) returns (DoFinalizeResponseProto);
  DoFinalizeResponseProto doFinalize(DoFinalizeRequestProto request) throws ServiceException;

  //  rpc canRollBack(CanRollBackRequestProto) returns (CanRollBackResponseProto);
  CanRollBackResponseProto canRollBack(CanRollBackRequestProto request) throws ServiceException;

  //  rpc doRollback(DoRollbackRequestProto) returns (DoRollbackResponseProto);
  DoRollbackResponseProto doRollback(DoRollbackRequestProto request) throws ServiceException;

  //  rpc getJournalState(GetJournalStateRequestProto) returns (GetJournalStateResponseProto);
   GetJournalStateResponseProto getJournalState(GetJournalStateRequestProto request) throws ServiceException;

  //  rpc newEpoch(NewEpochRequestProto) returns (NewEpochResponseProto);
  NewEpochResponseProto newEpoch(NewEpochRequestProto request) throws ServiceException;

  //  rpc format(FormatRequestProto) returns (FormatResponseProto);
  FormatResponseProto format(FormatRequestProto request) throws ServiceException;

  //  rpc journal(JournalRequestProto) returns (JournalResponseProto);
  JournalResponseProto journal(JournalRequestProto request) throws ServiceException;

  //  rpc heartbeat(HeartbeatRequestProto) returns (HeartbeatResponseProto);
  HeartbeatResponseProto heartbeat(HeartbeatRequestProto request) throws ServiceException;

  //  rpc startLogSegment(StartLogSegmentRequestProto)
  //      returns (StartLogSegmentResponseProto);
  StartLogSegmentResponseProto startLogSegment(StartLogSegmentRequestProto request) throws ServiceException;

  //  rpc finalizeLogSegment(FinalizeLogSegmentRequestProto)
  //      returns (FinalizeLogSegmentResponseProto);
  FinalizeLogSegmentResponseProto finalizeLogSegment(FinalizeLogSegmentRequestProto request) throws ServiceException;

  //  rpc purgeLogs(PurgeLogsRequestProto)
  //      returns (PurgeLogsResponseProto);
  PurgeLogsResponseProto purgeLogs(PurgeLogsRequestProto request) throws ServiceException;
  //  rpc getEditLogManifest(GetEditLogManifestRequestProto)
  //      returns (GetEditLogManifestResponseProto);
  GetEditLogManifestResponseProto getEditLogManifest(GetEditLogManifestRequestProto request) throws ServiceException;

  //  rpc prepareRecovery(PrepareRecoveryRequestProto)
  //      returns (PrepareRecoveryResponseProto);
  PrepareRecoveryResponseProto prepareRecovery(PrepareRecoveryRequestProto req) throws ServiceException;
  //  rpc acceptRecovery(AcceptRecoveryRequestProto)
  //      returns (AcceptRecoveryResponseProto);
  AcceptRecoveryResponseProto acceptRecovery(AcceptRecoveryRequestProto request) throws ServiceException;
};

};
#endif
