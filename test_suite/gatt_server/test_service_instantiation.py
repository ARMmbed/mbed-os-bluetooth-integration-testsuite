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
from common.device import log
from common.gap_utils import gap_connect, discover_user_services, make_uuid, assert_uuid_equals

SERVICE_16BIT_UUID = 0xFFEE
SERVICE_128BIT_UUID = make_uuid()
CHARACTERISTIC_16BIT_UUID = 0xFFAA
CHARACTERISTIC_128BIT_UUID = make_uuid()

PARAM_SERVICES = [
    ("single 16bit uuid service", [SERVICE_16BIT_UUID]),
    ("multiple 16bit uuid services with same uuid", [SERVICE_16BIT_UUID for i in range(3)]),
    ("multiple 16bit uuid services", [SERVICE_16BIT_UUID + i for i in range(3)]),
    ("single 128bit uuid service", [SERVICE_128BIT_UUID]),
    ("multiple 128bit uuid services with same uuid", [SERVICE_128BIT_UUID for i in range(3)]),
    ("multiple 128bit uuid services", [make_uuid() for i in range(3)])
]

PARAM_CHARACTERISTICS = [
    ("no characteristic", []),
    ("single 16bit uuid characteristic", [CHARACTERISTIC_16BIT_UUID]),
    ("multiple 16bit uuid characteristics", [CHARACTERISTIC_16BIT_UUID + i for i in range(4)]),
    ("single 128bit uuid characteristic", [CHARACTERISTIC_128BIT_UUID]),
    ("multiple 128bit uuid characteristics", [make_uuid() for i in range(4)])
]


def instantiation_of_services(central, peripheral, services_uuids, characteristics_uuids=[]):
    """test for instantiation of services characteristic"""
    declared_services = []

    # declare services and commit them
    for uuid in services_uuids:
        peripheral.gattServer.declareService(uuid)
        for characteristic_uuid in characteristics_uuids:
            peripheral.gattServer.declareCharacteristic(characteristic_uuid)
        declared_service = peripheral.gattServer.commitService().result

        # assert that characteristics UUID match and that services UUID match
        assert_uuid_equals(uuid, declared_service["UUID"])
        assert len(characteristics_uuids) == len(declared_service["characteristics"])
        for i in range(len(characteristics_uuids)):
            assert_uuid_equals(characteristics_uuids[i], declared_service["characteristics"][i]["UUID"])

        declared_services.append(declared_service)

    # connect central and peripheral and discover user services
    central_connection_handle, _ = gap_connect(central, peripheral)
    discovered_services = discover_user_services(central, central_connection_handle)

    # there should be only one service
    assert len(services_uuids) == len(discovered_services)

    # services discovery should return services in declaration order
    for i in range(len(services_uuids)):
        declared_service = declared_services[i]
        discovered_service = discovered_services[i]

        assert_uuid_equals(declared_service["UUID"], discovered_service["UUID"])
        assert declared_service["handle"] == discovered_service["start_handle"]

        # discovery process should report the same characteristics as the ones registered
        assert len(declared_service["characteristics"]) == len(discovered_service["characteristics"])

        for j in range(len(declared_service["characteristics"])):
            declared_characteristic = declared_service["characteristics"][j]
            discovered_characteristic = discovered_service["characteristics"][j]

            assert declared_characteristic["value_handle"] == discovered_characteristic["value_handle"]
            assert_uuid_equals(declared_characteristic["UUID"], discovered_characteristic["UUID"])


@pytest.mark.ble41
@pytest.mark.parametrize('service', PARAM_SERVICES)
@pytest.mark.parametrize('characteristic', PARAM_CHARACTERISTICS)
def test_combination_of_services_and_characteristics(central, peripheral, service, characteristic):
    log.info('Testing instantiation of {} with {}'.format(service[0], characteristic[0]))
    instantiation_of_services(central, peripheral, service[1], characteristic[1])


@pytest.mark.ble41
def test_no_services(central, peripheral):
    """Discovery of a gatt peripheral with no services instantiated by the user should yield no user services"""
    # connect central and peripheral and discover user services
    central_connection_handle, _ = gap_connect(central, peripheral)
    discovered_services = discover_user_services(central, central_connection_handle)
    assert 0 == len(discovered_services)
