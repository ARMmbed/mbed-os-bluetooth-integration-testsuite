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
#ifndef BLE_CLIAPP_UTIL_DETAIL_RAII_GATT_ATTRIBUTE_H_
#define BLE_CLIAPP_UTIL_DETAIL_RAII_GATT_ATTRIBUTE_H_

#include <stdint.h>
#include "ble/gatt/GattAttribute.h"
#include "util/Vector.h"

namespace detail {

class RAIIGattAttribute : public GattAttribute {
public:

    RAIIGattAttribute(const UUID& uuid);

    ~RAIIGattAttribute();

    void setValue(const container::Vector<uint8_t>& value);

    bool setMaxLength(uint16_t max);

    void setVariableLength(bool hasVariableLen);

    void releaseAttributeValue();
private:
    RAIIGattAttribute(const RAIIGattAttribute&);
    RAIIGattAttribute& operator=(const RAIIGattAttribute&);
};

}

#endif //BLE_CLIAPP_UTIL_DETAIL_RAII_GATT_ATTRIBUTE_H_
