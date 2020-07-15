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
from common.fixtures import BoardAllocator
from common.gap_utils import get_rand_data

ADV_FLAGS = ["LE_GENERAL_DISCOVERABLE", "BREDR_NOT_SUPPORTED"]

RANDOM_NON_RESOLVABLE_MSB_RANGE = ['0', '1', '2', '3']
RANDOM_RESOLVABLE_MSB_RANGE = ['4', '5', '6', '7']
CONNECTION_PARAMS = [50, 100, 0, 600]
CONNECTION_TIMEOUT = 10000
CONNECTION_SCAN_PARAMS = [100, 100, 0, False]
SCAN_TIMEOUT = 500
SCAN_PARAMS = [10, 10, 0, 1]


def init_device(device: BleDevice):
    device.ble.init()
    device.securityManager.init(True, False, 'IO_CAPS_NONE', '*', True, '*')
    device.gap.enablePrivacy(True)
    device.securityManager.setPairingRequestAuthorisation(False)


def configure_peripheral(peripheral, rand_data):
    # setup default privacy mode
    peripheral.gap.setPeripheralPrivacyConfiguration(
        False,
        "PERFORM_PAIRING_PROCEDURE"
    )

    # setup advertising payload
    peripheral.gap.clearAdvertisingPayload()
    peripheral.gap.accumulateAdvertisingPayload("FLAGS", *ADV_FLAGS)
    peripheral.gap.accumulateAdvertisingPayload("MANUFACTURER_SPECIFIC_DATA", rand_data)
    peripheral.gap.setAdvertisingInterval(100)
    peripheral.gap.setAdvertisingTimeout(0)

    # Scan parameter
    peripheral.gap.setScanParams(*SCAN_PARAMS)


def get_connection_args(address_type, address):
    connection_args = [address_type, address]
    connection_args.extend(CONNECTION_PARAMS)
    connection_args.extend(CONNECTION_SCAN_PARAMS)
    connection_args.append(CONNECTION_TIMEOUT)
    return connection_args


def address_is_random_resolvable(address):
    return address[0] in RANDOM_RESOLVABLE_MSB_RANGE


def address_is_random_non_resolvable(address):
    return address[0] in RANDOM_NON_RESOLVABLE_MSB_RANGE


def address_matches_type(address, address_type):
    if address_type == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE":
        return address_is_random_resolvable(address)
    else:
        return address_is_random_non_resolvable(address)


def get_advertising_data(peripheral):
    return peripheral.gap.getAdvertisingPayload().result['raw']


def get_peripheral_address(central, peripheral):
    # note advertising MUST be started; we cannot start and stop it in this
    # function as the address may changed if it is restarted after
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result

    return (
        scan_records[0]["peerAddrType"],
        scan_records[0]["peerAddr"]
    )


def perform_bonding(peripheral, central):
    peripheral.gap.setAdvertisingType("ADV_CONNECTABLE_UNDIRECTED")
    peripheral.gap.startAdvertising()

    peripheral_address_type, peripheral_address = get_peripheral_address(central, peripheral)

    # connect to the device
    connection_params = get_connection_args(
        peripheral_address_type,
        peripheral_address
    )
    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(1000)
    central_connection = central.gap.connect(*connection_params).result
    peripheral_connection = peripheral_connection.result

    # wait for security manager
    peripheral_bond = peripheral.securityManager.waitForEvent.setAsync()(
        peripheral_connection["handle"],
        15000
    )

    central_bond = central.securityManager.waitForEvent(
        central_connection["handle"],
        15000
    ).result
    peripheral_bond = peripheral_bond.result

    central.gap.disconnect(central_connection["handle"], "REMOTE_USER_TERMINATED_CONNECTION")

    # FIXME: needs event from the peripheral and central that they have been
    # disconnected
    sleep(1)


@pytest.fixture(scope="function")
def rand_data():
    rand_data = get_rand_data("MANUFACTURER_SPECIFIC_DATA")
    # make sure that the data have an acceptable size
    if len(rand_data) > 16:
        rand_data = rand_data[0:16]
    return rand_data


