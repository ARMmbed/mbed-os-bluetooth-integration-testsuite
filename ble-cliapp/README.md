This application is an interactive shell for mbed BLE communicating through UART.
This can be used by a user to debug BLE issues or run automated tests.  

# Compilation

Use [mbed-cli](https://github.com/ARMmbed/mbed-cli) to compile the application: 

```shell
mbed compile -t <compiler> -m <target> --profile toolchains_profile.json
```

> **Note:** It is important to apply the profile located in 
`toolchains_profile.json` to reduce the size of the binary.

# First run

Once the application is compiled, load it onto your board and open a serial port 
terminal targeting the COM port of your board. It is important to set the 
baudrate to 115.200 baud. 

A good way to start with the application is to create a simple beacon.

### Initializing BLE

First, ble has to be initialised. In C++, this would be achieve by the following 
lines of code: 

```c++
BLE& ble = BLE::Instance()
ble.init();
```

With ble-cliapp enter the following command to achieve the same result: 

```
ble init
```

> Invoking a command follow the format:  `<command module> <command name> [arguments]`.
> A module is a cohesive group of command.

In this case, `ble` is the ble module and `init` is the initialization command 
of the ble module. No arguments are supplied.

The board should respond with the following JSON document: 

```json
{
        "status": 0
}
```

The `status` field indicate if the command has succeed or not. A value of 0 
indicate that the command execution was a success. The list of status code can 
be found in the enumeration 
[CommandResponse::StatusCode_t](source/CLICommand/CommandResponse.h).


### Setting up an advertising payload 

The advertising payload of the beacon for this example will just be a 
complete local name with `FOO_BAR_BAZ` as the value.

The C++ code for this operation would be: 

```c++
const char device_name[] = "FOO_BAR_BAZ";

ble::AdvertisingDataBuilder adv_data_builder;
adv_data_builder.setName(device_name);

ble.gap().setAdvertisingPayload(
    ble::LEGACY_ADVERTISING_HANDLE,
    adv_data_builder.getAdvertisingData()
);
```

While with ble-cliapp the boilerplate is eliminated and can be reduce to the 
command: 

```
advDataBuilder setName FOO_BAR_BAZ true
gap applyAdvPayloadFromBuilder 0
```

The command `advDataBuilder` is used to prepare the advertising payload. The command `applyAdvPayloadFromBuilder` finalises the payload.

It is possible to verify the content of the advertising payload by typing the 
command `gap getAdvertisingPayload`. The device should respond with the 
following JSON document: 

```json
{
        "status": 0,
        "result": {
                "COMPLETE_LOCAL_NAME": "FOO_BAR_BAZ",
                "raw": "0C09464F4F5F4241525F42415A"
        }
}
```

A new field, `result`, is present in the response. It is used by commands to 
transmit the result of the command if any. For the command `getAdvertisingPayload`
it is the content of the payload. If the command does not succeed, the response 
can also carry details about the faillure in an `error` field instead of a 
`result` field.


### Start advertising 

In C++ this would be achieved by: 

```c++
gap.startAdvertising(LEGACY_ADVERTISING_HANDLE);
```

while with ble-cliapp this can be done with the command 

```
gap startAdvertising 0 0 0
```

For explanation of the parameters see the commands documentation. At this point,
the device should be visible with a BLE scanner and advertise the complete
local name `FOO_BAR_BAZ`.


# Exploration 

It is possible to list commands available in every module and query help about 
each of them in ble-cliapp itself but this is only possible with
ENABLE_COMMAND_HELP macro enabled.

### Modules available 

The modules available in ble-cliapp are: 

* `ble`
* `gap`
* `gattServer`
* `gattClient`
* `securityManager`

### List commands of a module 

Every module have a builtin command which list all the commands of the module, 
it is the `list` command which can be invoked with `<module name> <list>`.

For instance, to list the commands in the `gap` module, the command `gap list` 
should be entered.

Every entry in the `result` array is a command belonging to the module. 

### Help of command 

To get details about a command, it is possible to use the builtin `help` command.
It syntax is: `<module name> <help> <command name>`

For example, to get more informations around the command connect of the gap 
module, just enter the command: `gap help connect`. The device should output
the result listing the command name, the expected parameters with their type and 
the results sent back in case of success.


## License and contributions

The software is provided under the Apache-2.0 license.
