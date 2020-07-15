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

from math import ceil, floor
from random import randint

import pytest

NUM_REPEATED_CALLS = 10
# These values are from the BLE specification v4.2 in Vol 3, Part C, Section 12.3
SLAVE_LATENCY_RANGE = (0x0000, 0x01F3)
CONN_SUPERVISION_TIMEOUT_RANGE = (0x000A, 0x0C80)
CONN_INTERVAL_RANGE = (0x0006, 0x0C80)


@pytest.fixture(scope="function")
def dut(board_allocator):
    device = board_allocator.allocate(name='DUT')
    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


def assert_connection_params(min_connection_interval, max_connection_interval, slave_latency, connection_supervision_timeout):
    def assert_value(value_range, x):
        assert value_range[0] <= x <= value_range[1]

    # Make sure that the values are acceptable
    assert_value(CONN_INTERVAL_RANGE, min_connection_interval)
    assert_value(CONN_INTERVAL_RANGE, max_connection_interval)
    assert_value(SLAVE_LATENCY_RANGE, slave_latency)
    assert_value(CONN_SUPERVISION_TIMEOUT_RANGE, connection_supervision_timeout)

    # Make sure that the min interval isn't larger than max interval
    assert min_connection_interval <= max_connection_interval


def generate_ran_connection_params():
    # This values are generated to comply with the specification see Bluetooth spec v4.2 Vol 6 Part B
    # Section 4.5.2 and Bluetooth spec v4.2 Vol 3 Part C Section 12.3
    # Select random slaveLatency within range
    slave_latency = randint(*SLAVE_LATENCY_RANGE)

    # Select random maxConnectionInterval within range of connectionSupervisionTimeout
    max_connection_interval_upper_bound = ((CONN_SUPERVISION_TIMEOUT_RANGE[1] - 1) * 10) / int((1 + slave_latency) * 1.25 * 2)
    max_connection_interval = randint(ceil(CONN_INTERVAL_RANGE[0]), floor(max_connection_interval_upper_bound))

    # Select random minConnectionInterval that is at most maxConnectionInterval
    min_connection_interval = randint(ceil(CONN_INTERVAL_RANGE[0]), floor(max_connection_interval))

    # Select random connectionSupervisor timeout within range
    min_connection_supervision_timeout = (int((1 + slave_latency) * 1.25 * max_connection_interval * 2) / 10) + 1
    connection_supervision_timeout = randint(ceil(min_connection_supervision_timeout), floor(CONN_SUPERVISION_TIMEOUT_RANGE[1]))

    return {
        "minConnectionInterval": min_connection_interval,
        "maxConnectionInterval": max_connection_interval,
        "slaveLatency": slave_latency,
        "connectionSupervisionTimeout": connection_supervision_timeout
    }


def generate_invalid_connection_params():
    # this function will generate parameters with values that are out of range
    # according to the specification. However, it does NOT generate values that
    # are within range but still do not satisfy the conditions:
    # (1 + connSlaveLatency) * connInterval * 2. and maxInterval >= minInterval.
    # For more information refer to Bluetooth spec v4.2 Vol 6 Part B Section 4.5.2
    # and Bluetooth spec v4.2 Vol 3 Part C Section 12.3
    slave_latency = randint(ceil(SLAVE_LATENCY_RANGE[1] + 1), 0xFFFF)
    min_connection_interval = randint(ceil(CONN_INTERVAL_RANGE[1] + 1), 0xFFFF)
    max_connection_interval = randint(ceil(CONN_INTERVAL_RANGE[1] + 1), 0xFFFF)
    connection_supervision_timeout = randint(ceil(CONN_SUPERVISION_TIMEOUT_RANGE[1] + 1), 0xFFFF)
    return {
        "minConnectionInterval": min_connection_interval,
        "maxConnectionInterval": max_connection_interval,
        "slaveLatency": slave_latency,
        "connectionSupervisionTimeout": connection_supervision_timeout
    }


def generate_invalid_connection_params_within_range(self):
    # this function will generate parameters with values that are within range
    # according to the specification. However, it the params do NOT satisfy the
    # condition: (1 + connSlaveLatency) * connInterval * 2. For more
    # information refer to Bluetooth spec v4.2 Vol 6 Part B Section 4.5.2 and
    # Bluetooth spec v4.2
    # Vol 3 Part C Section 12.3
    slave_latency = randint(*SLAVE_LATENCY_RANGE)
    min_connection_interval = randint(*CONN_INTERVAL_RANGE)
    max_connection_interval = randint(*CONN_INTERVAL_RANGE)
    connection_supervision_timeout_upper_bound = int((1 + slave_latency) * 1.25 * max_connection_interval * 2) / 10
    connection_supervision_timeout = randint(
        CONN_SUPERVISION_TIMEOUT_RANGE[0],
        connection_supervision_timeout_upper_bound
    )

    return {
        "minConnectionInterval": min_connection_interval,
        "maxConnectionInterval": max_connection_interval,
        "slaveLatency": slave_latency,
        "connectionSupervisionTimeout": connection_supervision_timeout
    }


def connection_params_str(p):
    return ",".join([
        str(p["minConnectionInterval"]),
        str(p["maxConnectionInterval"]),
        str(p["slaveLatency"]),
        str(p["connectionSupervisionTimeout"])
    ])


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing BLE API")
def test_default_connection_parameters(device):
    """Check that after initialization, default connection params are valid"""
    connection_params = dut.gap.getPreferredConnectionParams().result
    assert_connection_params(**connection_params)


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing BLE API")
def test_repeated_call_to_get_preferred_connection_params(dut):
    """Repeated calls to getPreferredConnectionParams should return the same value"""
    connection_params = dut.gap.getPreferredConnectionParams().result
    for i in range(NUM_REPEATED_CALLS):
        assert connection_params == dut.gap.getPreferredConnectionParams().result


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing BLE API")
def test_set_preferred_connection_params(dut):
    """setPreferredConnectionParams should set valid peripheral's connection preferred params"""
    connection_params = generate_ran_connection_params()
    dut.gap.setPreferredConnectionParams(connection_params_str(connection_params))
    assert connection_params == dut.gap.getPreferredConnectionParams().result


@pytest.mark.skip(reason="Missing BLE API")
def test_set_preferred_connection_params_multiple_times(dut):
    """setPreferredConnectionParams should update connection params at each call"""
    for i in range(NUM_REPEATED_CALLS):
        connection_params = generate_ran_connection_params()
        dut.gap.setPreferredConnectionParams(connection_params_str(connection_params))
        assert connection_params == dut.gap.getPreferredConnectionParams().result


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing BLE API")
def test_set_preferred_connection_params_multiple_times_with_single_value(dut):
    """repeated calls to setPreferredConnectionParams with the same value should have effect of a single call"""
    connection_params = generate_ran_connection_params()
    for i in range(NUM_REPEATED_CALLS):
        dut.gap.setPreferredConnectionParams(connection_params_str(connection_params))
        assert connection_params == dut.gap.getPreferredConnectionParams().result


@pytest.mark.ble41
@pytest.mark.skip(reason="Missing BLE API")
def test_set_preferred_connection_params_with_invalid_parameters(dut):
    """make sure that setPreferredConnectionParams reject invalid parameters"""
    set_preferred_connection_params = dut.gap.setPreferredConnectionParams.withRetcode(-1)

    for i in range(NUM_REPEATED_CALLS):
        connection_params = generate_invalid_connection_params()
        error = set_preferred_connection_params(connection_params_str(connection_params)).error
        assert error == "BLE_ERROR_PARAM_OUT_OF_RANGE"
