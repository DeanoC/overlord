#include "core/core.h"
#include "zynqps8/display_port/aux.hpp"
#include "zynqps8/display_port/display.hpp"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/dp.h"
#include "utils/busy_sleep.h"
#include "dbg/print.h"
#include "dbg/assert.h"

namespace DisplayPort::Display {

static void StallWhileAuxBusy(Connection* link) {
	uint32_t timeout = 100;

	static uint32_t const mask = DP_REPLY_STATUS_REQUEST_IN_PROGRESS | DP_REPLY_STATUS_REPLY_IN_PROGRESS;

	do {
		uint32_t const status = HW_REG_READ1(DP, REPLY_STATUS);
		if((status & mask) == 0) return;
		Utils_BusyMicroSleep(20);
		timeout--;
	} while(timeout >= 0);

	debug_print("StallWhileAuxBusy timeout");
}


static void StallUntilAuxReplyIsDone(Connection* link) {
	uint32_t timeout = 100;

	do {
		uint32_t const status = HW_REG_READ1(DP, REPLY_STATUS);

		if((status & DP_REPLY_STATUS_REPLY_RECEIVED) &&
			!(status & DP_REPLY_STATUS_REPLY_IN_PROGRESS)) return;

		Utils_BusyMicroSleep(20);
		timeout--;
	} while(timeout >= 0);

	debug_print("StallUntilAuxReplyIsDone timeout");

}
static void StallForAuxReady(Connection* display) {
	uint32_t timeout = 100;

	do {
		uint32_t const status = HW_REG_READ1(DP, INTERRUPT_SIGNAL_STATE);
		if(	(status & DP_INTERRUPT_SIGNAL_STATE_REQUEST_STATE) == false) return;

		Utils_BusyMicroSleep(20);
		timeout--;
	} while(timeout >= 0);

	debug_print("StallForAuxReady timeout");

}

static bool AuxReadUpto16Bytes(Connection *link, uint32_t address, uint32_t const numBytes, uint8_t *data) {
	assert(numBytes <= 16);
	StallWhileAuxBusy(link);

	uint32_t tryCount = 100;
	Retry:;
	StallForAuxReady(link);
	HW_REG_WRITE1(DP, AUX_ADDRESS, address);
	HW_REG_WRITE1(DP, AUX_COMMAND_REGISTER,
						 HW_REG_ENCODE_FIELD(DP,
																 AUX_COMMAND_REGISTER,
																 AUX_CH_COMMAND,
																 DP_AUX_COMMAND_REGISTER_AUX_CH_COMMAND_AUX_READ) |
								 HW_REG_ENCODE_FIELD(DP, AUX_COMMAND_REGISTER, NUM_OF_BYTES, numBytes - 1));

	StallUntilAuxReplyIsDone(link);
	uint32_t const replyCode = HW_REG_READ1(DP, AUX_REPLY_CODE);
	if (HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE0, replyCode) == DP_AUX_REPLY_CODE_CODE0_AUX_NACK)
		return false;
	else if (HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE0, replyCode) == DP_AUX_REPLY_CODE_CODE0_AUX_DEFER) {
		Utils_BusyMicroSleep(20);

		// try again
		tryCount--;
		if (tryCount > 0)
			goto Retry;
		return false;
	}
	assert(HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE0, replyCode) == DP_AUX_REPLY_CODE_CODE0_AUX_ACK);

	tryCount = 100;
	RetryDataCount:;
	uint32_t const byteCount = HW_REG_READ1(DP, REPLY_DATA_COUNT);
	if (byteCount != numBytes && tryCount > 0) {
		Utils_BusyMicroSleep(100);
		tryCount--;
		goto RetryDataCount;
	}
	if (byteCount == numBytes) {
		for (unsigned int i = 0; i < byteCount; ++i) {
			data[i] = HW_REG_READ1(DP, AUX_REPLY_DATA);
		}
		return true;
	} else {
		return false;
	}
}

bool AuxRead(Connection *link, uint32_t address, uint32_t numBytes, uint8_t *data) {
	while(numBytes > 16) {
		if(!AuxReadUpto16Bytes(link, address, 16, data)) {
			return false;
		}
		data += 16;
		address += 16;
		numBytes -= 16;
	}

	if(numBytes > 0) {
		return AuxReadUpto16Bytes(link, address, numBytes, data);
	} else {
		return true;
	}
}

