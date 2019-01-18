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

#ifndef SRC_INCLUDE_TTSERRORS_H_
#define SRC_INCLUDE_TTSERRORS_H_

#include <string>
#include <map>


namespace TTSErrors
{

enum TTSErrors
{
    UNKNOWN_ERROR = 8000,
    SERVICE_NOT_READY,
    INIT_ERROR,
    INVALID_PARAM,
    TTS_INTERNAL_ERROR,
    PARAM_MISSING,
    LANG_NOT_SUPPORTED,
    COUNTRY_NOT_SUPPORTED,
    PLAY_ERROR,
    AUDIO_RES_UNAVAILABLE,
    SPEECH_DATA_CREATION_ERROR,
    FINALIZE_ERROR,
    SERVICE_ALREADY_RUNNING,
    INVALID_JSON_FORMAT,
    INPUT_TEXT_EMPTY,
    ERROR_NONE,
    TTS_ERROR_NOT_SUPPORTED = 8282,
};

enum TTSConfigErrors
{
    TTS_CONFIG_ERROR_NONE = 2000,
    TTS_CONFIG_ERROR_LOAD,
    TTS_CONFIG_ERROR_NOCATEGORY,
    TTS_CONFIG_ERROR_NOKEY
};

static std::map<int, std::string> mTTSErrorTextTable =
{
        { TTS_ERROR_NOT_SUPPORTED, "Not supported yet" },
        { TTS_CONFIG_ERROR_NONE, "Configuration success" },
        { TTS_CONFIG_ERROR_LOAD, "Failed to load config file" },
        { TTS_CONFIG_ERROR_NOCATEGORY, "No category foud in config" },
        { TTS_CONFIG_ERROR_NOKEY, "No valid found in config" },
        { UNKNOWN_ERROR, "Unknown error" },
        { SERVICE_NOT_READY, "Service is not ready" },
        { INIT_ERROR, "Initialize error" },
        { INVALID_PARAM, "Invalid parameter" },
        { TTS_INTERNAL_ERROR, "Internal error" },
        { PARAM_MISSING, "Required parameter is missing" },
        { LANG_NOT_SUPPORTED, "Not supported language" },
        { COUNTRY_NOT_SUPPORTED, "Not supported country" },
        { PLAY_ERROR, "Play error" },
        { AUDIO_RES_UNAVAILABLE, "Audio resource is not available" },
        { SPEECH_DATA_CREATION_ERROR, "Speech data creation error" },
        { FINALIZE_ERROR, "Finalize error" },
        { SERVICE_ALREADY_RUNNING, "Service is already running" },
        { INVALID_JSON_FORMAT, "Invalid JSON format" },
        { INPUT_TEXT_EMPTY, "Input text must not be empty" },
        { ERROR_NONE, "No error" },
};
std::string getTTSErrorString(int errorCode);
}

using TTSErrorCodes = TTSErrors::TTSErrors;
using TTSConfigError = TTSErrors::TTSConfigErrors;

#endif /* SRC_INCLUDE_TTSERRORS_H_ */
