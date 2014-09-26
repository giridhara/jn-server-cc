/*
 * TestJournal.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: psarda
 */
#include <string>
#include <src/Journal.h>
#include <src/TestJNClientOutputStream.h>
#include <gtest/gtest.h>

using namespace JournalServiceServer;

const string JID="TestCluster";
const string LOGDIR="/home/psarda/qfsbase/jn/data/TestCluster";
const int LAYOUTVERSION = -57;
const string TESTDATA("TestTransactionData");


RequestInfo makeRI(int serial) {
    RequestInfo req(JID, 1, serial, 0);
    return req;
}

string createTransaction(const seq_t txId, const string data) {
    TestJNClientOutputStream outStream(txId, data);
    unsigned int length(0);
    const char* records = outStream.get(length);
    string str(records, length);
    return str;
}

NamespaceInfo createFakeNSINFO(){
    hadoop::hdfs::NamespaceInfoProto nsInfoProto;
    nsInfoProto.set_blockpoolid("bp-id");
    nsInfoProto.set_buildversion("0.1");
    nsInfoProto.set_softwareversion("0.1");
    nsInfoProto.set_unused(1);
    hadoop::hdfs::StorageInfoProto* storage = new hadoop::hdfs::StorageInfoProto();
    storage->set_clusterid("CID-007");
    storage->set_ctime(0);
    storage->set_layoutversion(LAYOUTVERSION);
    storage->set_namespceid(123);
    nsInfoProto.set_allocated_storageinfo(storage);
    NamespaceInfo nsInfo(nsInfoProto);
    return nsInfo;
}

NamespaceInfo createFakeNSINFO_2(){
    hadoop::hdfs::NamespaceInfoProto nsInfoProto;
    nsInfoProto.set_blockpoolid("bp-id");
    nsInfoProto.set_buildversion("0.1");
    nsInfoProto.set_softwareversion("0.1");
    nsInfoProto.set_unused(1);
    hadoop::hdfs::StorageInfoProto* storage = new hadoop::hdfs::StorageInfoProto();
    storage->set_clusterid("CID-007");
    storage->set_ctime(0);
    storage->set_layoutversion(LAYOUTVERSION);
    storage->set_namespceid(6789);
    nsInfoProto.set_allocated_storageinfo(storage);
    NamespaceInfo nsInfo(nsInfoProto);
    return nsInfo;
}

  /**
   * Test whether JNs can correctly handle editlog that cannot be decoded.
   */
TEST(TestJournal, testScanEditLog) {
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());

    journal->format(FAKE_NSINFO);

    // use a future layout version
    journal->startLogSegment(makeRI(1), 1, LAYOUTVERSION);

    // in the segment we write garbage editlog, which can be scanned but
    // cannot be decoded
    int numTxns = 5;
    journal->journal(makeRI(2), 1, 1, 1, createTransaction(1, TESTDATA));
    journal->journal(makeRI(3), 1, 2, 1, createTransaction(2, TESTDATA));
    journal->journal(makeRI(4), 1, 3, 1, createTransaction(3, TESTDATA));
    journal->journal(makeRI(5), 1, 4, 1, createTransaction(4, TESTDATA));
    journal->journal(makeRI(6), 1, 5, 1, createTransaction(5, TESTDATA));

    // verify the in-progress editlog segment
    bool isSegmentInitialized = false;

    hadoop::hdfs::SegmentStateProto segmentState;
    journal->getSegmentInfo(1, segmentState, isSegmentInitialized);

    ASSERT_EQ(true, isSegmentInitialized);
    ASSERT_EQ(true, segmentState.isinprogress());
    ASSERT_EQ(numTxns, segmentState.endtxid());
    ASSERT_EQ(1, segmentState.starttxid());

    // finalize the segment and verify it again
    journal->finalizeLogSegment(makeRI(7), 1, numTxns);
    segmentState.Clear();
    isSegmentInitialized = false;
    journal->getSegmentInfo(1, segmentState, isSegmentInitialized);
    ASSERT_EQ(true, isSegmentInitialized);
    ASSERT_EQ(false, segmentState.isinprogress());
    ASSERT_EQ(numTxns, segmentState.endtxid());
    ASSERT_EQ(1, segmentState.starttxid());
}

TEST(TestJournal, testEpochHandling){
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());

    journal->format(FAKE_NSINFO);
    long int lpe;
    journal->getLastPromisedEpoch(lpe);
    ASSERT_EQ(0, lpe);
    hadoop::hdfs::NewEpochResponseProto newEpochRespProto;
    journal->newEpoch(FAKE_NSINFO, 1, newEpochRespProto);
    ASSERT_EQ(false, newEpochRespProto.has_lastsegmenttxid());
    journal->getLastPromisedEpoch(lpe);
    ASSERT_EQ(1, lpe);
    newEpochRespProto.Clear();
    journal->newEpoch(FAKE_NSINFO, 3, newEpochRespProto);
    ASSERT_EQ(false, newEpochRespProto.has_lastsegmenttxid());
    journal->getLastPromisedEpoch(lpe);
    ASSERT_EQ(3, lpe);
    newEpochRespProto.Clear();
    ASSERT_EQ(-1, journal->newEpoch(FAKE_NSINFO, 3, newEpochRespProto));
    //"Proposed epoch 3 <= last promise 3"
    ASSERT_EQ(-1, journal->startLogSegment(makeRI(1), 12345,LAYOUTVERSION));
    //"epoch 1 is less than the last promised epoch 3"
    journal->journal(makeRI(1), 12345, 100, 1, createTransaction(100, TESTDATA));
    //"epoch 1 is less than the last promised epoch 3"
}

