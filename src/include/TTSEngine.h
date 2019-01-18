// Copyright (c) 2019 LG Electronics, Inc.
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

#ifndef SRC_CORE_TTSENGINE_H_
#define SRC_CORE_TTSENGINE_H_

#include <string>
#include <luna-service2/lunaservice.hpp>

class TTSEngine
{
public:
    TTSEngine() = default;
    virtual ~TTSEngine() = default;
    virtual void getStatus() = 0;
    virtual void getSupportedLanguages(std::vector<std::string> &  vecLang) = 0;
    virtual int speak(const std::string text, LSHandle* sh, std::string language) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void init() = 0;
    virtual void deInit() = 0;
    virtual std::string getName()=0;
};

#endif /* SRC_CORE_TTSENGINE_H_ */
