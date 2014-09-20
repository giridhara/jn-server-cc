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
#include "JNClientInputStream.h"

using std::string;

namespace JournalServiceServer
{

class EditLogFile
{
public:
    EditLogFile(string fullFileName, long firstTxId,
                    long lastTxId, bool inProgress)
        :
        fullFileName(fullFileName),
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

    //copy constructor
    EditLogFile(const EditLogFile& other)
      : firstTxId(other.firstTxId),
        inProgress(other.inProgress)
    {
      fullFileName = other.fullFileName;
      lastTxId = other.lastTxId;
      corruptHeader = other.corruptHeader;
    }

//    //assignment opertor
//    EditLogFile& operator = (const EditLogFile& other){
//        cout << "assignment operator called for file " << other.file << endl;
//        file = other.file;
//        firstTxId =  other.firstTxId;
//        lastTxId =other.lastTxId;
//        inProgress = other.inProgress;
//        corruptHeader = other.corruptHeader;
//
//        return *this;
//    }


    EditLogFile():
        fullFileName(""),
        firstTxId(INVALID_TXID),
        lastTxId(INVALID_TXID),
        inProgress(false),
        corruptHeader(false)
    {}

    bool isInitialized() {
        return !fullFileName.empty();
    }

    virtual ~EditLogFile() {}

    const long getFirstTxId() const{
      return firstTxId;
    }

    long getLastTxId() const{
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

    const bool isInProgress() {
      return inProgress;
    }

    string getFile() {
      return fullFileName;
    }



    bool hasCorruptHeader() {
      return corruptHeader;
    }

    const bool operator<(const EditLogFile &other) const;
    const bool operator>(const EditLogFile &other) const;
    const bool operator==(const EditLogFile &other) const;

    int scanLog() {
      return JNClientInputStream::scanLog(fullFileName, lastTxId, corruptHeader);
    }

    int moveAsideCorruptFile() {
        //assert hasCorruptHeader;
       return renameSelf(".corrupt");
    }

   int  moveAsideTrashFile(long markerTxid){
    // assert this.getFirstTxId() >= markerTxid;
       return renameSelf(".trash");
   }

   int moveAsideEmptyFile() {
    // assert lastTxId == HdfsConstants.INVALID_TXID;
       return renameSelf(".empty");
   }

   int renameSelf(string newSuffix);

private :
    string fullFileName;
    // firstTxId is supposed to be a constant
    long firstTxId;
    long lastTxId;
    //inProgress is supposed to be a constant
    bool inProgress;
    bool corruptHeader;

};

} /* namespace JournalServiceServer */

#endif /* EDITLOGFILE_H_ */
