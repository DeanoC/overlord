#include "core/core.h"
#include "platform/memory_map.h"

#if defined(S_AXI_HPC0_FPD_BASE_ADDR)
static_assert(S_AXI_HPC0_FPD_BASE_ADDR == 0xFD360000U);
#endif
#if defined(S_AXI_HPC1_FPD_BASE_ADDR)
static_assert(S_AXI_HPC1_FPD_BASE_ADDR == 0xFD370000U);
#endif
#if defined(S_AXI_HP0_FPD_BASE_ADDR)
static_assert(S_AXI_HP0_FPD_BASE_ADDR == 0xFD380000U);
#endif
#if defined(S_AXI_HP1_FPD_BASE_ADDR)
static_assert(S_AXI_HP1_FPD_BASE_ADDR == 0xFD390000U);
#endif
#if defined(S_AXI_HP2_FPD_BASE_ADDR)
static_assert(S_AXI_HP2_FPD_BASE_ADDR == 0xFD3A0000U);
#endif
#if defined(S_AXI_HP3_FPD_BASE_ADDR)
static_assert(S_AXI_HP3_FPD_BASE_ADDR == 0xFD3B0000U);
#endif
#if defined(S_AXI_LPD_BASE_ADDR)
static_assert(S_AXI_LPD_BASE_ADDR == 0xFF9B0000U);
#endif

#if defined(ACPU_GIC_BASE_ADDR)
static_assert(ACPU_GIC_BASE_ADDR == 0xF9000000U);
#endif
#if defined(ACPU_GICC_BASE_ADDR)
static_assert(ACPU_GICC_BASE_ADDR == 0xF9020000U);
#endif
#if defined(ACPU_GICD_BASE_ADDR)
static_assert(ACPU_GICD_BASE_ADDR == 0xF9010000U);
#endif

#if defined(LPD_DMA_CH0_BASE_ADDR)
static_assert(LPD_DMA_CH0_BASE_ADDR == 0xFFA80000U);
#endif
#if defined(LPD_DMA_CH1_BASE_ADDR)
static_assert(LPD_DMA_CH1_BASE_ADDR == 0xFFA90000U);
#endif
#if defined(LPD_DMA_CH2_BASE_ADDR)
static_assert(LPD_DMA_CH2_BASE_ADDR == 0xFFAA0000U);
#endif
#if defined(LPD_DMA_CH3_BASE_ADDR)
static_assert(LPD_DMA_CH3_BASE_ADDR == 0xFFAB0000U);
#endif
#if defined(LPD_DMA_CH4_BASE_ADDR)
static_assert(LPD_DMA_CH4_BASE_ADDR == 0xFFAC0000U);
#endif
#if defined(LPD_DMA_CH5_BASE_ADDR)
static_assert(LPD_DMA_CH5_BASE_ADDR == 0xFFAD0000U);
#endif
#if defined(LPD_DMA_CH6_BASE_ADDR)
static_assert(LPD_DMA_CH6_BASE_ADDR == 0xFFAE0000U);
#endif
#if defined(LPD_DMA_CH7_BASE_ADDR)
static_assert(LPD_DMA_CH7_BASE_ADDR == 0xFFAF0000U);
#endif
#if defined(FPD_DMA_CH0_BASE_ADDR)
static_assert(FPD_DMA_CH0_BASE_ADDR == 0xFD500000U);
#endif
#if defined(FPD_DMA_CH1_BASE_ADDR)
static_assert(FPD_DMA_CH1_BASE_ADDR == 0xFD510000U);
#endif
#if defined(FPD_DMA_CH2_BASE_ADDR)
static_assert(FPD_DMA_CH2_BASE_ADDR == 0xFD520000U);
#endif
#if defined(FPD_DMA_CH3_BASE_ADDR)
static_assert(FPD_DMA_CH3_BASE_ADDR == 0xFD530000U);
#endif
#if defined(FPD_DMA_CH4_BASE_ADDR)
static_assert(FPD_DMA_CH4_BASE_ADDR == 0xFD540000U);
#endif
#if defined(FPD_DMA_CH5_BASE_ADDR)
static_assert(FPD_DMA_CH5_BASE_ADDR == 0xFD550000U);
#endif
#if defined(FPD_DMA_CH6_BASE_ADDR)
static_assert(FPD_DMA_CH6_BASE_ADDR == 0xFD560000U);
#endif
#if defined(FPD_DMA_CH7_BASE_ADDR)
static_assert(FPD_DMA_CH7_BASE_ADDR == 0xFD570000U);
#endif

