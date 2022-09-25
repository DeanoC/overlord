#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"


#include "core/core.h"
#include "usb/usb.hpp"

namespace XHCI ::TRB {

enum class Type : uint8_t {
	Reserved = 0,
	Normal = 1,
	SetupStage = 2,
	DataStage = 3,
	StatusStage = 4,
	Isoch = 5,
	Link = 6,
	EventData = 7,
	Noop = 8,

	EnableSlotCommand = 9,
	DisableSlotCommand = 10,
	AddressDeviceCommand = 11,
	ConfigureEndpointCommand = 12,
	EvaluateContextCommand = 13,
	ResetEndpointCommand = 14,
	StopEndpointCommand = 15,
	SetTRDequeuePointerCommand = 16,
	ResetDeviceCommand = 17,
	ForceEventCommand = 18,
	NegotiateBandwidthCommand = 19,
	SetLatencyToleranceValueCommand = 20,
	GetPortBandwidth = 21,
	ForceHeaderCommand = 22,
	NoopCommand = 23,
	GetExtendedPropertyCommand = 24,
	SetExtendedPropertyCommand = 26,

	TransferEvent = 32,
	CommandCompletionEvent = 33,
	PortStatusChangeEvent = 34,
	BandwidthRequestEvent = 35,
	DoorbellEvent = 36,
	HostControllerEvent = 37,
	DeviceNotification = 38,
	MFINDEXWrapEvent = 39,
};

struct PACKED ALIGN( 4 )  Template {
	uint64_t parameters;

	uint32_t status;

	uint32_t cycle: 1;                  // C
	uint32_t evaluateNext: 1;        // ENT
	uint32_t reserved: 8;
	TRB::Type type: 6;
	uint32_t control: 16;

	static void Dump( Template const *e_ );

};

struct ALIGN( 4 ) PACKED Normal {
	union {
		struct {
			uint32_t dataBufferPointerHi;
			uint32_t dataBufferPointerLo;
		};
		uint64_t dataBufferPointer;
	};

	uint32_t transferLength: 17;
	uint32_t TDSize: 5;
	uint32_t interrupterTarget: 10;

	uint32_t cycle: 1;                      // C
	uint32_t evaluateNext: 1;              // ENT
	uint32_t interruptOnShortPacket: 1;    // ISP
	uint32_t noSnoop: 1;                    // NS
	uint32_t chain: 1;                      // CH
	uint32_t interruptOnCompletion: 1;      // IOC
	uint32_t immediateData: 1;              // IDT
	uint32_t reserved1: 2;
	uint32_t blockEventInterrupt: 1;        // BEI
	TRB::Type type: 6;
	uint32_t reserved0: 16;
};

struct ALIGN( 4 ) PACKED SetupStage {
	USB::RequestData requestData;

	uint32_t transferLength: 17; // always 8
	uint32_t reserved0: 5;
	uint32_t interrupterTarget: 10;

	uint32_t cycle: 1;                  // C
	uint32_t reserved3: 4;
	uint32_t interruptOnCompletion: 1;    // IOC
	uint32_t immediateData: 1;            // IDT
	uint32_t reserved2: 3;
	TRB::Type type: 6;
	USB::TransferType transferType: 2;        // TRT
	uint32_t reserved1: 14;

	static void Dump( SetupStage const *e_ );
};

static_assert( sizeof( SetupStage ) == sizeof( Template ));

struct ALIGN( 4 ) PACKED DataStage {
	union {
		struct {
			uint32_t dataBufferPointerHi;
			uint32_t dataBufferPointerLo;
		};
		uint64_t dataBufferPointer;
	};

	uint32_t transferLength: 17;
	uint32_t TDSize: 5;
	uint32_t interrupterTarget: 10;

	uint32_t cycle: 1;                      // C
	uint32_t evaluateNext: 1;              // ENT
	uint32_t interruptOnShortPacket: 1;    // ISP
	uint32_t noSnoop: 1;                    // NS
	uint32_t chain: 1;                      // CH
	uint32_t interruptOnCompletion: 1;      // IOC
	uint32_t immediateData: 1;              // IDT
	uint32_t reserved1: 2;
	uint32_t blockEventInterrupt: 1;        // BEI
	TRB::Type type: 6;
	USB::Direction direction: 1;            // DIR
	uint32_t reserved0: 15;

