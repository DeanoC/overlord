#pragma once

#include "core/c/math.h"

typedef enum {
	GICIN_RPU0_PERF_MON 	= 40, // GICP0 bit 0
	GICIN_RPU1_PERF_MON,
	GICIN_OCM_ERR,
	GICIN_LPD_APB,
	GICIN_RPU0_ECC,
	GICIN_RPU1_ECC,
	GICIN_NAND,
	GICIN_QSPI,
	GICIN_GPIO,
	GICIN_I2C0,
	GICIN_I2C1, // 50
	GICIN_SPI0,
	GICIN_SPI1,
	GICIN_UART0,
	GICIN_UART1,
	GICIN_CAN0,
	GICIN_CAN1,
	GICIN_LPD_APM,
	GICIN_RTC_ALARM,
	GICIN_RTC_SECONDS,
	GICIN_CLKMON, // 60
	GICIN_IPI_CH7,  // AKA IPI RPU PL2
	GICIN_IPI_CH8,  // AKA IPI RPU PL1
	GICIN_IPI_CH9,  // AKA IPI RPU PL0
	GICIN_IPI_CH10, // AKA IPI RPU PL3 // 64 GICP1 bit 1
	GICIN_IPI_CH1,	// AKA IPI RPU CPU0
	GICIN_IPI_CH2,	// AKA IPI RPU CPU1
	GICIN_IPI_CH0,	// AKA IPI APU CPU
	GICIN_TTC0_1,
	GICIN_TTC0_2,
	GICIN_TTC0_3, // 70
	GICIN_TTC1_1,
	GICIN_TTC1_2,
	GICIN_TTC1_3,
	GICIN_TTC2_1,
	GICIN_TTC2_2,
	GICIN_TTC2_3,
	GICIN_TTC3_1,
	GICIN_TTC3_2,
	GICIN_TTC3_3,
	GICIN_SDIO0, // 80
	GICIN_SDIO1,
	GICIN_SDIO0_WAKEUP,
	GICIN_SDUI1_WAKEUP,
	GICIN_LPD_SWDT,
	GICIN_CSU_SWDT,
	GICIN_LPD_ATB,
	GICIN_AIB,
	GICIN_SYSMON,
	GICIN_GEM0,
	GICIN_GEM0_WAKEUP, // 90
	GICIN_GEM1,
	GICIN_GEM1_WAKEUP,
	GICIN_GEM2,
	GICIN_GEM2_WAKEUP,
	GICIN_GEM3,
	GICIN_GEM3_WAKEUP, // 96 GICP2 bit 1
	GICIN_USB0_ENDPOINT_BULK,
	GICIN_USB0_ENDPOINT_ISOCHRONOUS,
	GICIN_USB0_ENDPOINT_CONTROLLER,
	GICIN_USB0_ENDPOINT_CONTROL, // 100
	GICIN_USB0_OTG,
	GICIN_USB1_ENDPOINT_BULK,
	GICIN_USB1_ENDPOINT_ISOCHRONOUS,
	GICIN_USB1_ENDPOINT_CONTROLLER,
	GICIN_USB1_ENDPOINT_CONTROL,
	GICIN_USB1_OTG,
	GICIN_USB0_WAKEUP,
	GICIN_USB1_WAKEUP,
	GICIN_LPD_DMA_0,
	GICIN_LPD_DMA_1, // 110
	GICIN_LPD_DMA_2,
	GICIN_LPD_DMA_3,
	GICIN_LPD_DMA_4,
	GICIN_LPD_DMA_5,
	GICIN_LPD_DMA_6,
	GICIN_LPD_DMA_7,
	GICIN_CSU,
	GICIN_CSU_DMA,
	GICIN_EFUSE,
	GICIN_LPD_XMPU_XPPU, // 120
	GICIN_PL_PS_0,
	GICIN_PL_PS_1,
	GICIN_PL_PS_2,
	GICIN_PL_PS_3,
	GICIN_PL_PS_4,
	GICIN_PL_PS_5,
	GICIN_PL_PS_6, // 128 GICP3 bit 1
	GICIN_PL_PS_7,
	GICIN_RESERVED_0,
	GICIN_RESERVED_1, // 130
	GICIN_RESERVED_2,
	GICIN_RESERVED_3,
	GICIN_RESERVED_4,
	GICIN_RESERVED_5,
	GICIN_RESERVED_6,
	GICIN_PL_PS_8,
	GICIN_PL_PS_9,
	GICIN_PL_PS_10,
	GICIN_PL_PS_11,
	GICIN_PL_PS_12, // 140
	GICIN_PL_PS_13,
	GICIN_PL_PS_14,
	GICIN_PL_PS_15,
	GICIN_DDR,
	GICIN_FPD_SWDT,
	GICIN_PCIE_MSI0,
	GICIN_PCIE_MSI1,
	GICIN_PCIE_INTX,
	GICIN_PCIE_DMA,
	GICIN_PCIE_MSC, // 150
	GICIN_DISPLAY_PORT,
	GICIN_FPD_APB,
	GICIN_FPD_ATB,
	GICIN_DPDMA,
	GICIN_FPD_APM,
	GICIN_FPD_DMA_0,
	GICIN_FPD_DMA_1,
	GICIN_FPD_DMA_2,
	GICIN_FPD_DMA_3,
	GICIN_FPD_DMA_4, // 160 // 160 GICP4 bit 1
	GICIN_FPD_DMA_5,
	GICIN_FPD_DMA_6,
	GICIN_FPD_DMA_7,
	GICIN_GPU,
	GICIN_SATA,
	GICIN_FPD_XMPU,
	GICIN_APU0_VCPUMNT,
	GICIN_APU1_VCPUMNT,
	GICIN_APU2_VCPUMNT,
	GICIN_APU3_VCPUMNT, // 170
	GICIN_APU0_CTI,
	GICIN_APU1_CTI,
	GICIN_APU2_CTI,
	GICIN_APU3_CTI,
	GICIN_APU0_PERF_MON,
	GICIN_APU1_PERF_MON,
	GICIN_APU2_PERF_MON,
	GICIN_APU3_PERF_MON,
	GICIN_APU0_COMM,
	GICIN_APU1_COMM, // 180
	GICIN_APU2_COMM,
	GICIN_APU3_COMM,
	GICIN_L2_CACHE,
	GICIN_APU_EXT_ERROR,
	GICIN_APU_REG_ERROR,
	GICIN_CCI,
	GICIN_SMMU       // 187 // 187 GICP4 bit 27
} GICInterrupt_Names;
static_assert(GICIN_RPU0_PERF_MON == 40);
static_assert(GICIN_I2C1 == 50);
static_assert(GICIN_CLKMON == 60);
static_assert(GICIN_TTC0_3 == 70);
static_assert(GICIN_SDIO0 == 80);
static_assert(GICIN_GEM0_WAKEUP == 90);
static_assert(GICIN_USB0_ENDPOINT_CONTROL == 100);
static_assert(GICIN_LPD_DMA_1 == 110);
static_assert(GICIN_LPD_XMPU_XPPU == 120);
static_assert(GICIN_RESERVED_1 == 130);
static_assert(GICIN_PL_PS_12 == 140);
static_assert(GICIN_PCIE_MSC == 150);
static_assert(GICIN_FPD_DMA_4 == 160);
static_assert(GICIN_APU3_VCPUMNT == 170);
static_assert(GICIN_APU1_COMM == 180);
static_assert(GICIN_SMMU == 187);

