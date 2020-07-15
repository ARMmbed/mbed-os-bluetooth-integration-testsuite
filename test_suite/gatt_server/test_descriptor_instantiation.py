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
from common.gap_utils import gap_connect, discover_descriptors, discover_user_services, assert_uuid_equals, make_uuid

CHARACTERISTIC_16BIT_UUID = 0xDEAD
DESCRIPTOR_16BIT_UUID = 0xFAAD

PARAM_PROPERTIES = [
    ("notify", ["notify"]),
    ("indicate", ["indicate"]),
    ("notify and indicate", ["notify", "indicate"]),
]

PARAM_CHARACTERISTICS = [
    ("16bit uuid characteristic", CHARACTERISTIC_16BIT_UUID),
    ("128bit uuid characteristic", make_uuid())
]

PARAM_DESCRIPTORS = [
    ("no descriptor", []),
    ("single 16bit uuid descriptor", [DESCRIPTOR_16BIT_UUID]),
    ("multiple 16bit uuid descriptors", [DESCRIPTOR_16BIT_UUID + i for i in range(2)]),
    ("single 128bit uuid descriptor", [make_uuid()]),
    ("multiple 128bit uuid descriptors", [make_uuid() for i in range(2)])
]


@pytest.mark.ble41
@pytest.mark.parametrize('char_property', PARAM_PROPERTIES)
@pytest.mark.parametrize('descriptor', PARAM_DESCRIPTORS)
def test_characteristic_has_cccd(central, peripheral, char_property, descriptor):
    log.info(
        'Testing characteristic with a property {} and {} has a CCCD available'.format(char_property[0], descriptor[0])
    )
    properties = char_property[1]
    descriptors = descriptor[1]

    services_uuids = [
        0xBEAF,
        make_uuid()
    ]

    characteristics = [
        0xDEAD,
        make_uuid()
    ]

    declared_services = []

    # declare services and commit them
    for uuid in services_uuids:
        peripheral.gattServer.declareService(uuid)
        for characteristic in characteristics:
            peripheral.gattServer.declareCharacteristic(characteristic)
            peripheral.gattServer.setCharacteristicProperties(*properties)
            for descriptor in descriptors:
                peripheral.gattServer.declareDescriptor(descriptor)
        declared_service = peripheral.gattServer.commitService().result

        # assert that characteristics UUID match and that services UUID match
        assert_uuid_equals(uuid, declared_service["UUID"])
        assert len(characteristics) == len(declared_service["characteristics"])
        for i in range(len(characteristics)):
            characteristic = characteristics[i]
            declared_characteristic = declared_service["characteristics"][i]
            assert_uuid_equals(characteristic, declared_characteristic["UUID"])
            declared_descriptors = declared_characteristic["descriptors"]
            for j in range(len(descriptors)):
                assert_uuid_equals(descriptors[j], declared_descriptors[j]["UUID"])

        declared_services.append(declared_service)

    # connect central and peripheral and discover user services
    central_connection_handle, _ = gap_connect(central, peripheral)
    discovered_services = discover_user_services(central, central_connection_handle)

    # the number of services discovered should be equal to the number of services registered
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

            # discover descriptors and compare against what has been registered
            discovered_descriptors = discover_descriptors(
                central,
                central_connection_handle,
                discovered_characteristic["start_handle"],
                discovered_characteristic["end_handle"]
            )

            # find the CCCD
            cccd = [x for x in discovered_descriptors if x["UUID"] == 0x2902]
            assert 1 == len(cccd)
            discovered_descriptors.remove(cccd[0])

            # notify/indicate descriptor is implicit
            assert len(declared_characteristic["descriptors"]) == len(discovered_descriptors)
            for k in range(len(declared_characteristic["descriptors"])):
                declared_descriptor = declared_characteristic["descriptors"][k]
                discovered_descriptor = discovered_descriptors[k]
                assert_uuid_equals(declared_descriptor["UUID"], discovered_descriptor["UUID"])
                assert declared_descriptor["handle"] == discovered_descriptor["handle"]


@pytest.mark.ble41
@pytest.mark.parametrize('characteristic', PARAM_CHARACTERISTICS)
@pytest.mark.parametrize('descriptor', PARAM_DESCRIPTORS)
def test_allow_characteristic(central, peripheral, characteristic, descriptor):
    log.info('Testing {} with {} is allowed'.format(characteristic[0], descriptor[0]))

    characteristics = [
        {"uuid": characteristic[1], "descriptors_uuid": descriptor[1]}
    ]

    services_uuids = [
        0xBEAF,
        make_uuid()
    ]

    declared_services = []

    # declare services and commit them
    for uuid in services_uuids:
        peripheral.gattServer.declareService(uuid)
        for characteristic in characteristics:
            peripheral.gattServer.declareCharacteristic(characteristic["uuid"])
            for descriptor_uuid in characteristic["descriptors_uuid"]:
                peripheral.gattServer.declareDescriptor(descriptor_uuid)
        declared_service = peripheral.gattServer.commitService().result

        # assert that characteristics UUID match and that services UUID match
        assert_uuid_equals(uuid, declared_service["UUID"])
        assert len(characteristics) == len(declared_service["characteristics"])
        for i in range(len(characteristics)):
            characteristic = characteristics[i]
            declared_characteristic = declared_service["characteristics"][i]
            assert_uuid_equals(characteristic["uuid"], declared_characteristic["UUID"])
            descriptors_uuid = characteristic["descriptors_uuid"]
            declared_descriptors = declared_characteristic["descriptors"]
            for j in range(len(descriptors_uuid)):
                assert_uuid_equals(descriptors_uuid[j], declared_descriptors[j]["UUID"])

        declared_services.append(declared_service)

    # connect central and peripheral and discover user services
    central_connection_handle, _ = gap_connect(central, peripheral)
    discovered_services = discover_user_services(central, central_connection_handle)

    # the number of services discovered should be equal to the number of services registered
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

            # discover descriptors and compare against what has been registered
            discovered_descriptors = discover_descriptors(
                central,
                central_connection_handle,
                discovered_characteristic["start_handle"],
                discovered_characteristic["end_handle"]
            )

            assert len(declared_characteristic["descriptors"]) == len(discovered_descriptors)
            for k in range(len(declared_characteristic["descriptors"])):
                declared_descriptor = declared_characteristic["descriptors"][k]
                discovered_descriptor = discovered_descriptors[k]
                assert_uuid_equals(declared_descriptor["UUID"], discovered_descriptor["UUID"])
                assert declared_descriptor["handle"] == discovered_descriptor["handle"]
