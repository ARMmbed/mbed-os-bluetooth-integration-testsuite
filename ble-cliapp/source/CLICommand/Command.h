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
#ifndef BLE_CLIAPP_CLICOMMAND_COMMAND_H_
#define BLE_CLIAPP_CLICOMMAND_COMMAND_H_

#include <core-util/SharedPointer.h>
#include "CommandResponse.h"
#include "CommandArgs.h"
#include "CommandArgDescription.h"

/**
 * Alias for a command response shared pointer. 
 * A SharedPointer is used to handle the  
 */
typedef const mbed::util::SharedPointer<CommandResponse> CommandResponsePtr;


/**
 * @brief Description of a command. 
 * @details A command is composed of an handler and metadata describing the command. 
 * These metadata help the system to associate a command with a name, provide help 
 * to the user and automatically verify that the number of argument in input match 
 * the number of arguments expected by the handler.
 * 
 * The best way to define a command is to use the helper macros defined in the file 
 * CommandHelper.h to declare it and use the macro CMD_INSTANCE to define it.
 * 
 * A command should be a part of a CommandSuite instance to be usefull.
 */
struct Command {
    /**
     * @brief Return the name of the command. 
     * @details The name of the command is used by the command suite to dispatch 
     * inputs from the command line to the correct handler. 
     * @warning The name of the command should not contain space.
     * @return The name of this command.
     */
    const char* (* const name )();

#if defined(ENABLE_COMMAND_HELP)
    /**
     * @brief The help associated with the command.
     * @detail the help of the command will be displayed when the user enter the command: 
     * <module_name> help <command_name>.
     * @warning The name of the command should not contain formating like tab or line return
     * because it will be integrated in a JSON payload.
     */
    const char* (* const help)();
#endif
    /**
     * @brief Return the list of the arguments expected by this command. 
     * @details The list of argument will be displayed when user asked for the help of the 
     * command. 
     * 
     * The command suite will also automatically reject any invocation of this command if
     * the command line contains less arguments than the number of arguments in this list. 
     */
    ConstArray<CommandArgDescription> (* const argsDescription)();

    /**
     * @brief Return a flat representation of the results returned by the command.
     * @details JSON objects can be returned by the command. In that case, it is 
     * still possible to provide a flat representation of the results as long as the 
     * data returned are structured.   
     * 
     * See the command connect in the Gap command module as an example. 
     */
    ConstArray<CommandArgDescription> (* const resultDescription)();

    /**
     * @brief return the maximum number of arguments that the command can handle.
     * @details This parameter is used to prefilter invalid command requests and 
     * reduce the number of parameters that has to be checked by the handler.
     * If the command line in input contains more arguments than the maximum 
     * number of arguments expected by the command, the command is not invoked 
     * and the system returns INVALID_PARAMETERS instead.  
     */
    std::size_t (* const maximumArgsRequired)();

    /**
     * @brief The actual command handler.
     * @detail A command handler is a function which handle commands from the command
     * line. It receive args in input and a response object it has to fullfill.
     * 
     * @param args : array of arguments of the function. It does not contain the
     * command name at first argument.
     * 
     * @param res : response of the command. It is expected that the handler set the
     * status code then provide a result for the command. This parameter is a shared
     * pointer to a CommandResponse instance, it allow the command to launch an 
     * asynchronous operation, terminate this handler and complete the command at 
     * a latter stage just by copying the instance of the response. 
     * A Command is considered complete once the CommandResponse pointer is not 
     * referenced anymore. 
     * This means that as long as the response exist in the system others command will
     * be rejected.
     * 
     * @note You don't have to manually close the response,  just let the destructor handle 
     * this task.
     */
    void (* const handler)(
        const CommandArgs& args,
        const CommandResponsePtr& res
    );
};

#endif //BLE_CLIAPP_CLICOMMAND_COMMAND_H_
