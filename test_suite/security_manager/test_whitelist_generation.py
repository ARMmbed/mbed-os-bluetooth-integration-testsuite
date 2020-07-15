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
from time import sleep


@pytest.mark.ble41
def test_generate_whitelist_from_bonded_devices(central, peripheral):
    """Generate whitelist based on bonded devices"""
    central_ss, peripheral_ss = init_security_sessions(central, peripheral)

    # generation should create an empty whitelist
    result = central.securityManager.generateWhitelistFromBondTable.withRetcode(0)().result
    assert len(result) == 0

    # remember responder address which will later end up in the whitelist
    responder_address = peripheral.gap.getAddress.withRetcode(0)().result

    # connect and pair to create a bond
    central_ss.connect(peripheral_ss)
    central_ss.start_pairing()
    central_ss.expect_pairing_success()
    sleep(1)

    central.gap.disconnect(central_ss.connection_handle, "USER_TERMINATION")

    # generation should crate whitelist with one entry
    result = central.securityManager.generateWhitelistFromBondTable.withRetcode(0)().result
    assert len(result) == 2

    # check entry against the responder address
    assert responder_address in result
