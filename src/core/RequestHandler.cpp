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

#include <RequestHandler.h>
#include <StatusHandler.h>

RequestHandler::RequestHandler(std::shared_ptr<EngineHandler> engineHandler) : mSpeakRequestQueueDisplay1("QUEUE_SPEAK1"), mSpeakRequestQueueDisplay2("QUEUE_SPEAK2"), mControlRequestQueueDisplay1("QUEUE_CONTROL_1"), mControlRequestQueueDisplay2("QUEUE_CONTROL_2"), mEngineHandler(engineHandler)
{
}

RequestHandler::~RequestHandler()
{

}

bool RequestHandler::sendRequest(TTSRequest* request, int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    LOG_DEBUG("Request Type: %d", request->getType());

    if (request->getType() == SPEAK)
    {
        SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(request->getRequest());
        if(ptrSpeakRequest->msgParameters->bClear)
        {
          LOG_DEBUG("RequestHandler: DisplayId = %d", displayId);
          TTSRequest* pRunningRequest = mEngineHandler->getRunningSpeakRequest(displayId);
          if(nullptr != pRunningRequest)
          {
             SpeakRequest* pRunningSpeakRequest = reinterpret_cast<SpeakRequest*>(pRunningRequest->getRequest());

             if( pRunningSpeakRequest->msgParameters->displayId ==  displayId &&
                 pRunningSpeakRequest->msgParameters->eStatus == MsgStatus::TTS_MSG_PLAY)
             {
                stopSpeech(displayId); // send stop clear message
                pRunningSpeakRequest->msgParameters->eStatus = TTS_MSG_STOP;
                pRunningSpeakRequest->msgParameters->displayId = displayId;
                LOG_DEBUG("Previous speak request is stopped\n");
              }
          }
          if (displayId)
              mSpeakRequestQueueDisplay2.clearQueue(displayId);
          else
              mSpeakRequestQueueDisplay1.clearQueue(displayId);
        }
        if (displayId)
            mSpeakRequestQueueDisplay2.addRequest(request, displayId);
        else
            mSpeakRequestQueueDisplay1.addRequest(request, displayId);
    }
    else if (request->getType() == STOP )
    {
        StopRequest* ptrStopRequest = reinterpret_cast<StopRequest*>(request->getRequest());
        std::string stopAppID = ptrStopRequest->sAppID;
        std::string stopMsgID = ptrStopRequest->sMsgID;
        LOG_DEBUG("RequestHandler::sendRequest: Before CheckToStopRunningSpeak \n");

        if( CheckToStopRunningSpeak(mEngineHandler->getRunningSpeakRequest(displayId),request ) )
        {
             LOG_DEBUG("RequestHandler::sendRequest: After CheckToStopRunningSpeak \n");
             if (displayId)
                mControlRequestQueueDisplay2.addRequest(request, displayId);
             else
                mControlRequestQueueDisplay1.addRequest(request, displayId);
        }
        else
        {
            delete request;
            request = nullptr;
        }
	if (displayId)
            mSpeakRequestQueueDisplay2.removeRequest(stopAppID,stopMsgID, displayId);
        else
            mSpeakRequestQueueDisplay1.removeRequest(stopAppID,stopMsgID, displayId);
    }
    else if (request->getType() == GET_LANGUAGES )
    {
        mEngineHandler->getLanguages(request, displayId);
    }
    else if (request->getType() == GET_STATUS )
    {
        LOG_DEBUG("Request Type: %d", request->getType());
        mEngineHandler->getStatusInfo(request, displayId);
    }
    else
    {
        LOG_DEBUG("Request Type: %d", request->getType());
        if (displayId)
            mControlRequestQueueDisplay2.addRequest(request, displayId);
        else
            mControlRequestQueueDisplay1.addRequest(request, displayId);
    }

    return true;
}

void RequestHandler::start(int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (displayId)
        mControlRequestQueueDisplay2.start(displayId);
    else
        mControlRequestQueueDisplay1.start(displayId);

    if (displayId)
        mSpeakRequestQueueDisplay2.start(displayId);
    else
        mSpeakRequestQueueDisplay1.start(displayId);
}

void RequestHandler::stop(int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (displayId)
        mSpeakRequestQueueDisplay2.stop(displayId);
    else
        mSpeakRequestQueueDisplay1.stop(displayId);

    if (displayId)
        mControlRequestQueueDisplay2.stop(displayId);
    else
        mControlRequestQueueDisplay1.stop(displayId);
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

void RequestHandler::stopSpeech(int displayId)
{
    LOG_DEBUG("RequestHandler::stopSpeechRequest  DisplayId = %d", displayId);
    StopRequest *stopRequest = new (std::nothrow)StopRequest;
    if(stopRequest == nullptr){
        return;
    }
    stopRequest->displayId = displayId;
    TTSRequest* request =new (std::nothrow)TTSRequest(reinterpret_cast<RequestType*>(stopRequest), mEngineHandler);
    if(request == nullptr){
        delete stopRequest;
        return;
    }
    if(displayId)
        mControlRequestQueueDisplay2.addRequest(request, displayId);
    else
       mControlRequestQueueDisplay1.addRequest(request, displayId);
}
