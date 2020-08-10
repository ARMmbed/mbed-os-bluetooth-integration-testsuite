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
#ifndef BLE_CLIAPP_GAP_ADVERTISING_DATA_SERIALIZER_H_
#define BLE_CLIAPP_GAP_ADVERTISING_DATA_SERIALIZER_H_

#include "Serialization/Serializer.h"
#include <algorithm>
#include "Serialization/UUID.h"
#include "Hex.h"

#include "CLICommand/CommandArgs.h"
#include "Serialization/Serializer.h"
#include "Serialization/JSONOutputStream.h"

template<>
struct SerializerDescription<ble::adv_data_type_t::type> {
    typedef ble::adv_data_type_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::adv_data_type_t::FLAGS, "FLAGS" },
            { ble::adv_data_type_t::INCOMPLETE_LIST_16BIT_SERVICE_IDS, "INCOMPLETE_LIST_16BIT_SERVICE_IDS" },
            { ble::adv_data_type_t::COMPLETE_LIST_16BIT_SERVICE_IDS, "COMPLETE_LIST_16BIT_SERVICE_IDS" },
            { ble::adv_data_type_t::INCOMPLETE_LIST_32BIT_SERVICE_IDS, "INCOMPLETE_LIST_32BIT_SERVICE_IDS" },
            { ble::adv_data_type_t::COMPLETE_LIST_32BIT_SERVICE_IDS, "COMPLETE_LIST_32BIT_SERVICE_IDS" },
            { ble::adv_data_type_t::INCOMPLETE_LIST_128BIT_SERVICE_IDS, "INCOMPLETE_LIST_128BIT_SERVICE_IDS" },
            { ble::adv_data_type_t::COMPLETE_LIST_128BIT_SERVICE_IDS, "COMPLETE_LIST_128BIT_SERVICE_IDS" },
            { ble::adv_data_type_t::LIST_128BIT_SOLICITATION_IDS, "LIST_128BIT_SOLICITATION_IDS" },
            { ble::adv_data_type_t::SHORTENED_LOCAL_NAME, "SHORTENED_LOCAL_NAME" },
            { ble::adv_data_type_t::COMPLETE_LOCAL_NAME, "COMPLETE_LOCAL_NAME" },
            { ble::adv_data_type_t::TX_POWER_LEVEL, "TX_POWER_LEVEL" },
            { ble::adv_data_type_t::DEVICE_ID, "DEVICE_ID" },
            { ble::adv_data_type_t::SLAVE_CONNECTION_INTERVAL_RANGE, "SLAVE_CONNECTION_INTERVAL_RANGE" },
            { ble::adv_data_type_t::SERVICE_DATA, "SERVICE_DATA" },
            { ble::adv_data_type_t::SERVICE_DATA_16BIT_ID, "SERVICE_DATA_16BIT_ID" },
            { ble::adv_data_type_t::SERVICE_DATA_128BIT_ID, "SERVICE_DATA_128BIT_ID" },
            { ble::adv_data_type_t::APPEARANCE, "APPEARANCE" },
            { ble::adv_data_type_t::ADVERTISING_INTERVAL, "ADVERTISING_INTERVAL" },
            { ble::adv_data_type_t::MANUFACTURER_SPECIFIC_DATA, "MANUFACTURER_SPECIFIC_DATA" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::adv_data_type_t::type";
    }
};

