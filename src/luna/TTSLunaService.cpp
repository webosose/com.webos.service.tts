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

#include <string>
#include <TTSErrors.h>
#include <TTSLog.h>
#include <TTSLunaService.h>
#include <TTSRequestTypes.h>
#include <StatusHandler.h>

#define GET_SYSTEM_SETTINGS  "luna://com.webos.service.settings/getSystemSettings"
#define GET_VOLUME  "luna://com.webos.service.audio/master/getVolume"

const std::string service_name = "com.webos.service.tts";
const double dTTSPitch = 0.0;
const double dTTSSpeechRate = 0.0;

LSHandle* TTSLunaService::lsHandle = nullptr;

TTSLunaService::TTSLunaService(RequestHandler* requestHandler, std::shared_ptr<EngineHandler> engineHandler)
        : LS::Handle(LS::registerService(service_name.c_str())), mRequestHandler(requestHandler), mEngineHandler(engineHandler), mParamListManager(nullptr)
{
    TTSLunaService::lsHandle = this->get();
    registerService();
}

TTSLunaService::~TTSLunaService()
{
    StatusHandler::GetInstance()->Unregister(this);
}

void TTSLunaService::registerService()
{
    LOG_DEBUG("Register Service\n");
    LS_CREATE_CATEGORY_BEGIN(TTSLunaService, rootAPI)
    LS_CATEGORY_METHOD(speak)
    LS_CATEGORY_METHOD(speakVKB)
    LS_CATEGORY_METHOD(stop)
    LS_CATEGORY_METHOD(start)
    LS_CATEGORY_METHOD(setAudioGuidanceOnOff)
    LS_CATEGORY_METHOD(getAvailableLanguages)
    LS_CATEGORY_METHOD(getStatus)
    LS_CATEGORY_METHOD(msgFeedback)
    LS_CREATE_CATEGORY_END

    try {
        this->registerCategory("/", LS_CATEGORY_TABLE_NAME(rootAPI), nullptr, nullptr);
        this->setCategoryData("/", this);
    } catch (LS::Error &lunaError) {
        //LOG_ERROR(MSGID_LUNA_ERROR_RESPONSE, "%s", lunaError.get()->message);
    }
    StatusHandler::GetInstance()->Register(this);
}

bool TTSLunaService::speak(LSMessage &message)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LS::Message request(&message);
    pbnjson::JValue requestObj;
    pbnjson::JValue responseObj = pbnjson::Object();

    int parseError = 0;
    bool retVal = false;

    const std::string schema = STRICT_SCHEMA(PROPS_6(PROP(text, string), PROP(clear, boolean), PROP(subscribe, boolean), PROP(appID, string),
                                                                  PROP(feedback, boolean), PROP(language, string))REQUIRED_1(text));

    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError))
    {
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::INVALID_JSON_FORMAT);
        LSUtils::respondWithError(request, errorStr, TTSErrors::INVALID_JSON_FORMAT);
        return true;
    }

    SpeakRequest *speakRequest = new (std::nothrow)SpeakRequest;
    if(speakRequest == nullptr){
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_MEMORY_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_MEMORY_ERROR);
        LOG_ERROR(MSGID_TTS_MEMORY_ERROR, 0, "Failed To Allocatememory for SpeakRequest");
        return true;
    }
    requestObj["text"].asString(speakRequest->text_to_speak);
    speakRequest->sh = this->get();
    speakRequest->replyCB = responseCallback;
    speakRequest->message = request;

    if ((speakRequest->text_to_speak).empty())
    {
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::INPUT_TEXT_EMPTY);
        LSUtils::respondWithError(request, errorStr, TTSErrors::INPUT_TEXT_EMPTY);
        delete speakRequest;
        speakRequest = nullptr;
        return true;
    }

    addParameters(message);
    speakRequest->msgParameters = mParameterList;

    if(mParameterList->bSubscribed)
    {
        bool ret = addSubscription(lsHandle, &message, speakRequest->msgParameters->sMsgID);
    }

    TTSRequest* ttsRequest = new (std::nothrow) TTSRequest(reinterpret_cast<RequestType*>(speakRequest), mEngineHandler);
    retVal = mRequestHandler->sendRequest(ttsRequest);

    if(retVal)
    {
        LOG_DEBUG("Speak Request Complete\n");
        responseObj.put("language", mParameterList->sLangStr);
        responseObj.put("returnValue", true);
        if(mParameterList->bSubscribed)
            responseObj.put("subscribed", true);
        else
            responseObj.put("subscribed", false);
        if(mParameterList->bFeedback == true || mParameterList->bSubscribed == true)
            responseObj.put("msgID", mParameterList->sMsgID);
        LSUtils::postToClient(request, responseObj);
    }
    else
    {
        LOG_DEBUG("Speak Request Not Sent\n");
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::SPEECH_DATA_CREATION_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::SPEECH_DATA_CREATION_ERROR);
    }

    return true;
}

