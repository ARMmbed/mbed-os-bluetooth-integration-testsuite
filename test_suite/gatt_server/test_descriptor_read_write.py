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
from common.fixtures import BoardAllocator
from common.gap_utils import gap_connect, discover_descriptors, discover_user_services


@pytest.fixture(scope="function")
def dual_role(board_allocator: BoardAllocator):
    device = board_allocator.allocate('dual_role')
    assert device is not None

    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


def instantiate_descriptor(server, uuid, initial_value=None, fixed_size=True, max_length=None):
    """
    Instantiate the service with the descriptor under test.
    Return the handle of the descriptor under test.
    """
    gatt_server = server.gattServer

    gatt_server.declareService(0xFFFB)
    gatt_server.declareCharacteristic(0xDEAF)
    gatt_server.setCharacteristicProperties("notify", "indicate")
    gatt_server.declareDescriptor(uuid)
    if initial_value is not None:
        gatt_server.setDescriptorValue(initial_value)
    gatt_server.setDescriptorVariableLength(not fixed_size)
    if max_length is not None:
        gatt_server.setDescriptorMaxLength(max_length)
    declared_service = gatt_server.commitService().result

    if initial_value is not None:
        assert initial_value == declared_service["characteristics"][0]["descriptors"][0]["value"]

    return declared_service["characteristics"][0]["descriptors"][0]["handle"]


def verify_client_descriptor_value(client, connection_handle, descriptor_handle, expected):
    value = client.gattClient.readCharacteristicDescriptor(
        connection_handle,
        descriptor_handle
    ).result["data"]
    assert expected == value


def write_fixed_value(central, peripheral):
    initial_value = "00112233"
    partial_updates = ["AA112233", "AAAA2233", "AAAAAA33", "AAAAAAAA"]
    descriptor_length = 4
    descriptor_handle = instantiate_descriptor(peripheral, 0xDEAD, initial_value, True, descriptor_length)

    if central:
        central_handle, peripheral_handle = gap_connect(central, peripheral)

    # writes within fixed size should work by merging the values
    for i in range(1, descriptor_length):
        new_value = "AA" * i
        merged_value = partial_updates[i - 1]
        if central:
            central.gattClient.writeCharacteristicDescriptor(central_handle, descriptor_handle, new_value)
            verify_client_descriptor_value(central, central_handle, descriptor_handle, merged_value)
        else:
            peripheral.gattServer.write(descriptor_handle, new_value)

        assert merged_value == peripheral.gattServer.read(descriptor_handle).result

    # revert to initial value
    peripheral.gattServer.write(descriptor_handle, initial_value)
    if central:
        verify_client_descriptor_value(central, central_handle, descriptor_handle, initial_value)

    # writes larger than fixed size should be completely rejected
    for i in range(descriptor_length + 1, 8):
        new_value = "AA" * i
        if central:
            central.gattClient.writeCharacteristicDescriptor.withRetcode(-1)(central_handle, descriptor_handle, new_value)
            verify_client_descriptor_value(central, central_handle, descriptor_handle, initial_value)
        else:
            peripheral.gattServer.write.withRetcode(-1)(descriptor_handle, new_value)
        # expect no update to value
        assert initial_value == peripheral.gattServer.read(descriptor_handle).result


@pytest.mark.ble41
def test_clients_read_descriptor(central, peripheral, dual_role):
    """It should be possible for a client to read a user defined descriptor."""
    value = "00112233"
    descriptor_handle = instantiate_descriptor(dual_role, 0xDEAD, value)
    central_handle, dual_handle_as_peripheral = gap_connect(central, dual_role)
    dual_handle_as_central, peripheral_handle = gap_connect(dual_role, peripheral)

    verify_client_descriptor_value(central, central_handle, descriptor_handle, value)
    verify_client_descriptor_value(peripheral, peripheral_handle, descriptor_handle, value)


@pytest.mark.ble41
def test_server_read_descriptor(peripheral):
    """It should be possible for a server to read a user defined descriptor."""
    value = "00112233"
    descriptor_handle = instantiate_descriptor(peripheral, 0xDEAD, value)

    assert value == peripheral.gattServer.read(descriptor_handle).result


