// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <QJournalProtocolPB.h>

using namespace std;
using namespace QJournalProtocolProtos;

class HelloClient : public Ice::Application
{
public:

    HelloClient();

    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    HelloClient app;
    return app.main(argc, argv, "config.client");
}

HelloClient::HelloClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
HelloClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    QJournalProtocolPBPrx prx = QJournalProtocolPBPrx::checkedCast(communicator()->propertyToProxy("QjournalProtocolPB.Proxy"));
    if(!prx)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    hadoop::hdfs::JournalIdProto* journalid = new hadoop::hdfs::JournalIdProto();
    journalid->set_identifier("abc");
//    hadoop::hdfs::IsFormattedRequestProto req;
//    req.set_allocated_jid(journalid);
//    IsFormattedResponseProto resp;

    hadoop::hdfs::GetJournalStateRequestProto req;
    req.set_allocated_jid(journalid);
    GetJournalStateResponseProto resp;

    menu();

    char c;
    do
    {
        try
        {
            cout << "==> ";
            cin >> c;
            if(c == 'f')
            {
//                resp = prx->isFormatted(req);
//                cout << "is JN formatted :" << resp.isformatted() << endl;
            }else if(c == 's')
            {
                resp = prx->getJournalState(req);
                cout << "lastpromisedepoch : " << resp.lastpromisedepoch() << endl;
                cout << "fromurl : " << resp.fromurl() << endl;
                cout << "httpport : " << resp.httpport() << endl;
            }
            else if(c == '?')
            {
                menu();
            }
            else
            {
                cout << "unknown command `" << c << "'" << endl;
                menu();
            }
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
        }
    }
    while(cin.good() && c != 'x');

    return EXIT_SUCCESS;
}

void
HelloClient::menu()
{
    cout <<
        "usage:\n"
        "f: is formatted\n"
        "s: get jorunal state\n"
        "?: help\n";
}
