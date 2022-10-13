#pragma once
#include "platform/registers/ipi.h"
#include "platform/registers/ipi_buffer.h"
#include "platform/memory_map.h"
#include "osservices.h"
#ifdef __cplusplus
EXTERN_C {
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

typedef enum PACKED {
	IBO_APU 				= 0x400,
	IBO_R5F_0				= 0x0,
	IBO_R5F_1				= 0x200,

	IBO_PL_0				= 0x600,
	IBO_PL_1				= 0x800,
	IBO_PL_2				= 0xA00,
	IBO_PL_3				= 0xC00,

	IBO_PMU					= 0xE00,
} IPI_BUFFER_OFFSET;

WARN_UNUSED_RESULT CONST_EXPR ALWAYS_INLINE IPI_Agent IPI_ChannelToAgent(IPI_Channel channel) {
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
WARN_UNUSED_RESULT CONST_EXPR ALWAYS_INLINE IPI_BUFFER_OFFSET IPI_ChannelToBuffer(IPI_Channel channel) {
	switch(channel) {
		case IC_APU: return IBO_APU;
		case IC_R5F_0: return IBO_R5F_0;
		case IC_R5F_1: return IBO_R5F_1;
		case IC_PL_0: return IBO_PL_0;
		case IC_PL_1: return IBO_PL_1;
		case IC_PL_2: return IBO_PL_2;
		case IC_PL_3: return IBO_PL_3;
		case IC_PMU_0:
		case IC_PMU_1:
		case IC_PMU_2:
		case IC_PMU_3:
			return IBO_PMU;
		default: return IBO_APU;
	}
}
#ifdef __cplusplus
	typedef enum OS_ServiceFunc : uint8_t {
#else
	typedef enum PACKED  {
#endif
	OSF_PTR_PRINT = 0,						// debug print from a ddr buffer, ptr must be valid until response
	OSF_DDR_LO_BLOCK_ALLOC, 			// allocate 1MB chunks of DDR in low (32bit) range
	OSF_DDR_HI_BLOCK_ALLOC, 			// allocate 1MB chunks of DDR in high (64bit) range
	OSF_FETCH_BOOT_DATA,					// retrieve the stored boot data
	OSF_FIRE_AND_FORGET_BIT = 0x80,

	OSF_INLINE_PRINT = OSF_FIRE_AND_FORGET_BIT | 0, 					// inline debug print <= 29 bytes
	OSF_DDR_LO_BLOCK_FREE = OSF_FIRE_AND_FORGET_BIT | 1,  		// free previously allocated lo chunks
	OSF_DDR_HI_BLOCK_FREE = OSF_FIRE_AND_FORGET_BIT | 2,  		// free previously allocated hi chunks
	OSF_BOOT_COMPLETE = OSF_FIRE_AND_FORGET_BIT | 4,					// boot loader is done, passing some parameters upto PMU
	OSF_CPU_WAKE_OR_SLEEP = OSF_FIRE_AND_FORGET_BIT | 6, 			// power down or up CPUs
	OSF_DEVICE_WAKE_OR_SLEEP = OSF_FIRE_AND_FORGET_BIT | 7,			// power down or up the Devices like the FPGA
} OS_ServiceFunc;

typedef struct PACKED {
	// the ddr address can be above the 32 bit boundary but unless noted in the function
	// shouldn't be bigger than 64K (possible 128K depending on the os server state)
	// for lower 32 bit Ddr addresses can be upto 2GB packets
	uint8_t _padd_0[6];	// padd to 64 bit alignment
	uintptr_all_t packetDdrAddress; // only valid for non fire and forget
	uint32_t packetSize; // packetSize includes first 32 bytes which is an IPI3_Msg
} IPI3_DdrPacket;

#define IPI3_HEADER_SIZE 2
#define IPI3_PACKET_MINUS_HEADER_SIZE (32 - IPI3_HEADER_SIZE)

typedef struct PACKED {
	OS_ServiceFunc function; 	// 8 bits (256 max functions)
	uint8_t ddrPtrFlag : 1;		// packet is in ddr memory, ptr must be valid till response
	uint8_t : 0;

	struct PACKED {
		union {
			uint8_t padd[IPI3_PACKET_MINUS_HEADER_SIZE];					// IPI3_Msg always 32 bytes
			IPI3_DdrPacket DdrPacket;
			struct PACKED {
				uint8_t _reserved : 3;
				uint8_t size : 5;																		// size of text in this packet
				uint8_t text[IPI3_PACKET_MINUS_HEADER_SIZE-1];			// ASCII text without a /0 terminator
			} InlinePrint;
			struct PACKED {
				IPI3_DdrPacket DdrPacket;
				uint8_t text[IPI3_PACKET_MINUS_HEADER_SIZE-sizeof(IPI3_DdrPacket)];			// first n character, rest follow msg
			} PtrPrint;
			struct PACKED {
				uint8_t padd0[2]; 			// align to 32 bit boundary as we have the space to spare
				uint32_t blocks64KB;   // how many blocks (each 64KB) to allocate
				uint32_t tag;          // purely for debug/log
			} DdrLoBlockAlloc;
			struct PACKED {
				uint16_t blockCount; 	// how many 64KB blocks to free
				uint32_t offset; 			// offset from base address to free
				uint32_t tag;          // purely for debug/log
			} DdrLoBlockFree;
			struct PACKED {
				uint8_t padd0[2]; 	// align to 32 bit boundary as we have the space to spare
				uint32_t blocks64KB;   // how many blocks (each 64KB) to allocate
				uint32_t tag;          // purely for debug/log
			} DdrHiBlockAlloc;
			struct PACKED {
				uint16_t blockCount; 	// how many 64KB blocks to free
				uint32_t offset; 			// offset from base address to free
				uint32_t tag;          // purely for debug/log
			} DdrHiBlockFree;
			struct PACKED {
				BootData bootData;
			} BootData;
			struct PACKED {
				uint8_t sleepA53_0;
				uint8_t sleepA53_1;
				uint8_t sleepA53_2;
				uint8_t sleepA53_3;
				uint8_t sleepR5f_0;
				uint8_t sleepR5f_1;

				uint8_t wakeA53_0;
				uint8_t wakeA53_1;
				uint8_t wakeA53_2;
				uint8_t wakeA53_3;
				uint8_t wakeR5f_0;
				uint8_t wakeR5f_1;

				uintptr_all_t wakeAddress;
			} CPUWakeOrSleep;
			struct PACKED {
				uint8_t sleepFPGA;

				uint8_t wakeFPGA;
			} DeviceWakeOrSleep;
		};
	} Payload;
} IPI3_Msg;

typedef enum PACKED {
	IRR_SUCCESS = 1,
	IRR_UNKNOWN_FAILURE = 0,
	IRR_OUT_OF_MEMORY = -1,
	IRR_BAD_PARAMETERS = -2,
} IPI3_ResponseResult;

typedef struct PACKED {
	IPI3_ResponseResult result;
	union {
		uint8_t _padd[31]; // IPI3_Response is always 32 bytes
		struct PACKED {
			uint32_t offset; // offset in bytes from DDR LO base of our RAM
		} DdrLoBlockAlloc;
		struct PACKED {
			uint32_t offset; // offset in bytes block from DDR Hi base of our RAM
		} DdrHiBlockAlloc;
		struct PACKED {
			BootData bootData;
		} BootData;
	};
} IPI3_Response;

static_assert(sizeof(IPI3_Msg) == 32, "IPI3_Msg must be exactly 32 bytes");
static_assert(sizeof(IPI3_Msg) - IPI3_PACKET_MINUS_HEADER_SIZE == 2, "IPI3_Msg header must be 2 bytes");

static_assert(sizeof(IPI3_Response) == 32, "IPI3_Response must be exactly 32 bytes");

#define IPI_MSG(buffer, target) (uint8_t*)(IPI_BUFFER_BASE_ADDR + (buffer) + ((target)*0x40))
#define IPI_RESPONSE(buffer, target) (uint8_t*)(IPI_BUFFER_BASE_ADDR + (buffer) + 0x20 + ((target)*0x40))

void IPI3_OsService_Submit(const IPI3_Msg *msg);
void IPI3_OnService_SubmitAndFetchResponse(const IPI3_Msg * msg, IPI3_Response * reply);

#ifdef __cplusplus
}
#endif