#if defined(AMS_CTRL_BASE_ADDR)
static_assert(AMS_CTRL_BASE_ADDR == 0xFFA50000U);
#endif
#if defined(PLSYSMON_BASE_ADDR)
static_assert(PLSYSMON_BASE_ADDR == 0xFFA50c00U);
#endif
#if defined(PSSYSMON_BASE_ADDR)
static_assert(PSSYSMON_BASE_ADDR == 0xFFA50800U);
#endif

#if defined(APM_CCI_INTC_BASE_ADDR)
static_assert(APM_CCI_INTC_BASE_ADDR == 0xFD490000U);
#endif
#if defined(APM_INTC_OCM_BASE_ADDR)
static_assert(APM_INTC_OCM_BASE_ADDR == 0xFFA00000U);
#endif
#if defined(APM_LPD_FPD_BASE_ADDR)
static_assert(APM_LPD_FPD_BASE_ADDR == 0xFFA10000U);
#endif
#if defined(APM_DDR_BASE_ADDR)
static_assert(APM_DDR_BASE_ADDR == 0xFD0B0000U);
#endif

#if defined(APU_BASE_ADDR)
static_assert(APU_BASE_ADDR == 0xFD5C0000U);
#endif

#if defined(AXIPCIE_DMA0_BASE_ADDR)
static_assert(AXIPCIE_DMA0_BASE_ADDR == 0xFD0F0000U);
#endif
#if defined(AXIPCIE_DMA1_BASE_ADDR)
static_assert(AXIPCIE_DMA1_BASE_ADDR == 0xFD0F0080U);
#endif
#if defined(AXIPCIE_DMA2_BASE_ADDR)
static_assert(AXIPCIE_DMA2_BASE_ADDR == 0xFD0F0100U);
#endif
#if defined(AXIPCIE_DMA3_BASE_ADDR)
static_assert(AXIPCIE_DMA3_BASE_ADDR == 0xFD0F0180U);
#endif
#if defined(AXIPCIE_EGRESS0_BASE_ADDR)
static_assert(AXIPCIE_EGRESS0_BASE_ADDR == 0xFD0E0C00U);
#endif
#if defined(AXIPCIE_EGRESS1_BASE_ADDR)
static_assert(AXIPCIE_EGRESS1_BASE_ADDR == 0xFD0E0C20U);
#endif
#if defined(AXIPCIE_EGRESS2_BASE_ADDR)
static_assert(AXIPCIE_EGRESS2_BASE_ADDR == 0xFD0E0C40U);
#endif
#if defined(AXIPCIE_EGRESS3_BASE_ADDR)
static_assert(AXIPCIE_EGRESS3_BASE_ADDR == 0xFD0E0C60U);
#endif
#if defined(AXIPCIE_EGRESS4_BASE_ADDR)
static_assert(AXIPCIE_EGRESS4_BASE_ADDR == 0xFD0E0C80U);
#endif
#if defined(AXIPCIE_EGRESS5_BASE_ADDR)
static_assert(AXIPCIE_EGRESS5_BASE_ADDR == 0xFD0E0CA0U);
#endif
#if defined(AXIPCIE_EGRESS6_BASE_ADDR)
static_assert(AXIPCIE_EGRESS6_BASE_ADDR == 0xFD0E0CC0U);
#endif
#if defined(AXIPCIE_EGRESS7_BASE_ADDR)
static_assert(AXIPCIE_EGRESS7_BASE_ADDR == 0xFD0E0CE0U);
#endif
#if defined(AXIPCIE_INGRESS0_BASE_ADDR)
static_assert(AXIPCIE_INGRESS0_BASE_ADDR == 0xFD0E0800U);
#endif
#if defined(AXIPCIE_INGRESS1_BASE_ADDR)
static_assert(AXIPCIE_INGRESS1_BASE_ADDR == 0xFD0E0820U);
#endif
#if defined(AXIPCIE_INGRESS2_BASE_ADDR)
static_assert(AXIPCIE_INGRESS2_BASE_ADDR == 0xFD0E0840U);
#endif
#if defined(AXIPCIE_INGRESS3_BASE_ADDR)
static_assert(AXIPCIE_INGRESS3_BASE_ADDR == 0xFD0E0860U);
#endif
#if defined(AXIPCIE_INGRESS4_BASE_ADDR)
static_assert(AXIPCIE_INGRESS4_BASE_ADDR == 0xFD0E0880U);
#endif
#if defined(AXIPCIE_INGRESS5_BASE_ADDR)
static_assert(AXIPCIE_INGRESS5_BASE_ADDR == 0xFD0E08A0U);
#endif
#if defined(AXIPCIE_INGRESS6_BASE_ADDR)
static_assert(AXIPCIE_INGRESS6_BASE_ADDR == 0xFD0E08C0U);
#endif
#if defined(AXIPCIE_INGRESS7_BASE_ADDR)
static_assert(AXIPCIE_INGRESS7_BASE_ADDR == 0xFD0E08E0U);
#endif
#if defined(AXIPCIE_MAIN_BASE_ADDR)
static_assert(AXIPCIE_MAIN_BASE_ADDR == 0xFD0E0000U);
#endif

