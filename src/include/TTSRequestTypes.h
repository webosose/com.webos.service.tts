// Copyright (c) 2018-2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SRC_INCLUDE_TTSREQUESTTYPES_H_
#define SRC_INCLUDE_TTSREQUESTTYPES_H_

#include <string>
#include <luna-service2/lunaservice.hpp>
#include <TTSParameters.h>

enum REQUEST_TYPE
{
    SPEAK = 100, START, STOP, GET_STATUS, GET_LANGUAGES
};

typedef struct RequestType
{
    REQUEST_TYPE requestType;
    Parameters* msgParameters;
    LSHandle* sh;
} RequestType;

typedef struct SpeakRequest
{
    const REQUEST_TYPE commandId = SPEAK;
    Parameters* msgParameters;
    LSHandle* sh;
    std::string text_to_speak;
    void (*replyCB)(Parameters* msgParameters, LS::Message &message);
    LS::Message message;
} SpeakRequest;

typedef struct StartRequest
{
    const REQUEST_TYPE commandId = START;
    std::string text_to_speak;
} StartRequest;

typedef struct StopRequest
{
    const REQUEST_TYPE commandId = STOP;
    bool fadeOut = false;
    std::string sAppID;
    std::string sMsgID;
    int displayId;
} StopRequest;

typedef struct GetStatusRequest
{
    const REQUEST_TYPE commandId = GET_STATUS;
    LSHandle* sh;
    TTSStatus* pTTSStatus;
    void (*replyCB)(TTSStatus* pTTSStatus, LS::Message &message);
    LS::Message message;
    int displayId;
} GetStatusRequest;

typedef struct GetLanguageRequest
{
    const REQUEST_TYPE commandId = GET_LANGUAGES;
    std::vector<std::string>  vecLanguages;
    int displayId;
} GetLanguageRequest;

#endif /* SRC_INCLUDE_TTSREQUESTTYPES_H_ */
