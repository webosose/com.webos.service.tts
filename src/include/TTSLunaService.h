// Copyright (c) 2018-2022 LG Electronics, Inc.
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

#ifndef SRC_LUNA_TTSLUNASERVICE_H_
#define SRC_LUNA_TTSLUNASERVICE_H_

#include <luna-service2/lunaservice.hpp>
#include <EngineHandler.h>
#include <ParameterListManager.h>
#include <RequestHandler.h>
#include <TTSLunaUtils.h>
#include <TTSParameters.h>
#include <StatusHandler.h>
#include <TTSRequestTypes.h>

class TTSLunaService: public LS::Handle, public StatusObserver
{
public:
    TTSLunaService(RequestHandler* requestHandler, std::shared_ptr<EngineHandler> engineHandler);//, std::shared_ptr<TTSConfig> configHandler );
    virtual ~TTSLunaService();
    bool speak(LSMessage &message);
    bool speakVKB(LSMessage &message);
    bool stop(LSMessage &message);
    bool start(LSMessage &message);
    bool setAudioGuidanceOnOff(LSMessage &message);
    bool getAvailableLanguages(LSMessage &message);
    bool getStatus(LSMessage &message);
    bool msgFeedback(LSMessage &message);
    bool setParameters(LSMessage &message);

private :
    RequestHandler* mRequestHandler;
    std::shared_ptr<EngineHandler> mEngineHandler;
    Parameters* mParameterList;
    static LSHandle* lsHandle;

    void registerService();
    static void responseCallback(Parameters* paramList, LS::Message& message);
    void addParameters(LSMessage &message);
    bool addSubscription(LSHandle *sh, LSMessage *message, std::string key);
    void setLSHandle(LSHandle* handle);

    void update(Parameters* paramList, LS::Message &message);
    static void statusResponse(TTSStatus* pTTSStatus, LS::Message& message, bool status = true);
    static bool handle_getVolume_callback(LSHandle *sh, LSMessage *reply, void *ctx);
    static bool handle_getSettings_callback(LSHandle *sh, LSMessage *reply, void *ctx);
    static void finalize_getstatus_request(GetStatusRequest*, bool status);
};
#endif /* SRC_LUNA_TTSLUNASERVICE_H_ */

