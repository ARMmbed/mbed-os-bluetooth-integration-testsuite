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

SERVICE_UUID = 0xFFFB
CHARACTERISTIC_SIGN_UUID = 0xDEAD


def change_value(server, client, client_connection_handle, signed_handle, value, success, sign):
    original_value = server.read(signed_handle).result
    retcode = 0 if success else -1

    if sign:
        client.signedWriteWithoutResponse.withRetcode(retcode)(client_connection_handle, signed_handle, value)
    else:
        client.write.withRetcode(retcode)(client_connection_handle, signed_handle, value)

    # wait 1 second as we don't get acknowledgment of the write operation
    sleep(1)
    new_value = server.read(signed_handle).result

    if success:
        assert original_value != new_value, "Write failed"
    else:
        assert original_value == new_value, "Write should've failed"


def create_signed_characteristic(server):
    """Create a characteristic and return its handle"""
    properties_sign = ["broadcast", "read", "writeWoResp", "write", "notify", "indicate", "authSignedWrite"]

    server.declareService(SERVICE_UUID)

    server.declareCharacteristic(CHARACTERISTIC_SIGN_UUID)
    server.setCharacteristicProperties(*properties_sign)
    server.setCharacteristicValue("00000001")
    server.setCharacteristicSecurity(
        "UNAUTHENTICATED", "UNAUTHENTICATED", "UNAUTHENTICATED"
    )
    server.setCharacteristicVariableLength(False)

    declared_service = server.commitService().result
    return declared_service["characteristics"][0]["value_handle"]


def test_write_signed_data_encrypted(central, peripheral):
    """Write data that requires signing over an encrypted link"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)

    server = peripheral.gattServer
    client = central.gattClient

    # build server and create a characteristic that requires signing
    signed_handle = create_signed_characteristic(server)

    # connect without pairing
    central_handle, peripheral_handle = central_ss.connect(peripheral_ss)

    # encrypt which will trigger pairing and bonding
    central_ss.request_encryption(SecuritySession.ENCRYPTION_ENCRYPTED)
    central_ss.expect_encryption_changed(SecuritySession.ENCRYPTION_ENCRYPTED)

    # test characteristics over encrypted connection
    change_value(server, client, central_handle, signed_handle, value="00000002", success=True, sign=True)


@pytest.mark.ble41
@pytest.mark.smoketest
def test_write_signed_data_unencrypted(central, peripheral):
    """Write data that requires signing over an unencrypted link"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)

    server = peripheral.gattServer
    client = central.gattClient

    # build server and create a characteristic that requires signing
    signed_handle = create_signed_characteristic(server)

    # connect without pairing
    central_handle, peripheral_handle = central_ss.connect(peripheral_ss)

    # test that writes fail without encryption or signing
    change_value(server, client, central_handle, signed_handle, value="00000002", success=False, sign=False)

    # encrypt which will trigger pairing and bonding
    central_ss.request_encryption(SecuritySession.ENCRYPTION_ENCRYPTED)
    central_ss.expect_encryption_changed(SecuritySession.ENCRYPTION_ENCRYPTED)

    # Wait as we got no visibility on whether the keys have been exchanged or not
    sleep(1)

    # re-connect and test characteristics over unencrypted connection
    central.gap.disconnect(central_ss.connection_handle, "USER_TERMINATION")
    central_ss.connect(peripheral_ss)

    # test writing the characteristic with signing
    change_value(server, client, central_handle, signed_handle, value="00000002", success=True, sign=True)

    # writes without signing should fail
    change_value(server, client, central_handle, signed_handle, value="00000003", success=False, sign=False)
