# Copyright (c) 2009-2020 Arm Limited
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
    Utilities for gap API testing
"""
import random
import string
import uuid
from time import sleep
from uuid import UUID
from uuid import uuid4

from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED

APPEARANCES = [
    "UNKNOWN",
    "GENERIC_PHONE",
    "GENERIC_COMPUTER",
    "GENERIC_WATCH",
    "WATCH_SPORTS_WATCH",
    "GENERIC_CLOCK",
    "GENERIC_DISPLAY",
    "GENERIC_REMOTE_CONTROL",
    "GENERIC_EYE_GLASSES",
    "GENERIC_TAG",
    "GENERIC_KEYRING",
    "GENERIC_MEDIA_PLAYER",
    "GENERIC_BARCODE_SCANNER",
    "GENERIC_THERMOMETER",
    "THERMOMETER_EAR",
    "GENERIC_HEART_RATE_SENSOR",
    "HEART_RATE_SENSOR_HEART_RATE_BELT",
    "GENERIC_BLOOD_PRESSURE",
    "BLOOD_PRESSURE_ARM",
    "BLOOD_PRESSURE_WRIST",
    "HUMAN_INTERFACE_DEVICE_HID",
    "KEYBOARD",
    "MOUSE",
    "JOYSTICK",
    "GAMEPAD",
    "DIGITIZER_TABLET",
    "CARD_READER",
    "DIGITAL_PEN",
    "BARCODE_SCANNER",
    "GENERIC_GLUCOSE_METER",
    "GENERIC_RUNNING_WALKING_SENSOR",
    "RUNNING_WALKING_SENSOR_IN_SHOE",
    "RUNNING_WALKING_SENSOR_ON_SHOE",
    "RUNNING_WALKING_SENSOR_ON_HIP",
    "GENERIC_CYCLING",
    "CYCLING_CYCLING_COMPUTER",
    "CYCLING_SPEED_SENSOR",
    "CYCLING_CADENCE_SENSOR",
    "CYCLING_POWER_SENSOR",
    "CYCLING_SPEED_AND_CADENCE_SENSOR",
    "PULSE_OXIMETER_GENERIC",
    "PULSE_OXIMETER_FINGERTIP",
    "PULSE_OXIMETER_WRIST_WORN",
    "OUTDOOR_GENERIC",
    "OUTDOOR_LOCATION_DISPLAY_DEVICE",
    "OUTDOOR_LOCATION_AND_NAVIGATION_DISPLAY_DEVICE",
    "OUTDOOR_LOCATION_POD",
    "OUTDOOR_LOCATION_AND_NAVIGATION_POD"
]

DEVICE_NAME_MAX_LEN = 248
DEVICE_NAME_MIN_LEN = 1


def generate_rand_appearance():
    return APPEARANCES[random.randint(0, len(APPEARANCES) - 1)]


def generate_rand_device_name(min_len=DEVICE_NAME_MIN_LEN, max_len=DEVICE_NAME_MAX_LEN):
    # The upper bound comes from the bluetooth spec
    # The lower bound is because we cannot pass 0 characters as arguments to ble-cliapp
    device_name_len = random.randint(min_len, max_len)
    allowed_chars = string.ascii_letters + string.digits
    return ''.join(random.choice(allowed_chars) for _ in range(0, device_name_len))


ADV_DATA_TYPES = [
    # "FLAGS",
    "INCOMPLETE_LIST_16BIT_SERVICE_IDS",
    "COMPLETE_LIST_16BIT_SERVICE_IDS",
    # "INCOMPLETE_LIST_32BIT_SERVICE_IDS", # 32 bit not supported
    # "COMPLETE_LIST_32BIT_SERVICE_IDS",
    "INCOMPLETE_LIST_128BIT_SERVICE_IDS",
    "COMPLETE_LIST_128BIT_SERVICE_IDS",
    "SHORTENED_LOCAL_NAME",
    "COMPLETE_LOCAL_NAME",
    "TX_POWER_LEVEL",
    # "DEVICE_ID", # not yet implemented
    "SLAVE_CONNECTION_INTERVAL_RANGE",
    # "LIST_128BIT_SOLICITATION_IDS",
    # "SERVICE_DATA", # not yet implemented
    "APPEARANCE",
    "ADVERTISING_INTERVAL",
    "MANUFACTURER_SPECIFIC_DATA"
]

FLAGS = ["LE_LIMITED_DISCOVERABLE",
         "LE_GENERAL_DISCOVERABLE",
         "BREDR_NOT_SUPPORTED",
         "SIMULTANEOUS_LE_BREDR_C",
         "SIMULTANEOUS_LE_BREDR_H"]


def get_rand_uuid(size=16):
    assert (size in [16, 32, 128])

    if size == 128:
        retval = str(uuid.uuid4())
    else:
        retval = hex(int(random.getrandbits(size)))

    return retval


def get_rand_data(adv_data_type):
    assert (adv_data_type in ADV_DATA_TYPES)

    if adv_data_type == "FLAGS":
        randflags = []
        randflags += random.sample(FLAGS[:2], 1)
        randflags += random.sample(FLAGS[2:], 1)
        return randflags
    elif "_LIST_16BIT_SERVICE_IDS" in adv_data_type:
        return get_rand_uuid(16)
    # 32 bit not supported
    # elif "_LIST_32BIT_SERVICE_IDS" in advDataType:
    #    return randUUIDs(32)
    elif adv_data_type in ["LIST_128BIT_SOLICITATION_IDS", "INCOMPLETE_LIST_128BIT_SERVICE_IDS",
                           "COMPLETE_LIST_128BIT_SERVICE_IDS"]:
        return get_rand_uuid(128)
    elif adv_data_type in ["SHORTENED_LOCAL_NAME", "COMPLETE_LOCAL_NAME", "DEVICE_ID"]:
        return generate_rand_device_name(max_len=26)
    elif adv_data_type == "TX_POWER_LEVEL":
        return str(random.randint(-127, 127))
    elif adv_data_type == "SLAVE_CONNECTION_INTERVAL_RANGE":
        int_range = [int(random.getrandbits(16)) for _ in range(2)]
        int_range.sort()
        return " ".join([hex(x) for x in int_range])
    elif adv_data_type == "SERVICE_DATA":
        return hex(int(random.getrandbits(128)))
    elif adv_data_type == "APPEARANCE":
        return generate_rand_appearance()
    elif adv_data_type == "ADVERTISING_INTERVAL":
        return hex(int(random.getrandbits(16)))
    elif adv_data_type == "MANUFACTURER_SPECIFIC_DATA":
        char_set = string.ascii_letters + string.octdigits
        # size is at least 2 to contain company identifier
        str_len = random.randint(2, 29)
        rand_str = ''.join(random.choice(char_set) for i in range(str_len))
        return rand_str.encode().hex().lower()


def get_rand_data_type():
    return random.sample(ADV_DATA_TYPES, 1)[0]


class RandomAdvDataPair:
    def __init__(self):
        self.dataType = get_rand_data_type()
        self.data = get_rand_data(self.dataType)
        self.dataSize = 0

        if self.dataType in ["FLAGS", "TX_POWER_LEVEL"]:
            self.dataSize = 1
        elif self.dataType == "APPEARANCE":
            self.dataSize = 2
        elif "_LIST_16BIT_SERVICE_IDS" in self.dataType:
            self.dataSize = 2 * len(self.data.split(' '))
        # elif "_LIST_32BIT_SERVICE_IDS" in self.dataType: # 32 bit not supported
        #    self.dataSize = 4 * len(self.data.split(' '))
        elif self.dataType in ["LIST_128BIT_SOLICITATION_IDS", "INCOMPLETE_LIST_128BIT_SERVICE_IDS",
                               "COMPLETE_LIST_128BIT_SERVICE_IDS"]:
            self.dataSize = 16 * len(self.data.split(' '))
        elif self.dataType in ["SHORTENED_LOCAL_NAME", "COMPLETE_LOCAL_NAME", "DEVICE_ID", "SERVICE_DATA"]:
            self.dataSize = len(self.data)
        elif self.dataType == "SLAVE_CONNECTION_INTERVAL_RANGE":
            self.dataSize = 4
        elif self.dataType == "ADVERTISING_INTERVAL":
            self.dataSize = 2
        elif self.dataType == "MANUFACTURER_SPECIFIC_DATA":
            self.dataSize = len(self.data) / 2

        self.size = self.dataSize + 2


def start_advertising_of_type(peripheral, advertising_type):
    # params
    peripheral.advParams.setType(advertising_type)
    if advertising_type == "CONNECTABLE_UNDIRECTED":
        peripheral.advParams.setPrimaryInterval(100, 100)
    else:
        peripheral.advParams.setPrimaryInterval(200, 400)
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)

    # payload
    peripheral.advDataBuilder.clear()
    if advertising_type == "CONNECTABLE_UNDIRECTED":
        peripheral.advDataBuilder.setFlags("LE_GENERAL_DISCOVERABLE")
    peripheral.advDataBuilder.setFlags("BREDR_NOT_SUPPORTED")
    rand_data = get_rand_data("MANUFACTURER_SPECIFIC_DATA")
    if len(rand_data) > 10:
        rand_data = rand_data[0:10]
    peripheral.advDataBuilder.setManufacturerSpecificData(rand_data)
    peripheral.gap.applyAdvPayloadFromBuilder(LEGACY_ADVERTISING_HANDLE)

    # get resulting payload
    advertising_data = peripheral.advDataBuilder.getAdvertisingData().result

    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    return advertising_data


def start_scanning_for_data(central, advertising_data):
    central.scanParams.set1mPhyConfiguration(100, 100, False)
    central.scanParams.setFilter("NO_FILTER")
    central.gap.setScanParameters()
    scan_cmd = central.gap.scanForData(advertising_data, 1000)
    return scan_cmd.result


def connect_to_address(central, peripheral, peer_address_type, peer_address):
    peripheral_connection_cmd = peripheral.gap.waitForConnection.setAsync()(10000)
    sleep(0.1)
    central_connection = central.gap.connect(peer_address_type, peer_address).result
    peripheral_connection = peripheral_connection_cmd.result
    return central_connection, peripheral_connection


def gap_connect(central, peripheral):
    advertising_data = start_advertising_of_type(peripheral, "CONNECTABLE_UNDIRECTED")

    scan = start_scanning_for_data(central, advertising_data)
    assert len(scan) > 0
    central_connection, peripheral_connection = connect_to_address(
        central,
        peripheral,
        scan[0]["peer_address_type"],
        scan[0]["peer_address"]
    )

    # get the handle of the new connection
    return central_connection["connection_handle"], peripheral_connection["connection_handle"]


def discover_user_services(central, connection_handle):
    services = central.gattClient.discoverAllServicesAndCharacteristics(connection_handle).result
    # The two first services should be the GenericAccessService and the GenericAttribute service
    # their instantiation is tested somewhere else, it is safe to assume they are here
    # and return the other services (the one instantiated by the user)
    return services[2:]


def discover_descriptors(central, connection_handle, declaration_handle, end_handle):
    return central.gattClient.discoverAllCharacteristicsDescriptors(
        connection_handle,
        declaration_handle,
        end_handle
    ).result


def make_uuid():
    return str(uuid4())


def assert_uuid_equals(expected, current_value):
    if isinstance(expected, int):
        assert int == type(current_value)
        assert expected == current_value
    else:
        assert int != type(current_value)
        assert UUID(expected) == UUID(current_value)
