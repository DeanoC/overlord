#include "core/core.h"
#include "core/utils.hpp"
#include "core/bitops.h"
#include "core/math.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "dbg/assert.h"
#include "platform/registers/usb3_xhci.h"
#include "platform/registers/usb3_regs.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/cache.h"
#include "utils/busy_sleep.h"
#include "memory/memory.h"
#include "osservices/osservices.h"
#include "zynqps8/xhci/trb.hpp"
#include "zynqps8/xhci/xhci.hpp"
#include "gpio/gpio.hpp"
#include "zynqps8/i2c/i2c.hpp"
#include "core/unicode.h"
#include "usb/usb.hpp"
#include "usb/usb_device.hpp"
#include "usb/usb_pipe.hpp"
#include "usb/usb_hub.hpp"
#include "usb/usb_hid.hpp"


namespace XHCI {
/* Xilinx XHCI is a Synopsis Designware DWC 3.0 IP Core - some notes from there docs from the interwebs
 * Known Limitations
 * Like any other HW, DWC3 has its own set of limitations. To avoid constant questions about such problems, we decided
 * to document them here and have a single location to where we could point users.
 *
 * OUT Transfer Size Requirements
 * According to Synopsys Databook, all OUT transfer TRBs [1] must have their size field set to a value which is integer
 * divisible by the endpoint’s wMaxPacketSize.
 * This means that e.g. in order to receive a Mass Storage CBW [5], req->length must either be set to a value that’s
 * divisible by wMaxPacketSize (1024 on SuperSpeed, 512 on HighSpeed, etc), or DWC3 driver must add a Chained TRB
 * pointing to a throw-away buffer for the remaining length. Without this, OUT transfers will NOT start.
 *
 * Note that as of this writing, this won’t be a problem because DWC3 is fully capable of appending a chained TRB for
 * the remaining length and completely hide this detail from the gadget driver.
 * It’s still worth mentioning because this seems to be the largest source of queries about DWC3 and non-working transfers.
 *
 * TRB Ring Size Limitation
 * We, currently, have a hard limit of 256 TRBs [1] per endpoint, with the last TRB being a Link TRB [2] pointing back
 * to the first. This limit is arbitrary but it has the benefit of adding up to exactly 4096 bytes, or 1 Page.
 */

static void TryPopNewDeviceQ( USB::Controller *c_ );

static void HandleEnableSlot( USB::Controller *c_, uint8_t slotId_ );

static uintptr_t EndAlignAddress( uintptr_t addr_, size_t size_, uint32_t pageSize_ ) {
	uintptr_t const endAddress = addr_ + size_;
	if((addr_ & BitOp_CountToRightmostMask_U32( pageSize_ )) !=
	   (endAddress & BitOp_CountToRightmostMask_U32( pageSize_ ))) {
		return endAddress & BitOp_CountToRightmostMask_U32( pageSize_ );
	}
	return addr_;
}

static void ResetPort( bool usb2 ) {
	int timeout = 2000;
	if(usb2) {
		debug_printf( ANSI_YELLOW_PEN ANSI_BRIGHT_ON "Resetting USB2 port" ANSI_WHITE_PEN ANSI_BRIGHT_OFF"\n" );
		// make sure reset satus bit is clear
		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_20, PRC );

		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_20, PR );
		while(!HW_REG_GET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_20, PRC ) && timeout > 0) {
			Utils_BusyMilliSleep( 1 );
			timeout--;
		}
		if(timeout == 0) {
			debug_print( ANSI_RED_PAPER ANSI_BLINK_ON "USB 2 reset port timed out" ANSI_BLACK_PAPER ANSI_BLINK_OFF "\n" );
		}
		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_20, PRC );
	} else {
		debug_printf( ANSI_YELLOW_PEN "Resetting USB3 port" ANSI_WHITE_PEN "\n" );
		// make sure reset satus bit is clear
		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_30, PRC );

		timeout = 200;
		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_30, PR );
		while(!HW_REG_GET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_30, PRC ) && timeout > 0) {
			Utils_BusyMilliSleep( 10 );
			timeout--;
		}
		if(timeout == 0) {
			debug_print( ANSI_RED_PAPER ANSI_BLINK_ON "USB 3 reset port timed out" ANSI_BLACK_PAPER ANSI_BLINK_OFF "\n" );
		}
		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_30, PRC );
	}
}

static void FutureNewSlotId( USB::Controller *c_ ) {
	auto c = (XHCI::Controller *) c_;
	if(c->freeSlotIndices[0] == 0) {
		MultiCore_LockRecursiveMutex( &c->commandMutex );
		// need to enable a new slot from the hardware
		FillInEnableSlot( Controller::GetNextCommandTRB( c, 1 ));
		Controller::EnqueueCommandTRB( c );

		MultiCore_UnlockRecursiveMutex( &c->commandMutex );
		dsb();
		HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DB0, 0 );
	} else {
		// we can re-use the head slot
		uint8_t slotId = c->freeSlotIndices[0];
		memcpy( c->freeSlotIndices, c->freeSlotIndices + 1, 63 );
		c->freeSlotIndices[63] = 0;
		HandleEnableSlot( c, slotId );
	}
}


static void PostControlRequest( USB::Controller *controller_,
                                uint32_t slotId_,
                                USB::RequestData const *requestData_,
                                USB::Event *event_ ) {
	auto controller = (Controller *) controller_;
	DriverSlot *driverSlot = GetDriverSlotFromSlotId( controller, slotId_ );

	if(controller_->verbose > 3) {
		debug_printf( "XHCI: PostControl Request TRB = %p\n",
		              (void *) DriverSlot::GetNextEndpointTRB( driverSlot, 1, 1 ));
		USB::RequestData::Dump( requestData_ );
	}

	MultiCore_LockRecursiveMutex( &controller->commandMutex );
	uintptr_all_t dmaBuffer = 0;
	auto const maxPacketSize = driverSlot->deviceContext.controlEndPoint.maxPacketSize;
	assert(maxPacketSize != 0);

	auto tdSize = 2 + ((requestData_->wLength > 0) ? 1 : 0) + (event_ ? 1 : 0);

	// first up setup stage
	FillInSetupStage( DriverSlot::GetNextEndpointTRB( driverSlot, 1, tdSize--), requestData_ );
	DriverSlot::EnqueueEndpointTRB( driverSlot, 1 );

	assert( requestData_->wLength <= Controller::DMA_BUFFERSIZE );

	if(requestData_->wLength > 0) {
		// optional data stage to where we store the data
		dmaBuffer = (uintptr_all_t) controller->buffer512Freelist->alloc();
		assert( dmaBuffer );
		assert((dmaBuffer & 0x3F) == 0 );

		FillInInDataStage( DriverSlot::GetNextEndpointTRB( driverSlot, 1, tdSize ), requestData_->wLength, dmaBuffer, tdSize );
		tdSize--;
		DriverSlot::EnqueueEndpointTRB( driverSlot, 1 );
	}

	if(dmaBuffer) Cache_DCacheInvalidateRange( dmaBuffer, Controller::DMA_BUFFERSIZE );

	// status stage interrupt depends on event data or not
	FillInStatusStage( DriverSlot::GetNextEndpointTRB( driverSlot, 1, tdSize-- ),
	                   USB::Direction::Out,
	                   event_ == nullptr );
	DriverSlot::EnqueueEndpointTRB( driverSlot, 1 );

	// add event data stage if used
	if(event_) {
		// if we are moving data add the dma buffer we allocated
		if(requestData_->wLength > 0) event_->dmaBuffer = dmaBuffer;
		FillInEventData( DriverSlot::GetNextEndpointTRB( driverSlot, 1, tdSize-- ), (uintptr_all_t) event_ );
		DriverSlot::EnqueueEndpointTRB( driverSlot, 1 );
	}

	MultiCore_UnlockRecursiveMutex( &controller->commandMutex );

	assert(tdSize == 0);
	dsb();
	hw_RegWrite( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI_DB0_OFFSET + (slotId_ * 4), 1 );
}

