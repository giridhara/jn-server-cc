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
package org.apache.hadoop.ice.qjournal.ProtocolPB;

import java.io.IOException;
import java.net.URL;

import org.apache.hadoop.hdfs.protocol.HdfsConstants;
import org.apache.hadoop.hdfs.protocolPB.JournalProtocolPB;
import org.apache.hadoop.hdfs.protocolPB.PBHelper;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocol;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.AcceptRecoveryRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.AcceptRecoveryResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.CanRollBackRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.CanRollBackResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DiscardSegmentsRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DiscardSegmentsResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DoFinalizeRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DoFinalizeResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DoPreUpgradeRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DoPreUpgradeResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DoRollbackRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DoRollbackResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DoUpgradeRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.DoUpgradeResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.FinalizeLogSegmentRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.FinalizeLogSegmentResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.FormatRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.FormatResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.GetEditLogManifestRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.GetEditLogManifestResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.GetJournalCTimeRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.GetJournalCTimeResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.GetJournalStateRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.GetJournalStateResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.HeartbeatRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.HeartbeatResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.IsFormattedRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.IsFormattedResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.JournalIdProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.JournalRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.JournalResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.NewEpochRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.NewEpochResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.PrepareRecoveryRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.PrepareRecoveryResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.PurgeLogsRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.PurgeLogsResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.RequestInfoProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.StartLogSegmentRequestProto;
import org.apache.hadoop.hdfs.qjournal.protocol.QJournalProtocolProtos.StartLogSegmentResponseProto;
import org.apache.hadoop.hdfs.qjournal.protocol.RequestInfo;
import org.apache.hadoop.hdfs.server.common.HdfsServerConstants.NodeType;
import org.apache.hadoop.hdfs.server.common.StorageInfo;
import org.apache.hadoop.hdfs.server.namenode.NameNodeLayoutVersion;
import org.apache.hadoop.hdfs.server.protocol.JournalProtocol;

import QJournalProtocolProtos.ServiceException;
import QJournalProtocolProtos._QJournalProtocolPBDisp;

/**
 * Implementation for protobuf service that forwards requests
 * received on {@link JournalProtocolPB} to the
 * {@link JournalProtocol} server implementation.
 */
public class QJournalProtocolServerSideTranslatorPB extends _QJournalProtocolPBDisp
{
  /** Server side implementation to delegate the requests to */
  private final QJournalProtocol impl;

  private final static JournalResponseProto VOID_JOURNAL_RESPONSE =
  JournalResponseProto.newBuilder().build();

  private final static StartLogSegmentResponseProto
  VOID_START_LOG_SEGMENT_RESPONSE =
      StartLogSegmentResponseProto.newBuilder().build();

  public QJournalProtocolServerSideTranslatorPB(Object impl) {
    if (impl instanceof QJournalProtocol)
    {
      this.impl = (QJournalProtocol)impl;
    }
    else
    {
      System.out.println("Obtained implementation is incorrect");
      this.impl = null;
    }
  }

  public IsFormattedResponseProto isFormatted(IsFormattedRequestProto request,
      Ice.Current current) throws ServiceException {
    try
    {
      boolean ret = impl.isFormatted(
          convert(request.getJid()));
      return IsFormattedResponseProto.newBuilder()
          .setIsFormatted(ret)
          .build();
    }
    catch (IOException ioe)
    {
      throw new ServiceException(ioe.getMessage());
    }
  }

  public GetJournalStateResponseProto getJournalState(GetJournalStateRequestProto request,
      Ice.Current current) throws ServiceException{
    try {
      return impl.getJournalState(
          convert(request.getJid()));
    } catch (IOException ioe) {
      throw new ServiceException(ioe.getMessage());
    }
  }
  private String convert(JournalIdProto jid) {
    return jid.getIdentifier();
  }

  public NewEpochResponseProto newEpoch(NewEpochRequestProto request,
      Ice.Current current) throws ServiceException {
    try {
      return impl.newEpoch(
          request.getJid().getIdentifier(),
          PBHelper.convert(request.getNsInfo()),
          request.getEpoch());
    } catch (IOException ioe) {
      throw new ServiceException(ioe.getMessage());
    }
  }

  public FormatResponseProto format(FormatRequestProto request,
      Ice.Current current) throws ServiceException {
    try {
      impl.format(request.getJid().getIdentifier(),
          PBHelper.convert(request.getNsInfo()));
      return FormatResponseProto.getDefaultInstance();
    } catch (IOException ioe) {
      throw new ServiceException(ioe.getMessage());
    }
  }

