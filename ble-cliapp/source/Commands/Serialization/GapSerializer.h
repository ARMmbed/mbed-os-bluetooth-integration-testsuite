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
#ifndef BLE_CLIAPP_GAP_SERIALIZER_H_
#define BLE_CLIAPP_GAP_SERIALIZER_H_

#include <stdio.h>
#include "ble/Gap.h"
#include "Serialization/Serializer.h"
#include "Serialization/JSONOutputStream.h"

template<>
struct SerializerDescription<ble::phy_t::type> {
    typedef ble::phy_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::phy_t::LE_1M, "LE_1M" },
            { ble::phy_t::LE_2M, "LE_2M" },
            { ble::phy_t::LE_CODED, "LE_CODED" },
            { ble::phy_t::NONE, "NONE" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown phy_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::phy_t type) {
    return os << toString((ble::phy_t::type) type.value());
}

template<>
struct SerializerDescription<ble::peer_address_type_t> {
    typedef ble::peer_address_type_t type;

    static const ConstArray<ValueToStringMapping<ble::peer_address_type_t> > mapping() {
        static const ValueToStringMapping<ble::peer_address_type_t> map[] = {
            { ble::peer_address_type_t::PUBLIC, "PUBLIC" },
            { ble::peer_address_type_t::RANDOM, "RANDOM" },
            { ble::peer_address_type_t::PUBLIC_IDENTITY, "PUBLIC_IDENTITY" },
            { ble::peer_address_type_t::RANDOM_STATIC_IDENTITY, "RANDOM_STATIC_IDENTITY" },
            { ble::peer_address_type_t::ANONYMOUS, "ANONYMOUS" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::peer_address_type_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::peer_address_type_t addr) {
    return os << toString(addr);
}

struct MacAddressString_t {
    char str[sizeof("XX:XX:XX:XX:XX:XX")];
};

// MAC address serializer/deserializer
bool macAddressFromString(const char* str, ble::address_t& val);
MacAddressString_t macAddressToString(const ble::address_t& src);

struct MacAddress_t { 
    ble::address_t value;

    operator ble::address_t&() {
        return value;
    }

    operator const ble::address_t&() const {
        return value;
    }
};

static inline bool fromString(const char* str, MacAddress_t& val) { 
    return macAddressFromString(str, val);
}

static inline bool fromString(const char* str, ble::address_t& val) {
    return macAddressFromString(str, val);
}

inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const ble::address_t& addr) {
    MacAddressString_t converted;
    snprintf(converted.str, sizeof(converted.str), "%02X:%02X:%02X:%02X:%02X:%02X",
        addr[5], addr[4], addr[3],addr[2],addr[1],addr[0]);
    return os << converted.str;
}

/**
 * @brief serialization and deserialization of ble::peripheral_privacy_configuration_t::resolution_strategy_t
 */
template<>
struct SerializerDescription<ble::peripheral_privacy_configuration_t::resolution_strategy_t> {
    typedef ble::peripheral_privacy_configuration_t::resolution_strategy_t type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::peripheral_privacy_configuration_t::DO_NOT_RESOLVE, "DO_NOT_RESOLVE" },
            { ble::peripheral_privacy_configuration_t::REJECT_NON_RESOLVED_ADDRESS, "REJECT_NON_RESOLVED_ADDRESS" },
            { ble::peripheral_privacy_configuration_t::PERFORM_PAIRING_PROCEDURE, "PERFORM_PAIRING_PROCEDURE" },
            { ble::peripheral_privacy_configuration_t::PERFORM_AUTHENTICATION_PROCEDURE, "PERFORM_AUTHENTICATION_PROCEDURE" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::peripheral_privacy_configuration_t::resolution_strategy_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::peripheral_privacy_configuration_t::resolution_strategy_t strategy) {
    return os << toString(strategy);
}

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const ble::peripheral_privacy_configuration_t& configuration) {
    using namespace serialization;
    return os << startObject <<
        key("use_non_resolvable_random_address") << configuration.use_non_resolvable_random_address <<
        key("resolution_strategy") << configuration.resolution_strategy <<
    endObject;
}

/**
 * @brief serialization and deserialization of ble::central_privacy_configuration_t::resolution_strategy_t
 */
template<>
struct SerializerDescription<ble::central_privacy_configuration_t::resolution_strategy_t> {
    typedef ble::central_privacy_configuration_t::resolution_strategy_t type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::central_privacy_configuration_t::DO_NOT_RESOLVE, "DO_NOT_RESOLVE" },
            { ble::central_privacy_configuration_t::RESOLVE_AND_FILTER, "RESOLVE_AND_FILTER" },
            { ble::central_privacy_configuration_t::RESOLVE_AND_FORWARD, "RESOLVE_AND_FORWARD" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::central_privacy_configuration_t::resolution_strategy_t";
    }
};

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::central_privacy_configuration_t::resolution_strategy_t strategy) {
    return os << toString(strategy);
}

static inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const ble::central_privacy_configuration_t& configuration) {
    using namespace serialization;
    return os << startObject <<
        key("use_non_resolvable_random_address") << configuration.use_non_resolvable_random_address <<
        key("resolution_strategy") << configuration.resolution_strategy <<
    endObject;
}

template<>
struct SerializerDescription<ble::advertising_type_t::type> {
    typedef ble::advertising_type_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::advertising_type_t::CONNECTABLE_UNDIRECTED, "CONNECTABLE_UNDIRECTED" },
            { ble::advertising_type_t::CONNECTABLE_DIRECTED, "CONNECTABLE_DIRECTED" },
            { ble::advertising_type_t::SCANNABLE_UNDIRECTED, "SCANNABLE_UNDIRECTED" },
            { ble::advertising_type_t::NON_CONNECTABLE_UNDIRECTED, "NON_CONNECTABLE_UNDIRECTED" },
            { ble::advertising_type_t::CONNECTABLE_DIRECTED_LOW_DUTY, "CONNECTABLE_DIRECTED_LOW_DUTY" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::advertising_type_t";
    }
};

template<typename Layout, uint32_t TB, typename R, typename F>
bool fromString(const char* str, ble::Duration<Layout, TB, R, F>& duration) {
    Layout v = 0;
    if (!fromString(str, v)) {
        return false;
    }

    if (v < duration.MIN || v > duration.MAX) {
        return false;
    }

    duration = ble::Duration<Layout, TB, R, F>(v);
    return true;
}

template<typename Layout, uint32_t TB, typename R, typename F>
serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::Duration<Layout, TB, R, F> duration) {
    return os << duration.value();
}

template<>
struct SerializerDescription<ble::own_address_type_t::type > {
    typedef ble::own_address_type_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::own_address_type_t::PUBLIC, "PUBLIC" },
            { ble::own_address_type_t::RANDOM, "RANDOM" },
            { ble::own_address_type_t::RESOLVABLE_PRIVATE_ADDRESS_PUBLIC_FALLBACK, "RESOLVABLE_PRIVATE_ADDRESS_PUBLIC_FALLBACK" },
            { ble::own_address_type_t::RESOLVABLE_PRIVATE_ADDRESS_RANDOM_FALLBACK, "RESOLVABLE_PRIVATE_ADDRESS_RANDOM_FALLBACK" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::own_address_type_t";
    }
};

inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::own_address_type_t v) {
    return os << toString((ble::own_address_type_t::type) v.value());
}

