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
#ifndef BLE_CLIAPP_UTIL_DETAIL_RAII_GATT_CHARACTERISTIC_H_
#define BLE_CLIAPP_UTIL_DETAIL_RAII_GATT_CHARACTERISTIC_H_

#include <stdint.h>
#include "ble/gatt/GattCharacteristic.h"
#include "util/Vector.h"
#include "RAIIGattAttribute.h"

namespace detail {

class RAIIGattCharacteristic : public GattCharacteristic {
public:
    RAIIGattCharacteristic(const UUID& uuid);

    ~RAIIGattCharacteristic();

    void setValue(const container::Vector<uint8_t>& value);

    bool setMaxLength(uint16_t max);

    void setVariableLength(bool hasVariableLen);

    void setProperties(uint8_t newProperties);

    void setSecurity(GattCharacteristic::SecurityRequirement_t::type read_security,
        GattCharacteristic::SecurityRequirement_t::type write_security,
        GattCharacteristic::SecurityRequirement_t::type update_security);

    // note: the attribute will be moved and owned by this data structure ...
    void addDescriptor(RAIIGattAttribute* descriptor);

    void releaseAttributesValue();
private:
    RAIIGattCharacteristic(const RAIIGattCharacteristic&);
    RAIIGattCharacteristic& operator=(const RAIIGattCharacteristic&);
};

}

#endif //BLE_CLIAPP_UTIL_DETAIL_RAII_GATT_CHARACTERISTIC_H_
