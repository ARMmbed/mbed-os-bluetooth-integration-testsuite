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
from common.sm_utils import init_security_sessions


# These are "meta" test cases used in the test below
class JustWorks:
    def test(self, central_ss, peripheral_ss):
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


class PasskeyEntry:
    def __init__(self, responder_displays_n_inputs, initiator_displays_n_inputs):
        self._responder_displays_n_inputs = responder_displays_n_inputs
        self._initiator_displays_n_inputs = initiator_displays_n_inputs

    def test(self, central_ss, peripheral_ss):
        # Always start peripheral_ss first
        peripheral_ss.wait_for_event()
        central_ss.start_pairing()

        # We should get a request
        peripheral_ss.expect_pairing_request()

        # Accept request
        peripheral_ss.accept_pairing_request()

        # Get passkey
        passkey = None
        if self._responder_displays_n_inputs:
            passkey = peripheral_ss.expect_passkey_display()
            peripheral_ss.wait_for_event()
        elif self._initiator_displays_n_inputs:
            passkey = central_ss.expect_passkey_display()
            central_ss.wait_for_event()

        # If both devices only have a keyboard, create a passkey here
        if passkey is None:
            passkey = 943568

        # Input passkey
        if not self._responder_displays_n_inputs:
            peripheral_ss.expect_passkey_request()
            peripheral_ss.enter_passkey(passkey, asynchronous=True)
        if not self._initiator_displays_n_inputs:
            central_ss.expect_passkey_request()
            central_ss.enter_passkey(passkey)

        # Both should succeed here
        peripheral_ss.expect_pairing_success()
        central_ss.expect_pairing_success()


class NumericComparison:
    def test(self, central_ss, peripheral_ss):
        # Always start peripheral_ss first
        peripheral_ss.wait_for_event()
        central_ss.start_pairing()

        # We should get a request
        peripheral_ss.expect_pairing_request()

        # Accept request
        peripheral_ss.accept_pairing_request()

        # Get passkeys (should be identical)
        passkey1 = peripheral_ss.expect_passkey_display()
        passkey2 = central_ss.expect_passkey_display()

        assert passkey1 == passkey2

        # Expect confirmation requests
        # FIXME: too slow to set that expectation :(
        # peripheral_ss.expect_confirmation_request()
        # central_ss.expect_confirmation_request()

        # Accept on both ends
        peripheral_ss.enter_confirmation(True, asynchronous=True)
        central_ss.enter_confirmation(True)

        # Both should succeed here
        peripheral_ss.expect_pairing_success()
        central_ss.expect_pairing_success()


