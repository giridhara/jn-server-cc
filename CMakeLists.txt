# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.3)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-DBUILD_VERSION=${BUILD_VERSION}")
    set(CMAKE_CXX_FLAGS "-DSOFTWARE_VERSION=2.4.0")
endif()

set ( BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/target)
set ( ICE_BUILD_DIR ${BUILD_DIR}/generated-sources/ice )
set ( PROTO_BUILD_DIR ${BUILD_DIR}/generated-sources/proto )

# Add a clean-all target to the Makefile
ADD_CUSTOM_TARGET(clean-all COMMAND ${CMAKE_COMMAND} -P
${CMAKE_SOURCE_DIR}/clean-all.cmake)

file( MAKE_DIRECTORY ${ICE_BUILD_DIR} )
file( MAKE_DIRECTORY ${PROTO_BUILD_DIR} )

###############################
###### Compile ICE file #######
###############################

find_path( ICE_INCLUDE_PATH Ice.h
# installation selected by user
${ICE_HOME}/include/Ice
$ENV{ICE_HOME}/include/Ice
# debian package installs Ice here
/usr/include/Ice
)

if( ICE_INCLUDE_PATH )
  get_filename_component( ICE_HOME_INCLUDE ${ICE_INCLUDE_PATH} PATH )
  get_filename_component( ICE_HOME ${ICE_HOME_INCLUDE} PATH CACHE )
else (NOT ICE_INCLUDE_PATH)
  message(FATAL_ERROR "Could not find ICE_HOME")
endif (ICE_INCLUDE_PATH)

find_path( CURL_INCLUDE_PATH cURLpp.hpp
${CURL_HOME}/include/curlpp
$ENV{CURL_HOME}/include/curlpp
/usr/include/curlpp
)

if( CURL_INCLUDE_PATH )
  get_filename_component( CURL_HOME_INCLUDE ${CURL_INCLUDE_PATH} PATH )
  get_filename_component( CURL_HOME ${CURL_HOME_INCLUDE} PATH CACHE )
else (NOT CURL_INCLUDE_PATH )
  message(FATAL_ERROR "Could not find CURL_HOME")
endif (CURL_INCLUDE_PATH)

set ( HADOOP_ICE_SRC ${CMAKE_CURRENT_LIST_DIR}/ice-qjournal-protocol )
find_path( ICE_SLICE_DIR QJournalProtocolPB.ice
  ${HADOOP_ICE_SRC}
)

set ( SLICE_SRC QJournalProtocolPB.ice )

execute_process(
  COMMAND slice2cpp --output-dir ${ICE_BUILD_DIR} ${ICE_SLICE_DIR}/${SLICE_SRC} )

###############################
#### Proto file compilation ###
###############################

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set ( hdfs_proto_srcdir ${CMAKE_CURRENT_LIST_DIR}/ice-qjournal-protocol )
set ( common_proto_srcdir ${CMAKE_CURRENT_LIST_DIR}/ice-qjournal-protocol )

set( PROTO_SRCS
  ${hdfs_proto_srcdir}/QJournalProtocol.proto
  ${hdfs_proto_srcdir}/hdfs.proto
  ${common_proto_srcdir}/Security.proto
)

execute_process(
COMMAND protoc --proto_path=${common_proto_srcdir} --proto_path=${hdfs_proto_srcdir} --cpp_out=${PROTO_BUILD_DIR} ${PROTO_SRCS}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

###############################
## QJournal file compilation ##
###############################
include_directories(
${PROTO_BUILD_DIR}
${ICE_BUILD_DIR}
${CMAKE_CURRENT_SOURCE_DIR}
${CURL_INCLUDE_PATH})

set ( journalservice_files
ice-rpc-cc/src/Server.cc
src/JNStorage.cpp
src/FileJournalManager.cpp
src/JournalNodeHttpServer.cpp
src/JournalNodeRpcServer.cpp
src/EditLogFile.cpp
src/JNClientOutputStream.cpp
src/TestJNClientOutputStream.cpp
src/JNClientInputStream.cpp
src/Journal.cpp
util/Logger.cpp
util/JNServiceMiscUtils.cpp
util/mongoose.c
ice-qjournal-protocol/QJournalProtocolServerSideTranslatorPB.cpp )

FILE( GLOB protoSrcs ${PROTO_BUILD_DIR}/*.pb.cc  )
FILE( GLOB iceSrcs ${ICE_BUILD_DIR}/*.cpp  )

find_library(
  REQUIRED_LIBRARIES
  NAMES "curlpp" "protobuf" "Ice" "IceUtil"
  PATHS "/usr/lib64"
)

if (NOT REQUIRED_LIBRARIES)
  message( FATAL_ERROR "Required libraries not found" )
endif (NOT REQUIRED_LIBRARIES)

set(CMAKE_REQUIRED_LIBRARIES "$CMAKE_REQUIRED_LIBRARIES" "$REQUIRED_LIBRARIES")

set (exe_files JournalNode)
foreach (exe_file ${exe_files})
    add_executable (${exe_file}
			src/JournalNode.cpp
            ${journalservice_files}
			${protoSrcs}
			${iceSrcs}
        )
        target_link_libraries (${exe_file}
            IceUtil
            Ice
            protobuf
            curlpp
           pthread
			boost_filesystem
			boost_regex
			gtest
        )
endforeach (exe_file)
