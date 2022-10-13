#include "core/core.h"
#include "dbg/print.h"
#include "usb/usb.hpp"
#include "dbg/assert.h"
#include "dbg/ansi_escapes.h"

namespace USB {

char const * SpeedToString( Speed speed_ ) {
	switch(speed_) {
		case Speed::FullSpeed: return "FullSpeed";
		case Speed::LowSpeed: return "LowSpeed";
		case Speed::HighSpeed: return "HighSpeed";
		case Speed::SuperSpeed: return "SuperSpeed";
		default: return "UNKNOWN";
	}
}

const char *CompletionCodeToString( CompletionCode code_ ) {
	switch(code_) {
		case CompletionCode::Invalid:
			return "Invalid";
		case CompletionCode::Success:
			return "Success";
		case CompletionCode::DataBufferError:
			return "DataBufferError";
		case CompletionCode::BabbleDetectedError:
			return "BabbleDetectedError";
		case CompletionCode::USBTransactionError:
			return "USBTransactionError";
		case CompletionCode::Error:
			return "Error";
		case CompletionCode::StallError:
			return "StallError";
		case CompletionCode::ResourceError:
			return "ResourceError";
		case CompletionCode::BandwidthError:
			return "BandwidthError";
		case CompletionCode::NoSlotsAvailableError:
			return "NoSlotsAvailableError";
		case CompletionCode::InvalidStreamTypeError:
			return "InvalidStreamTypeError";
		case CompletionCode::SlotNotEnabledError:
			return "SlotNotEnabledError";
		case CompletionCode::EndpointNoEnabledError:
			return "EndpointNoEnabledError";
		case CompletionCode::ShortPacket:
			return "ShortPacket";
		case CompletionCode::RingUnderrun:
			return "RingUnderrun";
		case CompletionCode::RingOverrun:
			return "RingOverrun";
		case CompletionCode::VFEventRingFullError:
			return "VFEventRingFullError";
		case CompletionCode::ParameterError:
			return "ParameterError";
		case CompletionCode::BandwidthOverrunError:
			return "BandwidthOverrunError";
		case CompletionCode::ContextStateError:
			return "ContextStateError";
		case CompletionCode::NoPointResourceError:
			return "NoPointResourceError";
		case CompletionCode::EventRingFullError:
			return "EventRingFullError";
		case CompletionCode::IncompatibleDeviceError:
			return "IncompatibleDeviceError";
		case CompletionCode::MissedServiceError:
			return "MissedServiceError";
		case CompletionCode::CommandRingStopped:
			return "CommandRingStopped";
		case CompletionCode::CommandAborted:
			return "CommandAborted";
		case CompletionCode::Stopped:
			return "Stopped";
		case CompletionCode::StoppedLengthInvalid:
			return "StoppedLengthInvalid";
		case CompletionCode::DebugAbort:
			return "DebugAbort";
		case CompletionCode::StoppedShortPacket:
			return "StoppedShortPacket";
		case CompletionCode::Reserved:
			return "Reserved";
		case CompletionCode::IsochBufferOverrun:
			return "IsochBufferOverrun";
		case CompletionCode::EventLostError:
			return "EventLostError";
		case CompletionCode::UndefinedEvent:
			return "UndefinedEvent";
		case CompletionCode::InvalidStreamIDError:
			return "InvalidStreamIDError";
		case CompletionCode::SecondaryBandwidthError:
			return "SecondaryBandwidthError";
		case CompletionCode::SplitTransactionError:
			return "SplitTransactionError";
		case CompletionCode::VendorDefinedErrorStart:
			return "VendorDefinedErrorStart";
		case CompletionCode::VendorDefinedInfoStart:
			return "VendorDefinedInfoStart";
		default:
			return "UNKNOWN";
	}
}

const char *RequestCodeToString( RequestCode requestCode_ ) {
	switch(requestCode_) {
		case RequestCode::GetStatus:
			return "GetStatus";
		case RequestCode::ClearFeature:
			return "ClearFeature";
		case RequestCode::SetFeature:
			return "SetFeature";
		case RequestCode::SetAddress:
			return "SetAddress";
		case RequestCode::GetDescriptor:
			return "GetDescriptor";
		case RequestCode::SetDescriptor:
			return "SetDescriptor";
		case RequestCode::GetConfiguration:
			return "GetConfiguration";
		case RequestCode::SetConfiguration:
			return "SetConfiguration";
		case RequestCode::GetInterface:
			return "GetInterface";
		case RequestCode::SetInterface:
			return "SetInterface";
		case RequestCode::SynchFrame:
			return "SynchFrame";
		case RequestCode::SetEncryption:
			return "SetEncryption";
		case RequestCode::GetEncryption:
			return "GetEncryption";
		case RequestCode::SetHandShake:
			return "SetHandShake";
		case RequestCode::GetHandShake:
			return "GetHandShake";
		case RequestCode::SetConnection:
			return "SetConnection";
		case RequestCode::SetSecurity:
			return "SetSecurity";
		case RequestCode::GetSecurity:
			return "GetSecurity";
		case RequestCode::SetWUSBData:
			return "SetWUSBData";
		case RequestCode::LoopBackDataWrite:
			return "LoopBackDataWrite";
		case RequestCode::LoopBackDataRead:
			return "LoopBackDataRead";
		case RequestCode::SetInterfaceDS:
			return "SetInterfaceDS";
		case RequestCode::GetFWStatus:
			return "GetFWStatus";
		case RequestCode::SetFWStatus:
			return "SetFWStatus";
		case RequestCode::SetSel:
			return "SetSel";
		case RequestCode::SetIsochDelay:
			return "SetIsochDelay";
		default:
			return "UNKNOWN";
	}
}


const char *StandardFeatureSelectorsToString( StandardFeatureSelectors standardFeatureSelectors_ ) {
	switch(standardFeatureSelectors_) {
		case StandardFeatureSelectors::EndpointHalt:
			return "EndpointHalt";
		case StandardFeatureSelectors::DeviceRemoteWakeup:
			return "DeviceRemoteWakeup";
		case StandardFeatureSelectors::TestMode:
			return "TestMode";
		case StandardFeatureSelectors::b_hnp_enable:
			return "b_hnp_enable";
		case StandardFeatureSelectors::a_hnp_support:
			return "a_hnp_support";
		case StandardFeatureSelectors::a_alt_hnp_support:
			return "a_alt_hnp_support";
		case StandardFeatureSelectors::WUSB_Device:
			return "WUSB_Device";
		case StandardFeatureSelectors::U1Enable:
			return "U1Enable";
		case StandardFeatureSelectors::U2Enable:
			return "U2Enable";
		case StandardFeatureSelectors::LTMEnable:
			return "LTMEnable";
		case StandardFeatureSelectors::B3_NTF_HOST_REL:
			return "B3_NTF_HOST_REL";
		case StandardFeatureSelectors::B3_RSPEnable:
			return "B3_RSPEnable";
		case StandardFeatureSelectors::LDMEnable:
			return "LDMEnable";
		default:
			return "UNKNOWN";
	}
}

char const *DirectionToString( Direction dataDirection_ ) {
	switch(dataDirection_) {
		case Direction::Out:
			return "Out";
		case Direction::In:
			return "In";
		default:
			return "UNKNOWN";
	}
}

char const *TransferTypeToString( TransferType transferType_ ) {
	switch(transferType_) {
		case TransferType::NoDataStage:
			return "NoDataStage";
		case TransferType::Reserved:
			return "Reserved";
		case TransferType::OutDataStage:
			return "OutDataStage";
		case TransferType::InDataStage:
			return "InDataStage";
		default:
			return "UNKNOWN";
	}
}
char const *EndpointTransportTypeToString( EndpointTransportType endpointTransportType_) {
	switch(endpointTransportType_) {
		case EndpointTransportType::Control: return "Control EP";
		case EndpointTransportType::Isochronous: return "Isochronous EP";
		case EndpointTransportType::Bulk: return "Bulk EP";
		case EndpointTransportType::Interrupt: return "Interrupt EP";
		default:
			return "UNKNOWN";
	}
}


void RequestData::Dump( const RequestData *requestData ) {
	debug_printf( "Request Data %#06x %s\n", requestData->requestType, RequestCodeToString(requestData->code));
	debug_printf("wValue %#06x wIndex %#06x wLength %i\n", requestData->wValue, requestData->wIndex, requestData->wLength);
}

} // end namespace