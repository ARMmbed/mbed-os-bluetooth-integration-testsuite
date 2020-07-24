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

#ifndef BLE_CLIAPP_UTIL_CIRCULARBUFFER_H
#define BLE_CLIAPP_UTIL_CIRCULARBUFFER_H

#include <algorithm>

namespace util {

/** Templated Circular buffer class
 */
template<typename T, size_t BufferSize, typename CounterType = uint32_t>
class CircularBuffer {

public:
    CircularBuffer() : _head(0), _tail(0), _full(false) {
    }

    /**
     * Push the transaction to the buffer. This operation will fail if there is
     * no room left in the buffer.
     *
     * @param data Data to be pushed to the buffer
     * @return true if the operation succeed and false otherwise
     */
    bool push(const T& data) {
        if (full()) {
            return false;
        }

        _buffer[_head] = data;
        _head = incrementCounter(_head);

        if (_head == _tail) {
            _full = true;
        }

        return true;
    }

    /** Pop the transaction from the buffer
     *
     * @param data Data to be pushed to the buffer
     * @return True if the buffer is not empty and data contains a transaction, false otherwise
     */
    bool pop(T& data) {
        if(empty()) {
            return false;
        }

        data = _buffer[_tail];
        _tail = incrementCounter(_tail);
        _full = false;
        return true;
    }

    /**
     * @brief pop multiples elements from the buffer
     *
     * @param dest The array which will received the elements
     * @param len The number of elements to pop
     *
     * @return The number of elements pop
     */
    CounterType pop(T* dest, CounterType len) {
        if(empty()) {
            return 0;
        }

        if(_tail < _head) {
            // truncation if the count if there is not enough elements available
            if((_tail + len) > _head) {
                len = _head - _tail;
            }
            std::copy(_buffer + _tail, _buffer + _tail + len, dest);
            _tail += len;
            return len;
        } else {
            if((_tail + len ) <= BufferSize) {
                std::copy(_buffer + _tail, _buffer + _tail + len, dest);
                _tail += len;
                _tail = _tail % BufferSize;
                _full = false;
                return len;
            } else {
                // composition of previous operations
                CounterType firstChunk = pop(dest, BufferSize - _tail);
                return firstChunk + pop(dest + firstChunk, len - firstChunk);
            }
        }
    }

    /**
     * @brief pop multiples elements from the buffer
     *
     * @param dest The array which will received the elements
     *
     * @return The number of elements pop
     */
    template<CounterType N>
    CounterType pop(T (&dest)[N]) {
        return pop(dest, N);
    }

    /** Check if the buffer is empty
     *
     * @return True if the buffer is empty, false if not
     */
    bool empty() const {
        return (_head == _tail) && !_full;
    }

    /** Check if the buffer is full
     *
     * @return True if the buffer is full, false if not
     */
    bool full() const {
        return _full;
    }

    /**
     * Reset the buffer
     */
    void reset() {
        _head = 0;
        _tail = 0;
        _full = false;
    }

private:
    CounterType incrementCounter(CounterType val) {
        return (++val) % BufferSize;
    }

    T _buffer[BufferSize];
    CounterType _head;
    CounterType _tail;
    bool _full;
};

} // namespace util

#endif /* BLE_CLIAPP_UTIL_CIRCULARBUFFER_H */
