// Copyright (c) 2018-2023 LG Electronics, Inc.
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

RequestHandler::RequestHandler(std::shared_ptr<EngineHandler> engineHandler) :
        mSpeakRequestQueueDisplay1("QUEUE_SPEAK1"), mSpeakRequestQueueDisplay2(
                "QUEUE_SPEAK2"), mControlRequestQueueDisplay1(
                "QUEUE_CONTROL_1"), mControlRequestQueueDisplay2(
                "QUEUE_CONTROL_2"), mEngineHandler(engineHandler) {
}

bool RequestHandler::sendRequest(TTSRequest* request, unsigned int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    LOG_DEBUG("Request Type: %d", request->getType());

    if (request->getType() == SPEAK)
    {
        SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(request->getRequest());
        if(ptrSpeakRequest->msgParameters->bClear)
        {
          LOG_DEBUG("RequestHandler: DisplayId = %zu", displayId);
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
              mSpeakRequestQueueDisplay2.clearQueue();
          else
              mSpeakRequestQueueDisplay1.clearQueue();
        }
        if (displayId)
            mSpeakRequestQueueDisplay2.addRequest(request);
        else
            mSpeakRequestQueueDisplay1.addRequest(request);
    }
    else if (request->getType() == STOP )
    {
        StopRequest* ptrStopRequest = reinterpret_cast<StopRequest*>(request->getRequest());
        std::string stopAppID = ptrStopRequest->sAppID;
        std::string stopMsgID = ptrStopRequest->sMsgID;
        bool queueRet = false;
        bool runningRet = false;
        LOG_DEBUG("RequestHandler::sendRequest: Before CheckToStopRunningSpeak \n");

        if( CheckToStopRunningSpeak(mEngineHandler->getRunningSpeakRequest(displayId),request ) )
        {
             LOG_DEBUG("RequestHandler::sendRequest: After CheckToStopRunningSpeak \n");
             if (displayId)
                mControlRequestQueueDisplay2.addRequest(request);
             else
                mControlRequestQueueDisplay1.addRequest(request);
             runningRet = true;
        }
        else
        {
            delete request;
            request = nullptr;
        }
	if (displayId)
            queueRet = mSpeakRequestQueueDisplay2.removeRequest(stopAppID,stopMsgID);
        else
            queueRet = mSpeakRequestQueueDisplay1.removeRequest(stopAppID,stopMsgID);

        return (runningRet) ? runningRet : queueRet;
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
            mControlRequestQueueDisplay2.addRequest(request);
        else
            mControlRequestQueueDisplay1.addRequest(request);
    }

    return true;
}

void RequestHandler::start()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    mControlRequestQueueDisplay1.start();
    mSpeakRequestQueueDisplay1.start();
    mControlRequestQueueDisplay2.start();
    mSpeakRequestQueueDisplay2.start();
}

void RequestHandler::stop()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    mSpeakRequestQueueDisplay1.stop();
    mControlRequestQueueDisplay1.stop();
    mSpeakRequestQueueDisplay2.stop();
    mControlRequestQueueDisplay2.stop();
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

void RequestHandler::stopSpeech(unsigned int displayId)
{
    LOG_DEBUG("RequestHandler::stopSpeechRequest  DisplayId = %zu", displayId);
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
        mControlRequestQueueDisplay2.addRequest(request);
    else
       mControlRequestQueueDisplay1.addRequest(request);
}