#if defined(BBRAM_BASE_ADDR)
static_assert(BBRAM_BASE_ADDR == 0xFFCD0000U);
#endif

#if defined(CAN0_BASE_ADDR)
static_assert(CAN0_BASE_ADDR == 0xFF060000U);
#endif
#if defined(CAN1_BASE_ADDR)
static_assert(CAN1_BASE_ADDR == 0xFF070000U);
#endif

#if defined(CCI_GPV_BASE_ADDR)
static_assert(CCI_GPV_BASE_ADDR == 0xFD6E0000U);
#endif
#if defined(CCI_REG_BASE_ADDR)
static_assert(CCI_REG_BASE_ADDR == 0xFD5E0000U);
#endif

#if defined(CORESIGHT_A53_ROM_BASE_ADDR)
static_assert(CORESIGHT_A53_ROM_BASE_ADDR == 0xFEC00000U);
#endif
#if defined(CORESIGHT_A53_DBG0_BASE_ADDR)
static_assert(CORESIGHT_A53_DBG0_BASE_ADDR == 0xFEC10000U);
#endif
#if defined(CORESIGHT_A53_DBG1_BASE_ADDR)
static_assert(CORESIGHT_A53_DBG1_BASE_ADDR == 0xFED10000U);
#endif
#if defined(CORESIGHT_A53_DBG2_BASE_ADDR)
static_assert(CORESIGHT_A53_DBG2_BASE_ADDR == 0xFEE10000U);
#endif
#if defined(CORESIGHT_A53_DBG3_BASE_ADDR)
static_assert(CORESIGHT_A53_DBG3_BASE_ADDR == 0xFEF10000U);
#endif
#if defined(CORESIGHT_A53_CTI0_BASE_ADDR)
static_assert(CORESIGHT_A53_CTI0_BASE_ADDR == 0xFEC20000U);
#endif
#if defined(CORESIGHT_A53_CTI1_BASE_ADDR)
static_assert(CORESIGHT_A53_CTI1_BASE_ADDR == 0xFED20000U);
#endif
#if defined(CORESIGHT_A53_CTI2_BASE_ADDR)
static_assert(CORESIGHT_A53_CTI2_BASE_ADDR == 0xFEE20000U);
#endif
#if defined(CORESIGHT_A53_CTI3_BASE_ADDR)
static_assert(CORESIGHT_A53_CTI3_BASE_ADDR == 0xFEF20000U);
#endif
#if defined(CORESIGHT_A53_PMU0_BASE_ADDR)
static_assert(CORESIGHT_A53_PMU0_BASE_ADDR == 0xFEC30000U);
#endif
#if defined(CORESIGHT_A53_PMU1_BASE_ADDR)
static_assert(CORESIGHT_A53_PMU1_BASE_ADDR == 0xFED30000U);
#endif
#if defined(CORESIGHT_A53_PMU2_BASE_ADDR)
static_assert(CORESIGHT_A53_PMU2_BASE_ADDR == 0xFEE30000U);
#endif
#if defined(CORESIGHT_A53_PMU3_BASE_ADDR)
static_assert(CORESIGHT_A53_PMU3_BASE_ADDR == 0xFEF30000U);
#endif
#if defined(CORESIGHT_A53_ETM0_BASE_ADDR)
static_assert(CORESIGHT_A53_ETM0_BASE_ADDR == 0xFEC40000U);
#endif
#if defined(CORESIGHT_A53_ETM1_BASE_ADDR)
static_assert(CORESIGHT_A53_ETM1_BASE_ADDR == 0xFED40000U);
#endif
#if defined(CORESIGHT_A53_ETM2_BASE_ADDR)
static_assert(CORESIGHT_A53_ETM2_BASE_ADDR == 0xFEE40000U);
#endif
#if defined(CORESIGHT_A53_ETM3_BASE_ADDR)
static_assert(CORESIGHT_A53_ETM3_BASE_ADDR == 0xFEF40000U);
#endif

