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
#ifndef BLE_CLIAPP_CLICOMMAND_COMMANDTASKQUEUE_H_
#define BLE_CLIAPP_CLICOMMAND_COMMANDTASKQUEUE_H_

#include "EventQueue/EventQueue.h"

/**
 * @brief Returns the event queue used by the CLICommand module. 
 */
eq::EventQueue* getCLICommandEventQueue();


/**
 * @brief Set the event queue which will be used by the CLICommand module.
 */
void initCLICommandEventQueue(eq::EventQueue* event_queue);


#endif //BLE_CLIAPP_CLICOMMAND_COMMANDTASKQUEUE_H_
