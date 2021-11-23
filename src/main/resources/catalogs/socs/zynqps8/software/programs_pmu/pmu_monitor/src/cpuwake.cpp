#include "core/core.h"
#include "rom_extensions.h"

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
	RomServiceTable[REN_ACPU0WAKE]();
}
void A53WakeUp1() {
	RomServiceTable[REN_ACPU1WAKE]();
}
void A53WakeUp2() {
	RomServiceTable[REN_ACPU2WAKE]();
}
void A53WakeUp3() {
	RomServiceTable[REN_ACPU3WAKE]();
}

void A53WakeUp() {
	RomServiceTable[REN_ACPU0WAKE]();
	RomServiceTable[REN_ACPU1WAKE]();
	RomServiceTable[REN_ACPU2WAKE]();
	RomServiceTable[REN_ACPU3WAKE]();
}

void R5FWakeUp0() {
	RomServiceTable[REN_R5F0WAKE]();
}
void R5FWakeUp1() {
	RomServiceTable[REN_R5F1WAKE]();
}

void R5FWakeUp() {
	RomServiceTable[REN_R5F0WAKE]();
	RomServiceTable[REN_R5F1WAKE]();
}
