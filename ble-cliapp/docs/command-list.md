# Commands

Invoking a command is simple: 

```
<module> <command> [arguments]
```

Like mbed BLE, the shell is divided in modules which contain a set of commands, 
the following modules are accessible: 

* `ble`: Model the class `BLE`
* `gap`: Model the class `Gap`
* `gattServer`: Model the class `GattServer`
* `gattClient`: Model the class `GattClient`
* `securityManager`: Model the class `SecurityManager`

When the command as completed, the result is printed on the terminal, in a `json` 
object. This object contains the following properties: 

* `status`: returned status, if the result is equal to 0 then the command has 
been correctly executed. A negative value means that the command execution has 
failed. 
* `error`: This key will be present only if the command execution has failed. 
The value associated is a `json` value (generally a string).
* `result`: This key is present if the command has succeeded and there is a 
meaningful result to report to the user. The value associated is a `json` value, 
its format depend on the command invoked.

## ble module

The `ble` module expose functions from the class `BLE`:

### init
Initialize the ble API and underlying BLE stack.

* invocation: `ble init`
* arguments: None 
* result: None
* modeled after: `BLE::init`

### shutdown
Shutdown the current BLE instance, calling ble related function after this call 
may lead to faillure.

* invocation: `ble shutdown`
* arguments: None 
* result: None
* modeled after: `BLE::shutdown`

### reset
Reset the ble stack. If the stack is initialized, it will shut it down prior to 
initializing it again.

* invocation: `ble reset`
* arguments: None 
* result: None

### getVersion
Return the version of the BLE API.

* invocation: `ble getVersion`
* modeled after: `BLE::getVersion`
* arguments: None 
* result: The version of the underlying stack as a string.


## gap module

The `gap` module expose functions from the class `Gap`:


### getAddress

