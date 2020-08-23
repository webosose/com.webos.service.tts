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

#ifndef ENGINE_GOOGLEENGINEFACTORY_H_
#define ENGINE_GOOGLEENGINEFACTORY_H_

#include <memory>
#include <TTSEngine.h>
#include <TTSEngineFactory.h>

class GoogleTTSEngineFactory : public TTSEngineFactory
{
    public:
        virtual std::shared_ptr<TTSEngine> create(void) const;
        virtual const char* getName() const { return "google"; }
};
#endif
