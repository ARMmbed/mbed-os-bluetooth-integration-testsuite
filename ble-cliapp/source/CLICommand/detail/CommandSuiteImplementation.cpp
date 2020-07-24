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
#include <stdint.h>
#include "EventQueue/EventQueue.h"

#include "../CommandEventQueue.h"
#include "CommandSuiteImplementation.h"
#include <string.h>

using mbed::util::SharedPointer;

namespace {

static void whenAsyncCommandEnd(const CommandResponse* response) {
    getCLICommandEventQueue()->post(&cmd_ready, response->getStatusCode());
}

static const Command* getCommand(
    const char* name,
    const ConstArray<const Command*>& builtinCommands,
    const ConstArray<const Command*>& moduleCommands) {
    // builtin commands
    for(size_t i = 0; i < builtinCommands.count(); ++i) {
        if(strcmp(name, builtinCommands[i]->name()) == 0) {
            return (builtinCommands[i]);
        }
    }

    for(size_t i = 0; i < moduleCommands.count(); ++i) {
        if(strcmp(name, moduleCommands[i]->name()) == 0) {
            return (moduleCommands[i]);
        }
    }

    return NULL;
}

}

int CommandSuiteImplementation::commandHandler(
    int argc, char** argv,
    const ConstArray<const Command*>& builtinCommands,
    const ConstArray<const Command*>& moduleCommands) {
    const CommandArgs args(argc, argv);
    const char* commandName = args[1];
    const CommandArgs commandArgs(args.drop(2));

    SharedPointer<CommandResponse> response(new CommandResponse());

    const Command* command = getCommand(commandName, builtinCommands, moduleCommands);
    if(!command) {
        response->faillure("invalid command name, you can get all the command name for this module by using the command 'list'");
        return response->getStatusCode();
    }

    // check arguments
    if(commandArgs.count() < command->argsDescription().count()) {
        response->invalidParameters("not enough arguments");
        return response->getStatusCode();
    }

    if(commandArgs.count() > command->maximumArgsRequired()) {
        response->invalidParameters("too many arguments");
        return response->getStatusCode();
    }

    // execute the handler
    command->handler(commandArgs, response);

    // if response is not referenced elsewhere, this means that the execution is done,
    // just return the status code set
    // otherwise, tell the system that the execution continue and install continuation
    // callback
    if(response.use_count() == 1) {
        return response->getStatusCode();
    } else {
        response->setOnClose(whenAsyncCommandEnd);
        return CMDLINE_RETCODE_EXCUTING_CONTINUE;
    }
}

void CommandSuiteImplementation::help(
    const CommandArgs& args, const SharedPointer<CommandResponse>& response,
    const ConstArray<const Command*>& builtinCommands,
    const ConstArray<const Command*>& moduleCommands) {
    const Command* command = getCommand(args[0], builtinCommands, moduleCommands);
    if(!command) {
        response->invalidParameters("the name of this command does not exist, you can list the command by using the command 'list'");
    } else {
#if defined(ENABLE_COMMAND_HELP)        
        using namespace serialization;

        response->setStatusCode(CommandResponse::SUCCESS);
        JSONOutputStream& stream = response->getResultStream();
        ConstArray<CommandArgDescription> args_desc = command->argsDescription();
        ConstArray<CommandArgDescription> result_desc = command->resultDescription();

        stream << startObject <<
            key("command") << command->name() <<
            key("help") << command->help() << 
            key("arguments") << startArray;
            for (size_t i = 0; i < args_desc.count(); ++i) { 
                stream.formatValue("\"%s: %s - %s\"", args_desc[i].name, args_desc[i].type, args_desc[i].desc);
            }
            stream << endArray <<
            key("results") << startArray;
            for (size_t i = 0; i < result_desc.count(); ++i) { 
                stream.formatValue("\"%s: %s - %s\"", result_desc[i].name, result_desc[i].type, result_desc[i].desc);
            }
            stream << endArray <<
        endObject;
#else
        response->success("Commands help is deactivated, recompile with ENABLE_COMMAND_HELP defined");
#endif
    }
}

void CommandSuiteImplementation::list(
    const CommandArgs&, const SharedPointer<CommandResponse>& response,
    const ConstArray<const Command*>& builtinCommands,
    const ConstArray<const Command*>& moduleCommands) {
    using namespace serialization;

    response->setStatusCode(CommandResponse::SUCCESS);

    serialization::JSONOutputStream& os = response->getResultStream();

    os << startArray;
    // builtin commands
    for(size_t i = 0; i < builtinCommands.count(); ++i) {
        os << builtinCommands[i]->name();
    }

    for(size_t i = 0; i < moduleCommands.count(); ++i) {
        os << moduleCommands[i]->name();
    }

    os << endArray;
}
