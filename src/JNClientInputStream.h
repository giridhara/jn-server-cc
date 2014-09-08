/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* This class decodes the transactions which are obtained from the journal service
 * We need to adhere to the format which HDFS uses to dump its transaction.
 * This is because the servers read back the transactions from the logs when recovering.
 * In case the format does not match, server is unable to recover the logs.
 * The operation is stored in journal server as follows:
 * 1 byte of opcode (we are using DisallowSnapshotOp to dump our transaction)
 * 4 bytes(int) of length of data written (sizeof(txId) + sizeof(data) + sizeof(checksum))
 * 8 bytes(long) of txId
 * n number of data bytes
 * 4 bytes(int) of checksum of data written
 * Currently, we do not use the checksum to verify the correctness of logs obtained
*/
#ifndef ICE_UTIL_JN_CLIENT_INPUT_STREAM_H
#define ICE_UTIL_JN_CLIENT_INPUT_STREAM_H

#include "JournalServiceInterfaceUtils.h"
#include "JNClientErrorCodes.h"

#include <string>
using namespace std;

namespace hadoop
{
namespace JournalServiceClient
{
  class JNClientInputStream
  {
    TxLogs mLogs;
    seq_t mFirstTxId;
    seq_t mLastTxId;
    size_t mCurPosition;
    seq_t mNextTxId;
    bool mInProgress;
    string mStrm;

    public:
    JNClientInputStream(const string ostrm, const seq_t firstTxId, const seq_t lastTxId, const bool isInProgress);

    const seq_t getFirstTxId() const { return mFirstTxId;}
    const seq_t getLastTxId() const { return mLastTxId; }

    JNClientStatus readLogs(TxLogs& logs);

    const bool operator>(const JNClientInputStream &other) const;

    const bool operator<(const JNClientInputStream &other) const;

    const bool operator==(const JNClientInputStream &other) const;

    private:
    JNClientStatus readOp();

    JNClientStatus skipHeader();

    JNClientStatus readOpcode(unsigned char& opcode);

    template <typename T>
      T read_as_type();

    JNClientStatus readInt(int& i);

    JNClientStatus readLong(long& l);

    JNClientStatus readString(size_t length, string& data);

  };
}
}
#endif //ICE_UTIL_JN_CLIENT_INPUT_STREAM_H
