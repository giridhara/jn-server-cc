#ifndef ICE_UTIL_REQUESTINFO
#define ICE_UTIL_REQUESTINFO
#include <string>

#include "../ice-qjournal-protocol/QJournalProtocolPB.h"

using std::string;


namespace JournalServiceServer
{

class RequestInfo {
    static const long INVALID_TXID = -12345;
    private :
        const string jid;
        long epoch;
        long ipcSerialNumber;
        const long committedTxId;

    public :
        RequestInfo(string jid, long epoch, long ipcSerialNumber, long committedTxId)
            :
            jid(jid),
            epoch(epoch),
            ipcSerialNumber(ipcSerialNumber),
            committedTxId(committedTxId)
        {}

        RequestInfo(const hadoop::hdfs::RequestInfoProto& reqInfo)
            :
            jid(reqInfo.journalid().identifier()),
            epoch(reqInfo.epoch()),
            ipcSerialNumber(reqInfo.ipcserialnumber()),
            committedTxId(reqInfo.has_committedtxid()? reqInfo.committedtxid() : INVALID_TXID)
        {}

        long getEpoch() {
        return epoch;
        }

        void setEpoch(long epoch) {
            this->epoch = epoch;
        }

        string getJournalId() {
            return jid;
        }

        long getIpcSerialNumber() {
            return ipcSerialNumber;
        }

        void setIpcSerialNumber(long ipcSerialNumber) {
            this->ipcSerialNumber = ipcSerialNumber;
        }

        long getCommittedTxId() {
            return committedTxId;
        }

        bool hasCommittedTxId() {
            return (committedTxId != INVALID_TXID);
        }
};

}

#endif //ICE_UTIL_REQUESTINFO
