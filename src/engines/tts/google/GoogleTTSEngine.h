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

#ifndef SRC_ENGINES_GOOGLETTSENGINE_H_
#define SRC_ENGINES_GOOGLETTSENGINE_H_

#include <luna-service2/lunaservice.hpp>
#include <pthread.h>
#include <TTSEngine.h>
#include <TTSEngineFactory.h>
#include <vector>
#include <atomic>

#include <grpc++/grpc++.h>

#include <google/cloud/texttospeech/v1/cloud_tts.pb.h>
#include <google/cloud/texttospeech/v1/cloud_tts.grpc.pb.h>

#define DEFAULT_PITCH       0.0
#define DEFAULT_SPEAK_RATE  1.0
#define DEFAULT_DEADLINE_DURATION 1000
#define GOOGLE_ENV_FILE "/etc/google/google_tts_credentials.json"
#define DEFAULT_SPEECH_SAMPLE_RATE 22050

#define DISPLAY_0 0 //Display One Functionality
#define DISPLAY_1 1 //Display Two Functionality
using grpc::Channel;
using grpc::ChannelCredentials;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientAsyncResponseReader;
using grpc::CompletionQueue;
using google::cloud::texttospeech::v1::TextToSpeech;
using google::cloud::texttospeech::v1::SynthesizeSpeechRequest;
using google::cloud::texttospeech::v1::SynthesizeSpeechResponse;
using google::cloud::texttospeech::v1::ListVoicesRequest;
using google::cloud::texttospeech::v1::ListVoicesResponse;

class GoogleTTSEngine: public TTSEngine
{
public:
    GoogleTTSEngine(double pitch = DEFAULT_PITCH, double speakRate = DEFAULT_SPEAK_RATE);

    virtual ~GoogleTTSEngine() {};
    void getStatus();
    void getSupportedLanguages(std::vector<std::string> &  vecLang, unsigned int displayId);
    int speak(std::string text, LSHandle* sh, std::string language, unsigned int displayId);
    void start();
    void stop(unsigned int displayId);
    void init();
    void deInit();
    std::string getName();
    void setSpeakRate(double rate);
    void setPitch(double pitch);
    double getSpeakRate(void) const;
    double getPitch(void) const;
private:
    std::string mTextToSpeak;
    std::string mOutputLanguage;
    SynthesizeSpeechRequest mSpeechRequest;
    std::unique_ptr<TextToSpeech::Stub> mTextToSpeech;
    std::shared_ptr<ChannelCredentials> mCredentials;
    std::shared_ptr<Channel> mChannel;
    double mSpeakRate;
    double mPitch;
    std::vector<std::string> mAvailableLanguages;
    std::atomic<bool>mIsStopDisplay1 ;
    std::atomic<bool>mIsStopDisplay2 ;
};

#endif /* SRC_ENGINES_GOOGLETTSENGINE_H_ */
