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
#ifndef BLE_CLIAPP_COMMANDS_COMMON_H_
#define BLE_CLIAPP_COMMANDS_COMMON_H_

#include "CLICommand/CommandSuite.h"
#include "ble/BLE.h"
#include <core-util/SharedPointer.h>

/**
 * return the ble instance of this device
 */
inline BLE& get_ble() {
    return BLE::Instance();
}

/**
 * return the Gap instance of this device
 */
inline ble::Gap& gap() {
    return get_ble().gap();
}

/**
 * @brief Return the GattClient of this device.
 */
inline ble::GattClient& client() {
    return get_ble().gattClient();
}

/**
 * @brief Return the instance of the GattServer of this device.
 */
inline ble::GattServer& gattServer() {
    return get_ble().gattServer();
}

/**
 * @brief Return the instance of the security manager of this device.
 */
inline ble::SecurityManager& sm() {
    return get_ble().securityManager();
}

/**
 * @brief Report an error or a success for a command
 * 
 * @param response The response used to report the status.
 * @param err Generic ble error.
 */
inline void reportErrorOrSuccess(const mbed::util::SharedPointer<CommandResponse>& response, ble_error_t err) {
    if(err) {
        response->faillure(err);
    } else {
        response->success();
    }
}

/**
 * @brief Report an error or a success for a command. In case of success, the 
 * parameter res is streamed to the response.
 * 
 * @param response The response used to report the status.
 * @param err Generic ble error.
 * @param res The result to stream in case of success.
 */
template<typename T>
void reportErrorOrSuccess(const mbed::util::SharedPointer<CommandResponse>& response, ble_error_t err, const T& res) {
    if(err) {
        response->faillure(err);
    } else {
        response->success(res);
    }
}


#endif //BLE_CLIAPP_COMMANDS_COMMON_H_
