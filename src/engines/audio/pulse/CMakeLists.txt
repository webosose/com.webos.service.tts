# Copyright (c) 2018-2019 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

include(FindPkgConfig)

pkg_check_modules(PULSEAUDIO REQUIRED libpulse-mainloop-glib libpulse libpulse-simple)
webos_add_compiler_flags(ALL ${PULSEAUDIO_CFLAGS})

set(inc
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PULSEAUDIO_INCLUDE_DIRS}
    )
set(src
    ${CMAKE_CURRENT_SOURCE_DIR}/PulseAudioEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PulseAudioEngineFactory.cpp
    )
set(deps ${PULSEAUDIO_LDFLAGS})
TTS_ENGINE(audio "${src}" "${inc}" "${deps}")