// TODO more debug register banks (CTI to TSGEN)

#if defined(CRF_APB_BASE_ADDR)
static_assert(CRF_APB_BASE_ADDR == 0xFD1A0000U);
#endif
#if defined(CRL_APB_BASE_ADDR)
static_assert(CRL_APB_BASE_ADDR == 0xFF5E0000U);
#endif

#if defined(CSU_BASE_ADDR)
static_assert(CSU_BASE_ADDR == 0xFFCA0000U);
#endif
#if defined(CSUDMA_BASE_ADDR)
static_assert(CSUDMA_BASE_ADDR == 0xFFC80000U);
#endif

#if defined(CSU_WDT_BASE_ADDR)
static_assert(CSU_WDT_BASE_ADDR == 0xFFCB0000U);
#endif
#if defined(SWDT_BASE_ADDR)
static_assert(SWDT_BASE_ADDR == 0xFF150000U);
#endif
#if defined(WDT_BASE_ADDR)
static_assert(WDT_BASE_ADDR == 0xFD4D0000U);
#endif

#if defined(DDRC_BASE_ADDR)
static_assert(DDRC_BASE_ADDR == 0xFD070000U);
#endif

#if defined(DDR_PHY_BASE_ADDR)
static_assert(DDR_PHY_BASE_ADDR == 0xFD080000U);
#endif

#if defined(DDR_QOS_CTRL_BASE_ADDR)
static_assert(DDR_QOS_CTRL_BASE_ADDR == 0xFD090000U);
#endif

#if defined(DDR_XMPU0_CFG)
static_assert(DDR_XMPU0_CFG == 0xFD000000U);
#endif
#if defined(DDR_XMPU1_CFG)
static_assert(DDR_XMPU1_CFG == 0xFD010000U);
#endif
#if defined(DDR_XMPU2_CFG)
static_assert(DDR_XMPU2_CFG == 0xFD020000U);
#endif
#if defined(DDR_XMPU3_CFG)
static_assert(DDR_XMPU3_CFG == 0xFD030000U);
#endif
#if defined(DDR_XMPU4_CFG)
static_assert(DDR_XMPU4_CFG == 0xFD040000U);
#endif
#if defined(DDR_XMPU5_CFG)
static_assert(DDR_XMPU5_CFG == 0xFD050000U);
#endif

#if defined(DP_BASE_ADDR)
static_assert(DP_BASE_ADDR == 0xFD4A0000U);
#endif
#if defined(DPDMA_BASE_ADDR)
static_assert(DPDMA_BASE_ADDR == 0xFD4C0000U);
#endif

#if defined(EFUSE_BASE_ADDR)
static_assert(EFUSE_BASE_ADDR == 0xFFCC0000U);
#endif

#if defined(FPD_GPV_BASE_ADDR)
static_assert(FPD_GPV_BASE_ADDR == 0xFD700000U);
#endif
#if defined(FPD_SLCR_BASE_ADDR)
static_assert(FPD_SLCR_BASE_ADDR == 0xFD610000U);
#endif
#if defined(FPD_SLCR_SECURE_BASE_ADDR)
static_assert(FPD_SLCR_SECURE_BASE_ADDR == 0xFD690000U);
#endif

#if defined(XMPU_FPD_BASE_ADDR)
static_assert(XMPU_FPD_BASE_ADDR == 0xFD5D0000U);
#endif
#if defined(XMPU_SINK_ADDR)
static_assert(XMPU_SINK_ADDR == 0xFD4F0000U);
#endif

#if defined(GEM0_BASE_ADDR)
static_assert(GEM0_BASE_ADDR == 0xFF0B0000U);
#endif
#if defined(GEM1_BASE_ADDR)
static_assert(GEM1_BASE_ADDR == 0xFF0C0000U);
#endif
#if defined(GEM2_BASE_ADDR)
static_assert(GEM2_BASE_ADDR == 0xFF0D0000U);
#endif
#if defined(GEM3_BASE_ADDR)
static_assert(GEM3_BASE_ADDR == 0xFF0E0000U);
#endif

