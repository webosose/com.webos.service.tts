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
    if(mReqType == NULL){
        return;
    }
    if (mReqType->requestType == SPEAK)
    {
       SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(mReqType);
       if(ptrSpeakRequest->msgParameters){
           delete ptrSpeakRequest->msgParameters;
           ptrSpeakRequest->msgParameters = nullptr;
       }
       delete ptrSpeakRequest;
    }
    else if(mReqType->requestType == STOP)
    {
        StopRequest* ptrStopRequest = reinterpret_cast<StopRequest*>(mReqType);
        delete ptrStopRequest;
    }
    else if(mReqType->requestType == GET_LANGUAGES)
    {
        GetLanguageRequest* ptrGetLanguageRequest = reinterpret_cast<GetLanguageRequest*>(mReqType);
        ptrGetLanguageRequest->vecLanguages.clear();
        delete ptrGetLanguageRequest;
    }else if(mReqType->requestType == GET_STATUS){
        LOG_DEBUG("For Stop Memory Leak handled Properly");
    }else{
        LOG_WARNING(MSGID_TTS_ERROR, 0, "Unhandled Request Type %d, See for memory Leak", mReqType->requestType);
        delete mReqType;
    }
    mReqType = nullptr;
}

bool TTSRequest::execute()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("Executing Request: %d", mReqType->requestType);
    int displayId = 0;
    if (SPEAK == mReqType->requestType)
    {
        SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(mReqType);
        displayId = ptrSpeakRequest->msgParameters->displayId;
        LOG_DEBUG("TTSRequest::execute : SPEAK displayId = %d", displayId);
    }
    else if (STOP == mReqType->requestType)
    {
        StopRequest* ptrStopRequest = reinterpret_cast<StopRequest*>(mReqType);
        displayId = ptrStopRequest->displayId;
        LOG_DEBUG("TTSRequest::execute : STOP displayId = %d", displayId);
    }
    LOG_DEBUG("TTSRequest : displayId = %d", displayId);
    bool exeStatus = mEngineHandler->handleRequest(this, displayId);
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
