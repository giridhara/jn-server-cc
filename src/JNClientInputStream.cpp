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

#include "JNClientInputStream.h"

#include "../util/Logger.h"
#include "../util/JNServiceMiscUtils.h"

using namespace std;

namespace JournalServiceServer
{
  JNClientInputStream::JNClientInputStream(const string& ostrm):
    mCurPosition(0),
    mStrm(ostrm)
  {}

int
JNClientInputStream::scanLog(string& filename, long& lastTxId, bool& hasHeaderCorrupted) {
    std::ifstream t(filename.c_str());
    if(!t.is_open()) {
      LOG.error("Could not open file %s", filename.c_str());
      lastTxId = INVALID_TXID;
      hasHeaderCorrupted = true;
      return -1;
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    cout << "data read from file is '" << buffer.str() << "'" << endl;
    LOG.debug("length of data read from file is %d", buffer.str().length());
    JNClientInputStream in(buffer.str());
    t.close();
    if (in.mCurPosition == 0) {
      if (in.skipHeader() != 0)
      {
        LOG.error("Error while parsing transaction logs while skipping header");
        return -1;
      }
    }

    long tempLast = INVALID_TXID;
    long numValid = 0;
    while (true) {
        long txid = INVALID_TXID;
        if (in.scanOp(txid) != 0) {
          LOG.error("Got error after scanning over %d transactions at position %d", numValid, in.mCurPosition);
          return -1;
        }
        if(txid == INVALID_TXID){
            break;
        }
        if (tempLast == INVALID_TXID || txid > tempLast) {
            tempLast = txid;
        }
        numValid++;
    }
    lastTxId = tempLast;
    hasHeaderCorrupted = false;
    return 0;
//        return new EditLogValidation(lastPos, lastTxId, false);
}

/**
   * Similar with decodeOp(), but instead of doing the real decoding, we skip
   * the content of the op if the length of the editlog is supported.
   * @return the last txid of the segment, or INVALID_TXID on exception
*/
int
JNClientInputStream::scanOp(long& ret) {
    unsigned char opcode;
    //End of File Reached
    if(mCurPosition >= mStrm.length()) {
        ret =  INVALID_TXID;
        return 0;
    }
    if (readOpcode(opcode) != 0) {
        LOG.error("Error while parsing transaction logs to get opcode");
        ret = INVALID_TXID;
        return -1;
    }
    if(opcode != OPCODE){
        LOG.error("Error while parsing transaction logs to get opcode, found %d but expected %d", opcode, OPCODE);
        ret = INVALID_TXID;
        return -1;
    }

    int length;
    if (readInt(length) != 0) {  // read the length of the op
        LOG.error("Error while parsing transaction logs, obtained wrong length");
        return -1;
    }
    long txId;
    if (readLong(txId) != 0) {  // read the txid
        LOG.error("Error while parsing transaction logs.");
        return -1;
    }

    // read left over part of op which is the data written = length - sizeof(length) -sizeof(txId)
    if(scanString(length-12) != 0 ){
        LOG.error("Error while parsing transaction logs to get data");
        return -1;
    }

    int checksum;
    if (readInt(checksum) != 0){
        LOG.error("Error while parsing transaction logs to get checksum");
        return -1;
    }
    ret = txId;
    return 0;
}

  int
    JNClientInputStream::skipHeader()
    {
      int layoutVersion;
      int status = readInt(layoutVersion);
      if (status != 0) return -1;

      int layoutFlags;
      status = readInt(layoutFlags);
      if (status != 0) return -1;
      if (layoutFlags != 0)
        status = -1;
      return status;
    }

int
JNClientInputStream::readOpcode(unsigned char& opcode) {
    opcode = mStrm[mCurPosition++];
    return 0;
}

  template <typename T>
    T
    JNClientInputStream::read_as_type()
    {
      union
      {
        T u;
        unsigned char u8[sizeof(T)];
      } dest;

      for (size_t k = 0; k < sizeof(T); k++)
      {
        if (IS_LITTLE_ENDIAN)
          dest.u8[sizeof(T) - k - 1] = mStrm[mCurPosition++];
        else
          dest.u8[k] = mStrm[mCurPosition++];
      }
      return dest.u;
    }

int
JNClientInputStream::readInt(int& i) {
    i = read_as_type<int>();
    return 0;
}

int
JNClientInputStream::readLong(long& l) {
    l = read_as_type<long>();
    return 0;
}

int
JNClientInputStream::scanString(size_t length){
    mCurPosition += length;
    return 0;
}

}
