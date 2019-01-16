// Copyright (c) 2018 LG Electronics, Inc.
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

#ifndef SRC_CORE_TTSMANAGER_H_
#define SRC_CORE_TTSMANAGER_H_

#include <memory>
#include <glib-2.0/glib.h>
#include <EngineHandler.h>
#include <RequestHandler.h>
#include <TTSConfig.h>
#include <TTSLunaService.h>

class TTSManager
{
public:
    TTSManager();
    virtual ~TTSManager();
    bool init(GMainLoop *mainLoop);
    void deInit();

private:

    GMainLoop* mainLoop;
    TTSLunaService *mLunaService;
    RequestHandler *mRequestHandler;
    std::shared_ptr<EngineHandler> mEngineHandler;
};

#endif /* SRC_CORE_TTSMANAGER_H_ */
