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
from common.sm_utils import SecuritySession, init_security_sessions


@pytest.mark.ble41
def test_unsecured_link(central, peripheral):
    """Asking for an unsecured link should succeed immediately"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)
    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    central_ss.request_encryption(SecuritySession.ENCRYPTION_NOT_ENCRYPTED)

    # It should succeed immediately
    central_ss.expect_encryption_changed(SecuritySession.ENCRYPTION_NOT_ENCRYPTED)


@pytest.mark.ble41
def test_encryption_triggers_pairing(central, peripheral):
    """Asking for a link should trigger pairing and then succeed"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)
    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    peripheral_ss.wait_for_event()
    central_ss.request_encryption(SecuritySession.ENCRYPTION_ENCRYPTED)
    sleep(1)

    # Accept request, pairing should complete successfully
    peripheral_ss.accept_pairing_request()
    peripheral_ss.expect_pairing_success()

    # It should succeed
    central_ss.expect_encryption_changed(SecuritySession.ENCRYPTION_ENCRYPTED)


@pytest.mark.ble41
def test_secure_link_succeeds_immediately_if_paired(central, peripheral):
    """If pairing was successful then asking for a secure link should succeed immediately"""
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

    # It should succeed
    central_ss.request_encryption(SecuritySession.ENCRYPTION_ENCRYPTED)
    central_ss.expect_encryption_changed(SecuritySession.ENCRYPTION_ENCRYPTED)


@pytest.mark.ble41
def test_non_mitm_capable_device_create_non_mitm_secured_link(central, peripheral):
    """If a MITM-resistant level is requested and devices don't have the capabilities, then an 'encrypted' level
    should be achieved instead """
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)
    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    peripheral_ss.wait_for_event()
    central_ss.request_encryption(SecuritySession.ENCRYPTION_ENCRYPTED_WITH_MITM)

    # Accept request, pairing should complete successfully
    peripheral_ss.accept_pairing_request()

    # It should succeed
    central_ss.expect_encryption_changed(SecuritySession.ENCRYPTION_ENCRYPTED)


@pytest.mark.ble41
def test_mitm_capable_devices_create_mitm_secured_link(central, peripheral):
    """If a MITM-resistant level is requested and devices have the capabilities, then it's achieved"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral,
                                                       initiator_mitm=True, initiator_io_caps="IO_CAPS_KEYBOARD_ONLY",
                                                       responder_mitm=True, responder_io_caps="IO_CAPS_DISPLAY_ONLY")
    central_ss.connect(peripheral_ss)

    # Always start peripheral_ss first
    peripheral_ss.wait_for_event()
    central_ss.start_pairing()

    # Accept request & perform MITM protection
    peripheral_ss.expect_pairing_request()
    peripheral_ss.accept_pairing_request()

    passkey = peripheral_ss.expect_passkey_display()
    peripheral_ss.wait_for_event()
    central_ss.enter_passkey(passkey)

    peripheral_ss.expect_pairing_success()

    # Now it should succeed
    central_ss.request_encryption(SecuritySession.ENCRYPTION_ENCRYPTED_WITH_MITM)
    central_ss.expect_encryption_changed(SecuritySession.ENCRYPTION_ENCRYPTED_WITH_MITM)


@pytest.mark.ble41
def test_downgrade_of_security_fails(central, peripheral):
    """Asking for an encryption level downgrade should fail"""
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

    # It should fail
    central_ss.request_encryption_expect_failure(SecuritySession.ENCRYPTION_NOT_ENCRYPTED, -1)
