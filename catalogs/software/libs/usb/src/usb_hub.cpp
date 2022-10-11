#include "core/core.h"
#include "usb/usb_hub.hpp"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "dbg/assert.h"
#include "utils/busy_sleep.h"
#include "platform/cache.h"
#include "usb/usb_pipe.hpp"
#include "core/utils.hpp"
#include "core/math.h"

namespace USB {
#define HUBPORT_FLAG( s, w, x ) !!(s[w] & HubPortStatus##x)

static void PostSetDepth( HubDevice *device_) {
	USB::RequestData depthRequestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( HostToDevice, Class, Device ),
		.code = (USB::RequestCode) 0x0C,
		.wValue = (uint16_t) device_->hubDepth,
		.wIndex = 0,
		.wLength = 0,
	};
	device_->controller->postControlRequest( device_->controller, device_->slotId, &depthRequestData, nullptr );
}

static void PostGetHubStatus( HubDevice * hub_ );
static void PostGetPortStatus( HubDevice * hub_, uint16_t port_ );

static void HubGetHutStatusDone( Event *event_ ) {
	auto hub = (HubDevice *)event_->pipe->device;
	Cache_DCacheInvalidateLine(hub->statusCacheLineAddr);
	uint32_t status = *((uint8_t *)hub->statusCacheLineAddr);
	PostGetHubStatus(hub);

	if(status & 0x1) {
		debug_printf("Hub has changed\n");
	}

	for(int i = 1; i < hub->numPorts + 1;++i) {
		bool portChanged = status & (1 << i);
		if(portChanged) {
			PostGetPortStatus(hub, i );
		}
	}

}

static void PostGetHubStatus( HubDevice * hub_ ) {
	Cache_DCacheZeroLine(hub_->statusCacheLineAddr);
	Cache_DCacheCleanAndInvalidateLine(hub_->statusCacheLineAddr);

	InterruptPipe::AddInInterrupt(&hub_->statusPipe, 1, hub_->statusCacheLineAddr, &HubGetHutStatusDone, 0);
}

static void AfterNewDevice( struct Controller * controller_, uint64_t arg0_, uint64_t arg1_) {
	auto device = (Device *) arg0_;
	//uint8_t const portId = arg1_;

	assert(device != nullptr);
	assert(device->controlPipe);
	assert(device->controlPipe->postGetConfigurationDescriptor);

	auto hub = (HubDevice *) device->parentHub;
	assert(hub != nullptr);

	hub->currentlyResettingPort = false;
	device->controlPipe->postGetConfigurationDescriptor(device->controlPipe );
}

static void HubGetPortStatusDone( Event *event_ ) {
	assert( event_->dmaBuffer );

	auto hubPortStatus = (uint16_t *) event_->dmaBuffer;
	auto hub = (HubDevice *) event_->pipe->device;

	// if nothing has changed, nothing to do.
	if(hubPortStatus[1] == 0) return;
	// don't do anything whilst we are setting up a port
	if(hub->currentlyResettingPort) return;

	auto controlPort = (ControlPipe *)event_->pipe;
	auto portId = (uint16_t) event_->arg;

	debug_printf( ANSI_BRIGHT_ON "%i-%i " ANSI_BRIGHT_OFF, event_->pipe->device->slotId, (uint8_t) event_->arg );
	HubPortStatusDump( hubPortStatus, event_->pipe->device->descriptor.bcdUSBVersion >= 0x300, true );

	if(HUBPORT_FLAG( hubPortStatus, 1, Connect )) {
		USB::RequestData portClearConnectionRequestData = {
			.requestType = USB_MAKE_REQUEST_TYPE( HostToDevice, Class, Other ),
			.code = USB::RequestCode::ClearFeature,
			.wValue = (uint16_t) HubFeatureSelector::PortConnectionChange,
			.wIndex = portId,
			.wLength = 0,
		};
		ControlPipe::PostRequest( controlPort,&portClearConnectionRequestData,nullptr, 0 );
		if(HUBPORT_FLAG( hubPortStatus, 0, Connect )) {
			USB::RequestData portResetRequestData = {
				.requestType = USB_MAKE_REQUEST_TYPE( HostToDevice, Class, Other ),
				.code = USB::RequestCode::SetFeature,
				.wValue = (uint16_t) HubFeatureSelector::PortReset,
				.wIndex = portId,
				.wLength = 0,
			};
			ControlPipe::PostRequest( controlPort, &portResetRequestData, nullptr, 0 );
			Utils_BusyMilliSleep( 50 );
		} else {
			debug_printf( ANSI_BRIGHT_ON "Device removed TODO\n" ANSI_BRIGHT_OFF );
		}
	}
	if(HUBPORT_FLAG( hubPortStatus, 1, Reset ) && !hub->currentlyResettingPort) {
		hub->currentlyResettingPort = true;
		// the speed of this port is now be available store it
		if(HUBPORT_FLAG(hubPortStatus, 0, HighSpeed)) hub->portSpeed[portId] = USB::Speed::HighSpeed;
		else if(HUBPORT_FLAG(hubPortStatus, 0, LowSpeed)) hub->portSpeed[portId] = USB::Speed::LowSpeed;
		else if(HUBPORT_FLAG(hubPortStatus, 0, SSSpeed)) hub->portSpeed[portId] = USB::Speed::SuperSpeed;
		else hub->portSpeed[portId] = USB::Speed::FullSpeed;

		USB::RequestData portClearConnectionRequestData = {
			.requestType = USB_MAKE_REQUEST_TYPE( HostToDevice, Class, Other ),
			.code = USB::RequestCode::ClearFeature,
			.wValue = (uint16_t) HubFeatureSelector::PortResetChange,
			.wIndex = portId,
			.wLength = 0,
		};
		ControlPipe::PostRequest( controlPort,&portClearConnectionRequestData,nullptr, 0 );
		auto controller = event_->pipe->device->controller;
		controller->newDeviceFunc( controller, hub, portId, AfterNewDevice );
	}
}

