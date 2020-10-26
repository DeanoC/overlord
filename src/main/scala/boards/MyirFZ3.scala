package overlord.boards

import overlord._
import overlord.chips._

case class MyirFZ3() extends XilinxBoard("MyirFZ3")
{

  override val chips =  Seq(DDR4("4 GiB")) ++
                        SoCFromFile("ZynqPs8")  

  override val constraints = Array[BoardFeature](
    XiPin("PL_REF_CLK_25MHZ", "M6", ioStandard = XiC_LVCMOS18),

    XiPin("SYS_FAN_TACH", "AD14", ioStandard = XiC_LVCMOS33),
    XiPin("SYSTEM_STATUS_LED", "AA13", ioStandard = XiC_LVCMOS33),

    // J15
    XiDiffPin("HD_L06_25", Array("F12", "F11")),
    XiDiffPin("HD_L07_26", Array("G13", "F13")),
    XiDiffPin("HD_L08_25", Array("E12", "D11")),    
    XiDiffPin("HD_L10_25", Array("B11", "A10")),
    XiDiffPin("HP_BANK66_L01", Array("G1", "F1")),
    XiDiffPin("HP_BANK66_L02", Array("E1", "D1")),
    XiDiffPin("HP_BANK66_L07", Array("C1", "B1")),
    XiDiffPin("HP_BANK66_L08", Array("A2", "A1")),
    XiDiffPin("HP_BANK66_L12", Array("C3", "C2")),
    XiDiffPin("HP_BANK65_L07", Array("L1", "K1")),
    XiDiffPin("HP_BANK65_L08", Array("J1", "H1")),
    XiDiffPin("HP_BANK65_L12", Array("L3", "L2")),

    // J16
    XiPin("PS_MIO6","AF16"),
    XiPin("PS_MIO7","AH17"),
    XiPin("PS_MIO8","AF17"),
    XiPin("PS_MIO9","AC16"),
    XiDiffPin("HD_L03_26", Array("B13", "A13")),
    XiDiffPin("HD_L05_25", Array("G11", "F10")),
    XiDiffPin("HD_L06_26", Array("E14", "E13")),
    XiDiffPin("HD_L07_25", Array("E10", "D10")),
    XiDiffPin("HD_L08_26", Array("F15", "E15")),
    XiDiffPin("HD_L09_25", Array("C11", "B10")),
    XiDiffPin("HD_L10_26", Array("H14","H13")),
    XiDiffPin("HD_L11_25", Array("A12", "A11")),

    // BT1120
    XiPin("BT1120_CLK", "AC12"),
    XiPinArray( "BT1120_DATA", Array( "Y9",  "AA8", "AB9", "Y10", 
                                      "AH11", "AG10", "AF10", "AE10", 
                                      "AG11", "AF11", "AG21", "AF12", 
                                      "AD10", "AE12", "W10", "AD11")),
    XiPin("BT1120_UART0_RXD", "AH10"), // BT1120 GPIO X
    XiPin("BT1120_UART0_TXD", "AB10"), // BT1120 GPIO Y
    XiPin("BT1120_GPIO_Z", "AD12"),    // BT1120 GPIO Z

    // MIPI CSI
    XiDiffPin("MIPI_CSI_CLK", Array("AD5", "AD4")),
    XiDiffPin("MIPI_CSI_DATA0", Array("AC4", "AC3")),
    XiDiffPin("MIPI_CSI_DATA1", Array("AB4", "AB3")),
    XiDiffPin("MIPI_CSI_DATA2", Array("AB2", "AC2")),
    XiDiffPin("MIPI_CSI_DATA3", Array("AB1", "AC1")),
    XiPin("CAM_PWDN_n", "J7"),
    XiPin("CAM_RST_n", "H7"),
    XiPin("CAM_MCLK", "K8"),

    XiPin("QSPI_LOWER_SCK","AG15"),
    XiPinArray("QSPI_LOWER_DATA",Array("AH16","AG16","AF15","AH15")),
    XiPin("QSPI_LOWER_CS","AD16"),

    XiPin("SD0_EMMC_CMD","AC21"),
    XiPin("SD0_EMMC_CLK","AB20"),
    XiPin("SD0_EMMC_RST","AB18"),
    XiPinArray("SD0_EMMC_DATA",Array("AH18","AG18","AE18","AF18","AC18","AC19","AE19","AD19")),

    XiPin("SD1_WP","J20"),
    XiPin("SD1_CD","K20"),
    XiPin("SD1_CMD","M19"), // PS_MIO_50
    XiPin("SD1_CLK","L21"),
    XiPinArray("SD1_DATA",Array("L20","H21","J21","M18")),

    XiPin("USBHUB_RESET_N","J17"),
    XiPin("USBPHY_RESET_N","H18"),
    XiPin("USB0_CLK_IN", "G18"), // PS_MIO_52
    XiPin("USB0_DIR", "D16"),
    XiPin("USB0_NXT", "B16"),
    XiPin("USB0_STP", "F18"),
    XiPinArray("USB0_TX_DATA", Array("C16", "A16", "F17", "E17", "C17", "D17", "A17", "E18")),
    XiDiffPin("USB0_SSTX", Array("D23", "D24")),
    XiDiffPin("USB0_SSRX", Array("D27", "D28")),
    XiDiffPin("MGTR_USB3_0_CLK_26MHz", Array("E21", "E22")),

    XiPin("ENET_TXC", "E19"),
    XiPinArray("ENET_TDATA", Array("A18", "G19", "B18", "C18")),
    XiPin("ENET_TX_CTL", "D19"),
    XiPin("ENET_RXC", "C19"), // PS_MIO_70
    XiPinArray("ENET_RDATA", Array("B19", "G20", "G21", "D20")),
    XiPin("ENET_RX_CTL", "A19"),
    XiPin("ENET_MDC", "B20"),
    XiPin("ENET_MDIO", "F20"),
    XiPin("ENET_INT","H19"),
    XiPin("ENET_RST","K18"), // PS_MIO_40

    XiPin("WDT_FEED","J19"),

    XiPin("DEBUG_RXD","L17"),
    XiPin("DEBUG_TXD","H17"),
    XiPin("PS_PMIC_IRQ","L15"),
    XiPin("PL_PMIC_IRQ", "AE13"),

    XiPin("PMBUS_CLK", "AE15"), //PL_SMB_SCLK
    XiPin("PMBUS_DAT", "AG14"), // PL_SMB_SDA

    XiPin("CAN_TX","AB19"),
    XiPin("CAN_RX","AB21"),
    XiPin("PL_485_RXD", "AB15"),
    XiPin("PL_485_TXD", "AB14"),
    XiPin("PL_485_DE", "W14"),
    
    XiPin("SECURE_PIOA", "Y14"),
    XiPin("SECURE_PIOB", "Y13"),

    // i2c0   : cam (0x1B)
    XiPin("I2C0_SCL","L18"),
    XiPin("I2C0_SDA","K19"),
    // on i2c1: DeepCover Secure Auth chip(DS28C36BQ) 0x1B
    //        : 256Kb Eeprom (CAT24C256WI) 0x51, 
    //        : pcie routed to port
    //        : Clock Gen (si5332) Initial = 0x6A Config = 0x76
    //        : pmbus (TPS6508641RSKT) 0x5E
    XiPin("I2C1_SCL","J16"),
    XiPin("I2C1_SDA","L16"),

    XiDiffPin("MGT0_PCIE_TX0", Array("E25", "E26")),
    XiDiffPin("MGT0_PCIE_RX0", Array("F27", "F28")),
    XiDiffPin("MGTR_PCIE_CLK_100MHz", Array("F23", "F24")),
    XiPin("PCIE_RST","H16"),
    XiPin("PCIE_HOTPLUG_DET", "AG13"),

    XiDiffPin("MGT0_DP1_TX", Array("C25", "C26")),
    XiDiffPin("MGT0_DP0_TX", Array("B25", "B24")),
    XiDiffPin("MGTR_DP_CLK_27MHz", Array("C21", "C22")),
    XiPin("DP_DOUT","J15"),
    XiPin("DP_HP_DET","K15"),
    XiPin("DP_OE","G16"),
    XiPin("DP_DIN","F16") // PS_MIO_30
  )
}
