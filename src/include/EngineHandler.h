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

#ifndef SRC_CORE_ENGINEHANDLER_H_
#define SRC_CORE_ENGINEHANDLER_H_

#include <mutex>

#include <luna-service2/lunaservice.hpp>
#include <AudioEngine.h>
#include <TTSConfig.h>
#include <TTSEngine.h>
#include <TTSParameters.h>
#include <TTSRequestTypes.h>

class TTSRequest;
class EngineHandler
{

public:
    EngineHandler();
    virtual ~EngineHandler();

    bool handleRequest(TTSRequest* request, unsigned int displayId);
    void loadEngine();
    void unloadEngine();

    bool setConfig(bool status_flag);

    void getStatusInfo(TTSRequest* pTTSRequest, unsigned int displayId);
    void getLanguages(TTSRequest* pTTSRequest, unsigned int displayId);
    void saveSpeakRequestInfo(SpeakRequest* request, unsigned int displayId);
    bool getSpeakRequestInfo(unsigned int displayId, SpeakRequestInfo& info);
    void updateSpeakRequestInfo(unsigned int displayId, MsgStatus_t msgStatus);
    void removeSpeakRequestInfo(unsigned int displayId);
private:
    std::shared_ptr<TTSEngine> mTTSEngine[DUAL_DISPLAYS] = {nullptr};
    std::shared_ptr<AudioEngine> mAudioEngine[DUAL_DISPLAYS] = {nullptr};
    TTSConfig* mConfigHandler = {nullptr};
    pbnjson::JValue mTTSEngineName;
    pbnjson::JValue mAudioEngineName;
    pbnjson::JValue mDisplayCount;
    Task_Status_t meTTSTaskStatus[DUAL_DISPLAYS];
    std::string mCurrentLanguage[DUAL_DISPLAYS];
    std::map<unsigned int, SpeakRequestInfo> mSpeakRequestInfoMap;
    std::mutex mRunningInfoMutex[DUAL_DISPLAYS];
};

#endif /* SRC_CORE_ENGINEHANDLER_H_ */