static void PostGetPortStatus( HubDevice * hub_, uint16_t port_ ) {
	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Class, Other ),
		.code = USB::RequestCode::GetStatus,
		.wValue = 0,
		.wIndex = port_,
		.wLength = 4,
	};

	ControlPipe::PostRequest( hub_->controlPipe,
														&requestData,
														HubGetPortStatusDone,
														port_);
}

static void PostPowerAllPorts( HubDevice *hub_ ) {
	USB::RequestData portPowerRequestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( HostToDevice, Class, Other ),
		.code = USB::RequestCode::SetFeature,
		.wValue = (uint16_t) HubFeatureSelector::PortPower,
		.wIndex = 0,
		.wLength = 0,
	};
	for(uint32_t i = 0U; i < hub_->numPorts; ++i) {
		portPowerRequestData.wIndex = i + 1;
		ControlPipe::PostRequest( hub_->controlPipe,
		                          &portPowerRequestData,
		                          nullptr,
		                          0 );
	}
	Utils_BusyMilliSleep( hub_->hubDescriptor2.powerOnToPowerGood * 2 );
}

static void HubDescriptorDone( Event *event_ ) {
	assert(event_->dmaBuffer);
	Cache_DCacheInvalidateLine(event_->dmaBuffer);
	auto hubDescriptor = (HubDescriptor *) event_->dmaBuffer;
	auto *hub = (HubDevice *) event_->pipe->device;

	if(hub->descriptor.bcdUSBVersion >= 0x300) {
		if(event_->pipe->device->controller->verbose > 2) HubDescriptor3::Dump((HubDescriptor3 *) hubDescriptor );
		memcpy( &hub->hubDescriptor3, hubDescriptor, sizeof( HubDescriptor3 ));
		hub->version3 = true;
	} else {
		if(event_->pipe->device->controller->verbose > 2) HubDescriptor2::Dump((HubDescriptor2 *) hubDescriptor );
		memcpy( &hub->hubDescriptor2, hubDescriptor, sizeof( HubDescriptor2 ));
		hub->version3 = false;
	}
	// we speed upto 7 port hubs
	hub->numPorts = Math_Min_U8(hubDescriptor->numPorts, 7);
	hub->controlPipe->postConfigureContext(hub->controlPipe, hub->numPorts, 1, &hub->statusEP);
}

