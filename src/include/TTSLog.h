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

#ifndef SRC_UTILS_TTSLOG_H_
#define SRC_UTILS_TTSLOG_H_

#define MSGID_ERROR_CALL                "ERROR_CALL"
#define MSGID_MESSAGE_CALL              "MESSAGE_CALL"
#define MSGID_TTS_ERROR                 "TTS_ERROR"
#define MSGID_LUNA_ERROR_RESPONSE       "LUNA_ERROR_RESPONSE"
#define MSGID_LUNA_SERVICE_WARNING      "LUNA_SERVICE_WARNING"

//#ifdef USE_PMLOG
#include "PmLogLib.h"

#define LOG_CRITICAL(msgid, kvcount, ...)\
PmLogCritical(getTTSPmLogContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_ERROR(msgid, kvcount, ...)\
PmLogError(getTTSPmLogContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_WARNING(msgid, kvcount, ...)\
PmLogWarning(getTTSPmLogContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_INFO(msgid, kvcount, ...)\
PmLogInfo(getTTSPmLogContext(), msgid, kvcount, ##__VA_ARGS__)

#define LOG_DEBUG(...)\
PmLogDebug(getTTSPmLogContext(), ##__VA_ARGS__)

#define LOG_TRACE(...)\
PMLOG_TRACE(__VA_ARGS__);

extern PmLogContext getTTSPmLogContext();

/*#else
#include <stdio.h>

#define CONSOLE_LOGS(msgid, ...)\
printf(msgid);\
printf(" ");\
printf( __VA_ARGS__);\
printf("\n")

#define CONSOLE_DEBUG_LOGS(...)\
printf(__VA_ARGS__);\
printf("\n")

#define LOG_CRITICAL(msgid, kvcount, ...)\
CONSOLE_LOGS(msgid,kvcount, __VA_ARGS__)

#define LOG_ERROR(msgid, kvcount, ...)\
CONSOLE_LOGS(msgid, __VA_ARGS__)

#define LOG_WARNING(msgid, kvcount, ...)\
CONSOLE_LOGS(msgid, __VA_ARGS__)

#define LOG_INFO(msgid, kvcount, ...)\
CONSOLE_LOGS(msgid, __VA_ARGS__)

#define LOG_DEBUG(...)\
CONSOLE_DEBUG_LOGS(__VA_ARGS__)

#define LOG_TRACE(...)\
CONSOLE_DEBUG_LOGS(__VA_ARGS__)

#endif*/ /* USE_PMLOG */
#endif /* SRC_UTILS_TTSLOG_H_ */
