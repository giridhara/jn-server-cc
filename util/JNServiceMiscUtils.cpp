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
#include "Logger.h"
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

namespace JournalServiceServer
{
HostPortPair::HostPortPair(const string& name)
{
    size_t host_pos = name.find_first_of(":");
    hostname = name.substr(0, host_pos);
    port = atoi(name.substr(host_pos + 1).c_str());
}

bool HostPortPair::isValid(const string& name) {
    size_t host_pos = name.find_first_of(":");
    size_t length = name.size();
    if (host_pos == string::npos || host_pos == 0 || host_pos == length)
        return false;
    for (size_t i = host_pos + 1; i < length; i++) {
        if (name[i] < '0' || name[i] > '9')
            return false;
    }
    return true;
}

string getNameNodeFileName(const string& filenamePrefix, const long txid)
{
    ostringstream strm;
    strm << filenamePrefix << "_";
    strm << setfill('0') << setw(19) << txid;
    return strm.str();
}

string getInProgressEditsFileName(const long startTxId) {
    return getNameNodeFileName(EDITS_INPROGRESS, startTxId);
}

string getInProgressEditsFile(const string& currentDir, const long startTxId) {
    return string(currentDir + "/" + getInProgressEditsFileName(startTxId));
}

string getFinalizedEditsFileName(const long startTxId, const long endTxId) {
    ostringstream strm;
    strm << EDITS << "_";
    strm << setfill('0') << setw(19) << startTxId;
    strm << "-";
    strm << setfill('0') << setw(19) << endTxId;
    return strm.str();
  }

string getFinalizedEditsFile(const string& currentDir,
    const long startTxId, const long endTxId) {
    return string(currentDir + "/" +
        getFinalizedEditsFileName(startTxId, endTxId));
}

int file_exists(const string& name, bool& file_exists_flag) {
    bool temp = false;

    try {
        temp = boost::filesystem::is_regular(name);
    } catch (const boost::filesystem::filesystem_error& ex) {
        LOG.error("%s", ex.what());
        return -1;
    }
    file_exists_flag = temp;
    return 0;
}

int dir_exists(const string& name, bool& dir_exists_flag) {
    bool temp = false;

    try {
        temp = boost::filesystem::is_directory(name);
    } catch (const boost::filesystem::filesystem_error& ex) {
        LOG.error("%s", ex.what());
        return -1;
    }
    dir_exists_flag = temp;
    return 0;
}

int file_rename(const string& from, const string& to ) {
    try{
        boost::filesystem::rename(from, to);
    } catch (const boost::filesystem::filesystem_error& ex) {
        LOG.error("%s", ex.what());
        return -1;
    }
    return 0;
}

int file_delete(const string& name) {
    try{
        boost::filesystem::remove(name.c_str());
    } catch (const boost::filesystem::filesystem_error& ex) {
        LOG.error("%s", ex.what());
        return -1;
    }
    return 0;
}

/**
 * Move the src file to the name specified by target.
 * @param src the source file
 * @param target the target file
 * @exception IOException If this operation fails
 */
int replaceFile(const string& src, const string& target) {
  /* renameTo() has two limitations on Windows platform.
   * src.renameTo(target) fails if
   * 1) If target already exists OR
   * 2) If target is already open for reading/writing.
   */
    int numRetries = 5;
    while (numRetries-- > 0) {
        try {
            bool target_exists = false;
            file_exists(target, target_exists);
            if (!target_exists)
                break;
            if (file_delete(target) == 0) {
                break;
            }
            usleep(1 * 1000 * 1000);
        } catch (const boost::filesystem::filesystem_error& ex) {
            if (numRetries <= 0) {
                break;
            }
        }
    }
    return file_rename(src, target);
}

int
is_absolute_path(const string& path) {
    boost::filesystem::path temp(path);
    return !temp.root_directory().empty();
}

int
try_to_acquire_lockfile(const string &lockfn)
{
    const int fd = open(lockfn.c_str(), O_APPEND|O_CREAT|O_RDWR, 0644);
    if (fd < 0) {
        return (errno > 0 ? -errno : -1);
    }
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLK, &fl)) {
        const int err = errno;
        close(fd);
        return (err > 0 ? -err : -1);
    }
    return fd;
}

}

