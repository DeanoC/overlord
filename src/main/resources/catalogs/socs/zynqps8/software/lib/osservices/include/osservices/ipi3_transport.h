#pragma once
#include "hw_regs/ipi.h"
#include "hw_regs/ipi_buffer.h"

typedef enum {
	// APU
	IC_APU = IPI_PMU_1_ISR_CH0,

	// 2 R5Fs
	IC_R5F_0 = IPI_PMU_1_ISR_CH1,
	IC_R5F_1 = IPI_PMU_1_ISR_CH2,

	// 4 Microblaze PMU IPIs
	IC_PMU_0 = IPI_PMU_1_ISR_CH3,
	IC_PMU_1 = IPI_PMU_1_ISR_CH4,
	IC_PMU_2 = IPI_PMU_1_ISR_CH5,
	IC_PMU_3 = IPI_PMU_1_ISR_CH6,

	// 4 Custom FPGA cores
	IC_PL_0 = IPI_PMU_1_ISR_CH7,
	IC_PL_1 = IPI_PMU_1_ISR_CH8,
	IC_PL_2 = IPI_PMU_1_ISR_CH9,
	IC_PL_3 = IPI_PMU_1_ISR_CH10,

} IPI_Channel;

typedef enum PACKED {
	IA_APU = 0,
	IA_R5F_0 = 1,
	IA_R5F_1 = 2,
	IA_PL_0 = 3,
	IA_PL_1 = 4,
	IA_PL_2 = 5,
	IA_PL_3 = 6,
	IA_PMU = 7
} IPI_Agent;

ALWAYS_INLINE WARN_UNUSED_RESULT IPI_Agent IPI_ChannelToAgent(IPI_Channel channel) {
	switch(channel) {
		case IC_APU: return IA_APU;
		case IC_R5F_0: return IA_R5F_0;
		case IC_R5F_1: return IA_R5F_1;
		case IC_PL_0: return IA_PL_0;
		case IC_PL_1: return IA_PL_1;
		case IC_PL_2: return IA_PL_2;
		case IC_PL_3: return IA_PL_3;
		case IC_PMU_0:
		case IC_PMU_1:
		case IC_PMU_2:
		case IC_PMU_3:
			return IA_PMU;
		default: return IA_APU;
	}
}
ALWAYS_INLINE WARN_UNUSED_RESULT IPI_Channel IPI_AgentToChannel(IPI_Agent agent) {
	switch(agent) {
		case IA_APU: return IC_APU;
		case IA_R5F_0: return IC_R5F_0;
		case IA_R5F_1: return IC_R5F_1;
		case IA_PL_0: return IC_PL_0;
		case IA_PL_1: return IC_PL_1;
		case IA_PL_2: return IC_PL_2;
		case IA_PL_3: return IC_PL_3;
		case IA_PMU: return IC_PMU_0;
		default:
			return IC_APU;
	}
}
typedef enum PACKED {
	OSF_RESPONSE_BUFFER_FREE = 0, // lets the server know response buffer is free
	OSF_INLINE_PRINT, 			// inline print <= 30 bytes
	OSF_DDR_LO_BLOCK_ALLOC, // allocate 1MB chunks of DDR
	OSF_DDR_LO_BLOCK_FREE,  // free previously allocated chunks

} OS_ServiceFunc;

typedef struct PACKED {
	OS_ServiceFunc function; 	// 8 bits (256 max functions
	union {
		uint8_t _padd[31]; 			// IPI3_Msg is always 32 bytes
		struct PACKED {
			uint8_t size;					// size of text
			uint8_t text[30];			// ASCII text without a /0 terminator
		} OSF_InlinePrint;
		struct PACKED {
			uint8_t _padd_0[3]; 	// align to 32 bit boundray as we have the space to spare
			uint16_t blocks1MB;   // how many blocks (each 1MB) to allocate
		} OSF_DdrLoBlockAlloc;
		struct PACKED {
			uint8_t _padd_0[3]; 	// align to 32 bit boundray as we have the space to spare
			uint16_t free_blocks_starting_at; 		// offset in 1M blocks to free
		} OSF_DDRloBlockFree;
	};
} IPI3_Msg;

typedef enum PACKED {
	IRR_SUCCESS = 1,
	IRR_UNKNOWN_FAILURE = 0,
	IRR_OUT_OF_MEMORY = -1
} IPI3_ResponseResult;

typedef struct PACKED {
	IPI3_ResponseResult result;
	union {
		uint8_t _padd[32]; // IPI3_Response is always 32 bytes
		struct PACKED {
			uint16_t block_1MB_Offset; // offset in 1M block from DDR LO base of our RAM
		} OSF_DDRloBlockAlloc;

	};
} IPI3_Response;

static_assert(sizeof(IPI3_Msg) == 32, "IPI3_Msg must be exactly 32 bytes");

#define IPI_MSG_BEGIN(chan) (uint8_t*)((uintptr_t)(IPI_BUFFER_CH0_MSG_CH0_OFFSET + ((chan) * 0x200)))
#define IPI_RESPONSE_BEGIN(chan) (uint8_t*)((uintptr_t)(IPI_BUFFER_CH0_RESPONSE_CH0_OFFSET + ((chan) * 0x200)))

void IPI3_OsService_Submit(const IPI3_Msg *msg);
void IPI3_OnService_FetchResponse(IPI3_Response * reply);

