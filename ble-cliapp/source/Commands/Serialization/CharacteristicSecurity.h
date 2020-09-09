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
#ifndef BLE_CLIAPP_CHARACTERISTIC_SECURITY_H_
#define BLE_CLIAPP_CHARACTERISTIC_SECURITY_H_

#include <stdint.h>

#include "Serialization/Serializer.h"
#include "ble/gatt/GattCharacteristic.h"
#include "Serialization/JSONOutputStream.h"

template<>
struct SerializerDescription<ble::att_security_requirement_t::type> {
    typedef ble::att_security_requirement_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::att_security_requirement_t::NONE, "NONE" },
            { ble::att_security_requirement_t::UNAUTHENTICATED, "UNAUTHENTICATED" },
            { ble::att_security_requirement_t::AUTHENTICATED, "AUTHENTICATED" },
            { ble::att_security_requirement_t::SC_AUTHENTICATED, "SC_AUTHENTICATED" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::att_security_requirement_t::type";
    }
};

#endif //BLE_CLIAPP_CHARACTERISTIC_SECURITY_H_
