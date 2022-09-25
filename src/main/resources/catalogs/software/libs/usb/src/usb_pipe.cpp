#include "core/core.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "platform/cache.h"
#include "usb/usb.hpp"
#include "usb/usb_device.hpp"
#include "usb/usb_pipe.hpp"

namespace USB {


static void FinishConfigurationDescriptorCallback( USB::Event *event_ ) {
	assert(event_);
	auto c = (Controller *) event_->pipe->device->controller;
	auto device = (Device *) event_->pipe->device;

	Cache_DCacheInvalidateLine((uintptr_t)event_->dmaBuffer);
	auto const configurationDescriptor = (USB::ConfigurationDescriptor *) event_->dmaBuffer;
	if(configurationDescriptor->totalLength > 64) Cache_DCacheInvalidateRange(((uintptr_t)event_->dmaBuffer)+64, (configurationDescriptor->totalLength - 64 + 63) & 63);

	auto const deviceDescriptor = &device->descriptor;
	auto const deviceClass = deviceDescriptor->deviceClass;
	if(c->verbose > 2) USB::ConfigurationDescriptor::Dump( configurationDescriptor );

	if( !c->classHandlerTable[(uint8_t) deviceClass] && deviceClass != ClassCode::UseInterfaceClass ) {
		debug_printf(ANSI_YELLOW_PEN ANSI_BRIGHT_ON "%s usb class driver (FCDC) not available, ignoring device" ANSI_RESET_ATTRIBUTES "\n", USB::ClassCodeToString(deviceClass));
		return;
	}
	assert(configurationDescriptor->totalLength <= event_->arg);
	auto config = (ConfigurationDescriptor *) event_->dmaBuffer;
	auto interface = (InterfaceDescriptor *) (config + 1);

	if(deviceClass == ClassCode::UseInterfaceClass) {
		// search interfaces for a class we can handle
		for(int i = 0; i < config->numInterfaces;++i) {
			if(c->verbose > 2) InterfaceDescriptor::Dump( interface );
			auto vtable = c->classHandlerTable[(uint8_t)interface->classCode];
			if(vtable != nullptr) {
				debug_printf( ANSI_YELLOW_PEN ANSI_BRIGHT_ON "%s usb class ConfigurationDescriptorUpdate" ANSI_RESET_ATTRIBUTES "\n", USB::ClassCodeToString( interface->classCode ));
				device->vtable = vtable;
				vtable->configurationDescriptorReady( device, (ConfigurationDescriptor *) event_->dmaBuffer, interface );
				return;
			}
			debug_printf(ANSI_YELLOW_PEN ANSI_BRIGHT_ON "Use Interface Class %s ConfigurationDescriptorUpdate not available" ANSI_RESET_ATTRIBUTES "\n", USB::ClassCodeToString(interface->classCode));

			auto endpoints = (EndpointDescriptor *) (interface + 1);
			interface = (InterfaceDescriptor *) (endpoints + interface->numEndpoints);
		}
		debug_printf(ANSI_YELLOW_PEN ANSI_BRIGHT_ON "UseInterfaceClass no class driver available, ignoring device" ANSI_RESET_ATTRIBUTES "\n");
	} else {
		debug_printf( ANSI_YELLOW_PEN ANSI_BRIGHT_ON "%s usb class ConfigurationDescriptorUpdate" ANSI_RESET_ATTRIBUTES "\n", USB::ClassCodeToString( deviceClass ));
		auto vtable = c->classHandlerTable[(uint8_t)deviceClass];
		device->vtable = vtable;
		vtable->configurationDescriptorReady( device, (ConfigurationDescriptor *) event_->dmaBuffer, interface );
	}
}


static void PostGetConfigurationAfterSet( USB::Event *event_ ) {
	assert(event_);
	if(event_->pipe->device->controller->verbose > 2) debug_printf("PostGetConfigurationAfterSet %lu\n", event_->arg);

	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Standard, Device ),
		.code = USB::RequestCode::GetDescriptor,
		.wValue = (uint16_t) ((((uint16_t) USB::DescriptorType::Configuration) << 8) | 0),
		.wIndex = (uint16_t) 0,
		.wLength = (uint16_t) event_->arg,
	};

	ControlPipe::PostRequest((ControlPipe *) event_->pipe,
													 &requestData,
													 FinishConfigurationDescriptorCallback,
													 event_->arg );
}

