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

from typing import List, Optional, Any, Mapping

import mbed_lstools
import pytest
from mbed_flasher.flash import Flash

from .ble_device import BleDevice
from .serial_connection import SerialConnection
from .serial_device import SerialDevice


@pytest.fixture(scope="session")
def platforms(request):
    if request.config.getoption('platforms'):
        return request.config.getoption('platforms').split(',')
    else:
        return [
            'DISCO_L475VG_IOT01A',
            'NRF52840_DK'
        ]


@pytest.fixture(scope="session")
def binaries(request):
    if request.config.getoption('binaries'):
        platform_and_binaries = request.config.getoption('binaries').split(',')
        result = {}
        for pb in platform_and_binaries:
            pb = pb.split('=')
            result[pb[0]] = pb[1]
        return result
    return {}


@pytest.fixture(scope="session")
def serial_inter_byte_delay(request):
    if request.config.getoption('serial_inter_byte_delay'):
        return float(request.config.getoption('serial_inter_byte_delay'))
    return None


class BoardAllocation:
    def __init__(self, description: Mapping[str, Any]):
        self.description = description
        self.ble_device = None  # type: Optional[BleDevice]
        self.flashed = False


class BoardAllocator:
    def __init__(self, platforms_supported: List[str], binaries: Mapping[str, str], serial_inter_byte_delay: float):
        mbed_ls = mbed_lstools.create()
        boards = mbed_ls.list_mbeds(filter_function=lambda m: m['platform_name'] in platforms_supported)
        self.board_description = boards
        self.binaries = binaries
        self.allocation = []  # type: List[BoardAllocation]
        self.flasher = None
        self.serial_inter_byte_delay = serial_inter_byte_delay
        for desc in boards:
            self.allocation.append(BoardAllocation(desc))

    def allocate(self, name: str = None) -> Optional[BleDevice]:
        for alloc in self.allocation:
            if alloc.ble_device is None:
                # Flash if a binary is provided and the board hasn't been flashed yet
                platform = alloc.description['platform_name']
                binary = self.binaries.get(platform)
                if alloc.flashed is False and binary:
                    if self.flasher is None:
                        self.flasher = Flash()
                    self.flasher.flash(build=binary, target_id=alloc.description["target_id"])
                    alloc.flashed = True

                # Create the serial connection
                connection = SerialConnection(
                    port=alloc.description["serial_port"],
                    baudrate=115200,
                    inter_byte_delay=self.serial_inter_byte_delay
                )
                connection.open()

                # Create the serial device
                serial_device = SerialDevice(connection, name)
                serial_device.reset(duration=1)
                serial_device.flush(1)

                # Create the BleDevice
                alloc.ble_device = BleDevice(serial_device)

                alloc.ble_device.send('set --retcode true', 'retcode: 0')
                alloc.ble_device.send('echo off', 'retcode: 0')
                alloc.ble_device.send('set --vt100 off', 'retcode: 0')

                return alloc.ble_device
        return None

    def release(self, ble_device: BleDevice) -> None:
        for alloc in self.allocation:
            if alloc.ble_device == ble_device and alloc.ble_device is not None:
                # Restore the board
                alloc.ble_device.send('echo on', 'retcode: 0')
                alloc.ble_device.send('set --vt100 on', 'retcode: 0')
                alloc.ble_device.send('set retcode false')

                # Stop activities
                alloc.ble_device.device.stop()
                alloc.ble_device.device.serial.close()

                # Cleanup
                alloc.ble_device = None


@pytest.fixture(scope="session")
def board_allocator(platforms: List[str], binaries: Mapping[str, str], serial_inter_byte_delay: float):
    yield BoardAllocator(platforms, binaries, serial_inter_byte_delay)


@pytest.fixture(scope="function")
def device(board_allocator):
    device = board_allocator.allocate(name='DUT')
    yield device
    board_allocator.release(device)


@pytest.fixture(scope="function")
def peripheral(board_allocator: BoardAllocator):
    device = board_allocator.allocate('peripheral')
    assert device is not None

    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)


@pytest.fixture(scope="function")
def central(board_allocator: BoardAllocator):
    device = board_allocator.allocate('central')
    assert device is not None

    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)
