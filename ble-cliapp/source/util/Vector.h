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

#ifndef BLE_CLIAPP_DYNAMIC_ARRAY_H_
#define BLE_CLIAPP_DYNAMIC_ARRAY_H_

#include <memory>
#include <cstring>
#include <algorithm>
#include <utility>

namespace container {

template<typename T>
class Vector : private std::allocator<T> {

public:
    typedef std::allocator<T> Allocator;
    typedef typename Allocator::pointer iterator;
    typedef typename Allocator::const_pointer const_iterator;

    Vector() : _data(0), _size(0), _capacity(0) { }

    Vector(const Vector& that) : std::allocator<T>(that), _data(Vector<T>::allocate(that._size)), _size(that._size), _capacity(that._size) {
        for(std::size_t i = 0; i < _size; ++i) {
            new(_data + i) T(that._data[i]);
        }
    }

    ~Vector() {
        for(std::size_t i = 0; i < _size; ++i) {
            (_data + i)->~T();
        }
        std::allocator<T>::deallocate(_data, _capacity);
    }

    Vector& operator=(Vector that) {
        std::swap(this->_data, that._data);
        std::swap(this->_size, that._size);
        std::swap(this->_capacity, that._capacity);
        return *this;
    }

    typename Allocator::size_type size() const {
        return _size;
    }

    typename Allocator::size_type capacity() const{
        return _capacity;
    }

    typename Allocator::reference operator[](std::size_t index) {
        return _data[index];
    }

    typename Allocator::const_reference operator[](std::size_t index) const {
        return _data[index];
    }

    void push_back(const T& value) {
        if(_size == _capacity) {
            std::size_t newCapacity = ((_capacity * 1618) / 1000) + 1;
            typename Allocator::pointer newData = std::allocator<T>::allocate(newCapacity);
            for(std::size_t i = 0; i < _size; ++i) {
                new (newData + i) T(_data[i]);
                (_data + i)->~T();
            }
            if(_data) {
                std::allocator<T>::deallocate(_data, _capacity);
            }
            _capacity = newCapacity;
            _data = newData;
        }

        new (_data + _size) T(value);
        ++_size;
    }

    iterator begin() {
        return _data;
    }

    const_iterator cbegin() const {
        return _data;
    }

    iterator end() {
        return _data + _size;
    }

    const_iterator cend() const {
        return _data + _size;
    }

    friend bool operator==(const Vector& lhs, const Vector& rhs) {
        if(lhs._size != rhs._size) {
            return false;
        }

        for(std::size_t i = 0; i < lhs._size; ++i) {
            if(lhs._data[i] != rhs._data[i]) {
                return false;
            }
        }
        return true;
    }

    friend bool operator!=(const Vector& lhs, const Vector& rhs) {
        return !(lhs == rhs);
    }

private:
    typename Allocator::pointer _data;
    typename Allocator::size_type _size;
    typename Allocator::size_type _capacity;
};

} // namespace container

#endif //BLE_CLIAPP_DYNAMIC_ARRAY_H_
