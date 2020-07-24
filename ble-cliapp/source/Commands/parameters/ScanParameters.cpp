/* Copyright (c) 2015-2020 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "Serialization/GapSerializer.h"
#include "Serialization/BLECommonSerializer.h"
#include "CLICommand/CommandSuite.h"
#include "CLICommand/CommandHelper.h"

#include "ScanParameters.h"

namespace {

// global parameters being modified
ble::ScanParameters parameters;

DECLARE_CMD(Reset) {
    CMD_NAME("reset")
    CMD_HANDLER(CommandResponsePtr& response) {
        parameters = ble::ScanParameters();
        response->success();
    }
};

DECLARE_CMD(SetOwnAddressType) {
    CMD_NAME("setOwnAddressType")
    CMD_ARGS(
        CMD_ARG("ble::own_address_type_t::type", "type", "")
    )
    CMD_HANDLER(ble::own_address_type_t::type type, CommandResponsePtr& response) {
        parameters.setOwnAddressType(type);
        response->success();
    }
};

DECLARE_CMD(SetFilter) {
    CMD_NAME("setFilter")
    CMD_ARGS(
        CMD_ARG("ble::scanning_filter_policy_t::type", "filter", "")
    )
    CMD_HANDLER(ble::scanning_filter_policy_t::type filter, CommandResponsePtr& response) {
        parameters.setFilter(filter);
        response->success();
    }
};

DECLARE_CMD(SetPhys) {
    CMD_NAME("setPhys")
    CMD_ARGS(
        CMD_ARG("bool", "enable1M", ""),
        CMD_ARG("bool", "enableCoded", "")
    )
    CMD_HANDLER(bool enable1M, bool enableCoded, CommandResponsePtr& response) {
        parameters.setPhys(enable1M, enableCoded);
        response->success();
    }
};

DECLARE_CMD(Set1mPhyConfiguration) {
    CMD_NAME("set1mPhyConfiguration")
    CMD_ARGS(
        CMD_ARG("ble::scan_interval_t", "interval", ""),
        CMD_ARG("ble::scan_window_t", "window", ""),
        CMD_ARG("bool", "activeScanning", "")
    )
    CMD_HANDLER(ble::scan_interval_t interval, ble::scan_window_t window, bool activeScanning, CommandResponsePtr& response) {
        parameters.set1mPhyConfiguration(interval, window, activeScanning);
        response->success();
    }
};

DECLARE_CMD(SetCodedPhyConfiguration) {
    CMD_NAME("setCodedPhyConfiguration")
    CMD_ARGS(
        CMD_ARG("ble::scan_interval_t", "interval", ""),
        CMD_ARG("ble::scan_window_t", "window", ""),
        CMD_ARG("bool", "activeScanning", "")
    )
    CMD_HANDLER(ble::scan_interval_t interval, ble::scan_window_t window, bool activeScanning, CommandResponsePtr& response) {
        parameters.setCodedPhyConfiguration(interval, window, activeScanning);
        response->success();
    }
};

}

DECLARE_SUITE_COMMANDS(ScanParametersCommandSuiteDescription,
    CMD_INSTANCE(Reset),
    CMD_INSTANCE(SetOwnAddressType),
    CMD_INSTANCE(SetFilter),
    CMD_INSTANCE(SetPhys),
    CMD_INSTANCE(Set1mPhyConfiguration),
    CMD_INSTANCE(SetCodedPhyConfiguration)
);

const ble::ScanParameters& ScanParametersCommandSuiteDescription::get() {
    return parameters;
}