template<>
struct SerializerDescription<ble::advertising_filter_policy_t::type > {
    typedef ble::advertising_filter_policy_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::advertising_filter_policy_t::NO_FILTER, "NO_FILTER" },
            { ble::advertising_filter_policy_t::FILTER_SCAN_REQUESTS, "FILTER_SCAN_REQUESTS" },
            { ble::advertising_filter_policy_t::FILTER_CONNECTION_REQUEST, "FILTER_CONNECTION_REQUEST" },
            { ble::advertising_filter_policy_t::FILTER_SCAN_AND_CONNECTION_REQUESTS, "FILTER_SCAN_AND_CONNECTION_REQUESTS" },
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::advertising_filter_policy_t";
    }
};

template<>
struct SerializerDescription<ble::scanning_filter_policy_t::type > {
    typedef ble::scanning_filter_policy_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::scanning_filter_policy_t::NO_FILTER, "NO_FILTER" },
            { ble::scanning_filter_policy_t::FILTER_ADVERTISING, "FILTER_ADVERTISING" },
            { ble::scanning_filter_policy_t::NO_FILTER_INCLUDE_UNRESOLVABLE_DIRECTED, "NO_FILTER_INCLUDE_UNRESOLVABLE_DIRECTED" },
            { ble::scanning_filter_policy_t::FILTER_ADVERTISING_INCLUDE_UNRESOLVABLE_DIRECTED, "FILTER_ADVERTISING_INCLUDE_UNRESOLVABLE_DIRECTED" },
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::scanning_filter_policy_t";
    }
};


template<>
struct SerializerDescription<ble::initiator_filter_policy_t::type> {
    typedef ble::initiator_filter_policy_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::initiator_filter_policy_t::NO_FILTER, "NO_FILTER" },
            { ble::initiator_filter_policy_t::USE_WHITE_LIST, "USE_WHITE_LIST" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::initiator_filter_policy_t";
    }
};

template<>
struct SerializerDescription<ble::duplicates_filter_t::type> {
    typedef ble::duplicates_filter_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::duplicates_filter_t::DISABLE, "DISABLE" },
            { ble::duplicates_filter_t::ENABLE, "ENABLE" },
            { ble::duplicates_filter_t::PERIODIC_RESET, "PERIODIC_RESET" },
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::duplicates_filter_t";
    }
};

template<>
struct SerializerDescription<ble::peer_address_type_t::type> {
    typedef ble::peer_address_type_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::peer_address_type_t::PUBLIC, "PUBLIC" },
            { ble::peer_address_type_t::RANDOM, "RANDOM" },
            { ble::peer_address_type_t::PUBLIC_IDENTITY, "PUBLIC_IDENTITY" },
            { ble::peer_address_type_t::RANDOM_STATIC_IDENTITY, "RANDOM_STATIC_IDENTITY" },
            { ble::peer_address_type_t::ANONYMOUS, "ANONYMOUS" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::peer_address_type_t";
    }
};

template<>
struct SerializerDescription<ble::local_disconnection_reason_t::type> {
    typedef ble::local_disconnection_reason_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::local_disconnection_reason_t::USER_TERMINATION, "USER_TERMINATION" },
            { ble::local_disconnection_reason_t::AUTHENTICATION_FAILURE, "AUTHENTICATION_FAILURE" },
            { ble::local_disconnection_reason_t::LOW_RESOURCES, "LOW_RESOURCES" },
            { ble::local_disconnection_reason_t::PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED, "PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED" },
            { ble::local_disconnection_reason_t::POWER_OFF, "POWER_OFF" },
            { ble::local_disconnection_reason_t::UNACCEPTABLE_CONNECTION_PARAMETERS, "UNACCEPTABLE_CONNECTION_PARAMETERS" },
            { ble::local_disconnection_reason_t::UNSUPPORTED_REMOTE_FEATURE, "UNSUPPORTED_REMOTE_FEATURE" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::local_disconnection_reason_t";
    }
};