static void PostNormalTransfer( USB::Controller *controller_,
                                uint32_t slotId_,
                                uint8_t endpointId_,
                                uint32_t dataLength_,
                                uintptr_all_t data_,
                                USB::Event *event_ ) {
	auto c = (Controller *) controller_;
	DriverSlot *driverSlot = GetDriverSlotFromSlotId( c, slotId_ );

	uint16_t const maxPacketSize = driverSlot->deviceContext.endPoints[endpointId_].maxPacketSize;
	assert(dataLength_ > 0);
	assert(dataLength_ < 64 * 1024); // TODO split up big transfers?
	assert(maxPacketSize > 0);
	if(event_) assert(event_->dmaBuffer == 0);

	auto tdSize = 1 + (event_ ? 1 : 0);

	MultiCore_LockRecursiveMutex( &c->commandMutex );

	FillInNormal( DriverSlot::GetNextEndpointTRB( driverSlot, endpointId_, tdSize ), dataLength_, data_, tdSize );
	tdSize--;
	DriverSlot::EnqueueEndpointTRB( driverSlot, endpointId_ );

	FillInEventData( DriverSlot::GetNextEndpointTRB( driverSlot, endpointId_, tdSize-- ), (uintptr_all_t) event_ );
	DriverSlot::EnqueueEndpointTRB( driverSlot, endpointId_ );

	MultiCore_UnlockRecursiveMutex( &c->commandMutex );

	assert(tdSize == 0);
	dsb();
	hw_RegWrite( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI_DB0_OFFSET + (slotId_ * 4), endpointId_ );

}

static void PostEvent( USB::Controller *controller_,
                       uint32_t slotId_,
                       uint8_t endpointId_,
                       USB::Event *event_ ) {
	assert(event_);
	assert(event_->dmaBuffer == 0);

	auto controller = (Controller *) controller_;

	DriverSlot *driverSlot = GetDriverSlotFromSlotId( controller, slotId_ );

	MultiCore_LockRecursiveMutex( &controller->commandMutex );

	FillInEventData( DriverSlot::GetNextEndpointTRB( driverSlot, endpointId_, 1 ), (uintptr_all_t) event_ );
	DriverSlot::EnqueueEndpointTRB( driverSlot, endpointId_ );

	MultiCore_UnlockRecursiveMutex( &controller->commandMutex );
	dsb();
	hw_RegWrite( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI_DB0_OFFSET + (slotId_ * 4), endpointId_ );
}

// set into control posts postConfigureContext vtable
static void PostConfigureContext( USB::ControlPipe *controlPipe_,
                                  uint8_t numHubPorts_, // 0 for non hubs
                                  int numEPs,
                                  USB::EndpointDescriptor *eps_ ) {
	auto device = controlPipe_->device;
	auto c = (Controller *) device->controller;
	auto driverSlot = GetDriverSlotFromSlotId( c, device->slotId );
	//if(c->verbose > 3)
	// TODO HACK without this print, the ConfigureEndpoint callback doesn't happen...
	// It seems to be the write to the IPI trigger that causes something to serialise
	// its not purely timing (replacing print with busy wait DOESN'T fix up)
	// sometimes even with this it fails on reset of a port (never get the callback)
	debug_print( ANSI_BRIGHT_ON ANSI_YELLOW_PEN "PostConfigureContext\n" ANSI_RESET_ATTRIBUTES );

	dsb();

	// update from the xhci hardware version
	Cache_DCacheInvalidateRange((uintptr_t) c->contextBaseAddressArray[device->slotId], sizeof( DeviceContext ));
	assert( c->contextBaseAddressArray[device->slotId]->slotContext.slotState != SlotState::Disabled );
	memcpy( &driverSlot->deviceContext, c->contextBaseAddressArray[device->slotId], sizeof( DeviceContext ));

	InputContext *inputCtx = &driverSlot->inputContext;
	SlotContext *slotCtx = &driverSlot->deviceContext.slotContext;

	inputCtx->dropContextFlags = 0;
	inputCtx->addContextFlags = 0;
	slotCtx->contextEntryCount = 0;

	if(numHubPorts_) {
		auto parhub = (USB::HubDevice *) device->parentHub;
		auto hub = (USB::HubDevice *) device;
		slotCtx->hub = 1;
		slotCtx->hubPortCount = numHubPorts_;
		slotCtx->multiTT = false;
		hub->numPorts = numHubPorts_;
		if(parhub != nullptr) {
			hub->hubDepth = parhub->hubDepth + 1;
			slotCtx->parentHubSlotId = parhub->slotId;
//			slotCtx->parentPortNumber = device->
		} else {
			hub->hubDepth = 0;
		}
	}

	auto allocator = &c->memory.allocatorFuncs;
	for(int i = 0; i < numEPs; ++i) {
		USB::EndpointDescriptor const *endpointDescriptor = &eps_[i];
		int const address = eps_[i].address & 0xF;
		int const in = eps_[i].address >> 7;
		int id = ((address * 2) + in);
		SlotEndPoint *epc = &driverSlot->deviceContext.endPoints[id];

		inputCtx->addContextFlags |= (1 << id);

		// if first time this end point is used, allocate an event ring
		if(epc->TRBDequeuePointer == nullptr) {
			c->memory.current = (uint8_t *) EndAlignAddress((uintptr_t) c->memory.current, 256 * sizeof( TRB::Template ), c->pageSize );
			driverSlot->deviceBaseAddress[id] = (TRB::Template *) allocator->aalloc( allocator, 256 * sizeof( TRB::Template ), 64 );
			epc->TRBDequeuePointer = driverSlot->deviceBaseAddress[id];
			driverSlot->currentCycleBit[id] = false;
			Cache_DCacheZeroRange((uintptr_t)epc->TRBDequeuePointer, 256 * sizeof( TRB::Template ));
			Cache_DCacheCleanAndInvalidateRange((uintptr_t) epc->TRBDequeuePointer, 256 * sizeof( TRB::Template ));
		}
		epc->maxPacketSize = endpointDescriptor->maxPacketSize;
		epc->endpointType = (EndpointType) ((endpointDescriptor->attributes & 0xF) + ((eps_[i].address >> 5)));
		epc->averageTRBLength = 8;
		epc->errorCount = 3;
		epc->interval = endpointDescriptor->interval;
		epc->maxESITPayloadLo = 8;//endpointDescriptor->maxPacketSize;
	}

	Cache_DCacheCleanRange((uintptr_t) driverSlot, sizeof( DriverSlot ));
	MultiCore_LockRecursiveMutex( &c->commandMutex );
	TRB::FillInConfigureEndpoint( Controller::GetNextCommandTRB( c, 1 ), device->slotId, (uintptr_t) driverSlot );
	Controller::EnqueueCommandTRB( c );
	MultiCore_UnlockRecursiveMutex( &c->commandMutex );
	dsb();
	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DB0, 0 );
}

static void PostEvaluateContext( USB::Controller *controller_, uint8_t slotId_, uint16_t newMaxPacketSize_) {
	auto c = (Controller *) controller_;
	if(c->verbose > 3) debug_printf( ANSI_BRIGHT_ON ANSI_YELLOW_PEN "PostEvaluateContext\n" ANSI_RESET_ATTRIBUTES );
	assert(newMaxPacketSize_ > 0);
	auto driverSlot = GetDriverSlotFromSlotId( c, slotId_ );

	// update from the xhci hardware version
	Cache_DCacheInvalidateRange((uintptr_t) c->contextBaseAddressArray[slotId_], sizeof( DeviceContext ));
	assert( c->contextBaseAddressArray[slotId_]->slotContext.slotState != SlotState::Disabled );
	memcpy( &driverSlot->deviceContext, c->contextBaseAddressArray[slotId_], sizeof( DeviceContext ));

	InputContext *inputCtx = &driverSlot->inputContext;
	SlotContext *slotCtx = &driverSlot->deviceContext.slotContext;

	inputCtx->dropContextFlags = 0;
	inputCtx->addContextFlags = 1;
	slotCtx->contextEntryCount = 0;
	driverSlot->deviceContext.controlEndPoint.maxPacketSize = newMaxPacketSize_;

	Cache_DCacheCleanRange((uintptr_t) driverSlot, sizeof( DriverSlot ));
	MultiCore_LockRecursiveMutex( &c->commandMutex );
	TRB::FillInEvaluateContext( Controller::GetNextCommandTRB( c, 1 ), slotId_, (uintptr_t) driverSlot );
	Controller::EnqueueCommandTRB( c );
	MultiCore_UnlockRecursiveMutex( &c->commandMutex );
	dsb();
	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DB0, 0 );
}

static void XHCISyncedCallback( USB::Event *event_ ) {
	assert( event_ );
	assert( event_->pipe );
	assert( event_->pipe->device );

	auto const func = (USB::ContinueFunc) event_->arg;
	func( event_->pipe->device->controller, (uint64_t) event_, 0 );
}

