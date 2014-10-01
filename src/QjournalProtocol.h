#ifndef QJOURNALPROTOCOL_H_
#define QJOURNALPROTOCOL_H_

#include <QJournalProtocolPB.h>
#include "../util/RequestInfo.h"
#include "../util/NamespaceInfo.h"

#include <string>

//using namespace QJournalProtocolProtos;

namespace JournalServiceServer
{
class QJournalProtocol {
public :

    QJournalProtocol() {}
    virtual ~QJournalProtocol() {}

  /**
   * @return true if the given journal has been formatted and
   * contains valid data.
   */
  virtual int isFormatted(const string& journalId, bool&) = 0;

  /**
   * Get the current state of the journal, including the most recent
   * epoch number and the HTTP port.
   */
  virtual int getJournalState(const string& journalId, hadoop::hdfs::GetJournalStateResponseProto&) = 0;

  /**
   * Format the underlying storage for the given namespace.
   */
  virtual int format(const string& journalId, const NamespaceInfo& nsInfo) = 0;

  /**
   * Begin a new epoch. See the HDFS-3077 design doc for details.
   */
  virtual int newEpoch(const string& journalId, const NamespaceInfo& nsInfo,
          const uint64_t epoch, hadoop::hdfs::NewEpochResponseProto&) = 0;

  /**
   * Journal edit records.
   * This message is sent by the active name-node to the JournalNodes
   * to write edits to their local logs.
   */
  virtual int journal(const RequestInfo& reqInfo, const long segmentTxId,
          const long firstTxnId, const int numTxns, const string& records) = 0;

  /**
   * Start writing to a new log segment on the JournalNode.
   * Before calling this, one should finalize the previous segment
   * using {@link #finalizeLogSegment(RequestInfo, long, long)}.
   *
   * @param txid the first txid in the new log
   * @param layoutVersion the LayoutVersion of the new log
   */
  virtual int startLogSegment(const RequestInfo& reqInfo,
      const long txid, const int layoutVersion) = 0;

  /**
   * Finalize the given log segment on the JournalNode. The segment
   * is expected to be in-progress and starting at the given startTxId.
   *
   * @param startTxId the starting transaction ID of the log
   * @param endTxId the expected last transaction in the given log
   * @throws IOException if no such segment exists
   */
  virtual int finalizeLogSegment(const RequestInfo& reqInfo,
      const long startTxId, const long endTxId) = 0;

  /**
   * @param jid the journal from which to enumerate edits
   * @param sinceTxId the first transaction which the client cares about
   * @param inProgressOk whether or not to check the in-progress edit log
   *        segment
   * @return a list of edit log segments since the given transaction ID.
   */
  virtual int getEditLogManifest(const string& jid, const long sinceTxId,
          const bool inProgressOk, hadoop::hdfs::GetEditLogManifestResponseProto&) = 0;

  /**
   * Begin the recovery process for a given segment. See the HDFS-3077
   * design document for details.
   */
  virtual int prepareRecovery(const RequestInfo& reqInfo,
      const long segmentTxId, hadoop::hdfs::PrepareRecoveryResponseProto&) = 0;

  /**
   * Accept a proposed recovery for the given transaction ID.
   */
  virtual int acceptRecovery(const RequestInfo& reqInfo,
          const hadoop::hdfs::SegmentStateProto& stateToAccept, const string& fromUrl) = 0;
};

}

#endif //QJOURNALPROTOCOL_H_