#if defined(GPIO_BASE_ADDR)
static_assert(GPIO_BASE_ADDR == 0xFF0A0000U);
#endif

#if defined(GPU_BASE_ADDR)
static_assert(GPU_BASE_ADDR == 0xFD4B0000U);
#endif

#if defined(I2C0_BASE_ADDR)
static_assert(I2C0_BASE_ADDR == 0xFF020000U);
#endif
#if defined(I2C1_BASE_ADDR)
static_assert(I2C1_BASE_ADDR == 0xFF030000U);
#endif

#if defined(IOU_GPV_BASE_ADDR)
static_assert(IOU_GPV_BASE_ADDR == 0xFE000000U);
#endif
#if defined(IOU_SCNTR_BASE_ADDR)
static_assert(IOU_SCNTR_BASE_ADDR == 0xFF250000U);
#endif
#if defined(IOU_SCNTRS_BASE_ADDR)
static_assert(IOU_SCNTRS_BASE_ADDR == 0xFF260000U);
#endif
#if defined(IOU_SECURE_SLCR_BASE_ADDR)
static_assert(IOU_SECURE_SLCR_BASE_ADDR == 0xFF240000U);
#endif
#if defined(IOU_SLCR_BASE_ADDR)
static_assert(IOU_SLCR_BASE_ADDR == 0xFF180000U);
#endif

#if defined(IPI_BASE_ADDR)
static_assert(IPI_BASE_ADDR == 0xFF300000U);
#endif

#if defined(LD_GPV_BASE_ADDR)
static_assert(LD_GPV_BASE_ADDR == 0xFE100000U);
#endif
#if defined(LD_SLCR_BASE_ADDR)
static_assert(LD_SLCR_BASE_ADDR == 0xFF410000U);
#endif
#if defined(LD_SLCR_SECURE_BASE_ADDR)
static_assert(LD_SLCR_SECURE_BASE_ADDR == 0xFF4B0000U);
#endif

#if defined(XPPU_BASE_ADDR)
static_assert(XPPU_BASE_ADDR == 0xFF980000U);
#endif
#if defined(XPPU_SINK_BASE_ADDR)
static_assert(XPPU_SINK_BASE_ADDR == 0xFF9C0000U);
#endif

#if defined(NAND_BASE_ADDR)
static_assert(NAND_BASE_ADDR == 0xFF100000U);
#endif

#if defined(OCM_BASE_ADDR)
static_assert(OCM_BASE_ADDR == 0xFF960000U);
#endif

#if defined(XMPU_OCM_BASE_ADDR)
static_assert(XMPU_OCM_BASE_ADDR == 0xFFA70000U);
#endif

#if defined(PCIE_ATTRIB_BASE_ADDR)
static_assert(PCIE_ATTRIB_BASE_ADDR == 0xFD480000U);
#endif

#if defined(PMU_GLOBAL_BASE_ADDR)
static_assert(PMU_GLOBAL_BASE_ADDR == 0xFFD80000U);
#endif
#if defined(PMU_IOMODULE_BASE_ADDR)
static_assert(PMU_IOMODULE_BASE_ADDR == 0xFFD40000U);
#endif
#if defined(PMU_LMB_BRAM_BASE_ADDR)
static_assert(PMU_LMB_BRAM_BASE_ADDR == 0xFFD50000U);
#endif
#if defined(PMU_LOCAL_BASE_ADDR)
static_assert(PMU_LOCAL_BASE_ADDR == 0xFFD60000U);
#endif

#if defined(LPD_GPV_BASE_ADDR)
static_assert(LPD_GPV_BASE_ADDR == 0xFE100000U);
#endif
#if defined(LPD_SLCR_BASE_ADDR)
static_assert(LPD_SLCR_BASE_ADDR == 0xFF410000U);
#endif
#if defined(LPD_SLCR_SECURE_BASE_ADDR)
static_assert(LPD_SLCR_SECURE_BASE_ADDR == 0xFF4B0000U);
#endif


#if defined(QSPI_BASE_ADDR)
static_assert(QSPI_BASE_ADDR == 0xFF0F0000U);
#endif

#if defined(RCPU_GIC_BASE_ADDR)
static_assert(RCPU_GIC_BASE_ADDR == 0xF9000000U);
#endif
#if defined(RPU_BASE_ADDR)
static_assert(RPU_BASE_ADDR == 0xFF9A0000U);
#endif