bool TTSLunaService::speakVKB(LSMessage &message)
{
    LS::Message request(&message);
    const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_ERROR_NOT_SUPPORTED);
    LSUtils::respondWithError(request, errorStr, TTSErrorCodes::TTS_ERROR_NOT_SUPPORTED);
    return true;
}

bool TTSLunaService::start(LSMessage &message)
{
    LS::Message request(&message);
    std::string payload;

    pbnjson::JValue responseObj = pbnjson::Object();
    responseObj.put("returnValue", true);

    LSUtils::generatePayload(responseObj, payload);
    request.respond(payload.c_str());
    return true;
}

bool TTSLunaService::stop(LSMessage &message)
{
   LS::Message request(&message);
   std::string payload;
   bool retVal = false;
   pbnjson::JValue requestObj;
   int parseError = 0;
   const std::string schema = STRICT_SCHEMA(PROPS_3(PROP(msgID, string),PROP(appID, string),
                                                                          PROP(fadeOut, boolean)));
   if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError))
   {
      const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::INVALID_JSON_FORMAT);
      LSUtils::respondWithError(request, errorStr, TTSErrors::INVALID_JSON_FORMAT);
      return true;
   }
   std::string applicationID = requestObj["appID"].asString();
   std::string MessageID = requestObj["msgID"].asString();
   bool bfadeOut = requestObj["fadeOut"].asBool();
   StopRequest *stopRequest = new (std::nothrow) StopRequest;
   if(stopRequest == nullptr){
       return true;
   }
   stopRequest->sAppID = applicationID;
   stopRequest->sMsgID = MessageID;
   stopRequest->fadeOut = bfadeOut;
   TTSRequest* ttsRequest = new (std::nothrow) TTSRequest(reinterpret_cast<RequestType*>(stopRequest), mEngineHandler);
   if(ttsRequest == nullptr){
       delete stopRequest;
       stopRequest = nullptr;
       return true;
   }
   retVal = mRequestHandler->sendRequest(ttsRequest);
   if(retVal)
   {
      LOG_DEBUG("Stop Request Complete\n");
   }
   else
   {
      LOG_DEBUG("Stop Request Not Sent\n");
      const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_INTERNAL_ERROR);
      LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_INTERNAL_ERROR);
      return true;
   }

    pbnjson::JValue responseObj = pbnjson::Object();
    responseObj.put("returnValue", true);

    LSUtils::generatePayload(responseObj, payload);
    request.respond(payload.c_str());
    return true;
}

bool TTSLunaService::setAudioGuidanceOnOff(LSMessage &message)
{
    LS::Message request(&message);
    const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_ERROR_NOT_SUPPORTED);
    LSUtils::respondWithError(request, errorStr, TTSErrorCodes::TTS_ERROR_NOT_SUPPORTED);
    return true;
}

