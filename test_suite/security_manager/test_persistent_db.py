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
from common.sm_utils import init_security_sessions, SecuritySession
from time import sleep

SERVICE_UUID = 0xFFFB
CHARACTERISTIC_UUID = 0xDEAD


@pytest.mark.ble41
@pytest.mark.smoketest
def test_recovering_keys(central, peripheral):
    """Change databases and recover old keys"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)

    central.ble.createFilesystem()
    peripheral.ble.createFilesystem()

    # we create a new database and store keys in this one
    peripheral.securityManager.setDatabaseFilepath("/fs/sm_db1")
    central.securityManager.setDatabaseFilepath("/fs/sm_db1")

    peripheral.securityManager.preserveBondingStateOnReset(True)
    central.securityManager.preserveBondingStateOnReset(True)

    central_ss.connect(peripheral_ss)

    peripheral_ss.wait_for_event()

    # pair to bond and save the ltk in the /fs/sm_db1 for later
    central_ss.start_pairing()
    # accept request
    peripheral_ss.expect_pairing_request()
    peripheral_ss.accept_pairing_request()
    peripheral_ss.expect_pairing_success()
    # initiator should see pairing succeed
    central_ss.expect_pairing_success()

    # we will reconnect
    central.gap.disconnect(central_ss.connection_handle, "USER_TERMINATION")
    sleep(1)
    # but change database so that it doesn't have the keys
    peripheral.securityManager.setDatabaseFilepath("/fs/sm_db2")
    central.securityManager.setDatabaseFilepath("/fs/sm_db2")
    # now reconnect
    central_ss.connect(peripheral_ss)

    peripheral_ss.wait_for_event()

    # encrypt which will trigger pairing and bonding and prove we have no keys
    central_ss.request_encryption(SecuritySession.ENCRYPTION_ENCRYPTED)
    # reject request
    peripheral_ss.expect_pairing_request()
    peripheral_ss.accept_pairing_request()
    # initiator should see pairing fail
    peripheral_ss.expect_pairing_success()

    # we will reconnect
    central.gap.disconnect(central_ss.connection_handle, "USER_TERMINATION")
    sleep(1)
    # but first change database to recover old keys
    peripheral.securityManager.setDatabaseFilepath("/fs/sm_db1")
    central.securityManager.setDatabaseFilepath("/fs/sm_db1")
    # now reconnect
    central_ss.connect(peripheral_ss)

    # wait for a pairing request that should never come
    peripheral.securityManager.setPairingRequestAuthorisation(True).success()
    pairing_result = peripheral.securityManager.waitForEvent.setAsync().withRetcode(-1)(
        peripheral_ss.connection_handle, 1000
    )

    # encrypt with existing keys we found in the db (this should NOT trigger pairing)
    central_ss.request_encryption(SecuritySession.ENCRYPTION_ENCRYPTED)
    central_ss.expect_encryption_changed(SecuritySession.ENCRYPTION_ENCRYPTED)

    # make sure no pairing happened
    assert not pairing_result.success()
    assert "Pairing timeout" == pairing_result.result
