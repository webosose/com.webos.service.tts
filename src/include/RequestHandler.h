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

#ifndef SRC_CORE_REQUESTHANDLER_H_
#define SRC_CORE_REQUESTHANDLER_H_

#include <memory>
#include <vector>
#include <EngineHandler.h>
#include <RequestQueue.h>
#include <TTSRequest.h>

class RequestHandler
{
public:
    RequestHandler(std::shared_ptr<EngineHandler> engineHandler);
    virtual ~RequestHandler();
    bool sendRequest(TTSRequest* request, int displayId);
    void start(int displayId);
    void stop(int displayId);

private:
    bool CheckToStopRunningSpeak(TTSRequest* pRunningRequest, TTSRequest* pRequest);
    void stopSpeech(int displayId);
    RequestQueue mSpeakRequestQueueDisplay1;
    RequestQueue mSpeakRequestQueueDisplay2;
    RequestQueue mControlRequestQueueDisplay1;
    RequestQueue mControlRequestQueueDisplay2;
    std::shared_ptr<EngineHandler> mEngineHandler;
};

#endif /* SRC_CORE_REQUESTHANDLER_H_ */