static bool AuxWriteUpto16Bytes(Connection *link, uint32_t address, uint32_t numBytes, uint8_t const *data) {
	assert(numBytes <= 16);
	StallWhileAuxBusy(link);

	uint32_t tryCount = 100;

Retry:;
	StallForAuxReady(link);
	HW_REG_WRITE1(DP, AUX_ADDRESS, address);

	for (uint32_t i = 0; i < numBytes; ++i) {
		HW_REG_WRITE1(DP, AUX_WRITE_FIFO, data[i]);
	}

	HW_REG_WRITE1(DP, AUX_COMMAND_REGISTER,
						 HW_REG_ENCODE_FIELD(DP,
																 AUX_COMMAND_REGISTER,
																 AUX_CH_COMMAND,
																 DP_AUX_COMMAND_REGISTER_AUX_CH_COMMAND_AUX_WRITE) |
								 HW_REG_ENCODE_FIELD(DP, AUX_COMMAND_REGISTER, NUM_OF_BYTES, (numBytes - 1)));

	StallUntilAuxReplyIsDone(link);

	uint32_t const replyCode = HW_REG_READ1(DP, AUX_REPLY_CODE);
	if (HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE0, replyCode) == DP_AUX_REPLY_CODE_CODE0_AUX_NACK)
		return false;
	else if (HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE0, replyCode) == DP_AUX_REPLY_CODE_CODE0_AUX_DEFER) {
		Utils_BusyMicroSleep(20);

		// try again
		tryCount--;
		if (tryCount > 0)
			goto Retry;
		return false;
	}
	assert(HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE0, replyCode) == DP_AUX_REPLY_CODE_CODE0_AUX_ACK);

	return true;
}

bool AuxWrite(Connection *link, uint32_t address, uint32_t numBytes, uint8_t const *data) {
	while(numBytes > 16) {
		if(!AuxWriteUpto16Bytes(link, address, 16, data)) {
			return false;
		}
		data += 16;
		address += 16;
		numBytes -= 16;
	}

	if(numBytes > 0) {
		return AuxWriteUpto16Bytes(link, address, numBytes, data);
	} else {
		return true;
	}
}

static bool I2CReadUpto16Bytes(Connection *link, bool mot, uint32_t address, uint32_t const numBytes, uint8_t *data) {
	assert(numBytes <= 16);
	StallWhileAuxBusy(link);

	uint32_t tryCount = 100;
	Retry:;
	StallForAuxReady(link);
	HW_REG_WRITE1(DP, AUX_ADDRESS, address);
	if(mot) {
		HW_REG_WRITE1(DP, AUX_COMMAND_REGISTER,
							 HW_REG_ENCODE_FIELD(DP,
																	 AUX_COMMAND_REGISTER,
																	 AUX_CH_COMMAND,
																	 DP_AUX_COMMAND_REGISTER_AUX_CH_COMMAND_I2C_READ_MOT) |
									 HW_REG_ENCODE_FIELD(DP, AUX_COMMAND_REGISTER, NUM_OF_BYTES, numBytes - 1));
	} else {
		HW_REG_WRITE1(DP, AUX_COMMAND_REGISTER,
							 HW_REG_ENCODE_FIELD(DP,
																	 AUX_COMMAND_REGISTER,
																	 AUX_CH_COMMAND,
																	 DP_AUX_COMMAND_REGISTER_AUX_CH_COMMAND_I2C_READ) |
									 HW_REG_ENCODE_FIELD(DP, AUX_COMMAND_REGISTER, NUM_OF_BYTES, numBytes - 1));
	}
	StallUntilAuxReplyIsDone(link);
	uint32_t const replyCode = HW_REG_READ1(DP, AUX_REPLY_CODE);
	if (HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE1, replyCode) == DP_AUX_REPLY_CODE_CODE1_I2C_NACK) {
		debug_print("!DP_AUX_REPLY_CODE_CODE1_I2C_NACK\n");
		return false;
	}
	else if( (HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE1, replyCode) == DP_AUX_REPLY_CODE_CODE1_I2C_DEFER) ||
			(HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE0, replyCode) == DP_AUX_REPLY_CODE_CODE0_AUX_DEFER) ) {
		Utils_BusyMilliSleep(1);
		// try again
		tryCount--;
		if (tryCount > 0)
			goto Retry;
		debug_print("!DP_AUX_REPLY_CODE_CODE0_AUX_DEFER\n");
		return false;
	}
	assert(HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE1, replyCode) == DP_AUX_REPLY_CODE_CODE1_I2C_ACK);

	tryCount = 100;
	RetryDataCount:;
	uint32_t const byteCount = HW_REG_READ1(DP, REPLY_DATA_COUNT);
	if (byteCount != numBytes && tryCount > 0) {
		Utils_BusyMicroSleep(100);
		tryCount--;
		goto RetryDataCount;
	}
	if (byteCount == numBytes) {
		for (unsigned int i = 0; i < byteCount; ++i) {
			data[i] = HW_REG_READ1(DP, AUX_REPLY_DATA);
		}
		return true;
	} else {
		debug_print("!byteCount == numBytes\n");
		return false;
	}
}

