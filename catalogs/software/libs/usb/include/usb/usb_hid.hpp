#pragma once

#include "core/core.h"
#include "usb/usb.hpp"
#include "usb/usb_device.hpp"
#include "usb/usb_pipe.hpp"

namespace USB {
struct PACKED HIDDescriptor {
	uint8_t length;
	DescriptorType descriptorType;
	uint16_t bcdVersion;
	uint8_t countryCode; // for keyboards
	uint8_t numDescriptors;

	static void Dump( HIDDescriptor const * hidDescriptor);
};
static_assert(sizeof(HIDDescriptor) == 6);


enum class HIDInputDeviceType : uint8_t {
	GamePad,
	Mouse,
	Keyboard
};

// in general we expect Left axis to be XYZ and right X1Y1Z1 but depends on device tbh
// Z will probably be triggers
enum class HIDAxisDirection : uint8_t {
	X = 0,
	Y = 1,
	Z = 2,

	X1 = 3,
	Y1 = 4,
	Z1 = 5,
};
char const * HIDAxisDirectionToString(HIDAxisDirection direction_);


struct HIDGamePad {
	static constexpr int MAX_AXES = 8;

	uint8_t numAxes;
	uint8_t numButtons;
	uint32_t buttons;
	int16_t axisValues[MAX_AXES]; // -1 to 1 as symmetric signed 16 bit value (-32767 to -32767) (0xFFFF is never used!)

};

struct HIDDevice : public Device {
	uint8_t hasOutput : 1;
	HIDInputDeviceType : 4;

	EndpointDescriptor endpointDescriptors[2];
	InterruptPipe inputPipe;
	InterruptPipe outputPipe;
	union {
		HIDGamePad gamePad;
	};
	Memory_LinearAllocator dmaAllocator;

	uintptr_t inputDMAAddr;
	uint8_t inputDMASize;
	uint8_t numRawInputBlocks;
	uint8_t numRawOutputBlocks;

	struct ReportInputBlock * rawInputBlocks;
	struct ReportOutputBlock * rawOutputBlocks;
};

static_assert( sizeof( HIDDevice ) <= 1024 );

extern ClassHandlerVTable HIDVTable;

} // USB
