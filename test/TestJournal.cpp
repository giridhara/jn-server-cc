/*
 * TestJournal.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: psarda
 */
#include <string>
#include <src/Journal.h>
#include <src/TestJNClientOutputStream.h>

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

void testRestartJournal(){
    cout << "////BEGINNING TO RUN testRestartJournal TEST CASE////" << endl;
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
    cout << "lpe expected is 1" << endl;
    cout << "lpe returned is " << lpe << endl;
    assert(lpe == 1);

    hadoop::hdfs::NewEpochResponseProto resp;

    journal->newEpoch(FAKE_NSINFO, 2, resp);
    cout << "expected lastsegmenttxid is 1" << endl;
    cout << "returned lastsegmenttxid is " << resp.lastsegmenttxid() << endl;
    assert(1 == resp.lastsegmenttxid());
    cout << "****END OF testRestartJournal TEST CASE****" << endl;
}

void testFormatResetsCachedValues() {
    cout << "////BEGINNING TO RUN testFormatResetsCachedValues test case////" << endl;
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
    assert(12345L == lpe);
    long int lwe;
    journal->getLastWriterEpoch(lwe);
    assert(12345 == lwe);
    assert(journal->isFormatted());

    // Close the journal in preparation for reformatting it.
    journal->close();

    journal->format(createFakeNSINFO_2());
    journal->getLastPromisedEpoch(lpe);
    assert (0 == lpe);
    journal->getLastWriterEpoch(lwe);
    assert(0 == lwe);
    assert(journal->isFormatted());
    cout << "****END OF testFormatResetsCachedValues TEST CASE****" << endl;
}

/**
   * Test that, if the writer crashes at the very beginning of a segment,
   * before any transactions are written, that the next newEpoch() call
   * returns the prior segment txid as its most recent segment.
   */
void testNewEpochAtBeginningOfSegment() {
    cout << "////BEGINNING TO RUN testNewEpochAtBeginningOfSegment test case////" << endl;
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
    assert(1 == resp.lastsegmenttxid());
    cout << "****END OF testNewEpochAtBeginningOfSegment TEST CASE****" << endl;
}

int main() {
    testRestartJournal();
    testFormatResetsCachedValues();
    testNewEpochAtBeginningOfSegment();
}
