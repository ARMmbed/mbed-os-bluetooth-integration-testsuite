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
from typing import Mapping

import pytest

from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator


class TestParams:
    def __init__(self):
        self.advFlags = ["LE_GENERAL_DISCOVERABLE", "BREDR_NOT_SUPPORTED"]
        self.advData = {
            "companyID": "004C",
            "ID": "02",
            "len": "15",
            "proximityUUID": "E20A39F473F54BC4A12F17D1AD07A961",
            "majorNumber": "0001",
            "minorNumber": "0001",
            "txPower": "C8"
        }
        self.advData = "".join(self.advData.values())

        # Scan parameter
        self.scanTimeout = 2000
        self.scanParams = [10, 10, False]

        # Set some connection parameters
        self.connectionParams = [50, 100, 0, 600]
        self.connectionTimeout = 10000
        self.connectionScanParams = [100, 100, 0, False]

    def get_connection_args(self, address: Mapping[str, str]):
        connection_args = [address["address_type"], address["address"]]
        return connection_args


@pytest.fixture(scope="module")
def test_params():
    yield TestParams()


@pytest.fixture(scope="function")
def peripheral(board_allocator: BoardAllocator, test_params: TestParams):
    device = board_allocator.allocate('peripheral')
    assert device is not None

    # initialize the ble stack
    device.ble.init()

    # setup advertising intervals
    device.advParams.setPrimaryInterval(100, 100)
    device.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
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


@pytest.fixture(scope="function")
def central_address(central):
    yield central.gap.getAddress().result


@pytest.mark.ble41
def test_advertising(peripheral, peripheral_address, central, test_params):
    """validate that when a device is advertising and not disconnected,
    Gap::state.isAdvertisingActive==True """
    # start advertising on device 1
    peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # get the Gap state of device 1 and assert that it is equal to what is expected
    state = peripheral.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result
    assert state is True

    # Check that the device is actually advertising
    central.scanParams.set1mPhyConfiguration(*test_params.scanParams)
    central.gap.setScanParameters()  # apply scanParams
    scan_records = central.gap.scanForAddress(peripheral_address['address'], test_params.scanTimeout).result
    assert len(scan_records) > 0


@pytest.mark.ble41
def test_stop_advertising(peripheral, peripheral_address, central, test_params):
    """validate that when a device stop advertising and is disconnected,
    Gap::isAdvertisingActive==False"""
    # start advertising on broadcaster then stop it
    peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)
    peripheral.gap.stopAdvertising(LEGACY_ADVERTISING_HANDLE)

    sleep(1)  # advertising takes a bit of time to end

    # get the Gap state of device 1 and assert that it is equal to what is expected
    state = peripheral.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result
    assert state is False

    sleep(1)

    # Check that the device is no longer advertising
    central.scanParams.set1mPhyConfiguration(*test_params.scanParams)
    central.gap.setScanParameters()  # apply scanParams
    scan_records = central.gap.scanForAddress(peripheral_address['address'], test_params.scanTimeout).result
    assert len(scan_records) == 0


@pytest.mark.ble41
def test_connected(peripheral, peripheral_address, central, test_params):
    """validate that when a peripheral has been connected,
    Gap::isAdvertisingActive==False and the connection event is received"""
    # start advertising as connectable on peripheral
    peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # Assert connection will happen on the peripheral side
    connection = peripheral.gap.waitForConnection.setAsync()(50000)

    # wait for advertising to end
    sleep(1)

    # Establish the connection
    central.gap.connect(*test_params.get_connection_args(peripheral_address))
    assert connection.error is None and connection.result['status'] == "BLE_ERROR_NONE"

    # get the Gap state of device 1 and assert that it is equal to what is expected
    state = peripheral.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result
    assert state is False

    # Check that the device is no longer advertising
    central.scanParams.set1mPhyConfiguration(*test_params.scanParams)
    central.gap.setScanParameters()  # apply scanParams
    scan_records = central.gap.scanForAddress(peripheral_address['address'], test_params.scanTimeout).result
    assert len(scan_records) == 0


