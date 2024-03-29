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

#ifndef SRC_CORE_AUDIOENGINE_H_
#define SRC_CORE_AUDIOENGINE_H_


#include <string>

class AudioEngine
{
public:
    AudioEngine() = default;
    virtual ~AudioEngine() = default;
    virtual bool play(unsigned int displayId) = 0;
    virtual bool stop(unsigned int displayId) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void init() = 0;
    virtual void deInit() = 0;
};

#endif /* SRC_CORE_AUDIOENGINE_H_ */
