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

#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <pulse/error.h>
#include <unistd.h>
#include <AudioEngine.h>
#include <PulseAudioEngine.h>
#include <TTSLog.h>

#define BUFSIZE         1024
#define AUDIO_FILE_1    "/tmp/sttsResultOne.pcm"
#define AUDIO_FILE_2    "/tmp/sttsResultTwo.pcm"

static pa_sample_spec sample_spec =
{
    .format = PA_SAMPLE_S16LE,
    .rate = 22050,
    .channels = 1
};

PulseAudioEngine::PulseAudioEngine() : AudioEngine(), mIsStopPlaytts1(false), mIsStopPlaytts2(false), mSimpletts1(nullptr), mSimpletts2(nullptr)
{

}

void PulseAudioEngine::resume()
{

}

void PulseAudioEngine::pause()
{

}

bool PulseAudioEngine::play(int displayId)
{
    std::string audio_file;
    LOG_TRACE("Entering function %s", __FUNCTION__);
    if (mIsStopPlaytts1){
        mIsStopPlaytts1 = false;
    }
    if (mIsStopPlaytts2){
        mIsStopPlaytts2 = false;
    }

    if (displayId)
    {
        if (playAudiotts2(AUDIO_FILE_2))
            return true;
    }
    else
    {
        if (playAudiotts1(AUDIO_FILE_1))
            return true;
    }
    return true;
}
bool PulseAudioEngine::playAudiotts1(std::string audio_file)
{
    int fd;
    int error;
    bool retVal = true;
    LOG_DEBUG("Entering function : PulseAudioEngine::playAudiotts1");
    if ((fd = open(audio_file.c_str(), O_RDONLY)) < 0)
    {
        LOG_DEBUG("Error: File opening failed: %s", strerror(errno));
        retVal= false;
    }
    else
    {
        LOG_DEBUG("PulseAudioEngine::play : audio_file name =  %s", audio_file.c_str());
        if (!(mSimpletts1 = pa_simple_new(NULL, audio_file.c_str(), PA_STREAM_PLAYBACK, "tts1", "playback", &sample_spec, NULL, NULL, &error)))
        {
           LOG_DEBUG("Error: Playback stream creation failed: %s", pa_strerror(error));
           (void)close(fd);
           retVal = false;
        }
        else
        {
            uint8_t buf[BUFSIZE];
            ssize_t rSize;
            do
            {
                // Read the data
                if ((rSize = read(fd, buf, sizeof(buf))) <= 0)
                {
                    if (rSize == 0) // EOF
                       break;
                }
                if (true == mIsStopPlaytts1)
                {
                    LOG_INFO("tts:audio:pulse", 0, "INFO: Got Stop Command While Playing ");
                    break;
                }

                // Play the data
                if (pa_simple_write(mSimpletts1, buf, (size_t) rSize, &error) < 0)
                {
                    LOG_DEBUG("Error: Data playing failed: %s", pa_strerror(error));
                    (void)close(fd);
                    retVal = false;
                    break;
                }
            }while(rSize != 0);
        }
        if(retVal)
        {
            (void)close(fd);
            if (mIsStopPlaytts1)
            {
                mIsStopPlaytts1 = false;
                if (pa_simple_flush(mSimpletts1, &error) < 0)
                {
                    (void)fprintf(stderr, __FILE__": pa_simple_flush() failed: %s\n", pa_strerror(error));
                    LOG_DEBUG("Error: Sample flush failed");
                    retVal = false;
                }
            }
            else
            {
                if (pa_simple_drain(mSimpletts1, &error) < 0)
                {
                    LOG_DEBUG("Error: Sample drain failed: %s", pa_strerror(error));
                    retVal = false;
                }
            }
        }
        if(retVal==true)
        {
            if (mSimpletts1)
            {
                pa_simple_free(mSimpletts1);
                mSimpletts1 = nullptr;
            }
            LOG_DEBUG("PulseAudio Play is completed/n");
        }
    }
    return retVal;
}

bool PulseAudioEngine::playAudiotts2(std::string audio_file)
{
    int fd;
    int error;
    bool returnValue  = true;
    LOG_DEBUG("Entering function : PulseAudioEngine::playAudiotts2");
    if ((fd = open(audio_file.c_str(), O_RDONLY)) < 0)
    {
        LOG_DEBUG("Error: File opening failed: %s", strerror(errno));
        returnValue = false;
    }
    else
    {
        LOG_DEBUG("PulseAudioEngine::play : audio_file name =  %s", audio_file.c_str());

        if (!(mSimpletts2 = pa_simple_new(NULL, audio_file.c_str(), PA_STREAM_PLAYBACK, "tts2", "playback", &sample_spec, NULL, NULL, &error)))
        {
            LOG_DEBUG("Error: Playback stream creation failed: %s", pa_strerror(error));
            (void)close(fd);
            returnValue = false;
        }
        else
        {
            uint8_t buf[BUFSIZE];
            ssize_t rSize;
            do
            {
                // Read the data
                if ((rSize = read(fd, buf, sizeof(buf))) <= 0)
                {
                    if (rSize == 0) // EOF
                    break;
                }
                if (true == mIsStopPlaytts2)
                {
                    LOG_INFO("tts:audio:pulse", 0, "INFO: Got Stop Command While Playing ");
                    break;
                }

                // Play the data
                if (pa_simple_write(mSimpletts2, buf, (size_t) rSize, &error) < 0)
                {
                    LOG_DEBUG("Error: Data playing failed: %s", pa_strerror(error));
                    (void)close(fd);
                    returnValue = false;
	            break;
                }
            }while(rSize != 0);
        }
        if(returnValue)
        {
            (void)close(fd);
            if (mIsStopPlaytts2)
            {
                 mIsStopPlaytts2 = false;
                 if (pa_simple_flush(mSimpletts2, &error) < 0)
                 {
                      (void)fprintf(stderr, __FILE__": pa_simple_flush() failed: %s\n", pa_strerror(error));
                      LOG_DEBUG("Error: Sample flush failed");
                      returnValue = false;
                 }
            }
            else
            {
                if (pa_simple_drain(mSimpletts2, &error) < 0)
                {
                     LOG_DEBUG("Error: Sample drain failed: %s", pa_strerror(error));
                     returnValue = false;
                }
            }
        }
        if(returnValue)
        {
            if (mSimpletts2)
            {
                pa_simple_free(mSimpletts2);
                mSimpletts2 = nullptr;
            }
            LOG_DEBUG("PulseAudio Play is completed/n");
        }
    }
    return returnValue;
}

bool PulseAudioEngine::stop(int displayId)
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    if (displayId)
        mIsStopPlaytts2 = true;
    else
        mIsStopPlaytts1 = true;
    return true;
}

void PulseAudioEngine::init()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
}


void PulseAudioEngine::deInit()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
}
