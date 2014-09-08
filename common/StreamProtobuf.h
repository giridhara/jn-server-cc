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

#ifndef STREAM_PROTOBUF_H
#define STREAM_PROTOBUF_H

#include <Ice/Ice.h>
#include <google/protobuf/message_lite.h>

#ifdef ICE_CPP11
#include <type_traits>
#endif

//
// Tell Ice how to marshal and unmarshal a protobuf message to/from a sequence<byte>
//
namespace Ice
{

#ifdef ICE_CPP11
  //
  // We'll use std::enable_if to give protobuf its own helper category (see below)
  //
  const StreamHelperCategory StreamHelperCategoryProtobuf = 100;
#else
  //
  // We just assume a single "custom" category: Unknown
  //
  const StreamHelperCategory StreamHelperCategoryProtobuf = StreamHelperCategoryUnknown;
#endif

#ifdef ICE_CPP11

  template<typename T>
    struct StreamableTraits<T, typename std::enable_if<std::is_base_of< ::google::protobuf::MessageLite, T>::value >::type>
    {
      static const StreamHelperCategory helper = StreamHelperCategoryProtobuf;
      static const int minWireSize = 1;
      static const bool fixedLength = false;
    };

#else
  //
  // We use the default 'Unknown' StreamHelperCategory
  //
#endif

  template<typename T>
    struct StreamHelper<T, StreamHelperCategoryProtobuf>
    {
      template<class S> static inline void
        write(S* stream, const ::google::protobuf::MessageLite& v)
        {
          std::vector<Byte> data(v.ByteSize());
          if(!v.IsInitialized())
          {
            throw MarshalException(__FILE__, __LINE__,
                "message not fully initialized: " + v.InitializationErrorString());
          }

          Byte* r = v.SerializeWithCachedSizesToArray(&data[0]);
          if(r != &data[0] + data.size())
          {
            throw MarshalException(__FILE__, __LINE__, "message modified during marshaling");
          }

          stream->write(&data[0], &data[0] + data.size());
        }

      template<class S> static inline void
        read(S* stream, ::google::protobuf::MessageLite& v)
        {
          std::pair<const Byte*, const Byte*> data;
          stream->read(data);
          if(!v.ParseFromArray(data.first, static_cast<int>(data.second - data.first)))
          {
            throw MarshalException(__FILE__, __LINE__, "ParseFromArray failed");
          }
        }
    };

  //
  // Use default marshaling/unmarshaling, with optionalFormat = OptionalFormatVSize
  //
  template<>
    struct GetOptionalFormat<StreamHelperCategoryProtobuf, 1, false>
    {
      static const OptionalFormat value = OptionalFormatVSize;
    };

}
#endif
