#pragma once

#include "core/core.h"
#include "core/utf8.h"
#include "cadt/freelist.hpp"

namespace USB {

struct Device;

enum class CompletionCode : uint8_t {
	Invalid = 0,
	Success = 1,
	DataBufferError = 2,
	BabbleDetectedError = 3,
	USBTransactionError = 4,
	Error = 5,
	StallError = 6,
	ResourceError = 7,
	BandwidthError = 8,
	NoSlotsAvailableError = 9,
	InvalidStreamTypeError = 10,
	SlotNotEnabledError = 11,
	EndpointNoEnabledError = 12,
	ShortPacket = 13,
	RingUnderrun = 14,
	RingOverrun = 15,
	VFEventRingFullError = 16,
	ParameterError = 17,
	BandwidthOverrunError = 18,
	ContextStateError = 19,
	NoPointResourceError = 20,
	EventRingFullError = 21,
	IncompatibleDeviceError = 22,
	MissedServiceError = 23,
	CommandRingStopped = 24,
	CommandAborted = 25,
	Stopped = 26,
	StoppedLengthInvalid = 27,
	DebugAbort = 28,
	StoppedShortPacket = 29,
	Reserved = 30,
	IsochBufferOverrun = 31,
	EventLostError = 32,
	UndefinedEvent = 33,
	InvalidStreamIDError = 34,
	SecondaryBandwidthError = 35,
	SplitTransactionError = 36,

	VendorDefinedErrorStart = 192,
	VendorDefinedInfoStart = 224,
};


constexpr uint8_t RequestTypeHostToDevice = 0 << 7;
constexpr uint8_t RequestTypeDeviceToHost = 1 << 7;
constexpr uint8_t RequestTypeStandard = 0 << 5;
constexpr uint8_t RequestTypeClass = 1 << 5;
constexpr uint8_t RequestTypeVendor = 2 << 5;
constexpr uint8_t RequestTypeDevice = 0 << 0;
constexpr uint8_t RequestTypeInterface = 1 << 0;
constexpr uint8_t RequestTypeEndpoint = 2 << 0;
constexpr uint8_t RequestTypeOther = 3 << 0;

#define USB_MAKE_REQUEST_TYPE(d,t,r) (uint8_t)(USB::RequestType##d | USB::RequestType##t | USB::RequestType##r)

enum class RequestCode : uint8_t {
	GetStatus = 0,
	ClearFeature = 1,
	SetFeature = 3,
	SetAddress = 5,
	GetDescriptor = 6,
	SetDescriptor = 7,
	GetConfiguration = 8,
	SetConfiguration = 9,
	GetInterface = 10,
	SetInterface = 11,
	SynchFrame = 12,
	SetEncryption = 13,
	GetEncryption = 14,
	SetHandShake = 15,
	GetHandShake = 16,
	SetConnection = 17,
	SetSecurity = 18,
	GetSecurity = 19,
	SetWUSBData = 20,
	LoopBackDataWrite = 21,
	LoopBackDataRead = 22,
	SetInterfaceDS = 23,
	GetFWStatus = 26,
	SetFWStatus = 27,
	SetSel = 48,
	SetIsochDelay = 49
};


enum class StandardFeatureSelectors : uint8_t {
	EndpointHalt = 0,
	FunctionSuspend = 0,
	DeviceRemoteWakeup = 1,
	TestMode = 2,
	b_hnp_enable = 3,
	a_hnp_support = 4,
	a_alt_hnp_support = 5,
	WUSB_Device = 6,
	U1Enable = 48,
	U2Enable = 49,
	LTMEnable = 50,
	B3_NTF_HOST_REL = 51,
	B3_RSPEnable = 52,
	LDMEnable = 53,
};

enum class Direction : uint8_t {
	Out = 0,
	In = 1,
};

enum class TransferType : uint8_t {
	NoDataStage = 0,
	Reserved = 1,
	OutDataStage = 2,
	InDataStage = 3,
};

enum class EndpointTransportType : uint8_t {
	Control = 0,
	Isochronous = 1,
	Bulk = 2,
	Interrupt = 3
};

enum class Speed : uint8_t {
	FullSpeed = 1,
	LowSpeed = 2,
	HighSpeed = 3,
	SuperSpeed = 4,
};

struct PACKED RequestData {
	uint8_t requestType;
	USB::RequestCode code;
	uint16_t wValue;

	uint16_t wIndex;
	uint16_t wLength;

	static void Dump( RequestData const * requestData);

};
static_assert(sizeof(RequestData) == 8);


struct Pipe;
struct Event;
enum class ClassCode : uint8_t;

typedef void (*EventCallbackFunc)(Event * event_ );

struct PACKED Event {
	EventCallbackFunc callback;
	Pipe * pipe;
	uint64_t arg;
	uintptr_all_t dmaBuffer; // allocated and released automatically
};
static_assert(sizeof(Event) == 32);

typedef void (*ContinueFunc)(struct Controller * controller_, uint64_t arg0_, uint64_t arg1_);

// low level usb controller functions
typedef void (*ControlRequestFunc)( Controller * controller_,
                                    uint32_t slotId_,
                                    RequestData const * requestData_,
																		Event * event_);

typedef void (*NormalTransferFunc)( Controller * controller_,
                                    uint32_t slotId_,
																		uint8_t endpointId_,
																		uint32_t dataLength_,
																		uintptr_all_t data_,
																		Event * event_);
typedef void (*EventFunc)( Controller * controller_,
                                    uint32_t slotId_,
                                    uint8_t endpointId_,
                                    Event * event_);

// high level usb controller functions
typedef void (*NewDeviceFunc)(Controller * c_,struct HubDevice * parentHub_, uint8_t parentPortId_, USB::ContinueFunc func_);

struct Controller {
	static constexpr int MAX_DEVICE_SIZE = 1024;
	static constexpr int MAX_PIPE_SIZE = 128;

	ControlRequestFunc postControlRequest;
	NormalTransferFunc postNormalTransfer;
	EventFunc          postEvent;

	NewDeviceFunc newDeviceFunc;

	using PipeRamBlock = uint8_t[MAX_PIPE_SIZE];
	Cadt::FreeList<USB::Event>* eventFreelist;
	Cadt::FreeList<PipeRamBlock>* pipeFreelist;

	int verbose; // >0 to dump info to log (higher number more stuff)

	// mostly empty but 1 entry for each possible class 1KB (32 bit) or 2KB (64 bit)
	struct ClassHandlerVTable * classHandlerTable[256];
};

char const * SpeedToString( Speed speed_ );

char const * ClassCodeToString( ClassCode type_ );

const char * CompletionCodeToString( CompletionCode code_ );

const char * RequestCodeToString( RequestCode requestCode_ );

const char * StandardFeatureSelectorsToString( StandardFeatureSelectors standardFeatureSelectors_ );

char const * DirectionToString( Direction dataDirection_ );

char const * TransferTypeToString( TransferType transferType_ );

char const * EndpointTransportTypeToString( EndpointTransportType endpointTransportType_);

} // end namespace