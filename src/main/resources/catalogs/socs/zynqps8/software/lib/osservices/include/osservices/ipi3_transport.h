#pragma once
#include "hw_regs/ipi.h"
#include "hw_regs/ipi_buffer.h"
#ifdef __cplusplus
extern "C" {
#endif

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

	IC_CHANNEL_COUNT = 11
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

WARN_UNUSED_RESULT ALWAYS_INLINE IPI_Agent IPI_ChannelToAgent(IPI_Channel channel) {
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
WARN_UNUSED_RESULT ALWAYS_INLINE IPI_Channel IPI_AgentToChannel(IPI_Agent agent) {
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
#ifdef __cplusplus
	typedef enum OS_ServiceFunc : uint8_t {
#else
	typedef enum PACKED  {
#endif
	OSF_RESPONSE_BUFFER_FREE = 0, // lets the server know response buffer is free

	OSF_FIRE_AND_FORGET_BIT = 0x80,
	OSF_INLINE_PRINT = OSF_FIRE_AND_FORGET_BIT | 0, // inline print <= 29 bytes

	OSF_PTR_PRINT = 1,					// from a ddr buffer, ptr must be valid until response
	OSF_DDR_LO_BLOCK_ALLOC = 2, // allocate 1MB chunks of DDR
	OSF_DDR_LO_BLOCK_FREE = 3,  // free previously allocated chunks

} OS_ServiceFunc;

typedef struct PACKED {
	// used when groupFlag == false or when endGroupFlag == true
	OS_ServiceFunc function; 	// 8 bits (256 max functions)
	uint8_t ddrPtrFlag : 1;		// packet is in ddr memory, ptr must be valid till response
	uint8_t : 0;

	struct PACKED {
		union {
			uint8_t _padd[30];				// IPI3_Msg always 32 bytes
			struct PACKED {
				uintptr_all_t packetDdrAddress; // only valid for non fire and forget
				uint32_t packetSize; // packetSize includes first 32 bytes which is an IPI3_Msg
			} DdrPacket;
			struct PACKED {
				uint8_t _reserved : 3;
				uint8_t size : 5;			// size of text in this packet
				uint8_t text[29];			// ASCII text without a /0 terminator
			} InlinePrint;
			struct PACKED {
				uint8_t text[30];			// first 30 character, rest follow msg
			} PtrPrint;
			struct PACKED {
				uint8_t _padd_0[2]; 	// align to 32 bit boundary as we have the space to spare
				uint16_t blocks1MB;   // how many blocks (each 1MB) to allocate
			} DdrLoBlockAlloc;
			struct PACKED {
				uint8_t _padd_0[2]; 	// align to 32 bit boundary as we have the space to spare
				uint16_t free_blocks_starting_at; 		// offset in 1M blocks to free
			} DdrLoBlockFree;
		};
	} Payload;
} IPI3_Msg;

typedef enum PACKED {
	IRR_SUCCESS = 1,
	IRR_UNKNOWN_FAILURE = 0,
	IRR_OUT_OF_MEMORY = -1
} IPI3_ResponseResult;

typedef struct PACKED {
	IPI3_ResponseResult result;
	union {
		uint8_t _padd[31]; // IPI3_Response is always 32 bytes
		struct PACKED {
			uint16_t block_1MB_Offset; // offset in 1M block from DDR LO base of our RAM
		} DdrLoBlockAlloc;

	};
} IPI3_Response;

static_assert(sizeof(IPI3_Msg) == 32, "IPI3_Msg must be exactly 32 bytes");
static_assert(sizeof(IPI3_Response) == 32, "IPI3_Response must be exactly 32 bytes");

#define IPI_MSG_BEGIN(chan) (uint8_t*)((uintptr_t)(IPI_BUFFER_CH0_MSG_CH0_OFFSET + ((chan) * 0x200)))
#define IPI_RESPONSE_BEGIN(chan) (uint8_t*)((uintptr_t)(IPI_BUFFER_CH0_RESPONSE_CH0_OFFSET + ((chan) * 0x200)))

void IPI3_OsService_Submit(const IPI3_Msg *msg);
void IPI3_OnService_FetchResponse(IPI3_Response * reply);

#ifdef __cplusplus
}
#endif

