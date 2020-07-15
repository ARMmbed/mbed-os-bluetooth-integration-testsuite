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

from time import sleep
from typing import Dict
from typing import List

import pytest

from common.ble_device import BleDevice
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator

IGNORE_WHITELIST = "NO_FILTER"

FILTER_SCAN_REQUESTS = "FILTER_SCAN_REQUESTS"
FILTER_CONN_REQUEST = "FILTER_CONNECTION_REQUEST"
FILTER_ALL_REQUEST = "FILTER_SCAN_AND_CONNECTION_REQUESTS"

FILTER_ALL_ADV = "FILTER_ADVERTISING_INCLUDE_UNRESOLVABLE_DIRECTED"

SCAN_RESPONSE = True
NO_SCAN_RESPONSE = False
ACCEPT_CONN = True
REJECT_CONN = False

SCAN_DURATION = 2000

# random address
RANDOM_ADDRESS = "AA:AA:AA:AA:AA:AA"
RANDOM_ADDRESS_TYPE = "PUBLIC"
SCAN_TIMEOUT = 2000


@pytest.fixture(scope="function")
def scanner(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate("scanner")
    assert device
    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def broadcaster(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate('broadcaster')
    assert device
    device.ble.init()
    advertising_interval = 100
    advertising_type = "CONNECTABLE_UNDIRECTED"
    local_name = "foo"

    # setup the advertising interval
    device.advParams.setType(advertising_type)
    device.advParams.setPrimaryInterval(advertising_interval, advertising_interval)
    device.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)

    # setup the local name in the scan response
    device.advDataBuilder.clear()
    device.advDataBuilder.setName(local_name, True)
    device.gap.applyScanRespFromBuilder(LEGACY_ADVERTISING_HANDLE)

    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def broadcaster_address(broadcaster: BleDevice) -> Dict[str, str]:
    return broadcaster.gap.getAddress().result


def empty_whitelist(dev):
    return []


def random_whitelist(dev):
    return [RANDOM_ADDRESS_TYPE, RANDOM_ADDRESS]


def device_whitelist(device: BleDevice) -> List[str]:
    address = device.gap.getAddress()
    scanner_address_type = address.result['address_type']
    scanner_address = address.result['address']
    return [scanner_address_type, scanner_address]


@pytest.mark.ble41
@pytest.mark.parametrize(
    "policy,               whitelist,         expected_scan_response, expected_connection_request", [
    (IGNORE_WHITELIST,     empty_whitelist,   SCAN_RESPONSE,          ACCEPT_CONN),
    (IGNORE_WHITELIST,     device_whitelist,  SCAN_RESPONSE,          ACCEPT_CONN),
    (IGNORE_WHITELIST,     random_whitelist,  SCAN_RESPONSE,          ACCEPT_CONN),
    (FILTER_SCAN_REQUESTS, device_whitelist,  SCAN_RESPONSE,          ACCEPT_CONN),
    (FILTER_SCAN_REQUESTS, random_whitelist,  NO_SCAN_RESPONSE,       ACCEPT_CONN),
    (FILTER_CONN_REQUEST,  device_whitelist,  SCAN_RESPONSE,          ACCEPT_CONN),
    (FILTER_CONN_REQUEST,  random_whitelist,  SCAN_RESPONSE,          REJECT_CONN),
    (FILTER_ALL_REQUEST,   device_whitelist,  SCAN_RESPONSE,          ACCEPT_CONN),
    (FILTER_ALL_REQUEST,   random_whitelist,  NO_SCAN_RESPONSE,       REJECT_CONN),
])
def test_whitelist_adv_policy(broadcaster, scanner, broadcaster_address,
                              policy, whitelist, expected_scan_response, expected_connection_request):
    """Test whitelist policy of advertiser"""
    broadcaster.advParams.setFilter(policy)
    broadcaster.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    broadcaster.gap.setWhitelist(*whitelist(scanner))
    broadcaster.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # test scan response
    scanner.scanParams.set1mPhyConfiguration(100, 100, True)  # active scanning
    scanner.gap.setScanParameters()
    scans = scanner.gap.scanForAddress(broadcaster_address["address"], SCAN_TIMEOUT).result
    assert len(scans) > 0

    has_responded = len([scan for scan in scans if scan['scan_response']]) > 0
    assert has_responded == expected_scan_response

    # wait before connection, it avoid scan residue
    sleep(1)

    # ensure gap advertising state is correct
    if expected_connection_request:
        broadcaster.gap.waitForConnection.setAsync()(10000)  # Assert connection will happen later on the broadcaster
        scanner.gap.connect(broadcaster_address['address_type'], broadcaster_address['address'])
        sleep(1)  # wait for advertising to end
        assert broadcaster.gap.isAdvertisingActive(0).result is False
    else:
        assert broadcaster.gap.isAdvertisingActive(0).result is True


@pytest.mark.ble41
@pytest.mark.parametrize(
    "policy,           whitelist,        expected_scan_result", [
    (IGNORE_WHITELIST, empty_whitelist,  True),
    (IGNORE_WHITELIST, device_whitelist, True),
    (IGNORE_WHITELIST, random_whitelist, True),
    (FILTER_ALL_ADV,   empty_whitelist,  False),
    (FILTER_ALL_ADV,   device_whitelist, True),
    (FILTER_ALL_ADV,   random_whitelist, False)
])
def test_whitelist_scan_policy(scanner, broadcaster, broadcaster_address, policy, whitelist, expected_scan_result):
    # start advertising
    broadcaster.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    scanner.scanParams.set1mPhyConfiguration(100, 100, True)
    scanner.scanParams.setFilter(policy)
    scanner.gap.setScanParameters()

    scanner.gap.setWhitelist(*whitelist(broadcaster))

    scans = scanner.gap.scanForAddress(broadcaster_address['address'], SCAN_DURATION).result
    if expected_scan_result is True:
        assert len(scans) > 0
    else:
        assert len(scans) == 0
