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
#ifndef BLE_CLIAPP_CHARACTERISTIC_PROPERTIES_H_
#define BLE_CLIAPP_CHARACTERISTIC_PROPERTIES_H_

#include <stdint.h>

#include "Serialization/Serializer.h"
#include "ble/gatt/GattCharacteristic.h"
#include "Serialization/JSONOutputStream.h"


template<>
struct SerializerDescription<GattCharacteristic::Properties_t> {
    typedef GattCharacteristic::Properties_t type;

    static const ConstArray<ValueToStringMapping<GattCharacteristic::Properties_t> > mapping() {
        static const ValueToStringMapping<GattCharacteristic::Properties_t> map[] = {
            { GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_BROADCAST, "broadcast" },
            { GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ, "read" },
            { GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE, "writeWoResp" },
            { GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE, "write" },
            { GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY, "notify" },
            { GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE, "indicate" },
            { GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_AUTHENTICATED_SIGNED_WRITES, "authSignedWrite" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown GattCharacteristic::Properties_t";
    }
};


static inline bool characteristicPropertiesFromStrings(const ConstArray<const char*>& strings, uint8_t& result) {
    uint8_t properties = GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NONE;

    for(std::size_t i = 0; i < strings.count(); ++i) {
        GattCharacteristic::Properties_t prop;
        if(!fromString(strings[i], prop)) {
            return false;
        }

        properties |= prop;
    }

    result = properties;
    return true;
}

static inline serialization::JSONOutputStream& serializeCharacteristicProperties(
    serialization::JSONOutputStream& os, uint8_t properties) {
    using namespace serialization;

    os << startArray;
    for(int i = 0; i < 8; ++i) {
        GattCharacteristic::Properties_t property = static_cast<GattCharacteristic::Properties_t>(properties & (1 << i));
        if(property) {
            os << toString(property);
        }
    }
    os << endArray;

    return os;
}


#endif //BLE_CLIAPP_CHARACTERISTIC_PROPERTIES_H_
