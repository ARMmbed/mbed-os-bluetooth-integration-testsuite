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
from common.gap_utils import gap_connect

GENERIC_ATTRIBUTE_SERVICE_UUID = 0x1801
SERVICE_CHANGED_CHARACTERISTIC_UUID = 0x2A05


@pytest.mark.ble41
def test_gap_service(central, peripheral):
    """A GATT peripheral should provide a well formed Generic Attribute Profile service"""
    connection_handle, _ = gap_connect(central, peripheral)

    services = central.gattClient.discoverAllServicesAndCharacteristics(connection_handle).result

    # check that there is only one definition of the generic attribute service
    ga_services = [s for s in services if s["UUID"] == GENERIC_ATTRIBUTE_SERVICE_UUID]
    assert 1 == len(ga_services)
    ga_service = ga_services[0]

    ga_characteristics = ga_service["characteristics"]
    # there should be only one characteristic in this service
    assert 1 == len(ga_characteristics)

    # check that there is only one instance of the service changed characteristic
    # and that this characteristic is well formed
    service_changed_char = [s for s in ga_characteristics if s["UUID"] == SERVICE_CHANGED_CHARACTERISTIC_UUID]
    assert 1 == len(service_changed_char)
    assert ["indicate"] == service_changed_char[0]["properties"]

    # TODO: test if characteristics descriptors are here or not
    # TODO: test if the indicate flag can be writen and read
    # TODO: test if an indication is triggered once the service has changed
