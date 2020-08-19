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


#include <iostream>
#include <AudioEngineFactory.h>
#include <TTSLog.h>

AudioEngineFactory::Factories AudioEngineFactory::mFactories;
std::shared_ptr<AudioEngine> AudioEngineFactory::createAudioEngine(const std::string& name)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    std::shared_ptr<AudioEngine> engine;

    std::map<std::string, Factory>::iterator it;
    it = mFactories.find(name.c_str());
    if(it != mFactories.end())
    {
        Factory factory = mFactories.find(name.c_str())->second;
        engine = factory->create();
    }
    return engine;
}

