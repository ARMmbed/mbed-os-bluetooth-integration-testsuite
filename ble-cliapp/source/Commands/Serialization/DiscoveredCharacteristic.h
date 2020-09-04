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
#ifndef BLE_CLIAPP_DISCOVERED_CHARACTERISTIC_SERIALIZER_H_
#define BLE_CLIAPP_DISCOVERED_CHARACTERISTIC_SERIALIZER_H_

#include "Serialization/Serializer.h"
#include "ble/gatt/DiscoveredCharacteristic.h"
#include "Serialization/JSONOutputStream.h"

/**
 * @brief Serialize a discovered characteristic intance into a JSON stream.
 * @details The serialized object will be a map with the following attributes:
 *     - "UUID": The uuid of this gatt characteristic
 *     - "properties": An array of dynamic::Values containing the properties
 *                     associated with this characteristic
 *     - "start_handle": The first handle of the characteristic description
 *     - "value_handle": The handle containing the value of this characteristic
 *     - "end_handle": The last handle of the characteristic description
 *
 * @param os The output stream where the instance will be serialized
 * @param characteristic An instance of DiscoveredCharacteristic
 * @return os
 */
serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const DiscoveredCharacteristic& characteristic);

/**
 * @brief Serialize properties of discovered characteristic in a JSON stream
 * @details The returned object will be an array of the properties, the list of
 * the availables values which can populate the array are:
 *   - "broadcast": if the broadcasting the value permitted.
 *   - "read": if reading the value is permitted.
 *   - "writeWoResp": if writing the value with Write Command is permitted.
 *   - "write": if writing the value with Write Request permitted.
 *   - "notify": if notifications of the value is permitted
 *   - "indicate": if indication of the value is permitted
 *   - "authSignedWrite": if writing the value with Signed Write Command is permitted.
 *
 * @param os The output stream where the instance will be serialized
 * @param properties The properties to convert
 * @return os
 */
serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const DiscoveredCharacteristic::Properties_t& properties);


#endif //BLE_CLIAPP_DISCOVERED_CHARACTERISTIC_SERIALIZER_H_
