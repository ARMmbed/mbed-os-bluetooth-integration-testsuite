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
#ifndef BLE_CLIAPP_CLICOMMAND_UTIL_ASYNC_PROCEDURE_
#define BLE_CLIAPP_CLICOMMAND_UTIL_ASYNC_PROCEDURE_

#include "EventQueue/EventQueue.h"
#include "CLICommand/Command.h"

/**
 * @brief Base class for used to build Asynchronous commands.
 * @details This base class help command writer to write clean and efficient 
 * commands requiring asynchronous operations. It manages automatically the 
 * lifetime of the state associated with the operation, the command response
 * and timeout automatically if the operation didn't succeed in the expected time.
 * 
 * @code 
    struct MyLongProcedure : public AsyncProcedure {
        // Construct the procedure. 
        // The state required by the procedure is passed at construction time. 
        // It is mandatory to pass a CommandResponse and a timeout as these values are 
        // needed by the base class AsyncProcedure.
        MyLongProcedure(A0 stateA, A1 stateB, ..., const CommandResponse& res, uint32_t timeout) :
            AsyncProcedure(res, procedureTimeout), _stateA(stateA), _stateB(stateB), ... {
        }

        // Procedure starting point
        // If the procedure launch was successful returns true otherwise returns false
        virtual bool doStart() {
            // start the long operation, 
            // In this case, once the operation is done, whenResult member function will 
            // be called.
            return my_long_operation(_stateA, ..., this, &MyLongProcedure::whenResult)
        }

        // function called 
        void whenResult(bool success) { 
            // asynchronous operation done, fill the response
            if (success) { 
                response->success();
            } else { 
                response->faillure("Oups!");
            }

            // terminate the procedure. This step **is** explicit and shouldn't be omitted.
            terminate();
        }

        // what to do when timeout occur 
        void doWhenTimeout() { 
            // set response 
            // cleanup resources 
            // ...

            // DO NOT CALL terminate in this function.
        }

        A0 _stateA;
        A1 _stateB;
        // ... other state needed by the procedure
    };

    // start the procedure
    startProcedure<MyLongProcedure>(stateA, ..., response, 10 * 1000);
 * @endcode
 */

struct AsyncProcedure {

    template<typename ProcedureType, typename T0>
    friend void startProcedure(const T0& arg0);

    template<typename ProcedureType, typename T0, typename T1>
    friend void startProcedure(const T0& arg0, const T1& arg1);

    template<typename ProcedureType, typename T0, typename T1, typename T2>
    friend void startProcedure(const T0& arg0, const T1& arg1, const T2& arg2);

    template<typename ProcedureType, typename T0, typename T1, typename T2, typename T3>
    friend void startProcedure(const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3);

    template<typename ProcedureType, typename T0, typename T1, typename T2, typename T3, typename T4>
    friend void startProcedure(const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4);

    template<typename ProcedureType, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    friend void startProcedure(const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5);

protected:

    /**
     * @brief Construct an AsyncProcedure
     *
     * @param res The response wich will be written during the procedure life.
     * @param timeout The maximum amount of time before the procedure termination
     */
    AsyncProcedure(const CommandResponsePtr& res, uint32_t timeout);

    /**
     * @brief destructor for a procedure.
     */
    virtual ~AsyncProcedure();

    /**
     * @brief terminate the procedure any subsequent access to procedure member or
     * call to procedure member function is undefined.
     */
    void terminate();

    /**
     * @brief Implementation of start, implementer should return true if the
     * procedure was successfully launch and false otherwise
     * @return true if the procedure was successfully launch and false otherwise
     */
    virtual bool doStart() = 0;

    /**
     * @brief Called when a timeout occur
     * @note do **not** call terminate in this function. It is automatically 
     * done by the system.
     */
    virtual void doWhenTimeout();

    /**
     * @brief response of the procedure
     */
    CommandResponsePtr response;

private:
    /**
     * @brief start the procedure, it will call doStart. If doStart return false,
     * it will terminate the procedure.
     */
    void start();

    /**
     * @brief [brief description]
     * @details [long description]
     */
    void whenTimeout();


    eq::EventQueue::event_handle_t timeoutHandle;
    uint32_t timeout;
};

/**
 * @brief start a new procedure, variadic args will be forwarded to
 * ProcedureType constructor.
 *
 * @param args Args used to build the procedure
 * @tparam ProcedureType The type of procedure to start
 */
template<typename ProcedureType, typename T0>
void startProcedure(T0& arg0) {
    ProcedureType* proc = new ProcedureType(arg0);
    proc->start();
}

template<typename ProcedureType, typename T0, typename T1>
void startProcedure(const T0& arg0, const T1& arg1) {
    ProcedureType* proc = new ProcedureType(arg0, arg1);
    proc->start();
}

template<typename ProcedureType, typename T0, typename T1, typename T2>
void startProcedure(const T0& arg0, const T1& arg1, const T2& arg2) {
    ProcedureType* proc = new ProcedureType(arg0, arg1, arg2);
    proc->start();
}

template<typename ProcedureType, typename T0, typename T1, typename T2, typename T3>
void startProcedure(const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3) {
    ProcedureType* proc = new ProcedureType(arg0, arg1, arg2, arg3);
    proc->start();
}

template<typename ProcedureType, typename T0, typename T1, typename T2, typename T3, typename T4>
void startProcedure(const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
    ProcedureType* proc = new ProcedureType(arg0, arg1, arg2, arg3, arg4);
    proc->start();
}

template<typename ProcedureType, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
void startProcedure(const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5) {
    ProcedureType* proc = new ProcedureType(arg0, arg1, arg2, arg3, arg4, arg5);
    proc->start();
}


#endif //BLE_CLIAPP_CLICOMMAND_UTIL_ASYNC_PROCEDURE_
