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

#ifndef JNCLIENT_OUTPUT_STREAM_H
#define JNCLIENT_OUTPUT_STREAM_H

#include <string>

using std::string;

namespace JournalServiceServer
{
  class JNClientOutputStream
  {
    public:
      JNClientOutputStream(const string filename)
        :
          filename(filename)
      {}
      ~JNClientOutputStream();

     /** TODO : need to implement this function
     * Close the stream without necessarily flushing any pending data.
     * This may be called after a previous write or close threw an exception.
     */
      int abort();

      string getFileName() {
          return filename;
      }

      int close();

      int writeRaw(const char* records, int offset, int length);
      void setReadyToFlush();
      void flush(bool& shouldFsync);

    private:
      string filename;
      ofstream  stream;
  };
}
#endif //JNCLIENT_OUTPUT_STREAM_H
