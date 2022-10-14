#pragma once

#include "core/core.h"

namespace USB {

struct Device;
struct Pipe;
struct ControlPipe;

enum class PipeType : uint8_t {
	Control,
	Bulk,
	Interrupt,
	Isochronous,
};

struct Pipe {
	static void Init( Pipe * pipe_, Device * device_);
	PipeType type;
	Device * device;
};

typedef void (*ConfigureContextFunc)(ControlPipe * controlPipe_, uint8_t numHubPorts_, int numEPs, struct EndpointDescriptor * eps_);
typedef void (*GetConfigurationDescriptorFunc)(ControlPipe * controlPipe_);

struct ControlPipe : public Pipe {
	static void Init( ControlPipe * pipe_, Device * device_ );
	static void PostRequest( ControlPipe * pipe_, RequestData const *requestData_, EventCallbackFunc callback_, uint64_t callbackArg_);
	static void SyncCallback( ControlPipe * pipe_, EventCallbackFunc callback_, uint64_t callbackArg_);

	GetConfigurationDescriptorFunc postGetConfigurationDescriptor;

	// these function need setting up by the controller
	ConfigureContextFunc postConfigureContext;

};

struct InterruptPipe : public Pipe {
	static void Init( InterruptPipe * pipe_, Device * device_, uint8_t endpointId_ );

	static void AddInInterrupt( InterruptPipe * pipe_, uint8_t size_, uintptr_all_t dest_,  EventCallbackFunc callback_, uint64_t callbackArg_ );

	uint8_t endpointId;
};

} // USB
