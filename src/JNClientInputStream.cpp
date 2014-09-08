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
namespace hadoop
{
namespace JournalServiceClient
{
  JNClientInputStream::JNClientInputStream(const string ostrm, const seq_t firstTxId, const seq_t lastTxId, const bool isInProgress):
    mFirstTxId(firstTxId),mLastTxId(lastTxId),
    mCurPosition(0),mNextTxId(firstTxId),
    mInProgress(isInProgress), mStrm(ostrm) {}

  JNClientStatus
    JNClientInputStream::readLogs(TxLogs& logs)
    {
      JNClientStatus status(JNCLIENT_OK);
      while (mNextTxId <= mLastTxId)
      {
        status = readOp();
        if (status != JNCLIENT_OK)
          break;
      }
      logs = mLogs;
      return status;
    }

  const bool
    JNClientInputStream::operator>(const JNClientInputStream &other) const
    {
      // Finalized log is better than unfinalized
      if (mInProgress != other.mInProgress)
      {
        return !mInProgress;
      }
      if (mFirstTxId > other.mFirstTxId)
        return true;
      return false;
    }

  const bool
    JNClientInputStream::operator<(const JNClientInputStream &other) const
    {
      if (mInProgress != other.mInProgress)
      {
        return !mInProgress;
      }
      if (mFirstTxId < other.mFirstTxId)
        return true;
      return false;
    }

  const bool
    JNClientInputStream::operator==(const JNClientInputStream &other) const
    {
      if (mInProgress != other.mInProgress)
        return false;
      if (mFirstTxId != other.mFirstTxId)
        return false;
      if (mLastTxId != other.mLastTxId)
        return false;
      return true;
    }

  JNClientStatus
    JNClientInputStream::readOp()
    {
      JNClientStatus status(JNCLIENT_OK);

      if (mCurPosition == 0)
      {
        status = skipHeader();
        if (status != JNCLIENT_OK)
        {
          LOG.error("Error while parsing transaction logs while skipping header");
          return status;
        }
      }

      unsigned char opcode;
      status = readOpcode(opcode);
      if ( status != JNCLIENT_OK)
      {
        LOG.error("Error while parsing transaction logs while getting opcode");
        return status;
      }
      if (opcode != OPCODE)
      {
        LOG.error("Error while parsing transaction logs, obtained wrong opcode");
        return JNCLIENT_FAIL_INPUT_STREAM_PARSE_LOGS;
      }
      int length;
      status = readInt(length);
      if (status != JNCLIENT_OK)
      {
        LOG.error("Error while parsing transaction logs, obtained wrong length");
        return status;
      }
      seq_t txId;
      status = readLong(txId);
      if (status != JNCLIENT_OK)
      {
        LOG.error("Error while parsing transaction logs.");
        return status;
      }
      if (txId != mNextTxId)
      {
        LOG.error("Obtained wrong txid, expected %u found %u",mNextTxId, txId);
        return JNCLIENT_FAIL_INPUT_STREAM_PARSE_LOGS;
      }
      mNextTxId = txId + 1;

      // read left over part of op which is tha data written = length - sizeof(length) -sizeof(txId)
      string str;
      status = readString(length-12, str);

      if (status != JNCLIENT_OK)
      {
        LOG.error("Error while parsing transaction logs to get data");
        return status;
      }

      int checksum;
      status = readInt(checksum);
      if (status != JNCLIENT_OK)
      {
        LOG.error("Error while parsing transaction logs to get checksum");
        return status;
      }
      // TODO: Not doing anything with checksum currently.

      Log log(txId, str);
      mLogs.push_back(log);
      return status;
    }

  JNClientStatus
    JNClientInputStream::skipHeader()
    {
      int layoutVersion;
      JNClientStatus status = readInt(layoutVersion);
      if (status != JNCLIENT_OK) return status;

      int layoutFlags;
      status = readInt(layoutFlags);
      if (status != JNCLIENT_OK) return status;
      if (layoutFlags != 0)
        status = JNCLIENT_FAIL_INPUT_STREAM_PARSE_LOGS;
      return status;
    }

  JNClientStatus
    JNClientInputStream::readOpcode(unsigned char& opcode)
    {
      opcode = mStrm[mCurPosition++];
      return JNCLIENT_OK;
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
  JNClientStatus
    JNClientInputStream::readInt(int& i)
    {
      i = read_as_type<int>();
      return JNCLIENT_OK;
    }
  JNClientStatus
    JNClientInputStream::readLong(long& l)
    {
      l = read_as_type<long>();
      return JNCLIENT_OK;
    }
  JNClientStatus
    JNClientInputStream::readString(size_t length, string& data)
    {
      for (size_t i = 0; i < length; i++)
        data += mStrm[mCurPosition++];
      return JNCLIENT_OK;
    }
}
}
