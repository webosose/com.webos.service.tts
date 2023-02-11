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

#include <iostream>
#include <stdint.h>
#include <AudioEngineFactory.h>
#include <EngineHandler.h>
#include <TTSEngineFactory.h>
#include <TTSErrors.h>
#include <TTSLog.h>
#include <TTSLunaUtils.h>
#include <TTSRequest.h>
#include <StatusHandler.h>

EngineHandler::EngineHandler()
{
    mCurrentLanguage = "en-US";
    meTTSTaskStatus = TTS_TASK_NOT_READY;
    mRunningTTSRequest[DISPLAY_0] = nullptr;
    mRunningTTSRequest[DISPLAY_1] = nullptr;
    loadEngine();
}

EngineHandler::~EngineHandler()
{
    unloadEngine();
}

bool EngineHandler::handleRequest(TTSRequest* request, unsigned int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if(!mTTSEngine[displayId] ||
       !mAudioEngine[displayId])
    {
        meTTSTaskStatus = TTS_TASK_ERROR;
        LOG_DEBUG("Engine Not Created\n");
        return false;
    }

    if(request->getType() == SPEAK)
    {
        LOG_DEBUG("Handling Speak Request\n");

        int ttsRet = false;
        bool audioRet = false;
        unsigned int displayID = 0;

        RequestType* pRequestType = request->getRequest();
        SpeakRequest* pSpeakRequest = reinterpret_cast<SpeakRequest*>(pRequestType);

        pSpeakRequest->msgParameters->eTaskStatus = TTS_TASK_READY;
        mRunningTTSRequest[displayId] = request;
        meTTSTaskStatus = TTS_TASK_READY;
        mCurrentLanguage = pSpeakRequest->msgParameters->sLangStr;
        displayID = pSpeakRequest->msgParameters->displayId;

        pSpeakRequest->msgParameters->eStatus = TTS_MSG_PLAY;

        LOG_DEBUG("Handling Speak Request : displayId = displayID : %zu\n", displayID);
        ttsRet = mTTSEngine[displayID]->speak(pSpeakRequest->text_to_speak, pSpeakRequest->sh, pSpeakRequest->msgParameters->sLangStr, displayID);
        if(ttsRet == TTSErrors::ERROR_NONE)
        {
            LOG_DEBUG("Handling Speak Request : AudioEngine Play : displayID = %zu\n", displayID);
            audioRet = mAudioEngine[displayID]->play(displayID);
        }
        else if(ttsRet == TTSErrors::LANG_NOT_SUPPORTED)
        {
            LOG_DEBUG("Language Not Supported\n");
            pSpeakRequest->msgParameters->eTaskStatus = TTS_TASK_ERROR;
            pSpeakRequest->msgParameters->eLang = LANG_ERR;
            meTTSTaskStatus = TTS_TASK_ERROR;
        }

        if(audioRet)
        {
            if( TTS_MSG_PLAY ==  pSpeakRequest->msgParameters->eStatus )
               pSpeakRequest->msgParameters->eStatus = TTS_MSG_DONE;
            meTTSTaskStatus = TTS_TASK_DONE;
        }
        else
        {
            meTTSTaskStatus = TTS_TASK_ERROR;
            LOG_DEBUG("Play Error\n");
            pSpeakRequest->msgParameters->eTaskStatus = TTS_TASK_ERROR;
            pSpeakRequest->msgParameters->eStatus = TTS_MSG_ERROR;
        }
        if(pSpeakRequest->msgParameters->bSubscribed)
            pSpeakRequest->replyCB(pSpeakRequest->msgParameters, pSpeakRequest->message);

       mRunningTTSRequest[displayId] = nullptr;
    }
    else if(request->getType() == STOP)
    {
        LOG_DEBUG("Handling Stop Request\n");
        unsigned int displayID = 0;
        if(nullptr != mRunningTTSRequest[displayId])
        {
            SpeakRequest* pRunningSpeakRequest = reinterpret_cast<SpeakRequest*>(mRunningTTSRequest[displayId]->getRequest());
            displayID = pRunningSpeakRequest->msgParameters->displayId;
            if (displayID == displayId)
            {
                LOG_DEBUG("displayID && displayId are same = %zu\n", displayId);
                pRunningSpeakRequest->msgParameters->eStatus = TTS_MSG_STOP;
                (void)mTTSEngine[displayID]->stop(displayId);
                (void)mAudioEngine[displayID] ->stop(displayId);
            }
        }
    }
    return true;
}

