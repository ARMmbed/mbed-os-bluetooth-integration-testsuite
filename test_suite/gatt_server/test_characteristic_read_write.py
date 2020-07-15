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
from common.gap_utils import gap_connect


def instantiate_service(peripheral, properties, initial_value=None, fixed_size=True, max_length=None):
    """
    Instantiate the service with the characteristic under test.

    Args:
        peripheral (BleDevice): device where the gatt server resides.
        properties (List[str]): Properties of the characteristic.
        initial_value (str): HEX representation of the initial value of the characteristic.
        fixed_size (Bool): True if the size of the characteristic is fixed and False otherwise.
        max_length (int): Maximum number of bytes occupied by the characteristic value.

    Returns:
        int: handle of the characteristic under test
    """
    gatt_server = peripheral.gattServer

    gatt_server.declareService(0xFFFB)
    gatt_server.declareCharacteristic(0xDEAF)
    gatt_server.setCharacteristicProperties(*properties)
    if initial_value is not None:
        gatt_server.setCharacteristicValue(initial_value)
    if fixed_size is True:
        gatt_server.setCharacteristicVariableLength(False)
    else:
        gatt_server.setCharacteristicVariableLength(True)
    if max_length is not None:
        gatt_server.setCharacteristicMaxLength(max_length)
    declared_service = gatt_server.commitService().result

    if initial_value is not None:
        assert initial_value == declared_service["characteristics"][0]["value"]

    return declared_service["characteristics"][0]["value_handle"]


@pytest.mark.ble41
def test_read_characteristic(central, peripheral):
    """Reading a characteristic after its instantiation should return the value set during its instantiation"""
    characteristic_value = "00112233"
    characteristic_handle = instantiate_service(peripheral, ["read"], characteristic_value)
    assert characteristic_value == peripheral.gattServer.read(characteristic_handle).result
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)
    value_read_from_client = central.gattClient.readCharacteristicValue(
        client_connection_handle,
        characteristic_handle
    ).result["data"]
    assert characteristic_value == value_read_from_client


@pytest.mark.ble41
@pytest.mark.skip(reason="BLE API defect")
def test_fail_to_write_not_writable(central, peripheral):
    """It should not be possible for a client to write the value of a characteristic which is not writable."""
    characteristic_handle = instantiate_service(peripheral, ["read"], "00112233")

    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)
    _ = central.gattClient.write.withRetcode(-1)(
        client_connection_handle,
        characteristic_handle,
        "44556677"
    ).result


@pytest.mark.ble41
def test_write_not_writable_by_server(central, peripheral):
    """It should be possible for a server to write the value of a characteristic not writable by a client."""
    characteristic_handle = instantiate_service(peripheral, ["read"], "00112233")
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)
    new_value = "11223344"
    peripheral.gattServer.write(characteristic_handle, new_value)
    assert new_value == peripheral.gattServer.read(characteristic_handle).result
    value_read_from_client = central.gattClient.readCharacteristicValue(
        client_connection_handle,
        characteristic_handle
    ).result["data"]
    assert new_value == value_read_from_client


@pytest.mark.ble41
@pytest.mark.skip(reason="BLE API defect")
def test_fail_to_read_unreadable(central, peripheral):
    """It should not be possible for a client to read the value of a characteristic which is not readable."""
    characteristic_handle = instantiate_service(peripheral, ["write"], "00112233")
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)
    central.gattClient.readCharacteristicValue.withRetcode(-1)(
        client_connection_handle,
        characteristic_handle
    )


@pytest.mark.ble41
def test_write_writable(central, peripheral):
    """It should be possible for a client to write the value of a characteristic if the characteristic is writable."""
    characteristic_handle = instantiate_service(peripheral, ["read", "write"], "00112233")
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)

    # write the new value
    new_value = "11223344"
    central.gattClient.write(
        client_connection_handle,
        characteristic_handle,
        new_value
    )

    # check that the value is correct
    assert new_value == peripheral.gattServer.read(characteristic_handle).result
    value_read_from_client = central.gattClient.readCharacteristicValue(
        client_connection_handle,
        characteristic_handle
    ).result["data"]
    assert new_value == value_read_from_client


@pytest.mark.ble41
@pytest.mark.smoketest
def test_callback_on_write(central, peripheral):
    """It should be possible for a server to catch when a client write one of its characteristic."""
    characteristic_handle = instantiate_service(peripheral, ["read", "write"], "00112233")
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)

    # write the new value
    new_value = "11223344"
    write_event = peripheral.gattServer.waitForDataWritten.setAsync()(
        server_connection_handle,
        characteristic_handle,
        10000
    )
    central.gattClient.write(
        client_connection_handle,
        characteristic_handle,
        new_value
    )
    assert 0 == write_event.status
    assert server_connection_handle == write_event.result["connection_handle"]
    assert characteristic_handle == write_event.result["attribute_handle"]
    assert new_value == write_event.result["data"]


@pytest.mark.ble41
def test_server_fail_to_write_value_larger_than_max(central, peripheral):
    """It should not be possible for a server to write a value larger than what a characteristic can accept"""
    initial_value = "00112233"

    # test for a characteristic with a fixed length
    characteristic_handle = instantiate_service(peripheral, ["read", "write"], initial_value)
    peripheral.gattServer.write.withRetcode(-1)(characteristic_handle, initial_value + "55")
    assert initial_value == peripheral.gattServer.read(characteristic_handle).result

    # test for a characteristic with a variable length
    characteristic_handle = instantiate_service(
        peripheral, ["read", "write"], initial_value, False, len(initial_value) // 2
    )
    peripheral.gattServer.write.withRetcode(-1)(characteristic_handle, initial_value + "55")
    assert initial_value == peripheral.gattServer.read(characteristic_handle).result


