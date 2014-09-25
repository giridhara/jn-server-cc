/*
 * Journal.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#include "Journal.h"
#include "../util/JNServiceMiscUtils.h"
#include <stdlib.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <fstream>

namespace JournalServiceServer
{

Journal::~Journal()
{
    committedTxnId.reset();
    curSegment.reset();
}

/**
   * Reload any data that may have been cached. This is necessary
   * when we first load the Journal, but also after any formatting
   * operation, since the cached data is no longer relevant.
   */
void
Journal::refreshCachedData() {
    //TODO : this function is not complete
    //IOUtils.closeStream(committedTxnId);
    const string currentDir(storage.getCurrentDir());
    lastPromisedEpoch.reset(new PersistentLongFile(currentDir+ "/" +  LAST_PROMISED_FILENAME, 0));
    lastWriterEpoch.reset(new PersistentLongFile(currentDir + "/" + LAST_WRITER_EPOCH, 0));
    committedTxnId.reset(new BestEffortLongFile(currentDir + "/" + COMMITTED_TXID_FILENAME, INVALID_TXID));
  }

/**
* Scan the local storage directory, and return the segment containing
* the highest transaction.
* @return the EditLogFile with the highest transactions, or null
* if no files exist.
*/
//call isInitialized function on 'ret' before using it
int
Journal::scanStorageForLatestEdits(EditLogFile& ret) {
    bool hasCurrentDir = false;
    if(dir_exists(storage.getCurrentDir(), hasCurrentDir) !=0 ){
        return -1;
    }
    if (!hasCurrentDir) {
      return 0;
    }

    LOG.info("Scanning storage ");
    vector<EditLogFile> files;
    if(fjm.getLogFiles(0, files) != 0 ){
        return -1;
    }

    for (unsigned i = files.size(); i-- > 0; ){
        EditLogFile& latestLog = files[i];
        int rc = latestLog.scanLog();
        if(rc != 0)
            return -1;
        if(latestLog.getLastTxId() == INVALID_TXID) {
            LOG.warn("Latest log %s has no transactions. moving it aside and looking for previous log",
                    latestLog.getFile().c_str());
            latestLog.moveAsideEmptyFile();
        }else {
            ret = latestLog;
            return 0;
        }
    }

    LOG.info("No files in %s", storage.getCurrentDir().c_str());
    return 0;
}

int
Journal::format(const NamespaceInfo& nsInfo) {
    if(nsInfo.getNamespaceID() == 0) {
        LOG.error("can't format with uninitialized namespace info");
        abort();
    }
    LOG.info("Formatting journal with namespace info with nsid %d", nsInfo.getNamespaceID());
    int rc = storage.format(nsInfo);
    if(rc != 0)
        return -1;
    refreshCachedData();
    return 0;
}

int
Journal::newEpoch(NamespaceInfo& nsInfo, long epoch, hadoop::hdfs::NewEpochResponseProto& ret) {
    if(checkFormatted() != 0) {
        return -1;
    }

    if(storage.checkConsistentNamespace(nsInfo) != 0 ){
        return -1;
    }

    long lpe;

    if(getLastPromisedEpoch(lpe) != 0) {
        return -1;
    }

    // Check that the new epoch being proposed is in fact newer than
    // any other that we've promised.
    if (epoch <= lpe) {
        LOG.error("Proposed epoch %d <= last promise %d", epoch, lpe);
        return -1;
    }

    if(updateLastPromisedEpoch(lpe, epoch) != 0 ) {
        return -1;
    }

    if (abortCurSegment() != 0 ) {
        return -1;
    }

    EditLogFile latestFile;
    if(scanStorageForLatestEdits(latestFile) != 0 ) {
        return -1;
    }

    if(latestFile.isInitialized()) {
        ret.set_lastsegmenttxid(latestFile.getFirstTxId());
    }

    return 0;
}

