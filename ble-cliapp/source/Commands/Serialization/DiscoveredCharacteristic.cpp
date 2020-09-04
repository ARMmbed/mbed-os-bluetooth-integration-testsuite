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
#include "DiscoveredCharacteristic.h"
#include "ble/common/UUID.h"
#include "Serialization/UUID.h"

serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const DiscoveredCharacteristic& characteristic) {
    using namespace serialization;

    return os << startObject <<
        key("UUID") << characteristic.getUUID() <<
        key("properties") << characteristic.getProperties() <<
        key("start_handle") << characteristic.getDeclHandle() <<
        key("value_handle") << characteristic.getValueHandle() <<
        key("end_handle") << characteristic.getLastHandle() <<
    endObject;

}

serialization::JSONOutputStream& operator<<(serialization::JSONOutputStream& os, const DiscoveredCharacteristic::Properties_t& properties) {
    using namespace serialization;

    os << startArray;

    if (properties.broadcast()) {
        os << "broadcast";
    }
    if (properties.read()) {
        os << "read";
    }
    if (properties.writeWoResp()) {
        os << "writeWoResp";
    }
    if (properties.write()) {
        os << "write";
    }
    if (properties.notify()) {
        os << "notify";
    }
    if (properties.indicate()) {
        os << "indicate";
    }
    if (properties.authSignedWrite()) {
        os << "authSignedWrite";
    }

    return os << endArray;
}
