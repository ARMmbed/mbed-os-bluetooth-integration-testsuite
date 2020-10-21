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

import json
import queue
from time import sleep
from typing import List, Optional

from .device import Device

# some constants
LEGACY_ADVERTISING_HANDLE = 0
ADV_DURATION_FOREVER = 0
ADV_MAX_EVENTS_UNLIMITED = 0


class CommandResult:
    """Model a command result from ble-cliapp.
    Command results in ble-cliapp are expressed in the json object which
    contains two attributes:
      * status: The status of the command, 0 means a success while less
      than 0 indicate a failure.
      * result or error: If the command succeed then a result might be
      available in the result attribute. If the command execution fail then
      an error might be available in an error attribute. Both of these
      attributes are optional.

    To fit with the asynchronous nature of command response from the command
    line, the command result is not parsed right away but is rather parsed
    at the access point. The reason is simple, if the internal response object
    is accessed then the process block until a response is available or timeout.
    The lazy initialization prevent this behaviour and allow the application to
    launch several commands on multiple DUT before blocking.
    """

    def __init__(self, response):
        """Initialize the instance with a response issued from a command. """
        self.__response = response
        self.__initialized = False

    # pylint: disable=W0201,I0011
    def __initialize(self):
        """Delayed initialization.
        This function will extract the lines in the response, parse the JSON payload
        and add the attribute status, error and result in this object.
        """
        if self.__initialized is True:
            return

        # remove the retcode ...
        self.__response.lines.pop()
        json_response = json.loads("".join(self.__response.lines))
        self.status = json_response['status']
        self.error = json_response.get('error')
        self.result = json_response.get('result')
        self.__initialized = True

    def success(self):
        """Returns true if the command succeed and false otherwise.
        Warning: will block the process until a response is available or the
        command associated timeout."""
        return self.status == 0

    def __getattr__(self, name):
        """ Lazily instantiate the class if status, error or result are accessed."""
        if self.__initialized is True:
            raise AttributeError("attribute: %s does not exist in BleTestCase.Commandresult" % name)

        self.__initialize()
        return getattr(self, name)


def make_ble_command(module, cmd, argv):
    """ Create a command understandable by ble-cliapp.
      * module: The name of the module containing the command to invoke.
      * cmd: The name of the command to invoke.
      * argv: Arguments to pass to the command. Every argument is converted
      to its string representation during the construction of the command
      except for Bool value which are converted to true or false (lowercase).
    """

    def serialize_value(value):
        """serializer for values in argv"""
        if isinstance(value, bool):
            return "true" if value is True else "false"
        else:
            return str(value)

    argv = map(serialize_value, argv)
    cmd_args = [] if argv is None else " ".join(argv)
    return "{0} {1} {2}".format(module, cmd, cmd_args)


class BleCommand:
    """Ble command factory object.
    This factory is constructed from:
        * device: The device which will receive the command.
        * module: The name of the command module.
        * cmd: The name of the command to invoke when the call operator is
        used.

    It is possible to construct this object manually but it is safer and
    easier to construct it from a BLEDevice object:

        start_scan = BleCommand(dev, "gap", "startScan")

    is equivalent to

        start_scan = dev.gap.startScan

    Once a BLECommand is constructed, user code can:
        * Change the expected retcode by using the method withRetCode.

            start_scan.withRetCode(1)

        * Configure the command as an async one by using the method setAsync.

            start_scan.setAsync(True)

        * Invoke the command by using the call operator. This operation will
        return a CommandResult object.

            res = start_scan(10000, "0A:11:22:33:44:55")

    It is not absolutely necessary to keep track of the BleCommand object,
    configuration methods (withRetCode and setAsync) return self and chan be chained.

    It is better to chain calls directly and just store the response (if necessary!):

        res = dev.gap.startScan(10000, "0A:11:22:33:44:55")

    If the command needs extra configuration:
        res = dev.gap.startScan.withRetCode(-1).setAsync()(10000, "0A:11:22:33:44:55")
    """

    def __init__(self, device: 'BleDevice', module: str, cmd: str):
        """Construct a command factor from a device, a command module name and
        a command name.

        By default, the expected retcode of the command is 0 and the command is
        not an async one.
        """
        self.device = device
        self.module = module
        self.command = cmd
        self.expected_retcode = 0
        self.asynchronous = False

    def withRetcode(self, retcode: int) -> 'BleCommand':
        """Change the expected retcode of the command to retCode."""
        self.expected_retcode = retcode
        return self

    def setAsync(self, async_value: bool = True) -> 'BleCommand':
        """Change the command into an async one."""
        self.asynchronous = async_value
        return self

    def __call__(self, *argv):
        """Invoke the command and return the corresponding CommandResult object.
        The arguments of the command are stored in argv, they will be converted
        in strings before being send to the device.

        The command string generated and sent to the device will be:
            <module name> <command name> [str(argv)].

        The expected_retcode set and the async configuration are also sent to the
        system.
        """
        return self.device.command(
            make_ble_command(
                self.module,
                self.command,
                argv
            ),
            self.expected_retcode,
            self.asynchronous
        )


