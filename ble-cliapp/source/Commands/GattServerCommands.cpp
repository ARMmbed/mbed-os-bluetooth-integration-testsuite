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
#include "ble/services/HeartRateService.h"
#include "Serialization/Serializer.h"
#include "Serialization/UUID.h"
#include "Serialization/Hex.h"
#include "Serialization/CharacteristicProperties.h"
#include "Serialization/CharacteristicSecurity.h"
#include "Serialization/BLECommonSerializer.h"
#include "Serialization/GattCallbackParamTypes.h"

#include "util/ServiceBuilder.h"
#include "CLICommand/util/AsyncProcedure.h"

#include "Common.h"

#include "GattServerCommands.h"
#include "CLICommand/CommandHelper.h"

using mbed::util::SharedPointer;
using ble::Gap;
using ble::GattClient;
using ble::GattServer;
using ble::SecurityManager;

// isolation
namespace {

static ServiceBuilder* serviceBuilder = NULL;

static ::detail::RAIIGattService** gattServices = NULL;
static size_t gattServicesCount = 0;
static bool cleanupRegistered = false;

static void cleanupServiceBuilder();

static void whenShutdown(const ble::GattServer*) {
    cleanupServiceBuilder();
    for(size_t i = 0; i < gattServicesCount; ++i) {
        delete gattServices[i];
    }
    std::free(gattServices);
    gattServices = NULL;
    gattServicesCount = 0;
    gattServer().onShutdown().detach(whenShutdown);
    cleanupRegistered = false;
}

static void cleanupServiceBuilder() {
    if(!serviceBuilder) {
        return;
    }

    delete serviceBuilder;
    serviceBuilder = NULL;
}

static bool initServiceBuilder(const UUID& uuid) {
    if(serviceBuilder) {
        return false;
    }
    serviceBuilder = new ServiceBuilder(uuid);
    if(cleanupRegistered == false) {
        gattServer().onShutdown(whenShutdown);
        cleanupRegistered = true;
    }
    return true;
}


DECLARE_CMD(DeclareServiceCommand) {
    CMD_NAME("declareService")

    CMD_HELP("Start the declaration of a service, after this call, user can call declareCharacteristic to declare "
               "a characteristic inside the service, commitService to commit the service or cancelServiceDeclaration "
               "to cancel the service declaration")

    CMD_ARGS(
        CMD_ARG("UUID", "UUID", "The UUID of the service")
    )

    CMD_HANDLER(UUID serviceUUID, CommandResponsePtr& response) {
        if(initServiceBuilder(serviceUUID)) {
            response->success();
        } else {
            response->faillure("Impossible to start a service declaration, a service is already being declared");
        }
    }
};


DECLARE_CMD(DeclareCharacteristicCommand) {
    CMD_NAME("declareCharacteristic")

    CMD_HELP("Start the declaration of a characteristic, after this call, user can call declareCharacteristic to declare "
               "another characteristic inside the service, declareDescriptor to add a descriptor inside this characteristic, "
               "commitService to commit the service or cancelServiceDeclaration to cancel the service declaration")

    CMD_ARGS(
        CMD_ARG("UUID", "UUID", "The UUID of the characteristic")
    )

    CMD_HANDLER(UUID characteristicUUID, CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        serviceBuilder->declareCharacteristic(characteristicUUID);
        response->success();

    }
};


DECLARE_CMD(SetCharacteristicValueCommand) {
    CMD_NAME("setCharacteristicValue")

    CMD_HELP("Set the value of the characteristic being declared")

    CMD_ARGS(
        CMD_ARG("RawData_t", "value", "The value of the characteristic")
    )

    CMD_HANDLER(RawData_t characteristicValue, CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->setCharacteristicValue(characteristicValue)) {
            response->success();
        } else {
            response->faillure("Impossible to set a characteristic value");
        }
    }
};