@pytest.fixture(scope="function")
def peripheral(board_allocator: BoardAllocator, rand_data):
    device = board_allocator.allocate('peripheral')
    assert device is not None
    init_device(device)

    configure_peripheral(device, rand_data)
    yield device

    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def unknown_peripheral(board_allocator: BoardAllocator, rand_data):
    device = board_allocator.allocate('peripheral')
    assert device is not None
    init_device(device)

    configure_peripheral(device, rand_data)
    yield device

    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def central(board_allocator: BoardAllocator):
    device = board_allocator.allocate('central')
    assert device is not None
    init_device(device)
    device.gap.setCentralPrivacyConfiguration(
        False,
        "RESOLVE_AND_FORWARD"
    )

    yield device

    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def unknown_central(board_allocator: BoardAllocator):
    device = board_allocator.allocate('central')
    assert device is not None
    init_device(device)

    yield device

    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def peripheral_address(peripheral):
    yield peripheral.gap.getAddress().result


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
@pytest.mark.parametrize("advertising_type", [
     "ADV_CONNECTABLE_UNDIRECTED",
     # "ADV_CONNECTABLE_DIRECTED",   # Directed advertising not supported yet
     "ADV_SCANNABLE_UNDIRECTED",
     "ADV_NON_CONNECTABLE_UNDIRECTED"
])
def test_privacy_random_resolvable_address(peripheral, central, advertising_type):
    """validate that when privacy is enabled and a peripheral use a random
    resolvable address then a random resolvable address is used for all
    advertising modes."""
    peripheral.gap.setAdvertisingType(advertising_type)
    peripheral.gap.startAdvertising()
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result
    assert len(scan_records) > 0

    for scan in scan_records:
        assert scan["addressType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert scan["peerAddrType"] == "RANDOM"
        assert address_is_random_resolvable(scan["peerAddr"])

    peripheral.gap.stopAdvertising()


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
@pytest.mark.parametrize("advertising_type, address_type", [
    ("ADV_CONNECTABLE_UNDIRECTED", "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"),
    # ("ADV_CONNECTABLE_DIRECTED", "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"),  # Directed advertising not supported yet
    ("ADV_SCANNABLE_UNDIRECTED", "ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE"),
    ("ADV_NON_CONNECTABLE_UNDIRECTED", "ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE")
])
def test_privacy_random_non_resolvable_address(peripheral, central, advertising_type, address_type):
    """validate that when privacy is enabled and a peripheral use a random non
    resolvable address then a random non resolvable address is used during
    non connectable advertising and a resolvable one is used during
    connectable advertising."""
    # enable non resolvable addresses
    peripheral.gap.setPeripheralPrivacyConfiguration(
        True,
        "PERFORM_PAIRING_PROCEDURE"
    )

    peripheral.gap.setAdvertisingType(advertising_type)
    peripheral.gap.startAdvertising()
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result
    assert len(scan_records) > 0

    for scan in scan_records:
        assert scan["addressType"] == address_type
        assert scan["peerAddrType"] == "RANDOM"
        assert address_matches_type(scan["peerAddr"], address_type)

    peripheral.gap.stopAdvertising()


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
def test_perform_pairing_procedure_policy(central, peripheral):
    """validate that when a peripheral with privacy enabled with the policy
    PERFORM_PAIRING_PROCEDURE is connected by an unknown privacy enabled
    central then the pairing procedure is performed"""
    peripheral.gap.setAdvertisingType("ADV_CONNECTABLE_UNDIRECTED")
    peripheral.gap.startAdvertising()

    # get the private address of the device
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result
    assert len(scan_records) > 0

    peripheral_address_type = scan_records[0]["peerAddrType"]
    peripheral_address = scan_records[0]["peerAddr"]

    # connect to the device
    connection_params = get_connection_args(
        peripheral_address_type,
        peripheral_address
    )
    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(1000)
    central_connection = central.gap.connect(*connection_params).result
    peripheral_connection = peripheral_connection.result

    for connection_result in [central_connection, peripheral_connection]:
        assert connection_result["ownAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddressType"] == "RANDOM"

    # wait for security manager
    peripheral_bond = peripheral.securityManager.waitForEvent.setAsync()(
        peripheral_connection["handle"],
        15000
    )

    central_bond = central.securityManager.waitForEvent(
        central_connection["handle"],
        15000
    ).result
    peripheral_bond = peripheral_bond.result

    for bond in [central_bond, peripheral_bond]:
        assert bond["status"] == "pairingResult"
        assert bond["param"] == "SEC_STATUS_SUCCESS"


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
@pytest.mark.parametrize("advertising_type", [
     "ADV_CONNECTABLE_UNDIRECTED",
     # "ADV_CONNECTABLE_DIRECTED",  # Directed advertising not supported yet
     "ADV_SCANNABLE_UNDIRECTED",
     "ADV_NON_CONNECTABLE_UNDIRECTED"
])
def test_address_resolution_when_advertised(peripheral, central, advertising_type):
    """validate that when a peripheral with privacy enabled using private
    resolvable address advertise then a known peer that resolves addresses
    resolve the address advertised"""
    perform_bonding(peripheral, central)

    peripheral.gap.setAdvertisingType(advertising_type)
    peripheral.gap.startAdvertising()
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result
    assert len(scan_records) > 0

    for scan in scan_records:
        assert scan["peerAddrType"] in ["PUBLIC_IDENTITY", "RANDOM_STATIC_IDENTITY"]
        if scan["peerAddrType"] == "RANDOM_STATIC_IDENTITY":
            assert address_is_random_non_resolvable(scan["peerAddr"]) is False
            assert address_is_random_resolvable(scan["peerAddr"]) is False

    peripheral.gap.stopAdvertising()


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
def test_connection_with_resolved_address(peripheral, central):
    """validate that when a peripheral with privacy enabled using private
    resolvable address advertise then a known peer that resolves addresses
    can connect with the resolved address"""
    perform_bonding(peripheral, central)

    # connect to the device
    peripheral.gap.startAdvertising()
    address_type, address = get_peripheral_address(central, peripheral)

    connection_params = get_connection_args(
        address_type,
        address
    )
    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(1000)
    central_connection = central.gap.connect(*connection_params).result
    peripheral_connection = peripheral_connection.result

    for connection_result in [central_connection, peripheral_connection]:
        assert connection_result["ownAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddressType"] == address_type
        if address_type == "RANDOM_STATIC_IDENTITY":
            assert connection_result["peerAddrType"] == "ADDR_TYPE_RANDOM_STATIC"
        else:
            assert connection_result["peerAddrType"] == "ADDR_TYPE_PUBLIC"

    assert central_connection["peerAddr"] == address


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
@pytest.mark.parametrize("advertising_type", [
     "ADV_CONNECTABLE_UNDIRECTED",
     # "ADV_CONNECTABLE_DIRECTED",  # Directed advertising not supported yet
     "ADV_SCANNABLE_UNDIRECTED",
     "ADV_NON_CONNECTABLE_UNDIRECTED"
])
def test_central_not_resolve_policy(peripheral, central, advertising_type):
    """validate that when a peripheral with privacy enabled using private
    resolvable address advertise then a known peer that do not resolves
    addresses do not resolve the address advertised"""
    perform_bonding(peripheral, central)

    central.gap.setCentralPrivacyConfiguration(
        False,
        "DO_NOT_RESOLVE"
    )

    peripheral.gap.setAdvertisingType(advertising_type)
    peripheral.gap.startAdvertising()
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result
    assert len(scan_records) > 0

    for scan in scan_records:
        assert "RANDOM" == scan["peerAddrType"]
        assert address_is_random_resolvable(scan["peerAddr"])

    peripheral.gap.stopAdvertising()


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
def test_connection_to_non_resolved_address(peripheral, central):
    """validate that when a peripheral with privacy enabled using private
    resolvable address advertise then a known peer that do not resolves
    addresses can connect to the non resolved address"""
    perform_bonding(peripheral, central)

    central.gap.setCentralPrivacyConfiguration(
        False,
        "DO_NOT_RESOLVE"
    )

    peripheral.gap.setPeripheralPrivacyConfiguration(
        False,
        "DO_NOT_RESOLVE"
    )

    # connect to the device
    peripheral.gap.startAdvertising()
    address_type, address = get_peripheral_address(central, peripheral)
    assert "RANDOM" == address_type

    connection_params = get_connection_args(
        address_type,
        address
    )
    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(1000)
    central_connection = central.gap.connect(*connection_params).result
    peripheral_connection = peripheral_connection.result

    for connection_result in [central_connection, peripheral_connection]:
        assert connection_result["ownAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddressType"] == address_type
        assert connection_result["peerAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"

    assert central_connection["peerAddr"] == address


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
def test_resolve_private_address(peripheral, central):
    """Ensure that if the central privacy policy is set to resolve and filter
    and the device has no bond then resolved private addresses are not filtered"""
    advertising_types = [
        "ADV_CONNECTABLE_UNDIRECTED",
        # "ADV_CONNECTABLE_DIRECTED",
        "ADV_SCANNABLE_UNDIRECTED",
        "ADV_NON_CONNECTABLE_UNDIRECTED"
    ]

    central.gap.setCentralPrivacyConfiguration(
        False,
        "RESOLVE_AND_FILTER"
    )

    # test that scan works as intended
    for advertising_type in advertising_types:
        peripheral.gap.setAdvertisingType(advertising_type)
        peripheral.gap.startAdvertising()
        scan_records = central.gap.startScan(
            SCAN_TIMEOUT,
            get_advertising_data(peripheral)
        ).result
        assert len(scan_records) > 0

        for scan in scan_records:
            assert scan["addressType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
            assert scan["peerAddrType"] == "RANDOM"
            assert address_is_random_resolvable(scan["peerAddr"])

        peripheral.gap.stopAdvertising()

    # test that connection works as intended
    peripheral.gap.setAdvertisingType("ADV_CONNECTABLE_UNDIRECTED")
    peripheral.gap.startAdvertising()

    # get the private address of the device
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result
    assert len(scan_records) > 0

    peripheral_address_type = scan_records[0]["peerAddrType"]
    peripheral_address = scan_records[0]["peerAddr"]

    # connect to the device
    connection_params = get_connection_args(
        peripheral_address_type,
        peripheral_address
    )
    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(1000)
    central_connection = central.gap.connect(*connection_params).result
    peripheral_connection = peripheral_connection.result

    for connection_result in [central_connection, peripheral_connection]:
        assert connection_result["ownAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddressType"] == "RANDOM"


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
@pytest.mark.parametrize("advertising_type", [
     "ADV_CONNECTABLE_UNDIRECTED",
     # "ADV_CONNECTABLE_DIRECTED",
     "ADV_SCANNABLE_UNDIRECTED",
     "ADV_NON_CONNECTABLE_UNDIRECTED"
])
def test_resolve_and_filter_policy_with_unknown_peripheral(central, peripheral, unknown_peripheral, advertising_type):
    """Ensure that if the central privacy policy is set to resolve and filter
    and the device has one bond then non resolved private addresses are filtered"""
    perform_bonding(peripheral, central)

    central.gap.setCentralPrivacyConfiguration(
        False,
        "RESOLVE_AND_FILTER"
    )

    # test that scan works as intended
    unknown_peripheral.gap.setAdvertisingType(advertising_type)
    unknown_peripheral.gap.startAdvertising()
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(unknown_peripheral)
    ).result
    assert len(scan_records) == 0
    unknown_peripheral.gap.stopAdvertising()


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
def test_non_resolved_address_with_resolve_and_forward(peripheral, central, unknown_peripheral):
    """Ensure that if the central privacy policy is set to resolve and forward
    and the device has one bond then non resolved private addresses are forwarded"""
    perform_bonding(peripheral, central)

    advertising_types = [
        "ADV_CONNECTABLE_UNDIRECTED",
        # "ADV_CONNECTABLE_DIRECTED",
        "ADV_SCANNABLE_UNDIRECTED",
        "ADV_NON_CONNECTABLE_UNDIRECTED"
    ]

    central.gap.setCentralPrivacyConfiguration(
        False,
        "RESOLVE_AND_FORWARD"
    )

    # test that scan works as intended
    for advertising_type in advertising_types:
        unknown_peripheral.gap.setAdvertisingType(advertising_type)
        unknown_peripheral.gap.startAdvertising()
        scan_records = central.gap.startScan(
            SCAN_TIMEOUT,
            get_advertising_data(unknown_peripheral)
        ).result
        assert len(scan_records) > 0

        for scan in scan_records:
            assert scan["addressType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
            assert scan["peerAddrType"] == "RANDOM"
            assert address_is_random_resolvable(scan["peerAddr"])

        unknown_peripheral.gap.stopAdvertising()

    # test that connection works as intended
    unknown_peripheral.gap.setAdvertisingType("ADV_CONNECTABLE_UNDIRECTED")
    unknown_peripheral.gap.startAdvertising()

    # get the private address of the device
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(unknown_peripheral)
    ).result
    assert len(scan_records) > 0

    peripheral_address_type = scan_records[0]["peerAddrType"]
    peripheral_address = scan_records[0]["peerAddr"]

    # connect to the device
    connection_params = get_connection_args(
        peripheral_address_type,
        peripheral_address
    )
    peripheral_connection = unknown_peripheral.gap.waitForConnection.setAsync()(1000)
    central_connection = central.gap.connect(*connection_params).result
    peripheral_connection = peripheral_connection.result

    for connection_result in [central_connection, peripheral_connection]:
        assert connection_result["ownAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddressType"] == "RANDOM"


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
def test_reject_non_resolved_address_with_no_bond(peripheral, central):
    """validate that when a peripheral with privacy enabled, the privacy policy
    set to REJECT_NON_RESOLVED_ADDRESS and no bond is connected by an unknown
    privacy enabled then connection is accepted"""
    peripheral.gap.setPeripheralPrivacyConfiguration(
        False,
        "REJECT_NON_RESOLVED_ADDRESS"
    )

    peripheral.gap.setAdvertisingType("ADV_CONNECTABLE_UNDIRECTED")
    peripheral.gap.startAdvertising()

    # get the private address of the device
    scan_records = central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result
    assert len(scan_records) > 0

    peripheral_address_type = scan_records[0]["peerAddrType"]
    peripheral_address = scan_records[0]["peerAddr"]

    # connect to the device
    connection_params = get_connection_args(
        peripheral_address_type,
        peripheral_address
    )

    peripheral_connection = peripheral.gap.waitForConnection.setAsync()(1000)
    central_connection = central.gap.connect(*connection_params).result
    peripheral_connection = peripheral_connection.result

    for connection_result in [central_connection, peripheral_connection]:
        assert connection_result["ownAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddrType"] == "ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE"
        assert connection_result["peerAddressType"] == "RANDOM"

    # wait few seconds to ensure the connection is stable
    sleep(4)

    expected_state = {
        'advertising': False,
        'connected': True
    }

    assert expected_state == peripheral.gap.getState().result
    assert expected_state == central.gap.getState().result


@pytest.mark.ble42
@pytest.mark.skip(reason="Missing BLE API")
def test_reject_non_resolved_address_with_one_bond(peripheral, central, unknown_central):
    """validate that when a peripheral with:
          * privacy enabled
          * privacy policy set to REJECT_NON_RESOLVED_ADDRESS
          * at least a bond
        is connected by an unknown privacy enabled device then the connection
        is rejected"""
    perform_bonding(peripheral, central)

    peripheral.gap.setPeripheralPrivacyConfiguration(
        False,
        "REJECT_NON_RESOLVED_ADDRESS"
    )

    peripheral.gap.setAdvertisingType("ADV_CONNECTABLE_UNDIRECTED")
    peripheral.gap.startAdvertising()

    # get the private address of the device
    scan_records = unknown_central.gap.startScan(
        SCAN_TIMEOUT,
        get_advertising_data(peripheral)
    ).result
    assert len(scan_records) > 0

    peripheral_address_type = scan_records[0]["peerAddrType"]
    peripheral_address = scan_records[0]["peerAddr"]

    # connect to the device
    connection_params = get_connection_args(
        peripheral_address_type,
        peripheral_address
    )

    peripheral_connection = peripheral.gap.waitForConnection.withRetcode(-1).setAsync()(1000)
    central_connection = unknown_central.gap.connect(*connection_params).result
    assert peripheral_connection.error == "timeout"

    # wait few seconds to ensure devices are disconnected
    sleep(4)

    expected_peripheral_state = {
        'advertising': True,
        'connected': False
    }

    expected_central_state = {
        'advertising': False,
        'connected': False
    }

    assert expected_peripheral_state == peripheral.gap.getState().result
    assert expected_central_state == central.gap.getState().result
