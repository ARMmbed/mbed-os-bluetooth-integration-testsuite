# Copyright (c) 2021 Arm Limited
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
from common.ble_device import LEGACY_ADVERTISING_HANDLE, ADV_DURATION_FOREVER, ADV_MAX_EVENTS_UNLIMITED
from time import sleep

@pytest.mark.ble50
def test_restart_periodic_advertising(device):
    # Test that periodic advertising can be restarted if the stopped previously
    # This test does not validate that periodic advertising is operating on the
    # radio.
    device.ble.init()
    adv_handle = device.gap.createAdvertisingSet().result
    device.gap.startAdvertising(adv_handle, ADV_DURATION_FOREVER, LEGACY_ADVERTISING_HANDLE)
    # Actual test
    for i in range(3):
        assert device.gap.startPeriodicAdvertising(adv_handle).success()
        sleep(1)
        assert device.gap.stopPeriodicAdvertising(adv_handle).success()
    device.gap.stopAdvertising(adv_handle)
    device.gap.destroyAdvertisingSet(adv_handle)
    device.ble.shutdown()
