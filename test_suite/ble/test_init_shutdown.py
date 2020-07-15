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

NUM_REPEATED_CALLS = 10


@pytest.mark.ble41
def test_shutdown_before_init(device):
    """If the user call shutdown before init, it should fail"""
    error = device.ble.shutdown.withRetcode(-1)().error
    assert error == "BLE_ERROR_INITIALIZATION_INCOMPLETE"


@pytest.mark.ble41
def test_init_then_shutdown(device):
    """Call to init and shutdown (in this order) should succeed"""
    assert device.ble.init().success()
    assert device.ble.shutdown().success()


@pytest.mark.ble41
def test_repeated_shutdown_call_on_uninitialized(device):
    """Repeated call on shutdown over a not initialized instance of BLE should fail"""
    for i in range(NUM_REPEATED_CALLS):
        error = device.ble.shutdown.withRetcode(-1)().error
        assert error == "BLE_ERROR_INITIALIZATION_INCOMPLETE"


@pytest.mark.ble41
def test_repeated_call_to_init(device):
    """Repeated call to init should always succeed"""
    for i in range(NUM_REPEATED_CALLS):
        assert device.ble.init().success()


@pytest.mark.ble41
def test_repeated_init_shutdown(device):
    """Init and shutdown called iteratively in succession should succeed"""
    ble = device.ble
    for i in range(NUM_REPEATED_CALLS):
        assert ble.init().success()
        assert ble.shutdown().success()


@pytest.mark.ble41
def test_get_version(device):
    """getVersion called after init should return a non-empty string"""
    ble = device.ble
    ble.init()
    version_str = ble.getVersion().result
    assert (version_str is not None) and (len(version_str) > 0)
    ble.shutdown()


@pytest.mark.ble41
def test_repeated_get_version(device):
    """getVersion called repeatedly should return the same string"""
    ble = device.ble
    ble.init()
    version = ble.getVersion().result
    for i in range(NUM_REPEATED_CALLS):
        assert version == ble.getVersion().result
    ble.shutdown()
