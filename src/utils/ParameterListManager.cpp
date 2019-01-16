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

#include <ParameterListManager.h>
#include <TTSLog.h>

ParameterListManager::ParameterListManager()
{

}

ParameterListManager::~ParameterListManager()
{
    mClients.clear();
}

// TODO: Modify these functions according to requirement

void ParameterListManager::addClient(TTSRequest* requestType, std::string appID, std::string msgID)
{
    mClients[appID].insert(msgID);
    LOG_DEBUG("Client: %s added to %d queue", appID.c_str(), requestType->getType() );
}

bool ParameterListManager::removeClient(TTSRequest* requestType, std::string appID, std::string msgID)
{
    const auto &it = mClients.find(appID);

    if (mClients.end() != it)
    {
        LOG_DEBUG("Client: %s removed from %d queue", appID.c_str(), requestType->getType() );
        mClients.erase(it);
        return true;
    }
    else
    {
        LOG_DEBUG("Client: %s not removed, Type %d", appID.c_str(), requestType->getType() );
        return false;
    }
}

bool ParameterListManager::removeMessage(TTSRequest* requestType, std::string appID, std::string msgID)
{
    const auto &it = mClients.find(appID);

    if (mClients.end() == it) {
        return false;
    }

    const auto &iter = it->second.find(msgID);

    if (iter != it->second.end())
    {
        it->second.erase(iter);
        LOG_DEBUG("Message: %s removed for %s\n", msgID.c_str(), appID.c_str() );
        return true;
    }
    return false;
}

bool ParameterListManager::isClientExist(TTSRequest* requestType, std::string appID)
{
    const auto &it = mClients.find(appID);

    return (mClients.find(appID) != mClients.end()) ? true : false;
}