* invocation: `gap getAddress`
* description: Get the address and the type of address of this device
* arguments: None
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`OwnAddressType`](#ownaddresstype) **address_type**: Type of the address
  - [`MacAddress`](#macaddress) **address**: The address of the device
* modeled after: `Gap::getAddress`


### getMaxWhitelistSize

* invocation: `gap getMaxWhitelistSize`
* description: get the maximum size the whitelist can take
* arguments: None
* result: None
* modeled after: `Gap::getMaxWhitelistSize`


### getWhitelist

* invocation: `gap getWhitelist`
* description: Get the internal whitelist to be used by the Link Layer when scanning advertising or initiating a connection depending on the filter policies.
* arguments: None
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`AddressType`](#addresstype) **[i].address_type**: Type of the address
  - [`MacAddress`](#macaddress) **[i].address**: The mac address
* modeled after: `Gap::getWhitelist`


### setWhitelist

* invocation: `gap setWhitelist`
* description: Set the internal whitelist to be used by the Link Layer when scanning advertising or initiating a connection depending on the filter policies.
* arguments: None
* result: None
* modeled after: `Gap::setWhitelist`


### enablePrivacy

* invocation: `gap enablePrivacy`
* description: Enable or disable the privacy
* arguments:
  - [`bool`](#bool) **enable**: Enable or disable the privacy
* result: None
* modeled after: `Gap::enablePrivacy`


### setPeripheralPrivacyConfiguration

* invocation: `gap setPeripheralPrivacyConfiguration`
* description: Set the peripheral privacy configuration.
* arguments:
  - [`bool`](#bool) **use_non_resolvable_random_address**: Use non resolvable address in non connectable advertisements
  - [`PeripheralPrivacyResolutionStrategy`](#peripheralprivacyresolutionstrategy) **resolution_strategy**: Strategy used to resolve addresses present in scan and connection requests.
* result: None
* modeled after: `Gap::setPeripheralPrivacyConfiguration`


### getPeripheralPrivacyConfiguration

* invocation: `gap getPeripheralPrivacyConfiguration`
* description: Get the peripheral privacy configuration.
* arguments: None
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`bool`](#bool) **use_non_resolvable_random_address**: Indicates if non resolvable addresses are used in non connectable advertisements.
  - [`PeripheralPrivacyResolutionStrategy`](#peripheralprivacyresolutionstrategy) **resolution_strategy**: Strategy used to resolve address in scan and connection requests.
* modeled after: `Gap::getPeripheralPrivacyConfiguration`


### setCentralPrivacyConfiguration

* invocation: `gap setCentralPrivacyConfiguration`
* description: Set the central privacy configuration.
* arguments:
  - [`bool`](#bool) **use_non_resolvable_random_address**: Use non resolvable address in scan requests.
  - [`CentralPrivacyResolutionStrategy`](#centralprivacyresolutionstrategy) **resolution_strategy**: Strategy used to resolve addresses present in advertisement packets.
* result: None
* modeled after: `Gap::setCentralPrivacyConfiguration`


### getCentralPrivacyConfiguration

* invocation: `gap getCentralPrivacyConfiguration`
* description: Get the central privacy configuration.
* arguments: None
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`bool`](#bool) **use_non_resolvable_random_address**: Indicates if non resolvable addresses are used in scan request.
  - [`CentralPrivacyResolutionStrategy`](#centralprivacyresolutionstrategy) **resolution_strategy**: Strategy used to resolve addresses in advertisements.
* modeled after: `Gap::getCentralPrivacyConfiguration`


### readPhy

* invocation: `gap readPhy`
* description: Read current PHY of the connection.
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: The handle of the connection queried
* result: None
* modeled after: `Gap::readPhy`


### setPhy

* invocation: `gap setPhy`
* description: Set PHY preference for given connection.
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: The handle of the connection queried
  - [`uint8_t`](#uint8_t) **tx_phy**: Preferred tx PHYs mask
  - [`uint8_t`](#uint8_t) **rx_phy**: Preferred rx PHYs mask
  - [`uint8_t`](#uint8_t) **coded_symbol**: Preferred types of coding
* result: None
* modeled after: `Gap::setPhy`


### setPreferredPhys

* invocation: `gap setPreferredPhys`
* description: Set PHY preference for all connections.
* arguments:
  - [`uint8_t`](#uint8_t) **tx phy**: Preferred tx PHYs mask
  - [`uint8_t`](#uint8_t) **rx phy**: Preferred rx PHYs mask
* result: None
* modeled after: `Gap::setPreferredPhys`


### getMaxAdvertisingSetNumber

* invocation: `gap getMaxAdvertisingSetNumber`
* arguments: None
* result: None
* modeled after: `Gap::getMaxAdvertisingSetNumber`


### getMaxAdvertisingDataLength

* invocation: `gap getMaxAdvertisingDataLength`
* arguments: None
* result: None
* modeled after: `Gap::getMaxAdvertisingDataLength`


### createAdvertisingSet

* invocation: `gap createAdvertisingSet`
* arguments: None
* result: None
* modeled after: `Gap::createAdvertisingSet`


### destroyAdvertisingSet

* invocation: `gap destroyAdvertisingSet`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::destroyAdvertisingSet`


### setAdvertisingParameters

* invocation: `gap setAdvertisingParameters`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::setAdvertisingParameters`


### setAdvertisingPayload

* invocation: `gap setAdvertisingPayload`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
  - [`HexString`](#hexstring) **data**: 
* result: None
* modeled after: `Gap::setAdvertisingPayload`


### applyAdvPayloadFromBuilder

* invocation: `gap applyAdvPayloadFromBuilder`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: Advertising set (legacy = 0) to apply builder data to.
* result: None
* modeled after: `Gap::applyAdvPayloadFromBuilder`


### setAdvertisingScanResponse

* invocation: `gap setAdvertisingScanResponse`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
  - [`HexString`](#hexstring) **data**: 
* result: None
* modeled after: `Gap::setAdvertisingScanResponse`


### applyScanRespFromBuilder

* invocation: `gap applyScanRespFromBuilder`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: Advertising set (legacy = 0) to apply builder data to.
* result: None
* modeled after: `Gap::applyScanRespFromBuilder`


### startAdvertising

* invocation: `gap startAdvertising`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
  - [`AdvDuration`](#advduration) **duration**: 
  - [`uint8_t`](#uint8_t) **maxEvent**: 
* result: None
* modeled after: `Gap::startAdvertising`


### stopAdvertising

* invocation: `gap stopAdvertising`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::stopAdvertising`


### isAdvertisingActive

* invocation: `gap isAdvertisingActive`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::isAdvertisingActive`


### setPeriodicAdvertisingParameters

* invocation: `gap setPeriodicAdvertisingParameters`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
  - [`PeriodicInterval`](#periodicinterval) **min**: 
  - [`PeriodicInterval`](#periodicinterval) **max**: 
  - [`bool`](#bool) **advertiseTxPower**: 
* result: None
* modeled after: `Gap::setPeriodicAdvertisingParameters`


### setPeriodicAdvertisingPayload

* invocation: `gap setPeriodicAdvertisingPayload`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
  - [`HexString`](#hexstring) **data**: 
* result: None
* modeled after: `Gap::setPeriodicAdvertisingPayload`


### startPeriodicAdvertising

* invocation: `gap startPeriodicAdvertising`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::startPeriodicAdvertising`


### stopPeriodicAdvertising

* invocation: `gap stopPeriodicAdvertising`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::stopPeriodicAdvertising`


### isPeriodicAdvertisingActive

* invocation: `gap isPeriodicAdvertisingActive`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::isPeriodicAdvertisingActive`


### setScanParameters

* invocation: `gap setScanParameters`
* arguments: None
* result: None
* modeled after: `Gap::setScanParameters`


### startScan

* invocation: `gap startScan`
* arguments:
  - [`ScanDuration`](#scanduration) **duration**: 
  - [`DuplicatesFilter`](#duplicatesfilter) **filter**: 
  - [`ScanPeriod`](#scanperiod) **period**: 
* result: None
* modeled after: `Gap::startScan`


### scanForAddress

* invocation: `gap scanForAddress`
* arguments:
  - [`MacAddress`](#macaddress) **peer_address**: 
  - [`uint32_t`](#uint32_t) **timeout**: 
* result: None


### scanForData

* invocation: `gap scanForData`
* description: This will return only advertising reports of devices that advertise the given data payload.
* arguments:
  - [`MacAddress`](#macaddress) **peer_address**: 
  - [`uint32_t`](#uint32_t) **timeout**:
  - [`HexString`](#hexstring) **value**: The value of the advertising data payload to scan for. 
* result: None


### stopScan

* invocation: `gap stopScan`
* arguments: None
* result: None
* modeled after: `Gap::stopScan`


### createSync

* invocation: `gap createSync`
* arguments:
  - [`AddressType`](#addresstype) **peerAddressType**: 
  - [`MacAddress`](#macaddress) **peerAddress**: 
  - [`uint8_t`](#uint8_t) **sid**: 
  - [`uint16_t`](#uint16_t) **maxPacketSkip**: 
  - [`SyncTimeout`](#synctimeout) **timeout**: 
* result: None
* modeled after: `Gap::createSync`


### createSyncFromList

* invocation: `gap createSyncFromList`
* arguments:
  - [`uint16_t`](#uint16_t) **maxPacketSkip**: 
  - [`SyncTimeout`](#synctimeout) **timeout**: 
* result: None
* modeled after: `Gap::createSyncFromList`


### cancelCreateSync

* invocation: `gap cancelCreateSync`
* arguments: None
* result: None
* modeled after: `Gap::cancelCreateSync`


### terminateSync

* invocation: `gap terminateSync`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::terminateSync`


### addDeviceToPeriodicAdvertiserList

* invocation: `gap addDeviceToPeriodicAdvertiserList`
* arguments:
  - [`AddressType`](#addresstype) **peerAddressType**: 
  - [`MacAddress`](#macaddress) **peerAddress**: 
  - [`uint16_t`](#uint16_t) **sid**: 
* result: None
* modeled after: `Gap::addDeviceToPeriodicAdvertiserList`


### removeDeviceFromPeriodicAdvertiserList

* invocation: `gap removeDeviceFromPeriodicAdvertiserList`
* arguments:
  - [`AddressType`](#addresstype) **peerAddressType**: 
  - [`MacAddress`](#macaddress) **peerAddress**: 
  - [`uint16_t`](#uint16_t) **sid**: 
* result: None
* modeled after: `Gap::removeDeviceFromPeriodicAdvertiserList`


### clearPeriodicAdvertiserList

* invocation: `gap clearPeriodicAdvertiserList`
* arguments: None
* result: None
* modeled after: `Gap::clearPeriodicAdvertiserList`


### getMaxPeriodicAdvertiserListSize

* invocation: `gap getMaxPeriodicAdvertiserListSize`
* arguments: None
* result: None
* modeled after: `Gap::getMaxPeriodicAdvertiserListSize`


### connect

* invocation: `gap connect`
* arguments:
  - [`AddressType`](#addresstype) **peerAddressType**: 
  - [`MacAddress`](#macaddress) **peerAddress**: 
* result: None
* modeled after: `Gap::connect`


### waitForConnection

* invocation: `gap waitForConnection`
* arguments:
  - [`uint32_t`](#uint32_t) **timeout**: 
* result: None
* modeled after: `Gap::waitForConnection`


### cancelConnect

* invocation: `gap cancelConnect`
* description: This will attempt to cancel a connection process if it's still ongoing. This does not guarantee success
and a connection may or may not be created. If successful, this will produce a connection event showing a failed
connection. Will not drop a connection if it already has been established. 
* arguments:
  - [`AddressType`](#addresstype) **peerAddressType**: The address type of the connection to be canceled
  - [`MacAddress`](#macaddress) **peerAddress**: There can be many connection attempts, we need to specify the address
  so that the connection block can be recovered
* result: None
* modeled after: `Gap::cancelConnect`


### updateConnectionParameters

* invocation: `gap updateConnectionParameters`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
  - [ConnInterval`](#conninterval) **minConnectionInterval**: 
  - [ConnInterval`](#conninterval) **maxConnectionInterval**: 
  - [`uint16_t`](#uint16_t) **slaveLatency**: 
  - [`SupervisionTimeout`](#supervisiontimeout) **supervision_timeout**: 
* result: None
* modeled after: `Gap::updateConnectionParameters`


### manageConnectionParametersUpdateRequest

* invocation: `gap manageConnectionParametersUpdateRequest`
* arguments:
  - [`bool`](#bool) **manage**: 
* result: None
* modeled after: `Gap::manageConnectionParametersUpdateRequest`


### acceptConnectionParametersUpdate

* invocation: `gap acceptConnectionParametersUpdate`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
  - [ConnInterval`](#conninterval) **minConnectionInterval**: 
  - [ConnInterval`](#conninterval) **maxConnectionInterval**: 
  - [`uint16_t`](#uint16_t) **slaveLatency**: 
  - [`SupervisionTimeout`](#supervisiontimeout) **supervision_timeout**: 
* result: None
* modeled after: `Gap::acceptConnectionParametersUpdate`


### rejectConnectionParametersUpdate

* invocation: `gap rejectConnectionParametersUpdate`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
* result: None
* modeled after: `Gap::rejectConnectionParametersUpdate`


### disconnect

* invocation: `gap disconnect`
* arguments:
  - [`uint16_t`](#uint16_t) **handle**: 
  - [`LocalDisconnectionReason](#localdisconnectionreason) **reason**: 
* result: None
* modeled after: `Gap::disconnect`


### isRadioActive

* invocation: `gap isRadioActive`
* description: This is not part of Gap API and relies on implementation detail but is useful for tests.
* arguments: None 
* result: 
  - [`bool`](#bool) **result**: True if controller needs radio (there's scanning, advertising, connecting), false if it's idle. 
* modeled after: `impl::Gap::isRadioActive`


## gattClient module

The `gattClient` module expose the following functions from the class 
`GattClient`:


### discoverAllServicesAndCharacteristics

* invocation: `gattClient discoverAllServicesAndCharacteristics <connection_handle>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used by 
   the procedure.
* result: A JSON array of the discovered service. Each service is a JSON object 
which contains the following fields:
  - [`UUID`](#uuid) **UUID**: The UUID of the service.
  - [`uint16_t`](#uint16_t) **start_handle**: The first attribute handle of the 
  service.
  - [`uint16_t`](#uint16_t) **end_handle**: The last attribute handle of the 
  service.
  - **characteristics**: an array of JSON objects describing the characteristics 
  of the service. Each characteristics expose the following attributes:
    + [`UUID`](#uuid) **UUID**: The UUID of the characteristic.
    + **properties**: JSON array of the properties of the characteristics, each 
    property is modeled as a string:
      * "broadcast"
      * "read"
      * "writeWoResp"
      * "write"
      * "notify"
      * "indicate"
      * "authSignedWrite"
    + [`uint16_t`](#uint16_t) **start_handle**: The first attribute handle of the 
    characteristic.
    + [`uint16_t`](#uint16_t) **value_handle**: The handle used to retrieve the 
    value of the characteristic.
    + [`uint16_t`](#uint16_t) **end_handle**: The last attribute handle of the 
    characteristic.
* modeled after: `GattClient::launchServiceDiscovery`, 
`GattClient::onServiceDiscoveryTermination`.


### discoverAllServices

* invocation: `gattClient discoverAllServices <connection_handle>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used 
   by the procedure.
* result: A JSON array of the discovered service. Each service is a JSON object 
which contains the following fields:
  - [`UUID`](#uuid) **UUID**: The UUID of the service.
  - [`uint16_t`](#uint16_t) **start_handle**: The first attribute handle of the 
  service.
  - [`uint16_t`](#uint16_t) **end_handle**: The last attribute handle of the 
  service.
* modeled after: `GattClient::launchServiceDiscovery`, 
`GattClient::onServiceDiscoveryTermination`.



### discoverPrimaryServicesByUUID

* invocation: `gattClient discoverPrimaryServicesByUUID <connection_handle> 
<service_UUID>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used 
   by the procedure.
   - [`UUID`](#uuid) **service_UUID**: The UUID of the service to discover.
* result: A JSON array of the discovered service. Each service is a JSON object 
which contains the following fields:
  - [`UUID`](#uuid) **UUID**: The UUID of the service.
  - [`uint16_t`](#uint16_t) **start_handle**: The first attribute handle of the 
  service.
  - [`uint16_t`](#uint16_t) **end_handle**: The last attribute handle of the 
  service.
* modeled after: `GattClient::launchServiceDiscovery`, 
`GattClient::onServiceDiscoveryTermination`.



### discoverAllCharacteristicsDescriptors

* invocation: `gattClient discoverAllCharacteristicsDescriptors 
<connection_handle> <char_start> <char_end>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used 
   by the procedure.
   - [`uint16_t`](#uint16_t) **char_start**: The first attribute handle of the 
   characteristic targeted by the operation.
   - [`uint16_t`](#uint16_t) **char_end**: The last attribute handle of the 
   characteristic targeted by the operation.
* result: A JSON array of the discovered descriptors. Each discovered descriptor 
is a JSON object which contains the following fields:
  - [`uint16_t`](#uint16_t) **handle**: Attribute handle of the descriptor.
  - [`UUID`](#uuid) **UUID**: The UUID of the characteristic descriptor.
* modeled after: `GattClient::discoverCharacteristicDescriptors`



### readCharacteristicValue

* invocation: `gattClient readCharacteristicValue <connection_handle> <char_value_handle>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used by 
   the procedure.
   - [`uint16_t`](#uint16_t) **char_value_handle**: The attribute handle of the 
   value to read.
* result: The value of the characteristic, as an [`HexString`](#hexstring).
* modeled after: `GattClient::read` and `GattClient::onDataRead`


### writeWithoutResponse

* invocation: `gattClient writeWithoutResponse <connection_handle> <char_value_handle> <value>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used by 
   the procedure.
   - [`uint16_t`](#uint16_t) **char_value_handle**: The attribute handle of the 
   value to read.
   - [`HexString`](#hexstring) **value**: The value to write in the characteristic.
* result: None
* modeled after: `GattClient::write` and `GattClient::onDataWritten`



### write

* invocation: `gattClient write <connection_handle> <char_value_handle> <value>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used by 
   the procedure.
   - [`uint16_t`](#uint16_t) **char_value_handle**: The attribute handle of the 
   value to write.
   - [`HexString`](#hexstring) **value**: The value to write in the characteristic.
* result: None
* modeled after: `GattClient::write` and `GattClient::onDataWritten`


### readCharacteristicDescriptor

* invocation: `gattClient readCharacteristicDescriptor <connection_handle> <descriptor_handle>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used by 
   the procedure.
   - [`uint16_t`](#uint16_t) **descriptor_handle**: The attribute handle of the 
   descriptor to read.
* result: The value of the descriptor, as an [`HexString`](#hexstring).
* modeled after: `GattClient::read` and `GattClient::onDataRead`


### writeCharacteristicDescriptor

* invocation: `gattClient writeCharacteristicDescriptor <connection_handle> <descriptor_handle> <value>`
* arguments: 
   - [`uint16_t`](#uint16_t) **connection_handle**: The connection handle used by 
   the procedure.
   - [`uint16_t`](#uint16_t) **descriptor_handle**: The attribute handle of the 
   descriptor to write.
   - [`HexString`](#hexstring) **value**: The value to write in the descriptor.
* result: None
* modeled after: `GattClient::write` and `GattClient::onDataWritten`


### listenHVX

* invocation: `gattClient listenHVX <timeout>`
* arguments: 
   - [`uint16_t`](#uint16_t) **timeout**: Time to listen to server notification 
   or indication.
* result: The list of notifications or indication received from the server. 
Each record contains the following attributes: 
  - [`uint16_t`](#uint16_t) **connHandle**: The reference of the connection to 
  the GATT server which has issued the event.
  - [`uint16_t`](#uint16_t) **handle**: The GATT attribute which has initiated 
  the event.
  - [`HVXType_t`](#hvxtype_t) **type**: Type of the event (notification or 
  indication).
  - [`HexString`](#hexstring) **data**: Payload of the event.
* modeled after: `GattClient::onHVX` 



## gattServer module

The `gattServer` module allows a user to access the capabilities of the 
GattServer class but building a service is a bit different with this module than 
it is with the original class. Rather than registering a service all at once, 
user can split the declaration in multiple sub operations then commit the result 
in the GattServer when it is ready.

### declareService

* invocation: `gattServer declareService <service_uuid>`
* description: Start the declaration of a service, after this call, user can call:
  - `declareCharacteristic` to declare a characteristic inside the service, 
  - `commitService` to commit the service or 
  - `cancelServiceDeclaration` to cancel the service declaration.
* arguments: 
   - [`UUID`](#uuid) **service_uuid**: The UUID of the service declared.
* result: None


### declareCharacteristic

* invocation: `gattServer declareCharacteristic <characteristic_uuid>`
* description: Start the declaration of a characteristic, after this call, user 
can call:
  - `setCharacteristicValue` to set the value of this characteristic
  - `setCharacteristicProperties` to set the properties of this characteristic
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to add a descriptor inside this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: 
   - [`UUID`](#uuid) **characteristic_uuid**: The UUID of the characteristic 
   declared.
* result: None


### setCharacteristicValue

* invocation: `gattServer setCharacteristicValue <value>`
* description: Set the value of the characteristic being declared. After this 
call, user can call:
  - `setCharacteristicValue` to set the value of this to another value.
  - `setCharacteristicProperties` to set the properties of this characteristic
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to add a descriptor inside this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: 
   - [`HexString`](#hexstring) **value**: The value of the characteristic 
   declared.
* result: None



### setCharacteristicProperties

* invocation: `gattServer setCharacteristicProperties [properties]`
* description: Set the properties of the characteristic being declared. After 
this call, user can call:
  - `setCharacteristicValue` to set the value of this characteristic
  - `setCharacteristicProperties` to set the properties of this characteristic 
  to another value.
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to add a descriptor inside this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: A list of string mapping to the properties to set: 
  - "broadcast"
  - "read"
  - "writeWoResp"
  - "write"
  - "notify"
  - "indicate"
  - "authSignedWrite" 
* result: None


### setCharacteristicVariableLength

* invocation: `gattServer setCharacteristicVariableLength <enable>`
* description: Set a boolean value which indicate if the characteristic has a 
variable length. If the characteristic has a variable length then 
`setCharacteristicMaxLength`  could be call to bound the length to a maximum.
After this call, user can call:
  - `setCharacteristicValue` to set the value of this characteristic
  - `setCharacteristicProperties` to set the properties of this characteristic.
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to add a descriptor inside this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: 
   - [`bool`](#bool) **enable**: True to enable variable length and false to 
   enable fixed length.
* result: None


### setCharacteristicMaxLength

* invocation: `gattServer setCharacteristicMaxLength <max_length>`
* description: Set the maximum length of the value of the characteristic being 
declared.
After this call, user can call:
  - `setCharacteristicValue` to set the value of this characteristic
  - `setCharacteristicProperties` to set the properties of this characteristic.
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to add a descriptor inside this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: 
   - [`uint16_t`](#uint16_t) **max_length**: The max length that the characteristic 
   value can occupy.
* result: None


### declareDescriptor

* invocation: `gattServer declareDescriptor <descriptor_uuid>`
* description: Start the declaration of a descriptor which will be attached to 
the characteristic being declared.
After this call, user can call:
  - `setCharacteristicValue` to set the value of this characteristic
  - `setDescriptorVariableLength`  enable or disable variable length value of 
  this descriptor.
  - `setDescriptorMaxLength` set the maximum length that is allowed for the 
  value of this descriptor.
  - `setCharacteristicProperties` to set the properties of this characteristic.
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to start the declaration of another descriptor inside 
  this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: 
   - [`UUID`](#uuid) **max_length**: The max length that the characteristic 
   value can occupy.
* result: None


### setDescriptorValue

* invocation: `gattServer setDescriptorValue <value>`
* description: Set the value of the descriptor being declared. After this call, 
user can call:
  - `setDescriptorVariableLength`  enable or disable variable length value of 
  this descriptor. 
  - `setDescriptorMaxLength` set the maximum length that is allowed for the 
  value of this descriptor.
  - `setDescriptorValue` to set the value of this to another value.
  - `setCharacteristicProperties` to set the properties of this characteristic
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to add a descriptor inside this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: 
   - [`HexString`](#hexstring) **value**: The value of the descriptor declared.
* result: None


### setDescriptorVariableLength

* invocation: `gattServer setDescriptorVariableLength <enable>`
* description: Set a boolean value which indicate if the descriptor has a 
variable length. If the descriptor has a variable length then 
`setDescriptorMaxLength` could be call to bound the length to a maximum.
After this call, user can call:
  - `setDescriptorVariableLength`  enable or disable variable length value of 
  this descriptor. 
  - `setDescriptorMaxLength` set the maximum length that is allowed for the 
  value of this descriptor.
  - `setDescriptorValue` to set the value of this to another value.
  - `setCharacteristicValue` to set the value of this characteristic
  - `setCharacteristicProperties` to set the properties of this characteristic.
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to add a descriptor inside this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: 
   - [`bool`](#bool) **enable**: True to enable variable length and false to enable 
   fixed length.
* result: None


### setDescriptorMaxLength

* invocation: `gattServer setDescriptorMaxLength <max_length>`
* description: Set the maximum length of the value of the descriptor being 
declared.
After this call, user can call:
  - `setDescriptorVariableLength`  enable or disable variable length value of 
  this descriptor. 
  - `setDescriptorMaxLength` set the maximum length that is allowed for the 
  value of this descriptor.
  - `setDescriptorValue` to set the value of this to another value.
  - `setCharacteristicValue` to set the value of this characteristic
  - `setCharacteristicProperties` to set the properties of this characteristic.
  - `setCharacteristicVariableLength` enable or disable variable length value of 
  this characteristic.
  - `setCharacteristicMaxLength` set the maximum length that is allowed for the 
  value of this characteristic.
  - `declareCharacteristic` to declare another characteristic inside the service
  - `declareDescriptor` to add a descriptor inside this characteristic
  - `commitService` to commit the service 
  - `cancelServiceDeclaration` to cancel the service declaration 
* arguments: 
   - [`uint16_t`](#uint16_t) **max_length**: The max length that the descriptor 
   value can occupy.
* result: None


### commitService

* invocation: `gattServer commitService`
* description: Commit in the GattServer the service declared. After this call, 
the ongoing service declaration is reset.
* arguments: None
* result: A JSON object describing the service in the ATT DB. It contains the 
following fields:
  - [`UUID`](#uuid) **UUID**: The uuid of the service
  - [`uint16_t`](#uint16_t) **handle**: The attribute handle of the service. 
  - "characteristics": A JSON array of the characteristics presents in the 
  service. Each characteristic is a JSON object containing the following fields:
    + [`UUID`](#uuid) **UUID**: The uuid of the characteristic.
    + [`uint16_t`](#uint16_t) **value_handle**: The attribute handle of the value 
    of the characteristic. 
    + "properties": A JSON array of the properties of the characteristic. The 
    possible values are:
      * "broadcast"
      * "read"
      * "writeWoResp"
      * "write"
      * "notify"
      * "indicate"
      * "authSignedWrite" 
    + [`uint16_t`](#uint16_t) **length**: The length of the value of the 
    characteristic.
    + [`uint16_t`](#uint16_t) **max_length**: The maximum length that the value 
    of the characteristic can take.
    + [`bool`](#bool) **has_variable_length**: Flag indicating if the length of 
    the characteristic is fixed or variable.
    + [`HexString`](#hexstring) **value**: The value of the characteristic.
    + "descriptors": A JSON array containing the descriptors of the 
    characteristic:
      * [`UUID`](#uuid) **UUID**: The uuid of the descriptor.
      * [`uint16_t`](#uint16_t) **handle**: The attribute handle of the descriptor. 
      * [`uint16_t`](#uint16_t) **length**: The length of the value of the descriptor.
      * [`uint16_t`](#uint16_t) **max_length**: The maximum length that the value 
      of the descriptor can take.
      * [`bool`](#bool) **has_variable_length**: Flag indicating if the length 
      of the descriptor is fixed or variable
      * [`HexString`](#hexstring) **value**: The value of the descriptor.
* modeled after: `GattServer::addService`


### cancelServiceDeclaration

* invocation: `gattServer cancelServiceDeclaration`
* description: Cancel the current service declaration.
* arguments: None
* result: None



### read

* invocation: `gattServer read <attribute_handle> <connection_handle>`
* description: Read the value an attribute handle of the gatt server. The 
connection_handle parameter is optional.
* arguments: 
  - [`uint16_t`](#uint16_t) **attribute_handle**: The attribute handle to read. 
  - [`uint16_t`](#uint16_t) **connection_handle**: Optional parameter, useful to 
  read the value of CCCD for a given connection.
* result: 
  - [`HexString`](#hexstring) The value of the attribute.
* modeled after: `GattServer::read`


### write (gattServer)

* invocation: `gattServer write <attribute_handle> <value> <connection_handle>`
* description: Write the value an attribute handle of the gatt server. The 
connection_handle parameter is optional.
* arguments: 
  - [`uint16_t`](#uint16_t) **attribute_handle**: The attribute handle to write. 
  - [`HexString`](#hexstring) **value**: The value to write to the attribute.
  - [`uint16_t`](#uint16_t) **connection_handle**: Optional parameter, useful to 
  write the value of CCCD for a given connection.
* result: None
* modeled after: `GattServer::write`


### waitForDataWritten

* invocation: `gattServer waitForDataWritten <connection_handle> <attribute_handle> <timeout>`
* description: Wait for data of an attribute to be written.
* arguments: 
  - [`uint16_t`](#uint16_t) **connection_handle**: Handle of the connection 
  issuing the write request.
  - [`uint16_t`](#uint16_t) **attribute_handle**: The attribute handle to monitor. 
  - [`uint16_t`](#uint16_t) **timeout**: Maximum time allowed to this procedure; 
  in ms. 
* result: None
* modeled after: `GattServer::onDataWritten`




## securityManager module

The `securityManager` module allows a user to access capabilities of the 
SecurityManager class.


### init (securityManager)

* invocation: `securityManager init <enable_bonding> <require_mitm> 
<io_capabilities> <passkey>`
* description: Initialize and configure the security manager.
* arguments: 
  - [`bool`](#bool) **enable_bonding**: Allow or disallow bonding to the device.
  - [`bool`](#bool) **require_mitm**: Allow or disallow MITM protection.
  - [`SecurityIOCapabilities`](#io-capabilities) **io_capabilities**: I/O 
  capabilities of the device.
  - [`Passkey`](#passkey) **passkey**: The static passkey used by the security 
  manager.
* result: None
* modeled after: `SecurityManager::init`


### getAddressesFromBondTable

* invocation: `securityManager getAddressesFromBondTable <addresses_count>`
* description: Get the addresses from the bond table.
* arguments: 
  - [`uint8_t`](#uint8_t) **addresses_count**: The maximum number of addresses to 
  get. 
* result: A JSON array containing the addresses in the bond table. Each of this 
record is a JSON object containing the following fields:
  - [`AddressType`](#addresstype) **address_type**: The type of the address.
  - [`MacAddress`](#macaddress) **address**: The mac address.
* modeled after: `SecurityManager::getAddressesFromBondTable`


### setDatabaseFilepath

* invocation: `securityManager setDatabaseFilepath`
* description: Change the path to the database used by the security manager.
* arguments:
  - [`string`](#string) **dbPath**: Path to the file used to store Security Manager data
* result: None
* modeled after: `SecurityManager::setDatabaseFilepath`


### preserveBondingStateOnReset

* invocation: `securityManager preserveBondingStateOnReset`
* description: Normally all bonding information is lost when device is reset, this requests that the stack attempts to save the information and reload it during initialisation. This is not guaranteed.
* arguments:
  - [`bool`](#bool) **enable**: enable if true the stack will attempt to preserve bonding information on reset
* result: None
* modeled after: `SecurityManager::preserveBondingStateOnReset`


### purgeAllBondingState

* invocation: `securityManager purgeAllBondingState`
* description: Delete all peer device context and all related bonding information from the database within the security manager.
* arguments: None
* result: None
* modeled after: `SecurityManager::purgeAllBondingState`


### generateWhitelistFromBondTable

* invocation: `securityManager generateWhitelistFromBondTable`
* description: Create a list of addresses from all peers in the bond table and generate  an event which returns it as a whitelist. Pass in the container for the whitelist.  This will be returned by the event.
* arguments: None
* result: None
* modeled after: `SecurityManager::generateWhitelistFromBondTable`


### setPairingRequestAuthorisation

* invocation: `securityManager setPairingRequestAuthorisation`
* description: Tell the stack whether the application needs to authorise pairing requests or shoul they be automatically accepted.
* arguments:
  - [`bool`](#bool) **enable**: If set to true, pairingRequest in the event handler wil will be called and will require an action from the applicatio to continue with pairing by calling acceptPairingReques or cancelPairingRequest if the user wishes to reject it
* result: None
* modeled after: `SecurityManager::setPairingRequestAuthorisation`


### waitForEvent

* invocation: `securityManager waitForEvent`
* description: This waits for and handles incoming events (such as a procedure). It waits for a request from peer or pairing/encryption/etc event.
* arguments:
  - [`uint16_t`](#uint16_t) **connectionHandle**: The connection used by this procedur
  - [`uint16_t`](#uint16_t) **timeout**: Time after which this command should fai
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`string`](#string) **status**: Name of the last event raise
  - [`passkey`](#passkey) **passkey**: Passkey if received from the stack
* modeled after: `SecurityManager::waitForEvent`


### acceptPairingRequestAndWait

* invocation: `securityManager acceptPairingRequestAndWait`
* description: This waits for and handles an incoming or ongoing pairing procedure. It waits for a request from peer or pairing completion.
* arguments:
  - [`uint16_t`](#uint16_t) **connectionHandle**: The connection used by this procedure
  - [`uint16_t`](#uint16_t) **timeout**: Time after which this command should fai
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`string`](#string) **status**: Name of the last event raise
  - [`passkey`](#passkey) **passkey**: Passkey if received from the stack
* modeled after: `SecurityManager::acceptPairingRequestAndWait`


### rejectPairingRequest

* invocation: `securityManager rejectPairingRequest`
* description: This rejects an incoming pairing request.
* arguments:
  - [`uint16_t`](#uint16_t) **connectionHandle**: The connection used by this procedure
* result: None
* modeled after: `SecurityManager::rejectPairingRequest`


### enterConfirmationAndWait

* invocation: `securityManager enterConfirmationAndWait`
* description: This sends confirmation (yes or no) to the stack during pairing
* arguments:
  - [`uint16_t`](#uint16_t) **connectionHandle**: The connection used by this procedure
  - [`bool`](#bool) **confirm**: Whether to confirm the validity of the passkey
  - [`uint16_t`](#uint16_t) **timeout**: Time after which this command should fai
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`string`](#string) **status**: Name of the last event raise
  - [`passkey`](#passkey) **passkey**: Passkey if received from the stack
* modeled after: `SecurityManager::enterConfirmationAndWait`


### enterPasskeyAndWait

* invocation: `securityManager enterPasskeyAndWait`
* description: This sends confirmation (yes or no) to the stack during pairing
* arguments:
  - [`uint16_t`](#uint16_t) **connectionHandle**: The connection used by this procedure
  - [`passkey`](#passkey) **passkey**: Numeric passkey to use during pairing if asked for check (this is what the user would consider the passkey to be - this passkey can be set to something unexpected if required to simulate error cases).
  - [`uint16_t`](#uint16_t) **timeout**: Time after which this command should fai
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`string`](#string) **status**: Name of the last event raise
  - [`passkey`](#passkey) **passkey**: Passkey if received from the stack
* modeled after: `SecurityManager::enterPasskeyAndWait`


### requestPairingAndWait

* invocation: `securityManager requestPairingAndWait`
* description: This performs a pairing procedure when the device acts as an initiator.
* arguments:
  - [`uint16_t`](#uint16_t) **connectionHandle**: The connection used by this procedure
  - [`uint16_t`](#uint16_t) **pairing_timeout**: Time after which the authentication should fai
  - [`uint16_t`](#uint16_t) **timeout**: Time after which this command should fai
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`string`](#string) **status**: Name of the last event raise
  - [`passkey`](#passkey) **passkey**: Passkey if received from the stack
* modeled after: `SecurityManager::requestPairingAndWait`


### allowLegacyPairing

* invocation: `securityManager allowLegacyPairing`
* description: Allow of disallow the use of legacy pairing in case the application only wants to force the use of Secure Connections. If legacy pairing is disallowed and either side doesn't support Secure Connections the pairing will fail.
* arguments:
  - [`bool`](#bool) **allow**: if true, legacy pairing will be used if either peer doesn't support Secure Connections
* result: None
* modeled after: `SecurityManager::allowLegacyPairing`


### getSecureConnectionsSupport

* invocation: `securityManager getSecureConnectionsSupport`
* description: Check if the Secure Connections feature is supported by the stack and controller.
* arguments: None
* result: A JSON array containing the addresses in the bond table. Each of this # record is a JSON object containing the following fields:
  - [`bool`](#bool) **supported**: true if the Secure Connections method is supported, false otherwis
* modeled after: `SecurityManager::getSecureConnectionsSupport`


### setIoCapability

* invocation: `securityManager setIoCapability`
* description: Set the IO capability of the local device.
* arguments:
  - [`SecurityIOCapabilities`](#io-capabilities) **iocaps**: type of IO capabilities available on the local devic
* result: None
* modeled after: `SecurityManager::setIoCapability`


### setDisplayPasskey

* invocation: `securityManager setDisplayPasskey`
* description: Set the passkey that is displayed on the local device instead of using a randomly generated one
* arguments:
  - [`passkey`](#passkey) **passkey**: Numeric passkey to use during pairing if asked for check (this is what the user would consider the passkey to be - this passkey can be set to something unexpected if required to simulate error cases).
* result: None
* modeled after: `SecurityManager::setDisplayPasskey`


# Data types format: 

Every parameter used by a CLI commands is typed. This is the list of type 
recognized by ble-cliapp.


## Primitive types 

### bool

Boolean type, ble cliapp accept "true" or "false" as valid values.

* model `bool`

### int8_t

Signed integer on 8 bits. The application accept decimal and hexadecimal 
(0x...) representations.

* model `int8_t`

### uint8_t

Unsigned 8 bits integer. The application accept decimal and hexadecimal (0x...) 
representations.

* model `uint8_t`

### uint16_t

unsigned integer on 16 bits. The application accept decimal and hexadecimal 
(0x...) representations.

* model `uint16_t`

### uint32_t

unsigned integer on 32 bits. The application accept decimal and hexadecimal 
(0x...) representations.

* model `uint32_t`

### int32_t

signed integer on 32 bits. The application accept decimal and hexadecimal 
(0x...) representations.

* model `uint32_t`

### String

Model an ASCII string without quotes; spaces are not accepted at the moment.

### HexString

Hexadecimal representation for raw data. Each byte is represented as its two 
hexadecimal characters.

## BLE 

### OwnAddressType

* model: `own_address_type_t`

The enum value map to a string:

| C++                                                     | ble cliapp                              |
|---------------------------------------------------------|-----------------------------------------|
| own_address_type_t::PUBLIC                        | PUBLIC                                  |
| own_address_type_t::RANDOM                        | RANDOM                        |
| own_address_type_t::RESOLVABLE_PRIVATE_ADDRESS_PUBLIC_FALLBACK                 | RESOLVABLE_PRIVATE_ADDRESS_PUBLIC_FALLBACK                                  |
| own_address_type_t::RESOLVABLE_PRIVATE_ADDRESS_RANDOM_FALLBACK                 | RESOLVABLE_PRIVATE_ADDRESS_RANDOM_FALLBACK                 |


### AddressType

* model: `peer_address_type_t`

The enum value map to a string:

| C++                                                     | ble cliapp                              |
|---------------------------------------------------------|-----------------------------------------|
| peer_address_type_t::PUBLIC                        | PUBLIC                                  |
| peer_address_type_t::RANDOM                        | RANDOM                        |
| peer_address_type_t::PUBLIC_IDENTITY                 | PUBLIC_IDENTITY                                  |
| peer_address_type_t::RANDOM_STATIC_IDENTITY                 | RANDOM_STATIC_IDENTITY                 |
| peer_address_type_t::ANONYMOUS     | ANONYMOUS     |


### MacAddress

This type model a mac address, it is always represented a string: 
`XX:XX:XX:XX:XX:XX` where `X` is an hexadecimal character.

* model `address_t`

### UUID 

* model the class `UUID`.

The following formats are supported: 

* `0xXXXX` for 16 bits UUID constructed from an hex value
* `XXXXX` for 16 bits UUID constructed from a 16 bit number.
* `XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX` for 128 bits UUID

## Gap

### GapRole

* model `connection_role_t`.

| C++             | ble-cliapp   |
|-----------------|--------------|
| connection_role_t::PERIPHERAL | "PERIPHERAL" |
| connection_role_t::CENTRAL    | "CENTRAL"    |


### DisconnectionReason

* model `disconnection_reason_t`.

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| disconnection_reason_t::CONNECTION_TIMEOUT                          | "AUTHENTICATION_FAILURE"                          |
| disconnection_reason_t::CONNECTION_TIMEOUT           | "CONNECTION_TIMEOUT"           |
| disconnection_reason_t::REMOTE_USER_TERMINATED_CONNECTION | "REMOTE_USER_TERMINATED_CONNECTION" |
| disconnection_reason_t::REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES     | "REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES"     |
| disconnection_reason_t::REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF            | "REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF"            |
| disconnection_reason_t::LOCAL_HOST_TERMINATED_CONNECTION                  | "LOCAL_HOST_TERMINATED_CONNECTION"                  |
| disconnection_reason_t::UNACCEPTABLE_CONNECTION_PARAMETERS                  | "UNACCEPTABLE_CONNECTION_PARAMETERS"                  |


### LocalDisconnectionReason

* model `local_disconnection_reason_t` and `disconnection_reason_t`.

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| local_disconnection_reason_t::USER_TERMINATION                          | "USER_TERMINATION"                          |
| local_disconnection_reason_t::AUTHENTICATION_FAILURE           | "AUTHENTICATION_FAILURE"           |
| local_disconnection_reason_t::LOW_RESOURCES | "LOW_RESOURCES" |
| local_disconnection_reason_t::PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED     | "PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED"     |
| local_disconnection_reason_t::POWER_OFF            | "POWER_OFF"            |
| local_disconnection_reason_t::UNACCEPTABLE_CONNECTION_PARAMETERS                  | "UNACCEPTABLE_CONNECTION_PARAMETERS"                  |
| local_disconnection_reason_t::UNSUPPORTED_REMOTE_FEATURE                  | "UNSUPPORTED_REMOTE_FEATURE"                  |


### Appearance

* model `adv_data_appearance_t`.

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| adv_data_appearance_t::UNKNOWN| UNKNOWN |
| adv_data_appearance_t::GENERIC_PHONE| GENERIC_PHONE |
| adv_data_appearance_t::GENERIC_COMPUTER| GENERIC_COMPUTER |
| adv_data_appearance_t::GENERIC_WATCH| GENERIC_WATCH |
| adv_data_appearance_t::WATCH_SPORTS_WATCH| WATCH_SPORTS_WATCH |
| adv_data_appearance_t::GENERIC_CLOCK| GENERIC_CLOCK |
| adv_data_appearance_t::GENERIC_DISPLAY| GENERIC_DISPLAY |
| adv_data_appearance_t::GENERIC_REMOTE_CONTROL| GENERIC_REMOTE_CONTROL |
| adv_data_appearance_t::GENERIC_EYE_GLASSES| GENERIC_EYE_GLASSES |
| adv_data_appearance_t::GENERIC_TAG| GENERIC_TAG |
| adv_data_appearance_t::GENERIC_KEYRING| GENERIC_KEYRING |
| adv_data_appearance_t::GENERIC_MEDIA_PLAYER| GENERIC_MEDIA_PLAYER |
| adv_data_appearance_t::GENERIC_BARCODE_SCANNER| GENERIC_BARCODE_SCANNER |
| adv_data_appearance_t::GENERIC_THERMOMETER| GENERIC_THERMOMETER |
| adv_data_appearance_t::THERMOMETER_EAR| THERMOMETER_EAR |
| adv_data_appearance_t::GENERIC_HEART_RATE_SENSOR| GENERIC_HEART_RATE_SENSOR |
| adv_data_appearance_t::HEART_RATE_SENSOR_HEART_RATE_BELT| HEART_RATE_SENSOR_HEART_RATE_BELT |
| adv_data_appearance_t::GENERIC_BLOOD_PRESSURE| GENERIC_BLOOD_PRESSURE |
| adv_data_appearance_t::BLOOD_PRESSURE_ARM| BLOOD_PRESSURE_ARM |
| adv_data_appearance_t::BLOOD_PRESSURE_WRIST| BLOOD_PRESSURE_WRIST |
| adv_data_appearance_t::HUMAN_INTERFACE_DEVICE_HID| HUMAN_INTERFACE_DEVICE_HID |
| adv_data_appearance_t::KEYBOARD| KEYBOARD |
| adv_data_appearance_t::MOUSE| MOUSE |
| adv_data_appearance_t::JOYSTICK| JOYSTICK |
| adv_data_appearance_t::GAMEPAD| GAMEPAD |
| adv_data_appearance_t::DIGITIZER_TABLET| DIGITIZER_TABLET |
| adv_data_appearance_t::CARD_READER| CARD_READER |
| adv_data_appearance_t::DIGITAL_PEN| DIGITAL_PEN |
| adv_data_appearance_t::BARCODE_SCANNER| BARCODE_SCANNER |
| adv_data_appearance_t::GENERIC_GLUCOSE_METER| GENERIC_GLUCOSE_METER |
| adv_data_appearance_t::GENERIC_RUNNING_WALKING_SENSOR| GENERIC_RUNNING_WALKING_SENSOR |
| adv_data_appearance_t::RUNNING_WALKING_SENSOR_IN_SHOE| RUNNING_WALKING_SENSOR_IN_SHOE |
| adv_data_appearance_t::RUNNING_WALKING_SENSOR_ON_SHOE| RUNNING_WALKING_SENSOR_ON_SHOE |
| adv_data_appearance_t::RUNNING_WALKING_SENSOR_ON_HIP| RUNNING_WALKING_SENSOR_ON_HIP |
| adv_data_appearance_t::GENERIC_CYCLING| GENERIC_CYCLING |
| adv_data_appearance_t::CYCLING_CYCLING_COMPUTER| CYCLING_CYCLING_COMPUTER |
| adv_data_appearance_t::CYCLING_SPEED_SENSOR| CYCLING_SPEED_SENSOR |
| adv_data_appearance_t::CYCLING_CADENCE_SENSOR| CYCLING_CADENCE_SENSOR |
| adv_data_appearance_t::CYCLING_POWER_SENSOR| CYCLING_POWER_SENSOR |
| adv_data_appearance_t::CYCLING_SPEED_AND_CADENCE_SENSOR| CYCLING_SPEED_AND_CADENCE_SENSOR |
| adv_data_appearance_t::PULSE_OXIMETER_GENERIC| PULSE_OXIMETER_GENERIC |
| adv_data_appearance_t::PULSE_OXIMETER_FINGERTIP| PULSE_OXIMETER_FINGERTIP |
| adv_data_appearance_t::PULSE_OXIMETER_WRIST_WORN| PULSE_OXIMETER_WRIST_WORN |
| adv_data_appearance_t::GENERIC_WEIGHT_SCALE| GENERIC_WEIGHT_SCALE |
| adv_data_appearance_t::OUTDOOR_GENERIC| OUTDOOR_GENERIC |
| adv_data_appearance_t::OUTDOOR_LOCATION_DISPLAY_DEVICE| OUTDOOR_LOCATION_DISPLAY_DEVICE |
| adv_data_appearance_t::OUTDOOR_LOCATION_AND_NAVIGATION_DISPLAY_DEVICE| OUTDOOR_LOCATION_AND_NAVIGATION_DISPLAY_DEVICE |
| adv_data_appearance_t::OUTDOOR_LOCATION_POD| OUTDOOR_LOCATION_POD |
| adv_data_appearance_t::OUTDOOR_LOCATION_AND_NAVIGATION_POD| OUTDOOR_LOCATION_AND_NAVIGATION_POD |

### AdvertisingType

* model `advertising_type_t`.

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| advertising_type_t::CONNECTABLE_UNDIRECTED | "CONNECTABLE_UNDIRECTED" |
| advertising_type_t::CONNECTABLE_DIRECTED | "CONNECTABLE_DIRECTED" |
| advertising_type_t::SCANNABLE_UNDIRECTED | "SCANNABLE_UNDIRECTED" |
| advertising_type_t::NON_CONNECTABLE_UNDIRECTED | "NON_CONNECTABLE_UNDIRECTED" |
| advertising_type_t::CONNECTABLE_DIRECTED_LOW_DUTY | "CONNECTABLE_DIRECTED_LOW_DUTY" |



### AdvertisingDataType

* model `adv_data_type_t`.

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| adv_data_type_t::FLAGS| FLAGS |
| adv_data_type_t::INCOMPLETE_LIST_16BIT_SERVICE_IDS| INCOMPLETE_LIST_16BIT_SERVICE_IDS |
| adv_data_type_t::COMPLETE_LIST_16BIT_SERVICE_IDS| COMPLETE_LIST_16BIT_SERVICE_IDS |
| adv_data_type_t::INCOMPLETE_LIST_32BIT_SERVICE_IDS| INCOMPLETE_LIST_32BIT_SERVICE_IDS |
| adv_data_type_t::COMPLETE_LIST_32BIT_SERVICE_IDS| COMPLETE_LIST_32BIT_SERVICE_IDS |
| adv_data_type_t::INCOMPLETE_LIST_128BIT_SERVICE_IDS| INCOMPLETE_LIST_128BIT_SERVICE_IDS |
| adv_data_type_t::COMPLETE_LIST_128BIT_SERVICE_IDS| COMPLETE_LIST_128BIT_SERVICE_IDS |
| adv_data_type_t::LIST_128BIT_SOLICITATION_IDS| LIST_128BIT_SOLICITATION_IDS |
| adv_data_type_t::SHORTENED_LOCAL_NAME| SHORTENED_LOCAL_NAME |
| adv_data_type_t::COMPLETE_LOCAL_NAME| COMPLETE_LOCAL_NAME |
| adv_data_type_t::TX_POWER_LEVEL| TX_POWER_LEVEL |
| adv_data_type_t::DEVICE_ID| DEVICE_ID |
| adv_data_type_t::SLAVE_CONNECTION_INTERVAL_RANGE| SLAVE_CONNECTION_INTERVAL_RANGE |
| adv_data_type_t::SERVICE_DATA| SERVICE_DATA |
| adv_data_type_t::SERVICE_DATA_16BIT_ID| SERVICE_DATA_16BIT_ID |
| adv_data_type_t::SERVICE_DATA_128BIT_ID| SERVICE_DATA_128BIT_ID |
| adv_data_type_t::APPEARANCE| APPEARANCE |
| adv_data_type_t::ADVERTISING_INTERVAL| ADVERTISING_INTERVAL |
| adv_data_type_t::MANUFACTURER_SPECIFIC_DATA| MANUFACTURER_SPECIFIC_DATA |


### AdvertisingDataFlags 

* model `adv_data_flags_t`.

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| adv_data_flags_t::LE_LIMITED_DISCOVERABLE| LE_LIMITED_DISCOVERABLE |
| adv_data_flags_t::LE_GENERAL_DISCOVERABLE| LE_GENERAL_DISCOVERABLE |
| adv_data_flags_t::BREDR_NOT_SUPPORTED| BREDR_NOT_SUPPORTED |
| adv_data_flags_t::SIMULTANEOUS_LE_BREDR_C| SIMULTANEOUS_LE_BREDR_C |
| adv_data_flags_t::SIMULTANEOUS_LE_BREDR_H| SIMULTANEOUS_LE_BREDR_H |


### ScanningPolicyMode

* model `scanning_filter_policy_t`.

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| advertising_filter_policy_t::NO_FILTER | "NO_FILTER"  |
| advertising_filter_policy_t::FILTER_SCAN_REQUESTS | "FILTER_SCAN_REQUESTS" |
| advertising_filter_policy_t::FILTER_CONNECTION_REQUEST | "FILTER_CONNECTION_REQUEST"  |
| advertising_filter_policy_t::FILTER_SCAN_AND_CONNECTION_REQUESTS | "FILTER_SCAN_AND_CONNECTION_REQUESTS" |


### InitiatorPolicyMode

* model `initiator_filter_policy_t`

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| initiator_filter_policy_t::NO_FILTER | "NO_FILTER"  |
| initiator_filter_policy_t::USE_WHITE_LIST | "USE_WHITE_LIST" |


### AdvertisingPolicyMode

* model `advertising_filter_policy_t`

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| advertising_filter_policy_t::NO_FILTER | "NO_FILTER"  |
| advertising_filter_policy_t::FILTER_SCAN_REQUESTS | "FILTER_SCAN_REQUESTS"  |
| advertising_filter_policy_t::FILTER_CONNECTION_REQUEST | "FILTER_CONNECTION_REQUEST"  |
| advertising_filter_policy_t::FILTER_SCAN_AND_CONNECTION_REQUESTS | "FILTER_SCAN_AND_CONNECTION_REQUESTS" |



### AdvertisingPayload

A JSON object of the advertising payload: 
* **FLAGS**: JSON array of [`GapAdvertisingDataFlags`](#advertisingdataflags).
* **INCOMPLETE_LIST_16BIT_SERVICE_IDS** : JSON array of [`uint16_t`](#uint16_t) 
in the hex form (0xXXXX).
* **COMPLETE_LIST_16BIT_SERVICE_IDS** : JSON array of  [`uint16_t`](#uint16_t) 
in the hex form (0xXXXX).
* **INCOMPLETE_LIST_32BIT_SERVICE_IDS** : JSON array of [`uint32_t`](#uint32_t) 
in the hex form (0xXXXXXXXX).
* **COMPLETE_LIST_32BIT_SERVICE_IDS** : JSON array of [`uint32_t`](#uint32_t) in 
the hex form (0xXXXXXXXX).
* **SHORTENED_LOCAL_NAME** : [`String`](#string)
* **COMPLETE_LOCAL_NAME** : [`String`](#string)
* **MANUFACTURER_SPECIFIC_DATA** : [`HexString`](#hexstring)
* **raw**: The complete advertising payload in an [`HexString`](#hexstring).


### ConnInterval

Duration in units of 1250us within the range of 0x06 to 0x0C80.


### SupervisionTimeout

Duration in units of 10000us within the range of 0x0A to 0x0C80.


### ScanDuration

Duration in units of 10000us within the range of 0x00 to 0xFFFF.


### SyncTimeout

Duration in units of 10000us within the range of 0x0A to 0x4000.


### ScanPeriod

Duration in units of 1280000us within the range of 0x00 to 0xFFFF.


### PeriodicInterval

Duration in units of 1250us within the range of 0x06 to 0xFFFF.


### AdvDuration

Duration in units of 1000us within the range of 0x00 to 0xFFFF.


### DuplicatesFilter

* model `duplicates_filter_t`. 

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| duplicates_filter_t::DISABLE | "DISABLE" | 
| duplicates_filter_t::ENABLE | "DISABLE" | 
| duplicates_filter_t::PERIODIC_RESET | "PERIODIC_RESET" | 


### PeripheralPrivacyResolutionStrategy

* model `peripheral_privacy_configuration_t::resolution_strategy_t`. 

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| peripheral_privacy_configuration_t::DO_NOT_RESOLVE | "DO_NOT_RESOLVE" | 
| peripheral_privacy_configuration_t::REJECT_NON_RESOLVED_ADDRESS | "REJECT_NON_RESOLVED_ADDRESS" | 
| peripheral_privacy_configuration_t::PERFORM_PAIRING_PROCEDURE | "PERFORM_PAIRING_PROCEDURE" | 
| peripheral_privacy_configuration_t::PERFORM_AUTHENTICATION_PROCEDURE | "PERFORM_AUTHENTICATION_PROCEDURE" | 

### CentralPrivacyResolutionStrategy

* model `central_privacy_configuration_t::resolution_strategy_t`. 

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| central_privacy_configuration_t::DO_NOT_RESOLVE | "DO_NOT_RESOLVE" | 
| central_privacy_configuration_t::REJECT_NON_RESOLVED_ADDRESS | "RESOLVE_AND_FORWARD" | 
| central_privacy_configuration_t::PERFORM_PAIRING_PROCEDURE | "RESOLVE_AND_FILTER" |  

## SecurityManager

### Passkey 

* model `SecurityManager::Passkey_t`. 

It is a string of 6 digits.

### IO Capabilities

* model `SecurityManager::SecurityIOCapabilities_t`. 

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| SecurityManager::IO_CAPS_DISPLAY_ONLY | "IO_CAPS_DISPLAY_ONLY" | 
| SecurityManager::IO_CAPS_DISPLAY_YESNO | "IO_CAPS_DISPLAY_YESNO" | 
| SecurityManager::IO_CAPS_KEYBOARD_ONLY | "IO_CAPS_KEYBOARD_ONLY" | 
| SecurityManager::IO_CAPS_NONE | "IO_CAPS_NONE" | 
| SecurityManager::IO_CAPS_KEYBOARD_DISPLAY | "IO_CAPS_KEYBOARD_DISPLAY"| 


## GattClient

### HVXType_t

* model `HVXType_t`

| C++                                              | ble-cliapp                                    |
|--------------------------------------------------|-----------------------------------------------|
| BLE_HVX_NOTIFICATION | "BLE_HVX_NOTIFICATION"  |
| BLE_HVX_INDICATION | "BLE_HVX_INDICATION"  |