/**
   * Ensure that the given request is coming from the correct writer and in-order.
   * @param reqInfo the request info
   * @throws IOException if the request is invalid.
*/
int
Journal::checkRequest(const RequestInfo& reqInfo) {
    // Invariant 25 from ZAB paper
    long lpe;
    if(lastPromisedEpoch->get(lpe) != 0)
        return -1;
    if (reqInfo.getEpoch() < lpe) {
      LOG.error("IPC's epoch %d is less than the last promised epoch %d ",
              reqInfo.getEpoch(), lpe);
      return -1;
    } else if (reqInfo.getEpoch() > lpe) {
      // A newer client has arrived. Fence any previous writers by updating
      // the promise.
      if(updateLastPromisedEpoch(lpe, reqInfo.getEpoch()) != 0){
          return -1;
      }
    }

    // Ensure that the IPCs are arriving in-order as expected.
    if(reqInfo.getIpcSerialNumber() <= currentEpochIpcSerial) {
        LOG.error("IPC serial %s from client was not higher than prior highest IPC serial %s",
                reqInfo.getIpcSerialNumber(), currentEpochIpcSerial);
        return -1;
    }
    currentEpochIpcSerial = reqInfo.getIpcSerialNumber();

    if (reqInfo.hasCommittedTxId()) {
        long cti;
        if(committedTxnId->get(cti) != 0) {
            return -1;
        }
      if (reqInfo.getCommittedTxId() < cti) {
          LOG.error("Client trying to move committed txid backward from %d to ",
                  cti, reqInfo.getCommittedTxId());
          abort();
      }

      return committedTxnId->set(reqInfo.getCommittedTxId());
    }

    return 0;
}

int
Journal::checkWriteRequest(const RequestInfo& reqInfo) {
    if(checkRequest(reqInfo)!=0) {
        return -1;
    }

    long lwe;
    if(lastWriterEpoch->get(lwe) !=0 ){
        return -1;
    }

    if (reqInfo.getEpoch() != lwe) {
      LOG.error("IPC's epoch %d is not the current writer epoch %d",
              reqInfo.getEpoch(), lwe);
      return -1;
    }
    return 0;
}

/**
   * Finalize the log segment at the given transaction ID.
   */
int
Journal::finalizeLogSegment(const RequestInfo& reqInfo, const long startTxId,
      const long endTxId) {
    if(checkFormatted() != 0 ) {
        return -1;
    }

    if(checkRequest(reqInfo) != 0){
        return -1;
    }

    bool needsValidation = true;

    // Finalizing the log that the writer was just writing.
    if (startTxId == curSegmentTxId) {
      if (curSegment) {
        curSegment->close();
        curSegment.reset();
        curSegmentTxId = INVALID_TXID;
      }

    if (nextTxId != endTxId + 1) {
        LOG.error("Trying to finalize in-progress log segment %s "
                  "to end at txid %s but only written up to txid %s",
          startTxId, endTxId, nextTxId - 1);
        return -1;
    }
      // No need to validate the edit log if the client is finalizing
      // the log segment that it was just writing to.
      needsValidation = false;
    }

    EditLogFile elf;
    if (fjm.getLogFile(startTxId, elf) != 0) {
        return -1;
    }
    if (!elf.isInitialized()) {
      LOG.error("No log file to finalize at transaction ID %d", startTxId);
      return -1;
    }

    if (elf.isInProgress()) {
        if (needsValidation) {
            LOG.info("Validating log segment %s about to be finalized",  elf.getFile().c_str());
            if(elf.scanLog() != 0) {
                return -1;
            }
            if(elf.getLastTxId() != endTxId) {
                LOG.error("Trying to finalize in-progress log segment %s to end at txid %s but log %s on disk only contains up to txid %s",
                        startTxId, endTxId, elf.getFile().c_str(), elf.getLastTxId());
                return -1;
            }
        }
        if (fjm.finalizeLogSegment(startTxId, endTxId) != 0 ) {
          return -1;
        }
    }else {
        if(endTxId != elf.getLastTxId()) {
            LOG.error("Trying to re-finalize already finalized log [%d, %d] with different endTxId %d",
                    elf.getFirstTxId(), elf.getLastTxId(), endTxId);
            abort();
        }
    }

    // Once logs are finalized, a different length will never be decided.
    // During recovery, we treat a finalized segment the same as an accepted
    // recovery. Thus, we no longer need to keep track of the previously-
    // accepted decision. The existence of the finalized log segment is enough.
    if (purgePaxosDecision(elf.getFirstTxId()) != 0 ){
        return -1;
    }
    return 0;
}

/**
   * @return the current state of the given segment, or null if the
   * segment does not exist.
   */
