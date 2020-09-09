# Mbed OS Bluetooth integration testing suite

This repository contains code used for testing Bluetooth in Mbed OS. Each component is in its own folder
and comes with its own documentation.

## Summary of components

Integration testing is done by running an application called ble-cliapp on the board to be tested.
Different tests might require more than one board running at the same time (currently up to three).
Boards should be connected to the same host that runs the tests. Tests then communicate with the
boards to perform BLE communication.

### ble-cliapp

Mbed OS application that exposes the BLE API through a serial interface over USB.

### test-suite

Pytest based tests that use the serial interface to communicate with boards to perform BLE operations.

## License and contributions

The software is provided under the [Apache-2.0 license](LICENSE). Contributions to
this project are accepted under the same license.

