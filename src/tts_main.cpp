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

#include <iostream>
#include <glib-2.0/glib.h>
#include <memory>

#include <TTSLog.h>
#include <TTSManager.h>

static GMainLoop *mainLoop = nullptr;

void term_handler(int signum)
{
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
    g_main_loop_quit(mainLoop);
}

int main(int argc, char **argv)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    signal(SIGTERM, term_handler);
    signal(SIGABRT, term_handler);

    GMainLoop *mainLoop = g_main_loop_new(nullptr, FALSE);
    if (mainLoop == NULL) {
        LOG_DEBUG("Failed to create g_main_loop!");
        return EXIT_FAILURE;
    }

    std::unique_ptr<TTSManager> ttsManager(new TTSManager());
    if(!ttsManager || !ttsManager->init(mainLoop))
    {
        LOG_DEBUG("TTS Manager registration failed");
        g_main_loop_unref(mainLoop);
        return EXIT_FAILURE;
    }

    g_main_loop_run(mainLoop);
    ttsManager->deInit();

    g_main_loop_unref(mainLoop);
    ttsManager.reset();

    return 0;
}
