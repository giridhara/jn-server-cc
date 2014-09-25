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

/* This class encodes the transactions to the journal service
 * We need to adhere to the format which HDFS uses to dump its transaction.
 * This is because the servers read back the transactions from the logs when recovering.
 * In case the format does not match, server is unable to recover the logs.
 * The operation is stored in journal server as follows:
 * 1 byte of opcode (we are using DisallowSnapshotOp to dump our transaction)
 * 4 bytes(int) of length of data written (sizeof(txId) + sizeof(data) + sizeof(checksum))
 * 8 bytes(long) of txId
 * n number of data bytes
 * 4 bytes(int) of checksum of data written
*/

#ifndef TESTJNCLIENT_OUTPUT_STREAM_H
#define TESTJNCLIENT_OUTPUT_STREAM_H

#include <util/JNServiceMiscUtils.h>

#include <boost/crc.hpp>
#include <string.h>


using namespace std;

namespace JournalServiceServer
{
  class TestJNClientOutputStream
  {
    public:
      TestJNClientOutputStream(const long txId, const string data);
      ~TestJNClientOutputStream();

      const char* get(unsigned int& length);
    private:
      const int calculateSize() const {return 17 + mDataSize; /*1 + 4 + 8 + data.size() + 4*/}

      template <typename T>
        void store_as_big_endian(T u);

      void writeByte(char opcode) {mBuffer[mSize++] = opcode;}

      void writeInt(int i) { store_as_big_endian<int>(i);}

      void writeLong(long l) { store_as_big_endian<long>(l);}

      void writeString(string data) { memcpy(mBuffer + mSize, data.c_str(), mDataSize); mSize += mDataSize; }

      void writeChecksum();

      char*   mBuffer;
      size_t  mSize;
      char    mOpcode;
      size_t  mDataSize;
  };

}
#endif //JNCLIENT_OUTPUT_STREAM_H
