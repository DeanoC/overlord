#pragma once

#include "core/core.h"

namespace I2C {

enum class Speed {
	ONE_HUNDRED_KHZ = 0,
	FOUR_HUNDRED_KHZ = 1
};

enum MessageFlags {
	MF_Read = 0,
	MF_8BitRegisterAddress = 1,
};

struct Message {
	uint16_t registerAddress;
	uint8_t flags;

	uint32_t bufferLength;
	uint8_t * buffer;
};

void InitAsSupplier(Speed speed_);

// true if send was okay, false if nack'ed
bool Send(uint16_t address_, void* buffer_, uint8_t byteCount_);
void Receive(uint16_t address_, void * outBuffer_, uint8_t byteCount_);
void ReceiveLarge(uint16_t address_, void * outBuffer_, uint32_t byteCount_);

void SendMessages(uint16_t address_, uint32_t numberOfMessages_, Message const * messages_ );
template<typename VALUE>
VALUE ReadRegister8BitReg(uint16_t address_, VALUE registerAddress_) {
	VALUE buffer[1] = {};
	Message const msg = {
			.registerAddress = registerAddress_,
			.flags = MF_8BitRegisterAddress | MF_Read,
			.bufferLength = sizeof(VALUE),
			.buffer = (uint8_t *) buffer,
	};
	SendMessages(address_, 1, &msg);
	return buffer[0];
}
template<typename VALUE>
VALUE ReadRegister16BitReg(uint16_t address_, VALUE registerAddress_) {
	VALUE buffer[1] = {};
	Message const msg = {
			.registerAddress = registerAddress_,
			.flags = MF_Read,
			.bufferLength = sizeof(VALUE),
			.buffer = (uint8_t *) buffer,
	};
	SendMessages(address_, 1, &msg);
	return buffer[0];
}

template<typename VALUE>
void WriteRegister8BitReg(uint16_t address_, uint8_t registerAddress_, VALUE value_) {
	VALUE buffer[1] = { value_ };
	Message const msg = {
			.registerAddress = registerAddress_,
			.flags = MF_8BitRegisterAddress,
			.bufferLength = sizeof(VALUE),
			.buffer = (uint8_t *) buffer,
	};
	SendMessages(address_, 1, &msg);
}
template<typename VALUE>
void WriteRegister16BitReg(uint16_t address_, uint16_t registerAddress_, VALUE value_) {
	VALUE buffer[1] = { value_ };
	Message const msg = {
			.registerAddress = registerAddress_,
			.flags = 0,
			.bufferLength = sizeof(VALUE),
			.buffer = (uint8_t *) buffer,
	};
	SendMessages(address_, 1, &msg);
}


} // end namespace