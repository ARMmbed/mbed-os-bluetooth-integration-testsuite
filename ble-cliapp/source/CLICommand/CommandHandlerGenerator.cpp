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
#include "CommandHandlerGenerator.h"
#include "Serialization/Serializer.h"

void CommandHandlerGenerator::print_error(
    const CommandResponsePtr& response, 
    uint32_t index, 
    ConstArray<CommandArgDescription> (*argsDescription)()) { 
    using namespace serialization;

    response->setStatusCode(CommandResponse::INVALID_PARAMETERS);

    JSONOutputStream& out = response->getResultStream();

    out << startObject <<
        key("index") << index;

    ConstArray<CommandArgDescription> args_desc = argsDescription();
    if (args_desc.count() > index) { 
        out <<
        key("name") << args_desc[index].name <<
        key("type") << args_desc[index].type <<
        key("description") << args_desc[index].desc;
    } 
    out << endObject;
}