class BleCommandModule:
    """Module of commands for a particular device.
    Commands can be accessed like attributes/method if they exist in the
    module.
    Note: this class is only used to give a secure access to commands.
    """

    def __init__(self, device, module_name: str, commands: List[str]):
        """Create a new BleCommandModule.
            * device: the BleDevice used to send commands.
            * module_name: the name of this module.
            * commands: the list of the commands available from this module.
        """
        self.device = device
        self.module_name = module_name
        self.commands = commands

    def __getattr__(self, command_name: str) -> BleCommand:
        """Accessor to commands of the module.
        If command_name is a part of the commands of this module then this
        function return a new BleCommand constructed from: the device of this
        module, the module name and the name of the command.
        If command_name does not exist in the command of the module then an
        AttributeError exception is raised.
        """
        if command_name in self.commands:
            return BleCommand(self.device, self.module_name, command_name)
        else:
            raise AttributeError(
                "command: %s is not available in module %s" % (command_name, self.module_name)
            )


class BleDevice(Device):
    """Client to a device running ble-cliapp.

    User code can invoke commands with the string of the command:

    dev = tc.getDevice(1)
    res = dev.command("ble init")

    More importantly, this class adds a lot of syntactic sugar by allowing user
    code to access a module like an attribute and invoke a command from the
    module like a method.

    res = dev.ble.init().

    This code is equivalent to the string command presented earlier but is safer
    because accessing to a non existing command or a non existing module will
    raise an error:

    # invalid, raise an attribute error because the module dle does not exist
    res = dev.dle.init()

    # invalid, raise an attribute error because the command ini does not exist
    # in the module ble.
    res = dev.ble.ini()


    The command modules are created dynamically and are of type: BleCommandModule.
    The same things goes on commands accessed through via a command module; they
    are created dynamically and are of type BleCommand.
    Calling a BleCommand will return a CommandResult object:


    timeout = 10000
    dev_address = "AA:15:20:40:30:50"
    res = dev.gap.startScan(timeout, dev_address)

    In this code:
        * res is of type CommandResult
        * dev is of type BleDevice
        * dev.gap is of type BleCommandModule
        * dev.gap.startScan is of type BleCommand
    """

    # Modules and their command
    COMMAND_MODULES = {
        "ble": [
            "shutdown", "init", "reset", "getVersion", "createFilesystem"
        ],
        "gap": [
            "getAddress", "getMaxWhitelistSize", "getWhitelist", "setWhitelist",
            "enablePrivacy", "setPeripheralPrivacyConfiguration", "getPeripheralPrivacyConfiguration",
            "setCentralPrivacyConfiguration", "setPhy", "setPreferredPhys", "readPhy", "getMaxAdvertisingSetNumber",
            "getMaxAdvertisingDataLength", "createAdvertisingSet", "destroyAdvertisingSet",
            "setAdvertisingParameters", "setAdvertisingPayload", "applyAdvPayloadFromBuilder", "setAdvertisingScanResponse",
            "applyScanRespFromBuilder", "startAdvertising", "stopAdvertising", "isAdvertisingActive",
            "setPeriodicAdvertisingParameters", "setPeriodicAdvertisingPayload", "startPeriodicAdvertising",
            "stopPeriodicAdvertising", "isPeriodicAdvertisingActive", "setScanParameters",
            "startScan", "scanForAddress", "scanForData", "stopScan", "createSync", "createSyncFromList",
            "cancelCreateSync", "terminateSync", "addDeviceToPeriodicAdvertiserList",
            "removeDeviceFromPeriodicAdvertiserList", "clearPeriodicAdvertiserList",
            "getMaxPeriodicAdvertiserListSize", "connect", "waitForConnection",
            "startConnecting", "cancelConnect", "waitForDisconnection",
            "updateConnectionParameters", "manageConnectionParametersUpdateRequest",
            "acceptConnectionParametersUpdate", "rejectConnectionParametersUpdate",
            "disconnect", "isFeatureSupported", "isRadioActive"
        ],
        "gattClient": [
            "discoverAllServicesAndCharacteristics", "discoverAllServices",
            "discoverPrimaryServicesByUUID", "findIncludedServices",
            "discoverCharacteristicsOfService", "discoverCharacteristicsByUUID",
            "discoverAllCharacteristicsDescriptors", "readCharacteristicValue",
            "readUsingCharacteristicUUID", "readLongCharacteristicValue",
            "readMultipleCharacteristicValues", "writeWithoutResponse",
            "signedWriteWithoutResponse", "write", "writeLong", "reliableWrite",
            "readCharacteristicDescriptor", "readLongCharacteristicDescriptor",
            "writeCharacteristicDescriptor", "writeLongCharacteristicDescriptor",
            "negotiateAttMtu", "enableUnsolicitedHVX"
        ],
        "gattServer": [
            "instantiateHRM", "updateHRMSensorValue", "declareService",
            "declareCharacteristic", "setCharacteristicValue",
            "setCharacteristicProperties", "setCharacteristicVariableLength",
            "setCharacteristicMaxLength", "declareDescriptor",
            "setDescriptorValue", "setDescriptorVariableLength",
            "setDescriptorMaxLength", "commitService", "cancelServiceDeclaration",
            "read", "write", "waitForDataWritten", "setCharacteristicSecurity"
        ],
        "securityManager": [
            "init", "preserveBondingStateOnReset", "purgeAllBondingState",
            "generateWhitelistFromBondTable", "setPairingRequestAuthorisation",
            "waitForEvent", "acceptPairingRequestAndWait", "rejectPairingRequest", "enterConfirmationAndWait",
            "enterPasskeyAndWait", "requestPairingAndWait", "allowLegacyPairing", "getSecureConnectionsSupport",
            "setIoCapability", "setDisplayPasskey", "setLinkEncryptionAndWait", "setDatabaseFilepath"
        ],
        "advParams": [
            "reset", "setType", "setPrimaryInterval", "setPrimaryChannels", "setOwnAddressType",
            "setFilter", "setPhy", "setTxPower", "setSecondaryMaxSkip", "setScanRequestNotification",
            "setUseLegacyPDU", "includeTxPowerInHeader", "setAnonymousAdvertising"
        ],
        "advDataBuilder": [
            "getAdvertisingData", "addData", "appendData", "removeData", "addOrReplaceData", "addOrAppendData",
            "clear", "setAppearance", "setFlags", "setTxPowerAdvertised", "setName", "setManufacturerSpecificData",
            "setAdvertisingInterval", "setConnectionIntervalPreference", "setServiceData",
            "setLocalServiceList", "setRequestedServiceList"
        ],
        "scanParams": [
            "reset", "setOwnAddressType", "setFilter", "setPhys", "set1mPhyConfiguration", "setCodedPhyConfiguration"
        ]
    }

    def __init__(self, device: Device, command_delay=0):
        """Create a new BLEDevice instance."""
        self.device = device
        self.command_delay = command_delay
        self.events = queue.Queue()

    def command(self, cmd: str, expected_retcode: int = 0, async_command: bool = False) -> CommandResult:
        """Send a command to the device and return a CommandResult.
        cmd: The complete command string.
        expected_retcode: The return code expected. If the code
        returned by the command doesn't match the expected_retcode
        then an exception is thrown.
        async_command: Indicate if the command is asynchronous.
        """
        response = None

        if async_command:
            sleep(self.command_delay)
            self.device.send(cmd)

            class AsynchronousResponse:
                def __init__(self, device):
                    self.initialized = False
                    self.device = device
                    self._lines = None

                @property
                def lines(self):
                    if not self.initialized:
                        self._lines = self.device.wait_for_output('retcode: ' + str(expected_retcode))
                        self.initialized = True
                    return self._lines

            response = AsynchronousResponse(self)

        else:
            class SynchronousResponse:
                def __init__(self, lines: List[str]):
                    self.lines = lines

            response = SynchronousResponse(
                self.send(cmd, 'retcode: ' + str(expected_retcode))
            )

        return CommandResult(response)

    def __getattr__(self, module_name: str) -> BleCommandModule:
        """Dynamically generate attributes of command modules.
        This method will be called if the attribute lookup fails.
        If module_name is part of COMMAND_MODULES then a CommandModule is
        created and returned. Otherwise an Attribute error is raised.
        """
        commands = self.COMMAND_MODULES.get(module_name)
        if commands is not None:
            return BleCommandModule(self, module_name, commands)
        else:
            raise AttributeError("module: %s is not available in ble-cliapp" % module_name)

    # Add the proxy methods
    def send(self, command, expected_output=None, wait_before_read=None, wait_for_response=30, assert_output=True):
        sleep(self.command_delay)
        lines = self.device.send(command, expected_output, wait_before_read, wait_for_response, assert_output)
        return self._filter_events(lines)

    def flush(self, timeout: float = 0) -> [str]:
        lines = self.device.send(timeout)
        return self._filter_events(lines)

    def wait_for_output(self, search: str, timeout: float = 30, assert_timeout: bool = True) -> [str]:
        lines = self.device.wait_for_output(search, timeout, assert_timeout)
        return self._filter_events(lines)

    def _filter_events(self, lines: List[str]) -> Optional[List[str]]:
        if lines is None:
            return None
        unfiltered_lines = []
        for line in lines:
            if line.startswith('<<<'):
                self.events.put(line[len('<<<'):])
            else:
                unfiltered_lines.append(line)
        return unfiltered_lines
