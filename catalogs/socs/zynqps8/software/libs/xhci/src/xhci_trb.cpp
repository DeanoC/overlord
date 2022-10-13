#include "core/core.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "zynqps8/xhci/trb.hpp"

#define XHCI_LOG_ANSI(x) ANSI_CYAN_PEN x ANSI_WHITE_PEN "\n"

namespace XHCI ::TRB {
void Template::Dump( Template const * const e_ ) {
	Type type = (Type) e_->type;
	bool customDump = false;

	switch((Type) type) {
		case Type::Reserved:
			break;
		case Type::Normal:
			break;
		case Type::SetupStage:
			SetupStage::Dump((TRB::SetupStage *) e_ );
			customDump = true;
			break;
		case Type::DataStage:
			DataStage::Dump((TRB::DataStage *) e_ );
			customDump = true;
			break;
		case Type::StatusStage:
			StatusStage::Dump((TRB::StatusStage *) e_ );
			customDump = true;
			break;
		case Type::Isoch:
			break;
		case Type::Link:
			break;
		case Type::EventData:
			break;
		case Type::Noop:
			break;
		case Type::EnableSlotCommand:
			break;
		case Type::DisableSlotCommand:
			break;
		case Type::AddressDeviceCommand:
			AddressDeviceCommand::Dump((TRB::AddressDeviceCommand *) e_ );
			customDump = true;
			break;
		case Type::ConfigureEndpointCommand:
			break;
		case Type::EvaluateContextCommand:
			break;
		case Type::ResetEndpointCommand:
			break;
		case Type::StopEndpointCommand:
			break;
		case Type::SetTRDequeuePointerCommand:
			break;
		case Type::ResetDeviceCommand:
			break;
		case Type::ForceEventCommand:
			break;
		case Type::NegotiateBandwidthCommand:
			break;
		case Type::SetLatencyToleranceValueCommand:
			break;
		case Type::GetPortBandwidth:
			break;
		case Type::ForceHeaderCommand:
			break;
		case Type::NoopCommand:
			Noop::Dump((TRB::Noop *) e_ );
			customDump = true;
			break;
		case Type::GetExtendedPropertyCommand:
			break;
		case Type::SetExtendedPropertyCommand:
			break;
		case Type::TransferEvent:
			TransferEvent::Dump((TRB::TransferEvent *) e_);
			customDump = true;
			break;
		case Type::CommandCompletionEvent:
			CommandCompletionEvent::Dump((TRB::CommandCompletionEvent *) e_ );
			customDump = true;
			break;
		case Type::PortStatusChangeEvent:
			PortStatusChangeEvent::Dump((TRB::PortStatusChangeEvent *) e_ );
			customDump = true;
			break;
		case Type::BandwidthRequestEvent:
			break;
		case Type::DoorbellEvent:
			break;
		case Type::HostControllerEvent:
			HostControllerEvent::Dump((TRB::HostControllerEvent *) e_ );
			customDump = true;
			break;
		case Type::DeviceNotification:
			break;
		case Type::MFINDEXWrapEvent:
			break;
	}
	if(!customDump) {
		debug_printf( XHCI_LOG_ANSI("Template TRB (%#018lx): %#010lx %#010x cycle %i evalNext %i type %s control %i"),
		              (uintptr_all_t) e_,
		              e_->parameters,
		              e_->status,
		              e_->cycle, e_->evaluateNext, TypeToString( type ), e_->control );
	}
}

void Noop::Dump( Noop const * const e_ ) {
	debug_printf( XHCI_LOG_ANSI("Noop TRB: interrupterTarget %i cycle %i evalNext %i chain %i interuptionOnComplete %i type %s"),
	              e_->interrupterTarget,
	              e_->cycle, e_->evaluateNext, e_->chain, e_->interruptOnCompletion, TypeToString( e_->type ));
}
void TransferEvent::Dump( TransferEvent const * const e_ ) {
	auto const *te = (TRB::TransferEvent const *) e_;
	assert( te->type == TRB::Type::TransferEvent );

	debug_printf( XHCI_LOG_ANSI("TransferEvent TRB: %p %s transferLength %i"), (void*) e_, USB::CompletionCodeToString( te->completionCode ), te->transferLength );
	debug_printf( XHCI_LOG_ANSI("parameter %#018lx event data transfer event %i"), (uintptr_all_t) te->parameter, te->eventDataTransferEvent );
	debug_printf( XHCI_LOG_ANSI("slotId %#04x endpointId %#04x"),te->slotId, te->endpointId );
}

void PortStatusChangeEvent::Dump( PortStatusChangeEvent const * const e_ ) {
	debug_printf( XHCI_LOG_ANSI("PortStatusChangeEvent TRB: %p %s rootPortId %i cycle %i"),
	              (void*) e_, CompletionCodeToString((USB::CompletionCode) e_->completionCode ), e_->portId, e_->cycle );
}

void CommandCompletionEvent::Dump( CommandCompletionEvent const * const e_ ) {
	debug_printf( XHCI_LOG_ANSI("CommandCompletionEvent TRB: %p Command %s for %s"),
	              (void*) e_, USB::CompletionCodeToString((USB::CompletionCode) e_->completionCode ),
	              TRB::TypeToString((Type) ((TRB::Template *) e_->commandTRBPointer)->type ));
	debug_printf( XHCI_LOG_ANSI("\tcommandCompletionParameter %#0x slotId %i"),
	              e_->commandCompletionParameter,
	              e_->slotId );
	debug_printf( XHCI_LOG_ANSI("parameter %#018lx"), ((Template *) e_)->parameters );

}

void SetupStage::Dump( SetupStage const * const e_ ) {
	debug_printf( XHCI_LOG_ANSI("SetupStage TRB: %p requestType %#06x requestCode %s"),
	              (void*) e_,  e_->requestData.requestType, RequestCodeToString( e_->requestData.code ));
	debug_printf( XHCI_LOG_ANSI("wValue %#x wIndex %i wLength %i transferLength %u"),
	              e_->requestData.wValue, e_->requestData.wIndex, e_->requestData.wLength, e_->transferLength );
	debug_printf( XHCI_LOG_ANSI("interrupterTarget %i cycle %i interruptOnCompletion %i immediateData %i"),
	              e_->interrupterTarget, e_->cycle, e_->interruptOnCompletion, e_->immediateData );
	debug_printf( XHCI_LOG_ANSI("type %s TransferType %s"),
	              TypeToString( e_->type ), TransferTypeToString( e_->transferType ));
	debug_printf( XHCI_LOG_ANSI("parameter %#018lx"), ((Template *) e_)->parameters );
}

void DataStage::Dump( DataStage const * const e_ ) {
	debug_printf( XHCI_LOG_ANSI("DataStage TRB: %p payload %#lx"), (void*) e_, e_->dataBufferPointer );
	debug_printf( XHCI_LOG_ANSI("transferLength %u TDSize %u"), e_->transferLength, e_->TDSize );
	debug_printf( XHCI_LOG_ANSI("interrupterTarget %i cycle %i evaluateNext %i interruptOnShortPacket %i"),
	              e_->interrupterTarget, e_->cycle, e_->evaluateNext, e_->interruptOnShortPacket );
	debug_printf( XHCI_LOG_ANSI("noSnoop %i chain %i interruptOnCompletion %i immediateData %i"),
	              e_->noSnoop, e_->chain, e_->interruptOnCompletion, e_->immediateData );
	debug_printf( XHCI_LOG_ANSI("blockEventInterupt %i type %s direction %s"),
	              e_->blockEventInterrupt, TypeToString( e_->type ), DirectionToString( e_->direction ));
	debug_printf( XHCI_LOG_ANSI("parameter %#018lx"), ((Template *) e_)->parameters );
}

void StatusStage::Dump( StatusStage const * const e_ ) {
	debug_printf( XHCI_LOG_ANSI("StatusStage TRB: %p interrupterTarget %i cycle %i interruptOnCompletion %i evaluateNext %i"),
	              (void*) e_, e_->interrupterTarget, e_->cycle, e_->interruptOnCompletion, e_->evaluateNext );
	debug_printf( XHCI_LOG_ANSI("chain %i type %s direction %s"),
	              e_->chain, TypeToString( e_->type ), DirectionToString( e_->direction ));
	debug_printf( XHCI_LOG_ANSI("parameter %#018lx"), ((Template *) e_)->parameters );
}

void HostControllerEvent::Dump( HostControllerEvent const * const e_ ) {
	debug_printf( XHCI_LOG_ANSI("HostControllerEvent TRB: %p Command %s"),
	              (void*) e_, USB::CompletionCodeToString((USB::CompletionCode) e_->completionCode ));
}

void AddressDeviceCommand::Dump( AddressDeviceCommand const * const e_ ) {
	debug_printf( XHCI_LOG_ANSI("AddressDeviceCommand TRB: %p input context pointer %#018lx"), (void*) e_, e_->inputContextPointer );
	debug_printf( XHCI_LOG_ANSI("cycle %i type %s slotId %i"), e_->cycle, TypeToString( e_->type ), e_->slotID );
}



TRB::Template *FillInLink( TRB::Template * templateTRB_,
                           TRB::Template * to_,
                           bool toggleCycle_ ) {
	auto *linkTRB = (Link *) templateTRB_;
	memset( linkTRB, 0, sizeof( Link ));
	linkTRB->type = Type::Link;
	linkTRB->ringSegmentPointer = (uintptr_all_t)to_;
	linkTRB->toggleCycle = toggleCycle_;
	linkTRB->interruptOnCompletion = false;
	linkTRB->chain = true;

	return (TRB::Template *) linkTRB;
}

TRB::Template *FillInNoop( TRB::Template *templateTRB_ ) {
	auto *noopTRB = (Noop *) templateTRB_;
	memset( noopTRB, 0, sizeof( Noop ));
	noopTRB->interrupterTarget = 0;
	noopTRB->chain = false;
	noopTRB->interruptOnCompletion = true;
	noopTRB->type = Type::NoopCommand;
	return (TRB::Template *) noopTRB;
}

TRB::Template *FillInEnableSlot( TRB::Template *templateTRB_ ) {
	auto *esTRB = (EnableSlotCommand *) templateTRB_;
	memset( esTRB, 0, sizeof( EnableSlotCommand ));
	esTRB->type = Type::EnableSlotCommand;
	return (TRB::Template *) esTRB;
}

TRB::Template *FillInAddressDevice( TRB::Template *templateTRB_, uint32_t slotId_, bool bsar, uintptr_all_t inputContext_ ) {
	auto *adTRB = (AddressDeviceCommand *) templateTRB_;
	memset( adTRB, 0, sizeof( AddressDeviceCommand ));
	adTRB->type = Type::AddressDeviceCommand;
	adTRB->inputContextPointer = inputContext_;
	adTRB->blockSetAddressRequest = bsar;
	adTRB->slotID = slotId_;
	return (TRB::Template *) adTRB;
}

TRB::Template *FillInConfigureEndpoint( TRB::Template *templateTRB_, uint32_t slotId_, uintptr_all_t inputContext_ ) {
	auto *ceTRB = (ConfigureEndpointCommand *) templateTRB_;
	memset( ceTRB, 0, sizeof( ConfigureEndpointCommand ));
	ceTRB->type = Type::ConfigureEndpointCommand;
	ceTRB->inputContextPointer = inputContext_;
	ceTRB->slotId = slotId_;
	return (TRB::Template *) ceTRB;
}

TRB::Template *FillInEvaluateContext( TRB::Template *templateTRB_, uint32_t slotId_, uintptr_all_t inputContext_ ) {
	auto *ecTRB = (EvaluateContextCommand *) templateTRB_;
	memset( ecTRB, 0, sizeof( EvaluateContextCommand ));
	ecTRB->type = Type::EvaluateContextCommand;
	ecTRB->inputContextPointer = inputContext_;
	ecTRB->slotId = slotId_;
	return (TRB::Template *) ecTRB;
}

TRB::Template *FillInSetupStage( TRB::Template *templateTRB_,
                                 USB::RequestData const * requestData_) {
	auto *ssTRB = (SetupStage *) templateTRB_;
	memset( ssTRB, 0, sizeof( SetupStage ));

	memcpy(&ssTRB->requestData, requestData_, sizeof(USB::RequestData));
	ssTRB->transferLength = 8;
	ssTRB->type = Type::SetupStage;
	ssTRB->transferType = (requestData_->requestType >= 0x80) ? USB::TransferType::InDataStage : USB::TransferType::OutDataStage;
	ssTRB->immediateData = true;
	return (TRB::Template *) ssTRB;
}

TRB::Template *FillInStatusStage( TRB::Template *templateTRB_, USB::Direction dir_, bool interruptOnCompletion_ ) {
	auto *ssTRB = (StatusStage *) templateTRB_;
	memset( ssTRB, 0, sizeof( StatusStage ));

	ssTRB->type = Type::StatusStage;
	ssTRB->direction = dir_;
	ssTRB->chain = !interruptOnCompletion_;
	ssTRB->evaluateNext = !interruptOnCompletion_;
	ssTRB->interruptOnCompletion = interruptOnCompletion_;
	return (TRB::Template *) ssTRB;
}

TRB::Template *FillInOutDataStage( TRB::Template *templateTRB_, uint16_t payloadSize_, uintptr_all_t payload_ ) {
	auto *dsTRB = (DataStage *) templateTRB_;
	memset( dsTRB, 0, sizeof( DataStage ));
	dsTRB->type = Type::DataStage;
	dsTRB->TDSize = 0;
	dsTRB->direction = USB::Direction::Out;
	dsTRB->dataBufferPointer = payload_;
	dsTRB->evaluateNext = true;
	dsTRB->chain = true;
	// for short transfer OUT support 8 byte immediate payloads
	if(payloadSize_ <= 8) {
		dsTRB->immediateData = true;
		dsTRB->transferLength = 8;
	} else {
		dsTRB->immediateData = false;
		dsTRB->transferLength = payloadSize_;
	}
	return (TRB::Template *) dsTRB;

}

TRB::Template *FillInInDataStage( TRB::Template *templateTRB_, uint16_t payloadSize_, uintptr_all_t payload_, uint8_t maxTDSize_  ) {
	assert( payloadSize_ != 0);
	assert( payload_ != 0 );

	auto *dsTRB = (DataStage *) templateTRB_;
	memset( dsTRB, 0, sizeof( DataStage ));
	dsTRB->type = Type::DataStage;
	dsTRB->TDSize = maxTDSize_;
	dsTRB->direction = USB::Direction::In;
	dsTRB->dataBufferPointer = payload_;
	dsTRB->transferLength = payloadSize_;

	dsTRB->evaluateNext = true;
	dsTRB->chain = true;
	dsTRB->immediateData = false;
	dsTRB->interruptOnCompletion = false;
	dsTRB->interruptOnShortPacket = false;
	return (TRB::Template *) dsTRB;
}

TRB::Template *FillInEventData( TRB::Template *templateTRB_, uintptr_all_t eventData_ ) {
	auto *esTRB = (EventData *) templateTRB_;
	memset( esTRB, 0, sizeof( EventData ));
	esTRB->type = Type::EventData;
	esTRB->eventData = eventData_;
	esTRB->chain = false;
	esTRB->interruptOnCompletion = true;
	return (TRB::Template *) esTRB;
}

TRB::Template *FillInNormal( TRB::Template *templateTRB_, uint32_t dataLength_, uintptr_all_t data_, uint8_t maxTDSize_  ) {
	auto *nTRB = (Normal *) templateTRB_;
	memset( nTRB, 0, sizeof( EventData ));
	assert(dataLength_ < (1 << 17));

	nTRB->type = Type::Normal;
	nTRB->dataBufferPointer = data_;
	nTRB->transferLength = dataLength_;
	nTRB->TDSize = maxTDSize_;
	nTRB->chain = 1;
	nTRB->evaluateNext = true;
	nTRB->interruptOnCompletion = false;
	return (TRB::Template *) nTRB;
}

const char *TypeToString( Type type_ ) {
	switch(type_) {
		case Type::Reserved:
			return "Reserved";
		case Type::Normal:
			return "Normal";
		case Type::SetupStage:
			return "SetupStage";
		case Type::DataStage:
			return "DataStage";
		case Type::StatusStage:
			return "StatusStage";
		case Type::Isoch:
			return "Isoch";
		case Type::Link:
			return "Link";
		case Type::EventData:
			return "EventData";
		case Type::Noop:
			return "Noop";
		case Type::EnableSlotCommand:
			return "EnableSlotCommand";
		case Type::DisableSlotCommand:
			return "DisableSlotCommand";
		case Type::AddressDeviceCommand:
			return "AddressDeviceCommand";
		case Type::ConfigureEndpointCommand:
			return "ConfigureEndpointCommand";
		case Type::EvaluateContextCommand:
			return "EvaluateContextCommand";
		case Type::ResetEndpointCommand:
			return "ResetEndpointCommand";
		case Type::StopEndpointCommand:
			return "StopEndpointCommand";
		case Type::SetTRDequeuePointerCommand:
			return "SetTRDequeuePointerCommand";
		case Type::ResetDeviceCommand:
			return "ResetDeviceCommand";
		case Type::ForceEventCommand:
			return "ForceEventCommand";
		case Type::NegotiateBandwidthCommand:
			return "NegotiateBandwidthCommand";
		case Type::SetLatencyToleranceValueCommand:
			return "SetLatencyToleranceValueCommand";
		case Type::GetPortBandwidth:
			return "GetPortBandwidth";
		case Type::ForceHeaderCommand:
			return "ForceHeaderCommand";
		case Type::NoopCommand:
			return "NoopCommand";
		case Type::GetExtendedPropertyCommand:
			return "GetExtendedPropertyCommand";
		case Type::SetExtendedPropertyCommand:
			return "SetExtendedPropertyCommand";
		case Type::TransferEvent:
			return "TransferEvent";
		case Type::CommandCompletionEvent:
			return "CommandCompletionEvent";
		case Type::PortStatusChangeEvent:
			return "PortStatusChangeEvent";
		case Type::BandwidthRequestEvent:
			return "BandwidthRequestEvent";
		case Type::DoorbellEvent:
			return "DoorbellEvent";
		case Type::HostControllerEvent:
			return "HostControllerEvent";
		case Type::DeviceNotification:
			return "DeviceNotification";
		case Type::MFINDEXWrapEvent:
			return "MFINDEXWrapEvent";
		default:
			return "UNKNOWN";
	}
}


}