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

#include <RequestHandler.h>
#include <StatusHandler.h>

RequestHandler::RequestHandler(std::shared_ptr<EngineHandler> engineHandler)
        : mSpeakRequestQueue("QUEUE_SPEAK"), mControlRequestQueue("QUEUE_CONTROL"), mEngineHandler(engineHandler)
{
}

RequestHandler::~RequestHandler()
{
    stop();
}

bool RequestHandler::sendRequest(TTSRequest* request)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    LOG_DEBUG("Request Type: %d", request->getType());

    if (request->getType() == SPEAK)
    {
        SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(request->getRequest());
        if(ptrSpeakRequest->msgParameters->bClear)
        {
          TTSRequest* pRunningRequest = mEngineHandler->getRunningSpeakRequest();
          if(nullptr != pRunningRequest)
          {
             SpeakRequest* pRunningSpeakRequest = reinterpret_cast<SpeakRequest*>(pRunningRequest->getRequest());

             if( pRunningSpeakRequest->msgParameters->eStatus == MsgStatus::TTS_MSG_PLAY)
             {
                stopSpeech(); // send stop clear message
                pRunningSpeakRequest->msgParameters->eStatus = TTS_MSG_STOP;
                LOG_DEBUG("Previous speak request is stopped\n");
              }
          }
          mSpeakRequestQueue.clearQueue();
        }
        mSpeakRequestQueue.addRequest(request);
    }
    else if (request->getType() == STOP )
    {
        StopRequest* ptrStopRequest = reinterpret_cast<StopRequest*>(request->getRequest());
        std::string stopAppID = ptrStopRequest->sAppID;
        std::string stopMsgID = ptrStopRequest->sMsgID;

        if( CheckToStopRunningSpeak(mEngineHandler->getRunningSpeakRequest(),request ) )
        {
             mControlRequestQueue.addRequest(request);
        }
        else
        {
            delete request;
            request = nullptr;
        }
        mSpeakRequestQueue.removeRequest(stopAppID,stopMsgID);
    }
    else if (request->getType() == GET_LANGUAGES )
    {
        mEngineHandler->getLanguages(request);
    }
    else if (request->getType() == GET_STATUS )
    {
        LOG_DEBUG("Request Type: %d", request->getType());
        mEngineHandler->getStatusInfo(request);
    }
    else
    {
        LOG_DEBUG("Request Type: %d", request->getType());
        mControlRequestQueue.addRequest(request);
    }

    return true;
}

void RequestHandler::start()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    mControlRequestQueue.start();
    mSpeakRequestQueue.start();
}

void RequestHandler::stop()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    mSpeakRequestQueue.stop();
    mControlRequestQueue.stop();
}

bool RequestHandler::CheckToStopRunningSpeak(TTSRequest* pRunningRequest, TTSRequest* pRequest)
{
    bool bret = false;

    if(nullptr != pRunningRequest && nullptr != pRequest)
    {
        SpeakRequest* pRunningSpeakRequest = reinterpret_cast<SpeakRequest*>(pRunningRequest->getRequest());
        StopRequest* pnewStopRequest = reinterpret_cast<StopRequest*>(pRequest->getRequest());
        std::string requestAppID = pnewStopRequest->sAppID;
        std::string requestMsgID = pnewStopRequest->sMsgID;
        if(requestMsgID.empty() && requestAppID.empty())
        {
            bret = true;
        }
        else if( !requestMsgID.empty() &&
                    (requestMsgID.compare( pRunningSpeakRequest->msgParameters->sMsgID ) == 0))
        {
            bret = true;
        }
        else if( !requestAppID.empty() &&
                    (requestAppID.compare( pRunningSpeakRequest->msgParameters->sAppID ) == 0))
        {
            bret = true;
        }
    }
    return bret;
}

void RequestHandler::stopSpeech()
{
    StopRequest *stopRequest = new (std::nothrow)StopRequest;
    if(stopRequest == nullptr){
        return;
    }
    TTSRequest* request =new (std::nothrow)TTSRequest(reinterpret_cast<RequestType*>(stopRequest), mEngineHandler);
    if(request == nullptr){
        delete stopRequest;
        return;
    }

    mControlRequestQueue.addRequest(request);
}