@pytest.mark.ble41
def test_clients_write_descriptor(central, peripheral, dual_role):
    """It should be possible for a client to write a user defined descriptor."""
    initial_value = "00112233"
    new_value_client1 = "11223344"
    new_value_client2 = "44332211"

    descriptor_handle = instantiate_descriptor(dual_role, 0xDEAD, initial_value)
    central_handle, dual_handle_as_peripheral = gap_connect(central, dual_role)
    dual_handle_as_central, peripheral_handle = gap_connect(dual_role, peripheral)

    def client_write(client, client_handle, new_value):
        client.gattClient.writeCharacteristicDescriptor(client_handle, descriptor_handle, new_value)

        verify_client_descriptor_value(central, central_handle, descriptor_handle, new_value)
        verify_client_descriptor_value(peripheral, peripheral_handle, descriptor_handle, new_value)

        assert new_value == dual_role.gattServer.read(descriptor_handle).result

    client_write(central, central_handle, new_value_client1)
    client_write(peripheral, peripheral_handle, new_value_client2)

@pytest.mark.ble41
def test_server_write_descriptor(central, peripheral):
    """It should be possible for a server to write a user defined descriptor."""
    initial_value = "00112233"
    new_value = "11223344"

    descriptor_handle = instantiate_descriptor(peripheral, 0xDEAD, initial_value)
    central_handle, peripheral_handle = gap_connect(central, peripheral)

    peripheral.gattServer.write(descriptor_handle, new_value)
    verify_client_descriptor_value(central, central_handle, descriptor_handle, new_value)

    assert new_value == peripheral.gattServer.read(descriptor_handle).result


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing cli-app command to control descriptor permission")
def test_client_fail_write_not_writable_descriptor(central, peripheral):
    """It should not be possible for a client to write a descriptor not writable."""
    # user description descriptor is not writable by default, use this property to simulate this behavior
    initial_value = "001122"
    new_value = "112233"
    user_description_uuid = 0x2901

    # note: the descriptor handle returned by BLE_API is incorrect for user description
    descriptor_handle = instantiate_descriptor(dual_role, user_description_uuid, initial_value)
    central_handle, peripheral_handle = gap_connect(central, peripheral)
    gap_connect(dual_role, peripheral)

    central.gattClient.writeCharacteristicDescriptor(central_handle, descriptor_handle, new_value)
    verify_client_descriptor_value(central, central_handle, descriptor_handle, initial_value)


@pytest.mark.ble41
def test_server_write_not_writable_descriptor(central, peripheral):
    """It should be possible for a server to write a descriptor not writable by a client."""
    # user description descriptor is not writable by default, use this property to simulate this behavior
    initial_value = "001122"
    new_value = "112233"
    user_description_uuid = 0x2901

    # note: the descriptor handle returned by BLE_API is incorrect for user description
    descriptor_handle = instantiate_descriptor(peripheral, user_description_uuid, initial_value)
    central_handle, peripheral_handle = gap_connect(central, peripheral)

    peripheral.gattServer.write(descriptor_handle, new_value)
    verify_client_descriptor_value(central, central_handle, descriptor_handle, new_value)

    assert new_value == peripheral.gattServer.read(descriptor_handle).result


@pytest.mark.ble41
def test_server_catch_write_to_descriptor(central, peripheral):
    """It should be possible for a server to catch when a client write one of its descriptor."""
    initial_value = "00112233"
    new_value = "11223344"

    descriptor_handle = instantiate_descriptor(peripheral, 0xDEAD, initial_value)
    central_handle, peripheral_handle = gap_connect(central, peripheral)

    data_written_event = peripheral.gattServer.waitForDataWritten.setAsync()(peripheral_handle,
                                                                             descriptor_handle, 5000)

    central.gattClient.writeCharacteristicDescriptor(central_handle, descriptor_handle, new_value)

    assert 0 == data_written_event.status


@pytest.mark.ble41
def test_server_fail_write_descriptor_too_big(peripheral):
    """It should not be possible for a server to write a value larger than what a descriptor can accept"""
    initial_value = "00112233"
    new_value = "1122334455"

    descriptor_handle = instantiate_descriptor(peripheral, 0xDEAD, initial_value, max_length=4)

    peripheral.gattServer.write.withRetcode(-1)(descriptor_handle, new_value)
    assert initial_value == peripheral.gattServer.read(descriptor_handle).result


@pytest.mark.ble41
def test_client_fail_write_descriptor_too_big(central, peripheral):
    """It should not be possible for a client to write a value larger than what a descriptor can accept"""
    initial_value = "00112233"
    new_value = "1122334455"

    descriptor_handle = instantiate_descriptor(peripheral, 0xDEAD, initial_value, max_length=4)
    central_handle, peripheral_handle = gap_connect(central, peripheral)

    central.gattClient.writeCharacteristicDescriptor.withRetcode(-1)(central_handle, descriptor_handle, new_value)
    verify_client_descriptor_value(central, central_handle, descriptor_handle, initial_value)

    assert initial_value == peripheral.gattServer.read(descriptor_handle).result


