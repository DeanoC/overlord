#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "core/core.h"
#include "memory/memory.h"
#include "multi_core/mutex.h"
#include "cadt/freelist.hpp"
#include "core/utf8.h"
#include "trb.hpp"
#include "usb/usb.hpp"
#include "usb/usb_hub.hpp"

namespace XHCI {

#define XHCI_LOG_ANSI(x) ANSI_CYAN_PEN x ANSI_WHITE_PEN "\n"

enum class ExtensionID {
	Reserved = 0,
	LegacyUSB = 1,
	SupportedProtocol = 2,
	ExtendedPowerManagement = 3,
	IOVirtualization = 4,
	MessageInterrupt = 5,
	LocalMemory = 6,
	USBDebugCapability = 10,
	ExtendedMessageInterrupt = 17
};

enum class EndpointState {
	Disabled = 0,
	Running = 1,
	Halted = 2,
	Stopped = 3,
	Error = 4,
	Reserved0 = 5,
	Reserved1 = 6,
	Reserved2 = 7,
};

enum class EndpointType : uint8_t {
	NotValid = 0,
	IsochOut = 1,
	BulkOut = 2,
	InterruptOut = 3,
	Control = 4,
	IsochIn = 5,
	BulkIn = 6,
	InterruptIn = 7,
};

enum class SlotState : uint8_t {
	Disabled = 0,
	Default = 1,
	Addressed = 2,
	Configured = 3,
};


struct PACKED ERST {
	uint64_t ringSegmentBaseAddress;
	uint64_t ringSegmentSize: 16;
	uint64_t reserved0: 48;
};
static_assert( sizeof( ERST ) == 16 );

struct PACKED InputContext {
	uint32_t dropContextFlags;

	uint32_t addContextFlags;

	uint32_t reserved0[5];

	uint32_t configurationValue: 8;
	uint32_t interfaceNumber: 8;
	uint32_t alternateSetting: 8;
	uint32_t reserved1: 8;

	uint32_t xhiReserved1[8];
};
static_assert( sizeof( InputContext ) == 64 );

struct PACKED SlotContext {
	uint32_t routeString: 20;
	USB::Speed speed: 4;
	uint32_t reserved0: 1;
	uint32_t multiTT: 1;            // MTT
	uint32_t hub: 1;
	uint32_t contextEntryCount: 5;

	uint32_t maxExitLatency: 16;
	uint32_t rootPortId: 8;
	uint32_t hubPortCount: 8;

	uint32_t parentHubSlotId: 8;
	uint32_t parentPortNumber: 8;
	uint32_t TTThinkTime: 2;        // TTT
	uint32_t reserved1: 4;
	uint32_t interrupterTarget: 10;

	uint32_t usbDeviceAddress: 8;
	uint32_t reserved2: 19;
	SlotState slotState: 5;

	uint32_t xhiReserved0[4];
	uint32_t xhiReserved1[8];
};
static_assert( sizeof( SlotContext ) == 64 );

struct PACKED SlotEndPoint {
	SlotState EPState: 3;
	uint32_t reserved0: 5;
	uint32_t multi: 2;
	uint32_t maxPrimaryStreams: 5;
	uint32_t linearStreamArray: 1;
	uint32_t interval: 8;
	uint32_t maxESITPayloadHi: 8;

	uint32_t reserved1: 1;
	uint32_t errorCount: 2;
	EndpointType endpointType: 3;
	uint32_t reserved2: 1;
	uint32_t hostInitiateDisable: 1; // HID
	uint32_t maxBurstSize: 8;
	uint32_t maxPacketSize: 16;

	union {
		uint64_t dequeueCycleState: 1; // DCS
		TRB::Template * TRBDequeuePointer;
	};

	uint32_t averageTRBLength: 16;
	uint32_t maxESITPayloadLo: 16;

	uint32_t xhiReserved0[3];
	uint32_t xhiReserved1[8];

};
static_assert( sizeof( SlotEndPoint ) == 64 );

struct PACKED DeviceContext {
	union {
		struct {
			SlotContext slotContext;
			SlotEndPoint controlEndPoint;
		};
		SlotEndPoint endPoints[(16 * 2)];
	};
};
static_assert( sizeof( DeviceContext ) == 32*64 );

// NOTE: the TRB pointer survive attach/detach cycles
struct PACKED DriverSlot {
	InputContext inputContext;
	DeviceContext deviceContext;
	// before this is sent to xhci when configuring endpoints

	USB::Device *device;
	TRB::Template * deviceBaseAddress[(16 * 2)];
	bool currentCycleBit[(16 * 2)]; // 0 not used, 1 = control, 2+ normal endpoints

