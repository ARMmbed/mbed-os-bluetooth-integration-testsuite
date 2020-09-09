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
#ifndef BLE_CLIAPP_SECURITY_MANAGER_SERIALIZER_H_
#define BLE_CLIAPP_SECURITY_MANAGER_SERIALIZER_H_

#include "ble/SecurityManager.h"
#include "ble/common/BLETypes.h"
#include "Serialization/JSONOutputStream.h"
#include "Serialization/Serializer.h"

using ble::Gap;
using ble::GattClient;
using ble::GattServer;
using ble::SecurityManager;

template<>
struct SerializerDescription<SecurityManager::SecurityIOCapabilities_t> {
    typedef SecurityManager::SecurityIOCapabilities_t type;

    static const ConstArray<ValueToStringMapping<SecurityManager::SecurityIOCapabilities_t> > mapping() {
        static const ValueToStringMapping<SecurityManager::SecurityIOCapabilities_t> map[] = {
            { SecurityManager::IO_CAPS_DISPLAY_ONLY, "IO_CAPS_DISPLAY_ONLY" },
            { SecurityManager::IO_CAPS_DISPLAY_YESNO, "IO_CAPS_DISPLAY_YESNO" },
            { SecurityManager::IO_CAPS_KEYBOARD_ONLY, "IO_CAPS_KEYBOARD_ONLY" },
            { SecurityManager::IO_CAPS_NONE, "IO_CAPS_NONE" },
            { SecurityManager::IO_CAPS_KEYBOARD_DISPLAY, "IO_CAPS_KEYBOARD_DISPLAY" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown SecurityManager::SecurityIOCapabilities_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, SecurityManager::SecurityIOCapabilities_t type) {
    return os << toString(type);
}

template<>
struct SerializerDescription<SecurityManager::SecurityCompletionStatus_t> {
    typedef SecurityManager::SecurityCompletionStatus_t type;

    static const ConstArray<ValueToStringMapping<SecurityManager::SecurityCompletionStatus_t> > mapping() {
        static const ValueToStringMapping<SecurityManager::SecurityCompletionStatus_t> map[] = {
            { SecurityManager::SEC_STATUS_SUCCESS, "SEC_STATUS_SUCCESS" },
            { SecurityManager::SEC_STATUS_TIMEOUT, "SEC_STATUS_TIMEOUT" },
            { SecurityManager::SEC_STATUS_PDU_INVALID, "SEC_STATUS_PDU_INVALID" },
            { SecurityManager::SEC_STATUS_PASSKEY_ENTRY_FAILED, "SEC_STATUS_PASSKEY_ENTRY_FAILED" },
            { SecurityManager::SEC_STATUS_OOB_NOT_AVAILABLE, "SEC_STATUS_OOB_NOT_AVAILABLE" },
            { SecurityManager::SEC_STATUS_AUTH_REQ, "SEC_STATUS_AUTH_REQ" },
            { SecurityManager::SEC_STATUS_CONFIRM_VALUE, "SEC_STATUS_CONFIRM_VALUE" },
            { SecurityManager::SEC_STATUS_PAIRING_NOT_SUPP, "SEC_STATUS_PAIRING_NOT_SUPP" },
            { SecurityManager::SEC_STATUS_ENC_KEY_SIZE, "SEC_STATUS_ENC_KEY_SIZE" },
            { SecurityManager::SEC_STATUS_SMP_CMD_UNSUPPORTED, "SEC_STATUS_SMP_CMD_UNSUPPORTED" },
            { SecurityManager::SEC_STATUS_UNSPECIFIED, "SEC_STATUS_UNSPECIFIED" },
            { SecurityManager::SEC_STATUS_REPEATED_ATTEMPTS, "SEC_STATUS_REPEATED_ATTEMPTS" },
            { SecurityManager::SEC_STATUS_INVALID_PARAMS, "SEC_STATUS_INVALID_PARAMS" },
            { SecurityManager::SEC_STATUS_DHKEY_CHECK_FAILED, "SEC_STATUS_DHKEY_CHECK_FAILED" },
            { SecurityManager::SEC_STATUS_COMPARISON_FAILED, "SEC_STATUS_COMPARISON_FAILED" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown SecurityManager::SecurityCompletionStatus_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, SecurityManager::SecurityCompletionStatus_t type) {
    return os << toString(type);
}

struct SecurityManager_link_encryption_t : ble::link_encryption_t
{
    typedef ble::link_encryption_t::type type;
    SecurityManager_link_encryption_t(type value) : ble::link_encryption_t(value) { }

    SecurityManager_link_encryption_t(ble::link_encryption_t value) : ble::link_encryption_t(value) { }

    // Add default constructor
    SecurityManager_link_encryption_t() : ble::link_encryption_t(ble::link_encryption_t::NOT_ENCRYPTED) {}
};

template<>
struct SerializerDescription<SecurityManager_link_encryption_t> {
    typedef SecurityManager_link_encryption_t type;

    static const ConstArray<ValueToStringMapping<SecurityManager_link_encryption_t> > mapping() {
        static const ValueToStringMapping<SecurityManager_link_encryption_t> map[] = {
            { SecurityManager_link_encryption_t::NOT_ENCRYPTED, "NOT_ENCRYPTED" },
            { SecurityManager_link_encryption_t::ENCRYPTION_IN_PROGRESS, "ENCRYPTION_IN_PROGRESS" },
            { SecurityManager_link_encryption_t::ENCRYPTED, "ENCRYPTED" },
            { SecurityManager_link_encryption_t::ENCRYPTED_WITH_MITM, "ENCRYPTED_WITH_MITM" },
            { SecurityManager_link_encryption_t::ENCRYPTED_WITH_SC_AND_MITM, "ENCRYPTED_WITH_SC_AND_MITM" },
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown SecurityManager_link_encryption_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::link_encryption_t type) {
    return os << toString(SecurityManager_link_encryption_t(type));
}

struct SecurityManagerPasskey_t
{
    SecurityManager::Passkey_t value;

    operator SecurityManager::Passkey_t&() {
        return value;
    }

    operator const SecurityManager::Passkey_t&() const {
        return value;
    }
};

static inline bool fromString(const char* str, SecurityManagerPasskey_t& value) {
    // length should be 6!
    if( strlen(str) != sizeof(SecurityManager::Passkey_t) ) {
        return false;
    }

    // Validate and populate
    for (size_t i = 0; i < sizeof(SecurityManager::Passkey_t); ++i) {
        if((str[i] > '9') || (str[i] < '0')) {
            return false;
        }
        value.value[i] = str[i];
    }

    return true;
}

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const SecurityManagerPasskey_t passkey) {
    char passkey_str[sizeof(SecurityManagerPasskey_t) + 1] = {0};
    memcpy(passkey_str, passkey, sizeof(SecurityManagerPasskey_t));
    return os << passkey_str;
}

#endif //BLE_CLIAPP_SECURITY_MANAGER_SERIALIZER_H_
