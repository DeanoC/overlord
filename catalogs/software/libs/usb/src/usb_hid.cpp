#include "core/core.h"
#include "usb/usb_hid.hpp"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "dbg/assert.h"
#include "usb/usb_pipe.hpp"
#include "usb/usb_hid_report.hpp"
#include "core/math.h"
#include "core/utils.hpp"
#include "platform/cache.h"

namespace USB {

enum class ReportInputType : uint8_t {
	XAxis,
	YAxis,
	ZAxis,
	Buttons,
	Hat
};
static char const * ReportInputTypeToString(ReportInputType type_);
static_assert((uint8_t)ReportInputType::XAxis == (uint8_t)HIDAxisDirection::X);
static_assert((uint8_t)ReportInputType::YAxis == (uint8_t)HIDAxisDirection::Y);
static_assert((uint8_t)ReportInputType::ZAxis == (uint8_t)HIDAxisDirection::Z);

enum class ReportOutputType : uint8_t {
};

struct ReportInputBlock {
	uint8_t id;
	ReportInputType type;
	uint8_t typeIndex;
	uint16_t minimum;
	uint16_t maximum;
	uint16_t leftShift;
	uint16_t bitSize;
};
struct ReportOutputBlock {
	uint8_t id;
	ReportOutputType type;
	uint16_t leftShift;
	uint16_t mask;
};

static void HIDInterruptIn( Event *event_ ) {
	auto hid = (HIDDevice *) event_->pipe->device;
	InterruptPipe::AddInInterrupt(&hid->inputPipe, hid->inputDMASize, hid->inputDMAAddr, &HIDInterruptIn, 0);

	Cache_DCacheInvalidateRange(hid->inputDMAAddr, Core::alignTo(hid->inputDMASize, 64));
	auto data = *((uint64_t *)hid->inputDMAAddr);

	for(int i = 0; i < hid->numRawInputBlocks;++i) {
		auto input = hid->rawInputBlocks + i;
		if(input->type == ReportInputType::Buttons) {
			hid->gamePad.buttons = (data >> input->leftShift) & BitOp_PowerOfTwoToMask_U32(1 << input->bitSize);
		} else {
			// decode the unsigned bit range passed on to a symmetric signed 16 bit int
			uint16_t urange = (input->maximum - input->minimum) >> 1;
			uint16_t uval = (data >> input->leftShift) & BitOp_PowerOfTwoToMask_U32( 1 << input->bitSize );
			auto shiftAmount = (16 - input->bitSize);
			if(uval < urange) {
				uval = (uval << shiftAmount) | 0;
			} else if(uval > urange) {
				uval = ((uval << shiftAmount) | BitOp_PowerOfTwoToMask_U16( 1 << input->bitSize )) - 1;
			} else {
				uval = ((uval << shiftAmount) | BitOp_PowerOfTwoToMask_U16( 1 << input->bitSize ));
			}
			auto val = (int16_t) (uval - ((urange << shiftAmount) | BitOp_PowerOfTwoToMask_U16( 1 << input->bitSize)));


			if(input->typeIndex <= 1 && (int)input->type < 3) {
				int index = input->typeIndex * 3 + (int)input->type;
				hid->gamePad.axisValues[index] = val;
			} else {
				// buttons or hat or > 2 of an axis
			}
		}
	}
	for(int i = 0; i < 32;i++) {
		if(hid->gamePad.buttons & (1 << i)) {
		debug_printf("Button %i pressed\n", i);
		}
	}
	for(int i = 0; i < HIDGamePad::MAX_AXES;i++) {
		if(hid->gamePad.axisValues[i] != 0) {
			debug_printf( "%s: %i\n", HIDAxisDirectionToString((HIDAxisDirection)i), hid->gamePad.axisValues[i]);
		}
	}
}

static void HIDInterfacesEnabled( Device *device_ ) {
	auto hid = (HIDDevice *) device_;
	debug_printf( "HIDInterfacesEnabled\n" );
	InterruptPipe::Init( &hid->inputPipe, hid, EndpointDescriptor::AddressToEndpointId( &hid->endpointDescriptors[0] ));
	if(hid->hasOutput) InterruptPipe::Init( &hid->outputPipe, hid, EndpointDescriptor::AddressToEndpointId( &hid->endpointDescriptors[1] ));

	InterruptPipe::AddInInterrupt(&hid->inputPipe, hid->inputDMASize, hid->inputDMAAddr, &HIDInterruptIn, 0);
}

namespace HIDReportMain {
	enum Flags : uint16_t {
		Const       = 0x001,
		Var         = 0x002,
		Rel         = 0x004,
		Wrap        = 0x008,
		NonLinear   = 0x010,
		NotPerfered = 0x020,
		NullState   = 0x040,
		Volatile    = 0x080,
		BufferBytes = 0x100,
	};
}
struct ReportGlobalState {
	HIDReportUsagePage usagePage;
	uint8_t reportId;
	uint8_t reportSize;
	uint8_t reportCount;
	uint32_t logicalMinimum;
	uint32_t logicalMaximum;
	uint32_t physicalMinimum;
	uint32_t physicalMaximum;
	uint8_t unit;
	static void Dump( ReportGlobalState const * state);
};

struct ReportLocalState {
	static constexpr int MAX_ITEMS = 32;
	uint8_t usage[MAX_ITEMS];
	uint8_t itemCount;