@pytest.mark.ble41
def test_peripheral_advertising_while_connected(peripheral, peripheral_address, central, test_params):
    """validate that when a peripheral has been connected then start advertising during the connection,
    Gap::isAdvertisingActive==True """
    # start advertising as connectable on peripheral
    peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # Establish the connection
    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(10000)
    central.gap.connect(*test_params.get_connection_args(peripheral_address))

    # wait for advertising to end
    assert peripheral_connection.error is None and peripheral_connection.result['status'] == "BLE_ERROR_NONE"

    # start non connectable advertising on peripheral
    peripheral.advParams.setType("NON_CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # get the Gap state of device 1 and assert that it is equal to what is expected
    state = peripheral.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result
    assert state

    # Check that the device is actually advertising
    central.scanParams.set1mPhyConfiguration(*test_params.scanParams)
    central.gap.setScanParameters()  # apply scanParams
    scan_records = central.gap.scanForAddress(peripheral_address['address'], test_params.scanTimeout).result
    assert len(scan_records) > 0


@pytest.mark.ble41
def test_central_advertising_while_connected(peripheral, peripheral_address, central, central_address, test_params):
    """validate that when a central has been connected then start advertising during the connection,
    Gap::isAdvertisingActive==True """
    # start advertising as connectable on peripheral
    peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # Assert connection will happen on the peripheral side
    connection = peripheral.gap.waitForConnection.setAsync()(10000)

    # Establish the connection
    central.gap.connect(*test_params.get_connection_args(peripheral_address))

    # wait for advertising to end
    assert connection.error is None and connection.result['status'] == "BLE_ERROR_NONE"

    # start non connectable advertising on central
    central.advParams.setType("NON_CONNECTABLE_UNDIRECTED")
    central.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    central.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # get the Gap state of device 2 and assert that it is equal to what is expected
    state = central.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result
    assert state

    # Check that the central is actually advertising
    peripheral.scanParams.set1mPhyConfiguration(*test_params.scanParams)
    peripheral.gap.setScanParameters()  # apply scanParams
    scan_records = peripheral.gap.scanForAddress(central_address['address'], test_params.scanTimeout).result
    assert len(scan_records) > 0


@pytest.mark.ble41
def test_disconnection(peripheral, peripheral_address, central, central_address, test_params):
    """validate that when a central and peripheral are connected and not advertising, after the disconnection,
    Gap::isAdvertisingActive==False """
    # start advertising as connectable on peripheral
    peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # Assert connection will happen on the peripheral side
    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(10000)

    # Establish the connection
    conn_handle = central.gap.connect(*test_params.get_connection_args(peripheral_address)).result['connection_handle']

    # wait for advertising to end
    assert peripheral_connection.error is None and peripheral_connection.result['status'] == "BLE_ERROR_NONE"

    # disconnect
    central.gap.disconnect(conn_handle, "USER_TERMINATION")

    # wait for peripheral disconnection to settle
    sleep(1)

    # get the advertising state of both devices and assert that they are equal to what are expected
    state = central.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result
    assert state is False
    state = peripheral.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result
    assert state is False

    # Check that the central is not advertising
    peripheral.scanParams.set1mPhyConfiguration(*test_params.scanParams)
    peripheral.gap.setScanParameters()  # apply scanParams
    scan_records = peripheral.gap.scanForAddress(central_address['address'], test_params.scanTimeout).result
    assert len(scan_records) == 0

    # Check that the peripheral is not advertising
    central.scanParams.set1mPhyConfiguration(*test_params.scanParams)
    central.gap.setScanParameters()  # apply scanParams
    scan_records = central.gap.scanForAddress(peripheral_address['address'], test_params.scanTimeout).result
    assert len(scan_records) == 0


@pytest.mark.ble41
@pytest.mark.smoketest
def test_advertising_after_disconnection(peripheral, peripheral_address, central, central_address, test_params):
    """validate that when a central and peripheral are connected and advertising after the disconnection,
    Gap::isAdvertisingActive==True and advertisements are received"""

    # start advertising as connectable on peripheral
    peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # Assert connection will happen on the peripheral side
    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(10000)

    # Establish the connection
    conn_handle = central.gap.connect(*test_params.get_connection_args(peripheral_address)).result['connection_handle']

    # wait for advertising to end
    assert peripheral_connection.error is None and peripheral_connection.result['status'] == "BLE_ERROR_NONE"

    # disconnect
    central.gap.disconnect(conn_handle, "USER_TERMINATION")

    # wait for peripheral disconnection to settle
    sleep(1)

    for device in [peripheral, central]:
        # start non connectable advertising
        device.advParams.setType("NON_CONNECTABLE_UNDIRECTED")
        device.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)
        device.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

        # get the advertising state and assert that it is equal to what is expected
        state = device.gap.isAdvertisingActive(LEGACY_ADVERTISING_HANDLE).result
        assert state

        # set up scanning for the other device
        device.scanParams.set1mPhyConfiguration(*test_params.scanParams)
        device.gap.setScanParameters()  # apply scanParams

    scan_records = central.gap.scanForAddress(peripheral_address['address'], test_params.scanTimeout).result
    assert len(scan_records) > 0

    scan_records = peripheral.gap.scanForAddress(central_address['address'], test_params.scanTimeout).result
    assert len(scan_records) > 0