DECLARE_CMD(SetCharacteristicSecurityCommand) {
    CMD_NAME("setCharacteristicSecurity")

    CMD_HELP("Set the read, write and update security requirements of a characteristic "
             "being declared, this function expects three arguments, each being one of "
             "enum values 'NONE', 'UNAUTHENTICATED', "
             "'AUTHENTICATED', 'SC_AUTHENTICATED'")

    CMD_ARGS(
        CMD_ARG("GattCharacteristic::SecurityRequirement_t::type", "read_security", "The security requirement for the characteristic"),
        CMD_ARG("GattCharacteristic::SecurityRequirement_t::type", "write_security", "The security requirement for the characteristic"),
        CMD_ARG("GattCharacteristic::SecurityRequirement_t::type", "update_security", "The security requirement for the characteristic")
    )

    CMD_HANDLER(GattCharacteristic::SecurityRequirement_t::type read_security,
        GattCharacteristic::SecurityRequirement_t::type write_security,
        GattCharacteristic::SecurityRequirement_t::type update_security,
        CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->setCharacteristicSecurity(read_security, write_security, update_security)) {
            response->success();
        } else {
            response->faillure("Impossible to set the characteristic security");
        }
    }
};


DECLARE_CMD(SetCharacteristicPropertiesCommand) {
    CMD_NAME("setCharacteristicProperties")

    CMD_HELP("Set the properties of a characteristic being declared, this function expect a list of "
               "properties such as 'broadcast', 'read', 'writeWoResp', 'write', 'notify', 'indicate' and "
               "'authSignedWrite'")

    template<typename T>
    static std::size_t maximumArgsRequired() {
        return 0xFF;
    }

    CMD_HANDLER(const CommandArgs& args, CommandResponsePtr& response) {
        uint8_t properties;
        if(!characteristicPropertiesFromStrings(args, properties)) {
            response->invalidParameters("Properties are ill formed");
            return;
        }

        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->setCharacteristicProperties(properties)) {
            response->success();
        } else {
            response->faillure("Impossible to set the characteristic properties");
        }
    }
};


DECLARE_CMD(SetCharacteristicVariableLengthCommand) {
    CMD_NAME("setCharacteristicVariableLength")

    CMD_HELP("Set a boolean value which indicate if the characteristic has a variable len. If the "
               "characteristic has a variable len, max len could be set to bound the length to a maximum")

    CMD_ARGS(
        CMD_ARG("bool", "bool", "The value of the variable length property")
    )

    CMD_HANDLER(bool variableLen, CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->setCharacteristicVariableLength(variableLen)) {
            response->success();
        } else {
            response->faillure("Impossible to set the characteristic variable length attribute");
        }
    }
};


DECLARE_CMD(SetCharacteristicMaxLengthCommand) {
    CMD_NAME("setCharacteristicMaxLength")

    CMD_HELP("Set the maximum lenght that is allowed for the value of the characteristic being declared")

    CMD_ARGS(
        CMD_ARG("uint16_t", "max_len", "Maximum length of the value of the characteristic being declared")
    )

    CMD_HANDLER(uint16_t maxLen, CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->setCharacteristicMaxLength(maxLen)) {
            response->success();
        } else {
            response->faillure("Impossible to set the characteristic maximum length");
        }

    }
};


DECLARE_CMD(DeclareDescriptorCommand) {
    CMD_NAME("declareDescriptor")

    CMD_HELP("Start the declaration of a descriptor which will be attached to the characteristic being declared")

    CMD_ARGS(
        CMD_ARG("UUID", "uuid", "The UUID of the descriptor")
    )

    CMD_HANDLER(UUID descriptorUUID, CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->declareDescriptor(descriptorUUID)) {
            response->success();
        } else {
            response->faillure("Impossible to declare this descriptor");
        }
    }
};


DECLARE_CMD(SetDescriptorValueCommand) {
    CMD_NAME("setDescriptorValue")

    CMD_HELP("Set the value of the descriptor being declared")

    CMD_ARGS(
        CMD_ARG("RawData_t", "value", "The value of the descriptor")
    )

    CMD_HANDLER(RawData_t descriptorValue, CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->setDescriptorValue(descriptorValue)) {
            response->success();
        } else {
            response->faillure("Impossible to set the descriptor value");
        }
    }
};


DECLARE_CMD(SetDescriptorVariableLengthCommand) {
    CMD_NAME("setDescriptorVariableLength")

    CMD_HELP("Set a boolean value which indicate if the descriptor has a variable len. If the "
               "descriptor has a variable len, max len could be set to bound the length to a maximum")

    CMD_ARGS(
        CMD_ARG("bool", "variable_length", "The value of the variable length property")
    )

    CMD_HANDLER(bool variableLen, CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->setDescriptorVariableLength(variableLen)) {
            response->success();
        } else {
            response->faillure("Impossible to set the descriptor variable length attribute");
        }
    }
};


