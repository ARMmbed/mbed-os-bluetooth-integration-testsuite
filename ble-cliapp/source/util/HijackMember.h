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

#ifndef BLE_CLIAPP_UTIL_HIJACK_MEMBER_H_
#define BLE_CLIAPP_UTIL_HIJACK_MEMBER_H_

/**
 * @brief provide access to a private class member function or member data
 * @details It is possible to explicitly instantiate a template class with a
 * private pointer to member. When such instantiation occur, a friend function
 * getMember is instantiated. This function will return the pointer to member
 * used at the instantiation.
 *
 * @tparam MemberAccessor Act as a key to resolve the correct overload of getMember
 * @tparam MemberType The type of the pointer to member that will be hijacked
 * @tparam Member The pointer to member hijacked
 */
template<typename MemberAccessor, typename MemberType, MemberType Member>
struct hijacked_member_accessor {
    friend MemberType getMember(MemberAccessor) {
        return Member;
    }
};

/**
 * @brief Provide an accessors to a pointer to member, even if this one is private
 *
 * @param ACCESSOR name of the accessor to create
 * @param MEMBER_TYPE type of the pointer to member
 * @param MEMBER the pointer to member hijacked
 */
#define HIJACK_MEMBER(ACCESSOR, MEMBER_TYPE, MEMBER) \
    struct ACCESSOR##_CLASS__ { \
            ACCESSOR##_CLASS__() { } \
            friend MEMBER_TYPE getMember(ACCESSOR##_CLASS__);\
        };\
    template struct hijacked_member_accessor<ACCESSOR##_CLASS__, MEMBER_TYPE, MEMBER>; \
    static MEMBER_TYPE const volatile ACCESSOR = getMember(ACCESSOR##_CLASS__())



#endif //BLE_CLIAPP_UTIL_HIJACK_MEMBER_H_