TEST(TestJournal, testRestartJournal){
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());

    journal->format(FAKE_NSINFO);

    hadoop::hdfs::NewEpochResponseProto resProto;
    journal->newEpoch(FAKE_NSINFO, 1, resProto);
    journal->startLogSegment(makeRI(1), 1,
            LAYOUTVERSION);
    journal->journal(makeRI(2), 1, 1, 1, createTransaction(1, TESTDATA));

    // Don't finalize.
//TODO:
//    String storageString = journal.getStorage().toColonSeparatedString();
//    System.err.println("storage string: " + storageString);
    journal->close(); // close to unlock the storage dir

    // Now re-instantiate, make sure history is still there
    journal = new Journal(conf, LOGDIR, JID);
    if(journal->isFormatted()) {
        journal->getStorage().readProperties(journal->getStorage().getVersionFile());
    }

    // The storage info should be read, even if no writer has taken over.
//    assertEquals(storageString,
//        journal.getStorage().toColonSeparatedString());

    long int lpe;
    journal->getLastPromisedEpoch(lpe);
    ASSERT_EQ(1, lpe);

    hadoop::hdfs::NewEpochResponseProto resp;

    journal->newEpoch(FAKE_NSINFO, 2, resp);
    ASSERT_EQ(1, resp.lastsegmenttxid());
}

TEST(TestJournal, testFormatResetsCachedValues) {
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());

    journal->format(FAKE_NSINFO);
    hadoop::hdfs::NewEpochResponseProto resProto;
    journal->newEpoch(FAKE_NSINFO, 12345, resProto);
    RequestInfo reqi(JID, 12345, 1, 0);
    journal->startLogSegment(reqi, 1, LAYOUTVERSION);

    long int lpe;
    journal->getLastPromisedEpoch(lpe);
    ASSERT_EQ(lpe, 12345);
    long int lwe;
    journal->getLastWriterEpoch(lwe);
    ASSERT_EQ(lwe, 12345);
    ASSERT_EQ(true, journal->isFormatted());

    // Close the journal in preparation for reformatting it.
    journal->close();

    journal->format(createFakeNSINFO_2());
    journal->getLastPromisedEpoch(lpe);
    ASSERT_EQ(0, lpe);
    journal->getLastWriterEpoch(lwe);
    ASSERT_EQ(0, lwe);
    ASSERT_EQ(true, journal->isFormatted());
}

/**
   * Test that, if the writer crashes at the very beginning of a segment,
   * before any transactions are written, that the next newEpoch() call
   * returns the prior segment txid as its most recent segment.
   */
TEST(TestJournal, testNewEpochAtBeginningOfSegment) {
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());

    journal->format(FAKE_NSINFO);
    hadoop::hdfs::NewEpochResponseProto resProto;
    journal->newEpoch(FAKE_NSINFO, 1, resProto);
    journal->startLogSegment(makeRI(1), 1, LAYOUTVERSION);
    journal->journal(makeRI(2), 1, 1, 1, createTransaction(1, TESTDATA));
    journal->journal(makeRI(3), 1, 2, 1, createTransaction(2, TESTDATA));
    journal->finalizeLogSegment(makeRI(4), 1, 2);
    journal->startLogSegment(makeRI(5), 3, LAYOUTVERSION);
    hadoop::hdfs::NewEpochResponseProto resp;
    journal->newEpoch(FAKE_NSINFO, 2, resp);
    ASSERT_EQ(1, resp.lastsegmenttxid());
}

TEST(TestJournal, testFinalizeWhenEditsAreMissed) {
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());

    journal->format(FAKE_NSINFO);
    hadoop::hdfs::NewEpochResponseProto resProto;
    journal->newEpoch(FAKE_NSINFO, 1, resProto);
    journal->startLogSegment(makeRI(1), 1, LAYOUTVERSION);
    journal->journal(makeRI(2), 1, 1, 1, createTransaction(1, TESTDATA));
    journal->journal(makeRI(3), 1, 2, 1, createTransaction(2, TESTDATA));
    journal->journal(makeRI(4), 1, 3, 1, createTransaction(3, TESTDATA));

    // Try to finalize up to txn 6, even though we only wrote up to txn 3.
    ASSERT_EQ(-1, journal->finalizeLogSegment(makeRI(5), 1, 6));
    //"but only written up to txid 3"

    // Check that, even if we re-construct the journal by scanning the
    // disk, we don't allow finalizing incorrectly.
    journal->close();
    journal = new Journal(conf, LOGDIR, JID);
    if(journal->isFormatted()) {
        journal->getStorage().readProperties(journal->getStorage().getVersionFile());
    }

    ASSERT_EQ(-1, journal->finalizeLogSegment(makeRI(6), 1, 6));
    //"disk only contains up to txid 3"
}

