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
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s", __FUNCTION__);
    mCurrentLanguage[DISPLAY_0] = "en-US";
    mCurrentLanguage[DISPLAY_1] = "en-US";
    meTTSTaskStatus[DISPLAY_0] = TTS_TASK_NOT_READY;
    meTTSTaskStatus[DISPLAY_1] = TTS_TASK_NOT_READY;
    loadEngine();
}

EngineHandler::~EngineHandler()
{
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s", __FUNCTION__);
    unloadEngine();
}

bool EngineHandler::handleRequest(TTSRequest* request, unsigned int displayId)
{
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s type: %d disp: %u", __FUNCTION__,
            request->getType(), displayId);

    if (!mTTSEngine[displayId] || !mAudioEngine[displayId]) {
        meTTSTaskStatus[displayId] = TTS_TASK_ERROR;
        LOG_INFO(MSGID_ENGINE_HANDLER, 0, "Engine/(s) Not Created %s",
                __FUNCTION__);
        return false;
    }

    if (request->getType() == SPEAK) {
        int ttsRet = false;
        bool audioRet = false;
        unsigned int displayID = 0;

        RequestType *pRequestType = request->getRequest();
        SpeakRequest *pSpeakRequest =
                reinterpret_cast<SpeakRequest*>(pRequestType);

        pSpeakRequest->msgParameters->eTaskStatus = TTS_TASK_READY;
        saveSpeakRequestInfo(pSpeakRequest, displayId);
        meTTSTaskStatus[displayId] = TTS_TASK_READY;
        mCurrentLanguage[displayId] = pSpeakRequest->msgParameters->sLangStr;
        displayID = pSpeakRequest->msgParameters->displayId;

        pSpeakRequest->msgParameters->eStatus = TTS_MSG_PLAY;

        LOG_INFO(MSGID_ENGINE_HANDLER, 0,
                "Delegate Speak Request to speech engine on display: %u",
                displayID);
        ttsRet = mTTSEngine[displayID]->speak(pSpeakRequest->text_to_speak,
                pSpeakRequest->sh, pSpeakRequest->msgParameters->sLangStr,
                displayID);
        if (ttsRet == TTSErrors::ERROR_NONE) {
            LOG_INFO(MSGID_ENGINE_HANDLER, 0,
                    "Play speak request on audio engine on display: %u",
                    displayID);
            audioRet = mAudioEngine[displayID]->play(displayID);
        } else if (ttsRet == TTSErrors::LANG_NOT_SUPPORTED) {
            LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s Language Not Supported",
                    __FUNCTION__);
            pSpeakRequest->msgParameters->eTaskStatus = TTS_TASK_ERROR;
            pSpeakRequest->msgParameters->eLang = LANG_ERR;
            meTTSTaskStatus[displayId] = TTS_TASK_ERROR;
        }

        if (audioRet) {
            if (TTS_MSG_PLAY == pSpeakRequest->msgParameters->eStatus)
                pSpeakRequest->msgParameters->eStatus = TTS_MSG_DONE;
            meTTSTaskStatus[displayId] = TTS_TASK_DONE;
        } else {
            meTTSTaskStatus[displayId] = TTS_TASK_ERROR;
            LOG_INFO(MSGID_ENGINE_HANDLER, 0, "Play Error %s", __FUNCTION__);
            pSpeakRequest->msgParameters->eTaskStatus = TTS_TASK_ERROR;
            pSpeakRequest->msgParameters->eStatus = TTS_MSG_ERROR;
        }

        {
            std::lock_guard < std::mutex > lck(mRunningInfoMutex[displayID]);
            auto found = mSpeakRequestInfoMap.find(displayId);
            if (found != mSpeakRequestInfoMap.end()) {
                if (found->second.msgStatus == TTS_MSG_STOP)
                    pSpeakRequest->msgParameters->eStatus = TTS_MSG_STOP;
            }
        }

        if (pSpeakRequest->msgParameters->bSubscribed) {
            LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s notify speak update",
                    __FUNCTION__);
            pSpeakRequest->replyCB(pSpeakRequest->msgParameters,
                    pSpeakRequest->message);
        }

        removeSpeakRequestInfo(displayID);
    } else if (request->getType() == STOP) {
        SpeakRequestInfo info;
        bool speakRequestFound = getSpeakRequestInfo(displayId, info);
        if (speakRequestFound) {
            LOG_INFO(MSGID_ENGINE_HANDLER, 0,
                    "Stop running speak request on display: %u", displayId);
            updateSpeakRequestInfo(displayId, TTS_MSG_STOP);
            (void) mTTSEngine[displayId]->stop(displayId);
            (void) mAudioEngine[displayId]->stop(displayId);
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
                LOG_DEBUG("TTSEngine %s %u Not Found", mTTSEngineName.asString().c_str(), displayID);
                return;
            }
            LOG_DEBUG("TTSEngine %s %u Created", mTTSEngineName.asString().c_str(), displayID);
            mAudioEngine[displayID] = AudioEngineFactory::createAudioEngine(mAudioEngineName.asString());
            if(!mAudioEngine[displayID])
            {
                LOG_DEBUG("AudioEngine %s %u Not Found", mAudioEngineName.asString().c_str(), displayID);
                return;
            }
            LOG_DEBUG("AudioEngine %s %u Created", mAudioEngineName.asString().c_str(), displayID);

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

void EngineHandler::getStatusInfo(TTSRequest* pTTSRequest, unsigned int displayId)
{
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s", __FUNCTION__);
    RequestType* pReqType =  pTTSRequest->getRequest();
    GetStatusRequest* pgetStatusRequest = reinterpret_cast<GetStatusRequest*>(pReqType);
    pgetStatusRequest->pTTSStatus->status =  GET_TASK_STATUS_TEXT(meTTSTaskStatus[displayId]);
    pgetStatusRequest->pTTSStatus->ttsLanguageStr = mCurrentLanguage[displayId];

    pgetStatusRequest->pTTSStatus->pitch = mTTSEngine[displayId]->getPitch();
    pgetStatusRequest->pTTSStatus->speechRate = mTTSEngine[displayId]->getSpeakRate();
}

void EngineHandler::getLanguages(TTSRequest* pTTSRequest, unsigned int displayId)
{
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s", __FUNCTION__);
     RequestType* ptrRequestType = pTTSRequest->getRequest();
     GetLanguageRequest* ptrGetLanguageRequest = reinterpret_cast<GetLanguageRequest*>(ptrRequestType);
     mTTSEngine[displayId]->getSupportedLanguages(ptrGetLanguageRequest->vecLanguages, displayId);
}

void EngineHandler::saveSpeakRequestInfo(SpeakRequest* request,
        unsigned int displayId) {
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s", __FUNCTION__);
    std::lock_guard<std::mutex> lck (mRunningInfoMutex[displayId]);
    SpeakRequestInfo info;
    info.displayId = displayId;
    info.appId = request->msgParameters->sAppID;
    info.msgId = request->msgParameters->sMsgID;
    info.msgStatus = TTS_MSG_PLAY;
    mSpeakRequestInfoMap.insert({ displayId, info });
}
bool EngineHandler::getSpeakRequestInfo(unsigned int displayId, SpeakRequestInfo& info) {
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s", __FUNCTION__);
    std::lock_guard<std::mutex> lck (mRunningInfoMutex[displayId]);
    auto found = mSpeakRequestInfoMap.find(displayId);
    if(found != mSpeakRequestInfoMap.end()) {
        info.displayId = found->second.displayId;
        info.appId = found->second.appId;
        info.msgId = found->second.msgId;
        info.msgStatus = found->second.msgStatus;
        LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s return found request", __FUNCTION__);
        return true;
    }
    return false;

}
void EngineHandler::updateSpeakRequestInfo(unsigned int displayId,
        MsgStatus_t msgStatus) {
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s", __FUNCTION__);
    std::lock_guard < std::mutex > lck(mRunningInfoMutex[displayId]);
    auto found = mSpeakRequestInfoMap.find(displayId);
    if (found != mSpeakRequestInfoMap.end()) {
        found->second.msgStatus = msgStatus;
    }
}

void EngineHandler::removeSpeakRequestInfo(unsigned int displayId) {
    LOG_INFO(MSGID_ENGINE_HANDLER, 0, "%s", __FUNCTION__);
    std::lock_guard < std::mutex > lck(mRunningInfoMutex[displayId]);
    auto found = mSpeakRequestInfoMap.find(displayId);
    if (found != mSpeakRequestInfoMap.end()) {
        mSpeakRequestInfoMap.erase(found);
    }
}