	static void Dump( ReportLocalState const * state);
};



struct ReportState {
	struct ReportStateContext * context;
	ReportGlobalState globalState;
	ReportLocalState localState;

	int32_t ignoreCollection;

	HIDReportCollectionType curCollection;
	HIDReportGlobalDesktopUsage globalDesktopApplicationType;

};

struct ReportStateContext {
	static constexpr int MAX_STACK_DEPTH = 4;
	static constexpr int MAX_INPUT_BLOCKS = 128;
	static constexpr int MAX_OUTPUT_BLOCKS = 32;

	uint16_t curInputBitCount;
	uint16_t curOutputBitCount;

	uint8_t curDepth;
	uint8_t numInputs;
	uint8_t numOutputs;

	ReportState stack[MAX_STACK_DEPTH];
	ReportInputBlock inputs[MAX_INPUT_BLOCKS];
	ReportOutputBlock outputs[MAX_OUTPUT_BLOCKS];

};

void ButtonPageInput( ReportState const *reportState, uint32_t data );
void GlobalDesktopPageInput( ReportState const *reportState, uint32_t data );

void ProcessMainReport( ReportState * reportState, HIDReportMainTag mainTag, uint32_t data) {
	// collections we don't support are ignored
	if(mainTag == HIDReportMainTag::Collection) {
		auto collectionType = (HIDReportCollectionType) data;
		if(   collectionType != HIDReportCollectionType::Application &&
					collectionType != HIDReportCollectionType::Logical) {
			reportState->ignoreCollection++;
			reportState->localState.itemCount = 0; // resets any buffers being formed
			return;
		}
		if(reportState->curCollection == HIDReportCollectionType::Application ){
			if(reportState->localState.itemCount == 1){
				reportState->globalDesktopApplicationType = (HIDReportGlobalDesktopUsage)reportState->localState.usage[0];
			}
		}
		reportState->localState.itemCount = 0; // resets any buffers being formed
		reportState->curCollection = collectionType;
		return;
	} else if(mainTag == HIDReportMainTag::EndCollection) {
		if(reportState->ignoreCollection > 0) reportState->ignoreCollection--;
		return;
	}

	if(reportState->ignoreCollection > 0) return;

	if(mainTag == HIDReportMainTag::Input) {
		auto usagePage = reportState->globalState.usagePage;
		switch(usagePage) {
			case HIDReportUsagePage::Buttons: ButtonPageInput( reportState, data );
				break;
			case HIDReportUsagePage::GenericDesktop: GlobalDesktopPageInput( reportState, data );
				break;
			default: // ignore any pages we don't understand or want to understand...
				debug_printf("HID Report: Unsupported Usage Page %s(%#06hx)\n", HIDReportUsagePageToString(usagePage), (uint16_t)usagePage);
				break;
		}
		reportState->localState.itemCount = 0;
	}
}

void ButtonPageInput( ReportState const *reportState, uint32_t data ) {
	auto reportContext = reportState->context;
	if(data & HIDReportMain::Flags::Const) {
		reportContext->curInputBitCount += reportState->globalState.reportCount * reportState->globalState.reportSize;
		return;
	}
	auto newInput = &reportContext->inputs[reportContext->numInputs++];
	newInput->type = ReportInputType::Buttons;
	newInput->id = reportState->globalState.reportId;
	newInput->leftShift = reportContext->curInputBitCount;
	newInput->bitSize = reportState->globalState.reportCount * reportState->globalState.reportSize;
	reportContext->curInputBitCount += newInput->bitSize;
}

void GlobalDesktopPageInput( ReportState const *reportState, uint32_t data) {
	auto reportContext = reportState->context;

	// constants are just gaps in the report data, we just skip over them
	if(data & HIDReportMain::Flags::Const) {
		reportContext->curInputBitCount += reportState->globalState.reportSize * reportState->globalState.reportCount;
		return;
	}

	// only var, abs, nowrap, linear, prefered, non n ull, not voltile, bits are supported
	if( data != HIDReportMain::Flags::Var) {
		debug_printf("HID Report: Unsupported input flags: ");
		if(data & HIDReportMain::Flags::Var) debug_printf( "var " ); else debug_printf( "array " );
		if(data & HIDReportMain::Flags::Rel) debug_printf( "rel " ); else debug_printf( "abs " );
		if(data & HIDReportMain::Flags::Wrap) debug_printf( "wrap " ); else debug_printf( "nowrap " );
		if(data & HIDReportMain::Flags::NonLinear) debug_printf( "nonlinear " ); else debug_printf( "linear " );
		if(data & HIDReportMain::Flags::NotPerfered) debug_printf( "nonprefered " ); else debug_printf( "prefered " );
		if(data & HIDReportMain::Flags::NullState) debug_printf( "null " ); else debug_printf( "nonull " );
		if(data & HIDReportMain::Flags::Volatile) debug_printf( "volatile " );
		if(data &HIDReportMain::Flags:: BufferBytes) debug_printf( "buffered bytes" ); else debug_printf( "bits" );
		debug_print("\n");
		reportContext->curInputBitCount += reportState->globalState.reportSize * reportState->globalState.reportCount;
		return;
	}

	for(int i = 0; i < reportState->globalState.reportCount; ++i) {
		int index = Math_Min_I32(i, reportState->localState.itemCount);
		auto usage =(HIDReportGlobalDesktopUsage)reportState->localState.usage[index];
		switch(usage) {
			case HIDReportGlobalDesktopUsage::X:
			case HIDReportGlobalDesktopUsage::Y:
			case HIDReportGlobalDesktopUsage::Z: {
				auto newInput = &reportContext->inputs[reportContext->numInputs++];
				newInput->type = (ReportInputType)((int)ReportInputType::XAxis + (int)usage - (int)HIDReportGlobalDesktopUsage::X);
				newInput->id = reportState->globalState.reportId;
				newInput->leftShift = reportContext->curInputBitCount;
				newInput->bitSize = reportState->globalState.reportSize;
				newInput->minimum = reportState->globalState.logicalMinimum;
				newInput->maximum = reportState->globalState.logicalMaximum;
				reportContext->curInputBitCount += newInput->bitSize;
				break;
			}
			default: // ignore these bits in the input buffer as we don't know what to do with them
				debug_printf("HID Report: GlobalDesktop Usage not supported %s(%#04hhx)\n", HIDReportGlobalDesktopUsageToString(usage), (int)usage);
				reportContext->curInputBitCount += reportState->globalState.reportSize;
				break;
		}
	}
}

void ProcessGlobalReport(ReportState * reportState, HIDReportGlobalTag globalTag, uint32_t data) {
	if(reportState->ignoreCollection > 0) return;

	switch(globalTag) {
		case HIDReportGlobalTag::UsagePage:
			reportState->globalState.usagePage = (HIDReportUsagePage)data;
			return;
		case HIDReportGlobalTag::LogicalMinimum:
			reportState->globalState.logicalMinimum = data;
			return;
		case HIDReportGlobalTag::LogicalMaximum:
			reportState->globalState.logicalMaximum = data;
			return;
		case HIDReportGlobalTag::PhysicalMinimum:
			reportState->globalState.physicalMinimum = data;
			return;
		case HIDReportGlobalTag::PhysicalMaximum:
			reportState->globalState.physicalMaximum = data;
			return;
		case HIDReportGlobalTag::UnitExponent:
			return;
		case HIDReportGlobalTag::Unit:
			return;
		case HIDReportGlobalTag::ReportSize:
			if(data <= 0xFF ) reportState->globalState.reportSize = data;
			return;
		case HIDReportGlobalTag::ReportId:
			if(data <= 0xFF ) reportState->globalState.reportId = data;
			return;
		case HIDReportGlobalTag::ReportCount:
			if(data <= 0xFF ) reportState->globalState.reportCount = data;
			return;
		case HIDReportGlobalTag::Push:
			return;
		case HIDReportGlobalTag::Pop:
			return;
		default: break;
	}
}

void ProcessLocalReport(ReportState * reportState, HIDReportLocalTag localTag, uint32_t data) {
	if(reportState->ignoreCollection > 0) return;

	switch(localTag) {
		case HIDReportLocalTag::Usage:
			if(data <= 0xFF) reportState->localState.usage[reportState->localState.itemCount++] = (uint8_t) data;
			break;
		case HIDReportLocalTag::UsageMinimum:
		case HIDReportLocalTag::UsageMaximum:
		case HIDReportLocalTag::DesignatorIndex:
		case HIDReportLocalTag::DesignatorMinimum:
		case HIDReportLocalTag::DesignatorMaximum:
		case HIDReportLocalTag::StringIndex:
		case HIDReportLocalTag::StringMinimum:
		case HIDReportLocalTag::StringMaximum:
		case HIDReportLocalTag::Delimiter:
			break;
	}

}

static void PostConfigureContext( Event *event_ ) {
	auto hid = (HIDDevice *) event_->pipe->device;
	auto hidReport = (uint8_t *) event_->dmaBuffer;
	auto const hidReportEnd = hidReport + (uint16_t) event_->arg;

	ReportStateContext reportStateContext {
		.curDepth = 0
	};
	ReportState * reportState = reportStateContext.stack;
	memset(reportState, 0, sizeof(ReportState));
	reportState->context = &reportStateContext;

	if(hid->controller->verbose > 0) DumpHIDReport( hidReport, hidReportEnd );

	while(hidReport < hidReportEnd) {
		if(*hidReport == 0xFE) {
			// long tags are ignored
			hidReport += 1 + *(hidReport + 1);
		} else {
			// short tag
			uint8_t const header = *hidReport;
			uint8_t const tag = header >> 4;
			uint8_t const size = ((header & 0x3) == 0x3) ? 4 : (header & 0x3);
			auto type = (HIDReportShortItemType) ((header >> 2) & 0x3);
			uint32_t data = 0;
			switch(size) {
				case 1: data = *(hidReport + 1); break;
				case 2: data = *(uint16_t *) (hidReport + 1); break;
				case 4: data = *(uint32_t *) (hidReport + 1); break;
				default:;
			}

			switch(type) {
				case HIDReportShortItemType::Main:;
					ProcessMainReport(reportState, (HIDReportMainTag) tag, data);
					break;
				case HIDReportShortItemType::Global:;
					ProcessGlobalReport(reportState, (HIDReportGlobalTag) tag, data);
					break;
				case HIDReportShortItemType::Local:;
					ProcessLocalReport(reportState, (HIDReportLocalTag) tag, data);
					break;
				case HIDReportShortItemType::Reserved:break;
			}

			hidReport += 1 + size;
		}
	}

	hid->numRawInputBlocks = reportStateContext.numInputs;
	hid->numRawOutputBlocks = reportStateContext.numOutputs;
	hid->rawInputBlocks = (ReportInputBlock *) hid->dmaAllocator.allocatorFuncs.malloc((Memory_Allocator*)&hid->dmaAllocator, reportStateContext.numInputs * sizeof(ReportInputBlock));
	hid->rawOutputBlocks = (ReportOutputBlock *) hid->dmaAllocator.allocatorFuncs.malloc((Memory_Allocator*)&hid->dmaAllocator, reportStateContext.numOutputs * sizeof(ReportOutputBlock));

	static const int numReportTypes = (int)ReportInputType::Hat + 1;
	uint8_t typeIndex[numReportTypes];
	memset(typeIndex, 0, numReportTypes);

	for(int i = 0; i < hid->numRawInputBlocks;++i) {
		auto input = &reportStateContext.inputs[i];
		memcpy(hid->rawInputBlocks + i, input, sizeof(ReportInputBlock) );
		hid->rawInputBlocks[i].typeIndex = typeIndex[(uint8_t)input->typeIndex];
		typeIndex[(uint8_t)input->typeIndex]++;
	}
	// dma size
	auto dmaSize = Core::alignTo((reportStateContext.curInputBitCount + 7) / 8, 8);
	hid->inputDMAAddr = (uintptr_t) hid->dmaAllocator.allocatorFuncs.aalloc((Memory_Allocator*)&hid->dmaAllocator, dmaSize, 64);
	hid->inputDMASize = dmaSize;

	hid->controlPipe->postConfigureContext( hid->controlPipe, 0, hid->hasOutput ? 2 : 1, hid->endpointDescriptors );
}

static void PostSetInterfaceProtocol( Event *event_ ) {
	auto hid = (HIDDevice *) event_->pipe->device;
	if(hid->hasOutput) EndpointDescriptor::Dump( hid->endpointDescriptors + 1 );

	// get the HID report descriptor
	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Standard, Interface ),
		.code = USB::RequestCode::GetDescriptor,
		.wValue = 0x2200, // HID report
		.wIndex = (uint16_t) 0,
		.wLength = (uint16_t) event_->arg,
	};
	ControlPipe::PostRequest( hid->controlPipe, &requestData, &PostConfigureContext, event_->arg );

}

