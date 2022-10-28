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

set(inc ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB GOOGLE_SOURCE
        file(GLOB_RECURSE API ${GOOGLEAPIS_PATH}/api/*.cc)
        file(GLOB_RECURSE EXPERIMENTAL ${GOOGLEAPIS_PATH}/api/experimental/*.cc)
        file(GLOB_RECURSE TEXTTOSPEECH ${GOOGLEAPIS_PATH}/cloud/texttospeech/v1/*.cc)
)
set(src ${CMAKE_CURRENT_SOURCE_DIR}/GoogleTTSEngine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GoogleTTSEngineFactory.cpp
    ${GOOGLE_SOURCE}
)
 set(deps
     grpc
     grpc++
     protobuf
)
TTS_ENGINE(google "${src}" "${inc}" "${deps}")