static void PostNewDeviceCallback( USB::Event *event_ ) {
	assert( event_ );
	assert( event_->pipe );
	assert( event_->pipe->device );

	auto c = (Controller *) event_->pipe->device->controller;
	auto newDevice = c->inProgressNewDevice;

	// device now an address and data saved, so another device can proceed
	c->deviceAddressLock = false;
	TryPopNewDeviceQ( c );

	auto const func = (USB::ContinueFunc) newDevice.func;
	func( event_->pipe->device->controller, (uint64_t) event_->pipe->device , newDevice.parentPortId );
}

static void GetDeviceDescriptorStringSizeCallback( USB::Event *event_);

static void GetDeviceDescriptionUSBString( USB::Device *device_, uint8_t stringIndex_) {
	// first return the 1 byte size
	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Standard, Device ),
		.code = USB::RequestCode::GetDescriptor,
		.wValue = (uint16_t) (((uint16_t) USB::DescriptorType::String) << 8 | stringIndex_),
		.wIndex = (uint16_t) 0x409, // english-american
		.wLength = 1,
	};
	USB::ControlPipe::PostRequest( device_->controlPipe, &requestData, GetDeviceDescriptorStringSizeCallback, stringIndex_ );
}

static void FinishDeviceDescriptorStringTransferCallback( USB::Event *event_ ) {
	assert( event_ );
	assert( event_->pipe );
	assert( event_->pipe->device );
	assert( event_->pipe->device->controller );

	auto device = event_->pipe->device;

	// unpack args
	utf8_int8_t * stringDest = nullptr;
	if(event_->arg == device->descriptor.manufacturerIndex) stringDest = device->manufacturerString;
	else if(event_->arg == device->descriptor.productIndex) stringDest = device->productString;
	else if(event_->arg == device->descriptor.serialNumberIndex) stringDest = device->serialNumberString;
	else {
		debug_printf(ANSI_RESET_ATTRIBUTES ANSI_RED_PAPER ANSI_BLINK_ON "XHCI: Unknown device string index %lu" ANSI_RESET_ATTRIBUTES "\n", event_->arg);
		return;
	}

	Cache_DCacheInvalidateRange(event_->dmaBuffer, Controller::DMA_BUFFERSIZE);
	uint16_t const *const ucs16 = (uint16_t *) event_->dmaBuffer;
	// first byte length include 2 byte header (1 byte length, 0x3)
	uint16_t ucsLen = (*((uint8_t *) ucs16) - 2) / 2;
	uint32_t utf8Size = Core_UCS2toUTF8Len( ucs16 + 1, ucsLen );

	auto tmpBuf = (utf8_int8_t *)STACK_ALLOC(utf8Size);
	Core_UCS2toUTF8( ucs16 + 1, ucsLen, (utf8_int8_t *) tmpBuf );
	uint32_t utf8Count = Math_Min_U32(utf8Size, USB::Device::MaxStringSize - 1);
	memcpy(stringDest, tmpBuf, utf8Count);
	((utf8_int8_t *)stringDest)[utf8Count] = 0;

	//if(event_->pipe->device->controller->verbose > 3)
		debug_printf( XHCI_LOG_ANSI( "XHCI: String - %s" ), (char *) stringDest);

	uint8_t nextIndex = 0;
	if(event_->arg == device->descriptor.manufacturerIndex) {
		if(device->descriptor.productIndex) nextIndex = device->descriptor.productIndex;
		else if(device->descriptor.serialNumberIndex) nextIndex = device->descriptor.serialNumberIndex;
	} else if(event_->arg == device->descriptor.productIndex) {
		if(device->descriptor.serialNumberIndex) nextIndex = device->descriptor.serialNumberIndex;
	}

	if(nextIndex == 0) {
		auto c = (Controller *) event_->pipe->device->controller;
		auto slotId = event_->pipe->device->slotId;

		auto *tet = c->eventFreelist->alloc();
		tet->callback = &PostNewDeviceCallback;
		tet->pipe = event_->pipe;
		tet->arg = 0;
		tet->dmaBuffer = 0;
		PostEvent( c, slotId, 1, tet );
	} else {
		GetDeviceDescriptionUSBString(device, nextIndex);
	}
}
static void GetDeviceDescriptorStringSizeCallback( USB::Event *event_ ) {
	assert( event_ );
	assert( event_->pipe );
	assert( event_->pipe->type == USB::PipeType::Control);

	// unpack args
	uint8_t const stringIndex = event_->arg;

	// we should have fetched the 1 byte header
	Cache_DCacheInvalidateLine((uintptr_t) event_->dmaBuffer);
	uint8_t const len = *((uint8_t *) event_->dmaBuffer);

	// now fetch the whole thing
	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Standard, Device ),
		.code = USB::RequestCode::GetDescriptor,
		.wValue = (uint16_t) (((uint16_t) USB::DescriptorType::String) << 8 | stringIndex),
		.wIndex = (uint16_t) 0x409, // english-american
		.wLength = len,
	};
	USB::ControlPipe::PostRequest((USB::ControlPipe *) event_->pipe,
	                              &requestData,
	                              FinishDeviceDescriptorStringTransferCallback,
	                              event_->arg );

}
static void HandleGetDescriptor(Controller * c_, uint8_t slotId_) {
	auto c = (Controller *) c_;
	auto slotId = (uint8_t const) slotId_;
	if(c->verbose > 2) debug_print( XHCI_LOG_ANSI( "XHCI: HandleGetDescriptor" ) );

	auto driverSlot = GetDriverSlotFromSlotId( c, slotId );
	auto device = GetDeviceFromSlotId( c, slotId );

	USB::DeviceDescriptor const *deviceDescriptor = &device->descriptor;
	device->packetSize = USB::DeviceDescriptor::GetMaxPacketSize(deviceDescriptor);

	if(deviceDescriptor->manufacturerIndex) GetDeviceDescriptionUSBString( driverSlot->device, deviceDescriptor->manufacturerIndex);
	else if(deviceDescriptor->productIndex)  GetDeviceDescriptionUSBString( driverSlot->device, deviceDescriptor->productIndex);
	else if(deviceDescriptor->serialNumberIndex)  GetDeviceDescriptionUSBString( driverSlot->device, deviceDescriptor->serialNumberIndex);
	else {
		auto newDevice = c->inProgressNewDevice;
		auto *tet = c->eventFreelist->alloc();
		tet->callback = &PostNewDeviceCallback;
		tet->pipe = device->controlPipe;
		tet->arg = (uint64_t) newDevice.func;
		tet->dmaBuffer = 0;
		PostEvent( c, slotId, 1, tet );
	}
}

static void HandleEarlyGetDescriptor( USB::Event *event_ ) {
	auto c = (Controller *) event_->pipe->device->controller;
	auto slotId = (uint8_t const) (event_->arg >> 8);
	auto length = (uint8_t const) (event_->arg & 0xFF);

	// device descriptor come from hardware so let make sure cache is invalidated
	Cache_DCacheInvalidateLine((uintptr_t) event_->dmaBuffer);
	USB::DeviceDescriptor const *deviceDescriptor = (USB::DeviceDescriptor *) event_->dmaBuffer;

	auto driverSlot = GetDriverSlotFromSlotId( c, slotId );
	auto device = GetDeviceFromSlotId( c, slotId );

	if(c->verbose > 2) USB::DeviceDescriptor::Dump( deviceDescriptor, true );

	if(IsRootPortUSB2( c, driverSlot->deviceContext.slotContext.rootPortId ) && length == 8) {
		if(Controller::DoUSB2EnableSlotReset) {
			// TODO only do this on first time not on each hub?
			ResetPort( IsRootPortUSB2( c, driverSlot->deviceContext.slotContext.rootPortId ));
			driverSlot->deviceContext.controlEndPoint.TRBDequeuePointer = driverSlot->deviceBaseAddress[1];
		} else {
			if(c->verbose > 3) debug_print( XHCI_LOG_ANSI( "XHCI: EarlyGetDescriptor Device 8 byte" ) );
			if(c->verbose > 3) USB::DeviceDescriptor::Dump( deviceDescriptor, true );

			MultiCore_LockRecursiveMutex( &c->commandMutex );
			// now send a proper request
			FillInAddressDevice( Controller::GetNextCommandTRB( c, 1 ), slotId, false, (uintptr_t) driverSlot );
			Controller::EnqueueCommandTRB( c );
			MultiCore_UnlockRecursiveMutex( &c->commandMutex );
			driverSlot->deviceContext.controlEndPoint.TRBDequeuePointer = driverSlot->deviceBaseAddress[1];
			dsb();
			HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DB0, 0 );
		}
		return;
	} else {
		if(USB::DeviceDescriptor::GetMaxPacketSize( deviceDescriptor ) != device->packetSize) {
			memcpy( &device->descriptor, deviceDescriptor, sizeof( USB::DeviceDescriptor ));

			// update max packet size with evaluate slot context
			if(c->verbose > 3) debug_print( ANSI_BRIGHT_ON "XHCI: EarlyGetDescriptor Update Max Packet Size" ANSI_BRIGHT_OFF );
			if(c->verbose > 3) USB::DeviceDescriptor::Dump( deviceDescriptor, true );

			assert( USB::DeviceDescriptor::GetMaxPacketSize( deviceDescriptor ) > 0 );
			PostEvaluateContext( c, slotId, USB::DeviceDescriptor::GetMaxPacketSize( deviceDescriptor ));
		} else {
			if(c->verbose > 3) debug_print( ANSI_BRIGHT_ON "XHCI: HandleGetDescriptor Jump" ANSI_BRIGHT_OFF "\n");
			memcpy(&device->descriptor, deviceDescriptor, sizeof(USB::DeviceDescriptor));

			HandleGetDescriptor(c, slotId);
		}
	}
}