int
Journal::getSegmentInfo(long segmentTxId, hadoop::hdfs::SegmentStateProto& ssp, bool& isInitialized) {
    EditLogFile elf;
    if( fjm.getLogFile(segmentTxId, elf) != 0 ) {
        return -1;
    }
    if (!elf.isInitialized()) {
      isInitialized = false;
      return 0;
    }
    if (elf.isInProgress()) {
      if(elf.scanLog() != 0) {
          return -1;
      }
    }
    if (elf.getLastTxId() == INVALID_TXID) {
      LOG.info("Edit log file %s appears to be empty. Moving it aside...", elf.getFile().c_str());
      elf.moveAsideEmptyFile();
      isInitialized = false;
      return 0;
    }
    ssp.set_starttxid(segmentTxId);
    ssp.set_endtxid(elf.getLastTxId());
    ssp.set_isinprogress(elf.isInProgress());
    isInitialized = true;

    //TODO : Looks like i need to provide implementation for '<<' operator for EditLogFile,
    //so that logging as done below is possible
//    LOG.info("getSegmentInfo(" + segmentTxId + "): " + elf + " -> " +
//        TextFormat.shortDebugString(ret));
    return 0;
}

/**
   * Start a new segment at the given txid. The previous segment
   * must have already been finalized.
   */
int
Journal::startLogSegment(const RequestInfo& reqInfo, const long txid,
      const int layoutVersion) {
    //TODO : Have to implement below assert.
    //assert fjm != null;

    if (checkFormatted() != 0) {
        return -1;
    }

    if (checkRequest(reqInfo) != 0) {
        return -1;
    }

    if (curSegment) {
        ostringstream warnMsg;
        // TODO : Below msg is not entirely proper.
        warnMsg << "Client is requesting a new log segment " << txid
                << "though we are already writing " << ". "
                << "Aborting the current segment in order to begin the new one.";
        LOG.warn(warnMsg.str().c_str());
        // The writer may have lost a connection to us and is now
        // re-connecting after the connection came back.
        // We should abort our own old segment.
        if(abortCurSegment() != 0 ){
            return -1;
        }
    }

    // Paranoid sanity check: we should never overwrite a finalized log file.
    // Additionally, if it's in-progress, it should not contain any transactions
    // This can happen if the writer crashes exactly at the start of a segment.
    EditLogFile existing;
    if (fjm.getLogFile(txid, existing) != 0 ) {
        return -1;
    }
    if (existing.isInitialized()) {
        if (!existing.isInProgress()) {
            LOG.error("Already have a finalized segment %s beginning at ",
              existing.getFile().c_str(), txid);
            abort();
        }

        // If it's in-progress, it should only contain one transaction,
        // because the "startLogSegment" transaction is written alone at the
        // start of each segment.
        if (existing.scanLog() != 0 ){
            return -1;
        }
        if (existing.getLastTxId() != INVALID_TXID) {
            LOG.error("The log file %s seems to contain valid transactions" , existing.getFile().c_str());
            abort();
        }
    }

    long curLastWriterEpoch;
    if (lastWriterEpoch->get(curLastWriterEpoch) != 0 ) {
        return -1;
    }
    if (curLastWriterEpoch != reqInfo.getEpoch()) {
      LOG.info("Updating lastWriterEpoch from %d to %d", curLastWriterEpoch, reqInfo.getEpoch());
      if(lastWriterEpoch->set(reqInfo.getEpoch()) !=0 ) {
          return -1;
      }
    }

    // The fact that we are starting a segment at this txid indicates
    // that any previous recovery for this same segment was aborted.
    // Otherwise, no writer would have started writing. So, we can
    // remove the record of the older segment here.
    if (purgePaxosDecision(txid) != 0 ) {
        return -1;
    }

    if (fjm.startLogSegment(txid, layoutVersion, curSegment) != 0){
        return -1;
    }

    curSegmentTxId = txid;
    nextTxId = txid;

    return 0;
}

