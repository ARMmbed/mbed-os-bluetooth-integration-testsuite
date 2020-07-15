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

import pytest

from common.ble_device import BleDevice
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator
from common.gap_utils import generate_rand_device_name

NUM_REPEATED_CALLS = 10

GENERIC_ACCESS_SERVICE_UUID = 0x1800

DEVICE_NAME_CHARACTERISTIC_UUID = 0x2A00
APPEARANCE_CHARACTERISTIC_UUID = 0x2A01
# todo peripheral privacy flags
# todo reconnection address
PREFERRED_CONNECTION_PARAMETERS_UUID = 0x2A04


@pytest.fixture(scope="function")
def server(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate("server")
    assert device
    device.ble.init()
    device.advDataBuilder.clear()
    # add flags data into advertising payload
    advertising_flags = ["LE_GENERAL_DISCOVERABLE", "BREDR_NOT_SUPPORTED"]
    for f in advertising_flags:
        device.advDataBuilder.setFlags(f)
    device.gap.applyAdvPayloadFromBuilder(LEGACY_ADVERTISING_HANDLE)

    # set the advertising type
    device.advParams.setType("CONNECTABLE_UNDIRECTED")
    device.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)

    yield device

    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def client(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate('client')
    assert device
    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def client_connection_handle(server, client):
    # get the server mac address
    server_address = server.gap.getAddress().result

    # start advertising
    server.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # connect the client to the server
    connection_server = server.gap.waitForConnection.setAsync()(5000)
    connection = client.gap.connect(
        server_address["address_type"], server_address["address"]
    ).result
    connection_server = connection_server.result  # Wait for the completion of the operation on the server

    # get the handle of the new connection
    return connection["connection_handle"]


def discover_device_name_characteristic(client, client_connection_handle):
    services = client.gattClient.discoverAllServicesAndCharacteristics(client_connection_handle).result
    generic_access_service = [s for s in services if s["UUID"] == GENERIC_ACCESS_SERVICE_UUID][0]
    generic_access_characteristics = generic_access_service["characteristics"]
    return [s for s in generic_access_characteristics if s["UUID"] == DEVICE_NAME_CHARACTERISTIC_UUID][0]


def read_device_name_characteristic(client, client_connection_handle, device_name_char):
    return bytes.fromhex(client.gattClient.readCharacteristicValue(
        client_connection_handle,
        device_name_char["value_handle"]
    ).result["data"]).decode("utf-8")


@pytest.mark.ble41
def test_generic_access_service_existence(client, server, client_connection_handle):
    """A GATT server should provide a well formed Generic Access service"""
    services = client.gattClient.discoverAllServicesAndCharacteristics(client_connection_handle).result

    # check that there is only one definition of the generic access service
    generic_access_services = [s for s in services if s["UUID"] == GENERIC_ACCESS_SERVICE_UUID]
    assert len(generic_access_services) == 1

    generic_access_characteristics = generic_access_services[0]["characteristics"]

    # check that there is only one instance of the device name characteristic
    # and that this characteristic is well formed
    device_name_char = [s for s in generic_access_characteristics if s["UUID"] == DEVICE_NAME_CHARACTERISTIC_UUID]
    assert len(device_name_char) == 1
    properties = device_name_char[0]["properties"]
    assert "read" in properties
    assert set(properties).isdisjoint({"broadcast", "notify", "indicate"})
    # note: write is optional and not tested here

    # check that there is only one instance of the appearance characteristic
    # and that this characteristic is well formed
    appearance_char = [s for s in generic_access_characteristics if s["UUID"] == APPEARANCE_CHARACTERISTIC_UUID]
    assert len(appearance_char) == 1
    assert "read" in properties
    assert set(properties).isdisjoint({"broadcast", "notify", "indicate"})

    # check that there is only one instance of the preferred connection parameters characteristic
    # and that this characteristic is well formed
    connection_params_char = [s for s in generic_access_characteristics if s["UUID"] == PREFERRED_CONNECTION_PARAMETERS_UUID]
    assert len(connection_params_char) == 1
    assert connection_params_char[0]["properties"] == ["read"]


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing BLE API")
def test_device_name_in_generic_access_service(client, server, client_connection_handle):
    """The device name in the generic access service should be readable"""
    device_name_char = discover_device_name_characteristic(client, client_connection_handle)
    device_name = read_device_name_characteristic(client, client_connection_handle, device_name_char)
    # the device name read should match the device name in the server
    assert server.gap.getDeviceName().result == device_name


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing BLE API")
def test_device_name_in_generic_access_service_change(server, client, client_connection_handle):
    """If the device name change, it should be reflected by the device name characteristic
    in the generic access service"""
    device_name_char = discover_device_name_characteristic(client, client_connection_handle)

    for i in range(NUM_REPEATED_CALLS):
        device_name_set = generate_rand_device_name(1, 20)
        server.gap.setDeviceName(device_name_set)
        device_name_read = read_device_name_characteristic(client, client_connection_handle, device_name_char)
        # the device name read should match the device name in the server
        assert device_name_set == device_name_read


@pytest.mark.ble41
def test_client_change_device_name(server, client, client_connection_handle):
    """If the device name is writable in generic access service, the device
    name written by a client can be read back"""
    device_name_char = discover_device_name_characteristic(client, client_connection_handle)

    # if the device name is not writable, skip this test
    if "write" not in device_name_char["properties"]:
        return

    for i in range(NUM_REPEATED_CALLS):
        device_name = generate_rand_device_name(1, 20)
        device_name_hex = "".join([hex(ord(c))[2:].zfill(2).upper() for c in device_name])
        client.gattClient.write(client.connectionHandle, device_name_char["value_handle"], device_name_hex)
        # the device name read should match the device name in the server
        assert device_name == read_device_name_characteristic(client, client_connection_handle, device_name_char)


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing BLE API")
def test_client_change_device_name_visible_to_server(server, client, client_connection_handle):
    """If the device name is writable in generic access service, the device name written by
    a client should be available in the server"""
    device_name_char = discover_device_name_characteristic(client, client_connection_handle)

    # if the device name is not writable, skip this test
    if "write" not in device_name_char["properties"]:
        return

    for i in range(NUM_REPEATED_CALLS):
        device_name = generate_rand_device_name(1, 20)
        device_name_hex = "".join([hex(ord(c))[2:].zfill(2).upper() for c in device_name])
        client.gattClient.write(client_connection_handle, device_name_char["value_handle"], device_name_hex)
        # the device name read should match the device name in the server
        assert device_name, server.gap.getDeviceName().result
