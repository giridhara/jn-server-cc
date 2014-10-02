#ifndef ICE_UTIL_REQUESTINFO
#define ICE_UTIL_REQUESTINFO
#include <string>

#include <QJournalProtocolPB.h>

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
        RequestInfo(const string& jid, const long epoch, const long ipcSerialNumber, const long committedTxId)
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

        long getEpoch() const {
            return epoch;
        }

        void setEpoch(long epoch) {
            this->epoch = epoch;
        }

        string getJournalId() const{
            return jid;
        }

        long getIpcSerialNumber() const {
            return ipcSerialNumber;
        }

        void setIpcSerialNumber(long ipcSerialNumber) {
            this->ipcSerialNumber = ipcSerialNumber;
        }

        long getCommittedTxId() const {
            return committedTxId;
        }

        bool hasCommittedTxId() const {
            return (committedTxId != INVALID_TXID);
        }
};

}

#endif //ICE_UTIL_REQUESTINFO
