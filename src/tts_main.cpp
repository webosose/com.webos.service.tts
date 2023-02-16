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
#include <glib-2.0/glib.h>
#include <memory>

#include "TTSLog.h"
#include "TTSManager.h"

static GMainLoop *mainLoop = nullptr;

void term_handler(int signum) {
    LOG_TRACE("Entering function %s", __FUNCTION__);
    const char *str = nullptr;

    switch (signum) {
        case SIGTERM:
            str = "SIGTERM";
            break;

        case SIGABRT:
            str = "SIGABRT";
            break;

        default:
            str = "Unknown";
            break;
    }

    LOG_DEBUG("signal received.. signal[%s]", str);
    if (g_main_loop_is_running(mainLoop)) {
        g_main_loop_quit(mainLoop);
    }
}

int main(int argc, char **argv) {
    try {
        LOG_TRACE("Entering function %s", __FUNCTION__);

        signal(SIGTERM, term_handler);
        signal(SIGABRT, term_handler);

        mainLoop = g_main_loop_new(nullptr, FALSE);
        if (!mainLoop) {
            LOG_ERROR(MSGID_TTS_ERROR, 0, "Failed to create g_main_loop!");
            return EXIT_FAILURE;
        }

        TTSManager ttsManager;
        auto initialized = ttsManager.init(mainLoop);
        if (!initialized) {
            LOG_ERROR(MSGID_TTS_ERROR, 0, "TTS Manager registration failed!");
            g_main_loop_unref(mainLoop);
            return EXIT_FAILURE;
        }

        g_main_loop_run(mainLoop);
        LOG_INFO(MSGID_MESSAGE_CALL, 0, "tts mainloop exited.");
        g_main_loop_unref(mainLoop);
    } catch (...) {
        LOG_ERROR(MSGID_TTS_ERROR, 0,
                "TTS Manager registration failed with exception!");
    }

    return 0;
}