#if defined(RSA_BASE_ADDR)
static_assert(RSA_BASE_ADDR == 0xFFCE002CU);
#endif
#if defined(RSA_CORE_BASE_ADDR)
static_assert(RSA_CORE_BASE_ADDR == 0xFFCE0000U);
#endif

#if defined(RTC_BASE_ADDR)
static_assert(RTC_BASE_ADDR == 0xFFA60000U);
#endif

#if defined(SATA_AHCI_HBA_BASE_ADDR)
static_assert(SATA_AHCI_HBA_BASE_ADDR == 0xFD0C0000U);
#endif
#if defined(SATA_AHCI_PORT0_BASE_ADDR)
static_assert(SATA_AHCI_PORT0_BASE_ADDR == 0xFD0C0100U);
#endif
#if defined(SATA_AHCI_PORT1_BASE_ADDR)
static_assert(SATA_AHCI_PORT1_BASE_ADDR == 0xFD0C0180U);
#endif
#if defined(SATA_AHCI_VENDOR_BASE_ADDR)
static_assert(SATA_AHCI_VENDOR_BASE_ADDR == 0xFD0C00A0U);
#endif

#if defined(SD_BASE_ADDR)
static_assert(SD_BASE_ADDR == 0xFF160000U);
#endif
#if defined(SD1_BASE_ADDR)
static_assert(SD1_BASE_ADDR == 0xFF170000U);
#endif

#if defined(SERDES_BASE_ADDR)
static_assert(SERDES_BASE_ADDR == 0xFD400000U);
#endif
#if defined(SERDES2_BASE_ADDR)
static_assert(SERDES2_BASE_ADDR == 0xFD410000U);
#endif

#if defined(SIOU_BASE_ADDR)
static_assert(SIOU_BASE_ADDR == 0xFD3D0000U);
#endif

#if defined(SMMU_GPV_BASE_ADDR)
static_assert(SMMU_GPV_BASE_ADDR == 0xFD800000U);
#endif
#if defined(SMMU_REG_BASE_ADDR)
static_assert(SMMU_REG_BASE_ADDR == 0xFD5F0000U);
#endif

#if defined(SPI0_BASE_ADDR)
static_assert(SPI0_BASE_ADDR == 0xFF040000U);
#endif
#if defined(SPI1_BASE_ADDR)
static_assert(SPI1_BASE_ADDR == 0xFF050000U);
#endif

#if defined(TTC0_BASE_ADDR)
static_assert(TTC0_BASE_ADDR == 0xFF110000U);
#endif
#if defined(TTC1_BASE_ADDR)
static_assert(TTC1_BASE_ADDR == 0xFF120000U);
#endif
#if defined(TTC2_BASE_ADDR)
static_assert(TTC2_BASE_ADDR == 0xFF130000U);
#endif
#if defined(TTC3_BASE_ADDR)
static_assert(TTC3_BASE_ADDR == 0xFF140000U);
#endif

#if defined(UART0_BASE_ADDR)
static_assert(UART0_BASE_ADDR == 0xFF000000U);
#endif

#if defined(UART1_BASE_ADDR)
static_assert(UART1_BASE_ADDR == 0xFF010000U);
#endif


#if defined(USB30_REGS_BASE_ADDR)
static_assert(USB30_REGS_BASE_ADDR == 0xFF9D0000U);
#endif
#if defined(USB30_XHCI_BASE_ADDR)
static_assert(USB30_XHCI_BASE_ADDR == 0xFE200000U);
#endif
#if defined(USB31_REGS_BASE_ADDR)
static_assert(USB31_REGS_BASE_ADDR == 0xFF9E0000U);
#endif
#if defined(USB31_XHCI_BASE_ADDR)
static_assert(USB31_XHCI_BASE_ADDR == 0xFE300000U);
#endif

#if defined(VCU_ENC_TOP_BASE_ADDR)
static_assert(VCU_ENC_TOP_BASE_ADDR == 0xA0000000U);
#endif
#if defined(VCU_DEC_TOP_BASE_ADDR)
static_assert(VCU_DEC_TOP_BASE_ADDR == 0xA0020000U);
#endif
#if defined(VCU_SLCR_BASE_ADDR)
static_assert(VCU_SLCR_BASE_ADDR == 0xA0040000U);
#endif

