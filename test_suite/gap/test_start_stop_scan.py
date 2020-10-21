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
from common.gap_utils import get_rand_data

ADVERTISING_INTERVAL = 1000
ADVERTISING_TYPE = "CONNECTABLE_UNDIRECTED"


@pytest.fixture(scope="function")
def scanner(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate("scanner")
    assert device
    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def advertising_data():
    adv_data = get_rand_data("MANUFACTURER_SPECIFIC_DATA")
    # make sure that the data have an acceptable size
    if len(adv_data) > 26:
        adv_data = adv_data[0:26]
    return adv_data


@pytest.fixture(scope="function")
def advertiser(board_allocator: BoardAllocator, advertising_data: str) -> BleDevice:
    device = board_allocator.allocate('advertiser')
    assert device
    # Setup the advertiser
    device.ble.init()

    # set up advertising parameters
    device.advParams.setType(ADVERTISING_TYPE)
    device.advParams.setPrimaryInterval(ADVERTISING_INTERVAL, ADVERTISING_INTERVAL)
    device.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)

    # add emitter data into advertising payload
    device.advDataBuilder.setManufacturerSpecificData(advertising_data)

    # apply payload
    payload = device.advDataBuilder.getAdvertisingData().result
    device.gap.applyAdvPayloadFromBuilder(LEGACY_ADVERTISING_HANDLE)
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def advertiser_address(advertiser: BleDevice) -> str:
    return advertiser.gap.getAddress().result["address"]


@pytest.fixture(scope="function")
def payload(advertiser: BleDevice, advertising_data: str) -> str:
    advertiser.advDataBuilder.setManufacturerSpecificData(advertising_data)
    return advertiser.advDataBuilder.getAdvertisingData().result


@pytest.mark.ble41
def test_start_scan_when_peer_advertising(advertiser: BleDevice, scanner: BleDevice, advertiser_address: str, payload: str):
    """startScan should start a scan and report scan results when a device is advertising"""
    # Calling startAdvertising
    advertiser.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # start scan
    scan_timeout = ADVERTISING_INTERVAL + 200  # give 200 ms tolerance
    scanner.scanParams.set1mPhyConfiguration(100, 100, True)
    scanner.gap.setScanParameters()
    scans = scanner.gap.scanForAddress(advertiser_address, scan_timeout).result
    for scan in scans:
        if scan["scan_response"] is True:
            continue
        assert scan['payload'] == payload


@pytest.mark.ble41
def test_scan_should_not_report_advertisement_when_peer_inactive(scanner: BleDevice, advertiser_address: str):
    """startScan should not report scan results when there is no device advertising"""
    # start scan
    scan_timeout = ADVERTISING_INTERVAL + 200  # give 200 ms tolerance
    scanner.scanParams.set1mPhyConfiguration(100, 100, True)
    scanner.gap.setScanParameters()
    scans = scanner.gap.scanForAddress(advertiser_address, scan_timeout).result
    assert len(scans) == 0


@pytest.mark.ble41
def test_scan_start_stop_can_be_called_in_succession(scanner: BleDevice):
    """startScan should not report scan results when there is no device advertising"""
    # start scan

    scanner.gap.startScan(0, 'DISABLE', 0)
    assert scanner.gap.isRadioActive().result is True

    scanner.gap.stopScan()
    # this is async so we wait
    sleep(0.5)
    assert scanner.gap.isRadioActive().result is False

    for i in range(10):
        scanner.gap.stopScan()
        scanner.gap.startScan(0, 'DISABLE', 0)

    assert scanner.gap.isRadioActive().result is True

    for i in range(10):
        scanner.gap.startScan(0, 'DISABLE', 0)
        scanner.gap.stopScan()

    # this is async so we wait
    sleep(0.5)
    assert scanner.gap.isRadioActive().result is False
