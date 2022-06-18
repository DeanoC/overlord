#include "core/core.h"
#include "rom_extensions.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/crf_apb.h"

void A53Sleep0() {
	RomServiceTable[REN_ACPU0SLEEP]();
}
void A53Sleep1() {
	RomServiceTable[REN_ACPU1SLEEP]();
}
void A53Sleep2() {
	RomServiceTable[REN_ACPU2SLEEP]();
}
void A53Sleep3() {
	RomServiceTable[REN_ACPU3SLEEP]();
}

void A53Sleep() {
	RomServiceTable[REN_ACPU0SLEEP]();
	RomServiceTable[REN_ACPU1SLEEP]();
	RomServiceTable[REN_ACPU2SLEEP]();
	RomServiceTable[REN_ACPU3SLEEP]();
}
void R5FSleep0() {
	RomServiceTable[REN_R5F0SLEEP]();
}
void R5FSleep1() {
	RomServiceTable[REN_R5F1SLEEP]();
}

void R5FSleep() {
	RomServiceTable[REN_R5F0SLEEP]();
	RomServiceTable[REN_R5F1SLEEP]();
}

void A53WakeUp0() {
	// put apu in reset
	HW_REG_MERGE(CRF_APB, RST_FPD_APU,
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU0_RESET)  |
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU0_PWRON_RESET),
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU0_RESET, 1) |
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU0_PWRON_RESET, 1)
						 );
	RomServiceTable[REN_PWRUPACPU0]();
	RomServiceTable[REN_ACPU0WAKE]();
	HW_REG_MERGE(CRF_APB, RST_FPD_APU,
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU0_RESET)  |
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU0_PWRON_RESET),
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU0_RESET, 0) |
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU0_PWRON_RESET, 0)
	);

}
void A53WakeUp1() {
	// put apu in reset
	HW_REG_MERGE(CRF_APB, RST_FPD_APU,
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU1_RESET)  |
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU1_PWRON_RESET),
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU1_RESET, 1) |
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU1_PWRON_RESET, 1)
	);
	RomServiceTable[REN_PWRUPACPU1]();
	RomServiceTable[REN_ACPU1WAKE]();
	HW_REG_MERGE(CRF_APB, RST_FPD_APU,
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU1_RESET)  |
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU1_PWRON_RESET),
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU1_RESET, 0) |
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU1_PWRON_RESET, 0)
	);
}
void A53WakeUp2() {
	// put apu in reset
	HW_REG_MERGE(CRF_APB, RST_FPD_APU,
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU2_RESET)  |
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU2_PWRON_RESET),
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU2_RESET, 1) |
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU2_PWRON_RESET, 1)
	);
	RomServiceTable[REN_PWRUPACPU2]();
	RomServiceTable[REN_ACPU2WAKE]();
	HW_REG_MERGE(CRF_APB, RST_FPD_APU,
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU2_RESET)  |
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU2_PWRON_RESET),
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU2_RESET, 0) |
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU2_PWRON_RESET, 0)
	);
}
void A53WakeUp3() {
	// put apu in reset
	HW_REG_MERGE(CRF_APB, RST_FPD_APU,
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU3_RESET)  |
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU3_PWRON_RESET),
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU3_RESET, 1) |
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU3_PWRON_RESET, 1)
	);
	RomServiceTable[REN_PWRUPACPU3]();
	RomServiceTable[REN_ACPU3WAKE]();
	HW_REG_MERGE(CRF_APB, RST_FPD_APU,
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU3_RESET)  |
							 HW_REG_FIELD_MASK(CRF_APB, RST_FPD_APU, ACPU3_PWRON_RESET),
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU3_RESET, 0) |
							 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU3_PWRON_RESET, 0)
	);
}

void R5FWakeUp0() {
	RomServiceTable[REN_R5F0WAKE]();
}
void R5FWakeUp1() {
	RomServiceTable[REN_R5F1WAKE]();
}