template<>
struct SerializerDescription<ble::controller_supported_features_t::type> {
    typedef ble::controller_supported_features_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::controller_supported_features_t::LE_EXTENDED_ADVERTISING, "LE_EXTENDED_ADVERTISING" },
            { ble::controller_supported_features_t::LE_2M_PHY, "LE_2M_PHY" },
            { ble::controller_supported_features_t::CHANNEL_SELECTION_ALGORITHM_2, "CHANNEL_SELECTION_ALGORITHM_2" },
            { ble::controller_supported_features_t::CONNECTION_PARAMETERS_REQUEST_PROCEDURE, "CONNECTION_PARAMETERS_REQUEST_PROCEDURE" },
            { ble::controller_supported_features_t::EXTENDED_REJECT_INDICATION, "EXTENDED_REJECT_INDICATION" },
            { ble::controller_supported_features_t::EXTENDED_SCANNER_FILTER_POLICIES, "EXTENDED_SCANNER_FILTER_POLICIES" },
            { ble::controller_supported_features_t::LE_CODED_PHY, "LE_CODED_PHY" },
            { ble::controller_supported_features_t::LE_DATA_PACKET_LENGTH_EXTENSION, "LE_DATA_PACKET_LENGTH_EXTENSION" },
            { ble::controller_supported_features_t::LE_ENCRYPTION, "LE_ENCRYPTION" },
            { ble::controller_supported_features_t::LE_PERIODIC_ADVERTISING, "LE_PERIODIC_ADVERTISING" },
            { ble::controller_supported_features_t::LE_PING, "LE_PING" },
            { ble::controller_supported_features_t::LE_POWER_CLASS, "LE_POWER_CLASS" },
            { ble::controller_supported_features_t::LL_PRIVACY, "LL_PRIVACY" },
            { ble::controller_supported_features_t::SLAVE_INITIATED_FEATURES_EXCHANGE, "SLAVE_INITIATED_FEATURES_EXCHANGE" },
            { ble::controller_supported_features_t::STABLE_MODULATION_INDEX_RECEIVER, "STABLE_MODULATION_INDEX_RECEIVER" },
            { ble::controller_supported_features_t::STABLE_MODULATION_INDEX_TRANSMITTER, "STABLE_MODULATION_INDEX_TRANSMITTER" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::controller_supported_features_t";
    }
};


template<>
struct SerializerDescription<ble::advertising_data_status_t::type> {
    typedef ble::advertising_data_status_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::advertising_data_status_t::COMPLETE, "COMPLETE" },
            { ble::advertising_data_status_t::INCOMPLETE_MORE_DATA, "INCOMPLETE_MORE_DATA" },
            { ble::advertising_data_status_t::INCOMPLETE_DATA_TRUNCATED, "INCOMPLETE_DATA_TRUNCATED" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::advertising_data_status_t";
    }
};

inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::advertising_data_status_t v) {
    return os << toString((ble::advertising_data_status_t::type) v.value());
}

inline serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, ble::disconnection_reason_t v)
{
    switch (v.value()) {
        case ble::disconnection_reason_t::AUTHENTICATION_FAILURE:
            return os << "AUTHENTICATION_FAILURE";
        case ble::disconnection_reason_t::CONNECTION_TIMEOUT:
            return os << "CONNECTION_TIMEOUT";
        case ble::disconnection_reason_t::REMOTE_USER_TERMINATED_CONNECTION:
            return os << "REMOTE_USER_TERMINATED_CONNECTION";
        case ble::disconnection_reason_t::REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES:
            return os << "REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES";
        case ble::disconnection_reason_t::REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF:
            return os << "REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF";
        case ble::disconnection_reason_t::LOCAL_HOST_TERMINATED_CONNECTION:
            return os << "LOCAL_HOST_TERMINATED_CONNECTION";
        case ble::disconnection_reason_t::UNACCEPTABLE_CONNECTION_PARAMETERS:
            return os << "UNACCEPTABLE_CONNECTION_PARAMETERS";
        default:
            return os << "unknown disconnection_reason_t";
    }
}

#endif //BLE_CLIAPP_GAP_SERIALIZER_H_
