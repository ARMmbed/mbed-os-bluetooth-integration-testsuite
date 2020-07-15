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
def test_init_with_default_params(central):
    """Call to init with default parameters should succeed"""
    central.securityManager.init(True, True, "IO_CAPS_NONE", "*", True, "*").success()


@pytest.mark.ble41
def test_multiple_init(central):
    """Multiple calls to init should succeed"""
    for i in range(NUM_REPEATED_CALLS):
        central.securityManager.init(True, True, "IO_CAPS_NONE", "*", True, "*").success()


@pytest.mark.ble41
def test_multiple_init_with_different_params(central):
    """New call to init with different parameters should succeed"""
    central.securityManager.init(True, True, "IO_CAPS_NONE", "*", True, "*").success()
    central.securityManager.init(False, False, "IO_CAPS_KEYBOARD_DISPLAY", "*", False, "*").success()


@pytest.mark.ble41
def test_init_static_passkey(central):
    """Call to init with static passkey should succeed"""
    central.securityManager.init(True, True, "IO_CAPS_NONE", "123456", True, "*").success()


@pytest.mark.ble41
def test_init_with_different_capabilities(central):
    """Call to init with all possible SecurityIOCapabilities_t values should succeed"""
    central.securityManager.init(True, True, "IO_CAPS_DISPLAY_ONLY", "*", True, "*").success()
    central.securityManager.init(True, True, "IO_CAPS_DISPLAY_YESNO", "*", True, "*").success()
    central.securityManager.init(True, True, "IO_CAPS_KEYBOARD_ONLY", "*", True, "*").success()
    central.securityManager.init(True, True, "IO_CAPS_NONE", "*", True, "*").success()
    central.securityManager.init(True, True, "IO_CAPS_KEYBOARD_DISPLAY", "*", True, "*").success()