static void ShortHubDescriptorDone( Event *event_ ) {
	assert(event_->dmaBuffer);
	Cache_DCacheInvalidateLine(event_->dmaBuffer);
	auto hubDescriptor = (HubDescriptor *) event_->dmaBuffer;
	auto controlPipe = (ControlPipe *) event_->pipe;
	auto device = event_->pipe->device;

	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Class, Device ),
		.code = USB::RequestCode::GetDescriptor,
		.wValue = (uint16_t) ((device->descriptor.bcdUSBVersion >= 0x300) ? 0x2A00 : 0x2900),
		.wIndex = (uint16_t) 0,
		.wLength = hubDescriptor->length,
	};
	ControlPipe::PostRequest(controlPipe, &requestData, HubDescriptorDone, 0 );
}

static void HubEnumerate(Device * device_) {
	auto hub = (HubDevice *) device_;
	if(hub->descriptor.bcdUSBVersion >= 0x300) return;
//			if(hub_->descriptor.bcdUSBVersion < 0x300) return;

	// we need a cache line to accept the status, as we have space in our device slack, we set it up here
	hub->statusCacheLineAddr = Core::alignTo((uintptr_t)(hub+1), 64);
	assert(hub->statusCacheLineAddr+64 < ((uintptr_t)(hub) + 1024));

	hub->controlPipe->postGetConfigurationDescriptor(hub->controlPipe);
}

static void HubConfigurationDescriptorReady(Device * device_, ConfigurationDescriptor * configurationDescriptor_, InterfaceDescriptor * interface) {
	auto hub = (HubDevice *) device_;
	auto config = (ConfigurationDescriptor *) configurationDescriptor_;
	assert(config);

	auto endpoints = (EndpointDescriptor *) (interface + 1);
	assert(interface->numEndpoints == 1);
	memcpy( &hub->statusEP, endpoints, sizeof(EndpointDescriptor));

	USB::RequestData requestData = {
		.requestType = USB_MAKE_REQUEST_TYPE( DeviceToHost, Class, Device ),
		.code = USB::RequestCode::GetDescriptor,
		.wValue = (uint16_t) ((hub->descriptor.bcdUSBVersion >= 0x300) ? 0x2A00 : 0x2900),
		.wIndex = (uint16_t) 0,
		.wLength = 8,
	};
	ControlPipe::PostRequest(hub->controlPipe, &requestData, ShortHubDescriptorDone, 0 );
}

static void HubInterfacesEnabled(Device * device_) {
	auto hub = (HubDevice *) device_;
	InterruptPipe::Init(&hub->statusPipe, hub, EndpointDescriptor::AddressToEndpointId(&hub->statusEP));

	if(hub->descriptor.bcdUSBVersion >= 0x300) PostSetDepth( hub);
	PostPowerAllPorts(hub);
	PostGetHubStatus(hub);
}

void HubDescriptor2::Dump( HubDescriptor2 const *hubDescriptor_ ) {
	debug_printf( ANSI_BRIGHT_ON "Hub Descriptor2" ANSI_BRIGHT_OFF "\n" );
	debug_printf( "length %i, %#04x %s\n", hubDescriptor_->length, (uint8_t) hubDescriptor_->descriptorType, DescriptorTypeToString(hubDescriptor_->descriptorType) );
	debug_printf( "numPorts %i, characteristics %i\n", hubDescriptor_->numPorts, hubDescriptor_->characteristics );
	debug_printf( "powerOnToPowerGood %i, maxCurrent %i\n", hubDescriptor_->powerOnToPowerGood, hubDescriptor_->maxCurrent );
	debug_printf( "deviceRemovable0 %#04x, deviceRemovable1 %#04x\n", hubDescriptor_->deviceRemovable0, hubDescriptor_->deviceRemovable1 );
}

void HubDescriptor3::Dump( HubDescriptor3 const *hubDescriptor_ ) {
	debug_printf( ANSI_BRIGHT_ON "Hub Descriptor3" ANSI_BRIGHT_OFF "\n" );
	debug_printf( "length %i, %#04x %s\n", hubDescriptor_->length, (uint8_t) hubDescriptor_->descriptorType, DescriptorTypeToString(hubDescriptor_->descriptorType) );
	debug_printf( "numPorts %i, characteristics %i\n", hubDescriptor_->numPorts, hubDescriptor_->characteristics );
	debug_printf( "powerOnToPowerGood %i, maxCurrent %i\n", hubDescriptor_->powerOnToPowerGood, hubDescriptor_->maxCurrent );
	debug_printf( "maxLatency %i, delay %i\n", hubDescriptor_->maxLatency, hubDescriptor_->delay );
	debug_printf( "deviceRemovable %#04x\n", hubDescriptor_->deviceRemovable );
}