	static TRB::Template* GetNextEndpointTRB(DriverSlot* driverSlot_, uint8_t endpointIndex_, uint8_t maxTDSize_ );
	static TRB::Template* EnqueueEndpointTRB(DriverSlot* driverSlot_, uint8_t endpointIndex_);

} ALIGN(64);


// The XHCI main class, except Init never allocates
struct Controller : public USB::Controller {

	static constexpr bool DoUSB2EnableSlotReset = false; // true doesn't work yet but if needed isn't hard to fix

	static constexpr uint32_t EventRingSegmentTableSize = 1;
	static constexpr uint32_t EventSegmentTRBSize = 256;
	static constexpr uint32_t MaxNewDevicesInFlight = 16;

	uint32_t memoryBlockSize;
	uint32_t commandTRBCount;
	uint32_t pageSize;
	uint8_t maxSlots;
	uint8_t maxPorts;
	uint8_t maxEventRingSegmentTableSize;

	void * memoryBlock;
	DeviceContext * * contextBaseAddressArray;
	uintptr_all_t * scratchPadArray;
	DeviceContext * hwDeviceContextMem; // these are copied to contextBaseAddressArray on EnableSlot

	TRB::Template *commandRingBase;
	TRB::Template *commandEnqueuePtr;
	bool commandCycleBit;
	Core_Mutex commandMutex;

	ERST *primaryERST;
	bool primaryEventCycleBit;
	TRB::Template * primarySegments[EventRingSegmentTableSize];
	TRB::Template * primaryERSTWrapAddress;
	TRB::Template * currentPrimaryEventPtr;

	Memory_LinearAllocator memory;

	using DeviceRamBlock = uint8_t[MAX_DEVICE_SIZE];
	DriverSlot * driverSlots;
	DeviceRamBlock * deviceSlots;

	uint8_t freeSlotIndices[64];

	struct TupleNewDevice {
		USB::ContinueFunc func;
		USB::HubDevice * parent;
		uint8_t parentPortId;
	};
	TupleNewDevice newDeviceQ[MaxNewDevicesInFlight];
	TupleNewDevice inProgressNewDevice;
	bool deviceAddressLock;

	static constexpr int DMA_BUFFERSIZE = 512;
	static constexpr int MAX_DMA_BUFFERS_COUNT = 64;
	static constexpr int MAX_EVENT_COUNT = 32;
	static constexpr int MAX_PIPE_COUNT = 64;
	using DmaBuffer = uint8_t[DMA_BUFFERSIZE];

	Cadt::FreeList<DmaBuffer>* buffer512Freelist;

	/// allocate a block of ram (1MB currently) and setup the controller
	static Controller * Init( int verbose_);

	/// start the usb ports up
	static void Start( Controller * device_ );

	/// finish up and return the ram
	static void Fini( Controller * device_ );

	/// return the next command TRB handling linking
	static TRB::Template *GetNextCommandTRB( Controller *c_, uint8_t maxTDSize_ );

	/// pass the TRB from GetNextCommandTRB (thats been filled in) to the XHCI HW
	static TRB::Template *EnqueueCommandTRB( Controller *c_ );

};

ALWAYS_INLINE bool IsRootPortUSB2( Controller * device_, uint8_t portId_ ) {
	return (portId_ == 1) ? true : false;
}

ALWAYS_INLINE DriverSlot * GetDriverSlotFromSlotId( Controller * controller_, uint8_t slotId_ ) {
	return controller_->driverSlots + (slotId_ - 1);
}

ALWAYS_INLINE USB::Device * GetDeviceFromSlotId( Controller * controller_, uint8_t slotId_ ) {
	return (USB::Device *) (controller_->deviceSlots + (slotId_ - 1));
}

char const * SlotStateToString(SlotState slotState_);

void DumpCaps();

void DumpRootPort( uintptr_t portaddr_ );

void DumpCRCRBits( Controller * device_ );

void DumpUSBSTSBits( Controller * device_ );

void DumpUSBCMDBits( Controller * device_ );

void DumpGSTSBits( Controller * device_ );

void DumpIMan0Bits( Controller * device_ );

void Interrupter0BulkHandler( Controller * device_ );

void Interrupter0IsochronousHandler( Controller * device_ );

void Interrupter0ControllerHandler( Controller * device_ );

void Interrupter0ControlHandler( Controller * device_ );

void Interrupter0OTGHandler( Controller * device_ );

char const *SlotStateToString( SlotState slotState_ );


} // end namespace


#pragma GCC diagnostic pop