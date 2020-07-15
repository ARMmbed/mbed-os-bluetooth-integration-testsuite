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
from uuid import uuid4
from common.gap_utils import gap_connect, discover_user_services

properties_name = ["broadcast", "read", "writeWoResp", "write", "notify", "indicate", "authSignedWrite"]
properties_map = [(2 ** x, properties_name[x]) for x in range(len(properties_name))]


def get_characteristic_properties(v):
    properties = []
    for p in reversed(properties_map):
        if p[0] == 0:
            if v == 1:
                properties.append(p[1])
        elif (v / p[0]) == 1:
            properties.append(p[1])
            v = v - p[0]
    return properties


char_properties = [get_characteristic_properties(i) for i in range(0, 2 ** len(properties_name))]


def characteristics_properties(central, peripheral, characteristic_uuid, set_property):
    # test all combinations except this illegal set
    if set_property == ["authSignedWrite"]:
        return

    peripheral.gattServer.declareService(0xFFFB)
    peripheral.gattServer.declareCharacteristic(characteristic_uuid)
    peripheral.gattServer.setCharacteristicProperties(*set_property)
    # if the characteristic is readable, the characteristic value can't be 0
    # TODO: investigate if it is a bug or not
    peripheral.gattServer.setCharacteristicValue("AA")
    declared_service = peripheral.gattServer.commitService().result

    # assert that properties set match expectations
    assert set_property.sort() == declared_service["characteristics"][0]["properties"].sort()

    connection_handle_central, connection_handle_peripheral = gap_connect(central, peripheral)
    discovered_services = discover_user_services(central, connection_handle_central)
    assert set_property.sort() == discovered_services[0]["characteristics"][0]["properties"].sort()

    central.gap.disconnect(connection_handle_central, "USER_TERMINATION")


@pytest.mark.ble41
@pytest.mark.parametrize('char_property', char_properties)
def test_16bit_uuid(central, peripheral, char_property):
    """Characteristics properties set at instantiation should be visible
    during discovery for Characteristics with 16bits UUIDs"""
    characteristics_properties(central, peripheral, 0xDEAD, char_property)


@pytest.mark.ble41
@pytest.mark.parametrize('char_property', char_properties)
def test_128bits_uuid(central, peripheral, char_property):
    """Characteristics properties set at instantiation should be visible
    during discovery for Characteristics with 128bits UUIDs"""
    characteristics_properties(central, peripheral, str(uuid4()), char_property)
