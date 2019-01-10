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
#define AUDIO_FILE      "/tmp/sttsResult.pcm"

static pa_sample_spec sample_spec =
{
    .format = PA_SAMPLE_S16LE,
    .rate = 22050,
    .channels = 1
};

PulseAudioEngine::PulseAudioEngine() : AudioEngine(),mIsStopPlay(false)
{
    mSimple = nullptr;
}

void PulseAudioEngine::resume()
{

}

void PulseAudioEngine::pause()
{

}

bool PulseAudioEngine::play()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    if(mIsStopPlay){
        mIsStopPlay = false;
    }
    int error;
    int fd;

    if ((fd = open(AUDIO_FILE, O_RDONLY)) < 0)
    {
        LOG_DEBUG("Error: File opening failed: %s", strerror(errno));
        return false;
    }

    if (dup2(fd, STDIN_FILENO) < 0)
    {
        LOG_DEBUG("Error: File duplication failed: %s", strerror(errno));
        return false;
    }
    close(fd);

    // Create a new playback stream
    if (!(mSimple = pa_simple_new(NULL, AUDIO_FILE, PA_STREAM_PLAYBACK, "ptts", "playback", &sample_spec, NULL, NULL, &error)))
    {
        LOG_DEBUG("Error: Playback stream creation failed: %s", pa_strerror(error));
        return false;
    }

    for (;;)
    {
        uint8_t buf[BUFSIZE];
        ssize_t rSize;

        // Read the data
        if ((rSize = read(STDIN_FILENO, buf, sizeof(buf))) <= 0)
       {
            if (rSize == 0) // EOF
                break;
            LOG_DEBUG("Error: Data read failed: %s", strerror(errno));
            return false;
        }

        if( mIsStopPlay)
        {
            LOG_INFO("tts:audio:pulse", 0, "INFO: Got Stop Command While Playing ");
            break;
        }

        // Play the data
        if (pa_simple_write(mSimple, buf, (size_t) rSize, &error) < 0)
        {
            LOG_DEBUG("Error: Data playing failed: %s", pa_strerror(error));
            return false;
        }
    }

    if( mIsStopPlay)
    {
        mIsStopPlay = false;
        if(pa_simple_flush(mSimple, &error) < 0)
        {
             fprintf(stderr, __FILE__": pa_simple_flush() failed: %s\n", pa_strerror(error));
             LOG_DEBUG("Error: Sample flush failed");
             return false;
        }
    }else{
        // Make sure that every single sample was played
        if (pa_simple_drain(mSimple, &error) < 0)
        {
            LOG_DEBUG("Error: Sample drain failed: %s", pa_strerror(error));
            return false;
        }
    }

    if (mSimple)
    {
        pa_simple_free(mSimple);
        mSimple = nullptr;
    }

    return true;
}


bool PulseAudioEngine::stop()
{
    LOG_TRACE("Entering function %s", __FUNCTION__);
    mIsStopPlay = true;
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
