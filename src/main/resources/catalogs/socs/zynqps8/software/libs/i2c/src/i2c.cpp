//
// Created by deano on 4/15/22.
//

#include "core/core.h"
#include "core/math.h"
#include "dbg/assert.h"
#include "dbg/print.h"
#include "utils/busy_sleep.h"
#include "zynqps8/i2c/i2c.hpp"
#include "platform/registers/i2c.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "zynqps8/gic_v2/gic_v2.hpp"
#include "utils/slice.hpp"

#define XPAR_XIICPS_1_I2C_CLK_FREQ_HZ 99990005

namespace I2C {

#define FIFO_DEPTH          16	  /**< Number of bytes in the FIFO */
ALWAYS_INLINE void StallWhileBusIsBusy() {
	while(HW_REG_GET_BIT(I2C1, STATUS_REG, BA)) {
	};
}

static void FillTransmitFiFO(Utils::TrackingSlice<uint8_t>& slice_) {
	uint8_t const spaceInFifo = (uint8_t)FIFO_DEPTH - (uint8_t)HW_REG_GET(I2C1, TRANSFER_SIZE);
	uint8_t const bytesToTransfer = (spaceInFifo > slice_.left()) ? slice_.left() : spaceInFifo;
	for(uint8_t i = 0; i < bytesToTransfer; ++i) {
		HW_REG_SET(I2C1, I2C_DATA, *slice_);
		slice_++;
	}
}

void InitAsSupplier(Speed speed_) {

	HW_REG_SET(I2C1, INTRPT_DISABLE, I2C_INTRPT_DISABLE_USERMASK);
	HW_REG_SET(I2C1, CONTROL_REG, I2C_CONTROL_REG_CLR_FIFO);
	HW_REG_SET(I2C1, INTERRUPT_STATUS, 	HW_REG_GET(I2C1, INTERRUPT_STATUS));
	HW_REG_SET(I2C1, CONTROL_REG, 0);
	HW_REG_SET_FIELD(I2C1, TRANSFER_SIZE, TRANSFER_SIZE, 0);

	// ack master and 7/10 bit addressing are 'standard' for all transfers for us
	HW_REG_MERGE(I2C1, CONTROL_REG,
							 I2C_CONTROL_REG_ACK_EN_MASK |
							 I2C_CONTROL_REG_MS_MASK |
							 I2C_CONTROL_REG_NEA_MASK,

							 I2C_CONTROL_REG_ACK_EN |
							 I2C_CONTROL_REG_MS |
							 I2C_CONTROL_REG_NEA);

	HW_REG_SET_FIELD(I2C1, TIME_OUT, TO, 0x1F);
	HW_REG_SET(I2C1, STATUS_REG, 	HW_REG_GET(I2C1, STATUS_REG));

	switch(speed_){
		case Speed::ONE_HUNDRED_KHZ:
			HW_REG_SET_FIELD(I2C1, CONTROL_REG, DIVISOR_A, 0);
			HW_REG_SET_FIELD(I2C1, CONTROL_REG, DIVISOR_B, 50);
			break;
		case Speed::FOUR_HUNDRED_KHZ:
			HW_REG_SET_FIELD(I2C1, CONTROL_REG, DIVISOR_A, 0);
			HW_REG_SET_FIELD(I2C1, CONTROL_REG, DIVISOR_B, 11);
			break;

	}

}

void Send(uint16_t address_, void * buffer_, uint8_t byteCount_) {
	Utils::TrackingSlice<uint8_t> tslice {{ .data = (uint8_t *)buffer_, .size = byteCount_ }, 0};

	if(tslice.slice.size > FIFO_DEPTH) {
		HW_REG_MERGE(I2C1, CONTROL_REG, I2C_CONTROL_REG_HOLD_MASK | I2C_CONTROL_REG_RW_MASK, I2C_CONTROL_REG_HOLD);
	} else {
		HW_REG_MERGE(I2C1, CONTROL_REG, I2C_CONTROL_REG_HOLD_MASK | I2C_CONTROL_REG_RW_MASK, 0);
	}

	// clear interrupt status so we can monitor complete flag
	HW_REG_SET(I2C1, INTERRUPT_STATUS, 	HW_REG_GET(I2C1, INTERRUPT_STATUS));

	FillTransmitFiFO(tslice);
	HW_REG_SET(I2C1, I2C_ADDRESS, address_);

	while(tslice.left()) {
		while(HW_REG_GET_BIT(I2C1, STATUS_REG, TXDV)) {}
		FillTransmitFiFO(tslice);
	}

	if(tslice.slice.size > FIFO_DEPTH) {
		HW_REG_CLR_BIT(I2C1, CONTROL_REG, HOLD);
	}

	// stall until transfer is complete
	while(!HW_REG_GET_BIT(I2C1, INTERRUPT_STATUS, COMP)){
//		debug_printf("i2c1 interrupt status %#x\n", HW_REG_GET(I2C1, INTERRUPT_STATUS));
		assert(!HW_REG_GET_BIT(I2C1, INTERRUPT_STATUS, TO));
//		Utils_BusySecondSleep(1);
	};
}

void ReceiveLarge(uint16_t address_, void * outBuffer_, uint32_t byteCount_) {
	Utils::TrackingSlice<uint8_t> slice {{ .data = (uint8_t *)outBuffer_, .size = byteCount_ }, 0};
	while(slice.left() > 255) {
		Receive(address_, slice.current, 255);
		slice.increment(255);
	}

	if(slice.left()) {
		Receive(address_, slice.current, slice.left());
	}
}

void Receive(uint16_t address_, void * outBuffer_, uint8_t byteCount_) {
	Utils::TrackingSlice<uint8_t> tslice {{ .data = (uint8_t *)outBuffer_, .size = byteCount_ }, 0};

	if(tslice.slice.size > FIFO_DEPTH) {
		HW_REG_MERGE(I2C1, CONTROL_REG, I2C_CONTROL_REG_HOLD_MASK | I2C_CONTROL_REG_RW_MASK, I2C_CONTROL_REG_RW | I2C_CONTROL_REG_HOLD);
	} else {
		HW_REG_MERGE(I2C1, CONTROL_REG, I2C_CONTROL_REG_HOLD_MASK | I2C_CONTROL_REG_RW_MASK, I2C_CONTROL_REG_RW);
	}

	HW_REG_SET_FIELD(I2C1, TRANSFER_SIZE, TRANSFER_SIZE, byteCount_);
	HW_REG_SET(I2C1, I2C_ADDRESS, address_);

	while(tslice.left() > 0) {
		if(HW_REG_GET_BIT(I2C1, STATUS_REG, RXDV)) {
			*tslice = HW_REG_GET(I2C1, I2C_DATA);
			tslice++;
		}
	}

	if(tslice.slice.size > FIFO_DEPTH) {
		HW_REG_CLR_BIT(I2C1, CONTROL_REG, HOLD);
	}

	while(!HW_REG_GET_BIT(I2C1, INTERRUPT_STATUS, COMP)){};
	StallWhileBusIsBusy();
}

void SendMessages(uint16_t address_, uint32_t numberOfMessages_, Message const * messages_ ) {
	for(uint32_t i = 0; i < numberOfMessages_; ++i ) {
		Message const * msg = messages_ + i;
		if(msg->flags & MF_8BitRegisterAddress) {
			auto regAddr = (uint8_t)msg->registerAddress;
			debug_print("BOBa\n");
			Send(address_, &regAddr, 1);
			debug_print("BOBb\n");
		}	else {
			uint16_t regAddr = msg->registerAddress;
			Send(address_, &regAddr, 2);
		}

		if(msg->flags & MF_Read) {
			debug_print("BOBc1\n");
			Receive(address_, msg->buffer, msg->bufferLength);
			debug_print("BOBd1\n");
		} else {
			debug_print("BOBc2\n");
			Send(address_, msg->buffer, msg->bufferLength);
			debug_print("BOBd2\n");
		}
	}
}
uint8_t ReadRegister8(uint16_t address_, uint8_t registerAddress_) {

	uint8_t buffer[1];
	Message const msg = {
			.registerAddress = registerAddress_,
			.flags = MF_8BitRegisterAddress | MF_Read,
			.bufferLength = 1,
			.buffer = buffer,
	};
	SendMessages(address_, 1, &msg);
	return buffer[0];
}

uint16_t ReadRegister16(uint16_t address_, uint16_t registerAddress_) {
	uint8_t buffer[2];
	Message const msg = {
			.registerAddress = registerAddress_,
			.flags = MF_Read,
			.bufferLength = 2,
			.buffer = buffer,
	};
	SendMessages(address_, 1, &msg);
	return ((uint16_t *)buffer)[0];
}


}