	static void Dump( DataStage const *const e_ );

};

struct ALIGN( 4 ) PACKED StatusStage {
	uint64_t reserved0;

	uint32_t reserved1: 22;
	uint32_t interrupterTarget: 10;

	uint32_t cycle: 1;                      // C
	uint32_t evaluateNext: 1;              // ENT
	uint32_t reserved: 2;
	uint32_t chain: 1;                      // CH
	uint32_t interruptOnCompletion: 1;      // IOC
	uint32_t reserved3: 4;
	TRB::Type type: 6;
	USB::Direction direction: 1;           // DIR
	uint32_t reserved2: 15;

	static void Dump( StatusStage const *e_ );

};

struct ALIGN( 4 ) PACKED Isoch {
	union {
		struct {
			uint32_t dataBufferPointerHi;
			uint32_t dataBufferPointerLo;
		};
		uint64_t dataBufferPointer;
	};

	uint32_t transferLength: 17;
	uint32_t TDSize: 5;
	uint32_t interrupterTarget: 10;

	uint32_t cycle: 1;                          // C
	uint32_t evaluateNext: 1;                  // ENT
	uint32_t interruptOnShortPacket: 1;        // ISP
	uint32_t noSnoop: 1;                        // NS
	uint32_t chain: 1;                          // CH
	uint32_t interruptOnCompletion: 1;          // IOC
	uint32_t immediateData: 1;                  // IDT
	uint32_t transferBurstCount: 2;            // TBC
	uint32_t blockEventInterrupt: 1;            // BEI
	TRB::Type type: 6;
	uint32_t transferLastBurstPackCount: 4;    // TLBPC
	uint32_t frameID: 11;
	uint32_t startIsochASAP: 1;                // SIA

};

struct ALIGN( 4 ) PACKED Noop {
	uint64_t reserved0;

	uint32_t reserved1: 22;
	uint32_t interrupterTarget: 10;
	uint32_t : 0;

	uint32_t cycle: 1;                          // C
	uint32_t evaluateNext: 1;                // ENT
	uint32_t reserved4: 2;
	uint32_t chain: 1;                          // CH
	uint32_t interruptOnCompletion: 1;          // IOC
	uint32_t reserved3: 4;
	TRB::Type type: 6;
	uint32_t reserved2: 16;

	static void Dump( Noop const *e_ );

};

// event S (only found in the event ring)
struct ALIGN( 4 ) PACKED TransferEvent {
	uint64_t parameter;

	uint32_t transferLength: 24;
	USB::CompletionCode completionCode: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 1;
	uint32_t eventDataTransferEvent: 1;
	uint32_t reserved1: 7;
	TRB::Type type: 6;
	uint32_t endpointId: 5;
	uint32_t reserved0: 3;
	uint32_t slotId: 8;

	static void Dump( TransferEvent const *e_ );

};

struct ALIGN( 4 ) PACKED CommandCompletionEvent {
	union {
		struct {
			uint32_t commandTRBPointerHi;
			uint32_t commandTRBPointerLo; // 16 byte aligned
		};
		uint64_t commandTRBPointer;
	};

	uint32_t commandCompletionParameter: 24;
	USB::CompletionCode completionCode: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved1: 9;
	TRB::Type type: 6;
	uint32_t vfId: 8;
	uint32_t slotId: 8;

	static void Dump( CommandCompletionEvent const *e_ );

};

struct ALIGN( 4 ) PACKED PortStatusChangeEvent {
	uint32_t reserved0: 24;
	uint32_t portId: 8;

	uint32_t reserved1;

	uint32_t reserved2: 24;
	USB::CompletionCode completionCode: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 9;
	TRB::Type type: 6;
	uint32_t reserved3: 16;

	static void Dump( PortStatusChangeEvent const *e_ );
};

struct ALIGN( 4 ) PACKED BandwidthRequestEvent {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2: 24;
	USB::CompletionCode completionCode: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 9;
	TRB::Type type: 6;
	uint32_t reserved3: 8;
	uint32_t slotID: 8;
};

struct ALIGN( 4 ) PACKED DoorbellEvent {
	uint32_t reserved0: 24;
	uint32_t reason: 8;

	uint32_t reserved1;

