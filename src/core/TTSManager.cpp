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
#include <new>

#include "TTSLog.h"
#include "TTSManager.h"

bool TTSManager::init(GMainLoop *_mainLoop) {
    LOG_TRACE("Entering function %s", __FUNCTION__);

    bool initialized = true;
    mainLoop = _mainLoop;

    try {
        mLunaService.init();
    } catch (const std::bad_alloc &eh) {
        std::cerr << "Exception Memory allocation : " << eh.what() << '\n';
        LOG_ERROR(MSGID_TTS_MEMORY_ERROR, 0,
                "Exception Memory allocation failed : %s", eh.what());
        initialized = false;
    }

    mLunaService.attachToLoop(mainLoop);

    return initialized;
}
