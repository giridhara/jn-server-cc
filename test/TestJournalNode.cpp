/*
 * TestJournalNode.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: psarda
 */


#include <src/JournalNode.h>
#include <src/TestJNClientOutputStream.h>
#include <string>
#include <gtest/gtest.h>

using namespace JournalServiceServer;

const string JID="TestCluster";
const int LAYOUTVERSION = -57;
const string TESTDATA("TestTransactionData");

Ice::PropertiesPtr properties;

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

RequestInfo makeRI(int serial) {
    RequestInfo req(JID, 1, serial, 0);
    return req;
}

RequestInfo makeRI(int epoch, int serial) {
    RequestInfo req(JID, epoch, serial, 0);
    return req;
}

string createTransaction(const seq_t txId, const string data) {
    TestJNClientOutputStream outStream(txId, data);
    unsigned int length(0);
    const char* records = outStream.get(length);
    string str(records, length);
    return str;
}

TEST(TestJournalNode, testAcceptRecoveryBehavior) {
    (JournalServiceServer::global_jn).reset(new JournalServiceServer::JournalNode(properties));
    JournalServiceServer::global_jn->start();
    Journal* journal;
    global_jn->getOrCreateJournal(JID, journal);
    NamespaceInfo FAKE_NSINFO(createFakeNSINFO());
    journal->format(FAKE_NSINFO);

    JournalNodeRpcServer* rpcServer = global_jn->getJNRPCServer();

    // We need to run newEpoch() first, or else we have no way to distinguish
    // different proposals for the same decision.
//    try {
//      ch.prepareRecovery(1L).get();
//      fail("Did not throw IllegalState when trying to run paxos without an epoch");
//    } catch (ExecutionException ise) {
//      GenericTestUtils.assertExceptionContains("bad epoch", ise);  -- This error message is from in side client side of the system, hence not doing this call
//    }

    hadoop::hdfs::NewEpochResponseProto epochResp;
    rpcServer->newEpoch(JID, FAKE_NSINFO, (uint64_t)1, epochResp);

    // prepare() with no previously accepted value and no logs present
    hadoop::hdfs::PrepareRecoveryResponseProto prepRecoveryRespProto;

    rpcServer->prepareRecovery(makeRI(1), 1, prepRecoveryRespProto);
    cerr << "Prep" << endl;
    ASSERT_EQ(false, prepRecoveryRespProto.has_acceptedinepoch());
    ASSERT_EQ(false, prepRecoveryRespProto.has_segmentstate());

    // Make a log segment, and prepare again -- this time should see the
    // segment existing.
    ASSERT_EQ(0, rpcServer->startLogSegment(makeRI(2), 1, LAYOUTVERSION));
    ASSERT_EQ(0, rpcServer->journal(makeRI(3), 1, 1, 1, createTransaction(1, TESTDATA)));

    prepRecoveryRespProto.Clear();
    rpcServer->prepareRecovery(makeRI(4), 1, prepRecoveryRespProto);
    cerr << "Prep" << endl;
    ASSERT_EQ(false, prepRecoveryRespProto.has_acceptedinepoch());
    ASSERT_EQ(true, prepRecoveryRespProto.has_segmentstate());

    // accept() should save the accepted value in persistent storage
    rpcServer->acceptRecovery(makeRI(5), prepRecoveryRespProto.segmentstate(), string("file:///dev/null"));

    // So another prepare() call from a new epoch would return this value
    epochResp.Clear();
    rpcServer->newEpoch(JID, FAKE_NSINFO, (uint64_t)2, epochResp);
    prepRecoveryRespProto.Clear();
    rpcServer->prepareRecovery(makeRI(2,6), 1, prepRecoveryRespProto);

    ASSERT_EQ(1, prepRecoveryRespProto.acceptedinepoch());
    ASSERT_EQ(1, prepRecoveryRespProto.segmentstate().endtxid());

    // A prepare() or accept() call from an earlier epoch should now be rejected
    prepRecoveryRespProto.Clear();
    ASSERT_EQ(-1, rpcServer->prepareRecovery(makeRI(1,7), 1, prepRecoveryRespProto));
    //"epoch 1 is less than the last promised epoch 2"

    ASSERT_EQ(-1, rpcServer->acceptRecovery(makeRI(5), prepRecoveryRespProto.segmentstate(), string("file:///dev/null")));
    //"epoch 1 is less than the last promised epoch 2"
}

int main(int argc, char** argv) {
    properties = Ice::createProperties();
    properties->load(argv[1]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