# Test vectors: (peripheral I/O capabilities, central I/O capabilities,
# Secure Connections (= not Legacy Pairing), Key Generation Method)
KEYGEN_METHODS = [
    # Row 1
    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_DISPLAY_ONLY", False, JustWorks()),
    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_DISPLAY_ONLY", True, JustWorks()),

    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_DISPLAY_YESNO", False, JustWorks()),
    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_DISPLAY_YESNO", True, JustWorks()),

    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_KEYBOARD_ONLY", False, PasskeyEntry(True, False)),
    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_KEYBOARD_ONLY", True, PasskeyEntry(True, False)),

    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_NONE", False, JustWorks()),
    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_NONE", True, JustWorks()),

    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_KEYBOARD_DISPLAY", False, PasskeyEntry(True, False)),
    ("IO_CAPS_DISPLAY_ONLY", "IO_CAPS_KEYBOARD_DISPLAY", True, PasskeyEntry(True, False)),

    # Row 2
    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_DISPLAY_ONLY", False, JustWorks()),
    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_DISPLAY_ONLY", True, JustWorks()),

    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_DISPLAY_YESNO", False, JustWorks()),
    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_DISPLAY_YESNO", True, NumericComparison()),

    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_KEYBOARD_ONLY", False, PasskeyEntry(True, False)),
    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_KEYBOARD_ONLY", True, PasskeyEntry(True, False)),

    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_NONE", False, JustWorks()),
    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_NONE", True, JustWorks()),

    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_KEYBOARD_DISPLAY", False, PasskeyEntry(True, False)),
    ("IO_CAPS_DISPLAY_YESNO", "IO_CAPS_KEYBOARD_DISPLAY", True, NumericComparison()),

    # Row 3
    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_DISPLAY_ONLY", False, PasskeyEntry(False, True)),
    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_DISPLAY_ONLY", True, PasskeyEntry(False, True)),

    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_DISPLAY_YESNO", False, PasskeyEntry(False, True)),
    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_DISPLAY_YESNO", True, PasskeyEntry(False, True)),

    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_KEYBOARD_ONLY", False, PasskeyEntry(False, False)),
    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_KEYBOARD_ONLY", True, PasskeyEntry(False, False)),

    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_NONE", False, JustWorks()),
    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_NONE", True, JustWorks()),

    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_KEYBOARD_DISPLAY", False, PasskeyEntry(False, True)),
    ("IO_CAPS_KEYBOARD_ONLY", "IO_CAPS_KEYBOARD_DISPLAY", True, PasskeyEntry(False, True)),

    # Row 4
    ("IO_CAPS_NONE", "IO_CAPS_DISPLAY_ONLY", False, JustWorks()),
    ("IO_CAPS_NONE", "IO_CAPS_DISPLAY_ONLY", True, JustWorks()),

    ("IO_CAPS_NONE", "IO_CAPS_DISPLAY_YESNO", False, JustWorks()),
    ("IO_CAPS_NONE", "IO_CAPS_DISPLAY_YESNO", True, JustWorks()),

    ("IO_CAPS_NONE", "IO_CAPS_KEYBOARD_ONLY", False, JustWorks()),
    ("IO_CAPS_NONE", "IO_CAPS_KEYBOARD_ONLY", True, JustWorks()),

    ("IO_CAPS_NONE", "IO_CAPS_NONE", False, JustWorks()),
    ("IO_CAPS_NONE", "IO_CAPS_NONE", True, JustWorks()),

    ("IO_CAPS_NONE", "IO_CAPS_KEYBOARD_DISPLAY", False, JustWorks()),
    ("IO_CAPS_NONE", "IO_CAPS_KEYBOARD_DISPLAY", True, JustWorks()),

    # Row 5
    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_DISPLAY_ONLY", False, PasskeyEntry(False, True)),
    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_DISPLAY_ONLY", True, PasskeyEntry(False, True)),

    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_DISPLAY_YESNO", False, PasskeyEntry(False, True)),
    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_DISPLAY_YESNO", True, NumericComparison()),

    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_KEYBOARD_ONLY", False, PasskeyEntry(True, False)),
    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_KEYBOARD_ONLY", True, PasskeyEntry(True, False)),

    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_NONE", False, JustWorks()),
    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_NONE", True, JustWorks()),

    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_KEYBOARD_DISPLAY", False, PasskeyEntry(False, True)),
    ("IO_CAPS_KEYBOARD_DISPLAY", "IO_CAPS_KEYBOARD_DISPLAY", True, NumericComparison())
]


@pytest.mark.ble42
@pytest.mark.parametrize('responder_io_caps,initiator_io_caps,secure_connections,keygen', KEYGEN_METHODS)
def test_pairing_combinations(central, peripheral, responder_io_caps, initiator_io_caps, secure_connections, keygen):
    """Pairing procedure should follow table 2.8 from BLE Core spec Vol 3, Part H, 2.3.5.1"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral,
                                                       initiator_mitm=True, initiator_io_caps=initiator_io_caps,
                                                       responder_mitm=True, responder_io_caps=responder_io_caps)

    # Get Secure connections support
    sc_initiator = central.securityManager.getSecureConnectionsSupport().result
    sc_responder = peripheral.securityManager.getSecureConnectionsSupport().result

    if secure_connections:
        # Skip if unsupported
        if not sc_initiator or not sc_responder:
            return
    else:
        # Skip if unsupported
        if sc_initiator and sc_responder:
            return

        # Allow legacy pairing
        central.securityManager.allowLegacyPairing(True).success()
        peripheral.securityManager.allowLegacyPairing(True).success()

    central_ss.connect(peripheral_ss)

    # Perform pairing
    keygen.test(central_ss, peripheral_ss)


@pytest.mark.ble41
def test_pairing_with_defaults(central, peripheral):
    """Pairing should succeed with default parameters on both ends"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)
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