static void PostSetConfiguration( USB::Event *event_ ) {
	assert(event_);
	auto device = (Device *) event_->pipe->device;

	Cache_DCacheInvalidateLine((uintptr_t)event_->dmaBuffer);
	auto const configurationDescriptor = (USB::ConfigurationDescriptor *) event_->dmaBuffer;
	device->configuration = configurationDescriptor->configVal;

	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( HostToDevice, Standard, Device ),
		.code = USB::RequestCode::SetConfiguration,
		.wValue = device->configuration,
		.wIndex = 0,
		.wLength = 0,
	};
	ControlPipe::PostRequest( device->controlPipe,
	                          &requestData,
	                          PostGetConfigurationAfterSet,
	                          configurationDescriptor->totalLength );

}

static void GetConfigurationDescriptor(USB::ControlPipe * controlPipe_) {
	if(controlPipe_->device->controller->verbose > 2) debug_printf("GetConfigurationDescriptor for %#06x:%#06x\n", controlPipe_->device->descriptor.vid, controlPipe_->device->descriptor.pid);

	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Standard, Device ),
		.code = USB::RequestCode::GetDescriptor,
		.wValue = (uint16_t) ((((uint16_t) USB::DescriptorType::Configuration) << 8) | 0),
		.wIndex = (uint16_t) 0,
		.wLength = 9,
	};
	ControlPipe::PostRequest( controlPipe_,
														&requestData,
														PostSetConfiguration,
														0 );
}

void Pipe::Init( Pipe * pipe_, Device *device_ ) {
	pipe_->device = device_;
}

void ControlPipe::Init( ControlPipe *pipe_, Device *device_ ) {
	memset(pipe_, 0, sizeof(ControlPipe));
	Pipe::Init(pipe_, device_);
	pipe_->type = PipeType::Control;
	pipe_->postGetConfigurationDescriptor = &GetConfigurationDescriptor;
}

void ControlPipe::PostRequest( ControlPipe * pipe_, RequestData const *requestData_, EventCallbackFunc callback_, uint64_t callbackArg_ ) {
	auto * controller = pipe_->device->controller;
	USB::Event * event = nullptr;
	if(callback_ != nullptr) {
		event = controller->eventFreelist->alloc();
		assert( event );
		event->callback = callback_;
		event->arg = callbackArg_;
		event->pipe = pipe_;
		event->dmaBuffer = 0;
	}
	controller->postControlRequest(controller, pipe_->device->slotId, requestData_, event);
}

void ControlPipe::SyncCallback( ControlPipe * pipe_, EventCallbackFunc callback_, uint64_t callbackArg_) {
	assert(callback_)
	auto * controller = pipe_->device->controller;

	USB::Event * event = controller->eventFreelist->alloc();
	assert( event );
	event->callback = callback_;
	event->arg = callbackArg_;
	event->pipe = pipe_;
	event->dmaBuffer = 0;
	controller->postEvent(controller, pipe_->device->slotId, 1, event);
}

void InterruptPipe::Init( InterruptPipe * pipe_, Device * device_, uint8_t endpointId_ ) {
	memset(pipe_, 0, sizeof(InterruptPipe));
	Pipe::Init(pipe_, device_);
	pipe_->type = PipeType::Interrupt;
	pipe_->endpointId = endpointId_;
}

void InterruptPipe::AddInInterrupt( InterruptPipe * pipe_, uint8_t size_, uintptr_all_t dest_, EventCallbackFunc callback_, uint64_t callbackArg_ ) {
	auto * controller = pipe_->device->controller;
	USB::Event * event = nullptr;
	if(callback_ != nullptr) {
		event = controller->eventFreelist->alloc();
		assert( event );
		event->callback = callback_;
		event->arg = callbackArg_;
		event->pipe = pipe_;
		event->dmaBuffer = 0;
	}
	controller->postNormalTransfer(controller, pipe_->device->slotId, pipe_->endpointId, size_, dest_, event);
}

} // USB