DECLARE_CMD(SetDescriptorMaxLengthCommand) {
    CMD_NAME("setDescriptorMaxLength")

    CMD_HELP("Set the maximum lenght that is allowed for the value of the descriptor being declared")

    CMD_ARGS(
        CMD_ARG("uint16_t", "max_length", "Maximum length of the value of the descriptor being declared")
    )

    CMD_HANDLER(uint16_t maxLen, CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        if(serviceBuilder->setDescriptorMaxLength(maxLen)) {
            response->success();
        } else {
            response->faillure("Impossible to set the descriptor maximum length");
        }
    }
};


DECLARE_CMD(CommitServiceCommand) {
    CMD_NAME("commitService")

    CMD_HELP("commit the service declaration")

    CMD_RESULTS( 
        CMD_RESULT("JSON object", "", "The service declared"),
        CMD_RESULT("UUID", "UUID", "The UUID of the service"),
        CMD_RESULT("uint16_t", "handle", "The handle of the service declaration."),
        CMD_RESULT("JSON Array", "characteristics", "List of the characteristics of the service."),
        CMD_RESULT("UUID", "characteristics[].UUID", "UUID of a characteristic."),
        CMD_RESULT("uint16_t", "characteristics[].value_handle", "Handle of the value of a characteristic."),
        CMD_RESULT("JSON Array", "characteristics[].properties", "List of the properties of a characteristic."),
        CMD_RESULT("uint16_t", "characteristics[].length", "Lenght of the characteristic value."),
        CMD_RESULT("uint16_t", "characteristics[].max_length", "Maximum lenght of the characteristic value."),
        CMD_RESULT("bool", "characteristics[].has_variable_length", "Indicate if the characteristic can have a variable length."),
        CMD_RESULT("HexString", "characteristics[].value", "The value of a characteristic."),
        CMD_RESULT("JSON Array", "characteristics[].descriptors", "List of the descriptors of the characteristic."),
        CMD_RESULT("UUID", "characteristics[].descriptors[].UUID", "UUID of the descriptor."),
        CMD_RESULT("uint16_t", "characteristics[].descriptors[].handle", "Handle of the value of the descriptor."),
        CMD_RESULT("uint16_t", "characteristics[].descriptors[].length", "Lenght of the descriptor value."),
        CMD_RESULT("uint16_t", "characteristics[].descriptors[].max_length", "Maximum lenght of the descriptor value."),
        CMD_RESULT("bool", "characteristics[].descriptors[].has_variable_length", "Indicate if the descriptor can have a variable length."),
        CMD_RESULT("HexString", "characteristics[].descriptors[].value", "The value of the descriptor.")
    )

    CMD_HANDLER(CommandResponsePtr& response) {
        using namespace serialization;

        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }

        serviceBuilder->commit();
        ::detail::RAIIGattService* service = serviceBuilder->release();

        ble_error_t err = gattServer().addService(*service);
        if(err) {
            response->faillure(err);
            delete service;
        } else {
            response->success();
            // iterate over all handles
            serialization::JSONOutputStream& os = response->getResultStream() << startObject <<
                key("UUID") << service->getUUID() <<
                key("handle") << service->getHandle() <<
                key("characteristics") << startArray;

            for(uint16_t i = 0; i < service->getCharacteristicCount(); ++i) {
                GattCharacteristic& characteristic = *service->getCharacteristic(i);
                GattAttribute& characteristicAttribute = characteristic.getValueAttribute();

                os << startObject <<
                    key("UUID") << characteristicAttribute.getUUID() <<
                    key("value_handle") << characteristicAttribute.getHandle() <<
                    key("properties");  serializeCharacteristicProperties(os, characteristic.getProperties()) <<
                    key("length") << characteristicAttribute.getLength() <<
                    key("max_length") << characteristicAttribute.getMaxLength() <<
                    key("has_variable_length") << characteristicAttribute.hasVariableLength();

                if(characteristicAttribute.getLength()) {
                    os << key("value");
                    serializeRawDataToHexString(
                        os,
                        characteristicAttribute.getValuePtr(),
                        characteristicAttribute.getLength()
                    );
                } else {
                    os << key("value") << "";
                }

                os << key("descriptors") << startArray;
                for(uint16_t j = 0; j < characteristic.getDescriptorCount(); ++j) {
                    GattAttribute& descriptor = *characteristic.getDescriptor(j);
                    os << startObject <<
                        key("UUID") << descriptor.getUUID() <<
                        key("handle") << descriptor.getHandle() <<
                        key("length") << descriptor.getLength() <<
                        key("max_length") << descriptor.getMaxLength() <<
                        key("has_variable_length") << descriptor.hasVariableLength();

                    if(descriptor.getLength()) {
                        os << key("value");
                        serializeRawDataToHexString(
                            os,
                            descriptor.getValuePtr(),
                            descriptor.getLength()
                        );
                    } else {
                        os << key("value") << "";
                    }
                    os << endObject;
                }
                os << endArray;
                os << endObject;
            }
            os << endArray;
            os << endObject;

            // add the service inside the list of instantiated services
            gattServices = static_cast<::detail::RAIIGattService**>(std::realloc(gattServices, sizeof(*gattServices) * (gattServicesCount + 1)));
            gattServices[gattServicesCount] = service;
            gattServicesCount += 1;

            // release unused memory
            service->releaseAttributesValue();
        }

        // anyway, everything is cleaned up
        cleanupServiceBuilder();
    }
};


