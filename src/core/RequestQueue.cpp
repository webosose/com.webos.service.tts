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

#include <functional>
#include <future>
#include <RequestQueue.h>
#include <TTSLog.h>
#include <TTSRequest.h>
#include <StatusHandler.h>

RequestQueue::RequestQueue(std::string name)
{
    this->mQuit = true;
    this->mName = name;
}

RequestQueue::~RequestQueue()
{
    stop();
}

void RequestQueue::addRequest(Request* request)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("%s New request added to queue :%d\n", mName.c_str(), request->getType());

    std::unique_lock<std::mutex> lock(mMutex);
    mRequestQueue.push_back(request);
    lock.unlock();

    mCondVar.notify_one();
}

void RequestQueue::dispatchHandler()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    std::unique_lock < std::mutex > lock(mMutex);

    do {
        LOG_DEBUG("%s Waiting for request\n", mName.c_str());

        mCondVar.wait(lock, [this] {
            return (mRequestQueue.size() || mQuit);
        });

        if (mRequestQueue.size() && !mQuit) {
            Request* op = std::move(mRequestQueue.front());
            LOG_DEBUG("%sProcessing request %d\n", mName.c_str(), op->getType());
            popFront();
            lock.unlock();

            std::future<bool> fut = std::async(std::launch::async, [&op]() {return op->execute();});
            bool ret = fut.get();
            LOG_DEBUG("%s Executed Request %d status :%d\n", mName.c_str(), op->getType(), ret);
            delete op;
            op = nullptr;
            lock.lock();

        }
    } while (!mQuit);

    LOG_DEBUG("%s Dispatcher thread done\n", mName.c_str());

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
        mDispatcherThread.join();
    }
}

void RequestQueue::popFront()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (!mRequestQueue.empty()) mRequestQueue.erase(mRequestQueue.begin());
}

void RequestQueue::removeRequest(std::string sAppID, std::string sMsgID)
{
    if(sMsgID.empty() && sAppID.empty())
    {
        clearQueue();
        return;
    }
    std::vector<Request*>::iterator it = mRequestQueue.begin();
    while (it != mRequestQueue.end())
    {
        Request* ttsRequest = *it;
        SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(ttsRequest->getRequest());

        std::string QueueAppID = ptrSpeakRequest->msgParameters->sAppID;
        std::string QueueMsgID = ptrSpeakRequest->msgParameters->sMsgID;

        if( !sMsgID.empty()&& ( QueueMsgID.compare(sMsgID) == 0 ))
        {
            std::lock_guard<std::mutex> lock(mMutex);
            setRequestStatus(ttsRequest);
            it = mRequestQueue.erase(it);
            delete ttsRequest;
            ttsRequest = nullptr;
            break; //msgID is unique
        }
        else if ( QueueAppID.compare(sAppID) == 0)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            {
                setRequestStatus(ttsRequest);
                it = mRequestQueue.erase(it);
                delete ttsRequest;
                ttsRequest = nullptr;
            }
        }
        else
        {
            ++it;
        }
    }
}
void RequestQueue::clearQueue()
{
   LOG_TRACE("Entering function %s", __FUNCTION__);
   std::lock_guard<std::mutex> lock(mMutex);
   std::vector<Request*>::iterator it = mRequestQueue.begin();
   while (it != mRequestQueue.end())
   {
       Request* ttsRequest = *it;
       setRequestStatus(ttsRequest);
       it = mRequestQueue.erase(it);
       if (nullptr != ttsRequest){
          delete ttsRequest;
          ttsRequest = nullptr;
       }
   }
}

void RequestQueue::setRequestStatus(Request* pRequest)
{
    SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(pRequest->getRequest());
    ptrSpeakRequest->msgParameters->eStatus = TTS_MSG_CANCEL;
    StatusHandler::GetInstance()->Notify(ptrSpeakRequest->msgParameters, ptrSpeakRequest->message);
}
