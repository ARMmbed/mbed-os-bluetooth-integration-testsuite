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
#ifndef BLE_CLIAPP_CLICOMMAND_DETAIL_LISTCOMMANDBASE_H_
#define BLE_CLIAPP_CLICOMMAND_DETAIL_LISTCOMMANDBASE_H_

#include "../Command.h"

/**
 * @brief Abstract base implementation for the Help Command. 
 * @details Avoid the redeclaration of the name and help functions for 
 * every instantiated class of the HelpCommand.
 */
struct ListCommandBase : public BaseCommand {
    static const char* name() {
        return "list";
    }

    static const char* help() {
        return "list all the command in a module";
    }
};

#endif //BLE_CLIAPP_CLICOMMAND_DETAIL_LISTCOMMANDBASE_H_



