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
#include "JNServiceMiscUtils.h"

#include <stdlib.h>

namespace JournalServiceServer
{
HostPortPair::HostPortPair(string name)
  {
    size_t host_pos = name.find_first_of(":");
    hostname = name.substr(0, host_pos);
    port = atoi(name.substr(host_pos + 1).c_str());
  }

bool HostPortPair::isValid(string name) {
    size_t host_pos = name.find_first_of(":");
    size_t length = name.size();
    if (host_pos == string::npos || host_pos == 0 || host_pos == length)
      return false;
    for (size_t i = host_pos +1; i < length; i++)
    {
      if ( name[i] < '0' || name[i] > '9' )
        return false;
    }
    return true;
  }
}

