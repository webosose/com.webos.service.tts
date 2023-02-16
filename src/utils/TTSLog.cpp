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

#include <cstdlib>

#include "TTSLog.h"

static const char *const logContextName = "TTS";

PmLogContext getTTSPmLogContext() {
    LOG_TRACE("Entering function %s", __FUNCTION__);

    static PmLogContext logContext = 0;
    if (0 == logContext) {
        if (PmLogGetContext(logContextName, &logContext) != kPmLogErr_None) {
            LOG_ERROR(MSGID_ERROR_CALL, 0, "Failed to get logContext");
            exit (EXIT_FAILURE);
        }
    }
    return logContext;
}
