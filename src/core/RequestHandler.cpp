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

#include <RequestHandler.h>
#include <StatusHandler.h>

RequestHandler::RequestHandler(std::shared_ptr<EngineHandler> engineHandler) :
        mSpeakRequestQueueDisplay1("QUEUE_SPEAK_1"), mSpeakRequestQueueDisplay2(
                "QUEUE_SPEAK_2"), mControlRequestQueueDisplay1(
                "QUEUE_CONTROL_1"), mControlRequestQueueDisplay2(
                "QUEUE_CONTROL_2"), mEngineHandler(engineHandler) {
}

bool RequestHandler::sendRequest(TTSRequest *request, unsigned int displayId) {
    LOG_TRACE("Entering function %s", __FUNCTION__);
    LOG_INFO(MSGID_REQUEST_HANDLER, 0, "%s disp: %d request: %d", __FUNCTION__,
            (int )displayId, request->getType());

    switch (request->getType()) {
        case SPEAK: {
            SpeakRequest *ptrSpeakRequest =
                    reinterpret_cast<SpeakRequest*>(request->getRequest());
            if (ptrSpeakRequest->msgParameters->bClear) {
                SpeakRequestInfo info;
                bool speakRequestFound = mEngineHandler->getSpeakRequestInfo(
                        displayId, info);
                if (speakRequestFound) {
                    LOG_INFO(MSGID_REQUEST_HANDLER, 0,
                            "%s speak request found app: %s, msg: %s",
                            __FUNCTION__, info.appId.c_str(),
                            info.msgId.c_str());
                    if (info.msgStatus == MsgStatus::TTS_MSG_PLAY) {
                        LOG_INFO(MSGID_REQUEST_HANDLER, 0,
                                "%s stop speak request", __FUNCTION__);
                        stopSpeech(displayId); // send stop clear message
                        mEngineHandler->updateSpeakRequestInfo(displayId,
                                TTS_MSG_STOP);
                        LOG_INFO(MSGID_REQUEST_HANDLER, 0,
                                "%s disp: %d Previous speak request is stopped",
                                __FUNCTION__, (int )displayId);
                    }
                }
                if (displayId)
                    mSpeakRequestQueueDisplay2.clearQueue();
                else
                    mSpeakRequestQueueDisplay1.clearQueue();
            }
            if (displayId)
                mSpeakRequestQueueDisplay2.addRequest(request);
            else
                mSpeakRequestQueueDisplay1.addRequest(request);
            break;
        }
        case STOP: {
            StopRequest *ptrStopRequest =
                    reinterpret_cast<StopRequest*>(request->getRequest());
            std::string stopAppID = ptrStopRequest->sAppID;
            std::string stopMsgID = ptrStopRequest->sMsgID;
            bool queueRet = false;
            bool runningRet = false;

            SpeakRequestInfo info;
            if (mEngineHandler->getSpeakRequestInfo(displayId, info)
                    && CheckToStopRunningSpeak(info, request)) {
                LOG_INFO(MSGID_REQUEST_HANDLER, 0,
                        "%s disp: %d SpeakRequestInfo found", __FUNCTION__,
                        (int )displayId);
                if (displayId)
                    mControlRequestQueueDisplay2.addRequest(request);
                else
                    mControlRequestQueueDisplay1.addRequest(request);
                runningRet = true;
            } else {
                delete request;
            }
            if (displayId)
                queueRet = mSpeakRequestQueueDisplay2.removeRequest(stopAppID,
                        stopMsgID);
            else
                queueRet = mSpeakRequestQueueDisplay1.removeRequest(stopAppID,
                        stopMsgID);

            return (runningRet) ? runningRet : queueRet;
        }
        case GET_LANGUAGES: {
            LOG_DEBUG("Request Type: GET_LANGUAGES");
            mEngineHandler->getLanguages(request, displayId);
            break;
        }
        case GET_STATUS: {
            LOG_DEBUG("Request Type: GET_STATUS");
            mEngineHandler->getStatusInfo(request, displayId);
            break;
        }
        default: {
            LOG_DEBUG("Unknown request Type..");
            delete request;
        }
    }

    return true;
}

void RequestHandler::start() {
    LOG_TRACE("Entering function %s", __FUNCTION__);

    mControlRequestQueueDisplay1.start();
    mSpeakRequestQueueDisplay1.start();
    mControlRequestQueueDisplay2.start();
    mSpeakRequestQueueDisplay2.start();
}

void RequestHandler::stop() {
    LOG_TRACE("Entering function %s", __FUNCTION__);

    mSpeakRequestQueueDisplay1.stop();
    mControlRequestQueueDisplay1.stop();
    mSpeakRequestQueueDisplay2.stop();
    mControlRequestQueueDisplay2.stop();
}

bool RequestHandler::CheckToStopRunningSpeak(SpeakRequestInfo& runningRequest,
        TTSRequest *pRequest) {
    LOG_INFO(MSGID_REQUEST_HANDLER, 0, "%s ", __FUNCTION__);

    bool bret = false;

    if (pRequest) {
        StopRequest *pnewStopRequest =
                reinterpret_cast<StopRequest*>(pRequest->getRequest());
        std::string requestAppID = pnewStopRequest->sAppID;
        std::string requestMsgID = pnewStopRequest->sMsgID;
        if (requestMsgID.empty() && requestAppID.empty()) {
            bret = true;
        } else if (!requestMsgID.empty()
                && (requestMsgID.compare(runningRequest.msgId) == 0)) {
            bret = true;
        } else if (!requestAppID.empty()
                && (requestAppID.compare(runningRequest.appId) == 0)) {
            bret = true;
        }
    }
    return bret;
}

void RequestHandler::stopSpeech(unsigned int displayId) {
    LOG_INFO(MSGID_REQUEST_HANDLER, 0, "%s disp: %d", __FUNCTION__,
            (int )displayId);
    StopRequest *stopRequest = new (std::nothrow) StopRequest;
    if (stopRequest == nullptr) {
        return;
    }
    stopRequest->displayId = displayId;
    TTSRequest *request = new (std::nothrow) TTSRequest(
            reinterpret_cast<RequestType*>(stopRequest), mEngineHandler);
    if (request == nullptr) {
        delete stopRequest;
        return;
    }
    if (displayId)
        mControlRequestQueueDisplay2.addRequest(request);
    else
        mControlRequestQueueDisplay1.addRequest(request);
}