int
Journal::journal(const RequestInfo& reqInfo,
      long segmentTxId, long firstTxnId,
      int numTxns, const string& records) {

    LOG.debug("Received request from client to persist following record '%s'", records.c_str());

    if(checkFormatted() != 0 ) {
        return -1;
    }

    if(checkWriteRequest(reqInfo) != 0) {
        return -1;
    }

    if(!curSegment){
        LOG.error("Can't write, no segment open");
    }

    if (curSegmentTxId != segmentTxId) {
      // Sanity check: it is possible that the writer will fail IPCs
      // on both the finalize() and then the start() of the next segment.
      // This could cause us to continue writing to an old segment
      // instead of rolling to a new one, which breaks one of the
      // invariants in the design. If it happens, abort the segment
      // and throw an exception.
      LOG.error("Writer out of sync: it thinks it is writing segment %d but current segment is %d",
              segmentTxId, curSegmentTxId);
      if(abortCurSegment() != 0) {
          return -1;
      }
      return -1;
    }

    if(nextTxId != firstTxnId) {
        LOG.error("Can't write txid %d expecting nextTxId=", firstTxnId, nextTxId);
        return -1;
    }

    long lastTxnId = firstTxnId + numTxns - 1;
    LOG.debug("Writing txid %d-%d", firstTxnId, lastTxnId);

    // If the edit has already been marked as committed, we know
    // it has been fsynced on a quorum of other nodes, and we are
    // "catching up" with the rest. Hence we do not need to fsync.
    long ctid;
    if(committedTxnId->get(ctid) != 0) {
        return -1;
    }
    /* TODO :: every journanode irrespective of whether it is lagging or not
     * will flush the record to the disk. Might want to revisit this decision in future.
    bool isLagging = lastTxnId <= ctid;
    bool shouldFsync = !isLagging;
    */
    curSegment->writeRaw(records);
    if(!curSegment->flush()) {
        LOG.error("Not able to write record '%s'to disk ", records.c_str());
        abort();
    }

    highestWrittenTxId = lastTxnId;
    nextTxId = lastTxnId + 1;

    return 0;
}

/**
   * Remove the previously-recorded 'accepted recovery' information
   * for a given log segment, once it is no longer necessary.
   * @param segmentTxId the transaction ID to purge
   * @throws IOException if the file could not be deleted
   */
int
Journal::purgePaxosDecision(long segmentTxId) {
    string paxosFile = storage.getPaxosFile(segmentTxId);
    bool hasPaxosFile = false;
    if(file_exists(paxosFile, hasPaxosFile) !=0 ){
        return -1;
    }
    if (hasPaxosFile) {
      if (file_delete(paxosFile) != 0) {
        LOG.error("Unable to delete paxos file %s", paxosFile.c_str());
        return -1;
      }
    }
    return 0;
}

/**
 * @see QJournalProtocol#prepareRecovery(RequestInfo, long)
 */
int
Journal::prepareRecovery(
    const RequestInfo& reqInfo, const long segmentTxId, hadoop::hdfs::PrepareRecoveryResponseProto& ret){
    if(checkFormatted() != 0 ) {
        return -1;
    }
    if(checkRequest(reqInfo)!=0){
        return -1;
    }

    if(abortCurSegment() != 0){
        return -1;
    }

    hadoop::hdfs::PersistedRecoveryPaxosData previouslyAccepted;
    bool isPRPDInitialized = false;
    if( getPersistedPaxosData(segmentTxId, previouslyAccepted, isPRPDInitialized) != 0 ) {
        return -1;
    }

    if(completeHalfDoneAcceptRecovery(previouslyAccepted, isPRPDInitialized) != 0 ) {
        return -1;
    }

    hadoop::hdfs::SegmentStateProto segInfo;
    bool isSSPInitialized = false;
    if(getSegmentInfo(segmentTxId, segInfo, isSSPInitialized) != 0 ) {
        return -1;
    }

    bool hasFinalizedSegment = isSSPInitialized && !(segInfo.isinprogress());

    if (isPRPDInitialized && !hasFinalizedSegment) {
        hadoop::hdfs::SegmentStateProto acceptedState = previouslyAccepted.segmentstate();
        //TODO: converted java assert in to statement below, might have to revisit this decision.
        if(acceptedState.endtxid() != segInfo.endtxid()) {
            LOG.error("prev accepted: [%d, %d] \n on disk: [%d, %d]",
                    acceptedState.starttxid(), acceptedState.endtxid(),
                    segInfo.starttxid(),segInfo.endtxid());
            abort();
        }
        ret.set_acceptedinepoch(previouslyAccepted.acceptedinepoch());
        ret.set_allocated_segmentstate(new hadoop::hdfs::SegmentStateProto(acceptedState));
    }else {
        if (isSSPInitialized) {
            ret.set_allocated_segmentstate(new hadoop::hdfs::SegmentStateProto(segInfo));
        }
    }
    long lwe;
    if(lastWriterEpoch->get(lwe) != 0){
        return -1;
    }
    ret.set_lastwriterepoch(lwe);

    long cti;
    if(committedTxnId->get(cti) != 0){
        return -1;
    }
    if (cti != INVALID_TXID) {
        ret.set_lastcommittedtxid(cti);
    }

    LOG.info("Prepared recovery for segment %d", segmentTxId);
    return 0;
}

