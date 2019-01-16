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
#include <TTSConfig.h>
#include <TTSLog.h>

#define TTS_CONFIG_FILE "/etc/palm/tts/tts_config.json"

TTSConfigError TTSConfig::readFile()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    std::string pathToFile = TTS_CONFIG_FILE;
    TTSConfigError confErr = TTSConfigError::TTS_CONFIG_ERROR_NONE;
    m_fileDOM = pbnjson::JDomParser::fromFile(pathToFile.c_str());

    std::string tts_name = m_fileDOM["engine"]["tts_engine"].asString();

    if (!m_fileDOM.isObject()) confErr = TTSConfigError::TTS_CONFIG_ERROR_LOAD;

    return confErr;
}

TTSConfigError TTSConfig::getValue(const std::string& category, const std::string& key, pbnjson::JValue& value) const
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    TTSConfigError confErr = TTSConfigError::TTS_CONFIG_ERROR_NONE;

    if(!category.empty())
        value = m_fileDOM[category][key];

    return confErr;
}