@pytest.mark.ble41
@pytest.mark.skip(reason="BLE API defect")
def test_client_fail_to_write_value_larger_than_max(central, peripheral):
    """It should not be possible for a client to write a value larger than what a characteristic can accept"""
    initial_value = "00112233"
    fixed_characteristic_handle = instantiate_service(peripheral, ["read", "write"], initial_value)
    variable_characteristic_handle = instantiate_service(
        peripheral, ["read", "write"], initial_value, True, len(initial_value) // 2
    )
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)

    central.gattClient.write.withRetcode(-1)(
        client_connection_handle,
        fixed_characteristic_handle,
        initial_value + "55"
    )

    central.gattClient.write.withRetcode(-1)(
        client_connection_handle,
        variable_characteristic_handle,
        initial_value + "55"
    )

    assert initial_value == peripheral.gattServer.read(fixed_characteristic_handle).result
    assert initial_value == peripheral.gattServer.read(variable_characteristic_handle).result


@pytest.mark.ble41
def test_server_write_within_variable_size(central, peripheral):
    """It should be possible for a server to write any characteristic value which fit in the length of
    the characteristic if the size of the characteristic is variable."""
    initial_value = "00112233"
    characteristic_length = 8
    # The characteristic len is not fixed and its length is twice the length of the initial value.
    characteristic_handle = instantiate_service(
        peripheral, ["read", "write"], initial_value, False, characteristic_length
    )

    for i in range(1, characteristic_length):
        new_value = "AA" * i
        peripheral.gattServer.write(characteristic_handle, new_value)
        assert new_value == peripheral.gattServer.read(characteristic_handle).result


@pytest.mark.ble41
def test_client_write_within_variable_size(central, peripheral):
    """It should be possible for a client to write any characteristic value which fit in the length of
    the characteristic if the size of the characteristic is variable."""
    initial_value = "00112233"
    characteristic_length = 8
    # The characteristic len is not fixed and its length is twice the length of the initial value.
    characteristic_handle = instantiate_service(
        peripheral, ["read", "write"], initial_value, False, characteristic_length
    )
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)

    for i in range(1, characteristic_length):
        new_value = "AA" * i
        central.gattClient.write(
            client_connection_handle,
            characteristic_handle,
            new_value
        )
        assert new_value == peripheral.gattServer.read(characteristic_handle).result


@pytest.mark.ble41
@pytest.mark.skip(reason="BLE API defect")
def test_server_fail_write_fixed_size_with_wrong_size(central, peripheral):
    """If a characteristic has been instantiated with a fixed size then it should not be possible for a server to
    write this characteristic with a value of a different size."""
    initial_value = "00112233"
    characteristic_length = len(initial_value) // 2
    characteristic_handle = instantiate_service(
        peripheral, ["read", "write"], initial_value, True, characteristic_length
    )

    for i in range(1, characteristic_length * 2):
        if i == characteristic_length:
            continue
        new_value = "AA" * i
        peripheral.gattServer.write.withRetcode(-1)(characteristic_handle, new_value)
        assert initial_value == peripheral.gattServer.read(characteristic_handle).result


@pytest.mark.ble41
@pytest.mark.skip(reason="BLE API defect")
def test_client_fail_write_fixed_size_with_wrong_size(central, peripheral):
    """If a characteristic has been instantiated with a fixed size then it should not be possible for a client to
    write this characteristic with a value of a different size."""
    initial_value = "00112233"
    characteristic_length = len(initial_value) // 2
    characteristic_handle = instantiate_service(
        peripheral, ["read", "write"], initial_value, True, characteristic_length
    )
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)

    for i in range(1, characteristic_length * 2):
        if i == characteristic_length:
            continue
        new_value = "AA" * i
        central.gattClient.write.withRetcode(-1)(
            client_connection_handle,
            characteristic_handle,
            new_value
        )
        assert initial_value == peripheral.gattServer.read(characteristic_handle).result


@pytest.mark.ble41
def test_server_read_invalid_attribute(central, peripheral):
    """If a server try to read an invalid attribute handle, it should return an error"""
    peripheral.gattServer.read.withRetcode(-1)(0xFFFE)


def test_server_write_invalid_attribute(central, peripheral):
    """If a server try to write an invalid attribute handle, it should return an error"""
    peripheral.gattServer.write.withRetcode(-1)(0xFFFE, "00")


@pytest.mark.ble41
@pytest.mark.skip(reason="BLE API defect")
def test_client_read_invalid_attribute(central, peripheral):
    """If a client try to read an invalid attribute handle, it should return an error"""
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)
    central.gattClient.readCharacteristicValue.withRetcode(-1)(
        client_connection_handle,
        0xFFEE
    )


@pytest.mark.ble41
@pytest.mark.skip(reason="BLE API defect")
def test_client_write_invalid_attribute(central, peripheral):
    """If a client try to write an invalid attribute handle, it should return an error"""
    client_connection_handle, server_connection_handle = gap_connect(central, peripheral)
    central.gattClient.write.withRetcode(-1)(
        client_connection_handle,
        0xFFEE,
        "AA"
    )
