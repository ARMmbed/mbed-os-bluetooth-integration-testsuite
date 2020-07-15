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

import random

import pytest

from common.ble_device import BleDevice
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator

NUM_REPEATED_CALLS = 10
# TODO: These values need to be fine tuned when ble-cliapp is fixed to have
# data streamed to avoid running out of memory during the test
ADV_INTERVAL_MSEC = 1000
# startAdvertising takes 10ms (1/100 s) as a parameter
ADV_TIMEOUT_RANGE_10MS = (2 * 100, 10 * 100)
# if the packet was scanned X ms after the timeout its still a pass
ADV_TIMEOUT_TOLERANCE_MSEC = ADV_INTERVAL_MSEC


@pytest.fixture(scope="function")
def advertiser(board_allocator: BoardAllocator) -> BleDevice:
    dev = board_allocator.allocate("advertiser")
    assert dev is not None
    dev.ble.init()
    dev.advParams.setPrimaryInterval(ADV_INTERVAL_MSEC, ADV_INTERVAL_MSEC)
    dev.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    yield dev
    dev.ble.shutdown()
    board_allocator.release(dev)


@pytest.fixture("function")
def advertiser_address(advertiser) -> str:
    return advertiser.gap.getAddress().result["address"]


@pytest.fixture(scope="function")
def scanner(board_allocator: BoardAllocator) -> BleDevice:
    dev = board_allocator.allocate("scanner")
    assert dev is not None
    dev.ble.init()
    yield dev
    dev.ble.shutdown()
    board_allocator.release(dev)


@pytest.mark.ble41
def test_advertising_timeout(scanner: BleDevice, advertiser: BleDevice, advertiser_address: str):
    """Test that advertising timeout work as expected"""
    for _ in range(NUM_REPEATED_CALLS):
        # advertise for a limited duration
        advertising_timeout = random.randint(*ADV_TIMEOUT_RANGE_10MS)
        advertiser.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, advertising_timeout, ADV_MAX_EVENTS_UNLIMITED)

        # scan for three times the duration
        scanner.scanParams.set1mPhyConfiguration(100, 100, True)
        scanner.gap.setScanParameters()
        resp = scanner.gap.scanForAddress.setAsync()(advertiser_address, advertising_timeout * 10 * 3)  # units: 10ms -> ms

        # check whether any packet arrived after the timeout expired
        for packet in resp.result:
            assert packet["time"] < ((advertising_timeout * 10) + ADV_TIMEOUT_TOLERANCE_MSEC)

        # check advertising state
        advertiser_state = advertiser.gap.isAdvertisingActive(0).result
        assert advertiser_state is False
