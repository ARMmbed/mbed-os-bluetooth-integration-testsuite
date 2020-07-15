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
from functools import cmp_to_key
from time import sleep

import pytest

from common.ble_device import BleDevice
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator

NUM_REPEATED_CALLS = 10
# TODO: These values need to be tuned correctly to avoid false positives
# and actually put strain on the API!
EXPECTED_PACKETS_PER_SCAN = 6
MIN_NON_CONN_ADV_INTERVAL = 0x00A0
MIN_CONN_ADV_INTERVAL = 0x0020
# we can change the length of the test by increasing the maximum value
# of the advertising interval. Also notice that some values will cause
# the startScan call to fail because scan timeout is too long. Finally,
# setting MAX_ADV_INTERVAL to None will cause the scrip to set the max
# advertising value to the output of getMaxAdvertisingInterval. But this
# might fail if the value is too high.
MAX_ADV_INTERVAL = 1000
ADV_INTERVAL_TOLERANCE = 30
ADV_TYPES = [
    "CONNECTABLE_UNDIRECTED",
    # disabled for the moment, it is not yet supported
    # "CONNECTABLE_DIRECTED",
    "SCANNABLE_UNDIRECTED",
    "NON_CONNECTABLE_UNDIRECTED"
]

CONN_ADV_INTERVAL_RANGE = (MIN_CONN_ADV_INTERVAL, MAX_ADV_INTERVAL)
NON_CONN_ADV_INTERVAL_RANGE = (MIN_NON_CONN_ADV_INTERVAL, MAX_ADV_INTERVAL)
INTERVAL_UNIT_MS = 0.625


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


@pytest.fixture(scope="function")
def advertiser_address(advertiser: BleDevice) -> str:
    return advertiser.gap.getAddress().result["address"]


def generate_ran_adv_interval(advertising_type):
    advertising_interval = random.randint(*CONN_ADV_INTERVAL_RANGE)
    # TODO: update this once support for CONNECTABLE_DIRECTED is enabled
    if advertising_type in ["SCANNABLE_UNDIRECTED", "NON_CONNECTABLE_UNDIRECTED"]:
        advertising_interval = random.randint(*NON_CONN_ADV_INTERVAL_RANGE)

    # calculate scan_timeout to catch EXPECTED_PACKETS_PER_SCAN, the +1
    # allows 1 packet to be dropped
    scan_timeout = advertising_interval * (EXPECTED_PACKETS_PER_SCAN + 1)
    return advertising_interval, scan_timeout


def is_time_interval_between_packet_valid(packet_time_interval, advertising_interval, packet_drops_allowed=3):
    for i in range(0, packet_drops_allowed + 1):
        lower_bound = (i + 1) * (advertising_interval - ADV_INTERVAL_TOLERANCE) * INTERVAL_UNIT_MS
        upper_bound = (i + 1) * (advertising_interval + ADV_INTERVAL_TOLERANCE) * INTERVAL_UNIT_MS
        if lower_bound < packet_time_interval < upper_bound:
            return True
    return False


def check_packets_advertising_interval(scanned_packets, advertising_interval):
    # filter out scan responses
    scanned_packets = filter(lambda x: x["scan_response"] is False, scanned_packets)

    # sort the items based on time
    scanned_packets = sorted(scanned_packets, key=cmp_to_key(lambda x, y: x["time"] - y["time"]))

    for i in range(len(scanned_packets) - 1):
        time_diff = scanned_packets[i + 1]["time"] - scanned_packets[i]["time"]
        assert is_time_interval_between_packet_valid(time_diff, advertising_interval)


@pytest.mark.ble41
@pytest.mark.parametrize("advertising_type", ["CONNECTABLE_UNDIRECTED", "SCANNABLE_UNDIRECTED", "NON_CONNECTABLE_UNDIRECTED"])
def test_advertising_interval(advertiser: BleDevice, scanner: BleDevice, advertiser_address, advertising_type: str):
    """test that advertising interval works as expected for each advertising type"""
    for _ in range(NUM_REPEATED_CALLS):
        # generate some random interval within the acceptable bounds
        advertising_interval, scan_timeout = generate_ran_adv_interval(advertising_type)

        # set the advertising type and interval
        advertiser.advParams.setType(advertising_type)
        advertiser.advParams.setPrimaryInterval(advertising_interval, advertising_interval)
        advertiser.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
        advertiser.gap.applyAdvPayloadFromBuilder(LEGACY_ADVERTISING_HANDLE)

        # start advertising
        advertiser.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

        # configure the scanner so that we catch
        # EXPECTED_PACKETS_PER_SCAN packets. The idea is to avoid
        # running out of memory in the scanner when buffering packets
        scanner.scanParams.set1mPhyConfiguration(100, 100, False)
        scanner.gap.setScanParameters()
        resp = scanner.gap.scanForAddress(advertiser_address, scan_timeout)

        # verify the time interval between each packet is within acceptable bounds
        check_packets_advertising_interval(resp.result, advertising_interval)

        # check that all packets are of the expected advertising type
        connectable = advertising_type in ["CONNECTABLE_UNDIRECTED"]
        scannable = advertising_type in ["CONNECTABLE_UNDIRECTED", "SCANNABLE_UNDIRECTED"]
        directed = False  # until supported
        assert all([packet["connectable"] == connectable for packet in resp.result])
        assert all([packet["scannable"] == scannable for packet in resp.result])
        assert all([packet["directed"] == directed for packet in resp.result])

        # stop advertising
        advertiser.gap.stopAdvertising(LEGACY_ADVERTISING_HANDLE)
        sleep(1)  # wait for advertising to stop