bool I2CReadBlock(Connection *link, uint32_t address, uint32_t numBytes, uint8_t *data) {
	while(numBytes > 16) {
		if(!I2CReadUpto16Bytes(link, true, address, 16, data)) {
			return false;
		}
		data += 16;
		numBytes -= 16;
	}

	if(numBytes > 0) {
		return I2CReadUpto16Bytes(link, false, address, numBytes, data);
	} else {
		return false;
	}
}

static bool I2CWriteUpto16Bytes(Connection *link, bool mot, uint32_t address, uint32_t numBytes, uint8_t const *data) {
	assert(numBytes <= 16);
	StallWhileAuxBusy(link);

	uint32_t tryCount = 100;

	Retry:;
	StallForAuxReady(link);
	HW_REG_WRITE1(DP, AUX_ADDRESS, address);

	for (uint32_t i = 0; i < numBytes; ++i) {
		HW_REG_WRITE1(DP, AUX_WRITE_FIFO, data[i]);
	}

	if(mot) {
		HW_REG_WRITE1(DP, AUX_COMMAND_REGISTER,
							 HW_REG_ENCODE_FIELD(DP,
																	 AUX_COMMAND_REGISTER,
																	 AUX_CH_COMMAND,
																	 DP_AUX_COMMAND_REGISTER_AUX_CH_COMMAND_I2C_WRITE_MOT) |
									 HW_REG_ENCODE_FIELD(DP, AUX_COMMAND_REGISTER, NUM_OF_BYTES, (numBytes - 1)));
	} else {
		HW_REG_WRITE1(DP, AUX_COMMAND_REGISTER,
							 HW_REG_ENCODE_FIELD(DP,
																	 AUX_COMMAND_REGISTER,
																	 AUX_CH_COMMAND,
																	 DP_AUX_COMMAND_REGISTER_AUX_CH_COMMAND_I2C_WRITE) |
									 HW_REG_ENCODE_FIELD(DP, AUX_COMMAND_REGISTER, NUM_OF_BYTES, (numBytes - 1)));
	}
	StallUntilAuxReplyIsDone(link);

	uint32_t const replyCode = HW_REG_READ1(DP, AUX_REPLY_CODE);
	if (HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE1, replyCode) == DP_AUX_REPLY_CODE_CODE1_I2C_NACK)
		return false;
	else if (HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE1, replyCode) == DP_AUX_REPLY_CODE_CODE1_I2C_DEFER) {
		Utils_BusyMicroSleep(20);

		// try again
		tryCount--;
		if (tryCount > 0)
			goto Retry;
		return false;
	}
	assert(HW_REG_DECODE_FIELD(DP, AUX_REPLY_CODE, CODE1, replyCode) == DP_AUX_REPLY_CODE_CODE1_I2C_ACK);

	return true;
}

#define SEGPTR_ADDR				0x30
bool I2CRead(Connection *link, uint32_t address, uint32_t offset, uint32_t numBytes, uint8_t *data) {
	uint8_t segment = offset >> 8;
	uint8_t off = (uint8_t)(offset & 0xFF);
	uint8_t currentBytesToRead;
	if(numBytes >= (256U - off)) {
		currentBytesToRead = (256U - off);
	} else {
		currentBytesToRead = numBytes;
	}

	while(numBytes > 0) {
		if(!I2CWriteUpto16Bytes(link, false, SEGPTR_ADDR, 1, &segment)) return false;
		if(!I2CWriteUpto16Bytes(link, true, address, 1, &off)) return false;
		if(!I2CReadBlock(link, address, currentBytesToRead, data)) return false;

		numBytes -= currentBytesToRead;
		data += currentBytesToRead;
		segment++;
		off = 0;
		currentBytesToRead = numBytes >= 256 ? numBytes : 255;
	}
	debug_printf("numBytes %i", numBytes);

	assert(numBytes == 0);
	return true;
}


} // end namespace