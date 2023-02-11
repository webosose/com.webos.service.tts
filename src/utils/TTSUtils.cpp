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

#include <TTSUtils.h>
#include <TTSLunaUtils.h>

TTSUtils::TTSUtils()
{}

TTSUtils& TTSUtils::getInstance()
{
    static TTSUtils obj = TTSUtils();
    return obj;
}

bool TTSUtils::isValidDisplayId(LS::Message &request, pbnjson::JValue& requestObj, unsigned int &displayId)
{
    bool valid = true;
    if (requestObj.hasKey("displayId"))
        displayId = requestObj["displayId"].asNumber<int>();
    valid = (displayId > 1)? false : true;
    if (!valid)
    {
        const std::string errorStr = TTSErrors::getTTSErrorString(TTSErrors::INVALID_PARAM);
        LSUtils::respondWithError(request, errorStr, TTSErrors::INVALID_PARAM);
    }
    return valid;
}
