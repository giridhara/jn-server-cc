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

#include "JNClientOutputStream.h"

using namespace std;

namespace hadoop
{
namespace JournalServiceServer
{
  JNClientOutputStream::JNClientOutputStream(
      const seq_t txId,
	  const string data)
	  : mSize(0),
	    mOpcode(OPCODE),
		mDataSize(data.size())
  {
    int length = calculateSize();
    mBuffer = new char[length];
    writeByte(mOpcode);
    // write the length : content of the op  - op_code - checksum
    writeInt(length - 5);
    writeLong(txId);
    writeString(data);
    writeChecksum();
  }

  JNClientOutputStream::~JNClientOutputStream()
  {
    delete[] mBuffer;
  }

  template <typename T>
    void
    JNClientOutputStream::store_as_big_endian(T u)
    {
      union
      {
        T u;
        unsigned char u8[sizeof(T)];
      } source;

      source.u = u;
      for (size_t k = 0; k < sizeof(T); k++)
      {
          if (IS_LITTLE_ENDIAN)
            mBuffer[mSize++] = source.u8[sizeof(T) - k - 1];
          else
            mBuffer[mSize++] = source.u8[k];
      }
    }

  void
    JNClientOutputStream::writeChecksum()
    {
      boost::crc_32_type result;
      result.process_bytes(mBuffer, mSize);
      int checksum = result.checksum();
      writeInt(checksum);
    }

  const char*
    JNClientOutputStream::get(size_t& length)
  {
    length = mSize;
    return mBuffer;
  }
}
}
