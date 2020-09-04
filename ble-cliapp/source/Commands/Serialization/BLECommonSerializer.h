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
#ifndef BLE_CLIAPP_BLE_COMMON_SERIALIZER_H_
#define BLE_CLIAPP_BLE_COMMON_SERIALIZER_H_

#include "Serialization/Serializer.h"
#include "ble/common/blecommon.h"
#include "Serialization/JSONOutputStream.h"

template<>
struct SerializerDescription<ble_error_t> {
    typedef ble_error_t type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { BLE_ERROR_NONE, "BLE_ERROR_NONE" },
            { BLE_ERROR_BUFFER_OVERFLOW, "BLE_ERROR_BUFFER_OVERFLOW" },
            { BLE_ERROR_NOT_IMPLEMENTED, "BLE_ERROR_NOT_IMPLEMENTED" },
            { BLE_ERROR_PARAM_OUT_OF_RANGE, "BLE_ERROR_PARAM_OUT_OF_RANGE" },
            { BLE_ERROR_INVALID_PARAM, "BLE_ERROR_INVALID_PARAM" },
            { BLE_STACK_BUSY, "BLE_STACK_BUSY" },
            { BLE_ERROR_INVALID_STATE, "BLE_ERROR_INVALID_STATE" },
            { BLE_ERROR_NO_MEM, "BLE_ERROR_NO_MEM" },
            { BLE_ERROR_OPERATION_NOT_PERMITTED, "BLE_ERROR_OPERATION_NOT_PERMITTED" },
            { BLE_ERROR_INITIALIZATION_INCOMPLETE, "BLE_ERROR_INITIALIZATION_INCOMPLETE" },
            { BLE_ERROR_ALREADY_INITIALIZED, "BLE_ERROR_ALREADY_INITIALIZED " },
            { BLE_ERROR_UNSPECIFIED, "BLE_ERROR_UNSPECIFIED" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble_error_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble_error_t err) {
    return os << toString(err);
}

template<>
struct SerializerDescription<HVXType_t> {
    typedef HVXType_t type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { BLE_HVX_NOTIFICATION, "BLE_HVX_NOTIFICATION" },
            { BLE_HVX_INDICATION, "BLE_HVX_INDICATION" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown HVXType_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, HVXType_t type) {
    return os << toString(type);
}

#endif //BLE_CLIAPP_BLE_COMMON_SERIALIZER_H_
