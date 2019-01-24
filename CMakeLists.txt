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

cmake_minimum_required(VERSION 2.8.7)
project (tts-service)

add_definitions(-DNOT_SUPPORT_GFLAGS)
include(webOS/webOS)

webos_modules_init(1 0 0 QUALIFIER RC4)
webos_component(1 0 0)

set(ENGINES "google=src/engines/tts/google,audio=src/engines/audio/pulse")

macro(TTS_ENGINE name src inc deps)
  set(ENGINE_INC ${ENGINE_INC} ${inc} PARENT_SCOPE)
  set(ENGINE_SRC ${ENGINE_SRC} ${src} PARENT_SCOPE)
  set(ENGINE_LD_FLAGS  ${ENGINE_LD_FLAGS} ${deps} PARENT_SCOPE)
endmacro()

macro(REGISTER_ENGINE name)
  string(FIND ${name} "=" pos)
  if(pos GREATER -1)
    string(REPLACE "=" ";" list ${name})
    list(GET list 0 p_name)
    list(GET list 1 p_path)
    message(STATUS "REGISTER_ENGINE name='${p_name}' path='${p_path}'")
  endif()

  if (p_path)
    add_subdirectory(${p_path})
  endif()
endmacro()

if(ENGINES)
  string(REPLACE "," ";" ENGINEs ${ENGINES})
  foreach(entry ${ENGINEs})
    REGISTER_ENGINE(${entry})
  endforeach()
endif()

option (USE_PMLOG "Enable PmLogLib logging" ON)

include_directories(${ENGINE_INC})

include(FindPkgConfig)

pkg_check_modules(LS2 REQUIRED luna-service2++)
include_directories(${LS2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LS2_CFLAGS_OTHER})

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
include_directories(${PBNJSON_CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_CPP_CFLAGS_OTHER})

if (USE_PMLOG)
    pkg_check_modules(PMLOGLIB REQUIRED PmLogLib)
    include_directories(${PMLOGLIB_INCLUDE_DIRS})
    webos_add_compiler_flags(ALL ${PMLOGLIB_CFLAGS_OTHER})
endif()

include_directories(src/include)
include_directories(src/utils)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

file(GLOB TTS_SOURCE
	${CMAKE_SOURCE_DIR}/src/core/*.cpp
	${CMAKE_SOURCE_DIR}/src/luna/*.cpp
	${CMAKE_SOURCE_DIR}/src/core/*.cpp
	${CMAKE_SOURCE_DIR}/src/utils/*.cpp
	${CMAKE_SOURCE_DIR}/src/tts_main.cpp
)

add_executable(${CMAKE_PROJECT_NAME}  ${TTS_SOURCE} ${ENGINE_SRC})

set(LIBS
    ${LS2_LDFLAGS}
    ${GLIB2_LDFLAGS}
    ${PMLOGLIB_LDFLAGS}
    ${ENGINE_LD_FLAGS}
    ${PBNJSON_CPP_LDFLAGS}
    ${JSON_LDFLAGS}
    -lpthread
    -Wl,--no-as-needed
    )

install(FILES files/conf/tts_config.json DESTINATION ${WEBOS_INSTALL_WEBOS_SYSCONFDIR}/tts)

target_link_libraries (${CMAKE_PROJECT_NAME} ${LIBS})

install(TARGETS ${CMAKE_PROJECT_NAME}
        DESTINATION sbin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE)

webos_build_system_bus_files()
webos_build_configured_file(files/systemd/com.webos.service.tts.service SYSCONFDIR systemd/system)
