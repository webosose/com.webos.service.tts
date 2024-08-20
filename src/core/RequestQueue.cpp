// Copyright (c) 2018-2024 LG Electronics, Inc.
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

#include <functional>
#include <future>
#include <RequestQueue.h>
#include <TTSLog.h>
#include <TTSRequest.h>
#include <StatusHandler.h>

RequestQueue::RequestQueue()
{
    mQuit = true;
}
RequestQueue::RequestQueue(std::string name)
{
    mQuit = true;
    mName = std::move(name);
}

RequestQueue::~RequestQueue()
{
    mQuit = true;
    mCondVar.notify_all();
    if (mDispatcherThread.joinable()) {
        mDispatcherThread.join();
    }
}

void RequestQueue::addRequest(Request* request)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("%s New request added to queue :%d\n", mName.c_str(),
            request->getType());
    {
        std::lock_guard < std::mutex > lock(mMutex);
        mRequestQueue.push_back(request);
        LOG_INFO(MSGID_REQUEST_QUEUE, 0,
                "%s Name: %s new request added, queue size: %d", __FUNCTION__,
                mName.c_str(), (int )mRequestQueue.size());
    }
    mCondVar.notify_one();
}

void RequestQueue::dispatchHandler()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    std::unique_lock < std::mutex > lock(mMutex, std::defer_lock);
    lock.lock();

    do {
        LOG_INFO(MSGID_REQUEST_QUEUE, 0,
                "%s Name: %s Waiting for request.. queue size: %d",
                __FUNCTION__, mName.c_str(), (int )mRequestQueue.size());
        mCondVar.wait(lock, [this] {
            return (mRequestQueue.size() || mQuit);
        });

        if (mRequestQueue.size() && !mQuit) {
            Request *op = mRequestQueue.front();
            LOG_INFO(MSGID_REQUEST_QUEUE, 0,
                    "%s Name: %s, processing %d request", __FUNCTION__,
                    mName.c_str(), op->getType());
            mRequestQueue.erase(mRequestQueue.begin());
            lock.unlock();

            std::future<bool> fut = std::async(std::launch::async, [&op]() {
                return op->execute();
            });
            bool ret = fut.get();
            LOG_INFO(MSGID_REQUEST_QUEUE, 0,
                    "%s queue: %s Executed Request status :%d  queuesize: %d\n",
                    __FUNCTION__, mName.c_str(), ret,
                    (int )mRequestQueue.size());
            delete op;
            lock.lock();
        }
    } while (!mQuit);

    LOG_INFO(MSGID_REQUEST_QUEUE, 0, "%s dispatcher thread %s exiting..",
            __FUNCTION__, mName.c_str());
}

void RequestQueue::start()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("%s Starting dispatcher thread\n", mName.c_str());

    if (mQuit) {
        mQuit = false;
        mDispatcherThread = std::thread(std::bind(&RequestQueue::dispatchHandler, this));
    }
}

void RequestQueue::stop()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("%s Stopping  dispatcher thread\n", mName.c_str());

    if (!mQuit) {
        mQuit = true;
        mCondVar.notify_all();
        if (mDispatcherThread.joinable()) {
            mDispatcherThread.join();
        }
    }
}

bool RequestQueue::removeRequest(std::string sAppID, std::string sMsgID)
{
    LOG_INFO(MSGID_REQUEST_QUEUE, 0, "%s Name: %s sAppID: %s sMsgID: %s\n",
            __FUNCTION__, mName.c_str(), sAppID.c_str(), sMsgID.c_str());

    bool result = true;

    if (sMsgID.empty() && sAppID.empty()) {
        clearQueue();
        return result;
    }

    result = false;
    std::lock_guard < std::mutex > lock(mMutex);
    std::vector<Request*>::iterator it = mRequestQueue.begin();

    while (it != mRequestQueue.end()) {
        Request *ttsRequest = *it;
        SpeakRequest *ptrSpeakRequest =
                reinterpret_cast<SpeakRequest*>(ttsRequest->getRequest());

        std::string QueueAppID = ptrSpeakRequest->msgParameters->sAppID;
        std::string QueueMsgID = ptrSpeakRequest->msgParameters->sMsgID;

        if (!sMsgID.empty() && (QueueMsgID.compare(sMsgID) == 0)) {
            {
                setRequestStatus(ttsRequest);
                it = mRequestQueue.erase(it);
                LOG_INFO(MSGID_REQUEST_QUEUE, 0,
                        "%s Name: %s deleting tts request ", __FUNCTION__,
                        mName.c_str());
                delete ttsRequest;
                result = true;
            }
            break; //msgID is unique
        } else if (QueueAppID.compare(sAppID) == 0) {
            {
                setRequestStatus(ttsRequest);
                it = mRequestQueue.erase(it);
                LOG_INFO(MSGID_REQUEST_QUEUE, 0,
                        "%s Name: %s deleting tts request ", __FUNCTION__,
                        mName.c_str());
                delete ttsRequest;
                result = true;
            }
        } else {
            ++it;
        }
    }
    return result;
}

void RequestQueue::clearQueue()
{
    LOG_INFO(MSGID_REQUEST_QUEUE, 0, "%s Name: %s", __FUNCTION__,
            mName.c_str());

    std::lock_guard < std::mutex > lock(mMutex);
    std::vector<Request*>::iterator it = mRequestQueue.begin();
    while (it != mRequestQueue.end()) {
        Request *ttsRequest = *it;
        setRequestStatus(ttsRequest);
        it = mRequestQueue.erase(it);
        LOG_INFO(MSGID_REQUEST_QUEUE, 0, "%s Name: %s deleting tts request ",
                __FUNCTION__, mName.c_str());
        delete ttsRequest;
        ttsRequest = nullptr;
    }
}

void RequestQueue::setRequestStatus(Request* pRequest)
{
    SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(pRequest->getRequest());
    ptrSpeakRequest->msgParameters->eStatus = TTS_MSG_CANCEL;
    LOG_INFO(MSGID_REQUEST_QUEUE, 0, "%s queue: %s Notify request status ", __FUNCTION__, mName.c_str());
    StatusHandler::GetInstance()->Notify(ptrSpeakRequest->msgParameters, ptrSpeakRequest->message);
}