	uint32_t reserved2: 24;
	USB::CompletionCode completionCode: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved3: 9;
	TRB::Type type: 6;
	uint32_t vfId: 8;
	uint32_t slotID: 8;
};

struct ALIGN( 4 ) PACKED HostControllerEvent {

	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2: 24;
	USB::CompletionCode completionCode: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 9;
	TRB::Type type: 6;
	uint32_t reserved3: 16;

	static void Dump( HostControllerEvent const *e_ );

};

struct ALIGN( 4 ) PACKED DeviceNotificationEvent {
	uint64_t reserved0: 4;
	uint64_t notificationType: 4;
	uint64_t data: 56;

	uint32_t reserved1: 24;
	USB::CompletionCode completionCode: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved3: 9;
	TRB::Type type: 6;
	uint32_t reserved2: 8;
	uint32_t slotID: 8;
};

struct ALIGN( 4 ) PACKED MFIndexEvent {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2: 24;
	USB::CompletionCode completionCode: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 9;
	TRB::Type type: 6;
	uint32_t reserved3: 16;
};

struct ALIGN( 4 ) PACKED NoopEvent {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 9;
	TRB::Type type: 6;
	uint32_t reserved3: 16;
};

// Command TRBs

struct ALIGN( 4 ) PACKED EnableSlotCommand {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 9;
	Type type: 6;
	uint32_t reserved3: 10;
};

struct ALIGN( 4 ) PACKED DisableSlotCommand {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 9;
	TRB::Type type: 6;
	uint32_t reserved3: 8;
	uint32_t slotType: 8;
};

struct ALIGN( 4 ) PACKED AddressDeviceCommand {
	union {
		struct {
			uint32_t inputContextPointerHi;
			uint32_t inputContextPointerLo; // 16 byte aligned
		};
		uint64_t inputContextPointer;
	};

	uint32_t reserved0;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 8;
	uint32_t blockSetAddressRequest: 1;        // BSR
	TRB::Type type: 6;
	uint32_t reserved1: 8;
	uint32_t slotID: 8;

	static void Dump( AddressDeviceCommand const * e_ );

};

struct ALIGN( 4 ) PACKED ConfigureEndpointCommand {
	union {
		struct {
			uint32_t inputContextPointerHi;
			uint32_t inputContextPointerLo; // 16 byte aligned
		};
		uint64_t inputContextPointer;
	};

	uint32_t reserved0;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 8;
	uint32_t deconfigure: 1;                  // deconfigure
	TRB::Type type: 6;
	uint32_t reserved1: 8;
	uint32_t slotId: 8;

};

struct ALIGN( 4 ) PACKED EvaluateContextCommand {
	union {
		struct {
			uint32_t inputContextPointerHi;
			uint32_t inputContextPointerLo; // 16 byte aligned
		};
		uint64_t inputContextPointer;
	};

	uint32_t reserved0;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 8;
	uint32_t blockSetAddressRequest: 1;        // BSR
	TRB::Type type: 6;
	uint32_t reserved1: 8;
	uint32_t slotId: 8;

};

struct ALIGN( 4 ) PACKED ResetEndpointCommand {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 8;
	uint32_t transferStatePreserve: 1;        // TSP
	TRB::Type type: 6;
	uint32_t endpointID: 5;
	uint32_t reserved3: 3;
	uint32_t slotID: 8;

};

struct PACKED StopEndpointCommand {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 8;
	uint32_t transferStatePreserve: 1;        // TSP
	TRB::Type type: 6;
	uint32_t endpointID: 5;
	uint32_t reserved3: 2;
	uint32_t suspend: 1;                        // SP
	uint32_t slotID: 8;

};

struct ALIGN( 4 ) PACKED SetTRDequeuePointerCommand {
	union {
		struct {
			uint32_t newTRDequeuePointerHi;
			uint32_t dequeueCycleStatus: 1;        // DCS
			uint32_t streamContextType: 3;          // SCT
			uint32_t newTRDequeuePointerLo: 28;
		};
		uint64_t newTRDequeuePointer_Unclean: 60; // remember to shift or mask, bottom bits are used

	};

	uint32_t reserved0: 16;
	uint32_t streamID: 16;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 9;
	TRB::Type type: 6;
	uint32_t endpointID: 5;
	uint32_t reserved1: 3;
	uint32_t slotID: 8;

};

struct ALIGN( 4 ) PACKED ResetDeviceCommand {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2;

