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
#include "AsyncProcedure.h"
#include "../CommandEventQueue.h"

AsyncProcedure::AsyncProcedure(const CommandResponsePtr& res, uint32_t t) :
    response(res), timeoutHandle(NULL), timeout(t) {
}

AsyncProcedure::~AsyncProcedure() {
    if(timeoutHandle) {
        getCLICommandEventQueue()->cancel(timeoutHandle);
    }
}

void AsyncProcedure::start() {
    // register the timeout callback
    timeoutHandle = getCLICommandEventQueue()->post_in(
        &AsyncProcedure::whenTimeout,
        this,
        timeout
    );

    if(doStart() == false) {
        terminate();
        return;
    }

}

void AsyncProcedure::terminate() {
    delete this;
}

void AsyncProcedure::whenTimeout() {
    // detach whenConnected handle
    timeoutHandle = NULL;
    doWhenTimeout();
    terminate();
}

void AsyncProcedure::doWhenTimeout() {
    response->faillure("timeout");
}
