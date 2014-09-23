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
 * This is a helper class for jn-client.
 */
#ifndef UTIL_JNSERVICE_MISC_UTILS
#define UTIL_JNSERVICE_MISC_UTILS

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include "Constants.h"
#include <stdio.h>
#include <boost/filesystem.hpp>

using namespace std;
using std::string;


namespace JournalServiceServer
{
  const bool IS_LITTLE_ENDIAN = (1 == *(unsigned char *)&(const int){1});

string getNameNodeFileName(const string filenamePrefix, long txid);

string getInProgressEditsFileName(long startTxId);
string getInProgressEditsFile(string currentDir, long startTxId);

string getFinalizedEditsFileName(long startTxId, long endTxId);

string getFinalizedEditsFile(string currentDir,
    long startTxId, long endTxId);

int file_exists(const string& name, bool& file_exists_flag);
int dir_exists(const string& name, bool& dir_exists_flag);

int file_rename(const string& from, const string& to );

int file_delete(const string& name);

struct HostPortPair
  {
    string hostname;
    int port;
    HostPortPair(string name);

    static bool isValid(string name);
  };

}

#endif //UTIL_JNSERVICE_MISC_UTILS


