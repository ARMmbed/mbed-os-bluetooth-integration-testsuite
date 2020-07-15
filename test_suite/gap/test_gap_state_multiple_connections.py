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

from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator


@pytest.fixture(scope="function")
def peripheral1(board_allocator: BoardAllocator):
    device = board_allocator.allocate('peripheral1')
    assert device is not None
    device.ble.init()

    yield device

    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def peripheral2(board_allocator: BoardAllocator):
    device = board_allocator.allocate('peripheral2')
    assert device is not None
    device.ble.init()

    yield device

    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def central(board_allocator: BoardAllocator):
    device = board_allocator.allocate('central')
    assert device is not None

    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def peripheral_address(peripheral):
    yield peripheral.gap.getAddress().result


def makeConnectionArgs(address):
    connection_args = [address["address_type"], address["address"]]
    return connection_args


@pytest.mark.ble41
def test_multiple_central_connection(central, peripheral1, peripheral2):
    """This test checks whether Gap::isAdvertisingActive() reports correct value
    when connect, disconnect, advertising and timeout event occurs and the
    central is connected to multiple devices."""
    for peripheral in [peripheral1, peripheral2]:
        peripheral.advParams.setPrimaryInterval(100, 100)
        peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
        peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
        peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # Get the first peripheral's address
    peripheral1_address = peripheral1.gap.getAddress().result

    # Get the second peripheral's address
    peripheral2_address = peripheral2.gap.getAddress().result

    # Connect the central to the first peripheral

    # Assert peripheral 1 will later be notified of connection
    peripheral1.gap.waitForConnection.setAsync()(10000)

    # Establish the connection
    peripheral1_handle = central.gap.connect(
        *makeConnectionArgs(peripheral1_address)
    ).result["connection_handle"]

    # wait for the peripheral1 advertising to end
    sleep(1)

    assert central.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
    assert peripheral1.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
    assert peripheral2.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result

    # Assert peripheral 2 will later be notified of connection
    peripheral2.gap.waitForConnection.setAsync()(10000)

    # Connect the central to the second peripheral
    # Establish the connection
    peripheral2_handle = central.gap.connect(
        *makeConnectionArgs(peripheral2_address)
    ).result["connection_handle"]

    # wait for the peripheral2 advertising to end
    sleep(1)

    assert central.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
    assert peripheral1.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
    assert peripheral2.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False

    # Disconnect one of the devices and check Gap state
    central.gap.disconnect(peripheral1_handle, "USER_TERMINATION")
    sleep(1)
    assert central.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
    assert peripheral1.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
    assert peripheral2.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False

    # Disconnect the second peripheral  and check Gap state
    central.gap.disconnect(peripheral2_handle, "USER_TERMINATION")
    sleep(1)
    assert central.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
    assert peripheral1.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
    assert peripheral2.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result is False
