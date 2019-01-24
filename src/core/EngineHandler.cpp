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
        : mTTSEngine(nullptr),mRunningTTSRequest(nullptr)
{
    mCurrentLanguage = "en-US";
    meTTSTaskStatus = TTS_TASK_NOT_READY;
    loadEngine();
}

EngineHandler::~EngineHandler()
{
    unloadEngine();
}

bool EngineHandler::handleRequest(TTSRequest* request)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if(!mTTSEngine ||
       !mAudioEngine)
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

        RequestType* pRequestType = request->getRequest();
        SpeakRequest* pSpeakRequest = reinterpret_cast<SpeakRequest*>(pRequestType);

        pSpeakRequest->msgParameters->eTaskStatus = TTS_TASK_READY;
        mRunningTTSRequest = request;
        meTTSTaskStatus = TTS_TASK_READY;
        mCurrentLanguage = pSpeakRequest->msgParameters->sLangStr;
        pSpeakRequest->msgParameters->eStatus = TTS_MSG_PLAY;

        ttsRet = mTTSEngine->speak(pSpeakRequest->text_to_speak, pSpeakRequest->sh, pSpeakRequest->msgParameters->sLangStr);
        if(ttsRet == TTSErrors::ERROR_NONE)
        {
            audioRet = mAudioEngine->play();
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

        pSpeakRequest->replyCB(pSpeakRequest->msgParameters, pSpeakRequest->message);

       mRunningTTSRequest = nullptr;
    }
    else if(request->getType() == STOP)
    {
        LOG_DEBUG("Handling Stop Request\n");
        mTTSEngine->stop();
        mAudioEngine->stop();
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
    TTSEngineFactory::createTTSEngine(mTTSEngineName.asString(), mTTSEngine);
    if(!mTTSEngine)
    {
        LOG_DEBUG("TTSEngine %s Not Found", mTTSEngineName.asString().c_str());
        return;
    }
    LOG_DEBUG("TTSEngine %s Created", mTTSEngineName.asString().c_str());
    AudioEngineFactory::createAudioEngine(mAudioEngineName.asString(), mAudioEngine);
    if(!mAudioEngine)
    {
        LOG_DEBUG("AudioEngine %s Not Found", mAudioEngineName.asString().c_str());
        return;
    }
    LOG_DEBUG("AudioEngine %s Created", mAudioEngineName.asString().c_str());

    mTTSEngine->init();
}

void EngineHandler::unloadEngine()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (mTTSEngine) {
            mTTSEngine.reset();
    }
    if(mConfigHandler){
        delete mConfigHandler;
        mConfigHandler = nullptr;
    }
}

TTSRequest* EngineHandler::getRunningSpeakRequest()
{
    return mRunningTTSRequest;
}

void EngineHandler::getStatusInfo(TTSRequest* pTTSRequest)
{
    RequestType* pReqType =  pTTSRequest->getRequest();
    GetStatusRequest* pgetStatusRequest = reinterpret_cast<GetStatusRequest*>(pReqType);
    pgetStatusRequest->pTTSStatus->status =  GET_TASK_STATUS_TEXT(meTTSTaskStatus);
    pgetStatusRequest->pTTSStatus->ttsLanguageStr = mCurrentLanguage;
}

void EngineHandler::getLanguages(TTSRequest* pTTSRequest)
{
     RequestType* ptrRequestType = pTTSRequest->getRequest();
     GetLanguageRequest* ptrGetLanguageRequest = reinterpret_cast<GetLanguageRequest*>(ptrRequestType);
     mTTSEngine->getSupportedLanguages(ptrGetLanguageRequest->vecLanguages);
}
