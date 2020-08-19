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

#ifndef TTSPARAMPOLICY_H_
#define TTSPARAMPOLICY_H_

#include <string>

#define GET_MSG_STATUS_TEXT(x) TTS_MsgStatusTable[(x)]
#define GET_TASK_STATUS_TEXT(x) TTS_TaskStatusTable[(x)]

#define DUAL_DISPLAYS 2
#define DISPLAY_0 0 //Display One Functionality
#define DISPLAY_1 1 //Display Two Functionality
typedef enum _TTS_LANGUAGE_T
{
    LANG_ERR = -1,

    LANG_KOKR = 0,          // Korean
    LANG_ENUS,              // US English
    LANG_MNCH,              // Mandarin (China)
    LANG_JAJP,              // Japanese (Japan)
    LANG_FRFR,              // French
    LANG_ENGB,              // UK English
    LANG_ENIE,              // Ireland English
    LANG_DADK,              // Danish

    LANG_DEDE,              // German
    LANG_ESMX,              // American Spanish(Mexico)
    LANG_ESES,              // Spanish
    LANG_ESAR,              // Spanish Argentina
    LANG_FRCA,              // Canadian French

    LANG_ITIT,              // Italian
    LANG_NLNL,              // Dutch
    LANG_NBNO,              // Norwegian
    LANG_PLPL,              // Polish

    LANG_PTPT,              // EU Portuguese
    LANG_RURU,              // Russian
    LANG_FIFI,              // Finnish
    LANG_SVSE,              // Swedish
    LANG_TRTR,              // Turkish

    LANG_PTBR,              // Brazilian Portuguese
    LANG_ARSA,              // Arabic Saudi Arabia
    LANG_ENIN,              // English (India)
    LANG_IDID,              // Indonesian (Indonesia)

    LANG_THTH,              // Thai (Thailand)
    LANG_MAX,
} TTS_LANGUAGE_T;

typedef struct  _TTS_LANGUAGE_PARAM_T
{
    TTS_LANGUAGE_T ttsLanguage;
    std::string languageStr;
    std::string ttsLanguageStr;
} TTS_LANGUAGE_PARAM_T;

static TTS_LANGUAGE_PARAM_T TTSLanguageTable[LANG_MAX] = {
    {LANG_KOKR,     "ko-KR",        "KOK"},         // Korean
    {LANG_ENUS,     "en-US",        "ENU"},         // US English
    {LANG_MNCH,     "mn-CH",        "CHM"},       // Mandarin (China)
    {LANG_JAJP,     "ja-JP",        "JPJ"},            // Japanese (Japan)
    {LANG_FRFR,     "fr-FR",        "FRF"},         // French
    {LANG_ENGB,     "en-GB",        "ENG"},         // UK English
    {LANG_ENIE,     "en-IE",        "ENE"},         // Ireland English
    {LANG_DADK,     "da-DK",        "DAD"},         // Danish

    {LANG_DEDE,     "de-DE",        "GED"},         // German
    {LANG_ESMX,     "es-MX",        "SPM"},         // American Spanish(Mexico)
    {LANG_ESES,     "es-ES",        "SPE"},         // Spanish
    {LANG_ESAR,     "es-AR",        "SPA"},         // Spanish Argentina
    {LANG_FRCA,     "fr-CA",        "FRC"},         // Canadian French

    {LANG_ITIT,     "it-IT",        "ITI"},         // Italian
    {LANG_NLNL,     "nl-NL",        "DUN"},         // Dutch
    {LANG_NBNO,     "nb-NO",        "NON"},         // Norwegian
    {LANG_PLPL,     "pl-PL",        "PLP"},         // Polish

    {LANG_PTPT,     "pt-PT",        "PTP"},         // EU Portuguese
    {LANG_RURU,     "ru-RU",        "RUR"},         // Russian
    {LANG_FIFI,     "fi-FI",        "FIF"},         // Finnish
    {LANG_SVSE,     "sv-SE",        "SWS"},         // Swedish
    {LANG_TRTR,     "tr-TR",        "TRT"},         // Turkish

    {LANG_PTBR,     "pt-BR",        "PTB"},         // Brazilian Portuguese
    {LANG_ARSA,     "ar-SA",        "ARW"},         // Arabic Saudi Arabia
    {LANG_ENIN,     "en-IN",        "ENI"},         // English (India)
    {LANG_IDID,     "id-ID",        "IDI"},         // Indonesian (Indonesia)

    {LANG_THTH,     "th-TH",        "THT"},         // Thai (Thailand)
};

typedef enum MsgStatus
{
    TTS_MSG_PLAY = 0,
    TTS_MSG_DONE,
    TTS_MSG_STOP,
    TTS_MSG_CANCEL,
    TTS_MSG_ERROR,
} MsgStatus_t;

static std::string TTS_MsgStatusTable[] = {
    "playing",
    "done",
    "stopped",
    "canceled",
    "error",
};

typedef enum Task_Status
{
    TTS_TASK_ERROR = 0,
    TTS_TASK_NOT_READY,
    TTS_TASK_READY,
    TTS_TASK_DONE
} Task_Status_t;

typedef struct Parameters
{
    std::string sText;
    bool bClear;
    TTS_LANGUAGE_T eLang;
    std::string sLangStr;
    bool bFeedback;
    bool bSubscribed;
    std::string sMsgID;
    std::string sAppID;
    MsgStatus_t eStatus;
    Task_Status_t eTaskStatus;
    int displayId;
}Parameters;

static std::string TTS_TaskStatusTable[] = {
    "Error: TTS task is not running",
    "TTS task is not ready",
    "TTS task is ready",
    "TTS task is done",
};

typedef struct TTSStatus
{
    int errorCode;
    std::string status;
    std::string ttsLanguageStr;
    std::string ttsMenuLangStr;
    int pitch;
    int speechRate;
    int volume;
}TTSStatus;

typedef bool (*pfnSetTraceSubscriptionCB)(Parameters* paramList);
//typedef bool (*pfnSetTTSSubscriptionCB)(TTS_SUBSCRIPTION_MSG_T ttsMsg);

void registerTraceCallback(pfnSetTraceSubscriptionCB pfnTraceCallbackFunc);

#endif // TTSPARAMPOLICY_H_