@pytest.mark.ble41
def test_server_write_descriptor_within_size(peripheral):
    """If the size of a descriptor is variable then it should be possible for a server to write this descriptor
    with a value which fit in the length of the descriptor."""
    initial_value = "00112233"
    descriptor_length = 8
    descriptor_handle = instantiate_descriptor(peripheral, 0xDEAD, initial_value, False, descriptor_length)

    for i in range(1, descriptor_length):
        new_value = "AA" * i
        peripheral.gattServer.write(descriptor_handle, new_value)
        assert new_value == peripheral.gattServer.read(descriptor_handle).result


@pytest.mark.ble41
def test_client_write_descriptor_within_size(central, peripheral):
    """If the size of a descriptor is variable then it should be possible for a client to write this descriptor
    with a value which fit in the length of the descriptor."""
    initial_value = "00112233"
    descriptor_length = 8
    descriptor_handle = instantiate_descriptor(peripheral, 0xDEAD, initial_value, False, descriptor_length)
    central_handle, peripheral_handle = gap_connect(central, peripheral)

    for i in range(1, descriptor_length):
        new_value = "AA" * i
        central.gattClient.writeCharacteristicDescriptor(central_handle, descriptor_handle, new_value)
        verify_client_descriptor_value(central, central_handle, descriptor_handle, new_value)

        assert new_value == peripheral.gattServer.read(descriptor_handle).result


@pytest.mark.ble41
def test_server_fail_write_descriptor_when_larger_than_fixed_size(peripheral):
    """If a descriptor has been instantiated with a fixed size then it should be possible for a server to write
    to this descriptor with a smaller or equal size value but not larger."""
    write_fixed_value(None, peripheral)

@pytest.mark.ble41
def test_client_fail_write_descriptor_when_larger_than_fixed_size(central, peripheral):
    """If a descriptor has been instantiated with a fixed size then it should be possible for a client to write
    to this descriptor with a smaller or equal size value but not larger."""
    write_fixed_value(central, peripheral)


@pytest.mark.ble41
def test_cccd_not_shared_between_clients(central, peripheral, dual_role):
    """Client configuration descriptor values should not be shared across several clients"""
    cccd_uuid = 0x2902

    # CCCD is in the characteristic, there is no need to redeclare it, instantiate another random descriptor
    instantiate_descriptor(dual_role, 0xDEAD, "FEED")
    central_handle, dual_handle_as_peripheral = gap_connect(central, dual_role)
    dual_handle_as_central, peripheral_handle = gap_connect(dual_role, peripheral)

    services = discover_user_services(central, central_handle)
    characteristic = services[0]["characteristics"][0]
    # discover this descriptor
    descriptors = discover_descriptors(
        central,
        central_handle,
        characteristic["start_handle"],
        characteristic["end_handle"]
    )

    for descriptor in descriptors:
        if descriptor["UUID"] == cccd_uuid:
            descriptor_handle = descriptor["handle"]
            break

    def verify_cccds(expected_value_client_0, expected_value_client_1):
        verify_client_descriptor_value(central, central_handle, descriptor_handle, expected_value_client_0)
        assert expected_value_client_0 == dual_role.gattServer.read(descriptor_handle, dual_handle_as_peripheral).result

        verify_client_descriptor_value(peripheral, peripheral_handle, descriptor_handle, expected_value_client_1)
        assert expected_value_client_1 == dual_role.gattServer.read(descriptor_handle, dual_handle_as_central).result

    def write_value(client, handle, new_value):
        client.gattClient.writeCharacteristicDescriptor(handle, descriptor_handle, new_value)

    # at the beginning, CCCD should be equal to 0000
    verify_cccds("0000", "0000")

    # enable notifications for client 0 (note, send in little endian)
    write_value(central, central_handle, "0100")
    verify_cccds("0100", "0000")

    # enable indications for client 1 (note, send in little endian)
    write_value(peripheral, peripheral_handle, "0200")
    verify_cccds("0100", "0200")

    # disable notifications for client 0 (note, send in little endian)
    write_value(central, central_handle, "0000")
    verify_cccds("0000", "0200")

    # enable notification for client 1 (note, send in little endian)
    write_value(peripheral, peripheral_handle, "0100")
    verify_cccds("0000", "0100")
