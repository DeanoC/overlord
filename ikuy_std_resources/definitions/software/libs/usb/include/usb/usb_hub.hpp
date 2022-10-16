#pragma once

#include "usb/usb.hpp"
#include "usb/usb_device.hpp"
#include "usb/usb_pipe.hpp"

namespace USB {

enum class HubFeatureSelector {
	PortConnection = 0,
	PortEnable = 1,
	PortSuspend = 2,
	PortOverCurrent = 3,
	PortReset = 4,
	PortPower = 8,
	PortLowSpeed = 9,

	PortConnectionChange = 0 | 16,
	PortEnableChange = 1 | 16,
	PortSuspendChange = 2 | 16,
	PortOverCurrentChange = 3 | 16,
	PortResetChange = 4 | 16,
	PortPowerChange = 8 | 16,
	PortLowSpeedChange = 9 | 16,
};

struct PACKED HubDescriptor {
	uint8_t length;
	DescriptorType descriptorType;
	uint8_t numPorts;
	uint16_t characteristics;
	uint8_t powerOnToPowerGood;
	uint8_t maxCurrent;
};
static_assert( sizeof(HubDescriptor) == 7);
struct PACKED HubDescriptor2 : public HubDescriptor {

	uint8_t deviceRemovable0;
	uint8_t deviceRemovable1; // this is only if more than 7 ports
	static void Dump( HubDescriptor2 const * hubDescriptor_);
};
static_assert( sizeof(HubDescriptor2) == 9);

struct PACKED HubDescriptor3 :  public HubDescriptor {
	uint8_t maxLatency;
	uint16_t delay;
	uint16_t deviceRemovable;
	static void Dump( HubDescriptor3 const * hubDescriptor_);
};
static_assert( sizeof(HubDescriptor3) == 12);

constexpr uint16_t HubPortStatusConnect = 0x0001;
constexpr uint16_t HubPortStatusEnable = 0x0002;
constexpr uint16_t HubPortStatusSuspend = 0x0004;
constexpr uint16_t HubPortStatusOverCurrent = 0x0008;
constexpr uint16_t HubPortStatusReset = 0x0010;
constexpr uint16_t HubPortStatusPower = 0x0100;
constexpr uint16_t HubPortStatusLowSpeed = 0x0200;
constexpr uint16_t HubPortStatusHighSpeed = 0x0400;
constexpr uint16_t HubPortStatusStatusIndicator = 0x1000;

// USB 3 (SuperSpeed) only
constexpr uint16_t HubPortStatusSSLinkState = 0x01e0;
constexpr uint16_t HubPortStatusSSPower = 0x0200;
constexpr uint16_t HubPortStatusSSSpeed = 0x1c00;

// USB3 seems to not want to support ports > 4 ports, we support 7
// WTF? I know 7 isn't a power of two but the hub steals one bit for its own update status,
// so 7 ports bits in a byte
struct HubDevice : public Device {
	union {
		HubDescriptor2 hubDescriptor2;
		HubDescriptor3 hubDescriptor3;
	};
	EndpointDescriptor statusEP;
	InterruptPipe statusPipe;

	uint32_t version3 : 1;
	uint32_t currentlyResettingPort : 1;
	uint32_t numPorts : 4;
	uint32_t hubDepth : 5;

	Speed portSpeed[8]; // 0 is never used

	uintptr_all_t statusCacheLineAddr;

};

static_assert( sizeof( HubDevice ) <= 1024 );

extern ClassHandlerVTable HubVTable;

void HubPortStatusDump(uint16_t const * status_, bool superSpeed_, bool compact_);

} // end namespace