static void HandleEnableSlot( USB::Controller *c_, uint8_t slotId_ ) {
	auto c = (XHCI::Controller *) c_;
	if(slotId_ == 0) {
		debug_printf( ANSI_BRIGHT_ON ANSI_BLINK_ON ANSI_RED_PAPER "XHCI: Enable Slots failed" ANSI_BLACK_PAPER ANSI_BLINK_OFF ANSI_BRIGHT_OFF "\n" );
		return;
	}
	auto const newDevice = &c->inProgressNewDevice;

	uint8_t rootPortId = newDevice->parentPortId;
	if(newDevice->parent) {
		rootPortId = GetDriverSlotFromSlotId( c, newDevice->parent->slotId )->deviceContext.slotContext.rootPortId;
	}
	debug_printf( ANSI_BRIGHT_ON ANSI_GREEN_PAPER "XHCI: On root port %hhu slot %hhu is enabled" ANSI_BLACK_PAPER ANSI_BRIGHT_OFF "\n", rootPortId, slotId_ );

	assert( slotId_ > 0 && slotId_ < c->maxSlots + 1U );
	assert( rootPortId == 1 || rootPortId == 2 );

	c->contextBaseAddressArray[slotId_] = &c->hwDeviceContextMem[slotId_ - 1];
	// c->contextBaseAddressArray is accessed by HW flush the table around the change
	Cache_DCacheCleanAndInvalidateLine((uintptr_t) &c->contextBaseAddressArray[slotId_] );

	// this probably isn't necessary as its copied into *by* hardware but just in case
	memset( c->contextBaseAddressArray[slotId_], 0, sizeof( DeviceContext ));
	Cache_DCacheCleanAndInvalidateRange((uintptr_t) c->contextBaseAddressArray[slotId_], sizeof( DeviceContext ));

	DriverSlot *driverSlot = GetDriverSlotFromSlotId( c, slotId_ );
	USB::Device *device = GetDeviceFromSlotId( c, slotId_ );
	memset( device, 0, sizeof( Controller::DeviceRamBlock ));
	driverSlot->device = GetDeviceFromSlotId( c, slotId_ );
	device->controller = c_;
	device->slotId = slotId_;
	device->controlPipe = (USB::ControlPipe *) c->pipeFreelist->alloc();
	USB::ControlPipe::Init( device->controlPipe, device );
	device->controlPipe->postConfigureContext = &PostConfigureContext;

	InputContext *inputCtx = &driverSlot->inputContext;
	SlotContext *slotCtx = &driverSlot->deviceContext.slotContext;
	SlotEndPoint *epc = &driverSlot->deviceContext.controlEndPoint;
	memset( inputCtx, 0, sizeof( InputContext ));
	memset( slotCtx, 0, sizeof( SlotContext ));

	// if first time this driverSlot is used, allocate an event ring
	if(epc->TRBDequeuePointer == nullptr) {
		auto allocator = &c->memory.allocatorFuncs;
		c->memory.current = (uint8_t *) EndAlignAddress((uintptr_t) c->memory.current, 256 * sizeof( TRB::Template ), c->pageSize );
		driverSlot->deviceBaseAddress[1] = (TRB::Template *) allocator->aalloc( allocator, 256 * sizeof( TRB::Template ), 64 );
	}
	Cache_DCacheZeroRange((uintptr_t) driverSlot->deviceBaseAddress[1], 256 * sizeof( TRB::Template ));
	Cache_DCacheCleanAndInvalidateRange((uintptr_t) driverSlot->deviceBaseAddress[1], 256 * sizeof( TRB::Template ));

	epc->TRBDequeuePointer = driverSlot->deviceBaseAddress[1];
	epc->dequeueCycleState = false;
	driverSlot->currentCycleBit[1] = false;
	epc->endpointType = EndpointType::Control;
	epc->averageTRBLength = 8;
	epc->errorCount = 3;

	inputCtx->dropContextFlags = 0;
	inputCtx->addContextFlags = 3;
	slotCtx->contextEntryCount = 1;
	slotCtx->rootPortId = rootPortId;
	slotCtx->interrupterTarget = 0;
	slotCtx->slotState = SlotState::Default;

	if(newDevice->parent != nullptr) {
		DriverSlot *parentDriverSlot = GetDriverSlotFromSlotId( c, newDevice->parent->slotId );
		slotCtx->parentPortNumber = newDevice->parentPortId;
		slotCtx->routeString = (1 << ((slotCtx->parentPortNumber - 1) + (newDevice->parent->hubDepth * 4))) | parentDriverSlot->deviceContext.slotContext.routeString;
		slotCtx->parentHubSlotId = newDevice->parent->slotId;
		slotCtx->speed = newDevice->parent->portSpeed[newDevice->parentPortId];
		if(c->verbose > 2) debug_printf( ANSI_BRIGHT_ON "Slot root string %010x hubDepth %i speed %s\n" ANSI_BRIGHT_OFF, slotCtx->routeString, newDevice->parent->hubDepth, USB::SpeedToString(slotCtx->speed));
	} else {
		// no parent so speed and packet size based on root port
		slotCtx->speed = (rootPortId == 1) ? USB::Speed::HighSpeed : USB::Speed::SuperSpeed;
	}

	switch(slotCtx->speed) {
		case USB::Speed::FullSpeed: epc->maxPacketSize = 64; break;
		case USB::Speed::LowSpeed: epc->maxPacketSize = 8; break;
		case USB::Speed::HighSpeed: epc->maxPacketSize = 64; break;
		case USB::Speed::SuperSpeed: epc->maxPacketSize = 512; break;
		default: assert(false);
	}
	device->parentHub = newDevice->parent;
	device->packetSize = epc->maxPacketSize;

	Cache_DCacheCleanAndInvalidateRange((uintptr_t) driverSlot, sizeof( DriverSlot ));
	MultiCore_LockRecursiveMutex( &c->commandMutex );
	auto ptr = (TRB::AddressDeviceCommand *) Controller::GetNextCommandTRB( c, 1 );
	// for USB2 we do a dance with sending an 8 byte packet and reset first. USB3 don't bother
	FillInAddressDevice((TRB::Template *) ptr, slotId_,
	                    IsRootPortUSB2( c, rootPortId ),
	                    (uintptr_t) driverSlot );
	Controller::EnqueueCommandTRB( c );
	MultiCore_UnlockRecursiveMutex( &c->commandMutex );
	dsb();
	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DB0, 0 );
}

static void TryPopNewDeviceQ( USB::Controller *c_ ) {
	auto c = (XHCI::Controller *) c_;
	if(!c->deviceAddressLock) {
		c->inProgressNewDevice = c->newDeviceQ[0];
		if(c->inProgressNewDevice.func != nullptr) {
			c->deviceAddressLock = true;
			FutureNewSlotId( c_ );
		}
		memcpy( c->newDeviceQ, c->newDeviceQ + 1, sizeof( Controller::TupleNewDevice ) * Controller::MaxNewDevicesInFlight - 1 );
		c->newDeviceQ[Controller::MaxNewDevicesInFlight - 1].func = nullptr;
	}
}

