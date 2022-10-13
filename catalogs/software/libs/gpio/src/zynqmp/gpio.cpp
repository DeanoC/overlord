#include "core/core.h"
#include "gpio/gpio.hpp"
#include "dbg/assert.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/registers/gpio.h"
namespace Gpio {

void WriteBank(uint8_t bank_, uint32_t values_) {
	assert(bank_ < NumBanks);
	static constexpr uint32_t bankSize = GPIO_DATA_1_OFFSET - GPIO_DATA_0_OFFSET;
	hw_RegWrite(GPIO_BASE_ADDR, GPIO_DATA_0_OFFSET + (bank_ * bankSize), values_);
}

uint32_t ReadBank(uint8_t bank_) {
	assert(bank_ < NumBanks);
	static constexpr uint32_t bankSize = GPIO_DATA_1_RO_OFFSET - GPIO_DATA_0_RO_OFFSET;
	return hw_RegRead(GPIO_BASE_ADDR, GPIO_DATA_0_RO_OFFSET + (bank_ * bankSize));
}

void SetBankDirections(uint8_t bank_, uint32_t direction_) {
	assert(bank_ < NumBanks);
	static constexpr uint32_t bankSize = GPIO_DIRM_1_OFFSET - GPIO_DIRM_0_OFFSET;
	hw_RegWrite(GPIO_BASE_ADDR, GPIO_DIRM_0_OFFSET + (bank_ * bankSize), direction_);
}

uint32_t GetBankDirections(uint8_t bank_) {
	assert(bank_ < NumBanks);
	static constexpr uint32_t bankSize = GPIO_DIRM_1_OFFSET - GPIO_DIRM_0_OFFSET;
	return hw_RegRead(GPIO_BASE_ADDR, GPIO_DIRM_0_OFFSET + (bank_ * bankSize));
}

void SetBankOutputs(uint8_t bank_, uint32_t outputs_)  {
	assert(bank_ < NumBanks);
	static constexpr uint32_t bankSize = GPIO_OEN_1_OFFSET - GPIO_OEN_0_OFFSET;
	hw_RegWrite(GPIO_BASE_ADDR, GPIO_OEN_0_OFFSET + (bank_ * bankSize), outputs_);
}

uint32_t GetBankOutputs(uint8_t bank_) {
	assert(bank_ < NumBanks);
	static constexpr uint32_t bankSize = GPIO_OEN_1_OFFSET - GPIO_OEN_0_OFFSET;
	return hw_RegRead(GPIO_BASE_ADDR, GPIO_OEN_0_OFFSET + bank_ * bankSize);
}

constexpr void PinToBankAndOffset(uint8_t pin_, uint8_t & outBank_, uint8_t & outBitOffset_) {
	assert(pin_ < NumPins);
	uint32_t const bankTable[6] = {
		25,
		51,
		77,
		109,
		141,
		173
	};

	outBank_ = 0;

	while(outBank_ < 6) {
		if(pin_ < bankTable[outBank_]) break;
		outBank_++;
	}
	outBitOffset_ = pin_ % bankTable[outBank_];
}

void Write(uint8_t pin_, bool value_) {
	uint8_t bank;
	uint8_t bitOffset;
	PinToBankAndOffset(pin_, bank, bitOffset);

	if(bitOffset > 15) {
		static constexpr uint32_t bankSize = GPIO_MASK_DATA_1_MSW_OFFSET -GPIO_MASK_DATA_0_MSW_OFFSET;
		bitOffset = -16;
		uint32_t const bits = ((1 << (bitOffset + 16U)) ^ GPIO_MASK_DATA_0_MSW_MASK_0_MSW_MASK) | (value_ << bitOffset);
		hw_RegWrite(GPIO_BASE_ADDR, GPIO_MASK_DATA_0_MSW_OFFSET + (bank * bankSize), bits);
	} else {
		static constexpr uint32_t bankSize = GPIO_MASK_DATA_1_LSW_OFFSET -GPIO_MASK_DATA_0_LSW_OFFSET;
		uint32_t const bits = ((1 << (bitOffset + 16U)) ^ GPIO_MASK_DATA_0_LSW_MASK_0_LSW_MASK) | (value_ << bitOffset);
		hw_RegWrite(GPIO_BASE_ADDR, GPIO_MASK_DATA_0_LSW_OFFSET + (bank * bankSize), bits);
	}
}

bool Read(uint8_t pin_) {
	uint8_t bank;
	uint8_t bitOffset;
	PinToBankAndOffset(pin_, bank, bitOffset);
	uint32_t const bankBits = ReadBank(bank);
	return (bankBits >> bitOffset) & 0x1;
}

void SetDirection(uint8_t pin_, Direction direction_) {
	uint8_t bank;
	uint8_t bitOffset;
	PinToBankAndOffset(pin_, bank, bitOffset);
	uint32_t bankBits = GetBankDirections(bank);
	if(direction_ == Direction::Out) bankBits = bankBits | (1 << bitOffset);
	else bankBits = bankBits & ~(1 << bitOffset);
	SetBankDirections(bank, bankBits);
}

Direction GetDirection(uint8_t pin_) {
	uint8_t bank;
	uint8_t bitOffset;
	PinToBankAndOffset(pin_, bank, bitOffset);
	uint32_t const bankBits = GetBankDirections(bank);
	return (Direction) ((bankBits >> bitOffset) & 0x1);
}
void SetOutput(uint8_t pin_, bool output_) {
	uint8_t bank;
	uint8_t bitOffset;
	PinToBankAndOffset(pin_, bank, bitOffset);
	uint32_t bankBits = GetBankOutputs(bank);
	if(output_) bankBits = bankBits | (1 << bitOffset);
	else bankBits = bankBits & ~(1 << bitOffset);
	SetBankOutputs(bank, bankBits);
}

bool GetOutput(uint8_t pin_) {
	uint8_t bank;
	uint8_t bitOffset;
	PinToBankAndOffset(pin_, bank, bitOffset);
	uint32_t const bankBits = GetBankOutputs(bank);
	return ((bankBits >> bitOffset) & 0x1);
}

}