DECLARE_CMD(CancelServiceDeclarationCommand) {
    CMD_NAME("cancelServiceDeclaration")

    CMD_HELP("cancel the service declaration")

    CMD_HANDLER(CommandResponsePtr& response) {
        if(!serviceBuilder) {
            response->faillure("Their is no service being declared");
            return;
        }
        response->success();
        cleanupServiceBuilder();
    }
};


DECLARE_CMD(ReadCommand) {
    CMD_NAME("read")

    CMD_HELP("read the value of an attribute of the GATT server, this function take the"
               "attribute of the handle to read as first parameter. It is also possible to"
               "supply a connection handle has second parameter.")

    CMD_ARGS(
        CMD_ARG("uint16_t", "handle", "The handle of the attribute to read")
    )

    CMD_RESULTS( 
        CMD_RESULT("HexString", "", "The value read.")
    )

    template<typename T>
    static std::size_t maximumArgsRequired() {
        return 2;
    }

    CMD_HANDLER(const CommandArgs& args, CommandResponsePtr& response) {
        GattServer& server = gattServer();

        if(args.count() > 2) {
            response->invalidParameters("Too many arguments");
            return;
        }

        GattAttribute::Handle_t attributeHandle;
        if(!fromString(args[0], attributeHandle)) {
            response->invalidParameters("The attribute handle is ill formed");
            return;
        }

        if(args.count() == 2) {
            ble::connection_handle_t connectionHandle;
            if(!fromString(args[1], connectionHandle)) {
                response->invalidParameters("The connection handle is ill formed");
                return;
            }

            uint16_t length = 0;
            ble_error_t err = server.read(connectionHandle, attributeHandle, NULL, &length);
            if(err) {
                response->faillure(err);
                return;
            }

            uint8_t* buffer = new uint8_t[length];
            err = server.read(connectionHandle, attributeHandle, buffer, &length);
            if(err) {
                response->faillure(err);
            } else {
                serializeRawDataToHexString(response->getResultStream(), buffer, length);
                response->success();
            }
            delete[] buffer;
        } else {
            uint16_t length = 0;
            ble_error_t err = server.read(attributeHandle, NULL, &length);
            if(err) {
                response->faillure(err);
                return;
            }

            uint8_t* buffer = new uint8_t[length];
            err = server.read(attributeHandle, buffer, &length);
            if(err) {
                response->faillure(err);
            } else {
                serializeRawDataToHexString(response->getResultStream(), buffer, length);
                response->success();
            }
            delete[] buffer;
        }
    }
};


