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

#include <functional>
#include <future>
#include <RequestQueue.h>
#include <TTSLog.h>
#include <TTSRequest.h>
#include <StatusHandler.h>

RequestQueue::RequestQueue()
{
    this->mQuit = true;
}
RequestQueue::RequestQueue(std::string name)
{
    this->mQuit = true;
    this->mName = name;
}

RequestQueue::~RequestQueue()
{

}

void RequestQueue::addRequest(Request* request, int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("%s New request added to queue :%d\n", mName.c_str(), request->getType());
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mRequestQueue[displayId].push_back(request);
    }
    mCondVar.notify_one();
}

void RequestQueue::dispatchHandler(int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));   // Sleep for 1000 milliseconds

    std::unique_lock < std::mutex > lock(mMutex);

    do {
        LOG_DEBUG("%s Waiting for request\n", mName.c_str());
	if (displayId)
        {
            LOG_DEBUG("mCondVar");
            mCondVar.wait(lock, [this] {
                return (mRequestQueue[DISPLAY_1].size() || mQuit);
            });
        }
        else
        {
            mCondVar.wait(lock, [this] {
                return (mRequestQueue[DISPLAY_0].size() || mQuit);
            });
        }

        if (mRequestQueue[displayId].size() && !mQuit) {
            Request* op = std::move(mRequestQueue[displayId].front());
            LOG_DEBUG("%sProcessing request %d\n", mName.c_str(), op->getType());
            popFront(displayId);
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
void RequestQueue::start(int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("%s Starting dispatcher thread\n", mName.c_str());

    if (mQuit) {
        mQuit = false;
        mDispatcherThread[displayId] = std::thread(std::bind(&RequestQueue::dispatchHandler, this, displayId));
    }
}

void RequestQueue::stop(int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    LOG_DEBUG("%s Stopping  dispatcher thread\n", mName.c_str());

    if (!mQuit) {
        mQuit = true;
        mCondVar.notify_all();
    }
}

void RequestQueue::popFront(int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    if (!mRequestQueue[displayId].empty()){
       (void)(mRequestQueue[displayId].erase(mRequestQueue[displayId].begin()));
    }
}

bool RequestQueue::removeRequest(std::string sAppID, std::string sMsgID, int displayId)
{
    if(sMsgID.empty() && sAppID.empty())
    {
        clearQueue(displayId);
        return true;
    }
    std::vector<Request*>::iterator it = mRequestQueue[displayId].begin();
    bool del_ret = false;
    while (it != mRequestQueue[displayId].end())
    {
        Request* ttsRequest = *it;
        SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(ttsRequest->getRequest());

        std::string QueueAppID = ptrSpeakRequest->msgParameters->sAppID;
        std::string QueueMsgID = ptrSpeakRequest->msgParameters->sMsgID;

        if( !sMsgID.empty()&& ( QueueMsgID.compare(sMsgID) == 0 ))
        {
            std::lock_guard<std::mutex> lock(mMutex);
            setRequestStatus(ttsRequest);
            it = mRequestQueue[displayId].erase(it);
            delete ttsRequest;
            ttsRequest = nullptr;
            del_ret = true;
            break; //msgID is unique
        }
        else if ( QueueAppID.compare(sAppID) == 0)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            {
                setRequestStatus(ttsRequest);
                it = mRequestQueue[displayId].erase(it);
                delete ttsRequest;
                ttsRequest = nullptr;
                del_ret = true;
            }
        }
        else
        {
            ++it;
        }
    }
    return del_ret;
}
void RequestQueue::clearQueue(int displayId)
{
   LOG_TRACE("Entering function %s", __FUNCTION__);
   std::lock_guard<std::mutex> lock(mMutex);
   std::vector<Request*>::iterator it = mRequestQueue[displayId].begin();
   while (it != mRequestQueue[displayId].end())
   {
       Request* ttsRequest = *it;
       setRequestStatus(ttsRequest);
       it = mRequestQueue[displayId].erase(it);
       delete ttsRequest;
       ttsRequest = nullptr;
   }
   LOG_DEBUG("RequestQueue::clearQueue : Cleared request from mRequestQueue, displayId = %d", displayId);
}

void RequestQueue::setRequestStatus(Request* pRequest)
{
    SpeakRequest* ptrSpeakRequest = reinterpret_cast<SpeakRequest*>(pRequest->getRequest());
    ptrSpeakRequest->msgParameters->eStatus = TTS_MSG_CANCEL;
    StatusHandler::GetInstance()->Notify(ptrSpeakRequest->msgParameters, ptrSpeakRequest->message);
}