	uint32_t cycle: 1;                          // C
	uint32_t reserved4: 9;
	TRB::Type type: 6;
	uint32_t reserved3: 8;
	uint32_t slotID: 8;

};

struct ALIGN( 4 ) PACKED ForceEventCommand {
	union {
		struct {
			uint32_t eventTRBPointerHi;
			uint32_t eventTRBPointerLo; // 16 byte aligned
		};
		uint64_t eventTRBPointer;
	};

	uint32_t reserved0: 24;
	uint32_t vfInterrupterTarget: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 9;
	TRB::Type type: 6;
	uint32_t vfID: 8;
	uint32_t reserved1: 8;

};

typedef DisableSlotCommand NegotiateBandwidthCommand;

struct ALIGN( 4 ) PACKED SetLatencyToleranceValueCommand {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t reserved2;

	uint32_t cycle: 1;                                // C
	uint32_t reserved4: 9;
	uint32_t type: 6;
	uint32_t bestEffortLatencyToleranceValue: 12;    // BELT
	uint32_t reserved3: 4;

};

struct ALIGN( 4 ) PACKED GetPortBandwithCommand {
	union {
		struct {
			uint32_t portBandwidthContextPointerHi;
			uint32_t portBandwidthContexPointertLo; // 16 byte aligned
		};
		uint64_t portBandwidthContextPointer;
	};

	uint32_t reserved0;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 9;
	TRB::Type type: 6;
	uint32_t devSpeed: 4;
	uint32_t reserved1: 4;
	uint32_t hostSlotID: 8;

};

struct ALIGN( 4 ) PACKED ForceHeaderCommand {
	uint32_t packetType: 5;                  // TYPE
	uint32_t headerInfoLo: 27;

	uint32_t headerInfoMid;

	uint32_t headerInfoHi;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 9;
	TRB::Type type: 6;
	uint32_t reserved0: 8;
	uint32_t rootHubPortNumber: 8;

};

struct ALIGN( 4 ) PACKED GetExtendedPropertyCommand {
	union {
		struct {
			uint32_t extendedPropertyContextPointerHi;
			uint32_t extendedPropertyContextPointerLo; // 16 byte aligned
		};
		uint64_t extendedPropertyContextPointer;
	};

	uint32_t extendedCapabilityID: 16;
	uint32_t reserved0: 16;

	uint32_t cycle: 1;                          // C
	uint32_t reserved2: 9;
	uint32_t subType: 3;
	uint32_t endpointID: 5;
	uint32_t slotID: 8;

};

struct ALIGN( 4 ) PACKED SetExtendedPropertyCommand {
	uint32_t reserved0;

	uint32_t reserved1;

	uint32_t extendedCapabilityID: 16;
	uint32_t capabilityParameter: 8;
	uint32_t reserved2: 8;

	uint32_t cycle: 1;                          // C
	uint32_t reserved3: 9;
	TRB::Type type: 6;
	uint32_t subType: 3;
	uint32_t endpointID: 5;
	uint32_t slotID: 8;

};

// Other TRBs
struct ALIGN( 4 ) PACKED Link {
	union {
		struct {
			uint32_t ringSegmentPointerHi;
			uint32_t ringSegmentPointerLo; // 16 byte aligned
		};
		uint64_t ringSegmentPointer;
	};

	uint32_t reserved0: 24;
	uint32_t interrupterTarget: 8;

	uint32_t cycle: 1;                      // C
	uint32_t toggleCycle: 1;                // TC
	uint32_t reserved3: 2;
	uint32_t chain: 1;                      // CH
	uint32_t interruptOnCompletion: 1;      // IOC
	uint32_t reserved2: 4;
	TRB::Type type: 6;
	uint32_t reserved1: 16;

};

struct ALIGN( 4 ) PACKED EventData {
	uint64_t eventData;

	uint32_t reserved0: 24;
	uint32_t interrupterTarget: 8;

