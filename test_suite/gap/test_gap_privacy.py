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
from time import sleep
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.sm_utils import init_security_sessions

RANDOM_NON_RESOLVABLE_MSB_RANGE = ['0', '1', '2', '3']
RANDOM_RESOLVABLE_MSB_RANGE = ['4', '5', '6', '7']


def address_is_random_resolvable(address):
    return address[0] in RANDOM_RESOLVABLE_MSB_RANGE


def address_is_random_non_resolvable(address):
    return address[0] in RANDOM_NON_RESOLVABLE_MSB_RANGE


def perform_bonding_with_privacy_and_disconnect(peripheral, central):
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)

    peripheral.gap.enablePrivacy(True)
    central.gap.enablePrivacy(True)

    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    peripheral_ss.wait_for_event()
    central_ss.start_pairing()

    # We should get a request
    peripheral_ss.expect_pairing_request()

    # Accept request, pairing should complete successfully
    peripheral_ss.accept_pairing_request()
    peripheral_ss.expect_pairing_success()

    # central_ss should see pairing complete successfully too
    central_ss.expect_pairing_success()

    central.gap.disconnect(peripheral_ss.connection_handle, "USER_TERMINATION")

    # FIXME: needs event from the peripheral and central that they have been disconnected
    sleep(0.5)


def start_advertising_of_type(peripheral, advertising_type):
    peripheral.advParams.setType(advertising_type)
    peripheral.advParams.setPrimaryInterval(100, 100)
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)

    peripheral.advDataBuilder.clear()
    peripheral.advDataBuilder.setFlags("LE_GENERAL_DISCOVERABLE")
    advertising_data = peripheral.advDataBuilder.getAdvertisingData().result
    peripheral.gap.applyAdvPayloadFromBuilder(LEGACY_ADVERTISING_HANDLE)

    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    return advertising_data


def start_scanning_for_data(central, advertising_data):
    central.scanParams.set1mPhyConfiguration(100, 100, True)
    central.gap.setScanParameters()
    scan_results = central.gap.scanForData(advertising_data, 300).result
    return scan_results


def connect_to_address(central, peripheral, peer_address_type, peer_address):
    peripheral_connection_cmd = peripheral.gap.waitForConnection.setAsync()(10000)
    central_connection = central.gap.connect(peer_address_type, peer_address).result
    #sleep(0.1)
    peripheral_connection = peripheral_connection_cmd.result
    return central_connection


def init_privacy(device):
    device.securityManager.init(True, False, "IO_CAPS_NONE", "*", True, "*")
    device.gap.enablePrivacy(True)


@pytest.mark.ble42
@pytest.mark.parametrize("advertising_type, unresolvable", [
    ("CONNECTABLE_UNDIRECTED", True),
    ("SCANNABLE_UNDIRECTED", True),
    ("NON_CONNECTABLE_UNDIRECTED", True),
    ("CONNECTABLE_UNDIRECTED", False),
    ("SCANNABLE_UNDIRECTED", False),
    ("NON_CONNECTABLE_UNDIRECTED", False),
])
def test_privacy_random_resolvable_address(peripheral, central, advertising_type, unresolvable):
    """validate that when privacy is enabled and a peripheral use a random
    resolvable address then a random resolvable address is used for all
    advertising modes."""
    init_privacy(peripheral)
    init_privacy(central)

    central.gap.setPeripheralPrivacyConfiguration(unresolvable, "DO_NOT_RESOLVE")

    advertising_data = start_advertising_of_type(peripheral, advertising_type)

    central.gap.setCentralPrivacyConfiguration(False, "DO_NOT_RESOLVE")

    scan_results = start_scanning_for_data(central, advertising_data)

    for scan in scan_results:
        assert scan["peer_address_type"] == "RANDOM"
        if unresolvable:
            assert address_is_random_non_resolvable(scan["peer_address"])
        else:
            assert address_is_random_resolvable(scan["peer_address"])


@pytest.mark.ble42
@pytest.mark.parametrize("advertising_type", [
     "CONNECTABLE_UNDIRECTED",
     # "CONNECTABLE_DIRECTED",  # Directed advertising not supported yet
     "SCANNABLE_UNDIRECTED",
     "NON_CONNECTABLE_UNDIRECTED"
])
def test_address_resolution_when_advertised(peripheral, central, advertising_type):
    """validate that when a peripheral with privacy enabled using private
    resolvable address advertise then a known peer that resolves addresses
    resolve the address advertised"""
    perform_bonding_with_privacy_and_disconnect(peripheral, central)

    advertising_data = start_advertising_of_type(peripheral, advertising_type)

    central.gap.setCentralPrivacyConfiguration(False, "RESOLVE_AND_FORWARD")

    scan_results = start_scanning_for_data(central, advertising_data)

    for scan in scan_results:
        assert scan["peer_address_type"] == "RANDOM_STATIC_IDENTITY"
        assert address_is_random_resolvable(scan["peer_address"]) is False
        assert address_is_random_non_resolvable(scan["peer_address"]) is False


