//
// Created by deano on 7/27/22.
//

#include "core/core.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "usb/usb.hpp"
#include "usb/usb_device.hpp"

namespace USB {

const char *DescriptorTypeToString( DescriptorType descriptorType_ ) {
	switch(descriptorType_) {
		case DescriptorType::Device:
			return "Device";
		case DescriptorType::Configuration:
			return "Configuration";
		case DescriptorType::String:
			return "String";
		case DescriptorType::Interface:
			return "Interface";
		case DescriptorType::Endpoint:
			return "Endpoint";
		case DescriptorType::InterfacePower:
			return "InterfacePower";
		case DescriptorType::OTG:
			return "OTG";
		case DescriptorType::Debug:
			return "Debug";
		case DescriptorType::InterfaceAssociation:
			return "InterfaceAssociation";
		case DescriptorType::BOS:
			return "BOS";
		case DescriptorType::DeviceCapability:
			return "DeviceCapability";
		case DescriptorType::SuperspeedUSBEndpointCompanion:
			return "SuperspeedUSBEndpointCompanion";
		case DescriptorType::SuperspeedUSBIsochronousEndpointCompanion:
			return "SuperspeedUSBIsochronousEndpointCompanion";
		case DescriptorType::HID:
			return "HID";
		case DescriptorType::Hub:
			return "Hud";
		default:
			return "UNKNOWN";
	}
}

[[maybe_unused]] auto DeviceDescriptorTypeToString( DeviceDescriptorType type_ ) -> char const * {
	switch(type_) {
		case DeviceDescriptorType::Device: return "Device";
		case DeviceDescriptorType::Configuration: return "Configuration";
		case DeviceDescriptorType::String: return "String";
		case DeviceDescriptorType::Interface: return "Interface";
		case DeviceDescriptorType::Endpoint: return "Endpoint";
		case DeviceDescriptorType::DeviceQualifier: return "DeviceQualifier";
		case DeviceDescriptorType::OtherSpeedConfiguration: return "OtherSpeedConfiguration";
		case DeviceDescriptorType::InterfacePower: return "InterfacePower";
		default: return "UNKNOWN";
	}
}

auto ClassCodeToString( ClassCode code_ ) -> char const * {
	switch(code_) {
		case ClassCode::UseInterfaceClass: return "UseInterfaceClass";
		case ClassCode::Audio: return "Audio";
		case ClassCode::CDCControl: return "CDCControl";
		case ClassCode::HID: return "HID";
		case ClassCode::Physical: return "Physical";
		case ClassCode::Image: return "Image";
		case ClassCode::Printer: return "Printer";
		case ClassCode::MassStorage: return "MassStorage";
		case ClassCode::Hub: return "Hub";
		case ClassCode::CDCData: return "CDCData";
		case ClassCode::SmartCard: return "SmartCard";
		case ClassCode::ContentSecurity: return "ContentSecurity";
		case ClassCode::Video: return "Video";
		case ClassCode::PersonalHealthCare: return "PersonalHealthCare";
		case ClassCode::AudioVisual: return "AudioVisual";
		case ClassCode::Billboard: return "Billboard";
		case ClassCode::USBTypeCBridge: return "USBTypeCBridge";
		case ClassCode::Diagnostic: return "Diagnostic";
		case ClassCode::Wireless: return "Wireless";
		case ClassCode::Misc: return "Misc";
		case ClassCode::ApplicationSpecific: return "ApplicationSpecific";
		case ClassCode::VendorSpecific: return "VendorSpecific";
		default: return "UNKNOWN";
	}
}

void DeviceDescriptor::Dump( DeviceDescriptor const *deviceDescriptor, bool only8Bytes_ ) {
	assert( deviceDescriptor->descriptorType == DeviceDescriptorType::Device );
	debug_print(ANSI_BRIGHT_ON "USB DeviceDescriptor" ANSI_BRIGHT_OFF "\n");
	debug_printf( "length %i usbVersion %x class %s\n",
	              deviceDescriptor->length,
	              deviceDescriptor->bcdUSBVersion,
	              ClassCodeToString( deviceDescriptor->deviceClass ));
	debug_printf( "subclass %x protocol %x ",
	              deviceDescriptor->deviceSubClass,
	              deviceDescriptor->deviceProtocol );
	debug_printf( "max packet size %i\n", DeviceDescriptor::GetMaxPacketSize(deviceDescriptor));

	if(!only8Bytes_) {
		debug_printf( "vid %#06x pid %#06x bcdDeviceVersion %#06x\n",
		              deviceDescriptor->vid,
		              deviceDescriptor->pid,
		              deviceDescriptor->bcdDeviceVersion );
		debug_printf( "manufacturerIndex %i productIndex %i serialNumberIndex %i num configurations %i\n",
		              deviceDescriptor->manufacturerIndex,
		              deviceDescriptor->productIndex,
		              deviceDescriptor->serialNumberIndex,
		              deviceDescriptor->numConfigurations);
	}
}

void ConfigurationDescriptor::Dump( ConfigurationDescriptor const *configurationDescriptor ) {
	assert( configurationDescriptor->descriptorType == DescriptorType::Configuration );
	debug_print( ANSI_BRIGHT_ON
	"USB ConfigurationDescriptor"
	ANSI_BRIGHT_OFF
	"\n");
	debug_printf( "length %i totalLength %i\n",
	              configurationDescriptor->length,
	              configurationDescriptor->totalLength );
	debug_printf( "numInterfaces %i configVal %i, stringIndex %i\n",
	              configurationDescriptor->numInterfaces,
	              configurationDescriptor->configVal,
	              configurationDescriptor->stringIndex );
	debug_printf( "attributes %#04x maxPower %i\n",
	              configurationDescriptor->attributes,
	              configurationDescriptor->maxPower );
}

void InterfaceDescriptor::Dump( InterfaceDescriptor const *interfaceDescriptor ) {
	assert( interfaceDescriptor->descriptorType == DescriptorType::Interface );
	debug_print( ANSI_BRIGHT_ON
	"USB InterfaceDescriptor"
	ANSI_BRIGHT_OFF
	"\n");
	debug_printf( "length %i thisInterfaceIndex %i\n",
	              interfaceDescriptor->length,
	              interfaceDescriptor->thisInterfaceIndex );
	debug_printf( "alternateSet %i numEndpoints %i, classCode %s\n",
	              interfaceDescriptor->alternateSet,
	              interfaceDescriptor->numEndpoints,
	              ClassCodeToString( interfaceDescriptor->classCode ));
	debug_printf( "subClassCode %i protocol %i stringIndex %i\n",
	              interfaceDescriptor->subClassCode,
	              interfaceDescriptor->protocol,
	              interfaceDescriptor->stringIndex );
}

void EndpointDescriptor::Dump( EndpointDescriptor const *endpointDescriptor ) {
//	assert( endpointDescriptor->descriptorType == DescriptorType::Endpoint );
	debug_print( ANSI_BRIGHT_ON "USB EndpointDescriptor" ANSI_BRIGHT_OFF "\n");
	debug_printf( "length %i dir %s address %i (endpointId %i)\n",
	              endpointDescriptor->length,
	              USB::DirectionToString((USB::Direction) (endpointDescriptor->address >> 7)),
	              endpointDescriptor->address & 0xF,
								EndpointDescriptor::AddressToEndpointId(endpointDescriptor));
	debug_printf( "attributes %i %s maxPacketSize %i interval %i\n",
	              endpointDescriptor->attributes,
	              EndpointTransportTypeToString((USB::EndpointTransportType) (endpointDescriptor->attributes & 0xF)),
	              endpointDescriptor->maxPacketSize,
	              endpointDescriptor->interval );
}

} // USB