bool TTSLunaService::getAvailableLanguages(LSMessage &message)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LS::Message request(&message);
    std::string payload;
    bool retVal = false;

    GetLanguageRequest* ptrGetLanguageRequest = new (std::nothrow)GetLanguageRequest();
    if(ptrGetLanguageRequest == nullptr){
        LOG_ERROR(MSGID_TTS_MEMORY_ERROR, 0, "Memory Allocation Error In GetLanguageRequest");
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_MEMORY_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_MEMORY_ERROR);
        return true;
    }
    TTSRequest* ttsRequest = new (std::nothrow)TTSRequest(reinterpret_cast<RequestType*>(ptrGetLanguageRequest), mEngineHandler);
    if(ttsRequest == nullptr){
        delete ptrGetLanguageRequest;
        LOG_ERROR(MSGID_TTS_MEMORY_ERROR, 0, "Memory Allocation Error In GetLanguageRequest");
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_MEMORY_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_MEMORY_ERROR);
        return true;
    }
    retVal = mRequestHandler->sendRequest(ttsRequest);
    if(retVal)
    {
        LOG_DEBUG("Get Available Languages Request Complete\n");
    }
    else
    {
        LOG_DEBUG("Get Available Languages Request Not Sent\n");
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_INTERNAL_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_INTERNAL_ERROR);
        delete ttsRequest;
        ttsRequest = nullptr;
        return true;
    }

    pbnjson::JArray LanguagesJArray = pbnjson::JArray();

    std::vector<std::string>::iterator it = ptrGetLanguageRequest->vecLanguages.begin();
    while(it != ptrGetLanguageRequest->vecLanguages.end())
    {
       LanguagesJArray << pbnjson::JValue(*it);
       ++it;
    }

    pbnjson::JValue responseObj = pbnjson::Object();
    responseObj.put("Languages", LanguagesJArray);
    responseObj.put("returnValue", true);

    LSUtils::generatePayload(responseObj, payload);
    request.respond(payload.c_str());
    delete ttsRequest;
    ttsRequest = nullptr;
    return true;
}

bool TTSLunaService::getStatus(LSMessage &message)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LS::Message request(&message);
    pbnjson::JValue requestObj;
    int parseError = 0;
    bool retVal = false;
    std::string payload;

    const std::string schema = SCHEMA_ANY;

    if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError))
    {
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::INVALID_JSON_FORMAT);
        LSUtils::respondWithError(request, errorStr, TTSErrors::INVALID_JSON_FORMAT);
        return true;
    }
    GetStatusRequest* getStatusRequest = new (std::nothrow)GetStatusRequest;
    if(getStatusRequest == nullptr){
        LOG_ERROR(MSGID_TTS_MEMORY_ERROR, 0, "Memory Allocation Error In GetStatusRequest");
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_MEMORY_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_MEMORY_ERROR);
        return true;
    }
    getStatusRequest->pTTSStatus = new (std::nothrow)TTSStatus;
    if(getStatusRequest->pTTSStatus == nullptr){
        delete getStatusRequest;
        LOG_ERROR(MSGID_TTS_MEMORY_ERROR, 0, "Memory Allocation Error In GetStatusRequest");
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_MEMORY_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_MEMORY_ERROR);
        return true;
    }
    getStatusRequest->sh = this->get();
    getStatusRequest->replyCB = statusResponse;
    getStatusRequest->message = request;

    TTSRequest* ptrTTSRequest = new (std::nothrow)TTSRequest(reinterpret_cast<RequestType*>(getStatusRequest), mEngineHandler);
    if(ptrTTSRequest == nullptr){
        delete getStatusRequest->pTTSStatus;
        getStatusRequest->pTTSStatus=nullptr;
        delete getStatusRequest;
        getStatusRequest=nullptr;
        LOG_ERROR(MSGID_TTS_MEMORY_ERROR,0, "Memory Allocation Error In GetStatusRequest");
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_MEMORY_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_MEMORY_ERROR);
        return true;
    }
    retVal = mRequestHandler->sendRequest(ptrTTSRequest);

    if(retVal)
    {
        LOG_DEBUG("Get Status Request Complete\n");
    }
    else
    {
        LOG_DEBUG("Get Status Request Not Sent\n");
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_INTERNAL_ERROR);
        LSUtils::respondWithError(request, errorStr, TTSErrors::TTS_INTERNAL_ERROR);
    }

    getStatusRequest->pTTSStatus->pitch = dTTSPitch;
    getStatusRequest->pTTSStatus->speechRate = dTTSSpeechRate;


    LSError lserror;
    LSErrorInit(&lserror);
    if (!LSCallOneReply( TTSLunaService::lsHandle,
               GET_VOLUME,
               R"({})",
               handle_getVolume_callback, getStatusRequest, nullptr ,&lserror))
    {
               LOG_ERROR(MSGID_ERROR_CALL, 0, lserror.message);
               LSErrorFree(&lserror);
    }
    if (!LSCallOneReply( TTSLunaService::lsHandle,
               GET_SYSTEM_SETTINGS,
               R"({"category": "option","keys": ["menuLanguage"]})",
               handle_getSettings_callback, getStatusRequest, nullptr , &lserror))
    {
               LOG_ERROR(MSGID_ERROR_CALL, 0, lserror.message);
               LSErrorFree(&lserror);
    }
    delete ptrTTSRequest;
    ptrTTSRequest=nullptr;
    return true;
}