  public JournalResponseProto journal(JournalRequestProto req,
      Ice.Current current) throws ServiceException {
    try {
      impl.journal(convert(req.getReqInfo()),
          req.getSegmentTxnId(), req.getFirstTxnId(),
          req.getNumTxns(), req.getRecords().toByteArray());
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
    return VOID_JOURNAL_RESPONSE;
  }

  public HeartbeatResponseProto heartbeat(HeartbeatRequestProto req,
      Ice.Current current) throws ServiceException {
    try {
      impl.heartbeat(convert(req.getReqInfo()));
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
    return HeartbeatResponseProto.getDefaultInstance();
  }

  public StartLogSegmentResponseProto startLogSegment(StartLogSegmentRequestProto req,
      Ice.Current current) throws ServiceException {
    try {
      int layoutVersion = req.hasLayoutVersion() ? req.getLayoutVersion()
          : NameNodeLayoutVersion.CURRENT_LAYOUT_VERSION;
      impl.startLogSegment(convert(req.getReqInfo()), req.getTxid(),
          layoutVersion);
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
    return VOID_START_LOG_SEGMENT_RESPONSE;
  }

  public FinalizeLogSegmentResponseProto finalizeLogSegment(FinalizeLogSegmentRequestProto req,
      Ice.Current current)
      throws ServiceException {
    try {
      impl.finalizeLogSegment(convert(req.getReqInfo()),
          req.getStartTxId(), req.getEndTxId());
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
    return FinalizeLogSegmentResponseProto.newBuilder().build();
  }

  public PurgeLogsResponseProto purgeLogs(PurgeLogsRequestProto req,
      Ice.Current current) throws ServiceException {
    try {
      impl.purgeLogsOlderThan(convert(req.getReqInfo()),
          req.getMinTxIdToKeep());
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
    return PurgeLogsResponseProto.getDefaultInstance();
  }

  public GetEditLogManifestResponseProto getEditLogManifest(
      GetEditLogManifestRequestProto request, Ice.Current current)
      throws ServiceException {
    try {
      return impl.getEditLogManifest(
          request.getJid().getIdentifier(),
          request.getSinceTxId(),
          request.getInProgressOk());
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }

  public PrepareRecoveryResponseProto prepareRecovery(PrepareRecoveryRequestProto request,
      Ice.Current current) throws ServiceException {
    try {
      return impl.prepareRecovery(convert(request.getReqInfo()),
          request.getSegmentTxId());
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }

  public AcceptRecoveryResponseProto acceptRecovery(AcceptRecoveryRequestProto request,
      Ice.Current current) throws ServiceException {
    try {
      impl.acceptRecovery(convert(request.getReqInfo()),
          request.getStateToAccept(),
          new URL(request.getFromURL()));
      return AcceptRecoveryResponseProto.getDefaultInstance();
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }

  private RequestInfo convert(
      RequestInfoProto reqInfo) {
    return new RequestInfo(
        reqInfo.getJournalId().getIdentifier(),
        reqInfo.getEpoch(),
        reqInfo.getIpcSerialNumber(),
        reqInfo.hasCommittedTxId() ?
          reqInfo.getCommittedTxId() : HdfsConstants.INVALID_TXID);
  }

  public DiscardSegmentsResponseProto discardSegments(
      DiscardSegmentsRequestProto request, Ice.Current current)
      throws ServiceException {
    try {
      impl.discardSegments(convert(request.getJid()), request.getStartTxId());
      return DiscardSegmentsResponseProto.getDefaultInstance();
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }


  public DoPreUpgradeResponseProto doPreUpgrade(DoPreUpgradeRequestProto request,
      Ice.Current current) throws ServiceException {
    try {
      impl.doPreUpgrade(convert(request.getJid()));
      return DoPreUpgradeResponseProto.getDefaultInstance();
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }

  public DoUpgradeResponseProto doUpgrade(DoUpgradeRequestProto request,
      Ice.Current current) throws ServiceException {
    StorageInfo si = PBHelper.convert(request.getSInfo(), NodeType.JOURNAL_NODE);
    try {
      impl.doUpgrade(convert(request.getJid()), si);
      return DoUpgradeResponseProto.getDefaultInstance();
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }

  public DoFinalizeResponseProto doFinalize(DoFinalizeRequestProto request,
      Ice.Current current) throws ServiceException {
    try {
      impl.doFinalize(convert(request.getJid()));
      return DoFinalizeResponseProto.getDefaultInstance();
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }

  public CanRollBackResponseProto canRollBack(CanRollBackRequestProto request,
      Ice.Current current) throws ServiceException {
    try {
      StorageInfo si = PBHelper.convert(request.getStorage(), NodeType.JOURNAL_NODE);
      Boolean result = impl.canRollBack(convert(request.getJid()), si,
          PBHelper.convert(request.getPrevStorage(), NodeType.JOURNAL_NODE),
          request.getTargetLayoutVersion());
      return CanRollBackResponseProto.newBuilder()
          .setCanRollBack(result)
          .build();
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }

  public DoRollbackResponseProto doRollback(DoRollbackRequestProto request,
      Ice.Current current)
      throws ServiceException {
    try {
      impl.doRollback(convert(request.getJid()));
      return DoRollbackResponseProto.getDefaultInstance();
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }

  public GetJournalCTimeResponseProto getJournalCTime(GetJournalCTimeRequestProto request,
      Ice.Current current) throws ServiceException {
    try {
      Long resultCTime = impl.getJournalCTime(convert(request.getJid()));
      return GetJournalCTimeResponseProto.newBuilder()
          .setResultCTime(resultCTime)
          .build();
    } catch (IOException e) {
      throw new ServiceException(e.getMessage());
    }
  }
}