@pytest.mark.ble42
def test_connection_with_resolved_address(peripheral, central):
    """validate that when a peripheral with privacy enabled using private
    resolvable address advertise then a known peer that resolves addresses
    can connect with the resolved address"""
    perform_bonding_with_privacy_and_disconnect(peripheral, central)

    advertising_data = start_advertising_of_type(peripheral, "CONNECTABLE_UNDIRECTED")

    central.gap.setCentralPrivacyConfiguration(False, "RESOLVE_AND_FORWARD")

    scan_results = start_scanning_for_data(central, advertising_data)
    scan = scan_results[0]

    assert scan["peer_address_type"] == "RANDOM_STATIC_IDENTITY" or scan["peer_address_type"] == "PUBLIC_IDENTITY"

    central_connection = connect_to_address(central, peripheral, scan["peer_address_type"], scan["peer_address"])

    assert central_connection["peer_address_type"] == scan["peer_address_type"]
    assert central_connection["peer_address"] == scan["peer_address"]


@pytest.mark.ble42
@pytest.mark.parametrize("advertising_type", [
     "CONNECTABLE_UNDIRECTED",
     # "CONNECTABLE_DIRECTED",  # Directed advertising not supported yet
     "SCANNABLE_UNDIRECTED",
     "NON_CONNECTABLE_UNDIRECTED"
])
def test_central_not_resolve_policy(peripheral, central, advertising_type):
    """validate that when a peripheral with privacy enabled using private
    resolvable address advertise then a known peer that do not resolves
    addresses do not resolve the address advertised"""
    perform_bonding_with_privacy_and_disconnect(peripheral, central)

    advertising_data = start_advertising_of_type(peripheral, advertising_type)

    central.gap.setCentralPrivacyConfiguration(False, "DO_NOT_RESOLVE")

    scan_results = start_scanning_for_data(central, advertising_data)

    for scan in scan_results:
        assert scan["peer_address_type"] == "RANDOM"
        assert address_is_random_resolvable(scan["peer_address"])


@pytest.mark.ble42
def test_connection_to_non_resolved_address(peripheral, central):
    """validate that when a peripheral with privacy enabled using private
    resolvable address advertise then a known peer that do not resolves
    addresses can connect to the non resolved address"""
    perform_bonding_with_privacy_and_disconnect(peripheral, central)

    advertising_data = start_advertising_of_type(peripheral, "CONNECTABLE_UNDIRECTED")

    central.gap.setCentralPrivacyConfiguration(False, "DO_NOT_RESOLVE")

    scan_results = start_scanning_for_data(central, advertising_data)
    scan = scan_results[0]

    assert scan["peer_address_type"] == "RANDOM"
    assert address_is_random_resolvable(scan["peer_address"])

    central_connection = connect_to_address(central, peripheral, scan["peer_address_type"], scan["peer_address"])

    assert central_connection["peer_address_type"] == "RANDOM"
    assert central_connection["peer_address"] == scan["peer_address"]


@pytest.mark.ble42
@pytest.mark.parametrize("advertising_type", [
     "CONNECTABLE_UNDIRECTED",
     # "CONNECTABLE_DIRECTED",  # Directed advertising not supported yet
     "SCANNABLE_UNDIRECTED",
     "NON_CONNECTABLE_UNDIRECTED"
])
def test_resolve_and_filter_unknown_address(peripheral, central, advertising_type):
    """Ensure that if the central privacy policy is set to resolve and filter
    and the device has no bond then resolvable private addresses are not filtered"""
    init_privacy(peripheral)
    init_privacy(central)

    advertising_data = start_advertising_of_type(peripheral, advertising_type)

    central.gap.setCentralPrivacyConfiguration(False, "RESOLVE_AND_FILTER")

    scan_results = start_scanning_for_data(central, advertising_data)

    for scan in scan_results:
        assert scan["peer_address_type"] == "RANDOM"
        assert address_is_random_resolvable(scan["peer_address"])

@pytest.mark.ble42
@pytest.mark.parametrize("advertising_type", [
    "CONNECTABLE_UNDIRECTED",
    # "CONNECTABLE_DIRECTED",  # Directed advertising not supported yet
    "SCANNABLE_UNDIRECTED",
    "NON_CONNECTABLE_UNDIRECTED"
])
def test_resolve_and_filter_bonded_peer(peripheral, central, advertising_type):
    """Ensure that if the central privacy policy is set to resolve and filter
    bonded devices show up in scan results as resolved addresses"""
    perform_bonding_with_privacy_and_disconnect(peripheral, central)

    advertising_data = start_advertising_of_type(peripheral, advertising_type)

    central.gap.setCentralPrivacyConfiguration(False, "RESOLVE_AND_FILTER")

    scan_results = start_scanning_for_data(central, advertising_data)

    for scan in scan_results:
        assert scan["peer_address_type"] == "RANDOM_STATIC_IDENTITY"
        assert address_is_random_resolvable(scan["peer_address"]) is False
        assert address_is_random_non_resolvable(scan["peer_address"]) is False


