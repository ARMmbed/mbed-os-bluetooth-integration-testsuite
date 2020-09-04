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
#ifndef BLE_CLIAPP_SERIALIZATION_UUID_H_
#define BLE_CLIAPP_SERIALIZATION_UUID_H_

#include "ble/common/UUID.h"
#include "Serialization/JSONOutputStream.h"


/**
 * @brief Construct a UUID from an input string
 * @param str The string containing the UUID, it can have the following format:
 *   - 0xYYYY for 16 bits UUID constructed from the hex value
 *   - XXXXX for 16 bits UUID
 *   - XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX for 128 bits UUID
 * @param uuid The UUID to fill with the result of the conversion
 * @return true if the conversion has succeed and false otherwise
 */
bool fromString(const char* str, UUID& uuid);

/**
 * @brief convert an UUID instance to its string representation
 *
 * @param uuid the uuid to convert
 * @return It depend on the size of the UUID:
 *   - 16 bits UUID: the string will use the format 0xYYYY
 *   - 128 bits UUID: the string will use the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
 */
serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const UUID& uuid);

#endif //BLE_CLIAPP_SERIALIZATION_UUID_H_
