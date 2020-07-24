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
#ifndef BLE_CLIAPP_CLICOMMAND_COMMAND_HELPER_H_
#define BLE_CLIAPP_CLICOMMAND_COMMAND_HELPER_H_

#include "Command.h"
#include "BaseCommand.h"

/**
 * @brief Start the declaration of a new command
 * @details Users should describe their commands in the next block.
 * @param COMMAND_NAME the name of the class of the command.
 */
#define DECLARE_CMD(COMMAND_NAME) struct COMMAND_NAME : public BaseCommand


/**
 * @brief Declare the field name of the command.
 * 
 * @param COMMAND_NAME the name of the command, as an array of char.
 */
#define CMD_NAME(COMMAND_NAME) static const char* name() { \
    return COMMAND_NAME; \
}


/**
 * @brief Declare the help field of a command.
 * 
 * @param COMMAND_HELP the help string associated with the command.
 */
#define CMD_HELP(COMMAND_HELP) static const char* help() { \
    return COMMAND_HELP; \
}


/**
 * @brief Declare the arguments of the command.
 * 
 * @param a list of CMD_ARG
 */
#define CMD_ARGS(...) static ConstArray<CommandArgDescription> argsDescription() { \
        static const CommandArgDescription argsDescription[] = { __VA_ARGS__  }; \
        return ConstArray<CommandArgDescription>(argsDescription); \
    }


/**
 * @brief Declare an argument of the command.
 * 
 * @param type: type of the argument
 * @param name: name of the argument
 * @param desc: description of the argument
 */
#define CMD_ARG(type, name, desc) { type, name, desc }


/**
 * @brief Declare the result of the command
 * 
 * @param a list of CMD_RESULT
 */
#define CMD_RESULTS(...) static ConstArray<CommandArgDescription> resultDescription() { \
        static const CommandArgDescription resultDescription[] = { __VA_ARGS__  }; \
        return ConstArray<CommandArgDescription>(resultDescription); \
    }


/**
 * @brief Declare a result of the command.
 * 
 * @param type: type of the argument
 * @param name: name of the argument
 * @param desc: description of the argument
 */
#define CMD_RESULT(type, name, desc) { type, name, desc }



/**
 * @brief Declare the handler of a command.
 * 
 * @param ARGS_NAME name of the args in the handler function.
 * @param RESPONSE_NAME name of the response in the handler function. 
 * 
 * @return [description]
 */
#define CMD_HANDLER(...) \
    static void handler(__VA_ARGS__)


/**
 * @brief Helper which declare the function commands of a CommandSuite.
 * 
 * @param name: name of the command suite class.
 * @param ...: Command instances.
 */
#define DECLARE_SUITE_COMMANDS(COMMAND_SUITE_NAME, ...) \
    ConstArray<const Command*> COMMAND_SUITE_NAME::commands() { \
        static const Command* const commandHandlers[] = { \
            __VA_ARGS__ \
        }; \
        return ConstArray<const Command*>(commandHandlers); \
    }


/**
 * @brief return the instance of a command.
 * 
 * @param COMMAND_CLASS_NAME The name of the command class.
 */
#define CMD_INSTANCE(COMMAND_CLASS_NAME) \
    &CommandGenerator<COMMAND_CLASS_NAME>::command


#endif //BLE_CLIAPP_CLICOMMAND_COMMAND_HELPER_H_