/**
  * Retrieve the persisted data for recovering the given segment from disk.
  */
int
Journal::getPersistedPaxosData(long segmentTxId, hadoop::hdfs::PersistedRecoveryPaxosData& ret, bool& isInitialized) {
    string paxosFileName = storage.getPaxosFile(segmentTxId);
    bool paxos_file_exists_flag = false;

    if (file_exists(paxosFileName, paxos_file_exists_flag) != 0) {
        return -1;
    }
    if (!paxos_file_exists_flag) {
        // Default instance has no fields filled in (they're optional)
        isInitialized = false;
        return 0;
    }

    ifstream in(paxosFileName.c_str(), ios::in | ios::binary);
    if(!in.is_open()) {
       return -1;
    }
    hadoop::hdfs::PersistedRecoveryPaxosData tempPaxosData;
    bool parseSuccess = tempPaxosData.ParseFromIstream(&in);
    in.close();
    if ( !parseSuccess) {
       LOG.error("parsing persisted paxos data for segment %d is unsuccessful", segmentTxId);
       abort();
    }

    if (tempPaxosData.segmentstate().starttxid() != segmentTxId) {
       LOG.error("Bad persisted data for segment %d : %d ", segmentTxId, tempPaxosData.segmentstate().starttxid());
       abort();
    }
    ret = tempPaxosData;
    isInitialized = true;
    return 0;
}

/**
   * In the case the node crashes in between downloading a log segment
   * and persisting the associated paxos recovery data, the log segment
   * will be left in its temporary location on disk. Given the paxos data,
   * we can check if this was indeed the case, and &quot;roll forward&quot;
   * the atomic operation.
   *
   * See the inline comments in
   * {@link #acceptRecovery(RequestInfo, SegmentStateProto, URL)} for more
   * details.
   *
   * @throws IOException if the temporary file is unable to be renamed into
   * place
   */
int
Journal::completeHalfDoneAcceptRecovery(
      hadoop::hdfs::PersistedRecoveryPaxosData& paxosData, bool isInitialized) {
    if (!isInitialized) {
        return 0;
    }

    long segmentId = paxosData.segmentstate().starttxid();
    long epoch = paxosData.acceptedinepoch();

    string tmp = storage.getSyncLogTemporaryFile(segmentId, epoch);

    bool tmp_file_exists_flag = false;

    if(file_exists(tmp, tmp_file_exists_flag) != 0){
        return -1;
    }

    if (tmp_file_exists_flag) {
        string dst = storage.getInProgressEditLog(segmentId);
        LOG.info("Rolling forward previously half-completed synchronization: %s -> %s", tmp.c_str(), dst.c_str());
        return file_rename(tmp, dst);
    }

    return 0;
}

/**
   * Persist data for recovering the given segment from disk.
   */
int
Journal::persistPaxosData(long segmentTxId,
      hadoop::hdfs::PersistedRecoveryPaxosData& newData) {
    string f = storage.getPaxosFile(segmentTxId);
    //TODO: trying to write atomically here by writing to .tmp file first; upon successful write renaming it to file without .tmp extension
    string ftemp(f+".tmp");
    ofstream fos(ftemp.c_str(), ios::out | ios::trunc | ios::binary);
    if(!fos.is_open()) {
        return -1;
    }
    bool success = newData.SerializeToOstream(&fos);
    if(!success) {
        LOG.error("SerializeToOstream method failed");
        fos.close();
        file_delete(ftemp);
        return -1;
    }
    fos.flush();
    fos.close();
    return file_rename(ftemp, f);
}

/**
   * Check that the logs are non-overlapping sequences of transactions,
   * in sorted order. They do not need to be contiguous.
   * @throws IllegalStateException if incorrect
   */
