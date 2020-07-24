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
#include "Serialization/BLECommonSerializer.h"
#include "Serialization/GapSerializer.h"
#include "Serialization/GapAdvertisingDataSerializer.h"
#include "Serialization/Hex.h"
#include "Common.h"
#include "CLICommand/CommandSuite.h"
#include "CLICommand/CommandHelper.h"

#include "AdvDataBuilder.h"

namespace {

// global advertising data builder being modified
constexpr static uint8_t max_payload_len = 255; // maximum for BLE 5
static uint8_t adv_buffer[max_payload_len];
static ble::AdvertisingDataBuilder adv_data_builder(adv_buffer);

DECLARE_CMD(GetAdvertisingData) {
    CMD_NAME("getAdvertisingData")

    CMD_HANDLER(CommandResponsePtr& response) {
        // print raw advertising data bytes to serial
        response->success(adv_data_builder.getAdvertisingData());
    }
};

DECLARE_CMD(AddData) {
    CMD_NAME("addData")
    CMD_ARGS(
        CMD_ARG("ble::adv_data_type_t::type", "type", ""),
        CMD_ARG("RawData_t", "data", "")
    )

    CMD_HANDLER(ble::adv_data_type_t::type type, RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.addData(type, mbed::make_const_Span(data.cbegin(), data.size()));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(AppendData) {
    CMD_NAME("appendData")
    CMD_ARGS(
        CMD_ARG("ble::adv_data_type_t::type", "type", ""),
        CMD_ARG("RawData_t", "data", "")
    )

    CMD_HANDLER(ble::adv_data_type_t::type type, RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.appendData(type, mbed::make_const_Span(data.cbegin(), data.size()));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(RemoveData) {
    CMD_NAME("removeData")
    CMD_ARGS(
        CMD_ARG("ble::adv_data_type_t::type", "type", "")
    )

    CMD_HANDLER(ble::adv_data_type_t::type type, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.removeData(type);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(AddOrReplaceData) {
    CMD_NAME("addOrReplaceData")
    CMD_ARGS(
        CMD_ARG("ble::adv_data_type_t::type", "type", ""),
        CMD_ARG("RawData_t", "data", "")
    )

    CMD_HANDLER(ble::adv_data_type_t::type type, RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.addOrReplaceData(type, mbed::make_const_Span(data.cbegin(), data.size()));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(AddOrAppendData) {
    CMD_NAME("addOrAppendData")
    CMD_ARGS(
        CMD_ARG("ble::adv_data_type_t::type", "type", ""),
        CMD_ARG("RawData_t", "data", "")
    )

    CMD_HANDLER(ble::adv_data_type_t::type type, RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.addOrAppendData(type, mbed::make_const_Span(data.cbegin(), data.size()));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(Clear) {
    CMD_NAME("clear")
    CMD_HANDLER(CommandResponsePtr& response) {
        adv_data_builder.clear();
        response->success();
    }
};

DECLARE_CMD(SetAppearance) {
    CMD_NAME("setAppearance")
    CMD_ARGS(
        CMD_ARG("ble::adv_data_appearance_t::type", "appearance", "")
    )

    CMD_HANDLER(ble::adv_data_appearance_t::type appearance, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setAppearance(appearance);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetFlags) {
    CMD_NAME("setFlags")
    CMD_ARGS(
        CMD_ARG("ble::adv_data_flags_t", "flags", "")
    )

    CMD_HANDLER(ble::adv_data_flags_t flags, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setFlags(flags);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetTxPowerAdvertised) {
    CMD_NAME("setTxPowerAdvertised")
    CMD_ARGS(
        CMD_ARG("int8_t", "txPower", "")
    )

    CMD_HANDLER(int8_t txPower, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setTxPowerAdvertised(txPower);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetName) {
    CMD_NAME("setName")
    CMD_ARGS(
        CMD_ARG("char*", "name", ""),
        CMD_ARG("bool", "complete", "")
    )

    CMD_HANDLER(const CommandArgs& args, CommandResponsePtr& response) {
        bool complete;
        if (!fromString(args[1], complete)) {
            response->invalidParameters("complete should be a bool");
            return;
        }
        ble_error_t err = adv_data_builder.setName(args[0], complete);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetManufacturerSpecificData) {
    CMD_NAME("setManufacturerSpecificData")
    CMD_ARGS(
        CMD_ARG("RawData_t", "data", "")
    )

    CMD_HANDLER(RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setManufacturerSpecificData(mbed::make_const_Span(data.cbegin(), data.size()));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetAdvertisingInterval) {
    CMD_NAME("setAdvertisingInterval")
    CMD_ARGS(
        CMD_ARG("const uint32_t", "interval", "")
    )

    CMD_HANDLER(const uint32_t interval, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setAdvertisingInterval(ble::adv_interval_t(interval));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetConnectionIntervalPreference) {
    CMD_NAME("setConnectionIntervalPreference")
    CMD_ARGS(
        CMD_ARG("uint16_t", "min", ""),
        CMD_ARG("uint16_t", "max", "")
    )

    CMD_HANDLER(uint16_t min, uint16_t max, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setConnectionIntervalPreference(
            ble::conn_interval_t(min),
            ble::conn_interval_t(max)
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetServiceData) {
    CMD_NAME("setServiceData")
    CMD_ARGS(
        CMD_ARG("UUID", "service", ""),
        CMD_ARG("RawData_t", "data", "")
    )

    CMD_HANDLER(UUID service, RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setServiceData(
            service,
            mbed::make_const_Span(data.cbegin(), data.size())
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetLocalServiceList) {
    CMD_NAME("setLocalServiceList")
    CMD_ARGS(
        CMD_ARG("UUID", "data", ""),
        CMD_ARG("bool", "complete", "")
    )

    // The CLI app is only able to parse one UUID at a time
    CMD_HANDLER(UUID data, bool complete, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setLocalServiceList(
            mbed::make_const_Span(&data, 1),
            complete
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetRequestedServiceList) {
    CMD_NAME("setRequestedServiceList")
    CMD_ARGS(
        CMD_ARG("UUID", "data", "")
    )

    // The CLI app is only able to parse one UUID at a time
    CMD_HANDLER(UUID data, CommandResponsePtr& response) {
        ble_error_t err = adv_data_builder.setRequestedServiceList(mbed::make_const_Span(&data, 1));
        reportErrorOrSuccess(response, err);
    }
};

}

DECLARE_SUITE_COMMANDS(AdvertisingDataBuilderCommandSuiteDescription,
    CMD_INSTANCE(GetAdvertisingData),
    CMD_INSTANCE(AddData),
    CMD_INSTANCE(AppendData),
    CMD_INSTANCE(RemoveData),
    CMD_INSTANCE(AddOrReplaceData),
    CMD_INSTANCE(AddOrAppendData),
    CMD_INSTANCE(Clear),
    CMD_INSTANCE(SetAppearance),
    CMD_INSTANCE(SetFlags),
    CMD_INSTANCE(SetTxPowerAdvertised),
    CMD_INSTANCE(SetName),
    CMD_INSTANCE(SetManufacturerSpecificData),
    CMD_INSTANCE(SetAdvertisingInterval),
    CMD_INSTANCE(SetConnectionIntervalPreference),
    CMD_INSTANCE(SetServiceData),
    CMD_INSTANCE(SetLocalServiceList),
    CMD_INSTANCE(SetRequestedServiceList)
);

const mbed::Span<const uint8_t> AdvertisingDataBuilderCommandSuiteDescription::get()
{
    return adv_data_builder.getAdvertisingData();
}
