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
from common.gap_utils import gap_connect


def start_advertising(peripheral):
    peripheral.advDataBuilder.clear()
    # add flags data into advertising payload
    advertising_flags = ["LE_GENERAL_DISCOVERABLE", "BREDR_NOT_SUPPORTED"]
    for f in advertising_flags:
        peripheral.advDataBuilder.setFlags(f)
    peripheral.gap.applyAdvPayloadFromBuilder(LEGACY_ADVERTISING_HANDLE)

    # set the advertising type
    peripheral.advParams.setType("CONNECTABLE_UNDIRECTED")
    peripheral.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)

    # start advertising
    peripheral.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)


@pytest.mark.ble41
def test_cancel_connect(central, peripheral):
    """Connection attempt can be cancelled"""
    peripheral_address = peripheral.gap.getAddress().result

    # try to connect when no one is advertising
    central.gap.startConnecting(peripheral_address["address_type"], peripheral_address["address"])

    sleep(0.5)
    central.gap.cancelConnect(peripheral_address["address_type"], peripheral_address["address"])
    sleep(0.5)

    # after we already stopped trying to connect start advertising but expect to fail to connect

    start_advertising(peripheral)
    peripheral.gap.waitForConnection.setAsync().withRetcode(-1)(5000).result


@pytest.mark.ble41
def test_cancel_connect_too_late(central, peripheral):
    """Cancelling a connection already established should have no effect"""
    peripheral_address = peripheral.gap.getAddress().result

    central_handle, peripheral_handle = gap_connect(central, peripheral)

    # we cancel after the connection completes and it should have no effect
    central.gap.cancelConnect(peripheral_address["address_type"], peripheral_address["address"]).result

    sleep(2)

    # we make sure we're still connected by disconnecting now and checking the remote reason
    disconnection_cmd = peripheral.gap.waitForDisconnection.setAsync()(10000)
    central.gap.disconnect(central_handle, "USER_TERMINATION")

    disconnection_cmd.result
    assert disconnection_cmd.result['reason'] == 'REMOTE_USER_TERMINATED_CONNECTION'




