#pragma once

#include "core/core.h"
#include "core/utf8.h"

namespace USB {

struct Controller;


enum class DeviceDescriptorType : uint8_t {
	Device = 1,
	Configuration = 2,
	String = 3,
	Interface = 4,
	Endpoint = 5,
	DeviceQualifier = 6,
	OtherSpeedConfiguration = 7,
	InterfacePower = 8,
};

enum class ClassCode : uint8_t {
	UseInterfaceClass = 0x0,
	Audio = 0x1,
	CDCControl = 0x2,
	HID = 0x3,
	Physical = 0x5,
	Image = 0x6,
	Printer = 0x7,
	MassStorage = 0x8,
	Hub = 0x9,
	CDCData = 0xA,
	SmartCard = 0xB,
	ContentSecurity = 0xD,
	Video = 0xE,
	PersonalHealthCare = 0xF,
	AudioVisual = 0x10,
	Billboard = 0x11,
	USBTypeCBridge = 0x12,
	I3C = 0x3C,
	Diagnostic = 0xDC,
	Wireless = 0xE0,
	Misc = 0xEF,
	ApplicationSpecific = 0xFE,
	VendorSpecific = 0xFF,
};

enum class DescriptorType : uint8_t {
	Device = 1,
	Configuration = 2,
	String = 3,
	Interface = 4,
	Endpoint = 5,
	InterfacePower = 8,
	OTG = 9,
	Debug = 10,
	InterfaceAssociation = 11,
	BOS = 15,
	DeviceCapability = 16,
	HID = 0x21,
	Hub = 0x29,
	SuperspeedUSBEndpointCompanion = 48,
	SuperspeedUSBIsochronousEndpointCompanion = 49,
};

struct PACKED ConfigurationDescriptor {
	uint8_t length;
	DescriptorType descriptorType;
	uint16_t totalLength;
	uint8_t numInterfaces;
	uint8_t configVal;
	uint8_t stringIndex;
	uint8_t attributes;
	uint8_t maxPower;

	static void Dump( ConfigurationDescriptor const * configurationDescriptor);
};
static_assert(sizeof(ConfigurationDescriptor) == 9);

struct PACKED InterfaceDescriptor {
	uint8_t length;
	DescriptorType descriptorType;
	uint8_t thisInterfaceIndex;
	uint8_t alternateSet;
	uint8_t numEndpoints;
	ClassCode classCode;
	uint8_t subClassCode;
	uint8_t protocol;
	uint8_t stringIndex;

	static void Dump( InterfaceDescriptor const * interfaceDescriptor);
};
static_assert(sizeof(InterfaceDescriptor) == 9);

struct PACKED EndpointDescriptor {
	uint8_t length;
	DescriptorType descriptorType;
	uint8_t address;
	uint8_t attributes;
	uint16_t maxPacketSize;
	uint8_t interval;

	static ALWAYS_INLINE uint8_t AddressToEndpointId(EndpointDescriptor const * ep_) {
		return ((ep_->address & 0xF)<<1) | (ep_->address >> 7);
	}
	static void Dump( EndpointDescriptor const * endpointDescriptor);
};
static_assert(sizeof(EndpointDescriptor) == 7);

struct PACKED DeviceDescriptor {
	uint8_t length;                       // total length in bytes including following configurations etc.
	DeviceDescriptorType descriptorType;  // what type is this device?
	uint16_t bcdUSBVersion;               // BCD version of USB standard
	ClassCode deviceClass;              // which class this device belongs to
	uint8_t deviceSubClass;
	uint8_t deviceProtocol;
	uint8_t maxPacketSize;                // if bcdUSBVersion >= 300 2^maxPacketSize else maxPacketSize
	// initial 8 byte fetch ends here

	uint16_t vid;                         // vendor ID
	uint16_t pid;                         // product ID
	uint16_t bcdDeviceVersion;            // which version the device has (these 3 together uniquely identifier this device)
	uint8_t manufacturerIndex;            // index into string table for the manufacturer name
	uint8_t productIndex;                 // index into string table for the product name
	uint8_t serialNumberIndex;            // index into string table for the serial number
	uint8_t numConfigurations;            // number of configurations this device has

	// only8Bytes_ only dumps the first 8 bytes
	static void Dump( DeviceDescriptor const * deviceDescriptor, bool only8Bytes_ );
	ALWAYS_INLINE static uint32_t GetMaxPacketSize(DeviceDescriptor const * deviceDescriptor_) {
		return (deviceDescriptor_->bcdUSBVersion < 0x300) ? deviceDescriptor_->maxPacketSize : 1 << deviceDescriptor_->maxPacketSize;
	}
};
static_assert(sizeof(DeviceDescriptor) == 18);

typedef void (*CHEnumerateFunc)(Device* device_);
typedef void (*CHConfigurationDescriptorReadyFunc)(Device* device_, struct ConfigurationDescriptor * configurationDescriptor, struct InterfaceDescriptor * interfaceDescriptor);
typedef void (*CHInterfacesEnabledFunc)(Device* device_);

struct ClassHandlerVTable {
	CHEnumerateFunc enumerate; // hubs only
	CHConfigurationDescriptorReadyFunc configurationDescriptorReady;
	CHInterfacesEnabledFunc interfacesEnabled;
};

// all devices structures must be max 1KB of memory
struct Device {
	static const int MaxStringSize = 64;
	ClassHandlerVTable * vtable; // only valid once configurationDescriptorReady has been called back!!!
	struct ControlPipe * controlPipe;

	Controller * controller;
	DeviceDescriptor descriptor;

	struct HubDevice * parentHub; // nullptr if a root hub device

	uint16_t packetSize;
	uint8_t slotId;
	uint8_t configuration;

	utf8_int8_t manufacturerString[MaxStringSize];
	utf8_int8_t productString[MaxStringSize];
	utf8_int8_t serialNumberString[MaxStringSize];

};

ALWAYS_INLINE bool IsClassCodeForDevice( ClassCode code_ ) {
	switch(code_) {
		case ClassCode::UseInterfaceClass:
		case ClassCode::CDCControl:
		case ClassCode::Hub:
		case ClassCode::Billboard:
		case ClassCode::Diagnostic:
		case ClassCode::Misc:
		case ClassCode::VendorSpecific: return true;
		default: return false;
	}
}

ALWAYS_INLINE bool IsClassCodeForBoth( ClassCode code_ ) {
	switch(code_) {
		case ClassCode::CDCControl:
		case ClassCode::Diagnostic:
		case ClassCode::Misc:
		case ClassCode::VendorSpecific: return true;
		default: return false;
	}
}

ALWAYS_INLINE bool IsClassCodeForInterface( ClassCode code_ ) {
	return !IsClassCodeForDevice( code_ ) || IsClassCodeForBoth( code_ );
}

[[maybe_unused]] char const *DeviceDescriptorTypeToString( DeviceDescriptorType type_ );
const char *DescriptorTypeToString( DescriptorType descriptorType_ );

} // USB