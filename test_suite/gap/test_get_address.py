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

import re

import pytest

from common.ble_device import BleDevice
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator

NUM_REPEATED_CALLS = 10
VALID_BLE_ADDR = re.compile("([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}")

DUMMY_ADDRESS = "AA:AA:AA:AA:AA:AA"
RANDOM_STATIC_MSB_RANGE = ['C', 'D', 'E', 'F']
RANDOM_NON_RESOLVABLE_MSB_RANGE = ['0', '1', '2', '3']
RANDOM_RESOLVABLE_MSB_RANGE = ['4', '5', '6', '7']
UNASSIGNED_MSB_RANGE = ['8', '9', 'A', 'B']
ADVERTISING_INTERVAL = 100


@pytest.fixture(scope="function")
def scanner(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate("scanner")
    assert device
    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def advertiser(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate('advertiser')
    device.ble.init()
    device.advParams.setPrimaryInterval(ADVERTISING_INTERVAL, ADVERTISING_INTERVAL)
    device.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def advertiser_address(advertiser: BleDevice) -> str:
    advertiser.ble.init()
    return advertiser.gap.getAddress().result["address"]


def isRandomStaticAddress(address):
    return True if address[0] in RANDOM_STATIC_MSB_RANGE else False


def isRandomNonResolvableAddress(address):
    return True if address[0] in RANDOM_NON_RESOLVABLE_MSB_RANGE else False


def isRandomResolvableAddress(address):
    return True if address[0] in RANDOM_RESOLVABLE_MSB_RANGE else False


@pytest.mark.ble41
def test_default_address_is_random_static(scanner: BleDevice, advertiser: BleDevice):
    """getAddress should return a valid random static MAC address after initialization"""
    address = advertiser.gap.getAddress().result
    assert isRandomStaticAddress(address["address"])

    assert address["address_type"] == "RANDOM"
    assert VALID_BLE_ADDR.match(address["address"]) is not None

    # Check the address is correctly advertised
    advertiser.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)
    # start scanning for three times the interval
    scanner.scanParams.set1mPhyConfiguration(100, 100, False)
    scanner.gap.setScanParameters()  # apply scanParams
    scan_records = scanner.gap.scanForAddress(address["address"], ADVERTISING_INTERVAL * 10 * 3).result  # 10 ms -> ms

    assert len(scan_records) > 0

    # check that the address is correct
    for scan in scan_records:
        assert address["address"] == scan['peer_address']
    advertiser.gap.stopAdvertising(LEGACY_ADVERTISING_HANDLE)
