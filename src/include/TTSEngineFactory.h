// Copyright (c) 2009-2018 LG Electronics, Inc.
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

#ifndef SRC_CORE_TTSENGINEFACTORY_H_
#define SRC_CORE_TTSENGINEFACTORY_H_

#include <map>
#include <memory>
#include <TTSEngine.h>

class TTSEngineFactory
{
    public:
        virtual ~TTSEngineFactory() {}

        virtual std::shared_ptr<TTSEngine> create(void) const = 0;
        virtual const char* getName() const = 0;

        static std::shared_ptr<TTSEngine> createTTSEngine(const std::string& name);
        typedef std::shared_ptr<TTSEngineFactory> Factory;
        typedef std::map<std::string, Factory> Factories;

        template <typename T>
        struct Registrator
        {
            Registrator()
            {
                Factory factory { new T() };
                std::string key(factory->getName());
                std::pair<std::map<std::string, Factory>::iterator, bool > result;
                result = mFactories.insert(std::pair<std::string, Factory>(key, factory));
            }
        };

    private:
        static Factories mFactories;
};

#endif
