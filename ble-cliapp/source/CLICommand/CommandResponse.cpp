/* Copyright (c) 2015-2020 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <functional>
#include "CommandResponse.h"

using namespace serialization;

namespace {

void dummyOnClose(const CommandResponse*) { }

}

CommandResponse::CommandResponse() :
    onClose(dummyOnClose), out(), statusCode(), nameSet(0), argumentsSet(0),
    statusCodeSet(0), resultStarted(0), closed(0) {
    // start the output
    out << startObject;
}

CommandResponse::~CommandResponse() {
    close();
}

bool CommandResponse::setCommandName(const char* name) {
    if(nameSet) {
        return false;
    }

    out << key("name") << name;
    nameSet = 1;
    return true;
}

bool CommandResponse::setArguments(const CommandArgs& args) {
    if(argumentsSet) {
        return false;
    }

    out << key("arguments") << startArray;
    for(size_t i = 0; i < args.count(); ++i) {
        out << args[i];
    }
    out << endArray;
    argumentsSet = 1;
    return true;
}

bool CommandResponse::setStatusCode(StatusCode_t sc) {
    if(statusCodeSet) {
        return false;
    }

    statusCode = sc;

    out << key("status") << (int32_t) statusCode;
    statusCodeSet = 1;
    return true;
}

CommandResponse::StatusCode_t CommandResponse::getStatusCode() const {
    return statusCode;
}

JSONOutputStream& CommandResponse::getResultStream() {
    if(!resultStarted) {
        out << key(statusCode < 0 ? "error" : "result");
        resultStarted = 1;
    }

    return out;
}

void CommandResponse::setOnClose(const OnClose_t& onCloseCallBack) {
        onClose = onCloseCallBack;
}

void CommandResponse::close() {
    if(closed) {
        return;
    }

    out << endObject;
    out.flush();
    closed = 1;

    onClose(this);
}

bool CommandResponse::isClosed() {
    return closed;
}

bool CommandResponse::invalidParameters(const char* msg) {
    return setStatusCodeAndMessage(INVALID_PARAMETERS, msg);
}

bool CommandResponse::notImplemented(const char* msg) {
    return setStatusCodeAndMessage(COMMAND_NOT_IMPLEMENTED, msg);
}

bool CommandResponse::faillure(const char* msg) {
    return setStatusCodeAndMessage(FAIL, msg);
}

bool CommandResponse::success(const char* msg) {
    return setStatusCodeAndMessage(SUCCESS, msg);
}

bool CommandResponse::setStatusCodeAndMessage(StatusCode_t sc, const char* msg) {
    if(!setStatusCode(sc)) {
        return false;
    }

    if(msg) {
        getResultStream() << msg;
    }

    return true;
}
