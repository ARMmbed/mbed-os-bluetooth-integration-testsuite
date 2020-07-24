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

#include <cstdio>
#include <stdio.h>
#include <cstdarg>

#include "JSONOutputStream.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace serialization {

JSONOutputStream& JSONOutputStream::operator<<(bool value) {
    handleNewValue();
    write(value ? "true" : "false");
    commitValue();
    return *this;
}

JSONOutputStream& JSONOutputStream::operator<<(int8_t value) {
    return formatValue("%d", static_cast<int>(value));
}

JSONOutputStream& JSONOutputStream::operator<<(uint8_t value) {
    return formatValue("%u", static_cast<long unsigned int>(value));
}

JSONOutputStream& JSONOutputStream::operator<<(int16_t value) {
    return formatValue("%d", static_cast<int>(value));
}

JSONOutputStream& JSONOutputStream::operator<<(uint16_t value) {
    return formatValue("%u", static_cast<long unsigned int>(value));
}

JSONOutputStream& JSONOutputStream::operator<<(int32_t value) {
    return formatValue("%d", static_cast<int>(value));
}

JSONOutputStream& JSONOutputStream::operator<<(unsigned int value) {
    return formatValue("%u", static_cast<long unsigned int>(value));
}

JSONOutputStream& JSONOutputStream::operator<<(long unsigned int value) {
    return formatValue("%u", value);
}

JSONOutputStream& JSONOutputStream::operator<<(int64_t value) {
    return formatValue("%" PRId64, value);
}

JSONOutputStream& JSONOutputStream::operator<<(uint64_t value) {
    return formatValue("%" PRIu64, value);
}

JSONOutputStream& JSONOutputStream::operator<<(const char* value) {
    return formatValue("\"%s\"", value);
}

JSONOutputStream& startArray(JSONOutputStream& os) {
    os.write("[");
    return os;
}

JSONOutputStream& endArray(JSONOutputStream& os) {
    os.startNewValue = false;
    os.put(']');
    os.commitValue();
    return os;
}

JSONOutputStream& startObject(JSONOutputStream& os) {
    os.write("{");
    return os;
}

JSONOutputStream& endObject(JSONOutputStream& os) {
    os.startNewValue = false;
    os.put('}');
    os.commitValue();
    return os;
}

JSONOutputStream& nil(JSONOutputStream& os) {
    os.write("null");
    os.commitValue();
    return os;
 }

JSONOutputStream::JSONOutputStream(mbed::UnbufferedSerial& output) :
    out(output), startNewValue(false) {
}

JSONOutputStream::~JSONOutputStream() {
    const char str[] = "\r\n";
    out.write(str, strlen(str));
    flush();
}

JSONOutputStream& JSONOutputStream::format(const char *fmt, ...) {
    std::va_list args;
    va_start(args, fmt);
    vformat(fmt, args);
    va_end(args);
    return *this;
}

JSONOutputStream& JSONOutputStream::vformat(const char *fmt, std::va_list list) {
    handleNewValue();

    // ARMCC microlib does not properly handle a size of 0.
    // As a workaround supply a dummy buffer with a size of 1.
    char dummy_buf[1];
    int len = vsnprintf(dummy_buf, sizeof(dummy_buf), fmt, list);
    if (len < 100) {
        char temp[100];
        vsprintf(temp, fmt, list);
        out.write(temp, strlen(temp));
    } else {
        char *temp = new char[len + 1];
        vsprintf(temp, fmt, list);
        out.write(temp, strlen(temp));
        delete[] temp;
    }

    return *this;
}

void JSONOutputStream::put(char c) {
    handleNewValue();
    out.write(&c, 1);
}

void JSONOutputStream::write(const char* data, std::size_t count) {
    handleNewValue();
    out.write(data, count);
}

void JSONOutputStream::write(const char* data) {
    handleNewValue();
    out.write(data, strlen(data));
}

void JSONOutputStream::flush() {
}

void JSONOutputStream::commitValue() {
    startNewValue = true;
}

JSONOutputStream& JSONOutputStream::formatValue(const char *fmt, ...) {
    std::va_list args;
    va_start(args, fmt);
    vformatValue(fmt, args);
    va_end(args);
    return *this;
}

JSONOutputStream& JSONOutputStream::vformatValue(const char *fmt, std::va_list list) {
    vformat(fmt, list);
    commitValue();
    return *this;
}

void JSONOutputStream::handleNewValue() {
    if(startNewValue) {
        const char str[] = ",";
        out.write(str, 1);
        startNewValue = false;
    }
}

JSONOutputStream& operator<<(JSONOutputStream& os, const Key& k) {
    return os.format("\"%s\": ", k.str);
}

} // namespace serialization