bool TTSLunaService::msgFeedback(LSMessage &message)
{
    LS::Message request(&message);
    const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::TTS_ERROR_NOT_SUPPORTED);
    LSUtils::respondWithError(request, errorStr, TTSErrorCodes::TTS_ERROR_NOT_SUPPORTED);
    return true;
}

void TTSLunaService::responseCallback(Parameters* paramList, LS::Message& message)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    bool feedback = false;
    bool subscribed = false;

    if(paramList->eLang == LANG_ERR)
    {
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::LANG_NOT_SUPPORTED);
        LSUtils::respondWithError(message, errorStr, TTSErrors::LANG_NOT_SUPPORTED);
        return;
    }

    pbnjson::JValue responseObj = pbnjson::Object();
    responseObj.put("returnValue", true);

    feedback = paramList->bFeedback;

    subscribed = paramList->bSubscribed;
    if( feedback == true || subscribed == true )
    {
       responseObj.put("msgID", paramList->sMsgID);
    }

    if(subscribed)
    {
        std::string messageStatus = GET_MSG_STATUS_TEXT(paramList->eStatus);
        responseObj.put("msgStatus", messageStatus);
        responseObj.put("subscribed", false);

        LSError lserror;
        LSErrorInit(&lserror);

        std::string payload;
        LSUtils::generatePayload(responseObj, payload);

        if (!LSSubscriptionReply(TTSLunaService::lsHandle, GET_MSG_STATUS_TEXT(paramList->eStatus).c_str(), payload.c_str(), &lserror)) {
            LSErrorPrint(&lserror, stderr);
            LSErrorFree(&lserror);
        }
    }

    LSUtils::postToClient(message, responseObj);

}

void TTSLunaService::addParameters(LSMessage &message)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LS::Message request(&message);
    pbnjson::JValue requestObj = pbnjson::Object();
    int lCount;

    LSUtils::parsePayload(request.getPayload(), requestObj);

    mParameterList = new (std::nothrow) Parameters;

    if(mParameterList != nullptr)
    {
        requestObj["text"].asString(mParameterList->sText);
        mParameterList->bFeedback = requestObj["feedback"].asBool();
        mParameterList->bSubscribed = LSMessageIsSubscription(&message);
        mParameterList->eStatus = TTS_MSG_ERROR;
        mParameterList->sAppID = requestObj["appID"].asString();
        mParameterList->eTaskStatus = TTS_TASK_NOT_READY;
        mParameterList->eLang = LANG_ENUS;

        if(mParameterList->bSubscribed || mParameterList->bFeedback)
        {
            mParameterList->sMsgID = LSUtils::generateRandomString(12);
        }

        mParameterList->sLangStr = "en-US";
        std::string sInputLang;
        sInputLang = requestObj["language"].asString();

        if(!sInputLang.empty())
        {
            for(lCount = 0; lCount < LANG_MAX; lCount++)
            {
                if(!strcmp(TTSLanguageTable[lCount].languageStr.c_str(), sInputLang.c_str()))
                {
                    mParameterList->eLang = TTSLanguageTable[lCount].ttsLanguage;
                    mParameterList->sLangStr = sInputLang;
                    break;
                }
            }

            if(lCount == LANG_MAX)
            {
                mParameterList->eLang = LANG_ERR;
                mParameterList->eStatus = TTS_MSG_ERROR;
                const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::LANG_NOT_SUPPORTED);
                LSUtils::respondWithError(request, errorStr, TTSErrors::LANG_NOT_SUPPORTED);
            }
        }

        mParameterList->bClear = requestObj["clear"].asBool();
    }
}

