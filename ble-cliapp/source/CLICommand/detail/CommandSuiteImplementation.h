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
#ifndef BLE_CLIAPP_COMMAND_SUITE_IMPLEMENTATION_H_
#define BLE_CLIAPP_COMMAND_SUITE_IMPLEMENTATION_H_

#include "util/ConstArray.h"
#include "../CommandArgs.h"
#include "../Command.h"
#include "../CommandGenerator.h"

/**
 * @brief Implementation of command suite. This is used to reduce template instantiations.
 * It is not meant to be used directly.
 */
struct CommandSuiteImplementation {
    static int commandHandler(
        int argc, char** argv,
        const ConstArray<const Command*>& builtinCommands,
        const ConstArray<const Command*>& moduleCommands
    );

    /**
     * @brief builtin help command implementation
     */
    static void help(
        const CommandArgs& args, const mbed::util::SharedPointer<CommandResponse>& response,
        const ConstArray<const Command*>& builtinCommands,
        const ConstArray<const Command*>& moduleCommands
    );

    /**
     * @brief builtin list command implementation
     */
    static void list(
        const CommandArgs&, const mbed::util::SharedPointer<CommandResponse>& response,
        const ConstArray<const Command*>& builtinCommands,
        const ConstArray<const Command*>& moduleCommands
    );
};


#endif /* BLE_CLIAPP_COMMAND_SUITE_IMPLEMENTATION_H_ */