static void NewDevice( USB::Controller *c_,
                       USB::HubDevice *parentHub_,
                       uint8_t parentPortId_,
                       USB::ContinueFunc func_ ) {
	auto c = (XHCI::Controller *) c_;
	uint32_t newDeviceIndex = 0;
	while(newDeviceIndex < Controller::MaxNewDevicesInFlight && c->newDeviceQ[newDeviceIndex].func != nullptr) {
		newDeviceIndex++;
	}
	if(newDeviceIndex >= Controller::MaxNewDevicesInFlight) {
		debug_printf( ANSI_RED_PAPER ANSI_BRIGHT_ON ANSI_BLINK_ON "XHCI: New Device Q overflow!\n" ANSI_BLACK_PAPER ANSI_BRIGHT_OFF ANSI_BLINK_OFF );
		return;
	}
	debug_printf( ANSI_BRIGHT_ON "XHCI: New Device %p parent port %i!\n" ANSI_BRIGHT_OFF, (void *) parentHub_, parentPortId_ );
	c->newDeviceQ[newDeviceIndex] = {func_, parentHub_, parentPortId_};
	TryPopNewDeviceQ( c_ );
}

TRB::Template *Controller::GetNextCommandTRB( Controller *d_, uint8_t maxTDSize_ ) {
	if(d_->commandEnqueuePtr + maxTDSize_ >= (d_->commandRingBase + d_->commandTRBCount - 1)) {
		// add a link back to start of ring
		FillInLink( d_->commandEnqueuePtr, d_->commandRingBase, true );
		d_->commandEnqueuePtr->cycle = d_->commandCycleBit;
		Cache_DCacheCleanLine((uintptr_t)d_->commandEnqueuePtr);

		d_->commandCycleBit = !d_->commandCycleBit;
		d_->commandEnqueuePtr = d_->commandRingBase;
	}
	return d_->commandEnqueuePtr;
}

// trb_ should be a from GetNextCommandTRB()
TRB::Template *Controller::EnqueueCommandTRB( Controller *d_ ) {
	d_->commandEnqueuePtr->cycle = d_->commandCycleBit;
	Cache_DCacheCleanLine((uintptr_t) d_->commandEnqueuePtr );
	return d_->commandEnqueuePtr++;
}

// endpointIndex_ include slot and control slots
TRB::Template *DriverSlot::GetNextEndpointTRB( DriverSlot *driverSlot_, uint8_t endpointIndex_, uint8_t maxTDSize_ ) {
	assert( endpointIndex_ < 32 );
	assert( driverSlot_ );

	SlotEndPoint *xhciEndPoint = &driverSlot_->deviceContext.endPoints[endpointIndex_];
	TRB::Template *currentAddr = xhciEndPoint->TRBDequeuePointer;
	TRB::Template *const baseAddr = driverSlot_->deviceBaseAddress[endpointIndex_];
	assert( xhciEndPoint );
	assert( currentAddr );
	assert( baseAddr );

	// if we would write into the link spot, it means we need to wrap the ring buffer
	if((currentAddr + maxTDSize_) >= (baseAddr + 254)) {
		// add a link back to start of ring
		FillInLink( currentAddr, baseAddr, true );
		xhciEndPoint->TRBDequeuePointer->cycle = driverSlot_->currentCycleBit[endpointIndex_];
		Cache_DCacheCleanLine((uintptr_t) currentAddr );

		// update where will we be submitting from now and cycle bit
		driverSlot_->currentCycleBit[endpointIndex_] ^= true;
		currentAddr = baseAddr;
		xhciEndPoint->TRBDequeuePointer = currentAddr;
	}
	return (TRB::Template *) currentAddr;
}

TRB::Template *DriverSlot::EnqueueEndpointTRB( DriverSlot *driverSlot_, uint8_t endpointIndex_ ) {
	assert( endpointIndex_ < 32 );
	assert( driverSlot_ );
	SlotEndPoint *xhciEndPoint = &driverSlot_->deviceContext.endPoints[endpointIndex_];
	xhciEndPoint->TRBDequeuePointer->cycle = driverSlot_->currentCycleBit[endpointIndex_];
	Cache_DCacheCleanLine((uintptr_t) xhciEndPoint->TRBDequeuePointer );
	return xhciEndPoint->TRBDequeuePointer++;
}


static void PhyReset() {
	// Before Resetting PHY, put Core in Reset
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GCTL, CORESOFTRESET );

	HW_REG_WRITE( USB30_XHCI_BASE_ADDR, USB3_XHCI, GCTL, 0 );

	uint32_t pipeControl3Reg = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUSB3PIPECTL );
	pipeControl3Reg &= ~USB3_XHCI_GUSB3PIPECTL_UX_EXIT_IN_PX;
	pipeControl3Reg &= ~USB3_XHCI_GUSB3PIPECTL_SUSPENDENABLE;
	pipeControl3Reg |= USB3_XHCI_GUSB3PIPECTL_U2SSINACTP3OK;
	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUSB3PIPECTL, pipeControl3Reg );

	uint32_t phyControl2Reg = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUSB2PHYCFG );
	phyControl2Reg &= ~USB3_XHCI_GUSB2PHYCFG_SUSPENDUSB20;
	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUSB2PHYCFG, phyControl2Reg );

	// Assert USB3 PHY reset
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUSB3PIPECTL, PHYSOFTRST );

	// Assert USB2 PHY reset
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUSB2PHYCFG, PHYSOFTRST );

	Utils_BusyMilliSleep( 5 );

	// Clear USB3 PHY reset
	HW_REG_CLR_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUSB3PIPECTL, PHYSOFTRST );

	// Clear USB2 PHY reset
	HW_REG_CLR_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUSB2PHYCFG, PHYSOFTRST );

	Utils_BusyMilliSleep( 5 );

	// Take Core out of reset state after PHYS are stable
	HW_REG_CLR_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GCTL, CORESOFTRESET );
}

Controller *Controller::Init( int verbose_ ) {
	assert( HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, CAPLENGTH, CAPLENGTH ) == USB3_XHCI_USBCMD_OFFSET );
	assert( HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DBOFF ) == USB3_XHCI_DB0_OFFSET );

	int timeout = 2000;

	// abort any running command ring
	HW_REG_WRITE64( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, CRCR_LO, USB3_XHCI_CRCR_LO_CA );

	// reset usb5744 hub
	Gpio::SetOutput( 44, true );
	Gpio::SetDirection( 44, Gpio::Direction::Out );
	Gpio::Write( 44, true );
	Utils_BusyMilliSleep( 10 );
	Gpio::Write( 44, false );
	uint8_t buffer[3] = {0xAA, 0x56, 0x00};
	I2C::Send( 0x2D, buffer, 3 );

	// reset controller
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DCTL, CSFTRST );
	while(HW_REG_GET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DCTL, CSFTRST ) && timeout > 0) {
		Utils_BusyMicroSleep( 50 );
		timeout--;
	}
	if(timeout == 0) {
		debug_print( "XHCI: Reset controller time out\n" );
		return nullptr;
	}
#define DWC3_IP     0x5533
#define DWC31_IP    0x3331
#define DWC32_IP    0x3332
	uint32_t const dwc3VersionReg = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GSNPSID, SYNOPSYSID );
	switch(dwc3VersionReg >> 16) {
		case DWC3_IP: debug_printf( "XHCI: DWC3 revision %#6x\n", dwc3VersionReg & 0xFFFF );
			break;
		case DWC31_IP: debug_printf( "XHCI: DWC31 revision %i\n", dwc3VersionReg & 0xFFFF );
			break;
		case DWC32_IP: debug_printf( "XHCI: DWC32 revision %i\n", dwc3VersionReg & 0xFFFF );
			break;
		default: debug_printf( "XHCI: Unknown DWC3 version %i\n", dwc3VersionReg >> 16 );
			return nullptr;
	}

	HW_REG_CLR_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUCTL1, DEV_L1_EXIT_BY_HW );
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GUCTL, USBHSTINAUTORETRYEN );

	HW_REG_CLR_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GCTL, SCALEDOWN );
	HW_REG_CLR_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GCTL, DISSCRAMBLE );

	switch(HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GHWPARAMS1, USB3_EN_PWROPT )) {
		case USB3_XHCI_GHWPARAMS1_USB3_EN_PWROPT_PWROPT_CLK:
		case USB3_XHCI_GHWPARAMS1_USB3_EN_PWROPT_PWROPT_NO: HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GCTL, DSBLCLKGTNG );
			break;
		case USB3_XHCI_GHWPARAMS1_USB3_EN_PWROPT_PWROPT_HIB: HW_REG_CLR_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GCTL, GBLHIBERNATIONEN );
			// TODO			HW_REG_SET_BIT(HW_REG_GET_ADDRESS(USB30_XHCI), USB3_XHCI, GCTL, GBLHIBERNATIONEN);
			break;
	}

	// reset xhci
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBCMD, HCRST );
	timeout = 2000;
	while(HW_REG_GET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBCMD, HCRST ) && timeout > 0) {
		Utils_BusyMicroSleep( 50 );
		timeout--;
	}

	if(timeout == 0) {
		debug_print( "XHCI: Reset host controller time out\n" );
		return nullptr;
	}

	PhyReset();

	// start with the controller on stack and then move into alignment slack space when ready
	Controller stackDevice = {};
	auto controller = &stackDevice;
	controller->verbose = verbose_;
	controller->maxSlots = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS1, MAXSLOTS );
	controller->maxPorts = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS1, MAXPORTS );
	controller->pageSize = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PAGESIZE, PAGE_SIZE ) << 12;
	controller->maxEventRingSegmentTableSize = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS2, ERSTMAX );
	if(Controller::EventRingSegmentTableSize < controller->maxEventRingSegmentTableSize)
		controller->maxEventRingSegmentTableSize = Controller::EventRingSegmentTableSize;
	controller->commandTRBCount = (64 * 1024) / sizeof( TRB::Template );

	assert( controller->maxSlots == 64 ); // zynqmp supports 64 devices
	assert( controller->maxPorts == 2 ); // root hub on zynqmp has 1 USB2 and 1 USB 3 (in that order) ports

	// allocate direct memory from the Os for the physical pages needed for the XHCI hardware
	controller->memoryBlockSize = 16;  // in 64KB blocks
