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

/*
 * This is a dummy class which is used by HDFS namenode for storage and interaction with datanodes.
 * For now, we will be populating dummy values to the class.
 */

#ifndef ICE_UTIL_NAMESPACEINFO
#define ICE_UTIL_NAMESPACEINFO

#include "../ice-qjournal-protocol/QJournalProtocolPB.h"
#include "StorageInfo.h"

#include <stdint.h>
#include <string>
using namespace std;

#define Stringize(x) #x
#define Stringize_Value(x) Stringize(x)

//StorageInfo(int layoutV, string cid, int nsID,  uint64_t cT)

namespace JournalServiceServer
{

class NamespaceInfo :public  StorageInfo
{
    const string   bpId;

    public:
    NamespaceInfo(const hadoop::hdfs::NamespaceInfoProto& info)
        :
            StorageInfo(info.storageinfo().layoutversion(), info.storageinfo().clusterid(), info.storageinfo().namespceid(), info.storageinfo().ctime()),
            bpId(info.blockpoolid())
        {}

    ~NamespaceInfo(){}
};
}
#endif //ICE_UTIL_NAMESPACEINFO
