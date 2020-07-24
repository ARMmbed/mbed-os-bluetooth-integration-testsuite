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

#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "Serializer.h"

using std::strtoul;
using std::strtol;
using std::strcmp;

using std::numeric_limits;

bool fromString(const char* str, int8_t& val) {
    char* end;
    long tmp = strtol(str, &end, 0);
    if(str == end) {
        return false;
    }

    if(tmp < numeric_limits<int8_t>::min() || tmp > numeric_limits<int8_t>::max()) {
        return false;
    }

    val = (int8_t) tmp;
    return true;
}

bool fromString(const char* str, uint8_t& val) {
    char* end;
    long tmp = strtol(str, &end, 0);
    if(str == end) {
        return false;
    }

    if(tmp < numeric_limits<uint8_t>::min() || tmp > numeric_limits<uint8_t>::max()) {
        return false;
    }

    val = (uint8_t) tmp;
    return true;
}


bool fromString(const char* str, uint16_t& val) {
    char* end;
    unsigned long tmp = strtoul(str, &end, 0);
    if(str == end) {
        return false;
    }

    if(tmp > numeric_limits<uint16_t>::max()) {
        return false;
    }

    val = (uint16_t) tmp;
    return true;
}

bool fromString(const char* str, unsigned int& val) {
    char* end;
    unsigned long tmp = strtoul(str, &end, 0);
    if(str == end) {
        return false;
    }

    val = (uint32_t) tmp;
    return true;
}

bool fromString(const char* str, long unsigned int& val) {
    return fromString(str, (unsigned int&) val);
}


bool fromString(const char* str, bool& val) {
    if(strcmp(str, "true") == 0) {
        val = true;
        return true;
    }

    if(strcmp(str, "false") == 0) {
        val = false;
        return true;
    }

    if(strcmp(str, "1") == 0) {
        val = true;
        return true;
    }

    if(strcmp(str, "0") == 0) {
        val = false;
        return true;
    }

    return false;
}