/**
   * Ensure that finalizing a segment which doesn't exist throws the
   * appropriate exception.
   */
TEST(TestJournal, testFinalizeMissingSegment){
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());
    journal->format(FAKE_NSINFO);
    hadoop::hdfs::NewEpochResponseProto resProto;
    journal->newEpoch(FAKE_NSINFO, 1, resProto);
    ASSERT_EQ(-1, journal->finalizeLogSegment(makeRI(1), 1000, 1001));
//    "No log file to finalize at transaction ID 1000", e);
}

/**
   * Assume that a client is writing to a journal, but loses its connection
   * in the middle of a segment. Thus, any future journal() calls in that
   * segment may fail, because some txns were missed while the connection was
   * down.
   *
   * Eventually, the connection comes back, and the NN tries to start a new
   * segment at a higher txid. This should abort the old one and succeed.
*/
TEST(TestJournal, testAbortOldSegmentIfFinalizeIsMissed){
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());
    journal->format(FAKE_NSINFO);
    hadoop::hdfs::NewEpochResponseProto resProto;
    journal->newEpoch(FAKE_NSINFO, 1, resProto);

    // Start a segment at txid 1, and write a batch of 3 txns.
    journal->startLogSegment(makeRI(1), 1, LAYOUTVERSION);
    journal->journal(makeRI(2), 1, 1, 1, createTransaction(1, TESTDATA));
    journal->journal(makeRI(3), 1, 2, 1, createTransaction(2, TESTDATA));
    journal->journal(makeRI(4), 1, 3, 1, createTransaction(3, TESTDATA));
    bool inprogress_elf_exists = false;
    file_exists(journal->getStorage().getInProgressEditLog(1), inprogress_elf_exists);
    ASSERT_EQ(inprogress_elf_exists, true);

    // Try to start new segment at txid 6, this should abort old segment and
    // then succeed, allowing us to write txid 6-9.
    journal->startLogSegment(makeRI(5), 6, LAYOUTVERSION);
    journal->journal(makeRI(6), 6, 6, 1, createTransaction(6, TESTDATA));
    journal->journal(makeRI(7), 6, 7, 1, createTransaction(7, TESTDATA));
    journal->journal(makeRI(8), 6, 8, 1, createTransaction(8, TESTDATA));

    // The old segment should *not* be finalized.
    inprogress_elf_exists = false;
    file_exists(journal->getStorage().getInProgressEditLog(1), inprogress_elf_exists);
    ASSERT_EQ(inprogress_elf_exists, true);
    inprogress_elf_exists = false;
    file_exists(journal->getStorage().getInProgressEditLog(6), inprogress_elf_exists);
    ASSERT_EQ(inprogress_elf_exists, true);
}

TEST(TestJournal, testStartLogSegmentWhenAlreadyExists) {
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());
    journal->format(FAKE_NSINFO);
    hadoop::hdfs::NewEpochResponseProto resProto;
    journal->newEpoch(FAKE_NSINFO, 1, resProto);

    // Start a segment at txid 1
    journal->startLogSegment(makeRI(1), 1, LAYOUTVERSION);

    // Try to start new segment at txid 1, this should succeed, because
    // we are allowed to re-start a segment if segment is empty
    ASSERT_EQ(journal->startLogSegment(makeRI(2), 1,
        LAYOUTVERSION), 0);
    journal->journal(makeRI(3), 1, 1, 1, createTransaction(1, TESTDATA));

    // This time through, write more transactions afterwards, simulating
    // real user transactions.
    journal->journal(makeRI(4), 1, 2, 1, createTransaction(2, TESTDATA));
    journal->journal(makeRI(5), 1, 3, 1, createTransaction(3, TESTDATA));
    journal->journal(makeRI(6), 1, 4, 1, createTransaction(4, TESTDATA));

    // This should fail
    ASSERT_DEATH( journal->startLogSegment(makeRI(7), 1,
          LAYOUTVERSION), "seems to contain valid transactions");

    journal->finalizeLogSegment(makeRI(8), 1, 4);

    // Ensure that we cannot overwrite a finalized segment
    ASSERT_DEATH(journal->startLogSegment(makeRI(9), 1, LAYOUTVERSION), "have a finalized segment" );
}

TEST(TestJournal, testNamespaceVerification){
    Ice::PropertiesPtr conf = Ice::createProperties();
    Journal* journal = new Journal(conf, LOGDIR, JID);

    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());
    journal->format(FAKE_NSINFO);
    hadoop::hdfs::NewEpochResponseProto resProto;
    journal->newEpoch(FAKE_NSINFO, 1, resProto);

    resProto.Clear();
    NamespaceInfo FAKE_NSINFO_2(createFakeNSINFO_2());
    ASSERT_EQ(-1, journal->newEpoch(FAKE_NSINFO_2, 2, resProto));
    //"Incompatible namespaceID"
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
