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
#ifndef BLE_CLIAPP_CLICOMMAND_DETAIL_HELPCOMMAND_BASE_H_
#define BLE_CLIAPP_CLICOMMAND_DETAIL_HELPCOMMAND_BASE_H_

#include "../Command.h"

/**
 * @brief Abstract base implementation for the Help Command. 
 * @details Avoid the redeclaration of the name, help and argsDescription functions for 
 * every instantiated class of the HelpCommand.
 */
struct HelpCommandBase : public BaseCommand {
    static const char* name() {
        return "help";
    }

    static const char* help() {
        return "Print help about a command, you can list the command by using the command 'list'";
    }

    static ConstArray<CommandArgDescription> argsDescription() {
        static const CommandArgDescription argsDescription[] = {
            {
                "string",
                "<commandName>",
                "the name of a command you want help for, use the command 'list' to have a list of available commands"
            }
        };
        return ConstArray<CommandArgDescription>(argsDescription);
    }
};

#endif //BLE_CLIAPP_CLICOMMAND_DETAIL_HELPCOMMAND_BASE_H_



