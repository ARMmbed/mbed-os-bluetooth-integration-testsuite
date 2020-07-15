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

import pytest

from common.ble_device import BleDevice
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator

NUM_REPEATED_CALLS = 10
RSSI_TOLERANCE = 15


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
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.mark.ble50
def test_set_tx_power(advertiser):
    """Values between -127 and 126 (as per BLE specification) should be accepted by setTxPower"""
    tx_power_values = [-127, 40, 0, 40, 126]
    for power_value in tx_power_values:
        assert advertiser.advParams.setTxPower(power_value).success()


@pytest.mark.ble50
def test_tx_power_effect(advertiser, scanner):
    """Setting Tx power should have a visible effect"""
    # put some payload in the advertising packets
    advertiser_flags = ["LE_GENERAL_DISCOVERABLE", "BREDR_NOT_SUPPORTED"]
    for f in advertiser_flags:
        advertiser.advDataBuilder.setFlags(f)
    advertising_interval = 200
    advertiser.advDataBuilder.setAdvertisingInterval(advertising_interval)
    advertiser.gap.applyAdvPayloadFromBuilder(LEGACY_ADVERTISING_HANDLE)
    advertiser.advParams.setPrimaryInterval(advertising_interval, advertising_interval)
    advertiser.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)

    # get the power values
    tx_power_values = [-50, 126]
    advertiser_addr = advertiser.gap.getAddress().result["address"]

    def compute_average_rssi(tx_power, scan_time):
        # advertise with the maximum power and compute the average RSSI
        advertiser.advParams.setTxPower(tx_power)
        advertiser.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
        advertiser.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

        # start scanning for 3 seconds
        scanner.scanParams.set1mPhyConfiguration(100, 100, True)  # active scanning
        scanner.gap.setScanParameters()
        scans = scanner.gap.scanForAddress(advertiser_addr, scan_time).result
        assert len(scans) > 0
        rssi = 0
        for scan in scans:
            rssi += scan["rssi"]
        rssi /= len(scans)

        advertiser.gap.stopAdvertising(LEGACY_ADVERTISING_HANDLE)
        return rssi

    max_rssi = compute_average_rssi(max(tx_power_values), 500)
    sleep(5)
    min_rssi = compute_average_rssi(min(tx_power_values), 500)

    # check that the rssi value changes when transmit power value changes in the advertiser
    assert abs(max_rssi - min_rssi) > RSSI_TOLERANCE
