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
#include "Logger.h"

#include <ctime>
#include <sstream>

#include <sys/time.h>
#include <sys/resource.h>
#include "time.h"

// Create a global object LOG which can be used by whoever wants to log
// messages to a log file
namespace JournalServiceServer
{
  Logger LOG;

  Logger::Logger():
    mLevel(LOG_DEBUG),
    mBufMsgCountMax(10),
    mCurBufMsgCount(0)
  {
    mFstrm = stdout;
  }

  void
    Logger::setLogLevel(const LogLevel level)
    {
      mLevel = level;
    }

  void
    Logger::debug(const char* strFormat, ...)
    {
      va_list args;
      va_start(args, strFormat);
      write(LOG_DEBUG, strFormat, args);
      va_end(args);
    }

  void
    Logger::info(const char* strFormat, ...)
    {
      va_list args;
      va_start(args, strFormat);
      write(LOG_INFO, strFormat, args);
      va_end(args);
    }

  void
    Logger::warn(const char* strFormat, ...)
    {
      va_list args;
      va_start(args, strFormat);
      write(LOG_WARN, strFormat, args);
      va_end(args);
    }

  void
    Logger::error(const char* strFormat, ...)
    {
      va_list args;
      va_start(args, strFormat);
      write(LOG_ERROR, strFormat, args);
      va_end(args);
    }

  string getCurrentTime()
  {
    time_t t = time(0);   // get time now
    struct timeval tv;
    if (gettimeofday(&tv, 0) < 0)
        return "";
    ostringstream timeStr;
    struct tm * now = localtime( & t );
    timeStr << (now->tm_year + 1900) << '-' 
      << (now->tm_mon + 1) << '-'
      <<  now->tm_mday << '-'
      <<  now->tm_hour << ':'
      <<  now->tm_min  << ':'
      <<  now->tm_sec  << ':'
      <<  tv.tv_usec   << '\t';
    return timeStr.str();
  }
  void
    Logger::write(const LogLevel level, const char* strFormat, va_list args)
    {
      if (level < mLevel) return;

     string formatString(getCurrentTime());

      switch(level)
      {
        case LOG_DEBUG:
          formatString += "DEBUG:"; break;
        case LOG_INFO:
          formatString += "INFO:"; break;
        case LOG_WARN:
          formatString += "WARN:"; break;
        case LOG_ERROR:
          formatString += "ERROR:"; break;
      }
      formatString+=strFormat;
      vfprintf(mFstrm, formatString.c_str(), args);
      fprintf(mFstrm,"\n");
      mCurBufMsgCount++;
      if (level == LOG_ERROR || level == LOG_WARN || level == LOG_INFO || level == LOG_DEBUG || mCurBufMsgCount > mBufMsgCountMax)
      {
        fflush(mFstrm);
        mCurBufMsgCount=0;
      }
    }
}
