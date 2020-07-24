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
#include <stddef.h>
#include "BLECommonSerializer.h"
#include "GattCallbackParamTypes.h"
#include "Hex.h"

serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const GattReadCallbackParams& readResult) {
    using namespace serialization;
    os << startObject <<
        key("connection_handle") << readResult.connHandle <<
        key("attribute_handle") << readResult.handle <<
        key("offset") << readResult.offset <<
        key("status") << readResult.status <<
        key("length") << readResult.len <<
        key("data");
    return serializeRawDataToHexString(os, readResult.data, readResult.len) << endObject;
}

serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const GattWriteCallbackParams& writeResult) {
    using namespace serialization;

    os << startObject <<
        key("connection_handle") << writeResult.connHandle <<
        key("attribute_handle") << writeResult.handle <<
        key("offset") << writeResult.offset <<
        key("length") << writeResult.len <<
        key("write_operation_type") << writeResult.writeOp <<
        key("data");
    return serializeRawDataToHexString(os, writeResult.data, writeResult.len) << endObject;
}

serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, GattWriteCallbackParams::WriteOp_t writeOperation) {
    switch(writeOperation) {
        case GattWriteCallbackParams::OP_INVALID:
            return os << "OP_INVALID";
        case GattWriteCallbackParams::OP_WRITE_REQ:
            return os << "OP_WRITE_REQ";
        case GattWriteCallbackParams::OP_WRITE_CMD:
            return os << "OP_WRITE_CMD";
        case GattWriteCallbackParams::OP_SIGN_WRITE_CMD:
            return os << "OP_SIGN_WRITE_CMD";
        case GattWriteCallbackParams::OP_PREP_WRITE_REQ:
            return os << "OP_PREP_WRITE_REQ";
        case GattWriteCallbackParams::OP_EXEC_WRITE_REQ_CANCEL:
            return os << "OP_EXEC_WRITE_REQ_CANCEL";
        case GattWriteCallbackParams::OP_EXEC_WRITE_REQ_NOW:
            return os << "OP_EXEC_WRITE_REQ_NOW";
        default:
            return os << "invalid GattWriteCallbackParams::WriteOp_t operation";
    }
}