int
Journal::checkIfLogsInSequence(const vector<EditLogFile>& vec) {
    const EditLogFile* prev = 0;
    for (vector<EditLogFile>::const_iterator iter = vec.begin() ; iter != vec.end(); ++iter) {
        if (prev != 0) {
            if((*iter).getFirstTxId() <= prev->getLastTxId()) {
                LOG.error("Invalid log manifest (log [%d, %d] overlaps [%d, %d]",
                        (*iter).getFirstTxId(), (*iter).getLastTxId(),
                        prev->getFirstTxId(), prev->getLastTxId());
                return -1;
            }
        }
        prev = &(*iter);
    }
    return 0;
}

int
Journal::getEditLogManifest(const long sinceTxId, const bool inProgressOk, vector<EditLogFile>& ret) {
    // No need to checkRequest() here - anyone may ask for the list
    // of segments.
    if(checkFormatted() != 0 ) {
        return -1;
    }

    vector<EditLogFile> temp;

    if(fjm.getRemoteEditLogs(sinceTxId, inProgressOk, temp) != 0 ) {
        return -1;
    }

    if (inProgressOk) {
      EditLogFile* log = 0;
      vector<EditLogFile>::iterator iter = temp.begin();
      while (iter != temp.end())
      {
          log = &(*iter);
          if(log->isInProgress()){
              // erase returns the new iterator
              iter = temp.erase(iter);
              break;
          }else{
              ++iter;
          }
      }
      if (log != 0 && log->isInProgress()) {
          EditLogFile elf(" ", log->getFirstTxId(), getHighestWrittenTxId(), true);
        temp.push_back(elf);
      }
    }

    if(checkIfLogsInSequence(temp) != 0) {
        return -1;
    }

    ret = temp;
    return 0;
}

