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
#ifndef BLE_CLIAPP_SERIALIZATION_HEX_H_
#define BLE_CLIAPP_SERIALIZATION_HEX_H_

#include "util/Vector.h"
#include "Serialization/JSONOutputStream.h"
#include "platform/Span.h"

/**
 * @brief Convert the string representation of a byte in asci hexadecimal
 * characters to a byte.
 *
 * @param msb The ascii character representing the msb part of the byte
 * @param lsb The ascii character representing the lsb part of the byte
 * @param result the variable which will be filled with the result if the
 * conversion succeed.
 * @return true if the conversion has been made and false otherwise.
 */
bool asciiHexByteToByte(char msb, char lsb, uint8_t& result);

/**
 * @brief convert an arary of bytes to its representation as an hexadecimal string

 * @param data The data to convert
 * @param length The length of the data to convert
 *
 * @return The data as an hexadecimal string
 */
serialization::JSONOutputStream& serializeRawDataToHexString(serialization::JSONOutputStream& os, const uint8_t* data, std::size_t length);

/**
 * @brief Convert the string representation of bytes in ascii hexadecimal to
 * an array of bytes.
 *
 * @param data data to convert, it should be null terminated
 *
 * @return The converted data, if input data were invalid, the returned Vector is
 * invalid.
 */
container::Vector<uint8_t> hexStringToRawData(const char* data);

typedef container::Vector<uint8_t> RawData_t;

static inline bool fromString(const char* str, RawData_t& value) { 
    container::Vector<uint8_t> tmp = hexStringToRawData(str);
    if (tmp.size() == 0) { 
        return false;
    }
    value = tmp;
    return true;
}

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, mbed::Span<const uint8_t> data) {
    return serializeRawDataToHexString(os, data.data(), data.size());
}

#endif //BLE_CLIAPP_SERIALIZATION_HEX_H_