DECLARE_CMD(WriteCommand) {
    CMD_NAME("write")

    CMD_HELP("write the value of an attribute of the GATT server, this function take the"
               "attribute of the handle to write as first parameter and the value to write "
               "as second parameter. It is also possible to supply a connection handle has "
               "third parameter.")

    CMD_ARGS(
        CMD_ARG("uint16_t", "handle", "The handle of the attribute to write"),
        CMD_ARG("HexString", "value", "The value to write")
    )

    template<typename T>
    static std::size_t maximumArgsRequired() {
        return 3;
    }

    CMD_HANDLER(const CommandArgs& args, CommandResponsePtr& response) {
        GattServer& server = gattServer();

        if(args.count() > 3) {
            response->invalidParameters("Too many arguments");
            return;
        }

        GattAttribute::Handle_t attributeHandle;
        if(!fromString(args[0], attributeHandle)) {
            response->invalidParameters("The attribute handle is ill formed");
            return;
        }

        container::Vector<uint8_t> value = hexStringToRawData(args[1]);
        if(value.size() == 0) {
            response->invalidParameters("The value to write is ill formed");
            return;
        }

        ble_error_t err = BLE_ERROR_UNSPECIFIED;

        if(args.count() == 3) {
            ble::connection_handle_t connectionHandle;
            if(!fromString(args[2], connectionHandle)) {
                response->invalidParameters("The connection handle is ill formed");
                return;
            }

            err = server.write(connectionHandle, attributeHandle, value.begin(), value.size());
        } else {
            err = server.write(attributeHandle, value.begin(), value.size());
        }

        if(err) {
            response->faillure(err);
        } else {
            response->success();
        }
    }
};


DECLARE_CMD(WaitForDataWrittenCommand) {
    CMD_NAME("waitForDataWritten")

    CMD_HELP("Wait for a data to be written on a given characteristic from a given connection.")

    CMD_ARGS(
        CMD_ARG("uint16_t", "connection_handle", "The connection ID with the client supposed to write data"),
        CMD_ARG("uint16_t", "attribute_handle", "The attribute handle which will be written"),
        CMD_ARG("uint16_t", "timeout", "Maximum time allowed for this procedure")
    )

    CMD_HANDLER(ble::connection_handle_t connectionHandle, GattAttribute::Handle_t attributeHandle, uint16_t procedureTimeout, CommandResponsePtr& response) {
        startProcedure<WaitForDataWrittenProcedure>(
            response,
            procedureTimeout,
            connectionHandle,
            attributeHandle
        );
    }

    struct WaitForDataWrittenProcedure : public AsyncProcedure {
        WaitForDataWrittenProcedure(CommandResponsePtr& res, uint32_t procedureTimeout,
            ble::connection_handle_t connectionHandle, GattAttribute::Handle_t attributeHandle) :
            AsyncProcedure(res, procedureTimeout),
            connection(connectionHandle),
            attribute(attributeHandle) {
        }

        virtual ~WaitForDataWrittenProcedure() {
            gattServer().onDataWritten().detach(makeFunctionPointer(this, &WaitForDataWrittenProcedure::whenDataWritten));
        }

        virtual bool doStart() {
            gattServer().onDataWritten(this, &WaitForDataWrittenProcedure::whenDataWritten);
            return true;
        }

        void whenDataWritten(const GattWriteCallbackParams* params) {
            // filter events not relevant
            if(params->connHandle != connection || params->handle != attribute) {
                return;
            }

            response->success(*params);
            terminate();
        }

        ble::connection_handle_t connection;
        GattAttribute::Handle_t attribute;
    };
};

} // end of annonymous namespace


DECLARE_SUITE_COMMANDS(GattServerCommandSuiteDescription, 
    CMD_INSTANCE(DeclareServiceCommand),
    CMD_INSTANCE(DeclareCharacteristicCommand),
    CMD_INSTANCE(SetCharacteristicValueCommand),
    CMD_INSTANCE(SetCharacteristicSecurityCommand),
    CMD_INSTANCE(SetCharacteristicPropertiesCommand),
    CMD_INSTANCE(SetCharacteristicVariableLengthCommand),
    CMD_INSTANCE(SetCharacteristicMaxLengthCommand),
    CMD_INSTANCE(DeclareDescriptorCommand),
    CMD_INSTANCE(SetDescriptorValueCommand),
    CMD_INSTANCE(SetDescriptorVariableLengthCommand),
    CMD_INSTANCE(SetDescriptorMaxLengthCommand),
    CMD_INSTANCE(CommitServiceCommand),
    CMD_INSTANCE(CancelServiceDeclarationCommand),
    CMD_INSTANCE(ReadCommand),
    CMD_INSTANCE(WriteCommand),
    CMD_INSTANCE(WaitForDataWrittenCommand)
)
