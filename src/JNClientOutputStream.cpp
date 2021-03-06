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
#include "util/JNServiceMiscUtils.h"

namespace JournalServiceServer
{
JNClientOutputStream::~JNClientOutputStream()
{
    close();
}

void
JNClientOutputStream::close(){
    stream.close();
}

int JNClientOutputStream::abort(){
    stream.close();
    return 0;
}

int
JNClientOutputStream::writeRaw(const string& records) {
      stream << records;
      return 0;
  }

bool
JNClientOutputStream::flush(){
    stream.flush();
    return !stream.fail();
}

bool
JNClientOutputStream::create(const int layoutVersion) {
    stream.seekp(0);
    // write header - layout version , followed by layout flags
    writeInt(layoutVersion);
    writeInt(0);
    return flush();
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
           stream.write( reinterpret_cast < const char* >(&source.u8[sizeof(T) - k - 1]), 1);
         else
           stream.write(reinterpret_cast < const char* >(&source.u8[k]), 1);
     }
   }
}
