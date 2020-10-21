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
#include "Serialization/GapAdvertisingDataSerializer.h"
#include "Serialization/BLECommonSerializer.h"
#include "CLICommand/CommandSuite.h"
#include "CLICommand/util/AsyncProcedure.h"
#include "Common.h"
#include "CLICommand/CommandHelper.h"

#include "Timer.h"

#include "GapCommands.h"

#include "parameters/AdvertisingParameters.h"
#include "parameters/AdvDataBuilder.h"
#include "parameters/ScanParameters.h"
#include "parameters/ConnectionParameters.h"
#include "Serialization/Hex.h"
#include "util/HijackMember.h"
#include "GapImpl.h"

typedef bool (ble::impl::Gap::*gap_impl_is_radio_active_method)() const;
HIJACK_MEMBER(_gap_impl_accessor, ble::impl::Gap* Gap::*, &Gap::impl);
HIJACK_MEMBER(_gap_impl_is_radio_active_accessor, gap_impl_is_radio_active_method, &ble::impl::Gap::is_radio_active);

using mbed::util::SharedPointer;

using ble::Gap;
using ble::GattClient;
using ble::GattServer;
using ble::SecurityManager;


// isolation ...
namespace {

const ble::AdvertisingParameters &getAdvertisingParameters() {
    return AdvertisingParametersCommandSuiteDescription::get();
}

const ble::ScanParameters &getScanParameters() {
    return ScanParametersCommandSuiteDescription::get();
}

const ble::ConnectionParameters &getConnectionParameters() {
    return ConnectionParametersCommandSuiteDescription::get();
}

using namespace serialization;

static void printConnectionResult(serialization::JSONOutputStream& os, const ble::ConnectionCompleteEvent &event)
{
    os << startObject << key("status") << event.getStatus();

    if (event.getStatus() != BLE_ERROR_NONE) {
        os << endObject;
        return;
    }

    os << key("peer_address_type") << event.getPeerAddressType() <<
        key("peer_address") << event.getPeerAddress() <<
        key("interval") << event.getConnectionInterval() <<
        key("latency") << event.getConnectionLatency().value() <<
        key("supervision_timeout") << event.getSupervisionTimeout() <<
        key("connection_handle") << event.getConnectionHandle() <<
        key("own_role") << (event.getOwnRole() == ble::connection_role_t::CENTRAL ? "CENTRAL" : "PERIPHERAL") <<
        key("master_clock_accuracy") << event.getMasterClockAccuracy();

    if (event.getPeerResolvablePrivateAddress() != ble::address_t()) {
        os << key("peer_resolvable_private_address") << event.getPeerResolvablePrivateAddress();
    }

    if (event.getLocalResolvablePrivateAddress() != ble::address_t()) {
        os << key("local_resolvable_private_address") << event.getLocalResolvablePrivateAddress();
    }

    os << endObject;
}


static void printDisconnectionResult(serialization::JSONOutputStream& os, const ble::DisconnectionCompleteEvent &event)
{
    os << startObject;

    os << key("connection_handle") << event.getConnectionHandle()
        << key("reason") << event.getReason();

    os << endObject;
}

struct EventHandler : public ble::Gap::EventHandler {

    virtual void onScanRequestReceived(const ble::ScanRequestEvent &event)
    {
        JSONEventStream() << startObject <<
            key("type") << "event" <<
            key("name") << "scan_request_received" <<
            key("value") << startObject <<
                key("peer_address") << event.getPeerAddress() <<
                key("peer_address_type") << event.getPeerAddressType() <<
                key("advertising_handle") << event.getAdvHandle() <<
            endObject <<
        endObject;
    }

    void onAdvertisingStart(const ble::AdvertisingStartEvent &event) override
    {
        JSONEventStream output;
        output << startObject <<
            key("type") << "event" <<
            key("name") << "advertising_start" <<
            key("value") << startObject <<
                key("advertising_handle") << event.getAdvHandle() <<
            endObject <<
        endObject;
    }

    virtual void onAdvertisingEnd(const ble::AdvertisingEndEvent &event)
    {
        JSONEventStream output;
        output << startObject <<
            key("type") << "event" <<
            key("name") << "advertising_end" <<
            key("value") << startObject <<
                key("advertising_handle") << event.getAdvHandle() <<
                key("legacy") << event.isLegacy();
        if (!event.isLegacy()) {
                output <<
                key("completed_events") << event.getCompleted_events() <<
                key("is_connected") << event.isConnected();
            if (event.isConnected()) {
                output <<
                key("connection_handle") << event.getConnection();
            }
        }
            output <<
            endObject <<
        endObject;
    }

    virtual void onAdvertisingReport(const ble::AdvertisingReportEvent &event)
    {
        JSONEventStream os;

        os << startObject <<
            key("type") << "event" <<
            key("name") << "advertising_report" <<
            key("value") << startObject <<
                key("connectable") << event.getType().connectable() <<
                key("scannable") << event.getType().scannable_advertising() <<
                key("scan_response") << event.getType().scan_response() <<
                key("directed") << event.getType().directed_advertising() <<
                key("legacy") << event.getType().legacy_advertising() <<
                key("rssi") << event.getRssi() <<
                key("peer_address_type") << event.getPeerAddressType();

        if (event.getPeerAddressType() != ble::peer_address_type_t::ANONYMOUS) {
            os << key("peer_address") << event.getPeerAddress();
        }

        if (event.getType().directed_advertising()) {
            os << key("direct_address_type") << event.getDirectAddressType() <<
                    key("direct_address") << event.getDirectAddress();
        }

        if (event.getType().legacy_advertising() == false) {
            os << key("sid") << event.getSID() <<
                    key("tx_power") << event.getTxPower() <<
                    key("primary_phy") << event.getPrimaryPhy() <<
                    key("secondary_phy") << event.getSecondaryPhy() <<
                    key("data_status");

            switch (event.getType().data_status().value()) {
                case ble::advertising_data_status_t::COMPLETE:
                    os << "COMPLETE";
                    break;
                case ble::advertising_data_status_t::INCOMPLETE_MORE_DATA:
                    os << "INCOMPLETE_MORE_DATA";
                    break;
                case ble::advertising_data_status_t::INCOMPLETE_DATA_TRUNCATED:
                    os << "INCOMPLETE_DATA_TRUNCATED";
                    break;
                default:
                    os << "unknown";
                    break;
            }

            if (event.isPeriodicIntervalPresent()) {
                os << key("periodic_interval") << event.getPeriodicInterval();
            }
        }

        os <<   key("payload") << event.getPayload() <<
            endObject <<
        endObject;
    }

    virtual void onScanTimeout(const ble::ScanTimeoutEvent &event)
    {
        JSONEventStream() << startObject <<
            key("type") << "event" <<
            key("name") << "scan_timeout" <<
        endObject;
    }

