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
#ifndef BLE_CLIAPP_COMMAND_RESPONSE_H_
#define BLE_CLIAPP_COMMAND_RESPONSE_H_

#include <mbed-client-cli/ns_cmdline.h>

#include "CommandArgs.h"
#include "Serialization/JSONOutputStream.h"

/**
 * @brief A command response is the response to a command. It doesn't hold data
 * by itself but it provide functions to write the response.
 * @details A response has the following format, in a specific order:
 *   - command name
 *   - command args
 *   - status code
 *   - result
 */
class CommandResponse {

public:
    typedef void(*OnClose_t)(const CommandResponse*);

    /**
     * @brief construct a new response
     * @param outputStream The output stream used to write the response.
     * @important It is expected that outputStream reference remain valid until
     * the response has been closed.
     */
    CommandResponse();

    /**
     * @brief Destroy the command response and close the stream if the stream was not
     * already closed.
     */
    ~CommandResponse();

    enum StatusCode_t {
        COMMAND_BUSY =             2,   //!< Command Busy
        EXCUTING_CONTINUE =        1,  //!< Execution continue in background
        SUCCESS =                  0,   //!< Execution Success
        FAIL =                    -1,   //!< Execution Fail
        INVALID_PARAMETERS =      -2,   //!< Command parameters was incorrect
        COMMAND_NOT_IMPLEMENTED = -3,   //!< Command not implemented
        COMMAND_CB_MISSING =      -4,   //!< Command callback function missing
        COMMAND_NOT_FOUND =       -5   //!< Command not found
    };

    /**
     * @brief Set the command name associated with this response. If the
     * command name has been already set, this function will return false.
     * @param name The name of the command associated with this response.
     * @return true if the command name has been set and false otherwise
     */
    bool setCommandName(const char* name);

    /**
     * @brief Set the arguments associated with this response. If the arguments
     * have been already set, this function will return false.
     * @param args Arguments associated with this response
     * @return true if the arguments has been correctly set and false otherwise.
     */
    bool setArguments(const CommandArgs& args);

    /**
     * @brief Set the status code of the response, if it has already been set, the
     * function will return false.
     * @param statusCode The status code of the response
     * @return true if the status code has been correctly set and false otherwise
     */
    bool setStatusCode(StatusCode_t statusCode);

    /**
     * @brief return the current status code
     */
    StatusCode_t getStatusCode() const;

    /**
     * @brief Get the JSON stream of the response. User are expected to use this
     * stream to write the body of the response.
     * @note it is expected that setStatusCode, setCommandName and setArguments has
     * been called prior to this call.
     * @note It is expected that the user format properly the body of the response.
     * @important Use of the result of this function while isClosed() == true is undefined.
     * @important It is expected that the user does not call setStatusCode, setArgument
     * and setNameFunction until the result has been properly formed.
     * @return a JSON output stream.
     */
    serialization::JSONOutputStream& getResultStream();

    /**
     * @brief Set the callback to call when the response is closed
     * @param onCloseCallBack callback called when the stream is closed
     */
    void setOnClose(const OnClose_t& onCloseCallBack);

    /**
     * @brief Ask to close the response
     */
    void close();

    /**
     * @brief Indicate if the response has been closed or not.
     * @return true if the response has been closed and false otherwise
     */
    bool isClosed();

    /**
     * @brief shorthand for:
     * \code
     * response.setStatusCode(INVALID_PARAMETERS);
     * response.getResultStream() << msg;
     * \code
     *
     * @param msg The message to set in the result stream
     * @return true if it succeed and false otherwise
     */
    bool invalidParameters(const char* msg = NULL);

    /**
     * @brief same as bool invalidParameters(const char*) but for any type which
     * implement operator<<(JSONOutputStream&, const T&).
     *
     * @param val The value to put in the result stream.
     * @return true if it succeed and false otherwise.
     */
    template<typename T>
    bool invalidParameters(const T& val) {
        return setStatusCodeAndMessage(INVALID_PARAMETERS, val);
    }

    /**
     * @brief shorthand for:
     * \code
     * response.setStatusCode(COMMAND_NOT_IMPLEMENTED);
     * response.getResultStream() << msg;
     * \code
     *
     * @param msg The message to set in the result stream
     * @return true if it succeed and false otherwise
     */
    bool notImplemented(const char* msg = NULL);

    /**
     * @brief same as bool notImplemented(const char*) but for any type which
     * implement operator<<(JSONOutputStream&, const T&).
     *
     * @param val The value to put in the result stream.
     * @return true if it succeed and false otherwise.
     */
    template<typename T>
    bool notImplemented(const T& val) {
        return setStatusCodeAndMessage(COMMAND_NOT_IMPLEMENTED, val);
    }

    /**
     * @brief shorthand for:
     * \code
     * response.setStatusCode(FAIL);
     * response.getResultStream() << msg;
     * \code
     *
     * @param msg The message to set in the result stream
     * @return true if it succeed and false otherwise
     */
    bool faillure(const char* msg = NULL);

    /**
     * @brief same as bool faillure(const char*) but for any type which
     * implement operator<<(JSONOutputStream&, const T&).
     *
     * @param val The value to put in the result stream.
     * @return true if it succeed and false otherwise.
     */
    template<typename T>
    bool faillure(const T& val) {
        return setStatusCodeAndMessage(FAIL, val);
    }

    /**
     * @brief shorthand for:
     * \code
     * response.setStatusCode(SUCCESS);
     * response.getResultStream() << msg;
     * \code
     *
     * @param msg The message to set in the result stream
     * @return true if it succeed and false otherwise
     */
    bool success(const char* msg = NULL);

    /**
     * @brief same as bool success(const char*) but for any type which
     * implement operator<<(JSONOutputStream&, const T&).
     *
     * @param val The value to put in the result stream.
     * @return true if it succeed and false otherwise.
     */
    template<typename T>
    bool success(const T& val) {
        return setStatusCodeAndMessage(SUCCESS, val);
    }

private:
    bool setStatusCodeAndMessage(StatusCode_t sc, const char* msg);

    template<typename T>
    bool setStatusCodeAndMessage(StatusCode_t sc, const T& msg) {
        if(!setStatusCode(sc)) {
            return false;
        }

        getResultStream() << msg;
        return true;
    }

    OnClose_t onClose;
    serialization::JSONOutputStream out;
    StatusCode_t statusCode;
    bool nameSet:1;
    bool argumentsSet:1;
    bool statusCodeSet:1;
    bool resultStarted:1;
    bool closed:1;
};



#endif //BLE_CLIAPP_COMMAND_RESPONSE_H_