#define GICPROXY_INTERRUPT_GROUP_SIZE (LPD_SLCR_GICP1_IRQ_STATUS_OFFSET - LPD_SLCR_GICP0_IRQ_STATUS_OFFSET)

CONST_EXPR ALWAYS_INLINE GICInterrupt_Names GIC_ProxyToName(const uint32_t group, const uint32_t src) {
	switch(group) {
		default:
		case 0: return (GICInterrupt_Names)(GICIN_RPU0_PERF_MON + 0 + (Math_LogTwo_U32(src) - 8));
		case 1: return (GICInterrupt_Names)(GICIN_RPU0_PERF_MON + 24 + Math_LogTwo_U32(src));
		case 2: return (GICInterrupt_Names)(GICIN_RPU0_PERF_MON + 56 + Math_LogTwo_U32(src));
		case 3: return (GICInterrupt_Names)(GICIN_RPU0_PERF_MON + 88 + Math_LogTwo_U32(src));
		case 4: return (GICInterrupt_Names)(GICIN_RPU0_PERF_MON + 120 + Math_LogTwo_U32(src));
	}
}

CONST_EXPR ALWAYS_INLINE void GIC_NameToProxy(GICInterrupt_Names name, uint32_t* group, uint32_t* src) {
	const uint32_t group0start = GICIN_RPU0_PERF_MON;
	const uint32_t group0end = GICIN_RPU0_PERF_MON + 24;
	const uint32_t group1end = group0end + 32;
	const uint32_t group2end = group1end + 32;
	const uint32_t group3end = group2end + 32;
	const uint32_t group4end = group3end + 32;

	if(name < group0start) {
		*group = 0xFFFFFFFF; // error
		return;
	}

	if(name < group0end) {
		*group = 0;
		*src = Math_PowTwo_U32(8 + name - group0start);
	} else if(name < group1end) {
		*group = 1;
		*src = Math_PowTwo_U32(name - group1end);
	}else if(name < group2end) {
		*group = 2;
		*src = Math_PowTwo_U32(name - group2end);
	}else if(name < group3end) {
		*group = 3;
		*src = Math_PowTwo_U32(name - group3end);
	}else if(name < group4end) {
		*group = 4;
		*src = Math_PowTwo_U32(name - group4end);
	}
}

// return true to call the ROM routine
void GIC_Proxy();