    virtual void onPeriodicAdvertisingSyncEstablished(
        const ble::PeriodicAdvertisingSyncEstablishedEvent &event
    )
    {
        JSONEventStream output;

        output << startObject <<
            key("type") << "event" <<
            key("name") << "periodic_advertising_sync_established" <<
            key("value") << startObject <<
                key("status") << event.getStatus();

        if (event.getStatus() == BLE_ERROR_NONE) {
            output <<
                key("peer_address_type") << event.getPeerAddressType() <<
                key("peer_address") << event.getPeerAddress() <<
                key("sync_handle") << event.getSyncHandle() <<
                key("advertising_interval") << event.getAdvertisingInterval() <<
                key("peer_phy") << event.getPeerPhy() <<
                key("sid") << event.getSid() <<
                key("peer_clock_accuracy") << ble::clock_accuracy_t(event.getPeerClockAccuracy()).get_ppm();
        }

        output <<
            endObject <<
        endObject;
    }

    virtual void onPeriodicAdvertisingReport(
        const ble::PeriodicAdvertisingReportEvent &event
    )
    {
        JSONEventStream() << startObject <<
            key("type") << "event" <<
            key("name") << "periodic_advertising_report" <<
            key("value") << startObject <<
                key("sync_handle") << event.getSyncHandle() <<
                key("rssi") << event.getRssi() <<
                key("tx_power") << event.getTxPower() <<
                key("data_status") << event.getDataStatus() <<
                key("data") << event.getPayload() <<
            endObject <<
        endObject;
    }

    virtual void onPeriodicAdvertisingSyncLoss(
        const ble::PeriodicAdvertisingSyncLoss &event
    )
    {
        JSONEventStream() << startObject <<
            key("type") << "event" <<
            key("name") << "periodic_advertising_sync_loss" <<
            key("value") << startObject <<
                key("sync_handle") << event.getSyncHandle() <<
            endObject <<
        endObject;
    }

    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event)
    {
        JSONEventStream os;

        os << startObject <<
            key("type") << "event" <<
            key("name") << "connection_complete" <<
            key("value");

        printConnectionResult(os, event);

        os << endObject;
    }

    virtual void onUpdateConnectionParametersRequest(
        const ble::UpdateConnectionParametersRequestEvent &event
    )
    {
        JSONEventStream() << startObject <<
            key("type") << "event" <<
            key("name") << "update_connection_parameters_request" <<
            key("value") << startObject <<
                key("connection_handle") << event.getConnectionHandle() <<
                key("min_connection_interval") << event.getMinConnectionInterval() <<
                key("max_connection_interval") << event.getMaxConnectionInterval() <<
                key("slave_latency") << event.getSlaveLatency().value() <<
                key("supervision_timeout") << event.getSupervisionTimeout() <<
            endObject <<
        endObject;
    }

    virtual void onConnectionParametersUpdateComplete(
        const ble::ConnectionParametersUpdateCompleteEvent &event
    )
    {
        JSONEventStream os;

        os << startObject <<
            key("type") << "event" <<
            key("name") << "on_connection_parameters_update_complete" <<
            key("value") << startObject <<
                key("connection_handle") << event.getConnectionHandle() <<
                key("status") << event.getStatus();

        if (event.getStatus() == BLE_ERROR_NONE) {
            os <<
                key("connection_interval") << event.getConnectionInterval() <<
                key("slave_latency") << event.getSlaveLatency().value() <<
                key("supervision_timeout") << event.getSupervisionTimeout();
        }

        os <<
            endObject <<
        endObject;
    }

    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
    {
        JSONEventStream() << startObject <<
            key("type") << "event" <<
            key("name") << "disconnection_complete" <<
            key("value") << startObject <<
                key("connection_handle") << event.getConnectionHandle() <<
                key("reason") << event.getReason() <<
            endObject <<
        endObject;

        disconnection_chain.call(event);
    }

    virtual void onReadPhy(
        ble_error_t status,
        ble::connection_handle_t connectionHandle,
        ble::phy_t txPhy,
        ble::phy_t rxPhy
    )
    {
        JSONEventStream os;
        os << startObject <<
            key("type") << "event" <<
            key("name") << "read_phy" <<
            key("value") << startObject <<
                key("connection_handle") << connectionHandle <<
                key("status") << status;
        if (status == BLE_ERROR_NONE) {
            os <<
                key("tx_phy") << txPhy <<
                key("rx_phy") << rxPhy;
        }
        os <<
            endObject <<
        endObject;
    }

    virtual void onPhyUpdateComplete(
        ble_error_t status,
        ble::connection_handle_t connectionHandle,
        ble::phy_t txPhy,
        ble::phy_t rxPhy
    )
    {
        JSONEventStream os;
        os << startObject <<
            key("type") << "event" <<
            key("name") << "phy_update_complete" <<
            key("value") << startObject <<
                key("connection_handle") << connectionHandle <<
                key("status") << status;
        if (status == BLE_ERROR_NONE) {
            os <<
                key("tx_phy") << txPhy <<
                key("rx_phy") << rxPhy;
        }
        os <<
            endObject <<
        endObject;
    }

    CallChainOfFunctionPointersWithContext<const ble::DisconnectionCompleteEvent&> disconnection_chain;
};

static EventHandler handler;

void enable_event_handling() {
    gap().setEventHandler(&handler);
}

DECLARE_CMD(GetAddressCommand) {
    CMD_NAME("getAddress")
    CMD_HELP(
        "Get the address and the type of address of this device."
    )

    CMD_RESULTS(
        CMD_RESULT("ble::own_address_type_t", "address_type", "Type of the address"),
        CMD_RESULT("ble::address_t", "address", "The address of the device")
    )

    CMD_HANDLER(CommandResponsePtr& response) {
        using namespace serialization;

        ble::own_address_type_t addressType;
        ble::address_t address;

        ble_error_t err = gap().getAddress(addressType, address);
        if(err) {
            response->faillure(err);
            return;
        }

        // building the result object
        response->success();
        response->getResultStream() << startObject <<
            key("address_type") << addressType <<
            key("address") << address <<
        endObject;
    }
};

DECLARE_CMD(GetMaxWhitelistSizeCommand) {
    CMD_NAME("getMaxWhitelistSize")
    CMD_HELP("get the maximum size the whitelist can take")

    CMD_HANDLER(CommandResponsePtr& response) {
        response->success(gap().getMaxWhitelistSize());
    }
};

DECLARE_CMD(GetWhitelistCommand) {
    CMD_NAME("getWhitelist")
    CMD_HELP("Get the internal whitelist to be used by the Link Layer when scanning,"
               "advertising or initiating a connection depending on the filter policies.")

    CMD_RESULTS(
        CMD_RESULT("JSON Array", "", "Array of the address in the whitelist"),
        CMD_RESULT("JSON Object", "[i]", "Description of an address"),
        CMD_RESULT("AddressType_t", "[i].address_type", "Type of the address"),
        CMD_RESULT("MacAddress_t", "[i].address", "The mac address"),
    )

    CMD_HANDLER(CommandResponsePtr& response) {
        using namespace serialization;

        ble::whitelist_t::entry_t* addresses = new ble::whitelist_t::entry_t[gap().getMaxWhitelistSize()]();
        ble::whitelist_t whiteList = {
            addresses,
            /* size */ 0,
            /* capacity */ gap().getMaxWhitelistSize()
        };

        ble_error_t err = gap().getWhitelist(whiteList);
        if(err) {
            response->faillure(err);
            delete[] addresses;
            return;
        }

        response->success();
        serialization::JSONOutputStream& os = response->getResultStream();

        os << startArray;
        for(std::size_t i = 0; i < whiteList.size; ++i) {
            os << startObject <<
                key("address_type") << whiteList.addresses[i].type <<
                key("address") << whiteList.addresses[i].address <<
            endObject;
        }
        os << endArray;

        delete[] addresses;
    }
};

