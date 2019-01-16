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


#include "StatusHandler.h"

void StatusHandler::Register(StatusObserver* ptrStatusObserver) {
    if(ptrStatusObserver)
        _obStatusObserver->push_front(ptrStatusObserver);
}

void StatusHandler::Unregister(StatusObserver* ptrStatusObserver) {

    if(ptrStatusObserver)
        _obStatusObserver->remove(ptrStatusObserver);

}

void StatusHandler::Notify(Parameters* paramList, LS::Message &message) {
    for (std::list<StatusObserver*>::iterator it = _obStatusObserver->begin(); it != _obStatusObserver->end(); it++)
        (*it)->update(paramList, message);
}

StatusHandler::StatusHandler() {

    _obStatusObserver = new std::list<StatusObserver*>();

}

StatusHandler::StatusHandler(const StatusHandler& obj) {
    _obStatusObserver = new std::list<StatusObserver *>();
    *_obStatusObserver = *obj._obStatusObserver;
}


StatusHandler::~StatusHandler() {
    _obStatusObserver->clear();
    delete _obStatusObserver;
    _obStatusObserver = nullptr;
}

StatusHandler* StatusHandler::GetInstance()
{
   static StatusHandler _StatusHandler;
   return &_StatusHandler;

}
