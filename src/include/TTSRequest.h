// Copyright (c) 2018-2019 LG Electronics, Inc.
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

#ifndef SRC_CORE_TTSREQUEST_H_
#define SRC_CORE_TTSREQUEST_H_

#include <memory>
#include <EngineHandler.h>
#include <Request.h>

class TTSRequest: public Request
{
public:
    TTSRequest(RequestType *reqType, std::shared_ptr<EngineHandler> engineHandler);
    virtual ~TTSRequest();
    REQUEST_TYPE getType();
    RequestType* getRequest();
    bool execute();

private:
    RequestType* mReqType;
    TTSRequest & operator = (const TTSRequest &rh)= delete;
    TTSRequest (const TTSRequest &rh)= delete;

    std::shared_ptr<EngineHandler> mEngineHandler;
};

#endif /* SRC_CORE_TTSREQUEST_H_ */