template<>
struct SerializerDescription<ble::adv_data_appearance_t::type> {
    typedef ble::adv_data_appearance_t::type type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::adv_data_appearance_t::UNKNOWN, "UNKNOWN" },
            { ble::adv_data_appearance_t::GENERIC_PHONE, "GENERIC_PHONE" },
            { ble::adv_data_appearance_t::GENERIC_COMPUTER, "GENERIC_COMPUTER" },
            { ble::adv_data_appearance_t::GENERIC_WATCH, "GENERIC_WATCH" },
            { ble::adv_data_appearance_t::WATCH_SPORTS_WATCH, "WATCH_SPORTS_WATCH" },
            { ble::adv_data_appearance_t::GENERIC_CLOCK, "GENERIC_CLOCK" },
            { ble::adv_data_appearance_t::GENERIC_DISPLAY, "GENERIC_DISPLAY" },
            { ble::adv_data_appearance_t::GENERIC_REMOTE_CONTROL, "GENERIC_REMOTE_CONTROL" },
            { ble::adv_data_appearance_t::GENERIC_EYE_GLASSES, "GENERIC_EYE_GLASSES" },
            { ble::adv_data_appearance_t::GENERIC_TAG, "GENERIC_TAG" },
            { ble::adv_data_appearance_t::GENERIC_KEYRING, "GENERIC_KEYRING" },
            { ble::adv_data_appearance_t::GENERIC_MEDIA_PLAYER, "GENERIC_MEDIA_PLAYER" },
            { ble::adv_data_appearance_t::GENERIC_BARCODE_SCANNER, "GENERIC_BARCODE_SCANNER" },
            { ble::adv_data_appearance_t::GENERIC_THERMOMETER, "GENERIC_THERMOMETER" },
            { ble::adv_data_appearance_t::THERMOMETER_EAR, "THERMOMETER_EAR" },
            { ble::adv_data_appearance_t::GENERIC_HEART_RATE_SENSOR, "GENERIC_HEART_RATE_SENSOR" },
            { ble::adv_data_appearance_t::HEART_RATE_SENSOR_HEART_RATE_BELT, "HEART_RATE_SENSOR_HEART_RATE_BELT" },
            { ble::adv_data_appearance_t::GENERIC_BLOOD_PRESSURE, "GENERIC_BLOOD_PRESSURE" },
            { ble::adv_data_appearance_t::BLOOD_PRESSURE_ARM, "BLOOD_PRESSURE_ARM" },
            { ble::adv_data_appearance_t::BLOOD_PRESSURE_WRIST, "BLOOD_PRESSURE_WRIST" },
            { ble::adv_data_appearance_t::HUMAN_INTERFACE_DEVICE_HID, "HUMAN_INTERFACE_DEVICE_HID" },
            { ble::adv_data_appearance_t::KEYBOARD, "KEYBOARD" },
            { ble::adv_data_appearance_t::MOUSE, "MOUSE" },
            { ble::adv_data_appearance_t::JOYSTICK, "JOYSTICK" },
            { ble::adv_data_appearance_t::GAMEPAD, "GAMEPAD" },
            { ble::adv_data_appearance_t::DIGITIZER_TABLET, "DIGITIZER_TABLET" },
            { ble::adv_data_appearance_t::CARD_READER, "CARD_READER" },
            { ble::adv_data_appearance_t::DIGITAL_PEN, "DIGITAL_PEN" },
            { ble::adv_data_appearance_t::BARCODE_SCANNER, "BARCODE_SCANNER" },
            { ble::adv_data_appearance_t::GENERIC_GLUCOSE_METER, "GENERIC_GLUCOSE_METER" },
            { ble::adv_data_appearance_t::GENERIC_RUNNING_WALKING_SENSOR, "GENERIC_RUNNING_WALKING_SENSOR" },
            { ble::adv_data_appearance_t::RUNNING_WALKING_SENSOR_IN_SHOE, "RUNNING_WALKING_SENSOR_IN_SHOE" },
            { ble::adv_data_appearance_t::RUNNING_WALKING_SENSOR_ON_SHOE, "RUNNING_WALKING_SENSOR_ON_SHOE" },
            { ble::adv_data_appearance_t::RUNNING_WALKING_SENSOR_ON_HIP, "RUNNING_WALKING_SENSOR_ON_HIP" },
            { ble::adv_data_appearance_t::GENERIC_CYCLING, "GENERIC_CYCLING" },
            { ble::adv_data_appearance_t::CYCLING_CYCLING_COMPUTER, "CYCLING_CYCLING_COMPUTER" },
            { ble::adv_data_appearance_t::CYCLING_SPEED_SENSOR, "CYCLING_SPEED_SENSOR" },
            { ble::adv_data_appearance_t::CYCLING_CADENCE_SENSOR, "CYCLING_CADENCE_SENSOR" },
            { ble::adv_data_appearance_t::CYCLING_POWER_SENSOR, "CYCLING_POWER_SENSOR" },
            { ble::adv_data_appearance_t::CYCLING_SPEED_AND_CADENCE_SENSOR, "CYCLING_SPEED_AND_CADENCE_SENSOR" },
            { ble::adv_data_appearance_t::PULSE_OXIMETER_GENERIC, "PULSE_OXIMETER_GENERIC" },
            { ble::adv_data_appearance_t::PULSE_OXIMETER_FINGERTIP, "PULSE_OXIMETER_FINGERTIP" },
            { ble::adv_data_appearance_t::PULSE_OXIMETER_WRIST_WORN, "PULSE_OXIMETER_WRIST_WORN" },
            { ble::adv_data_appearance_t::GENERIC_WEIGHT_SCALE, "GENERIC_WEIGHT_SCALE" },
            { ble::adv_data_appearance_t::OUTDOOR_GENERIC, "OUTDOOR_GENERIC" },
            { ble::adv_data_appearance_t::OUTDOOR_LOCATION_DISPLAY_DEVICE, "OUTDOOR_LOCATION_DISPLAY_DEVICE" },
            { ble::adv_data_appearance_t::OUTDOOR_LOCATION_AND_NAVIGATION_DISPLAY_DEVICE, "OUTDOOR_LOCATION_AND_NAVIGATION_DISPLAY_DEVICE" },
            { ble::adv_data_appearance_t::OUTDOOR_LOCATION_POD, "OUTDOOR_LOCATION_POD" },
            { ble::adv_data_appearance_t::OUTDOOR_LOCATION_AND_NAVIGATION_POD, "OUTDOOR_LOCATION_AND_NAVIGATION_POD" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::adv_data_appearance_t::type";
    }
};

template<>
struct SerializerDescription<ble::adv_data_flags_t> {
    typedef ble::adv_data_flags_t type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { ble::adv_data_flags_t::LE_LIMITED_DISCOVERABLE, "LE_LIMITED_DISCOVERABLE" },
            { ble::adv_data_flags_t::LE_GENERAL_DISCOVERABLE, "LE_GENERAL_DISCOVERABLE" },
            { ble::adv_data_flags_t::BREDR_NOT_SUPPORTED, "BREDR_NOT_SUPPORTED" },
            { ble::adv_data_flags_t::SIMULTANEOUS_LE_BREDR_C, "SIMULTANEOUS_LE_BREDR_C" },
            { ble::adv_data_flags_t::SIMULTANEOUS_LE_BREDR_H, "SIMULTANEOUS_LE_BREDR_H" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown ble::adv_data_flags_t";
    }
};

#endif //BLE_CLIAPP_GAP_ADVERTISING_DATA_SERIALIZER_H_
