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
#include <algorithm>
#include <cstdio>
#include <stddef.h>

#include "Serialization/UUID.h"
#include "Hex.h"
#include "Serialization/Serializer.h"

using namespace serialization;

namespace {
static bool longUUIDfromString(const char* str, UUID& uuid) {
    UUID::LongUUIDBytes_t uuidData;

    for(size_t i = 0, currentStrIndex = 0; i < sizeof(uuidData); ++i, currentStrIndex += 2) {
        // check for separator
        if(currentStrIndex == 8 || currentStrIndex == 13 || currentStrIndex == 18 || currentStrIndex == 23) {
            if(str[currentStrIndex] != '-') {
                return false;
            }
            ++currentStrIndex;
        }

        if(!asciiHexByteToByte(str[currentStrIndex], str[currentStrIndex + 1], uuidData[i])) {
            return false;
        }
    }

    uuid = UUID(uuidData);

    return true;
}

static JSONOutputStream& serializeLongUUID(JSONOutputStream& os, const uint8_t* data) {
    return os.formatValue(
        "\"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\"",
        data[15], data[14], data[13], data[12], data[11], data[10], data[9], data[8],
        data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]
    );
}

static bool shortUUIDFromString(const char* str, UUID& uuid) {
    UUID::ShortUUIDBytes_t uuidValue;

    if(!fromString(str, uuidValue)) {
        return false;
    }

    uuid = UUID(uuidValue);
    return true;
}

static JSONOutputStream& serializeShortUUID(JSONOutputStream& os, uint16_t uuid) {
    return os << uuid;
}

}

bool fromString(const char* str, UUID& uuid) {
    if(strlen(str) == (sizeof("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX") - 1)) {
        return longUUIDfromString(str, uuid);
    } else {
        return shortUUIDFromString(str, uuid);
    }
}

serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const UUID& uuid) {
    if(uuid.shortOrLong() == UUID::UUID_TYPE_SHORT) {
        return serializeShortUUID(os, uuid.getShortUUID());
    } else {
        return serializeLongUUID(os, uuid.getBaseUUID());
    }
}


