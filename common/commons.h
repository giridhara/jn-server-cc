/*
 * commons.h
 *
 *  Created on: Sep 2, 2014
 *      Author: psarda
 */

#ifndef COMMONS_H_
#define COMMONS_H_

namespace JournalServiceServer
{
struct CallStatus{
    int status;
    string message;
public:
    CallStatus(int status, string message)
        :status(status),
         message(message)
    {}
};

struct EditLogValidation {
    const long validLength;
    const long endTxId;
    const bool corruptHeader;

    EditLogValidation(long validLength, long endTxId,
        bool corruptHeader)
        :
        validLength(validLength),
        endTxId(endTxId),
        corruptHeader(corruptHeader)
    {}

    long getValidLength() { return validLength; }

    long getEndTxId() { return endTxId; }

    bool hasCorruptHeader() { return corruptHeader; }
  };

}



#endif /* COMMONS_H_ */
