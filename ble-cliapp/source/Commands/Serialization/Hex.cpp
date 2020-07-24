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
#include <cstdlib>
#include <stdint.h>
#include <cstdio>

#include "Hex.h"

using std::size_t;
using std::strtol;
using std::strlen;

bool asciiHexByteToByte(char msb, char lsb, uint8_t& result) {
    char hexStr[] = { msb, lsb, 0 };
    char* end;

    long conversionResult = strtol(hexStr, &end, 16);
    if (end == hexStr) {
        return false;
    }

    result = (uint8_t) conversionResult;
    return true;
}

serialization::JSONOutputStream& serializeRawDataToHexString(serialization::JSONOutputStream& os, const uint8_t* data, size_t length) {
    os.put('"');
    for (size_t i = 0; i < length; ++i) {
        os.format("%02X", data[i]);
    }
    os.put('"');
    os.commitValue();

    return os;
}

container::Vector<uint8_t> hexStringToRawData(const char* data) {
    container::Vector<uint8_t> result;

    size_t strLen = strlen(data);

    if (strLen % 2) {
        return result;
    }

    for(size_t i = 0; i < strLen; i += 2) {
        uint8_t convertedByte;
        if(asciiHexByteToByte(data[i], data[i + 1], convertedByte) == false) {
            // this is for RVO
            result = container::Vector<uint8_t>();
            return result;
        }
        result.push_back(convertedByte);
    }

    return result;
}