@pytest.mark.ble41
def test_pairing_fail_if_rejected(central, peripheral):
    """Pairing should fail with default parameters on both ends if request is rejected"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)
    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    peripheral_ss.wait_for_event()
    central_ss.start_pairing()

    # We should get a request
    peripheral_ss.expect_pairing_request()

    # Reject request (method is still successful)
    peripheral_ss.reject_pairing_request()

    # central_ss should see pairing fail
    central_ss.expect_pairing_failure()


@pytest.mark.ble41
def test_pairing_fail_if_passkey_wrong(central, peripheral):
    """MITM with IO_CAPS_KEYBOARD_ONLY on both ends and two different passkeys are input should fail pairing"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral,
                                                       initiator_mitm=True, initiator_io_caps="IO_CAPS_KEYBOARD_ONLY",
                                                       responder_mitm=True, responder_io_caps="IO_CAPS_KEYBOARD_ONLY")
    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    peripheral_ss.wait_for_event()
    central_ss.start_pairing()

    # We should get a request
    peripheral_ss.expect_pairing_request()

    # Accept request
    peripheral_ss.accept_pairing_request()

    # Wait for passkey requests
    peripheral_ss.expect_passkey_request()
    central_ss.expect_passkey_request()

    # Input passkeys
    peripheral_ss.enter_passkey(123456, asynchronous=True)
    central_ss.enter_passkey(654321)

    # Both should fail here
    peripheral_ss.expect_pairing_failure()
    central_ss.expect_pairing_failure()


@pytest.mark.ble42
def test_secure_connections_pairing_fails_if_comparison_fails(central, peripheral):
    """In Secure Connections pairing, when doing numeric comparison and rejecting comparison, pairing should fail"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral,
                                                       initiator_mitm=True, initiator_io_caps="IO_CAPS_DISPLAY_YESNO",
                                                       responder_mitm=True, responder_io_caps="IO_CAPS_DISPLAY_YESNO")
    # Get Secure connections support
    sc_initiator = central.securityManager.getSecureConnectionsSupport().result
    sc_responder = peripheral.securityManager.getSecureConnectionsSupport().result

    # Skip if unsupported
    if not sc_initiator or not sc_responder:
        return

    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    peripheral_ss.wait_for_event()
    central_ss.start_pairing()

    # We should get a request
    peripheral_ss.expect_pairing_request()

    # Accept request
    peripheral_ss.accept_pairing_request()

    # Get passkeys (should be identical)
    passkey1 = peripheral_ss.expect_passkey_display()
    passkey2 = central_ss.expect_passkey_display()

    assert passkey1 == passkey2

    # Expect confirmation requests
    # FIXME: too slow to set that expectation :(
    # peripheral_ss.expect_confirmation_request()
    # central_ss.expect_confirmation_request()

    # Reject on one end
    peripheral_ss.enter_confirmation(True, asynchronous=True)
    central_ss.enter_confirmation(False)

    # Both should fail here
    peripheral_ss.expect_pairing_failure()
    central_ss.expect_pairing_failure()


@pytest.mark.ble41
def test_responder_can_reject_pairing_request(central, peripheral):
    """A slave should be able to reject a pairing request"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)
    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    peripheral_ss.wait_for_event()
    central_ss.start_pairing()

    # We should get a request
    peripheral_ss.expect_pairing_request()

    # Reject pairing request
    peripheral_ss.reject_pairing_request()

    # central_ss should see pairing fail
    central_ss.expect_pairing_failure()