#if CPU_a53
	// TODO Can't use hi memory yet as may overwrite our self :(
//	controller->memoryBlock = (void*) OsService_DdrHiBlockAlloc(controller->memoryBlockSize);
	controller->memoryBlock = (void *) (uintptr_all_t) OsService_DdrLoBlockAlloc( controller->memoryBlockSize, OS_SERVICE_TAG('U', 'S', 'B', '3') );
#else
	controller->memoryBlock = (void*) OsService_DdrLoBlockAlloc(controller->memoryBlockSize);
#endif
	memset( controller->memoryBlock, 0, controller->memoryBlockSize * 64 * 1024 );

	// no tracking should be used for these physical pages
	memcpy( &controller->memory, &Memory_LinearAllocatorTEMPLATE, sizeof( Memory_LinearAllocator ));

	Memory_LinearAllocator *linearAllocatorInternal = &controller->memory;
	linearAllocatorInternal->bufferStart = controller->memoryBlock;
	linearAllocatorInternal->current = (uint8_t *) controller->memory.bufferStart;
	linearAllocatorInternal->bufferEnd = (void *) (controller->memory.current + (controller->memoryBlockSize * 64 * 1024));

	// relocate usb controller structure off stack, need to repoint internal allocator
	controller = (Controller *) linearAllocatorInternal->allocatorFuncs.malloc( &linearAllocatorInternal->allocatorFuncs, sizeof( Controller ));
	memcpy( controller, &stackDevice, sizeof( Controller ));
	linearAllocatorInternal = &controller->memory;
	Memory_Allocator *linearAllocator = &controller->memory.allocatorFuncs;

	// allocate pointer to upto maxSlots+1 devices (64 on zynqmp)
	controller->contextBaseAddressArray = (DeviceContext **) linearAllocator->aalloc( linearAllocator, (controller->maxSlots + 1) * sizeof( DeviceContext * ), 64 );
	memset( controller->contextBaseAddressArray, 0, (controller->maxSlots + 1) * sizeof( DeviceContext * ));

	if(controller->verbose > 1)
		debug_printf( XHCI_LOG_ANSI( "XHCI: ContextBaseAddressArray %#018lx" ), (uintptr_all_t) controller->contextBaseAddressArray );

	// allocate scratchpads
	uint32_t const maxScratchPadBuffs = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS2, MAXSCRATCHPADBUFS ) |
	                                    (HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, HCSPARAMS2, MAXSCRATCHPADBUFS_HI ) << 5);

	controller->scratchPadArray = (uintptr_all_t *) linearAllocator->aalloc( linearAllocator, sizeof( uintptr_all_t ) * maxScratchPadBuffs, 64 );
	memset( controller->scratchPadArray, 0, sizeof( uintptr_all_t ) * maxScratchPadBuffs );

	for(uint32_t i = 0; i < maxScratchPadBuffs; ++i) {
		controller->scratchPadArray[i] = (uintptr_all_t) linearAllocator->aalloc( linearAllocator, controller->pageSize, controller->pageSize );
	}

	// allocate ERSTs
	controller->primaryERST = (ERST *) linearAllocator->aalloc( linearAllocator, Controller::EventRingSegmentTableSize * sizeof( ERST ), controller->pageSize );


	// allocate free lists
	controller->buffer512Freelist = Cadt::FreeList<uint8_t[Controller::DMA_BUFFERSIZE]>::Create( Controller::MAX_DMA_BUFFERS_COUNT, linearAllocator );
	controller->eventFreelist = Cadt::FreeList<USB::Event>::Create( Controller::MAX_EVENT_COUNT, linearAllocator );
	controller->pipeFreelist = Cadt::FreeList<uint8_t[USB::Controller::MAX_PIPE_SIZE]>::Create( Controller::MAX_PIPE_COUNT, linearAllocator );

	// allocator drivers slots and devices up front (could be reduced to save memory)
	controller->driverSlots = (DriverSlot *) linearAllocator->aalloc( linearAllocator, controller->maxSlots * sizeof( DriverSlot ), 64 );
	memset( controller->driverSlots, 0, controller->maxSlots * sizeof( DriverSlot ));
	controller->deviceSlots = (DeviceRamBlock *) linearAllocator->aalloc( linearAllocator, controller->maxSlots * sizeof( DeviceRamBlock ), sizeof( DeviceRamBlock ) );
	memset( controller->deviceSlots, 0, controller->maxSlots * sizeof( DeviceRamBlock ));

	// allocate the space for all controller context
	linearAllocatorInternal->current = (uint8_t *) EndAlignAddress((uintptr_t) linearAllocatorInternal->current, controller->maxSlots * sizeof( DeviceContext ), controller->pageSize );
	controller->hwDeviceContextMem = (DeviceContext *) linearAllocator->aalloc( linearAllocator, controller->maxSlots * sizeof( DeviceContext ), 64 );
	memset( controller->hwDeviceContextMem, 0, controller->maxSlots * sizeof( DeviceContext ));

	// align the trb bases to 64K segment
	uint32_t spaceSlackMem = (uint8_t *) Core::alignTo((uintptr_t) linearAllocatorInternal->current, 64 * 1024 ) - controller->memory.current;
	linearAllocatorInternal->current = (uint8_t *) Core::alignTo((uintptr_t) linearAllocatorInternal->current, 64 * 1024 );
	if(controller->verbose > 1)
		debug_printf( XHCI_LOG_ANSI( "XHCI: %u slack in initial 64K segment" ), spaceSlackMem );

	// allocate primary event ring and segmentsf
	for(uint32_t i = 0; i < Controller::EventRingSegmentTableSize; ++i) {
		controller->primarySegments[i] = (TRB::Template *) linearAllocator->aalloc( linearAllocator, Controller::EventSegmentTRBSize * sizeof( TRB::Template ), 64 );

		memset( controller->primarySegments[i], 0, Controller::EventSegmentTRBSize * sizeof( TRB::Template ));
		Cache_DCacheCleanAndInvalidateRange((uintptr_t) controller->primarySegments[i], Controller::EventSegmentTRBSize * sizeof( TRB::Template ));
		controller->primaryERST[i].ringSegmentBaseAddress = (uint64_t) controller->primarySegments[i];
		controller->primaryERST[i].ringSegmentSize = Controller::EventSegmentTRBSize;
		if(controller->verbose > 1)
			debug_printf( XHCI_LOG_ANSI( "XHCI: Event ring %i: BA %#lx size %i TRBs" ), i, controller->primaryERST[i].ringSegmentBaseAddress, controller->primaryERST[i].ringSegmentSize );
	}
	controller->primaryERSTWrapAddress = (controller->primarySegments[Controller::EventRingSegmentTableSize - 1] + Controller::EventSegmentTRBSize - 1);

	// allocate cmd trb and set it up
	controller->commandRingBase = (TRB::Template *) linearAllocator->aalloc( linearAllocator, controller->commandTRBCount * sizeof( TRB::Template ), 64 );
	memset( controller->commandRingBase, 0x0, controller->commandTRBCount * sizeof( TRB::Template ));

	// fill in xhci context base addresses
	controller->contextBaseAddressArray[0] = (DeviceContext *) controller->scratchPadArray; // first context is actually a scratch pad

	controller->currentPrimaryEventPtr = (TRB::Template *) controller->primaryERST->ringSegmentBaseAddress;
	controller->primaryEventCycleBit = true;

	controller->commandEnqueuePtr = controller->commandRingBase;
	controller->commandCycleBit = true;

	// fill in functions for usb controller to call in this controller
	controller->postControlRequest = &PostControlRequest;
	controller->postNormalTransfer = &PostNormalTransfer;
	controller->postEvent = &PostEvent;

	controller->newDeviceFunc = &NewDevice;

	// we have to always support hub class devices
	controller->classHandlerTable[(uint8_t) USB::ClassCode::Hub] = &USB::HubVTable;
	controller->classHandlerTable[(uint8_t) USB::ClassCode::HID] = &USB::HIDVTable;

	// flush all the cache to ensure everything has been written out to memory for Hardware
	Cache_DCacheCleanAndInvalidate();

	//--------------
	// start the register programming
	//--------------

	// check we are halted
	assert( HW_REG_GET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS, HCH ));

	// make sure we own it and not some bios/os
	timeout = 2000;
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBLEGSUP, HC_OS_OWNED );
	while(!HW_REG_GET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBLEGSUP, HC_OS_OWNED ) && timeout > 0) {
		Utils_BusyMilliSleep( 1 );
		timeout--;
	}
	if(timeout == 0) {
		debug_print( ANSI_RED_PEN "XHCI usb legacy time out" ANSI_WHITE_PEN "\n" );
		return nullptr;
	}

	// set cache coherant USB
	//HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_REGS ), USB3_REGS, COHERENCY, USB3_REGS_COHERENCY_USB );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_REGS ), USB3_REGS, COHERENCY, 0 );

	// enable error interrupts
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_REGS ), USB3_REGS, IR_ENABLE, HOST_SYS_ERR );
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_REGS ), USB3_REGS, IR_ENABLE, ADDR_DEC_ERR );

	// Host mode
	HW_REG_SET_ENUM( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GCTL, PRTCAPDIR, HOST );
	HW_REG_SET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GSBUSCFG0, DATRDREQINFO, 0xF );
	HW_REG_SET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GSBUSCFG0, DESRDREQINFO, 0xF );
	HW_REG_SET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GSBUSCFG0, DATWRREQINFO, 0xF );
	HW_REG_SET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, GSBUSCFG0, DESWRREQINFO, 0xF );

	HW_REG_WRITE64( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DCBAAP_LO, (uintptr_all_t) controller->contextBaseAddressArray );
	HW_REG_WRITE64( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, CRCR_LO, ((uintptr_all_t) controller->commandRingBase) | USB3_XHCI_CRCR_LO_RCS );
	HW_REG_SET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, CONFIG, MAXSLOTSEN, controller->maxSlots );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DNCTRL, 0xFF );

	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, IMAN_0,
	              HW_REG_FIELD_MASK( USB3_XHCI, IMAN_0, IP ) |
	              HW_REG_FIELD_MASK( USB3_XHCI, IMAN_0, IE ));
	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, IMOD_0, 0 );
	HW_REG_SET_FIELD( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, ERSTSZ_0, ERS_TABLE_SIZE, Controller::EventRingSegmentTableSize );
	HW_REG_WRITE64( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, ERDP_LO_0, ((uintptr_all_t) controller->primaryERST->ringSegmentBaseAddress) | USB3_XHCI_ERDP_LO_0_EHB );
	HW_REG_WRITE64( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, ERSTBA_LO_0, (uintptr_all_t) controller->primaryERST );

	HW_REG_WRITE( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS, HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS ));

	return controller;
}