static void HIDConfigurationDescriptorReady( Device *device_, ConfigurationDescriptor *config_, InterfaceDescriptor *interface ) {
	auto hid = (HIDDevice *) device_;
	assert( config_ );

	PERSISTANT_MEMORY_BUFFER_ALLOCATOR(hid->dmaAllocator, Core::alignTo((uintptr_t)(hid+1), 64), 1024 - sizeof(HIDDevice));

	auto hidDescriptor = (HIDDescriptor *) (interface + 1);
	HIDDescriptor::Dump( hidDescriptor );
	assert( hidDescriptor->numDescriptors > 0 );
	auto *reportHeader = (uint8_t *) (hidDescriptor + 1);

	// we only care about 1st report descriptor (not sure can have more than 1?)
	uint16_t reportDescriptorLength = 0;
	for(int j = 0; j < hidDescriptor->numDescriptors; ++j) {
		if(reportHeader[0] == 0x22 && reportDescriptorLength == 0) reportDescriptorLength = *(uint16_t *) (reportHeader + 1);
		reportHeader += 3;
	}
	assert( reportDescriptorLength != 0 );

	// copy endpoints and decide if this HID device has output
	auto endpoints = (EndpointDescriptor *) (reportHeader);
	hid->hasOutput = (interface->numEndpoints == 2) ? true : false;
	memcpy( hid->endpointDescriptors + 0, endpoints + 0, sizeof( EndpointDescriptor ));
	if(hid->hasOutput) memcpy( hid->endpointDescriptors + 1, endpoints + 1, sizeof( EndpointDescriptor ));

	// make sure we are using the HID protocol and not boot protocol
	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( HostToDevice, Class, Interface ),
		.code = USB::RequestCode::SetInterface,
		.wValue = 0x1,
		.wIndex = (uint16_t) 0,
		.wLength = 0,
	};
	ControlPipe::PostRequest( hid->controlPipe, &requestData, &PostSetInterfaceProtocol, reportDescriptorLength );
}


