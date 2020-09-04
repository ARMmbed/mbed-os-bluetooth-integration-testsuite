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
#ifndef BLE_CLIAPP_GATT_CALLABCK_PARAM_TYPES_SERIALIZER_H_
#define BLE_CLIAPP_GATT_CALLABCK_PARAM_TYPES_SERIALIZER_H_

#include "Serialization/Serializer.h"
#include "ble/gatt/DiscoveredCharacteristic.h"

#include "Serialization/JSONOutputStream.h"

/**
 * @brief Serialize the result of GattClient read operation
 * @details The returned object will be a map with the following attributes:
 *     - "connection_handle": The handle of the connection where the read occur
 *     - "attribute_handle": The handle of attribute read
 *     - "offset": Offset of the data read
 *     - "length": lenght of the data read
 *     - "data": The data represented as an hex string
 *
 * @param os The output stream where serialized data will be written
 * @param readResult An instance of GattReadCallbackParams containing the value
 * of a read operation
 * @return os
 */
serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const GattReadCallbackParams& readResult);

/**
 * @brief Serialize the result of GattClient write operation
 * @details The returned object will be a map with the following attributes:
 *     - "connection_handle": The handle of the connection where the write occur
 *     - "attribute_handle": The handle of the attribute written
 *     - "offset": Offset of the data written
 *     - "length": lenght of the data written
 *     - "write_operation_type": The type of write operation
 *
 *  @param os The output stream where serialized data will be written
 * @param writeResult An instance of GattWriteCallbackParams containing the value
 * of a write operation
 * @return os
 */
serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const GattWriteCallbackParams& writeResult);


/**
 * @brief Serialize a Write operation
 *
 * @param os Output stream where serialized data will be written
 * @param writeOperation The write operation to serialize
 * @return os
 */
serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, GattWriteCallbackParams::WriteOp_t writeOperation);

#endif //BLE_CLIAPP_GATT_CALLABCK_PARAM_TYPES_SERIALIZER_H_
