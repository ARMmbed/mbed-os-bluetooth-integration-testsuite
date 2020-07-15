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

from common.gap_utils import gap_connect

NUM_REPEATED_CALLS = 10
DEFAULT_PAIRING_TIMEOUT = 30000
DEFAULT_TIMEOUT = 10000


class SecuritySession(object):
    ENCRYPTION_NOT_ENCRYPTED = "NOT_ENCRYPTED"
    ENCRYPTION_ENCRYPTED = "ENCRYPTED"
    ENCRYPTION_ENCRYPTED_WITH_MITM = "ENCRYPTED_WITH_MITM"

    def __init__(self, dut, mitm_protection, io_capabilities, bondable=False):
        self.dut = dut
        self.connection_handle = None
        self._current_command = None

        # Init params are:
        # * Bondable: boolean
        # * Perform MITM-resistant authentication
        # * I/O capabilities: SecurityIOCapabilities_t
        #       IO_CAPS_DISPLAY_ONLY
        #       IO_CAPS_DISPLAY_YESNO
        #       IO_CAPS_KEYBOARD_ONLY
        #       IO_CAPS_NONE
        #       IO_CAPS_KEYBOARD_DISPLAY
        # * Use static passkey: 6 digits or '*' to generate randomly
        # * Exchange signing keys: boolean
        self.dut.securityManager.init(bondable, mitm_protection, io_capabilities, "*", True, "*")

    def expect_pairing_request(self):
        self._current_command.success()
        assert "pairingRequest" == self._current_command.result["status"]

    def expect_passkey_display(self):
        self._current_command.success()
        assert "passkeyDisplay" == self._current_command.result["status"]
        return self._current_command.result["param"]

    def expect_confirmation_request(self):
        self._current_command.success()
        assert "confirmationRequest" == self._current_command.result["status"]

    def expect_passkey_request(self):
        self._current_command.success()
        assert "passkeyRequest" == self._current_command.result["status"]

    def expect_pairing_success(self):
        self._current_command.success()
        assert {'status': "pairingResult", 'param': 'SEC_STATUS_SUCCESS'} == self._current_command.result

    def expect_pairing_failure(self):
        self._current_command.success()
        assert "pairingResult" == self._current_command.result['status']
        assert self._current_command.result['param'] != 'SEC_STATUS_SUCCESS'

    def expect_encryption_changed(self, expected_encryption_level):
        self._current_command.success()
        assert "linkEncryptionResult" == self._current_command.result['status']
        assert expected_encryption_level == self._current_command.result['param']

    def wait_for_event(self):
        # Enable pairing requests
        self.dut.securityManager.setPairingRequestAuthorisation(True).success()

        # Wait for event
        self._current_command = self.dut.securityManager.waitForEvent.setAsync()(
            self.connection_handle, DEFAULT_TIMEOUT)

    def start_pairing(self):
        # Start pairing procedure
        self._current_command = self.dut.securityManager.requestPairingAndWait.setAsync()(
            self.connection_handle, DEFAULT_PAIRING_TIMEOUT, DEFAULT_TIMEOUT)

    def request_encryption(self, encryption_level):
        # Request encryption
        self._current_command = self.dut.securityManager.setLinkEncryptionAndWait.setAsync()(
            self.connection_handle, encryption_level, DEFAULT_TIMEOUT)

    def request_encryption_expect_failure(self, encryption_level, return_code):
        # Request encryption which should fail
        self._current_command = self.dut.securityManager.setLinkEncryptionAndWait.withRetcode(return_code)(
            self.connection_handle, encryption_level, DEFAULT_TIMEOUT)

    def accept_pairing_request(self):
        self._current_command = self.dut.securityManager.acceptPairingRequestAndWait(self.connection_handle,
                                                                                     DEFAULT_TIMEOUT)

    # Synchronous command
    def reject_pairing_request(self):
        self.dut.securityManager.rejectPairingRequest(self.connection_handle).success()

    def enter_confirmation(self, confirm_or_not, asynchronous=False):
        self._current_command = self.dut.securityManager.enterConfirmationAndWait.setAsync(asynchronous)(
            self.connection_handle, confirm_or_not, DEFAULT_TIMEOUT
        )

    def enter_passkey(self, passkey, return_code=0, asynchronous=False):
        self._current_command = self.dut.securityManager.enterPasskeyAndWait.setAsync(asynchronous).withRetcode(
            return_code)(self.connection_handle, passkey, DEFAULT_TIMEOUT)

    def connect(self, responder_session):
        central_handle, peripheral_handle = gap_connect(self.dut, responder_session.dut)
        self.connection_handle = central_handle
        responder_session.connection_handle = peripheral_handle
        return central_handle, peripheral_handle


def init_security_sessions(initiator, responder,
                           initiator_mitm=False, initiator_io_caps="IO_CAPS_NONE", initiator_bondable=True,
                           responder_mitm=False, responder_io_caps="IO_CAPS_NONE", responder_bondable=False):
    initiator_session = SecuritySession(
        initiator,
        initiator_mitm,
        initiator_io_caps,
        bondable=initiator_bondable
    )

    responder_session = SecuritySession(
        responder,
        responder_mitm,
        responder_io_caps,
        bondable=responder_bondable
    )

    return initiator_session, responder_session
