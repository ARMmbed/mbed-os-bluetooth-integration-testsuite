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
#include "RAIIGattAttribute.h"
#include "util/HijackMember.h"

HIJACK_MEMBER(_valuePtr_accessor, uint8_t* GattAttribute::*, &GattAttribute::_valuePtr);
HIJACK_MEMBER(_lenMax_accessor, uint16_t GattAttribute::*, &GattAttribute::_lenMax);
HIJACK_MEMBER(_len_accessor, uint16_t GattAttribute::*, &GattAttribute::_len);
HIJACK_MEMBER(_hasVariableLen_accessor, bool GattAttribute::*, &GattAttribute::_hasVariableLen);

namespace detail {

RAIIGattAttribute::RAIIGattAttribute(const UUID& uuid)  :
    GattAttribute(uuid, NULL, 0, 0, true) {
}

RAIIGattAttribute::~RAIIGattAttribute() {
    if(this->*_valuePtr_accessor) {
        delete[] (this->*_valuePtr_accessor);
    }
}

void RAIIGattAttribute::setValue(const container::Vector<uint8_t>& newValue) {
    uint8_t*& valuePtr = (this->*_valuePtr_accessor);
    uint16_t& len = this->*_len_accessor;
    uint16_t& maxLen = this->*_lenMax_accessor;

    if(valuePtr) {
        delete[] valuePtr;
        valuePtr = NULL;
        len = 0;
    }

    if(newValue.size()) {
        valuePtr = new uint8_t[newValue.size()]();
        std::memcpy(valuePtr, newValue.cbegin(), newValue.size());
        len = newValue.size();
    }

    if(maxLen < len) {
        maxLen = len;
    }
}

bool RAIIGattAttribute::setMaxLength(uint16_t max) {
    if(max < this->*_len_accessor) {
        return false;
    }

    this->*_lenMax_accessor = max;
    return true;
}

void RAIIGattAttribute::setVariableLength(bool hasVariableLen) {
    this->*_hasVariableLen_accessor = hasVariableLen;
}

void RAIIGattAttribute::releaseAttributeValue() {
    uint8_t*& valuePtr = (this->*_valuePtr_accessor);
    uint16_t& len = this->*_len_accessor;

    if(valuePtr) {
        delete[] valuePtr;
        valuePtr = NULL;
        len = 0;
    }
}

}