DECLARE_CMD(SetWhitelistCommand) {
    CMD_NAME("setWhitelist")
    CMD_HELP("Set the internal whitelist to be used by the Link Layer when scanning,"
               "advertising or initiating a connection depending on the filter policies.")

    template<typename T>
    static std::size_t maximumArgsRequired() {
        return 0xFF;
    }

    CMD_HANDLER(const CommandArgs& args, CommandResponsePtr& response) {
        if(args.count() % 2) {
            response->invalidParameters("[ <addressType> <address> ] expected");
            return;
        }

        uint8_t addressCount = args.count() / 2;
        ble::whitelist_t::entry_t* addresses = new ble::whitelist_t::entry_t[addressCount]();

        // fill the input
        for(uint8_t i = 0; i < addressCount; ++i) {
            if(!fromString(args[i * 2], addresses[i].type)) {
                response->invalidParameters("invalid address type");
                delete[] addresses;
                return;
            }

            if(!macAddressFromString(args[(i * 2) + 1], addresses[i].address)) {
                response->invalidParameters("invalid address");
                delete[] addresses;
                return;
            }
        }

        ble::whitelist_t whiteList = {
            addresses,
            /* size */ addressCount,
            /* capacity */ addressCount
        };
        ble_error_t err = gap().setWhitelist(whiteList);
        delete[] addresses;

        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(EnablePrivacyCommand) {
    CMD_NAME("enablePrivacy")
    CMD_HELP("Enable or disable the privacy")

    CMD_ARGS(
        CMD_ARG("bool", "enable", "Enable or disable the privacy")
    )

    CMD_HANDLER(bool enable, CommandResponsePtr& response)
    {
        if (enable) {
            startProcedure<EnablePrivacyProcedure>(response, 2000 /* timeout in ms */);
        } else {
            reportErrorOrSuccess(response, gap().enablePrivacy(false));
        }
    }

    struct EnablePrivacyProcedure : public AsyncProcedure, Gap::EventHandler {
        EnablePrivacyProcedure(
            const SharedPointer<CommandResponse>& response,
            uint32_t procedureTimeout
        ) : AsyncProcedure(response, procedureTimeout)
        {
        }

        virtual ~EnablePrivacyProcedure()
        {
            // revert to default event handler
            enable_event_handling();
        }

        virtual bool doStart()
        {
            gap().setEventHandler(this);
            ble_error_t result = gap().enablePrivacy(true);
            if (result != BLE_ERROR_NONE) {
                reportErrorOrSuccess(
                    response,
                    result
                );
                return false;
            }
            return true;
        }

        void onPrivacyEnabled()
        {
            response->success();
            terminate();
        }

        virtual void doWhenTimeout() {
            response->faillure(BLE_ERROR_INTERNAL_STACK_FAILURE);
            terminate();
        }
    };

};

DECLARE_CMD(SetPeripheralPrivacyConfigurationCommand) {
    CMD_NAME("setPeripheralPrivacyConfiguration")
    CMD_HELP("Set the peripheral privacy configuration.")

    CMD_ARGS(
        CMD_ARG("bool", "use_non_resolvable_random_address", "Use non resolvable address in non connectable advertisements"),
        CMD_ARG("ble::peripheral_privacy_configuration_t::resolution_strategy_t", "resolution_strategy", "Strategy used to resolve addresses present in scan and connection requests.")
    )

    CMD_HANDLER(
        bool use_non_resolvable_random_address,
        ble::peripheral_privacy_configuration_t::resolution_strategy_t& resolution_strategy,
        CommandResponsePtr& response
    ) {
        ble::peripheral_privacy_configuration_t configuration = {
            use_non_resolvable_random_address,
            resolution_strategy
        };

        reportErrorOrSuccess(
            response,
            gap().setPeripheralPrivacyConfiguration(&configuration)
        );
    }
};

DECLARE_CMD(GetPeripheralPrivacyConfigurationCommand) {
    CMD_NAME("getPeripheralPrivacyConfiguration")
    CMD_HELP("Get the peripheral privacy configuration.")

    CMD_RESULTS(
        CMD_RESULT("bool", "use_non_resolvable_random_address", "Indicates if non resolvable addresses are used in non connectable advertisements."),
        CMD_RESULT("ble::peripheral_privacy_configuration_t::resolution_strategy_t", "resolution_strategy", "Strategy used to resolve address in scan and connection requests."),
    )

    CMD_HANDLER(CommandResponsePtr& response) {
        ble::peripheral_privacy_configuration_t configuration;

        reportErrorOrSuccess(
            response,
            gap().getPeripheralPrivacyConfiguration(&configuration),
            configuration
        );
    }
};

DECLARE_CMD(SetCentralPrivacyConfigurationCommand) {
    CMD_NAME("setCentralPrivacyConfiguration")
    CMD_HELP("Set the central privacy configuration.")

    CMD_ARGS(
        CMD_ARG("bool", "use_non_resolvable_random_address", "Use non resolvable address in scan requests."),
        CMD_ARG("ble::central_privacy_configuration_t::resolution_strategy_t", "resolution_strategy", "Strategy used to resolve addresses present in advertisement packets.")
    )

    CMD_HANDLER(
        bool use_non_resolvable_random_address,
        ble::central_privacy_configuration_t::resolution_strategy_t& resolution_strategy,
        CommandResponsePtr& response
    ) {
        ble::central_privacy_configuration_t configuration = {
            use_non_resolvable_random_address,
            resolution_strategy
        };

        reportErrorOrSuccess(
            response,
            gap().setCentralPrivacyConfiguration(&configuration)
        );
    }
};

DECLARE_CMD(GetCentralPrivacyConfigurationCommand) {
    CMD_NAME("getCentralPrivacyConfiguration")
    CMD_HELP("Get the central privacy configuration.")

    CMD_RESULTS(
        CMD_RESULT("bool", "use_non_resolvable_random_address", "Indicates if non resolvable addresses are used in scan request."),
        CMD_RESULT("ble::central_privacy_configuration_t::resolution_strategy_t", "resolution_strategy", "Strategy used to resolve addresses in advertisements."),
    )

    CMD_HANDLER(CommandResponsePtr& response) {
        ble::central_privacy_configuration_t configuration;

        reportErrorOrSuccess(
            response,
            gap().getCentralPrivacyConfiguration(&configuration),
            configuration
        );
    }
};

DECLARE_CMD(ReadPhyCommand) {
    CMD_NAME("readPhy")
    CMD_HELP("Read current PHY of the connection.")

    CMD_ARGS(
        CMD_ARG("uint16_t", "handle", "The handle of the connection queried")
    )

    CMD_HANDLER(
        ble::connection_handle_t handle,
        CommandResponsePtr& response
    ) {
        startProcedure<ReadPhyProcedure>(handle, response, 1000 /* timeout in ms */);
    }

    struct ReadPhyProcedure : public AsyncProcedure, Gap::EventHandler {
        ReadPhyProcedure(
            ble::connection_handle_t connectionHandle,
            const SharedPointer<CommandResponse>& response,
            uint32_t procedureTimeout
        ) : AsyncProcedure(response, procedureTimeout), handle(connectionHandle) { }

        virtual ~ReadPhyProcedure() {
            // revert to default event handler
            enable_event_handling();
        }

        virtual bool doStart() {
            gap().setEventHandler(this);
            ble_error_t result = gap().readPhy(handle);
            if (result != BLE_ERROR_NONE) {
                reportErrorOrSuccess(
                    response,
                    result
                );
                return false;
            }
            return true;
        }

        void onReadPhy(
            ble_error_t status,
            ble::connection_handle_t connectionHandle,
            ble::phy_t txPhy,
            ble::phy_t rxPhy
        ) {
            serialization::JSONOutputStream& os = response->getResultStream();

            response->success();

            os << serialization::startObject <<
                serialization::key("handle") << connectionHandle <<
                serialization::key("txPhy") << txPhy <<
                serialization::key("rxPhy") << rxPhy <<
            serialization::endObject;

            terminate();
        }

        ble::connection_handle_t handle;
    };
};

DECLARE_CMD(SetPhyCommand) {
    CMD_NAME("setPhy")
    CMD_HELP("Set PHY preference for given connection.")

    CMD_ARGS(
        CMD_ARG("uint16_t", "handle", "The handle of the connection queried"),
        CMD_ARG("uint8_t", "tx_phy", "Preferred tx PHYs mask"),
        CMD_ARG("uint8_t", "rx_phy", "Preferred rx PHYs mask"),
        CMD_ARG("uint8_t", "coded_symbol", "Preferred types of coding")
    )

    CMD_HANDLER(
        ble::connection_handle_t handle,
        uint8_t tx_phy,
        uint8_t rx_phy,
        uint8_t coded_symbol,
        CommandResponsePtr& response
    ) {
        reportErrorOrSuccess(
            response,
            gap().setPhy(
                handle,
                (ble::phy_set_t*)&tx_phy,
                (ble::phy_set_t*)&rx_phy,
                (ble::coded_symbol_per_bit_t::type)coded_symbol
            )
        );
    }
};

DECLARE_CMD(SetPreferredPhysCommand) {
    CMD_NAME("setPreferredPhys")
    CMD_HELP("Set PHY preference for all connections.")

    CMD_ARGS(
        CMD_ARG("uint8_t", "tx phy", "Preferred tx PHYs mask"),
        CMD_ARG("uint8_t", "rx phy", "Preferred rx PHYs mask")
    )

    CMD_HANDLER(
        uint8_t tx_phy,
        uint8_t rx_phy,
        CommandResponsePtr& response
    ) {
        reportErrorOrSuccess(
            response,
            gap().setPreferredPhys(
                (ble::phy_set_t*)&tx_phy,
                (ble::phy_set_t*)&rx_phy
            )
        );
    }
};

DECLARE_CMD(GetMaxAdvertisingSetNumber) {
    CMD_NAME("getMaxAdvertisingSetNumber")
    CMD_HANDLER(CommandResponsePtr& response) {
        response->success(gap().getMaxAdvertisingSetNumber());
    }
};

DECLARE_CMD(GetMaxAdvertisingDataLength) {
    CMD_NAME("getMaxAdvertisingDataLength")
    CMD_HANDLER(CommandResponsePtr& response) {
        response->success(gap().getMaxAdvertisingDataLength());
    }
};

DECLARE_CMD(CreateAdvertisingSet) {
    CMD_NAME("createAdvertisingSet")
    CMD_HANDLER(CommandResponsePtr& response) {
        ble::advertising_handle_t handle;
        ble_error_t err = gap().createAdvertisingSet(&handle, getAdvertisingParameters());
        reportErrorOrSuccess(response, err, handle);
    }
};

DECLARE_CMD(DestroyAdvertisingSet) {
    CMD_NAME("destroyAdvertisingSet")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        ble_error_t err = gap().destroyAdvertisingSet(handle);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetAdvertisingParameters) {
    CMD_NAME("setAdvertisingParameters")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        ble_error_t err = gap().setAdvertisingParameters(handle, getAdvertisingParameters());
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetAdvertisingPayload) {
    CMD_NAME("setAdvertisingPayload")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", ""),
        CMD_ARG("RawData_t", "data", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = gap().setAdvertisingPayload(handle, mbed::make_Span(data.cbegin(), data.size()));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(ApplyAdvPayloadFromBuilder) {
    CMD_NAME("applyAdvPayloadFromBuilder")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "Advertising set (legacy = 0) to apply builder data to.")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        ble_error_t err = gap().setAdvertisingPayload(handle, AdvertisingDataBuilderCommandSuiteDescription::get());
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetAdvertisingScanResponse) {
    CMD_NAME("setAdvertisingScanResponse")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", ""),
        CMD_ARG("RawData_t", "data", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = gap().setAdvertisingScanResponse(handle, mbed::make_Span(data.cbegin(), data.size()));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(ApplyScanRespFromBuilder) {
    CMD_NAME("applyScanRespFromBuilder")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "Advertising set (legacy = 0) to apply builder data to.")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        ble_error_t err = gap().setAdvertisingScanResponse(handle, AdvertisingDataBuilderCommandSuiteDescription::get());
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(StartAdvertising) {
    CMD_NAME("startAdvertising")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", ""),
        CMD_ARG("ble::adv_duration_t", "duration", ""),
        CMD_ARG("uint8_t", "maxEvent", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, ble::adv_duration_t duration, uint8_t maxEvents, CommandResponsePtr& response) {
        startProcedure<StartAdvertisingProcedure>(handle, duration, maxEvents, response);
    }

    struct StartAdvertisingProcedure: public AsyncProcedure, Gap::EventHandler {
        StartAdvertisingProcedure(
            ble::advertising_handle_t handle,
            ble::adv_duration_t duration,
            uint8_t maxEvents,
            CommandResponsePtr& response
        ) : AsyncProcedure(response, 1000),
            _handle(handle), _duration(duration), _maxEvents(maxEvents)
        {
            gap().setEventHandler(this);
        }

        // AsyncProcedure implementation

        ~StartAdvertisingProcedure() override {
            // revert to default event handler
            enable_event_handling();
        }

        bool doStart() override {
            ble_error_t err = gap().startAdvertising(_handle, _duration, _maxEvents);
            if (err != BLE_ERROR_NONE) {
                response->faillure(err);
                return false;
            }
            return true;
        }

        // Gap::EventHandler implementation

        void onAdvertisingStart(const ble::AdvertisingStartEvent &event) override
        {
            if (event.getAdvHandle() != _handle) {
                return;
            }

            response->success();
            response->getResultStream() << startObject <<
                key("handle") << event.getAdvHandle() <<
            endObject;

            terminate();
        }

    private:
        ble::advertising_handle_t _handle;
        ble::adv_duration_t _duration;
        uint8_t _maxEvents;
    };
};

DECLARE_CMD(StopAdvertising) {
    CMD_NAME("stopAdvertising")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        startProcedure<StopAdvertisingProcedure>(handle, response);
    }

    struct StopAdvertisingProcedure: public AsyncProcedure, Gap::EventHandler {
        StopAdvertisingProcedure(
            ble::advertising_handle_t handle,
            CommandResponsePtr& response
        ) : AsyncProcedure(response, 1000), _handle(handle)
        {
            gap().setEventHandler(this);
        }

        // AsyncProcedure implementation

        ~StopAdvertisingProcedure() override {
            // revert to default event handler
            enable_event_handling();
        }

        bool doStart() override {
            ble_error_t err = gap().stopAdvertising(_handle);
            if (err != BLE_ERROR_NONE) {
                response->faillure(err);
                return false;
            }
            return true;
        }

        // Gap::EventHandler implementation

        void onAdvertisingEnd(const ble::AdvertisingEndEvent &event) override
        {
            if (event.getAdvHandle() != _handle) {
                return;
            }

            response->success();
            response->getResultStream() << startObject <<
                key("handle") << event.getAdvHandle() <<
                endObject;

            terminate();
        }

    private:
        ble::advertising_handle_t _handle;
    };
};

DECLARE_CMD(IsAdvertisingActive) {
    CMD_NAME("isAdvertisingActive")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        bool active = gap().isAdvertisingActive(handle);
        response->success(active);
    }
};

DECLARE_CMD(SetPeriodicAdvertisingParameters) {
    CMD_NAME("setPeriodicAdvertisingParameters")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", ""),
        CMD_ARG("ble::periodic_interval_t", "min", ""),
        CMD_ARG("ble::periodic_interval_t", "max", ""),
        CMD_ARG("bool", "advertiseTxPower", "")
    )
    CMD_HANDLER(
        ble::advertising_handle_t handle,
        ble::periodic_interval_t min,
        ble::periodic_interval_t max,
        bool advertiseTxPower,
        CommandResponsePtr& response
    )
    {
        ble_error_t err = gap().setPeriodicAdvertisingParameters(
            handle,
            min,
            max,
            advertiseTxPower
        );

        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(SetPeriodicAdvertisingPayload) {
    CMD_NAME("setPeriodicAdvertisingPayload")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", ""),
        CMD_ARG("RawData_t", "data", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, RawData_t& data, CommandResponsePtr& response) {
        ble_error_t err = gap().setPeriodicAdvertisingPayload(handle, mbed::make_Span(data.cbegin(), data.size()));
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(StartPeriodicAdvertising) {
    CMD_NAME("startPeriodicAdvertising")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        ble_error_t err = gap().startPeriodicAdvertising(handle);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(StopPeriodicAdvertising) {
    CMD_NAME("stopPeriodicAdvertising")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        ble_error_t err = gap().stopPeriodicAdvertising(handle);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(IsPeriodicAdvertisingActive) {
    CMD_NAME("isPeriodicAdvertisingActive")
    CMD_ARGS(
        CMD_ARG("ble::advertising_handle_t", "handle", "")
    )
    CMD_HANDLER(ble::advertising_handle_t handle, CommandResponsePtr& response) {
        bool active = gap().isPeriodicAdvertisingActive(handle);
        response->success(active);
    }
};

DECLARE_CMD(SetScanParameters) {
    CMD_NAME("setScanParameters")
    CMD_HANDLER(CommandResponsePtr& response) {
        ble_error_t err = gap().setScanParameters(getScanParameters());
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(StartScan) {
    CMD_NAME("startScan")
    CMD_ARGS(
        CMD_ARG("ble::scan_duration_t", "duration", ""),
        CMD_ARG("ble::duplicates_filter_t::type", "filter", ""),
        CMD_ARG("ble::scan_period_t", "period", ""),
    )
    CMD_HANDLER(
        ble::scan_duration_t duration,
        ble::duplicates_filter_t::type filter,
        ble::scan_period_t period,
        CommandResponsePtr& response
    )
    {
        ble_error_t err = gap().startScan(duration, filter, period);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(ScanForAddress) {
    CMD_NAME("scanForAddress")
    CMD_ARGS(
        CMD_ARG("ble::address_t", "peer_address", ""),
        CMD_ARG("uint32_t", "timeout", "")
    )
    CMD_HANDLER(
        ble::address_t peer_address,
        uint32_t timeout,
        CommandResponsePtr& response
    ) {
        startProcedure<ScanForAddressProcedure>(peer_address, timeout, response);
    }

    struct ScanForAddressProcedure: public AsyncProcedure, Gap::EventHandler {
        ScanForAddressProcedure(
            ble::address_t peer_address,
            uint32_t timeout,
            CommandResponsePtr& response
        ) : AsyncProcedure(response, timeout), peer_address(peer_address)
        {
            gap().setEventHandler(this);
            ble_error_t err = gap().startScan();
            if (err != BLE_ERROR_NONE) {
                response->faillure(err);
                terminate();
            }

            timer.reset();
            timer.start();

            // Define success = scan started. Results (can be empty) will be in an array
            response->success();
            response->getResultStream() << startArray;
        }

        // AsyncProcedure implementation

        virtual ~ScanForAddressProcedure(){
            // revert to default event handler
            enable_event_handling();
        }

        virtual bool doStart() {
            return true;
        }

        virtual void doWhenTimeout() {
            timer.stop();
            gap().stopScan();
            response->getResultStream() << endArray;
        }

        // Gap::EventHandler implementation

        virtual void onAdvertisingReport(const ble::AdvertisingReportEvent &event)
        {
            if (event.getPeerAddress() != peer_address) {
                return;
            }

            JSONOutputStream& os = response->getResultStream();

            os << startObject <<
                key("time") << (uint32_t) std::chrono::duration_cast<std::chrono::milliseconds>(timer.elapsed_time()).count() <<
                key("connectable") << event.getType().connectable() <<
                key("scannable") << event.getType().scannable_advertising() <<
                key("scan_response") << event.getType().scan_response() <<
                key("directed") << event.getType().directed_advertising() <<
                key("legacy") << event.getType().legacy_advertising() <<
                key("rssi") << event.getRssi() <<
                key("peer_address_type") << event.getPeerAddressType();

            if (event.getPeerAddressType() != ble::peer_address_type_t::ANONYMOUS) {
                os << key("peer_address") << event.getPeerAddress();
            }

            if (event.getType().directed_advertising()) {
                os << key("direct_address_type") << event.getDirectAddressType() <<
                        key("direct_address") << event.getDirectAddress();
            }

            if (event.getType().legacy_advertising() == false) {
                os << key("sid") << event.getSID() <<
                        key("tx_power") << event.getTxPower() <<
                        key("primary_phy") << event.getPrimaryPhy() <<
                        key("secondary_phy") << event.getSecondaryPhy() <<
                        key("data_status");

                switch (event.getType().data_status().value()) {
                    case ble::advertising_data_status_t::COMPLETE:
                        os << "COMPLETE";
                        break;
                    case ble::advertising_data_status_t::INCOMPLETE_MORE_DATA:
                        os << "INCOMPLETE_MORE_DATA";
                        break;
                    case ble::advertising_data_status_t::INCOMPLETE_DATA_TRUNCATED:
                        os << "INCOMPLETE_DATA_TRUNCATED";
                        break;
                    default:
                        os << "unknown";
                        break;
                }

                if (event.isPeriodicIntervalPresent()) {
                    os << key("periodic_interval") << event.getPeriodicInterval();
                }
            }

            os <<   key("payload") << event.getPayload() <<
            endObject;
        }

    private:
        ble::address_t peer_address;
        mbed::Timer timer;
    };
};

DECLARE_CMD(ScanForData) {
    CMD_NAME("scanForData")
    CMD_ARGS(
        CMD_ARG("RawData_t", "data", ""),
        CMD_ARG("uint32_t", "timeout", "")
    )
    CMD_HANDLER(
        container::Vector<uint8_t>& data,
        uint32_t timeout,
        CommandResponsePtr& response
    ) {
        startProcedure<ScanForDataProcedure>(data, timeout, response);
    }

    struct ScanForDataProcedure: public AsyncProcedure, Gap::EventHandler {
        ScanForDataProcedure(
            container::Vector<uint8_t> data,
            uint32_t timeout,
            CommandResponsePtr& response
        ) : AsyncProcedure(response, timeout), _data(data)
        {
            gap().setEventHandler(this);
            ble_error_t err = gap().startScan();
            if (err != BLE_ERROR_NONE) {
                response->faillure(err);
                terminate();
            }

            timer.reset();
            timer.start();

            // Define success = scan started. Results (can be empty) will be in an array
            response->success();
            response->getResultStream() << startArray;
        }

        // AsyncProcedure implementation

        virtual ~ScanForDataProcedure(){
            // revert to default event handler
            enable_event_handling();
        }

        virtual bool doStart() {
            return true;
        }

        virtual void doWhenTimeout() {
            timer.stop();
            gap().stopScan();
            response->getResultStream() << endArray;
        }

        // Gap::EventHandler implementation

        virtual void onAdvertisingReport(const ble::AdvertisingReportEvent &event)
        {
            if (memcmp(event.getPayload().data(), _data.begin(), _data.size())) {
                return;
            }

            JSONOutputStream& os = response->getResultStream();

            os << startObject <<
               key("time") << (int32_t) timer.read_ms() <<
               key("connectable") << event.getType().connectable() <<
               key("scannable") << event.getType().scannable_advertising() <<
               key("scan_response") << event.getType().scan_response() <<
               key("directed") << event.getType().directed_advertising() <<
               key("legacy") << event.getType().legacy_advertising() <<
               key("rssi") << event.getRssi() <<
               key("peer_address_type") << event.getPeerAddressType();

            if (event.getPeerAddressType() != ble::peer_address_type_t::ANONYMOUS) {
                os << key("peer_address") << event.getPeerAddress();
            }

            if (event.getType().directed_advertising()) {
                os << key("direct_address_type") << event.getDirectAddressType() <<
                   key("direct_address") << event.getDirectAddress();
            }

            if (event.getType().legacy_advertising() == false) {
                os << key("sid") << event.getSID() <<
                   key("tx_power") << event.getTxPower() <<
                   key("primary_phy") << event.getPrimaryPhy() <<
                   key("secondary_phy") << event.getSecondaryPhy() <<
                   key("data_status");

                switch (event.getType().data_status().value()) {
                    case ble::advertising_data_status_t::COMPLETE:
                        os << "COMPLETE";
                        break;
                    case ble::advertising_data_status_t::INCOMPLETE_MORE_DATA:
                        os << "INCOMPLETE_MORE_DATA";
                        break;
                    case ble::advertising_data_status_t::INCOMPLETE_DATA_TRUNCATED:
                        os << "INCOMPLETE_DATA_TRUNCATED";
                        break;
                    default:
                        os << "unknown";
                        break;
                }

                if (event.isPeriodicIntervalPresent()) {
                    os << key("periodic_interval") << event.getPeriodicInterval();
                }
            }

            os <<   key("payload") << event.getPayload() <<
               endObject;
        }

    private:
        container::Vector<uint8_t> _data;
        mbed::Timer timer;
    };
};

DECLARE_CMD(StopScan) {
    CMD_NAME("stopScan")
    CMD_HANDLER(CommandResponsePtr& response) {
        ble_error_t err = gap().stopScan();
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(CreateSync) {
    CMD_NAME("createSync")
    CMD_ARGS(
        CMD_ARG("ble::peer_address_type_t::type", "peerAddressType", ""),
        CMD_ARG("ble::address_t", "peerAddress", ""),
        CMD_ARG("uint8_t", "sid", ""),
        CMD_ARG("uint16_t", "maxPacketSkip", ""),
        CMD_ARG("ble::sync_timeout_t", "timeout", ""),
    )
    CMD_HANDLER(
        ble::peer_address_type_t::type peerAddressType,
        ble::address_t &peerAddress,
        uint8_t sid,
        uint16_t maxPacketSkip,
        ble::sync_timeout_t timeout,
        CommandResponsePtr& response
    )
    {
        ble_error_t err = gap().createSync(
            peerAddressType,
            peerAddress,
            sid,
            maxPacketSkip,
            timeout
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(CreateSyncFromList) {
    CMD_NAME("createSyncFromList")
    CMD_ARGS(
        CMD_ARG("uint16_t", "maxPacketSkip", ""),
        CMD_ARG("ble::sync_timeout_t", "timeout", ""),
    )
    CMD_HANDLER(
        uint16_t maxPacketSkip,
        ble::sync_timeout_t timeout,
        CommandResponsePtr& response
    )
    {
        ble_error_t err = gap().createSync(
            maxPacketSkip,
            timeout
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(CancelCreateSync) {
    CMD_NAME("cancelCreateSync")
    CMD_HANDLER(CommandResponsePtr& response) {
        ble_error_t err = gap().cancelCreateSync();
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(TerminateSync) {
    CMD_NAME("terminateSync")
    CMD_ARGS(
        CMD_ARG("ble::periodic_sync_handle_t", "handle", "")
    )
    CMD_HANDLER(ble::periodic_sync_handle_t handle, CommandResponsePtr& response) {
        ble_error_t err = gap().terminateSync(handle);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(AddDeviceToPeriodicAdvertiserList) {
    CMD_NAME("addDeviceToPeriodicAdvertiserList")
    CMD_ARGS(
        CMD_ARG("ble::peer_address_type_t::type", "peerAddressType", ""),
        CMD_ARG("ble::address_t", "peerAddress", ""),
        CMD_ARG("ble::advertising_sid_t", "sid", ""),
    )
    CMD_HANDLER(
        ble::peer_address_type_t::type peerAddressType,
        ble::address_t &peerAddress,
        ble::advertising_sid_t sid,
        CommandResponsePtr& response
    ) {
        ble_error_t err = gap().addDeviceToPeriodicAdvertiserList(
            peerAddressType,
            peerAddress,
            sid
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(RemoveDeviceFromPeriodicAdvertiserList) {
    CMD_NAME("removeDeviceFromPeriodicAdvertiserList")
    CMD_ARGS(
        CMD_ARG("ble::peer_address_type_t::type", "peerAddressType", ""),
        CMD_ARG("ble::address_t", "peerAddress", ""),
        CMD_ARG("ble::advertising_sid_t", "sid", ""),
    )
    CMD_HANDLER(
        ble::peer_address_type_t::type peerAddressType,
        ble::address_t &peerAddress,
        ble::advertising_sid_t sid,
        CommandResponsePtr& response
    ) {
        ble_error_t err = gap().removeDeviceFromPeriodicAdvertiserList(
            peerAddressType,
            peerAddress,
            sid
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(ClearPeriodicAdvertiserList) {
    CMD_NAME("clearPeriodicAdvertiserList")
    CMD_HANDLER(CommandResponsePtr& response) {
        ble_error_t err = gap().clearPeriodicAdvertiserList();
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(GetMaxPeriodicAdvertiserListSize) {
    CMD_NAME("getMaxPeriodicAdvertiserListSize")
    CMD_HANDLER(CommandResponsePtr& response) {
        response->success(gap().getMaxPeriodicAdvertiserListSize());
    }
};

DECLARE_CMD(StartConnecting) {
    CMD_NAME("startConnecting")
    CMD_ARGS(
        CMD_ARG("ble::peer_address_type_t::type", "peerAddressType", ""),
        CMD_ARG("ble::address_t", "peerAddress", "")
    )
    CMD_HANDLER(
        ble::peer_address_type_t::type peerAddressType,
        ble::address_t &peerAddress,
        CommandResponsePtr& response
    ) {
        ble_error_t err = gap().connect(peerAddressType, peerAddress, getConnectionParameters());
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(Connect) {
    CMD_NAME("connect")
    CMD_ARGS(
        CMD_ARG("ble::peer_address_type_t::type", "peerAddressType", ""),
        CMD_ARG("ble::address_t", "peerAddress", "")
    )
    CMD_HANDLER(
        ble::peer_address_type_t::type peerAddressType,
        ble::address_t &peerAddress,
        CommandResponsePtr& response
    ) {
        startProcedure<ConnectProcedure>(
            peerAddressType, peerAddress, response, 40 * 1000 /* ms */
        );
    }

    struct ConnectProcedure : public AsyncProcedure, Gap::EventHandler {
        ConnectProcedure(
            ble::peer_address_type_t::type peerAddressType,
            ble::address_t peerAddress,
            CommandResponsePtr& response,
            uint32_t procedureTimeout
        ) : AsyncProcedure(response, procedureTimeout)
        {
            gap().setEventHandler(this);
            ble_error_t err = gap().connect(peerAddressType, peerAddress, getConnectionParameters());
            if (err != BLE_ERROR_NONE) {
                response->faillure(err);
                terminate();
            }
        }

        virtual ~ConnectProcedure() {
            // revert to default event handler
            enable_event_handling();
        }

        // AsyncProcedure implementation
        virtual bool doStart() {
            return true;
        }

        // Gap::EventHandler implementation
        virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event)
        {
            if (event.getStatus() != BLE_ERROR_NONE) {
                response->faillure(event.getStatus());
                terminate();
                return;
            }

            response->success();
            serialization::JSONOutputStream& os = response->getResultStream();
            printConnectionResult(os, event);
            terminate();
        }
    };
};

DECLARE_CMD(WaitForConnection) {
    CMD_NAME("waitForConnection")
    CMD_ARGS(
        CMD_ARG("uint32_t", "timeout", "")
    )
    CMD_HANDLER(
        uint32_t timeout,
        CommandResponsePtr& response
    ) {
        startProcedure<waitForConnectionProcedure>(
            response, timeout /* ms */
        );
    }

    struct waitForConnectionProcedure : public AsyncProcedure, Gap::EventHandler {
        waitForConnectionProcedure(
            CommandResponsePtr& response,
            uint32_t procedureTimeout
        ) : AsyncProcedure(response, procedureTimeout)
        {
            gap().setEventHandler(this);
        }

        virtual ~waitForConnectionProcedure() {
            // revert to default event handler
            enable_event_handling();
        }

        // AsyncProcedure implementation
        virtual bool doStart() {
            return true;
        }

        // Gap::EventHandler implementation
        virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event)
        {
            if (event.getStatus() != BLE_ERROR_NONE) {
                response->faillure(event.getStatus());
                terminate();
                return;
            }

            response->success();
            serialization::JSONOutputStream& os = response->getResultStream();
            printConnectionResult(os, event);
            terminate();
        }
    };
};



DECLARE_CMD(WaitForDisconnection) {
    CMD_NAME("waitForDisconnection")
    CMD_ARGS(
        CMD_ARG("uint32_t", "timeout", "")
    )
    CMD_HANDLER(
        uint32_t timeout,
        CommandResponsePtr& response
    ) {
        startProcedure<waitForDisconnectionProcedure>(
            response, timeout /* ms */
        );
    }

    struct waitForDisconnectionProcedure : public AsyncProcedure, Gap::EventHandler {
        waitForDisconnectionProcedure(
            CommandResponsePtr& response,
            uint32_t procedureTimeout
        ) : AsyncProcedure(response, procedureTimeout)
        {
            gap().setEventHandler(this);
        }

        virtual ~waitForDisconnectionProcedure() {
            // revert to default event handler
            enable_event_handling();
        }

        // AsyncProcedure implementation
        virtual bool doStart() {
            return true;
        }

        // Gap::EventHandler implementation
        virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
        {
            response->success();
            serialization::JSONOutputStream& os = response->getResultStream();
            printDisconnectionResult(os, event);
            terminate();
        }
    };
};

DECLARE_CMD(CancelConnect) {
    CMD_NAME("cancelConnect")
    CMD_HANDLER(
        CommandResponsePtr& response) {
        ble_error_t err = gap().cancelConnect();
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(UpdateConnectionParameters) {
    CMD_NAME("updateConnectionParameters")
    CMD_ARGS(
        CMD_ARG("ble::connection_handle_t", "handle", ""),
        CMD_ARG("ble::conn_interval_t", "minConnectionInterval", ""),
        CMD_ARG("ble::conn_interval_t", "maxConnectionInterval", ""),
        CMD_ARG("uint16_t", "slaveLatency", ""),
        CMD_ARG("ble::supervision_timeout_t", "supervision_timeout", "")
    )
    CMD_HANDLER(
        ble::connection_handle_t connectionHandle,
        ble::conn_interval_t minConnectionInterval,
        ble::conn_interval_t maxConnectionInterval,
        uint16_t slaveLatency,
        ble::supervision_timeout_t supervision_timeout,
        CommandResponsePtr& response
    ) {
        ble_error_t err = gap().updateConnectionParameters(
            connectionHandle,
            minConnectionInterval,
            maxConnectionInterval,
            slaveLatency,
            supervision_timeout
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(ManageConnectionParametersUpdateRequest) {
    CMD_NAME("manageConnectionParametersUpdateRequest")
    CMD_ARGS(
        CMD_ARG("bool", "manage", "")
    )
    CMD_HANDLER(bool manage, CommandResponsePtr& response) {
        ble_error_t err = gap().manageConnectionParametersUpdateRequest(manage);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(AcceptConnectionParametersUpdate) {
    CMD_NAME("acceptConnectionParametersUpdate")
    CMD_ARGS(
        CMD_ARG("ble::connection_handle_t", "handle", ""),
        CMD_ARG("ble::conn_interval_t", "minConnectionInterval", ""),
        CMD_ARG("ble::conn_interval_t", "maxConnectionInterval", ""),
        CMD_ARG("uint16_t", "slaveLatency", ""),
        CMD_ARG("ble::supervision_timeout_t", "supervision_timeout", "")
    )
    CMD_HANDLER(
        ble::connection_handle_t connectionHandle,
        ble::conn_interval_t minConnectionInterval,
        ble::conn_interval_t maxConnectionInterval,
        uint16_t slaveLatency,
        ble::supervision_timeout_t supervision_timeout,
        CommandResponsePtr& response
    ) {
        ble_error_t err = gap().acceptConnectionParametersUpdate(
            connectionHandle,
            minConnectionInterval,
            maxConnectionInterval,
            slaveLatency,
            supervision_timeout
        );
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(RejectConnectionParametersUpdate) {
    CMD_NAME("rejectConnectionParametersUpdate")
    CMD_ARGS(
        CMD_ARG("ble::connection_handle_t", "handle", ""),
    )
    CMD_HANDLER(ble::connection_handle_t handle, CommandResponsePtr& response) {
        ble_error_t err = gap().rejectConnectionParametersUpdate(handle);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(Disconnect) {
    CMD_NAME("disconnect")
    CMD_ARGS(
        CMD_ARG("ble::connection_handle_t", "handle", ""),
        CMD_ARG("ble::local_disconnection_reason_t::type", "reason", ""),
    )
    CMD_HANDLER(
        ble::connection_handle_t handle,
        ble::local_disconnection_reason_t::type reason,
        CommandResponsePtr& response
    ) {
        ble_error_t err = gap().disconnect(handle, reason);
        reportErrorOrSuccess(response, err);
    }
};

DECLARE_CMD(IsFeatureSupported) {
    CMD_NAME("isFeatureSupported")
    CMD_ARGS(
        CMD_ARG("ble::controller_supported_features_t::type", "feature", ""),
    )
    CMD_HANDLER(ble::controller_supported_features_t::type feature, CommandResponsePtr& response) {
        response->success(gap().isFeatureSupported(feature));
    }
};

DECLARE_CMD(IsRadioActive) {
    CMD_NAME("isRadioActive")
    CMD_HANDLER(CommandResponsePtr& response) {
        ble::impl::Gap*& gap_impl = gap().*_gap_impl_accessor;
        bool active = (gap_impl->*_gap_impl_is_radio_active_accessor)();
        response->success(active);
    }
};

static const Command* const _cmd_handlers[] = {
    CMD_INSTANCE(GetAddressCommand),
    CMD_INSTANCE(GetMaxWhitelistSizeCommand),
    CMD_INSTANCE(GetWhitelistCommand),
    CMD_INSTANCE(SetWhitelistCommand),
    CMD_INSTANCE(EnablePrivacyCommand),
    CMD_INSTANCE(SetPeripheralPrivacyConfigurationCommand),
    CMD_INSTANCE(GetPeripheralPrivacyConfigurationCommand),
    CMD_INSTANCE(SetCentralPrivacyConfigurationCommand),
    CMD_INSTANCE(GetCentralPrivacyConfigurationCommand),
    CMD_INSTANCE(SetPhyCommand),
    CMD_INSTANCE(SetPreferredPhysCommand),
    CMD_INSTANCE(ReadPhyCommand),

    CMD_INSTANCE(GetMaxAdvertisingSetNumber),
    CMD_INSTANCE(GetMaxAdvertisingDataLength),
    CMD_INSTANCE(CreateAdvertisingSet),
    CMD_INSTANCE(DestroyAdvertisingSet),
    CMD_INSTANCE(SetAdvertisingParameters),
    CMD_INSTANCE(SetAdvertisingPayload),
    CMD_INSTANCE(ApplyAdvPayloadFromBuilder),
    CMD_INSTANCE(SetAdvertisingScanResponse),
    CMD_INSTANCE(ApplyScanRespFromBuilder),
    CMD_INSTANCE(StartAdvertising),
    CMD_INSTANCE(StopAdvertising),
    CMD_INSTANCE(IsAdvertisingActive),
    CMD_INSTANCE(SetPeriodicAdvertisingParameters),
    CMD_INSTANCE(SetPeriodicAdvertisingPayload),
    CMD_INSTANCE(StartPeriodicAdvertising),
    CMD_INSTANCE(StopPeriodicAdvertising),
    CMD_INSTANCE(IsPeriodicAdvertisingActive),
    CMD_INSTANCE(SetScanParameters),
    CMD_INSTANCE(StartScan),
    CMD_INSTANCE(ScanForAddress),
    CMD_INSTANCE(ScanForData),
    CMD_INSTANCE(StopScan),
    CMD_INSTANCE(CreateSync),
    CMD_INSTANCE(CreateSyncFromList),
    CMD_INSTANCE(CancelCreateSync),
    CMD_INSTANCE(TerminateSync),
    CMD_INSTANCE(AddDeviceToPeriodicAdvertiserList),
    CMD_INSTANCE(RemoveDeviceFromPeriodicAdvertiserList),
    CMD_INSTANCE(ClearPeriodicAdvertiserList),
    CMD_INSTANCE(GetMaxPeriodicAdvertiserListSize),
    CMD_INSTANCE(Connect),
    CMD_INSTANCE(StartConnecting),
    CMD_INSTANCE(WaitForConnection),
    CMD_INSTANCE(WaitForDisconnection),
    CMD_INSTANCE(CancelConnect),
    CMD_INSTANCE(UpdateConnectionParameters),
    CMD_INSTANCE(ManageConnectionParametersUpdateRequest),
    CMD_INSTANCE(AcceptConnectionParametersUpdate),
    CMD_INSTANCE(RejectConnectionParametersUpdate),
    CMD_INSTANCE(Disconnect),
    CMD_INSTANCE(IsFeatureSupported),
    CMD_INSTANCE(IsRadioActive)
};

} // end of annonymous namespace

ConstArray<const Command*> GapCommandSuiteDescription::commands() {
    init();
    return ConstArray<const Command*>(
        sizeof(_cmd_handlers)/sizeof(_cmd_handlers[0]), _cmd_handlers
    );
}

void GapCommandSuiteDescription::init()
{
    enable_event_handling();
}

void GapCommandSuiteDescription::add_disconnection_callback(
    FunctionPointerWithContext<const ble::DisconnectionCompleteEvent&> callback
) {
    handler.disconnection_chain.add(callback);
}

void GapCommandSuiteDescription::detach_disconnection_callback(
    FunctionPointerWithContext<const ble::DisconnectionCompleteEvent&> callback
) {
    handler.disconnection_chain.detach(callback);
}
