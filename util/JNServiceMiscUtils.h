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

using namespace std;
using std::string;


namespace JournalServiceServer
{
  const bool IS_LITTLE_ENDIAN = (1 == *(unsigned char *)&(const int){1});

string getNameNodeFileName(const string filenamePrefix, long txid) {
     ostringstream strm;
     strm << filenamePrefix;
     strm  << setfill('0') << setw(19) << txid;
    return strm.str();
}

string getInProgressEditsFileName(long startTxId) {
    return getNameNodeFileName(EDITS_INPROGRESS, startTxId);
}

string getInProgressEditsFile(string currentDir, long startTxId) {
    return string(currentDir + "/" + getInProgressEditsFileName(startTxId));
}

string getFinalizedEditsFileName(long startTxId, long endTxId) {
    ostringstream strm;
    strm << EDITS << "_";
    strm << setfill('0') << setw(19) << startTxId;
    strm << "-";
    strm << setfill('0') << setw(19) << endTxId;
    return strm.str();
  }

string getFinalizedEditsFile(string currentDir,
    long startTxId, long endTxId) {
    return string(currentDir + "/" +
        getFinalizedEditsFileName(startTxId, endTxId));
}

const bool file_exists(const string& name) {
    ifstream ifs (name.c_str());
    if(!ifs.is_open()) {
        return false;
    }
    ifs.close();
    return true;
}

const int file_rename(const string& from, const string& to ) {
    return rename(from.c_str(), to.c_str());
}

}

#endif //UTIL_JNSERVICE_MISC_UTILS


