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
#include <new>
#include <TTSLog.h>
#include <TTSManager.h>

TTSManager::TTSManager()
        : mainLoop(nullptr), mLunaService(nullptr), mRequestHandler(nullptr)
{

}

TTSManager::~TTSManager()
{
    deInit();
}

bool TTSManager::init(GMainLoop *mainLoop)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    this->mainLoop = mainLoop;

    try
    {
        mEngineHandler = std::make_shared<EngineHandler>();
    }
    catch (std::bad_alloc& eh)
    {
        std::cerr << "bad_alloc caught: " << eh.what() << '\n';
        return false;
    }

    mRequestHandler = new (std::nothrow) RequestHandler(mEngineHandler);
    if(mRequestHandler == nullptr)
        return false;

    mLunaService = new (std::nothrow) TTSLunaService(mRequestHandler, mEngineHandler);
    if(mLunaService == nullptr)
        return false;

    mLunaService->attachToLoop(mainLoop);
    mRequestHandler->start();

    return true;
}

void TTSManager::deInit()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (mRequestHandler) {
        delete mRequestHandler;
    }
    if (mEngineHandler.get()) {
        mEngineHandler.reset();
    }
    if (mLunaService) {
        mLunaService->detach();
        delete mLunaService;
    }
}

