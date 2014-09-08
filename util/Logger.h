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
 * This file contains the logging interface to log debug/info/warn/error messages to a log file.
 */
#ifndef UTIL_LOGGER_H
#define UTIL_LOGGER_H

#include "Constants.h"

#include <ostream>      // std::flush
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <iostream>

using namespace std;

namespace JournalServiceServer
{
  class Logger
  {
    FILE* mFstrm;
    LogLevel mLevel;
    unsigned long mBufMsgCountMax;
    unsigned long mCurBufMsgCount;

    public:
    Logger();

    void setLogLevel(const LogLevel level);

    void debug(const char* strFormat, ...);

    void info(const char* strFormat, ...);

    void warn(const char* strFormat, ...);

    void error(const char* strFormat, ...);

    private:
    void write(const LogLevel level, const char* strFormat, va_list args);

  };

  extern Logger LOG;
}
#endif //UTIL_LOGGER_H