	uint32_t cycle: 1;                      // C
	uint32_t toggleCycle: 1;                // TC
	uint32_t reserved3: 2;
	uint32_t chain: 1;                      // CH
	uint32_t interruptOnCompletion: 1;      // IOC
	uint32_t reserved2: 3;
	uint32_t blockEventInterrupt: 1;        // BEI
	TRB::Type type: 6;
	uint32_t reserved1: 16;

};

static_assert( sizeof( Template ) == 16 );
static_assert( sizeof( Template ) == sizeof( Normal ));
static_assert( sizeof( Template ) == sizeof( SetupStage ));
static_assert( sizeof( Template ) == sizeof( DataStage ));
static_assert( sizeof( Template ) == sizeof( StatusStage ));
static_assert( sizeof( Template ) == sizeof( Isoch ));
static_assert( sizeof( Template ) == sizeof( Noop ));

static_assert( sizeof( Template ) == sizeof( TransferEvent ));
static_assert( sizeof( Template ) == sizeof( CommandCompletionEvent ));
static_assert( sizeof( Template ) == sizeof( PortStatusChangeEvent ));
static_assert( sizeof( Template ) == sizeof( BandwidthRequestEvent ));
static_assert( sizeof( Template ) == sizeof( DoorbellEvent ));
static_assert( sizeof( Template ) == sizeof( HostControllerEvent ));
static_assert( sizeof( Template ) == sizeof( DeviceNotificationEvent ));
static_assert( sizeof( Template ) == sizeof( MFIndexEvent ));
static_assert( sizeof( Template ) == sizeof( NoopEvent ));

static_assert( sizeof( Template ) == sizeof( EnableSlotCommand ));
static_assert( sizeof( Template ) == sizeof( DisableSlotCommand ));
static_assert( sizeof( Template ) == sizeof( AddressDeviceCommand ));
static_assert( sizeof( Template ) == sizeof( ConfigureEndpointCommand ));
static_assert( sizeof( Template ) == sizeof( EvaluateContextCommand ));
static_assert( sizeof( Template ) == sizeof( ResetEndpointCommand ));
static_assert( sizeof( Template ) == sizeof( StopEndpointCommand ));
static_assert( sizeof( Template ) == sizeof( SetTRDequeuePointerCommand ));
static_assert( sizeof( Template ) == sizeof( ResetDeviceCommand ));
static_assert( sizeof( Template ) == sizeof( ForceEventCommand ));
static_assert( sizeof( Template ) == sizeof( NegotiateBandwidthCommand ));
static_assert( sizeof( Template ) == sizeof( SetLatencyToleranceValueCommand ));
static_assert( sizeof( Template ) == sizeof( GetPortBandwithCommand ));
static_assert( sizeof( Template ) == sizeof( ForceHeaderCommand ));
static_assert( sizeof( Template ) == sizeof( GetExtendedPropertyCommand ));
static_assert( sizeof( Template ) == sizeof( SetExtendedPropertyCommand ));
static_assert( sizeof( Template ) == sizeof( Link ));
static_assert( sizeof( Template ) == sizeof( EventData ));

TRB::Template *FillInLink( TRB::Template *templateTRB_, TRB::Template * to_, bool toggleCycle_ );

TRB::Template *FillInNoop( TRB::Template *templateTRB_ );

TRB::Template *FillInEnableSlot( TRB::Template *templateTRB_ );

TRB::Template *FillInAddressDevice( TRB::Template *templateTRB_, uint32_t slotId_, bool bsar, uintptr_all_t inputContext_ );

TRB::Template *FillInConfigureEndpoint( TRB::Template *templateTRB_, uint32_t slotId_, uintptr_all_t inputContext_ );

TRB::Template *FillInEvaluateContext( TRB::Template *templateTRB_, uint32_t slotId_, uintptr_all_t inputContext_ );

TRB::Template *FillInSetupStage( TRB::Template *templateTRB_,
                                 USB::RequestData const * requestData_);

TRB::Template *FillInOutDataStage( TRB::Template *templateTRB_, uint16_t payloadSize_, uintptr_all_t payload_ );

TRB::Template *FillInInDataStage( TRB::Template *templateTRB_, uint16_t payloadSize_, uintptr_all_t payload_, uint8_t maxTDSize_ );

TRB::Template *FillInStatusStage( TRB::Template *templateTRB_, USB::Direction dir_, bool interruptOnCompletion_ );

TRB::Template *FillInEventData( TRB::Template *templateTRB_, uintptr_all_t eventData_ );

TRB::Template *FillInNormal( TRB::Template *templateTRB_, uint32_t dataLength_, uintptr_all_t data_, uint8_t maxTDSize_ );

char const *TypeToString( Type type_ );

} // end namespaces

#pragma GCC diagnostic pop
