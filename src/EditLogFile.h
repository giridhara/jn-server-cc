/*
 * EditLogFile.h
 *
 *  Created on: Sep 7, 2014
 *      Author: psarda
 */

#ifndef EDITLOGFILE_H_
#define EDITLOGFILE_H_

#include <string>
#include<assert.h>

using std::string;

namespace JournalServiceServer
{

class EditLogFile
{
public:
    EditLogFile(string file, long firstTxId,
                    long lastTxId, bool inProgress)
        :
        file(file),
        firstTxId(firstTxId),
        lastTxId(lastTxId),
        inProgress(inProgress),
        corruptHeader(false)

    {
        //TODO :: have to write proper asserts
//          assert (lastTxId == INVALID_TXID && isInProgress)
//            || (lastTxId != INVALID_TXID && lastTxId >= firstTxId);
//          assert (firstTxId > 0) || (firstTxId == INVALID_TXID);
//          assert (file != 0);
//
//          assert(!isInProgress ||
//              lastTxId == INVALID_TXID);
    }

    EditLogFile(const EditLogFile& other)
      : firstTxId(other.firstTxId),
        inProgress(other.inProgress)
    {
      file = other.file;
      lastTxId = other.lastTxId;
      corruptHeader = other.corruptHeader;
    }

    virtual ~EditLogFile() {}

    const long getFirstTxId() {
      return firstTxId;
    }

    long getLastTxId() {
      return lastTxId;
    }

    bool containsTxId(long txId) {
      return firstTxId <= txId && txId <= lastTxId;
    }

//    /**
//         * Find out where the edit log ends.
//         * This will update the lastTxId of the EditLogFile or
//         * mark it as corrupt if it is.
//         */
//        void validateLog() {
//          EditLogValidation val = EditLogFileInputStream.validateEditLog(file);
//          this.lastTxId = val.getEndTxId();
//          this.hasCorruptHeader = val.hasCorruptHeader();
//        }
//
//        void scanLog() {
//          EditLogValidation val = EditLogFileInputStream.scanEditLog(file);
//          this.lastTxId = val.getEndTxId();
//          this.hasCorruptHeader = val.hasCorruptHeader();
//        }

        const bool isInProgress() {
          return inProgress;
        }

        string getFile() {
          return file;
        }

        bool hasCorruptHeader() {
          return corruptHeader;
        }

private :
    string file;
    // firstTxId is supposed to be a constant
    const long firstTxId;
    long lastTxId;
    //inProgress is supposed to be a constant
    const bool inProgress;
    bool corruptHeader;

};

} /* namespace JournalServiceServer */

#endif /* EDITLOGFILE_H_ */
