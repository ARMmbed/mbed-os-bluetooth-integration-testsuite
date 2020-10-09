# BLE tests

This repository contains a wide range of tests for validating ports of the
[Mbed BLE API](https://github.com/ARMmbed/mbed-os/tree/master/features/FEATURE_BLE).

<!-- TOC -->

<!--needs updating-->

- [Principle of operation](#principle-of-operation)
- [Requirements](#requirements)
- [Running the tests](#running-the-tests)
  - [List existing tests](#list-existing-tests)
  - [Run all tests](#run-all-tests)
  - [Run specific tests](#run-specific-tests)
  - [Run test by markers](#run-test-by-markers)
  - [Select targets](#select-targets)
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

Testing happens at system level on real hardware. The machine (a PC) running the
tests is connected to multiple boards running a Remote Procedure Call (RPC) server of Mbed BLE. The tests
send a succession of commands to the connected boards and
assert the results reported by the hardware.

For example, here is a test validating that advertising works:

**Preconditions:** Two devices are connected to the host: an observer and a broadcaster.

The test:

1. Initialises both devices.
2. Queries the MAC address of the broadcaster.
3. Asks the broadcaster to advertise.
4. The observer listens for advertisements coming from the MAC address of the
broadcaster.

**Postcondition:** Advertising from the broadcaster should have been collected.

To simplify the development of new tests, the tests are coded in Python. Sending commands to the device under test is very similar to calling functions
of the API.

# Requirements

The tests are written using [pytest](https://docs.pytest.org/en/stable/contents.html)
and standard Mbed components list and flash the device under test.

To run the tests, you need:

* The Bluetooth test suite: this repository.

    To install required Python packages run the command `pip install -r requirements.txt`.

    We recommend using a virtual environment to prevent dependencies issues. To
    create and run the virtual environment:

    ```sh
    mkdir venv
    virtualenv venv
    cd venv
    source bin/activate
    ```

* Boards supporting BLE. The most demanding test in these
test suite requires three boards to be connected to the machine running the test.

* [ble-cliapp](https://github.com/ARMmbed/ble-cliapp): The RPC server running on
the board.

    > **Note:** This documentation is **not** a reference of `ble-cliapp`. Consult the documentation of that project for more information.  

# Running the tests

## List existing tests

To list tests in the test suite:

```
pytest --collect-only
```

## Run all tests

1. Compile the `ble-cliapp` application for the boards you're using for testing.
1. Flash the application to the boards.
1. Run the command `pytest`.<!--where? In the same location as the mbed compile command?-->

    The terminal displays the executed commands.
1. When all tests have run, the console displays a summary of the results.

## Run specific tests

pytest allows you to run tests in a specific folder or file or individual tests:

* Folder: `pytest <folder name>`
* Test file: `pytest <file path>`
* Test within a file: `pytest <file path>::<test name>`

To use multiple tests cases, test files or test paths, separate them with spaces:

```
pytest <test folder1> <test folder 2> <file path> <file path>::<test name>
```

## Run tests by markers

Some of the tests has been annotated with a pytest marker:

- `ble41`: Test of Bluetooth 4.1 features.
- `ble42`: Test of Bluetooth 4.2 features.
- `ble50`: Test of Bluetooth 5.0 features.
- `smoketest`: Small set of smoke tests.

To run the test annotated with `marker name`, run the command `pytest -m <marker name>`.

## Select targets

To select specific targets for the test, use the command line
argument `--platforms=`. To specify multiple platforms, separate them with a comma `,`.<!--this isn't a type - it's a specific board, isn't it?-->

For example, to run tests on `NRF52840_DK`:

```sh
pytest --platforms=NRF52840_DK
```

## Flash targets

To flash targets with a binary before the test starts, use `--flash=`.<!--do I have to? is that part of the steps you described above for test running? or is it if I have something else I need on the board, and I can skip this test?-->

You can define a list of platform/binary pairs using the syntax
`platform:binary`. To specify multiple pairs, separate them with a comma `,`.

For example, to run tests on `NRF52840_DK` and flash the binary
`ble-cliapp.hex` (already in the working directory):<!--doesn't it first flash and then run the tests?-->

```sh
pytest --flash=NRF52840_DK:ble-cliapp.hex
```

********************************************************************************

# Extending the test suite

the Mbed OS BLE test suite is built using pytest. We recommend reading the [pytest documentation](https://docs.pytest.org/en/stable/) to solve problems
not covered by this document.

## Test conventions

To be discovered by pytest:

- Test files must start with the name `test`.<!--name here, prefix in the next line - are they really different things?-->
- Test functions name must start with the prefix `test`.

## Assertions

pytest does not define an assertion library. To assert a condition, use the Python keyword `assert`.

## Fixtures

[Fixtures](https://docs.pytest.org/en/stable/fixture.html#fixture) inject dependency into a test function.

The key fixture used by the BLE test suite is `board_allocator`. It allocates boards at the beginning of the test session and releases them at the end. This fixture serves as a base to create test-specific fixtures, which need to set up the devices before the test start.

For example, consider the following fixture that returns a BLE peripheral:

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

1. The decorator declares a fixture with a function scope, meaning the returned object is destroyed after the function that used it ends.
1. The function prototype accepts a `board_allocator` argument, which pytest automatically resolves to the `board_allocator` fixture.
1. A board is allocated and the BLE API is initialised.<!--who does the allocating?-->
1. The peripheral fixture uses the `yield` keyword to return the device object to the pytest framework.
1. pytest reenters the peripheral fixture, which continues the execution after the line yield device. The peripheral fixture then release the board using `board_allocator.release(device)`.

To inject a fixture into a test function, the test function must declare a parameter
with the fixture name:

```python
def test_something(peripheral):
    # test body
    pass
```

## Interacting with devices under test (DUT)

Boards are passed to the test using the fixture mechanism.<!--devices, boards and targets seem to be used interchangeably-->

### Sending commands

> **Note:** Command syntax and returned values of `ble-cliapp` is documented in
`ble-cliapp` documentation.

To send "raw" commands, use the `command` method exposed by
`BleDevice` objects:

```python
dev.command("<command string>")
```

With BLE devices, you can also access the commands module like a
regular variable, and invoke a command like a regular method:


```python
module = dev.module_name
command = module.command_name
command_response = command("arg1", "arg2", ...)

# or

command_response = dev.module_name.command_name("arg1", "arg2", ...)
```

### Handling response

You can capture the response to a command in a variable of type `CommandResult`.

You can access the status of the response by accessing the member variable
`status`:

```python
command_response = dev.module_name.command_name("arg1", "arg2", ...)
command_status = command_response.status
```

If the command has a result, then the member variable `result` will expose it. If
the command has an error, the member variable `error` will expose it.

```python
command_result = command_response.result
command_error = command_response.error
```

`ble-cliapp` reports error and results in JSON. The parsing of these JSON bits is
made on your behalf, so you can access the result like a regular Python
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

Thanks to this, you can handle response like a regular Python data
structure and apply common operations like iteration.

As an example, consider the command `getState` from the `gap` module. It returns
a JSON object containing two values: `advertising` and `connected`. To use the result like a Python object:

```python
# the extraction of the result happens inline right after the call by accessing
# the result member of the command response
gap_state = dev.gap.getState().result
assertEqual({ 'advertising': True, 'connected': True}, gap_state)
```

### Expected return code

Every sent command is associated with an *expected return code*. If the response
doesn't reply with the expected return code, then an exception is raised and the
test ends.

By default, the expected return code associated with the command is 0 - the code
for success. To test commands that will fail and return a negative error code (and carry an
`error` variable), you need to set the correct expected return code using the function `withRetCode` from the BleCommand class.

The method returning `self` <!--is there a word missing here?-->you can chain this call with the command call.

```python
response = dev.module_name.command_name.withRetcode(retcode)(arg1, arg2, ...)
```

### Dealing with asynchronicity

When the script sends a command, it blocks further test execution until the DUT responds, or until a timeout is met. This is not very convenient when an
operation involving devices on multiple ends has to be tested. For instance, in
the context of BLE it wouldn't be possible in a single test to ensure that a
connection happens on both ends (client and server) and fire the correct event.

It's better to fire asynchronous commands. These commands won't
block the script execution when sending the command, but rather when accessing the
content (member variables) of the response.

Use the function `setAsync` of the BleCommand class:


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
