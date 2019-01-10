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

#include <fstream>
#include <sstream>
#include <GoogleTTSEngine.h>
#include <TTSErrors.h>
#include <TTSLog.h>
#include <algorithm>

using google::cloud::texttospeech::v1::AudioConfig;
using google::cloud::texttospeech::v1::SynthesisInput;
using google::cloud::texttospeech::v1::VoiceSelectionParams;
using google::cloud::texttospeech::v1::AudioEncoding;

#define GOOGLE_APPLICATION_ENDPOINT   "texttospeech.googleapis.com"
#define AUDIO_FILE                    "/tmp/sttsResult.pcm"
#define DEFAULT_LANGUAGE              "en-US"
#define TTS_ENGINE_NAME               "google"
#define GOOGLE_TTS_REQUEST_TAG        1

GoogleTTSEngine::GoogleTTSEngine(double pitch, double speakRate) : TTSEngine(),mSpeakRate(speakRate),mPitch(pitch),
     mIsStop(false)
{
    mCredentials = grpc::GoogleDefaultCredentials();
}

void GoogleTTSEngine::getStatus()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
}

void GoogleTTSEngine::getSupportedLanguages(std::vector<std::string> &  vecLang)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    if(mAvailableLanguages.size() != 0){
        vecLang = mAvailableLanguages;
        return;
    }

    auto channel = grpc::CreateChannel(GOOGLE_APPLICATION_ENDPOINT, mCredentials);
    std::unique_ptr<TextToSpeech::Stub> textToSpeech(TextToSpeech::NewStub(channel));
    ListVoicesRequest listVoicesRequest;
    ListVoicesResponse listVoicesResponse;
    ClientContext context;
    Status status = textToSpeech->ListVoices(&context, listVoicesRequest, &listVoicesResponse);
    if(status.ok()){
        int totalSounds = listVoicesResponse.voices_size();
        for(int i=0; i< totalSounds; i++){
            ::google::cloud::texttospeech::v1::Voice tmpVoice = listVoicesResponse.voices(i);
            int totalLanguageCodes = tmpVoice.language_codes_size();
            for(int j=0; j < totalLanguageCodes; j++){
                std::string languageCodeStr = tmpVoice.language_codes(j);
                if ( std::find(mAvailableLanguages.begin(), mAvailableLanguages.end(), languageCodeStr) == mAvailableLanguages.end() ){
                    mAvailableLanguages.push_back(languageCodeStr);
                }
            }
        }
        vecLang = mAvailableLanguages;
    }else{
        LOG_DEBUG("Google TextToSpeech LanguageList Error: %d ErrorMsg:%s", status.error_code(), status.error_message().c_str());
    }
}

int GoogleTTSEngine::speak(std::string text, LSHandle* sh, std::string language)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    if(mIsStop){
        mIsStop = false;
    }
    auto channel = grpc::CreateChannel(GOOGLE_APPLICATION_ENDPOINT, mCredentials);

    std::unique_ptr<TextToSpeech::Stub> textToSpeech = TextToSpeech::NewStub(channel);

    SynthesizeSpeechRequest speechRequest;
    SynthesisInput* synthInput = speechRequest.mutable_input();
    synthInput->set_text(text);

    VoiceSelectionParams* voiceSelParams = speechRequest.mutable_voice();
    if(!language.empty())
        voiceSelParams->set_language_code(language);
    else
        voiceSelParams->set_language_code(DEFAULT_LANGUAGE);

    AudioConfig *audio_config = speechRequest.mutable_audio_config();
    audio_config->set_audio_encoding(AudioEncoding::LINEAR16);

    SynthesizeSpeechResponse speechResponse;
    ClientContext context;
    CompletionQueue grpcCallQueue;
    Status gStatus;
    std::unique_ptr<ClientAsyncResponseReader<SynthesizeSpeechResponse> > ttsRpc(textToSpeech->PrepareAsyncSynthesizeSpeech(&context, speechRequest, &grpcCallQueue));
    ttsRpc->StartCall();
    ttsRpc->Finish(&speechResponse, &gStatus, (void*)GOOGLE_TTS_REQUEST_TAG);
    void* got_tag = nullptr;
    bool ok = false;
    do{
        switch(grpcCallQueue.AsyncNext(&got_tag, &ok, std::chrono::system_clock::now() + std::chrono::milliseconds(DEFAULT_DEADLINE_DURATION))){
            case grpc::CompletionQueue::SHUTDOWN:
                LOG_DEBUG("While Waiting For Reply from Google...Got SHUTDOWN");
                break;
            case grpc::CompletionQueue::GOT_EVENT:
                LOG_DEBUG("While Waiting For Reply from Google...Got EVENT");
                break;
            case grpc::CompletionQueue::TIMEOUT:
                LOG_DEBUG("Waiting For Reply from Google...");
                break;
            default:
                LOG_DEBUG("While Waiting For Reply from Google...Got NO CASE MATCH");
                break;
        }
        if(mIsStop){
            LOG_DEBUG("Got Stop While Waiting For Reply From Google");
            break;
        }
    }while(!(ok && (got_tag == (void *)GOOGLE_TTS_REQUEST_TAG)));
    if(mIsStop){
        mIsStop = false;
        return TTSErrors::SPEECH_DATA_CREATION_ERROR;
    }
    if(gStatus.error_code() == grpc::StatusCode::OK)
    {
        std::string synthOutput = speechResponse.audio_content();
        std::ofstream outfile;

        outfile.open(AUDIO_FILE);
        if(outfile.is_open())
            outfile << synthOutput << std::endl;

        outfile.close();
    }
    else
    {
        LOG_DEBUG("Synthesize speech failed: Error %d: %s", gStatus.error_code(), gStatus.error_message().c_str());
        return TTSErrors::SPEECH_DATA_CREATION_ERROR;
    }

    return TTSErrors::ERROR_NONE;
}

void GoogleTTSEngine::start()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
}

void GoogleTTSEngine::stop()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    mIsStop = true;
}

void GoogleTTSEngine::init()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
}

void GoogleTTSEngine::deInit()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);

    if (mCredentials.get()) {
        mCredentials.reset();
    }
}

std::string GoogleTTSEngine::getName()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    return TTS_ENGINE_NAME;
}

void GoogleTTSEngine::setSpeakRate(double rate)
{
    if(rate >= 0.25 && rate <= 4.0){
        mSpeakRate = rate;
    }else{
        LOG_DEBUG("Speak Rate %f Not Supported, Leaving as It is %f", rate, mSpeakRate);
    }
}

void GoogleTTSEngine::setPitch(double pitch)
{
    if(pitch >= -20.0 && pitch <= 20.0){
        mPitch = pitch;
    }else{
        LOG_DEBUG("Pitch Rate %f Not Supported, Leaving as It is %f", pitch, mPitch);
    }
}

double GoogleTTSEngine::getSpeakRate(void) const
{
    return mSpeakRate;
}

double GoogleTTSEngine::getPitch(void) const
{
    return mPitch;
}
