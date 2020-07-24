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

#include <stdio.h>
#include "GapSerializer.h"

// MAC address serializer/deserializer
bool macAddressFromString(const char* str, ble::address_t& val) {
    // since nano libc does not properly handle all format flags, we rely on integers for sscanf
    int tmp[ble::address_t::size()];

    int count = std::sscanf(str, "%x:%x:%x:%x:%x:%x", &tmp[5], &tmp[4], &tmp[3], &tmp[2], &tmp[1], &tmp[0]);

    if(count == ble::address_t::size()) {
        for(size_t i = 0; i < ble::address_t::size(); ++i) {
            val[i] = (uint8_t) tmp[i];
        }
        return true;
    }

    return false;
}

MacAddressString_t macAddressToString(const ble::address_t& src) {
    MacAddressString_t converted;
    snprintf(converted.str, sizeof(converted.str), "%02X:%02X:%02X:%02X:%02X:%02X", src[5], src[4], src[3],src[2],src[1],src[0]);
    return converted;
}
