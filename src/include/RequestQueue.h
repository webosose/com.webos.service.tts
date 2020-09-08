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

#ifndef REQUESTQUEUE_H_
#define REQUESTQUEUE_H_

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <Request.h>
#include <TTSLog.h>

class TTSRequest;
class RequestQueue
{
public:
    RequestQueue();
    RequestQueue(std::string name);
    virtual ~RequestQueue();
    void addRequest(Request* request, int displayId);
    void start(int displayId);
    void stop(int displayId);
    bool removeRequest(std::string sAppID, std::string sMsgID, int displayId);
    void clearQueue(int displayId);

private:
    void dispatchHandler(int displayId);
    void popFront(int displayId);
    void setRequestStatus(Request* request);
    volatile bool mQuit;
    std::string mName;
    std::thread mDispatcherThread[DUAL_DISPLAYS];
    std::vector<Request*> mRequestQueue[DUAL_DISPLAYS];
    std::mutex mMutex;
    std::condition_variable mCondVar;
};

#endif /* REQUESTQUEUE_H_ */