bool TTSLunaService::addSubscription(LSHandle *sh, LSMessage *message, std::string key)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if((sh == nullptr) ||
        (message == nullptr) ||
        (key == ""))
        return false;

    LSError lserror;
    LSErrorInit(&lserror);
    if(!LSSubscriptionAdd(sh, key.c_str(), message, &lserror))
    {
        LSErrorPrint(&lserror, stderr);
        LSErrorFree(&lserror);
        return false;
    }

    return true;
}
void TTSLunaService::update(Parameters* paramList, LS::Message &message)
{
    LOG_DEBUG(" TTSLunaService::update entry\n");
    responseCallback(paramList,message);
}

void TTSLunaService::statusResponse(TTSStatus* pTTSStatus, LS::Message& message)
{
    std::string payload;
    pbnjson::JValue responseObj = pbnjson::Object();
    pbnjson::JValue configJSON = pbnjson::Object();
    configJSON.put("pitch", pTTSStatus->pitch);
    configJSON.put("ttsCurrLang", pTTSStatus->ttsLanguageStr);
    configJSON.put("speech rate", pTTSStatus->speechRate);
    configJSON.put("status", pTTSStatus->status);
    configJSON.put("volume", pTTSStatus->volume);
    configJSON.put("ttsMenuLang", pTTSStatus->ttsMenuLangStr);

    responseObj.put("status", configJSON);
    responseObj.put("returnValue", true);

    LSUtils::generatePayload(responseObj, payload);
    message.respond(payload.c_str());
}

bool TTSLunaService::handle_getVolume_callback(LSHandle *sh, LSMessage *message, void *ctx)
{
    if (nullptr == ctx){
        return false;
    }

    LSMessageRef(message);
    int iVolume = 0;
    pbnjson::JValue root;
    const char *msgPayload;

    msgPayload = LSMessageGetPayload(message);
    if (!LSUtils::parsePayload(msgPayload, root)) {
        LSMessageUnref(message);
        return false;
    }
    GetStatusRequest* pgetStatusRequest = static_cast<GetStatusRequest *> (ctx);

    const pbnjson::JValue& jArray = root["volumeStatus"];
    if (!jArray.isArray()) {
        LSMessageUnref(message);
        return false;
    }

    for (pbnjson::JValue it : jArray.items())
    {
        pbnjson::JValue  jVolume = it["volume"];
        if(jVolume.isNumber())
            iVolume = jVolume.asNumber<int>();
    }

    pgetStatusRequest->pTTSStatus->volume = iVolume;
    LOG_DEBUG("handle_getVolume_callback volume = %d", iVolume);
    LSMessageUnref(message);
    return true;
}

bool TTSLunaService::handle_getSettings_callback(LSHandle *sh, LSMessage *message, void *ctx)
{
     if (nullptr == ctx){
        return false;
    }
    LSMessageRef(message);
    const char *msgPayload;
    msgPayload = LSMessageGetPayload(message);
    pbnjson::JValue root = pbnjson::JDomParser::fromString(msgPayload);
    if(!LSUtils::parsePayload(msgPayload, root)) {
        LSMessageUnref(message);
        return false;
    }
    GetStatusRequest* pgetStatusRequest = static_cast<GetStatusRequest *> (ctx);
    std::string MenuLanghStr;
    if(root["settings"].isObject())
    {
        const pbnjson::JValue& j_locale = root["settings"];
        if (j_locale.hasKey("menuLanguage")) {
            MenuLanghStr = j_locale["menuLanguage"].asString();
        }
    }
    pgetStatusRequest->pTTSStatus->ttsMenuLangStr = MenuLanghStr ;
    pgetStatusRequest->replyCB(pgetStatusRequest->pTTSStatus, pgetStatusRequest->message);
    LSMessageUnref(message);
    delete pgetStatusRequest->pTTSStatus;
    delete pgetStatusRequest;
    return true;
}