void EngineHandler::loadEngine()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    TTSConfigError err = TTSErrors::TTS_CONFIG_ERROR_NONE;

    mConfigHandler = new (std::nothrow)TTSConfig;
    if(mConfigHandler == nullptr){
        LOG_ERROR(MSGID_TTS_ERROR, 0, "Memory Allocation Failed TTSConfig");
        return;
    }
    err = mConfigHandler->readFile();

    if (err != TTSErrors::TTS_CONFIG_ERROR_NONE)
    {
        LOG_DEBUG("File Read failed: %d ", err);
        return;
    }
    err = mConfigHandler->getValue("engine", "tts_engine", mTTSEngineName);
    if(err != TTSErrors::TTS_CONFIG_ERROR_NONE)
    {
        LOG_DEBUG("Error In Reading Config for tts_engine : %d ", err);
        return;
    }
    err = mConfigHandler->getValue("engine", "audio_engine", mAudioEngineName);
    if(err != TTSErrors::TTS_CONFIG_ERROR_NONE)
    {
        LOG_DEBUG("Error In Reading Config for audio_engine : %d ", err);
        return;
    }
    err = mConfigHandler->getValue("engine", "displayCount", mDisplayCount);
    if(err != TTSErrors::TTS_CONFIG_ERROR_NONE)
    {
        LOG_DEBUG("Error In Reading Config for displayCount: %d ", err);
        return;
    }
    if (mDisplayCount.isNumber())
    {
        int displayCount = mDisplayCount.asNumber<int>();
        LOG_DEBUG("displayCount =  %d :", displayCount);
        for (unsigned int displayID = 0; displayID < displayCount; displayID++)
        {
            mTTSEngine[displayID] = TTSEngineFactory::createTTSEngine(mTTSEngineName.asString());
            if(!mTTSEngine[displayID])
            {
                LOG_DEBUG("TTSEngine %s %zu Not Found", mTTSEngineName.asString().c_str(), displayID);
                return;
            }
            LOG_DEBUG("TTSEngine %s %zu Created", mTTSEngineName.asString().c_str(), displayID);
            mAudioEngine[displayID] = AudioEngineFactory::createAudioEngine(mAudioEngineName.asString());
            if(!mAudioEngine[displayID])
            {
                LOG_DEBUG("AudioEngine %s %zu Not Found", mAudioEngineName.asString().c_str(), displayID);
                return;
            }
            LOG_DEBUG("AudioEngine %s %zu Created", mAudioEngineName.asString().c_str(), displayID);

            // TODO: Decide if init required
            mTTSEngine[displayID]->init();
        }
    }
}

void EngineHandler::unloadEngine()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if(mConfigHandler){
        delete mConfigHandler;
        mConfigHandler = nullptr;
    }
}

TTSRequest* EngineHandler::getRunningSpeakRequest(unsigned int displayId)
{
    return mRunningTTSRequest[displayId];
}

void EngineHandler::getStatusInfo(TTSRequest* pTTSRequest, unsigned int displayId)
{
    RequestType* pReqType =  pTTSRequest->getRequest();
    GetStatusRequest* pgetStatusRequest = reinterpret_cast<GetStatusRequest*>(pReqType);
    pgetStatusRequest->pTTSStatus->status =  GET_TASK_STATUS_TEXT(meTTSTaskStatus);
    pgetStatusRequest->pTTSStatus->ttsLanguageStr = mCurrentLanguage;

    pgetStatusRequest->pTTSStatus->pitch = mTTSEngine[displayId]->getPitch();
    pgetStatusRequest->pTTSStatus->speechRate = mTTSEngine[displayId]->getSpeakRate();
}

void EngineHandler::getLanguages(TTSRequest* pTTSRequest, unsigned int displayId)
{
     RequestType* ptrRequestType = pTTSRequest->getRequest();
     GetLanguageRequest* ptrGetLanguageRequest = reinterpret_cast<GetLanguageRequest*>(ptrRequestType);
     mTTSEngine[displayId]->getSupportedLanguages(ptrGetLanguageRequest->vecLanguages, displayId);
}
