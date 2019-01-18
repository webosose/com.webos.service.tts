// Copyright (c) 2018-2019 LG Electronics, Inc.
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

#include <iostream>
#include <EngineHandler.h>
#include <TTSLog.h>
#include <TTSRequest.h>

TTSRequest::TTSRequest(RequestType *reqType, std::shared_ptr<EngineHandler> engineHandler)
        : mReqType(reqType), mEngineHandler(engineHandler)
{

}

TTSRequest::~TTSRequest()
{
    if (mReqType) delete mReqType;
}

bool TTSRequest::execute()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("Executing Request: %d", mReqType->requestType);
    bool exeStatus = mEngineHandler->handleRequest(this);
    LOG_DEBUG("Executing Request: %d exeStatus:%d", mReqType->requestType ,exeStatus);
    return exeStatus;
}

REQUEST_TYPE TTSRequest::getType()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    return mReqType->requestType;
}

RequestType* TTSRequest::getRequest()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    return mReqType;
}
