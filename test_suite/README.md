# BLE tests

This repository contains a wide range of tests for 
[mbed BLE](https://github.com/ARMmbed/mbed-os/tree/master/features/FEATURE_BLE).
These tests are used to validate ports of the API.

<!-- TOC -->

- [BLE tests](#ble-tests)
- [Principle of operation](#principle-of-operation)
- [Requirements](#requirements)
- [Running the tests](#running-the-tests)
  - [Run specific tests](#run-specific-tests)
  - [List existing tests](#list-existing-tests)
  - [Run test with markers](#run-test-with-markers)
  - [Select targets type](#select-targets-type)
- [Extending the test suite](#extending-the-test-suite)
  - [Test conventions](#test-conventions)
  - [Fixtures](#fixtures)
  - [Interacting with devices under test (DUT)](#interacting-with-devices-under-test-dut)
    - [Sending commands](#sending-commands)
    - [Handling response](#handling-response)
    - [Expected return code](#expected-return-code)
    - [Dealing with asynchronicity](#dealing-with-asynchronicity)

<!-- /TOC -->


# Principle of operation

Testing happen at system level on real hardware. The machine (a PC) running the 
tests is connected to multiple boards running an RPC server of mbed BLE. Tests
consist in a succession of commands being sent to the connected boards and 
assertion of the results reported by the hardware. 

For instance, a test validating that advertising work might be sumarized in the 
following scenario: 

**Preconditions:** 2 devices are connected to the host. 

1. The two devices, one observer and one broadcaster, are initialized. 
2. The MAC address of the broadcaster is queried. 
3. The broadcaster is asked to advertise
4. The observer listen for advertisements comming from the MAC address of the 
broadcaster. 

**PostCondition:** Advertising from the broadcaster should have been collected.

To simplify the development of new tests, the test are coded in Python and 
sending commands to the device under test is very similar to calling functions 
of the API.


# Requirements 

To run the test, two dedicated software components are needed: 
* The bluetooth test suite: This repository
* [ble-cliapp](../ble-cliapp): The RPC server running on
the board. 

The tests are written using [pytest](https://docs.pytest.org/en/stable/contents.html) 
and standard mbed components are used to list and flash device under test. 
To install required python packages run the command `pip install -r requirements.txt`.

It is advised to use a virtual environment to prevent dependencies issues, to 
create and run the virtual environment execute the following commands: 

```sh
mkdir venv
virtualenv venv
cd venv 
source bin/activate
```

Then install Python packages required to run the test suite:

```
<in the test suite folder>
pip install -r requirements.txt
```

Boards supporting BLE are required as well. The most demanding test in these 
test suite requires 3 boards to be connected to the machine running the test. 

> **Note:** This documentation is **not** a reference of `ble-cliapp`. Consult 
> the documentation of that project to get more insight about it.

# Running the tests 

* Compile the `ble-cliapp` application for the boards involved in the test and 
flash the hardware with it. 
* Run the command: `python -m pytest`
* The commands executed should be displayed in the terminal. 
* At the end of the test suite execution a summary of the results is displayed 
  in the console.


## Run specific tests 

Pytest allows you to run tests in a specific folder or file or individual tests:
- folder: `python -m pytest <folder name>`
- test file: `python -m pytest <file path>` 
- test within a file: `python -m pytest <file path>::<test name>`  


Multiple tests cases, test files or test path can be specified. They are separated 
by spaces.

```
python -m pytest <test folder1> <test folder 2> <file path> <file path>::<test name>
```

## List existing tests 

To list tests in the test suite use the command: 

```
python -m pytest --collect-only
```

## Run test with markers

Some of the tests has been annotated with a pytest marker. Four markers are 
available: 
- ble41: Test of Bluetooth 4.1 features
- ble42: Test of Bluetooth 4.2 features
- ble50: Test of Bluetooth 5.0 features
- smoketest: Small set of smoke tests.

Run the command `python -m pytest -m <marker name>` to run the test anotated with `marker name`.

## Select targets type

To select specific target types that should run the test, use the command line 
argument `--platforms=`. Multiple platforms can be specified, they must be separated 
with a `,`. 

For example, this command runs tests on `NRF52840_DK`: 

```sh
python -m pytest --platforms=NRF52840_DK
```


## Flash targets

It is possible to flash targets with a binary before the test start. Use the 
argument `--flash=`. A list of platform/binary pair can be defined using the syntax
`platform:binary`. Multiple pairs can be specified, they must be separated 
with a `,`. 

For example, this following command run tests on `NRF52840_DK` and flash the binary
`ble-cliapp.hex` present in the working directory: 

```sh
pytest --flash=NRF52840_DK:ble-cliapp.hex
```

********************************************************************************

# Extending the test suite 

Mbed OS BLE test suite is built using pytest. We recomand reading the exhaustive 
pytest [documentation](https://docs.pytest.org/en/stable/) when specific problems 
not covered by this document need to be solved. 


## Test conventions 

Test files must start with the name `test` to be discovered by pytest. 
Similarly, test functions name must start with the prefix `test`. 


## Assertions 

Unlike other xUnit tests framework, pytest does not define an assertion library.
To assert a condition, just use the Python keyword `assert`.


## Fixtures 

[Fixtures](https://docs.pytest.org/en/stable/fixture.html#fixture) are a key 
concept in pytest to inject dependency into a test function. 

The key fixture used by the ble test suite is `board_allocator` which is 
available for the whole test session. Boards can be allocated at the begining 
of the test and release at its end. This fixture serves as a base to create test 
specific fixtures which will set up the devices before the test start. 

For instance consider the following fixture that returns a BLE peripheral: 

```python
@pytest.fixture(scope="function") 
def peripheral(board_allocator: BoardAllocator):
    device = board_allocator.allocate('peripheral')
    assert device is not None

    device.ble.init()
    yield device
    device.ble.shutdown()
    board_allocator.release(device)
```

The decorator declares a fixture with a function scope meaning the object returned
is destroyed after the function that used it ends. 
Then the function prototype accepts a `board_allocator` argument. It is automatically
resolved by pytest to the `board_allocator` fixture. 
Then a board is allocated and the BLE API is initialised.

The device object is then returned to the pytest framework using the `yield` keyword. 

The function is reentered at the end of the fixture scope and the board is released. 


To inject a fixture into a test function, the test function must declare a parameter
with the fixture name: 

```python
def test_something(peripheral):
    # test body
    pass
```

## Interacting with devices under test (DUT)

Boards are passed to the test using the fixture mechanism.

### Sending commands 

> **Note:** Command syntax and returned values of `ble-cliapp` is documented in 
`ble-cliapp` documentation.

It is possible to send "raw" commands using the `command` method exposed by 
`BleDevice` objects: 

```python
dev.command("<command string>")
```

However with ble devices it is also possible to access commands module like a 
regular variable and invoke a command like a regular method: 


```python
module = dev.module_name
command = module.command_name 
command_response = command("arg1", "arg2", ...)

# or 

command_response = dev.module_name.command_name("arg1", "arg2", ...)
```

### Handling response 

Response to a command can be captured in a variable. The type of this variable 
will be `CommandResult`. 

Users can access the status of the response by accessing the member variable 
`status`: 

```python
command_response = dev.module_name.command_name("arg1", "arg2", ...)
command_status = command_response.status
```

If the command has a result then the member variable `result` expose it and if 
the command has an error it is exposed by the member variable `error`. 

```python 
command_result = command_response.result
command_error = command_response.error
```

`ble-cliapp` reports error and result in JSON. The parsing of these JSON bits is 
made on behalf of the user so they can access the result like a regular Python 
data structure: 


| JSON          | Python    |
|---------------|-----------|
| object        | dict      |
| array         | list      |
| string        | unicode   |
| number (int)  | int, long |
| number (real) | float     |
| true          | True      |
| false         | False     |
| null          | None      |

As a consequence it is possible to handle response like a regular python data 
structure and apply common operations like iteration on it. 

As an example consider the command `getState` from the `gap` module. It returns 
a JSON object containing two values named `advertising` and `connected`. It is 
possible to use the result like a python object: 

```python
# the extraction of the result happens inline right after the call by accessing 
# the result member of the command response
gap_state = dev.gap.getState().result
assertEqual({ 'advertising': True, 'connected': True}, gap_state)
```


### Expected return code 

Every command sent is associated with an `expected return code`. If the response 
doesn't reply the return code expected then an exception is raised and the 
test ends. 

By default the expected return code associated with the command is 0; the code 
for success. 

To test commands which will fail and return a negative error code (and carry an 
`error` variable) it is necessary to set the correct expected return code. 

This can be achieved with the function `withRetCode` from the BleCommand class. 
The method returning `self` it is possible to chain this call with the command 
call. 

```python
response = dev.module_name.command_name.withRetcode(retcode)(arg1, arg2, ...)
```


### Dealing with asynchronicity 

When a command is sent, the script block the execution until a response is 
received from the DUT or a timeout is met. This is not very convenient when an 
operation involving devices on multiple ends has to be tested. For instance, in 
the context of BLE it wouldn't be possible in a single test to ensure that a 
connection happens on both end (client and server) and fire the correct event. 

Thankfully, it is possible to fire asynchronous commands, these commands won't 
block the execution of the script when the command is sent but rather when the 
content (member variables) of the response is accessed. 

This behavior can be achieved by using the function `setAsync` of the BleCommand
class: 


```python
# send a wait for connection command to a peripheral 
# The command will not block the script until any member 
# of peripheral_connection_response are accessed.
peripheral_connection_response = peripheral.gap.waitForConnection.setAsync()(10000)

# establish the connection from the central.
# this is not an asynchronous command. 
# It will block until the device respond
central_connection_handle = central.gap.connect(connection_args).result["handle"]

# Access the result on the peripheral 
# It will wait for the response if it hasn't been received yet.
peripheral_conenction_handle = peripheral_connection_response.result["handle"] 
```