void HubPortStatusDump( uint16_t const *status_, bool superSpeed_, bool compact_ ) {
	if(!compact_) {
		debug_printf( ANSI_BRIGHT_ON "Hub Port Status" ANSI_BRIGHT_OFF "\n" );
		auto raw = (uint8_t *) status_;
		debug_printf( "PortStatus: %#04x %#04x %#04x %#04x\n", raw[0], raw[1], raw[2], raw[3] );

		debug_printf( "connect %i enable %i\n", HUBPORT_FLAG( status_, 0, Connect ), HUBPORT_FLAG( status_, 0, Enable ));
		debug_printf( "suspend %i overCurrent %i\n", HUBPORT_FLAG( status_, 0, Suspend ), HUBPORT_FLAG( status_, 0, OverCurrent ));
		debug_printf( "reset %i power %i\n", HUBPORT_FLAG( status_, 0, Reset ), HUBPORT_FLAG( status_, 0, Power ));
		debug_printf( "lowSpeed %i highSpeed %i\n", HUBPORT_FLAG( status_, 0, LowSpeed ), HUBPORT_FLAG( status_, 0, HighSpeed ));
		debug_printf( "StatusIndicator %i\n", HUBPORT_FLAG( status_, 0, StatusIndicator ));
		debug_printf( "Change Flags:\n");
		debug_printf( "connect %i enable %i\n", HUBPORT_FLAG( status_, 1, Connect ), HUBPORT_FLAG( status_, 0, Enable ));
		debug_printf( "suspend %i overCurrent %i\n", HUBPORT_FLAG( status_, 1, Suspend ), HUBPORT_FLAG( status_, 2, OverCurrent ));
		debug_printf( "reset %i\n", HUBPORT_FLAG( status_, 1, Reset ));

	} else {
		debug_printf( ANSI_BRIGHT_ON "Hub Port Status: " ANSI_BRIGHT_OFF );
		if(HUBPORT_FLAG( status_, 0, Connect )) debug_printf( "Connected " );
		if(HUBPORT_FLAG( status_, 0, Enable )) debug_printf( "Enabled " );
		if(HUBPORT_FLAG( status_, 0, Reset )) debug_printf( "Reset " );
		if(HUBPORT_FLAG( status_, 0, Power )) debug_printf( "Power " );
		if(superSpeed_) {
			switch((status_[0] & HubPortStatusSSLinkState) >> 5) {
				case 0: debug_printf( "U0 " );
					break;
				case 1: debug_printf( "U1 " );
					break;
				case 2: debug_printf( "U2 " );
					break;
				case 3: debug_printf( "U3 " );
					break;
				case 4: debug_printf( "SS.disabled " );
					break;
				case 5: debug_printf( "Rx.Detect " );
					break;
				case 6: debug_printf( "SS.Inactive " );
					break;
				case 7: debug_printf( "Polling " );
					break;
				case 8: debug_printf( "Recovery " );
					break;
				case 9: debug_printf( "Hot Reset " );
					break;
				case 0xA: debug_printf( "Compliance " );
					break;
				case 0xB: debug_printf( "Loopback " );
					break;
				default:;
			}
		}
		if(HUBPORT_FLAG( status_, 0, Suspend )) debug_printf( "Suspend " );
		if(HUBPORT_FLAG( status_, 0, HighSpeed )) debug_printf( "Highspeed " );
		if(superSpeed_) {
			if(HUBPORT_FLAG( status_, 0, SSPower )) debug_printf( "SSPower " );
		} else {
			if(HUBPORT_FLAG( status_, 0, LowSpeed )) debug_printf( "Lowspeed " );
		}
		if(HUBPORT_FLAG( status_, 1, Connect )) debug_printf( "Connect Changed" );
		if(HUBPORT_FLAG( status_, 1, Reset )) debug_printf( "Reset Changed" );

		debug_print("\n");

	}
}

ClassHandlerVTable HubVTable = {
	.enumerate = HubEnumerate,
	.configurationDescriptorReady = HubConfigurationDescriptorReady,
	.interfacesEnabled = HubInterfacesEnabled,
};

} // end namespace