void Controller::Start( Controller *controller_ ) {
	if(controller_->verbose) debug_printf( XHCI_LOG_ANSI( "XHCI::Start - %#018lx" ), (uintptr_t) controller_ );

	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, DCTL, RUN_STOP );

	HW_REG_RMW( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBCMD,
	            HW_REG_FIELD_MASK( USB3_XHCI, USBCMD, R_S ) |
	            HW_REG_FIELD_MASK( USB3_XHCI, USBCMD, INTE ) |
	            HW_REG_FIELD_MASK( USB3_XHCI, USBCMD, HSEE ),

	            HW_REG_ENCODE_FIELD( USB3_XHCI, USBCMD, R_S, 1 ) |
	            HW_REG_ENCODE_FIELD( USB3_XHCI, USBCMD, INTE, 1 ) |
	            HW_REG_ENCODE_FIELD( USB3_XHCI, USBCMD, HSEE, 1 ));
	Utils_BusyMilliSleep( 100 );

	int timeout = 2000;
	while(HW_REG_GET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS, HCH ) && timeout > 0) {
		Utils_BusyMilliSleep( 1 );
		timeout--;
	}
	if(timeout == 0) {
		debug_print( "XHCI HCH time out\n" );
		return;
	}

	timeout = 2000;
	while(HW_REG_GET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS, CNR ) && timeout > 0) {
		Utils_BusyMilliSleep( 1 );
		timeout--;
	}
	if(timeout == 0) {
		debug_print( ANSI_RED_PEN "XHCI Running time out" ANSI_WHITE_PEN "\n" );
		return;
	}
	if(controller_->verbose > 1) debug_print( XHCI_LOG_ANSI( "XHCI: Start completed" ) );
	if(controller_->verbose > 1) debug_printf( XHCI_LOG_ANSI( "XHCI: Memory Unsed from chunk %li KiB" ), (((uint8_t *)controller_->memory.bufferEnd) - controller_->memory.current)/1024 );
}

static void HandlePortStatusChangeNewDevice( USB::Controller *controller_, uint64_t arg0_, uint64_t arg1_ ) {
	auto device = (USB::Device *) arg0_;
	assert(device != nullptr);
	auto c = (Controller *) device->controller;
	//uint8_t const portId = arg1_;

	auto const deviceDescriptor = &device->descriptor;
	auto const deviceClass = deviceDescriptor->deviceClass;
	if( c->classHandlerTable[(uint8_t) deviceClass] ) {
		debug_printf(ANSI_YELLOW_PEN ANSI_BRIGHT_ON "%s usb class Enumerate" ANSI_RESET_ATTRIBUTES "\n", USB::ClassCodeToString(deviceClass));
		auto vtable = c->classHandlerTable[(uint8_t)deviceClass];
		vtable->enumerate( device);
	} else {
		debug_printf(ANSI_YELLOW_PEN ANSI_BRIGHT_ON "%s usb class driver (PSC) not available, ignoring device" ANSI_RESET_ATTRIBUTES "\n", USB::ClassCodeToString(deviceClass));
	}
}

static void HandleConfigureEndpoint( USB::Controller *controller_, uint64_t arg0_, uint64_t arg1_ ) {
	auto event = (USB::Event *) arg0_;
	assert( event );
	assert( event->pipe );
	assert( event->pipe->device );

	auto device = event->pipe->device;
	device->vtable->interfacesEnabled(device);
}


