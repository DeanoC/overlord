package overlord.boards

import overlord._
import overlord.chips._

case class PynqZ2() extends XilinxBoard("PynqZ2")
{
  override val chips = List[Chip](
    DDR3("512 MiB")
  )

  val qspi0 : Array[(String, XiConstraintDirection, Boolean)] = Array(
    ("A7", XiC_OutputDirection, true), // 1 qspi0_ss_b
    ("B8", XiC_BiDirection, false), // qspi0_io[0]
    ("D6", XiC_BiDirection, false), // qspi0_io[1]
    ("B7", XiC_BiDirection, false), // qspi0_io[2]
    ("A6", XiC_BiDirection, false), // qspi0_io[3]
    ("A5", XiC_OutputDirection, false), // qspi0_sclk
    ("D5", XiC_BiDirection, false) // qspi0_fbclk
  )

  val uart0 : Array[(String, String, XiConstraintDirection)] = Array(
    ("rx", "C5", XiC_InputDirection), // 14
    ("tx", "C8", XiC_OutputDirection) // 15
  )


  val enet0 : Array[(String, String, XiConstraintDirection)] = Array(
    ("reset", "B5", XiC_OutputDirection),  // 9 
    ("tx_clk", "A19", XiC_OutputDirection), // 16 
    ("txd[0]", "E14", XiC_OutputDirection), // 
    ("txd[1]", "B18", XiC_OutputDirection), // 
    ("txd[2]", "D10", XiC_OutputDirection), // 
    ("txd[3]", "A17", XiC_OutputDirection), // 0
    ("tx_ctrl", "F14", XiC_OutputDirection), // 
    ("rx_clk", "B17", XiC_InputDirection),  //
    ("rxd[0]", "D11", XiC_InputDirection),  //
    ("rxd[1]", "A16", XiC_InputDirection),  //
    ("rxd[2]", "F15", XiC_InputDirection),  //
    ("rxd[3]", "A15", XiC_InputDirection),  //
    ("rx_ctrl", "D13", XiC_InputDirection),  //
    ("mdio", "C10", XiC_BiDirection),     // 52
    ("mdio", "C11", XiC_BiDirection)     // 53
  )

  val usb0 : Array[(String, String, XiConstraintDirection)] = Array(
    ("dir", "C13", XiC_InputDirection),
    ("stp", "C15", XiC_OutputDirection),
    ("nxt", "E16", XiC_InputDirection), 
    ("clk", "A11", XiC_BiDirection),  
    ("data[0]", "A14", XiC_BiDirection),
    ("data[1]", "D15", XiC_BiDirection),
    ("data[2]", "A12", XiC_BiDirection),
    ("data[3]", "F12", XiC_BiDirection),
    ("data[4]", "C16", XiC_BiDirection),
    ("data[5]", "A10", XiC_BiDirection),
    ("data[6]", "E13", XiC_BiDirection),
    ("data[7]", "C18", XiC_BiDirection),
    ("reset", "D16", XiC_OutputDirection)
  )
  override val constraints = Array[BoardFeature](
    XiPin("clk","H16"),
    XiPinArray("switches", Array("M20", "M19"), buffering = XiC_InputBuffer),
    XiPinArray("rgb_led0", Array("N15", "G17", "L15")),
    XiPinArray("rgb_led1", Array("M15", "L14", "G14")),
    XiPinArray("leds", Array("R14", "P14", "N16", "M14")),
    XiPinArray("btns", Array("D19", "D20", "L20", "L19")),
    XiPinArray("pmod_a", Array("Y18", "Y19", "Y16", "Y17", "U18", "U19", "W18", "W19")),
    XiPinArray("pmod_b", Array("W14", "Y14", "T11", "T10", "V16", "W16", "V12", "W13")),

    XiPinArray("adr", Array("M17", "M18")),
    XiPin("au_mclk_r","U5"),
    XiPin("au_sda_r","T9"),
    XiPin("au_scl_r","U9"),
    XiPin("au_dout_r","F17"),
    XiPin("au_din_r","G18"),
    XiPin("au_wclk_r","T17"),
    XiPin("au_bclk_r","R18"),

    XiDiffPin("ar_an0", Array("E17", "D18")),
    XiDiffPin("ar_an1", Array("E18", "E19")),
    XiDiffPin("ar_an2", Array("K14", "J14")),
    XiDiffPin("ar_an3", Array("K16", "J16")),
    XiDiffPin("ar_an4", Array("J20", "H20")),
    XiDiffPin("ar_an5", Array("G19", "G20")),

    XiPinArray("ar", Array( "T14", "U12", "U13", "V13", "V15", "T15", "R16", 
                            "U17", "V17", "V18", "T16", "R17", "P18", "N17")),
    XiPinArray("a", Array("Y11", "Y12", "W11", "V11", "T5", "U10")),

    XiPin("ck_miso", "W15"),
    XiPin("ck_mosi", "T12"),
    XiPin("ck_sck", "H15"),
    XiPin("ck_ss", "F16"),

    XiPinArray("rpio", Array( "Y16", "Y17", "W18", "W19", "Y18", "Y19", "U18", 
                              "U19", "F19", "V10", "V8", "W10", "B20", 
                              "B20","W8", "V6", "Y6", "B19","U7","C20", "Y8",
                              "A20","Y9","U8", "W6", "Y7", "F20", "W9" )),

    XiPin("hdmi_tx_cec", "G15"),
    XiDiffPin("hdmi_tx_clk", Array("L16", "L17"), ioStandard = XiC_TDMS_33),
    XiDiffPin("hdmi_tx_d0", Array("K17", "K18"), ioStandard = XiC_TDMS_33),
    XiDiffPin("hdmi_tx_d1", Array("K19", "J19"), ioStandard = XiC_TDMS_33),
    XiDiffPin("hdmi_tx_d2", Array("J18", "H18"), ioStandard = XiC_TDMS_33),
    XiPin("hdmi_tx_hpdn", "R19"),

    XiPin("hdmi_rx_cec", "H17"),
    XiDiffPin("hdmi_rx_clk", Array("N18", "P19"), ioStandard = XiC_TDMS_33),
    XiDiffPin("hdmi_rx_d0", Array("V20", "W20"), ioStandard = XiC_TDMS_33),
    XiDiffPin("hdmi_rx_d1", Array("T20", "U20"), ioStandard = XiC_TDMS_33),
    XiDiffPin("hdmi_rx_d2", Array("N20", "P20"), ioStandard = XiC_TDMS_33),
    XiPin("hdmi_rx_hpdn", "T19"),
    XiPin("hdmi_rx_scl", "U14"),
    XiPin("hdmi_rx_sda", "U15"),

    XiPinArray( name = "mio", 
                pins = Array("E6", 
                        "D8", 
                        "E9", 
                        "C6", 
                        "D9", 
                        "E8"
                ), 
                direction = XiC_BiDirection,
                pullup = true,
                slew = XiC_SlowSlew,
                drive = 8),

    XiPinArray( name = "mio18", 
                pins = Array( "B12", // 48 
                              "C12", 
                              "B13", 
                              "B9"  // 51
                        ), 
                ioStandard = XiC_LVCMOS18,
                direction = XiC_BiDirection,
                pullup = true,
                slew = XiC_SlowSlew,
                drive = 8),

    XiPinArray( name = "qspi0", 
                pins = qspi0.unzip3._1, 
                directions = qspi0.unzip3._2,
                pullups = qspi0.unzip3._3,
                slew = XiC_SlowSlew,
                drive = 8),

    XiPinArray( name = "uart0", 
                pinNames = uart0.unzip3._1,
                pins = uart0.unzip3._2, 
                directions = uart0.unzip3._3,
                pullup = true,
                slew = XiC_SlowSlew,
                drive = 8),

    XiPinArray( name = "enet0",
                pinNames = enet0.unzip3._1,
                pins = enet0.unzip3._2,
                directions = enet0.unzip3._3,
                ioStandard = XiC_LVCMOS18,
                pullup = true,
                drive = 8,
                slew = XiC_SlowSlew),

    XiPinArray( name = "usb0",
                pinNames = usb0.unzip3._1,
                pins = usb0.unzip3._2,
                directions = usb0.unzip3._3,
                ioStandard = XiC_LVCMOS18,
                pullup = true,
                drive = 8,
                slew = XiC_SlowSlew),

    XiPinArray( name = "sd0",
                pins = Array( "D14",  // 40 clk
                              "C17",  // cmd
                              "E12",  // data[0]
                              "A9",   // data[1]
                              "F13",  // data[2]
                              "B15",  // data[3]
                              "B14"),  // 47 cd
                direction = XiC_BiDirection,
                ioStandard = XiC_LVCMOS18,
                pullup = true,
                drive = 8,
                slew = XiC_SlowSlew
              ),
    XiPinArray( name = "DDR_DQ", 
                pins = Array( "C3", "B3", "A2", "A4", "D3", "D1", "C1", "E1", "E3", "E3",
                              "G3", "H3", "J3", "H2", "H1", "J1", "P1", "P3", "R3", "R1",
                              "T4", "U4", "U2", "U3", "V1", "Y3", "W1", "Y4", "Y2", "W3",
                              "V2", "V3"),
                ioStandard = XiC_SSTL15_T_DCI,
                slew = XiC_FastSlew,
                pullup = true,
                direction = XiC_BiDirection
                )
  )

}