void ReportGlobalState::Dump( ReportGlobalState const * state) {
	debug_printf("%s: id %i bitCount %i count %i\n",
							 HIDReportUsagePageToString(state->usagePage), state->reportId, state->reportSize, state->reportCount);
	debug_printf("logicalMin %i logicalMax %i PhysicalMin %i PhysicalMax %i Unit %i\n",
	             state->logicalMinimum, state->logicalMaximum, state->physicalMinimum, state->physicalMaximum, state->unit);
}

void ReportLocalState::Dump( ReportLocalState const * state) {
	debug_printf( "Local items %i Usages:\n", state->itemCount);
	for(int i = 0;i < state->itemCount;++i) {
		debug_printf( "  %s\n", HIDReportGlobalDesktopUsageToString((HIDReportGlobalDesktopUsage)state->usage[i]));
	}
}

void HIDDescriptor::Dump( HIDDescriptor const *hidDescriptor_ ) {
	debug_printf( ANSI_BRIGHT_ON "HID Descriptor" ANSI_BRIGHT_OFF "\n" );
	debug_printf( "length %i, %#04x %s\n", hidDescriptor_->length, (uint8_t) hidDescriptor_->descriptorType, DescriptorTypeToString( hidDescriptor_->descriptorType ));
	debug_printf( "bcdVersion %#06x, countryCode %i\n", hidDescriptor_->bcdVersion, hidDescriptor_->countryCode );
	debug_printf( "numDescriptors %i\n", hidDescriptor_->numDescriptors );
}

static char const * ReportInputTypeToString(ReportInputType type_) {
	switch(type_) {
		case ReportInputType::XAxis: return "XAxis";
		case ReportInputType::YAxis: return "YAxis";
		case ReportInputType::ZAxis: return "ZAxis";
		case ReportInputType::Buttons: return "Buttons";
		case ReportInputType::Hat: return "Hat";
		default: return "UNKNOWN";
	}
}

char const * HIDAxisDirectionToString(HIDAxisDirection direction_) {
	switch(direction_) {
		case HIDAxisDirection::X: return "X";
		case HIDAxisDirection::Y: return "Y";
		case HIDAxisDirection::Z: return "Z";
		case HIDAxisDirection::X1: return "X1";
		case HIDAxisDirection::Y1: return "Y1";
		case HIDAxisDirection::Z1: return "Z1";
		default: return "UNKNOWN";
	}
}

ClassHandlerVTable HIDVTable{
	.enumerate = nullptr,
	.configurationDescriptorReady = &HIDConfigurationDescriptorReady,
	.interfacesEnabled = &HIDInterfacesEnabled
};

} // USB