static void HandleCommandCompletionEvent( Controller *c_, TRB::Template const *trbTemplate_ ) {
	auto const *cce = (TRB::CommandCompletionEvent *) trbTemplate_;
	assert( cce->type == TRB::Type::CommandCompletionEvent );
	auto const commandTRB = (TRB::Template const *const) cce->commandTRBPointer;
	if(c_->verbose > 3) debug_print( __PRETTY_FUNCTION__ );

	if((USB::CompletionCode) cce->completionCode != USB::CompletionCode::Success) {
		debug_printf( ANSI_RED_PAPER ANSI_BRIGHT_ON "Completion Code %s\n", USB::CompletionCodeToString((USB::CompletionCode) cce->completionCode ));
		if(cce->slotId > 0) {
			debug_printf( "id %i Slot state %s\n", cce->slotId, SlotStateToString( c_->contextBaseAddressArray[cce->slotId]->slotContext.slotState ));
		}
		debug_print( ANSI_YELLOW_PAPER ANSI_BRIGHT_ON );
		debug_print( "FROM: " );
		TRB::Template::Dump((TRB::Template const *) commandTRB );
		debug_print( ANSI_RESET_ATTRIBUTES );
		return;
	}

	if(cce->slotId != 0) {
		switch((TRB::Type) commandTRB->type) {
			case TRB::Type::EnableSlotCommand: {
				if(c_->verbose > 3) debug_print( " TRB::Type::EnableSlotCommand\n" );
				HandleEnableSlot( c_, cce->slotId );
				return;
			}
			case TRB::Type::AddressDeviceCommand: {
				if(c_->verbose > 3) debug_print( " TRB::Type::AddressDeviceCommand\n" );
				assert( c_->contextBaseAddressArray[cce->slotId]->slotContext.slotState != SlotState::Disabled );
				auto command = (TRB::AddressDeviceCommand *) cce->commandTRBPointer;
				USB::RequestData requestData = {
					.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Standard, Device ),
					.code = USB::RequestCode::GetDescriptor,
					.wValue = (uint16_t) ((((uint16_t) USB::DescriptorType::Device) << 8) | 0),
					.wIndex = (uint8_t) 0,
					.wLength = (uint16_t) (command->blockSetAddressRequest ? 8 : 18),
				};
				auto device = GetDeviceFromSlotId(c_, cce->slotId);
				auto tet = c_->eventFreelist->alloc();
				tet->callback = &HandleEarlyGetDescriptor;
				tet->pipe = device->controlPipe;
				tet->arg = (uint64_t) cce->slotId << 8 | requestData.wLength;
				tet->dmaBuffer = 0;
				c_->postControlRequest( c_, cce->slotId, &requestData, tet );
				break;
			}
			case TRB::Type::ConfigureEndpointCommand: {
				if(c_->verbose > 3) debug_print( " TRB::Type::ConfigureEndpointCommand\n" );
				auto driverSlot = GetDriverSlotFromSlotId( c_, cce->slotId );
				auto tet = c_->eventFreelist->alloc();
				tet->callback = &XHCISyncedCallback;
				tet->pipe = driverSlot->device->controlPipe;
				tet->dmaBuffer = (uintptr_all_t) c_->buffer512Freelist->alloc();
				assert( tet->dmaBuffer );
				tet->arg = (uint64_t) &HandleConfigureEndpoint;
				TRB::FillInEventData( DriverSlot::GetNextEndpointTRB( driverSlot, 1, 1 ), (uint64_t) tet );
				DriverSlot::EnqueueEndpointTRB( driverSlot, 1 );
				hw_RegWrite( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI_DB0_OFFSET + (cce->slotId * 4), 1 );
				break;
			}
			case TRB::Type::EvaluateContextCommand: {
				if(c_->verbose > 3) debug_print( " TRB::Type::EvaluateContextCommand\n" );
				HandleGetDescriptor(c_, cce->slotId);
				break;
			}
			default: break;
		}
	}
}


static void HandlePortStatusChangeEvent( Controller *c_, TRB::Template const *trbTemplate_ ) {
	auto const *psce = (TRB::PortStatusChangeEvent *) trbTemplate_;
	assert( psce->type == TRB::Type::PortStatusChangeEvent );

	// for usb2 we have to reset the port manually and what for the next port status change event
	if(IsRootPortUSB2( c_, psce->portId )) {
		uint32_t const portsc = *((uint32_t *) (USB30_XHCI_BASE_ADDR + USB3_XHCI_PORTSC_20_OFFSET));
		if(HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PED, portsc ) == false &&
		   HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PR, portsc ) == false &&
		   HW_REG_DECODE_BIT( USB3_XHCI, PORTSC_20, PLS, portsc ) == true) {
			HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, PORTSC_20, PR );
			return;
		}
	}
	NewDevice( c_, nullptr, psce->portId, &HandlePortStatusChangeNewDevice );
}

static void HandleTransferEvent( Controller *controller_, TRB::Template const *trbTemplate_ ) {
	auto const *te = (TRB::TransferEvent *) trbTemplate_;
	assert( te->type == TRB::Type::TransferEvent );

	if(te->completionCode != USB::CompletionCode::Success &&
	   te->completionCode != USB::CompletionCode::ShortPacket) {
		// TODO we need to reset the endpoint if we get a stall, some devices need this during get strings phase

		debug_printf( ANSI_BRIGHT_ON ANSI_BLINK_ON ANSI_RED_PEN "XHCI: TransferEvent error" ANSI_RESET_ATTRIBUTES "\n" );
		TRB::Template::Dump( trbTemplate_ );
		debug_printf( ANSI_BRIGHT_ON ANSI_BLUE_PEN "control EP state %hhu" ANSI_RESET_ATTRIBUTES "\n",
		              (uint8_t) GetDriverSlotFromSlotId( controller_, te->slotId )->deviceContext.controlEndPoint.EPState );
		return;
	}

	if(te->eventDataTransferEvent) {
		auto event = (USB::Event *) te->parameter;
		assert( event );
		assert( event->callback );
		event->callback( event );
		if(event->dmaBuffer) controller_->buffer512Freelist->release((Controller::DmaBuffer *) event->dmaBuffer );
		controller_->eventFreelist->release( event );
		return;
	}
}

void Interrupter0BulkHandler( Controller *controller_ ) {
	uint32_t const usbsts = HW_REG_READ( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS );

	if(HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, HSE, usbsts )) {
		debug_print( ANSI_RED_PEN "XHCI: Host System Error\n" ANSI_RESET_ATTRIBUTES );
		DumpUSBSTSBits( controller_ );
		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS, EINT );
		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, IMAN_0, IP );
		return;
	}
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, USBSTS, EINT );
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, IMAN_0, IP );
	// read from saved usbsts not the actual register
	if(HW_REG_DECODE_BIT( USB3_XHCI, USBSTS, EINT, usbsts )) {
		TRB::Template *trb = controller_->currentPrimaryEventPtr;
		// we never write to this array so, we can safely invalidate the whole cache line
		Cache_DCacheInvalidateLine(((uintptr_all_t) trb) & ~0x3FU );
		while(trb->cycle == controller_->primaryEventCycleBit) {
			Cache_DCacheInvalidateLine(((uintptr_all_t) trb) & ~0x3FU );

			if(controller_->verbose > 3) TRB::Template::Dump( trb );

			switch((TRB::Type) trb->type) {
				case TRB::Type::CommandCompletionEvent: HandleCommandCompletionEvent( controller_, trb );
					break;
				case TRB::Type::PortStatusChangeEvent: HandlePortStatusChangeEvent( controller_, trb );
					break;
				case TRB::Type::TransferEvent: HandleTransferEvent( controller_, trb );
					break;
				default: debug_printf( ANSI_BRIGHT_ON ANSI_BLINK_ON ANSI_RED_PEN "XHCI: Unhandled TRB event" ANSI_RESET_ATTRIBUTES "\n" );
					TRB::Template::Dump( trb );
					break;
			}

			if(controller_->currentPrimaryEventPtr + 1 > controller_->primaryERSTWrapAddress) {
				controller_->currentPrimaryEventPtr = controller_->primarySegments[0];
				controller_->primaryEventCycleBit ^= true;
			} else {
				controller_->currentPrimaryEventPtr = controller_->currentPrimaryEventPtr + 1;
			}
			trb = controller_->currentPrimaryEventPtr;
		}
	}
	HW_REG_WRITE64( HW_REG_GET_ADDRESS( USB30_XHCI ), USB3_XHCI, ERDP_LO_0, ((uintptr_all_t) controller_->currentPrimaryEventPtr) | USB3_XHCI_ERDP_LO_0_EHB );
}

void Interrupter0IsochronousHandler( Controller *controller_ ) {
	debug_print( "XHCI Interrupter 0 Isochronous\n" );
}

void Interrupter0ControllerHandler( Controller *controller_ ) {
	debug_print( "XHCI Interrupter 0 Controller\n" );
}

void Interrupter0ControlHandler( Controller *controller_ ) {
	debug_print( "XHCI Interrupter 0 Control\n" );
}

void Interrupter0OTGHandler( Controller *controller_ ) {
	debug_print( "XHCI Interrupter 0 OTG\n" );
}

void Controller::Fini( Controller *controller_ ) {
#if CPU_a53
	// TODO Can't use hi memory yet as may overwrite our self :(
	//OsService_DdrHiBlockFree((uintptr_all_t)controller_ ->memoryBlock, controller_ ->memoryBlockSize);
	OsService_DdrLoBlockFree((uintptr_lo_t) (uintptr_all_t) controller_->memoryBlock, controller_->memoryBlockSize, OS_SERVICE_TAG('U', 'S', 'B', '3') );
#else
	OsService_DdrLoBlockFree((uintptr_lo_t)controller_ ->memoryBlock, controller_ ->memoryBlockSize);
#endif
}

} // end namespace