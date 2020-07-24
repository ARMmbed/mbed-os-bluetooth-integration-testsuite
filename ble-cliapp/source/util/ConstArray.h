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

#ifndef BLE_CLIAPP_CONST_ARRAY_H_
#define BLE_CLIAPP_CONST_ARRAY_H_

#include <cstddef>
#include <stdint.h>

template<typename T>
struct ConstArray {

public:
    ConstArray() :
        _count(0), _elements(NULL) {
    }

    /**
     * Construct a new array of elements from a size and pointer to an array
     */
    ConstArray(int count, const T* elements) :
        _count(count), _elements(elements) {
    }

    template<std::size_t Elementscount>
    ConstArray(const T (&elements)[Elementscount]) :
        _count(Elementscount), _elements(elements) {
    }

    /**
     * return the count of arguments available
     */
    std::size_t count() const {
        return _count;
    }

    /**
     * return the element at the given index
     */
    const T& operator[](std::size_t index) const {
        return _elements[index];
    }

    ConstArray drop(std::size_t countTodrop) const {
        return (countTodrop < _count) ? ConstArray(_count - countTodrop, _elements + countTodrop) : ConstArray();
    }

private:
    const std::size_t _count;
    const T* _elements;
};

template<typename T, std::size_t Elementscount>
ConstArray<T> makeConstArray(const T (&elements)[Elementscount]) {
    return ConstArray<T>(elements);
}

#endif //BLE_CLIAPP_CONST_ARRAY_H_
