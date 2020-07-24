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

#ifndef BLE_CLIAPP_SERIALIZER_H_
#define BLE_CLIAPP_SERIALIZER_H_

#include "util/ConstArray.h"
#include <cstring>

/**
 * @brief simple POD object which map a value to a string
 * @tparam T the type of value to map
 */
template<typename T>
struct ValueToStringMapping {
    T value;
    const char* str;
};

/**
 * @brief Description of the Serialization used by Serializer<T> to create the
 * serialization and deserialization function. This class is mostly used to
 * generate enum serialization/deserialization functions.
 *
 * @detail Two functions and one typedef should be defined in specialization of
 * this class to makes the serialization viable:
 *   - typedef T type: yield the type targeted by the description.
 *   - ConstArray<ValueToStringMapping<type> > mapping(): Mapping between value
 *   and serialized values.
 *   - const char* errorMessage(): Error message that should be return by the
 *   toString function if the serialization fail.
 *
 * @code

   enum Color_t {
     BLUE,
     RED,
     GREEN
   };

    template<>
    struct SerializerDescription<Color_t> {
        typedef Color_t type;

        static const ConstArray<ValueToStringMapping<type> > mapping() {
            static const ValueToStringMapping<type> map[] = {
                { BLUE, "blue" },
                { RED, "red" },
                { GREEN, "green" }
            };

            return makeConstArray(map);
        }

        static const char* errorMessage() {
            return "unknown Color_t";
        }
    };

    // deserialization
    Color_t c;
    if (fromString("blue", c) ) {
        // handle error
    }

    // serialization
    const char* sc = toString(c);

 * @endcode
 *
 * @tparam T The type on which this descripion apply
 */
template<typename T>
struct SerializerDescription;

/**
 * @brief simple serializer logic.
 *
 * @tparam T The type of the serialization. It require that SerializerDescription<T> exist
 */
template<typename T>
struct Serializer {
    typedef SerializerDescription<T> description;
    typedef typename SerializerDescription<T>::type serialized_type;

    static const char* toString(const serialized_type& val) {
        return toString(
            val,
            description::mapping(),
            description::errorMessage()
        );
    }

    static bool fromString(const char* str, serialized_type& val) {
        return fromString(
            str,
            val,
            description::mapping()
        );
    }

private:
    static const char* toString(serialized_type value, const ConstArray<ValueToStringMapping<serialized_type> >& map, const char* error_message) {
        for(std::size_t i = 0; i < map.count(); ++i) {
            if(map[i].value == value) {
                return map[i].str;
            }
        }
        return error_message;
    }

    static bool fromString(const char* str, serialized_type& value, const ConstArray<ValueToStringMapping<serialized_type> >& map) {
        for(std::size_t i = 0; i < map.count(); ++i) {
            if(std::strcmp(map[i].str, str) == 0) {
                value = map[i].value;
                return true;
            }
        }
        return false;
    }
};

/**
 * @brief convert a value to a string.
 * SerializerDescription<T> should be specialized to make this function viable.
 */
template<typename T>
static const char* toString(const T& value) {
    return Serializer<T>::toString(value);
}

/**
 * @brief Convert a string to a value
 * SerializerDescription<T> should be specialized to make this function viable.
 */
template<typename T>
static bool fromString(const char* str, T& value) {
    return Serializer<T>::fromString(str, value);
}

/**
 * @brief Convert a string to an int8_t
 *
 * @param[in] str The string to convert
 * @param[out] val The result of the conversion
 *
 * @return true if the conversion succeed and false otherwise
 */
bool fromString(const char* str, int8_t& val);

/**
 * @brief Convert a string to an uint8_t
 *
 * @param[in] str The string to convert
 * @param[out] val The result of the conversion
 *
 * @return true if the conversion succeed and false otherwise
 */
bool fromString(const char* str, uint8_t& val);

/**
 * @brief Convert a string to an uint16_t
 *
 * @param[in] str The string to convert
 * @param[out] val The result of the conversion
 *
 * @return true if the conversion succeed and false otherwise
 */
bool fromString(const char* str, uint16_t& val);

/**
 * @brief Convert a string to an uint32_t
 *
 * @param[in] str The string to convert
 * @param[out] val The result of the conversion
 *
 * @return true if the conversion succeed and false otherwise
 */
bool fromString(const char* str, unsigned int& val);
bool fromString(const char* str, long unsigned int& val);

/**
 * @brief Convert a string to a bool
 *
 * @param[in] str The string to convert
 * @param[out] val The result of the conversion
 *
 * @return true if the conversion succeed and false otherwise
 */
bool fromString(const char* str, bool& val);

#endif //BLE_CLIAPP_SERIALIZER_H_
