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
#ifndef BLE_CLIAPP_COMMAND_SUITE_H_
#define BLE_CLIAPP_COMMAND_SUITE_H_

#include "util/ConstArray.h"
#include "CommandArgs.h"
#include "Command.h"
#include "BaseCommand.h"
#include "detail/CommandSuiteImplementation.h"
#include "CommandGenerator.h"
#include "detail/ListCommandBase.h"
#include "detail/HelpCommandBase.h"


/**
 * @brief Allow to easily group and add a suite of commands into the cli system.
 * @details This class is parametized by a description which should provide information to run.
 *
 * @tparam SuiteDescription The class describing the command suite. The class should
 * provide the following static methods :
 *    - static const char* name() const : The name of the suite. Each commands present in the suite
 *    will be available through this entry point.
 *    - static const char* info() : Informations about this command suite
 *    - static const char* man() : The manual of this command suite
 *    - static ConstArray<Command> commands() : The array of commands presents in the suite
 *
 * \code
 *
 * class DummyCommandSuite {
 *
 * public:
 *
 * static const char* name() const {
 *    return "dummy";
 * }
 *
 * static const char* info() {
 *     return "dummy info"
 * }
 *
 * static const char* man() {
 *     return "dummy <command> <command arguments>\r\n"\
 *            "    * dummy foo : print foo\r\n"
 *            "    * dummy bar : print bar\r\n";
 * }
 *
 * static ConstArray<Command*> commands() {
 *  	static const Command* commandHandlers[] = {
 *      	&&CommandGenerator<FooCommand>::command,
 *          &&CommandGenerator<BarCommand>::command
 *      };
 *
 *      return ConstArray<Command*>(commandHandlers);
 * }
 *
 * };
 *
 * \endcode
 *
 */
template<typename SuiteDescription>
class CommandSuite {

public:
    /**
     * @brief Register this command suite into the cli system
     */
    static void registerSuite() {
        cmd_add(
            SuiteDescription::name(),
            commandHandler,
#if ENABLE_COMMAND_INFO_AND_MANUAL
            SuiteDescription::info(),
            SuiteDescription::man()
#else
            "",
            ""
#endif
        );
    }

private:
    /**
     * @brief  Entry point for the command handler of the suite.
     * @details This function demultiplex command and args from CLI and execute the right comamnd.
     * It also collect results and format message result.
     *
     * @return a command status code as described in mbed-client-cli/ns_cmdline.h.
     */
    static int commandHandler(int argc, char** argv) {
        return CommandSuiteImplementation::commandHandler(
            argc,
            argv,
            getBuiltinCommands(),
            getModuleCommands()
        );
    }

    static ConstArray<const Command*> getModuleCommands() {
        return SuiteDescription::commands();
    }


#if ENABLE_BUILTIN_COMMANDS == 0
    static ConstArray<const Command*> getBuiltinCommands() {
        return ConstArray<const Command*>();
    }

#else
    static ConstArray<const Command*> getBuiltinCommands() {
        static const Command* const builtinCommands[] = {
            &CommandGenerator<HelpCommand>::command,
            &CommandGenerator<ListCommand>::command
        };
        return ConstArray<const Command*>(builtinCommands);
    }

    struct HelpCommand : public HelpCommandBase {
        static void handler(const CommandArgs& args, const mbed::util::SharedPointer<CommandResponse>& response) {
            CommandSuiteImplementation::help(
                args,
                response,
                getBuiltinCommands(),
                getModuleCommands()
            );
        }
    };

    struct ListCommand : public ListCommandBase {
        static void handler(const CommandArgs& args, const mbed::util::SharedPointer<CommandResponse>& response) {
            CommandSuiteImplementation::list(
                args,
                response,
                getBuiltinCommands(),
                getModuleCommands()
            );
        }
    };
#endif

};

/**
 * @brief Register a command suite into the system
 *
 * @tparam CommandSuiteDescription The command suite to register
 */
template<typename CommandSuiteDescription>
void registerCommandSuite() {
    CommandSuite<CommandSuiteDescription>::registerSuite();
}

#endif //BLE_CLIAPP_COMMAND_SUITE_H_
