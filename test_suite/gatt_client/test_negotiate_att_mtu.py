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

from common.ble_device import BleDevice
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from common.fixtures import BoardAllocator


@pytest.fixture(scope="function")
def server(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate("server")
    assert device
    device.ble.init()

    device.advDataBuilder.clear()
    # add flags data into advertising payload
    advertising_flags = ["LE_GENERAL_DISCOVERABLE", "BREDR_NOT_SUPPORTED"]
    for f in advertising_flags:
        device.advDataBuilder.setFlags(f)
    device.gap.applyAdvPayloadFromBuilder(LEGACY_ADVERTISING_HANDLE)

    # set the advertising type
    device.advParams.setType("CONNECTABLE_UNDIRECTED")
    device.gap.setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE)

    yield device

    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def client(board_allocator: BoardAllocator) -> BleDevice:
    device = board_allocator.allocate('client')
    assert device
    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.mark.ble42
def test_att_mtu_change(server, client):
    # get the server mac address
    server_address = server.gap.getAddress().result

    # start advertising
    server.gap.startAdvertising(LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED)

    # connect the client to the server
    connection_server = server.gap.waitForConnection.setAsync()(2000)
    connection = client.gap.connect(
        server_address["address_type"], server_address["address"]
    ).result
    connection_server.result  # Wait for the completion of the operation on the server

    # get the handle of the new connection
    connection_handle = connection["connection_handle"]

    mtu_res = client.gattClient.negotiateAttMtu(connection_handle, 3000)
    assert mtu_res.status == 0
    assert mtu_res.result["handle"] == connection_handle
    # no specific attMtuSize is expected, other than different than default
    assert mtu_res.result["attMtuSize"] > 23
