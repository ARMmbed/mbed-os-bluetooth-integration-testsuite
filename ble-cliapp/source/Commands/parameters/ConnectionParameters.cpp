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

#include "ConnectionParameters.h"

namespace {

// global parameters being modified
ble::ConnectionParameters parameters;

DECLARE_CMD(Reset) {
    CMD_NAME("reset")
    CMD_HANDLER(CommandResponsePtr& response) {
        parameters = ble::ConnectionParameters();
        response->success();
    }
};

DECLARE_CMD(SetScanParameters) {
    CMD_NAME("setScanParameters")
    CMD_ARGS(
        CMD_ARG("ble::phy_t::type", "phy", ""),
        CMD_ARG("ble::conn_interval_t", "min", ""),
        CMD_ARG("ble::conn_interval_t", "max", "")
    )
    CMD_HANDLER(ble::phy_t::type phy, ble::conn_interval_t min, ble::conn_interval_t max, CommandResponsePtr& response) {
        parameters.setScanParameters(phy, min, max);
        response->success();
    }
};

DECLARE_CMD(SetConnectionParameters) {
    CMD_NAME("setConnectionParameters")
    CMD_ARGS(
        CMD_ARG("ble::phy_t::type", "phy", ""),
        CMD_ARG("ble::conn_interval_t", "min", ""),
        CMD_ARG("ble::conn_interval_t", "max", ""),
        CMD_ARG("uint16_t", "slave_latency", ""),
        CMD_ARG("ble::supervision_timeout_t", "supervision_timeout", "")
    )
    CMD_HANDLER(
        ble::phy_t::type phy,
        ble::conn_interval_t min,
        ble::conn_interval_t max,
        uint16_t slave_latency,
        ble::supervision_timeout_t supervision_timeout,
        CommandResponsePtr& response
    ) {
        parameters.setConnectionParameters(phy, min, max, slave_latency, supervision_timeout);
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

DECLARE_CMD(SetFilterPolicy) {
    CMD_NAME("setFilterPolicy")
    CMD_ARGS(
        CMD_ARG("ble::initiator_filter_policy_t::type", "filter", "")
    )
    CMD_HANDLER(ble::initiator_filter_policy_t::type filter, CommandResponsePtr& response) {
        parameters.setFilter(filter);
        response->success();
    }
};

DECLARE_CMD(TogglePhy) {
    CMD_NAME("togglePhy")
    CMD_ARGS(
        CMD_ARG("bool", "phy1M", ""),
        CMD_ARG("bool", "phy2M", ""),
        CMD_ARG("bool", "phyCoded", ""),
    )
    CMD_HANDLER(bool phy1M, bool phy2M, bool phyCoded, CommandResponsePtr& response) {
        parameters.togglePhy(phy1M, phy2M, phyCoded);
        response->success();
    }
};

DECLARE_CMD(DisablePhy) {
    CMD_NAME("disablePhy")
    CMD_ARGS(
        CMD_ARG("ble::phy_t::type", "phy", "")
    )
    CMD_HANDLER(ble::phy_t::type phy, CommandResponsePtr& response) {
        parameters.disablePhy(phy);
        response->success();
    }
};

DECLARE_CMD(EnablePhy) {
    CMD_NAME("enablePhy")
    CMD_ARGS(
        CMD_ARG("ble::phy_t::type", "phy", "")
    )
    CMD_HANDLER(ble::phy_t::type phy, CommandResponsePtr& response) {
        parameters.enablePhy(phy);
        response->success();
    }
};

}

DECLARE_SUITE_COMMANDS(ConnectionParametersCommandSuiteDescription,
    CMD_INSTANCE(Reset),
    CMD_INSTANCE(SetScanParameters),
    CMD_INSTANCE(SetConnectionParameters),
    CMD_INSTANCE(SetOwnAddressType),
    CMD_INSTANCE(SetFilterPolicy),
    CMD_INSTANCE(TogglePhy),
    CMD_INSTANCE(DisablePhy),
    CMD_INSTANCE(EnablePhy)
);

const ble::ConnectionParameters& ConnectionParametersCommandSuiteDescription::get() {
    return parameters;
}