@pytest.mark.ble42
@pytest.mark.parametrize("advertising_type", [
     "CONNECTABLE_UNDIRECTED",
     # "CONNECTABLE_DIRECTED",  # Directed advertising not supported yet
     "SCANNABLE_UNDIRECTED",
     "NON_CONNECTABLE_UNDIRECTED"
])
def test_resolve_and_filter_unknown_address_with_bond_present(peripheral, central, peripheral2, advertising_type):
    """Ensure that if the central privacy policy is set to resolve and filter
    and the device has a bond then resolved private addresses are filtered"""
    init_privacy(peripheral2)
    perform_bonding_with_privacy_and_disconnect(peripheral, central)

    advertising_data = start_advertising_of_type(peripheral2, advertising_type)

    central.gap.setCentralPrivacyConfiguration(False, "RESOLVE_AND_FILTER")

    scan_results = start_scanning_for_data(central, advertising_data)

    assert len(scan_results) == 0


@pytest.mark.ble42
@pytest.mark.parametrize("advertising_type", [
     "CONNECTABLE_UNDIRECTED",
     # "CONNECTABLE_DIRECTED",  # Directed advertising not supported yet
     "SCANNABLE_UNDIRECTED",
     "NON_CONNECTABLE_UNDIRECTED"
])
def test_non_resolved_address_with_resolve_and_forward(peripheral, central, peripheral2, advertising_type):
    """Ensure that if the central privacy policy is set to resolve and forward
    and the device has one bond then non resolved private addresses are forwarded"""
    init_privacy(peripheral2)
    perform_bonding_with_privacy_and_disconnect(peripheral, central)

    advertising_data = start_advertising_of_type(peripheral2, advertising_type)

    central.gap.setCentralPrivacyConfiguration(False, "RESOLVE_AND_FORWARD")

    scan_results = start_scanning_for_data(central, advertising_data)

    for scan in scan_results:
        assert scan["peer_address_type"] == "RANDOM"
        assert address_is_random_resolvable(scan["peer_address"])


@pytest.mark.ble42
def test_reject_non_resolved_address_with_no_bond_accepts_connection(peripheral, central):
    """validate that when a peripheral with privacy enabled, the privacy policy
    set to REJECT_NON_RESOLVED_ADDRESS and no bond is connected by an unknown
    privacy enabled then connection is accepted"""
    init_privacy(peripheral)
    init_privacy(central)

    peripheral.gap.setPeripheralPrivacyConfiguration(False, "REJECT_NON_RESOLVED_ADDRESS")

    advertising_data = start_advertising_of_type(peripheral, "CONNECTABLE_UNDIRECTED")

    central.gap.setCentralPrivacyConfiguration(False, "DO_NOT_RESOLVE")

    scan_results = start_scanning_for_data(central, advertising_data)
    scan = scan_results[0]

    connect_to_address(central, peripheral, scan["peer_address_type"], scan["peer_address"])

@pytest.mark.ble42
def test_reject_non_resolved_address_with_one_bond(peripheral, central, central2):
    """validate that when a peripheral with:
          * privacy enabled
          * privacy policy set to REJECT_NON_RESOLVED_ADDRESS
          * at least a bond
        is connected by an unknown privacy enabled device then the connection
        is rejected"""
    init_privacy(central2)
    perform_bonding_with_privacy_and_disconnect(peripheral, central)

    peripheral.gap.setPeripheralPrivacyConfiguration(False, "REJECT_NON_RESOLVED_ADDRESS")

    advertising_data = start_advertising_of_type(peripheral, "CONNECTABLE_UNDIRECTED")

    central2.gap.setCentralPrivacyConfiguration(False, "DO_NOT_RESOLVE")

    scan_results = start_scanning_for_data(central2, advertising_data)
    scan = scan_results[0]

    central.gap.connect.withRetcode(-1)(scan["peer_address_type"], scan["peer_address"]).result


@pytest.mark.ble42
def test_perform_pairing_procedure_policy(central, peripheral):
    """validate that when a peripheral with privacy enabled with the policy
    PERFORM_PAIRING_PROCEDURE is connected by an unknown privacy enabled
    central then the pairing procedure is performed"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)

    peripheral.gap.enablePrivacy(True)
    central.gap.enablePrivacy(True)

    peripheral.gap.setPeripheralPrivacyConfiguration(False, "PERFORM_PAIRING_PROCEDURE")
    central.gap.setCentralPrivacyConfiguration(False, "DO_NOT_RESOLVE")

    central_ss.connect(peripheral_ss)

    # We should get a request automatically because of the policy
    peripheral_ss.wait_for_event()
    peripheral_ss.expect_pairing_request()

    # Accept request, pairing should complete successfully
    peripheral_ss.accept_pairing_request()
    peripheral_ss.expect_pairing_success()

    # central_ss should see pairing complete successfully too
    central_ss.expect_pairing_success()
