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

#ifndef SRC_ENGINE_PULSEAUDIOENGINE_H_
#define SRC_ENGINE_PULSEAUDIOENGINE_H_

#include <pulse/simple.h>
#include <AudioEngine.h>
#include <AudioEngineFactory.h>
#include <atomic>

class PulseAudioEngine: public AudioEngine
{
public:
    PulseAudioEngine();
    virtual ~PulseAudioEngine() {};
    void init();
    bool play(unsigned int displayId);
    bool playAudiotts1(std::string audio_file);
    bool playAudiotts2(std::string audio_file);
    bool stop(unsigned int displayId);
    void pause();
    void resume();
    void deInit();
private:
    pa_simple *mSimpletts1;
    pa_simple *mSimpletts2;
    std::atomic<bool> mIsStopPlaytts1;
    std::atomic<bool> mIsStopPlaytts2;
};

#endif /* SRC_ENGINE_PULSEAUDIOENGINE_H_ */