int
Journal::acceptRecovery(const RequestInfo& reqInfo,
      const hadoop::hdfs::SegmentStateProto& segment, const string& fromUrl) {
    if(checkFormatted() != 0 ) {
        return -1;
    }
    if(checkRequest(reqInfo)!=0){
        return -1;
    }

    if (abortCurSegment() != 0 ) {
        return -1;
    }

    long segmentTxId = segment.starttxid();

    // Basic sanity checks that the segment is well-formed and contains
    // at least one transaction.
    if (segment.endtxid()<=0 || segment.endtxid() < segmentTxId) {
        LOG.error("bad recovery state for segment %d: [%d, %d]", segmentTxId, segment.starttxid(), segment.endtxid());
        abort();
    }
    bool isOldDataInitialized = false;
    hadoop::hdfs::PersistedRecoveryPaxosData oldData;
    getPersistedPaxosData(segmentTxId, oldData, isOldDataInitialized);
    hadoop::hdfs::PersistedRecoveryPaxosData newData;
    newData.set_acceptedinepoch(reqInfo.getEpoch());
    hadoop::hdfs::SegmentStateProto* segmentCopy = new hadoop::hdfs::SegmentStateProto(segment);
    newData.set_allocated_segmentstate(segmentCopy);

    // If we previously acted on acceptRecovery() from a higher-numbered writer,
    // this call is out of sync. We should never actually trigger this, since the
    // checkRequest() call above should filter non-increasing epoch numbers.
    if (isOldDataInitialized) {
        if(oldData.acceptedinepoch() > reqInfo.getEpoch()) {
            LOG.error("Bad paxos transition, out-of-order epochs.");
            abort();
        }
    }

    string syncedFile;

    bool isCurrentSegmentInitialized = false;
    hadoop::hdfs::SegmentStateProto currentSegment;
    getSegmentInfo(segmentTxId, currentSegment, isCurrentSegmentInitialized);
    if (!isCurrentSegmentInitialized ||
        currentSegment.endtxid() != segment.endtxid()) {
      if (!isCurrentSegmentInitialized) {
        LOG.info("Synchronizing log [%d, %d]  : no current segment in place", segment.starttxid(), segment.endtxid());

        // Update the highest txid for lag metrics
        if(segment.endtxid() > highestWrittenTxId) {
            highestWrittenTxId = segment.endtxid();
        }
      } else {
        LOG.info("Synchronizing log [%d, %d] : old segment [%d, %d] is not the right length",
                segment.starttxid(), segment.endtxid(),
                currentSegment.starttxid(), currentSegment.endtxid());

        // Paranoid sanity check: if the new log is shorter than the log we
        // currently have, we should not end up discarding any transactions
        // which are already Committed.
        long cti;
        if (committedTxnId->get(cti) != 0){
            return -1;
        }

        //TODO: Didn't implement  txnRange function for now, hence had to do below two checks.
        //Might have to revisit this decision in case it is used many times
        if(!currentSegment.has_endtxid()) {
            LOG.error("invalid segment: [%d, %d]", currentSegment.starttxid(), currentSegment.endtxid());
            abort();
        }

        if(!segment.has_endtxid()) {
            LOG.error("invalid segment: [%d, %d]", segment.starttxid(), segment.endtxid());
            abort();
        }

        if( (cti >= currentSegment.starttxid() && cti <= currentSegment.endtxid()) &&
            (cti < segment.starttxid() || cti > segment.endtxid())) {
            LOG.error("Cannot replace segment [%d, %d] with new segment [%d,%d] : would discard already-committed txn %d",
                    currentSegment.starttxid(), currentSegment.endtxid(),
                    segment.starttxid(), segment.endtxid(),
                    cti);
            abort();
        }

        // Another paranoid check: we should not be asked to synchronize a log
        // on top of a finalized segment.
        if(!currentSegment.isinprogress()){
            LOG.error("Should never be asked to synchronize a different log on top of an already-finalized segment");
            abort();
        }

        // If we're shortening the log, update our highest txid
        // used for lag metrics.
        if (highestWrittenTxId >= currentSegment.starttxid() &&
                highestWrittenTxId <= currentSegment.endtxid()) {
            highestWrittenTxId =  segment.endtxid();
        }
      }

      if (syncLog(reqInfo, segment, fromUrl, syncedFile) != 0) {
          return -1;
      }
    } else {
      LOG.info("Skipping download of log [%d, %d] already have up-to-date logs",  segment.starttxid(), segment.endtxid() );
    }

    // This is one of the few places in the protocol where we have a single
    // RPC that results in two distinct actions:
    //
    // - 1) Downloads the new log segment data (above)
    // - 2) Records the new Paxos data about the synchronized segment (below)
    //
    // These need to be treated as a transaction from the perspective
    // of any external process. We do this by treating the persistPaxosData()
    // success as the "commit" of an atomic transaction. If we fail before
    // this point, the downloaded edit log will only exist at a temporary
    // path, and thus not change any externally visible state. If we fail
    // after this point, then any future prepareRecovery() call will see
    // the Paxos data, and by calling completeHalfDoneAcceptRecovery() will
    // roll forward the rename of the referenced log file.
    //
    // See also: HDFS-3955
    if (persistPaxosData(segmentTxId, newData) != 0) {
        return -1;
    }

    if (!syncedFile.empty()) {
      if( file_rename(syncedFile, storage.getInProgressEditLog(segmentTxId)) != 0 ){
          return -1;
      }
    }

    LOG.info("Accepted recovery for segment %d : [%d, %d] accepted in epoch %d", segmentTxId,
            newData.segmentstate().starttxid(), newData.segmentstate().endtxid(), newData.acceptedinepoch());
    return 0;
  }

/**
   * Synchronize a log segment from another JournalNode. The log is
   * downloaded from the provided URL into a temporary location on disk,
   * which is named based on the current request's epoch.
   *
   * @return the temporary location of the downloaded file
   */
int
Journal::syncLog(const RequestInfo& reqInfo,
      const hadoop::hdfs::SegmentStateProto& segment, const string& url, string &ret) {
    string tmpFile = storage.getSyncLogTemporaryFile(
        segment.starttxid(), reqInfo.getEpoch());

    ofstream os(tmpFile.c_str());
    int responseCode= -1;
    try{
        // That's all that is needed to do cleanup of used resources (RAII style).
        curlpp::Cleanup cleaner;
        curlpp::Easy myRequest;
        curlpp::options::WriteStream ws(&os);
        myRequest.setOpt(new curlpp::options::Url(url));
        myRequest.setOpt(ws);
        myRequest.perform();
        os.close();
        responseCode = curlpp::infos::ResponseCode::get(myRequest);
    }catch(...){
        os.close();
        LOG.error("exception raised while trying to download file %s from url %s", tmpFile.c_str(), url.c_str());
    }
    if (responseCode == 200) {
        ret = tmpFile;
    }else {
        LOG.error("unable to download file %s from url %s", tmpFile.c_str(), url.c_str());
        file_delete(tmpFile);
        return -1;
    }
    return 0;
}

} /* namespace JournalServiceServer */
