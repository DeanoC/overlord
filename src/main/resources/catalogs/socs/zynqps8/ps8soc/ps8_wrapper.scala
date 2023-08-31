import ikuy_utils._
import spinal.core._
import spinal.lib.bus.amba4.axi.Axi4
import spinal.lib.bus.amba4.axis.Axi4Stream
import spinal.lib.io.{TriState, TriStateArray, TriStateOutput}
import spinal.lib.{master, slave}

import java.nio.file.Path
import scala.language.postfixOps


// The Ps8 display core can be used in multiple ways (a few common case enumerated here)
// You can drive 1 or 2 of the planes from the PL (Live mode) but use PS timings
// psToPlDisplayPortTiming = true && plToPsDisplayPortVideo = true
// another the PL supply video and timings
// plToPsDisplayPortTiming = true && plToPsDisplayPortVideo = true
// you can drive audio from the PL
// plToPsDisplayPortAudio = true
// you can pipe the output from the PS DP chip to the PL
// video: psToPlDisplayPortVideo = true, audio: psToPlDisplayPortAudio = true
// there are also events and aux support if required
case class DisplayPortPsToPlTiming() extends Bundle {
	val hsync         = out Bool()
	val vsync         = out Bool()
	val displayEnable = out Bool()
}

case class ZynqMpPs8Config(psToPlAxiHpm0: Boolean = false,
                           psToPlAxiHpm1: Boolean = false,
                           psToPlAxiLpd: Boolean = false,
                           plToPsAxiHpc0: Boolean = false,
                           plToPsAxiHpc1: Boolean = false,
                           plToPsAxiHp0: Boolean = false,
                           plToPsAxiHp1: Boolean = false,
                           plToPsAxiHp2: Boolean = false,
                           plToPsAxiHp3: Boolean = false,
                           plToPsAxiLpd: Boolean = false,
                           plToPsAxiAcp: Boolean = false,
                           plToPsAxiAce: Boolean = false,
                           exposePlToPsAxiCounters: Boolean = false,
                           psToPlDisplayPortTiming: Boolean = false,
                           psToPlDisplayPortVideo: Boolean = false,
                           psToPlDisplayPortAudio: Boolean = false,
                           plToPsDisplayPortTiming: Boolean = false,
                           plToPsDisplayPortVideo: Boolean = false,
                           plToPsDisplayPortAudio: Boolean = false,
                           displayPortAux: Boolean = false,
                           displayPortEvents: Boolean = false,
                           gpio: Boolean = false,
                           plClocks: Array[Boolean] = Array[Boolean](false, false, false, false),
                           plClocksBuffered: Array[Boolean] = Array[Boolean](false, false, false, false),
                           i2c: Array[Boolean] = Array[Boolean](false, false),
                           uart: Array[Boolean] = Array[Boolean](false, false),
                           ttc: Array[Boolean] = Array[Boolean](false, false, false, false),
                           wdt: Array[Boolean] = Array[Boolean](false, false),
                           spi: Array[Boolean] = Array[Boolean](false, false),
                           sdio: Array[Boolean] = Array[Boolean](false, false),
                           can: Array[Boolean] = Array[Boolean](false, false),
                           trace: Boolean = false,
                           enetGemTsu: Boolean = false,
                           enetMdio: Array[Boolean] = Array[Boolean](false, false, false, false),
                           enetGmii: Array[Boolean] = Array[Boolean](false, false, false, false),
                           enetFifo: Array[Boolean] = Array[Boolean](false, false, false, false),
                           enetGemFmio: Array[Boolean] = Array[Boolean](false, false, false, false),
                           enetGem1588: Array[Boolean] = Array[Boolean](false, false, false, false),
                           enetGemMisc: Array[Boolean] = Array[Boolean](false, false, false, false),
                           usb: Array[Boolean] = Array[Boolean](false, false),
                          ) {}

class Axi4Counters extends Bundle {
	val read         = out UInt (8 bits)
	val write        = out UInt (8 bits)
	val addressRead  = out UInt (4 bits)
	val addressWrite = out UInt (4 bits)
}

case class AxiPsToPl() extends Bundle {
	val aclk = in Bool()
	val axi  = master(Axi4(PS8.PsToPLConfig))
}

case class AxiPlToPs(exposeCounters: Boolean) extends Bundle {
	val read_clk  = in Bool()
	val write_clk = in Bool()

	val counters = exposeCounters generate (new Axi4Counters)

	val axi = slave(Axi4(PS8.PlToPsConfig))
}

case class AxiPlToPsAcp() extends Bundle {
	val aclk = in Bool()
	val axi  = slave(Axi4(PS8.PlToPsAcpConfig))
}


case class DisplayPortPsToPlVideo() extends Bundle {
	val clk         = out Bool() // clk_ps_to_pl_dp_video
	val videoColour = out Bits (36 bits)
}

case class DisplayPortPsToPlAudio() extends Bundle {
	val clk         = out Bool() // clk_ps_to_pl_dp_audio
	val audioStream = master(Axi4Stream(PS8.AudioStreamConfig))
}

case class DisplayPortPlToPsTiming() extends Bundle {
	val hsync         = in Bool()
	val vsync         = in Bool()
	val displayEnable = in Bool()
}

case class DisplayPortPlToPsVideo() extends Bundle {
	val clk         = in Bool() // clk_pl_to_ps_dp_video_in
	val videoColour = in Bits (36 bits)
	val gfxAlpha    = in Bits (8 bits)
	val gfxColour   = in Bits (36 bits)
}

case class DisplayPortPlToPsAudio() extends Bundle {
	val clk         = in Bool() // clk_pl_to_ps_dp_s_axis_audio
	val audioStream = slave(Axi4Stream(PS8.AudioStreamConfig))
}

case class DisplayPortAux() extends Bundle {
	val dataIn             = in Bool()
	val dataOut            = out Bool()
	val dataOutputEnable_n = out Bool()
}

case class DisplayPortEvents() extends Bundle {
	val hotPlugDetect = in Bool()
	val customEvent0  = in Bool()
	val customEvent1  = in Bool()
	val vsyncEvent    = in Bool()
}

case class I2C() extends Bundle {
	val scl = master(TriState(Bool))
	val sda = master(TriState(Bool))
}

case class UART() extends Bundle {
	val tx    = out Bool()
	val rx    = in Bool()
	val cts_n = in Bool() // clear to send
	val rts_n = out Bool() // request to send
	val dsr_n = in Bool() // data set ready
	val dcd_n = in Bool() // date carrier detect
	val ri_n  = in Bool() // ring indicator
	val dtr_n = out Bool() // data terminal ready
}

case class TTC() extends Bundle {
	val clk  = in Bits (3 bits)
	val wave = out Bits (3 bits)
}

case class WDT() extends Bundle {
	val clk   = in Bool()
	val reset = out Bool()
}

case class SPI() extends Bundle {
	// in for a consumer
	// out for a supplier
	val sclk = master(TriState(Bool))

	// I = MISO master input
	// O = MOSI, master output
	val supplier = master(TriState(Bool))

	// I = MOSI slave input
	// O = MISO slave output
	val consumer = master(TriState(Bool))

	val selectIn  = in Bool()
	val selectOut = out Bits (3 bits)
	val selectT_n = out Bool()
}

case class SDIO() extends Bundle {
	val clkOut        = out Bool()
	val clkIn         = in Bool()
	val commandIn     = in Bool()
	val commandOut    = out Bool()
	val commandEnable = out Bool()
	val dataIn        = in Bits (8 bits)
	val dataOut       = out Bits (8 bits)
	val dataEnable    = out Bits (8 bits)
	val cardDetect    = in Bool()
	val writeProtect  = in Bool()
	val led           = out Bool()
	val busPower      = out Bool()
	val busVolt       = out Bits (3 bits)
}

case class CAN() extends Bundle {
	val phyTx = out Bool()
	val phyRx = in Bool()
}

case class PlClockDomain() extends Bundle {
	val clk   = out Bool()
	val rst_n = out Bool()
}

case class GPIO() extends Bundle {
	// there are 96 bits but 4 are reserved for reset lines
	val gpio = master(TriStateArray(92 bits))
}

case class TRACE() extends Bundle {
	val clk     = out Bool()
	val control = in Bool()
	val data    = in Bits (32 bits)
}

case class USB() extends Bundle {
	val hubPortOverCurrentUSB3 = in Bool()
	val hubPortOverCurrentUSB2 = in Bool()
	val u2dsVBusCtrl           = out Bool()
	val u3dsVBusCtrl           = out Bool()
}

case class ENET_GEMTSU() extends Bundle {
	val clockFromPl   = in Bool()
	val clockToPlBufG = out Bool()
	val clk           = in Bool()
	val timerCount    = out Bits (94 bits)
}

case class ENET_MDIO() extends Bundle {
	val mdc  = out Bool()
	val mdio = master(TriState(Bool))
}

case class ENET_GMII() extends Bundle {
	val receiveClk       = in Bool()
	val speedMode        = out Bits (3 bits)
	val carrierSend      = in Bool()
	val collisionDetect  = in Bool()
	val receiveData      = in Bits (8 bits)
	val receiveError     = in Bool()
	val receiveDataValid = in Bool()
	val transmitClk      = in Bool()
	val transmitData     = out Bits (8 bits)
	val transmitEnable   = out Bool()
	val transmitError    = out Bool()
}

case class ENET_FIFO() extends Bundle {
	val transmitDataReady       = in Bool()
	val transmitReady           = out Bool()
	val transmitValid           = in Bool()
	val transmitData            = in Bits (8 bits)
	val transmitStartOfPacket   = in Bool()
	val transmitEndOfPacket     = in Bool()
	val transmitError           = in Bool()
	val transmitUnderFlow       = in Bool()
	val transmitFlushed         = in Bool()
	val transmitControl         = in Bool()
	val transmitDmaEndToggle    = out Bool()
	val transmitDmaStatusToggle = in Bool()
	val transmitStatus          = out Bits (4 bits)
	val transmitFixedLat        = out Bool()

	val receiveReady         = out Bool()
	val receiveData          = out Bits (8 bits)
	val receiveStartOfPacket = out Bool()
	val receiveEndOfPacket   = out Bool()
	val receiveStatus        = out Bits (45 bits)
	val receiveError         = out Bool()
	val receiveOverflow      = out Bool()
	val receiveFlush         = out Bool()

	val signalDetect = in Bool()
}

case class ENET_GEMFMIO() extends Bundle {
	val transmitClockFromPl   = in Bool()
	val recieveClockFromPl    = in Bool()
	val transmitClockToPlBufG = out Bool()
	val receiveClockToPlBufG  = out Bool()
}

case class ENET_GEM1588() extends Bundle {
	val transmitStartOfFrame = out Bool()
	val transmitSyncFrame    = out Bool()
	val transmitDelayReq     = out Bool()
	val transmitPDelayReq    = out Bool()
	val transmitPDelayResp   = out Bool()

	val receiveStartOfFrame = out Bool()
	val receiveSyncFrame    = out Bool()
	val receiveDelayReq     = out Bool()
	val receivePDelayReq    = out Bool()
	val receivePDelayResp   = out Bool()

	val tsuIncControl        = in Bits (2 bits)
	val tsuTimerCompareValue = out Bool()
}

case class ENET_GEMMISC() extends Bundle {
	val externalIntIn = in Bool()
	val dmaBusWidth   = out Bits (2 bits)
}

case class ENET(config: ZynqMpPs8Config) extends Bundle {
	val mdio    = for (i <- 0 until 4) yield config.enetMdio(i) generate ENET_MDIO()
	val gmii    = for (i <- 0 until 4) yield config.enetGmii(i) generate ENET_GMII()
	val fifo    = for (i <- 0 until 4) yield config.enetFifo(i) generate ENET_FIFO()
	val gemFmio = for (i <- 0 until 4) yield config.enetGemFmio(i) generate ENET_GEMFMIO()
	val gem1588 = for (i <- 0 until 4) yield config.enetGem1588(i) generate ENET_GEM1588()
	val gemMisc = for (i <- 0 until 4) yield config.enetGemMisc(i) generate ENET_GEMMISC()
}

case class ZynqMpPs8(config: ZynqMpPs8Config) extends Component {
	val io = new Bundle {

		val psToPlAxiHpm0 = config.psToPlAxiHpm0 generate AxiPsToPl()
		val psToPlAxiHpm1 = config.psToPlAxiHpm1 generate AxiPsToPl()
		val psToPlAxiLpd  = config.psToPlAxiLpd generate AxiPsToPl()

		val plToPsAxiHpc0 = config.plToPsAxiHpc0 generate AxiPlToPs(config.exposePlToPsAxiCounters)
		val plToPsAxiHpc1 = config.plToPsAxiHpc1 generate AxiPlToPs(config.exposePlToPsAxiCounters)
		val plToPsAxiHp0  = config.plToPsAxiHp0 generate AxiPlToPs(config.exposePlToPsAxiCounters)
		val plToPsAxiHp1  = config.plToPsAxiHp1 generate AxiPlToPs(config.exposePlToPsAxiCounters)
		val plToPsAxiHp2  = config.plToPsAxiHp2 generate AxiPlToPs(config.exposePlToPsAxiCounters)
		val plToPsAxiHp3  = config.plToPsAxiHp3 generate AxiPlToPs(config.exposePlToPsAxiCounters)

		val plToPsAxiLpd = config.plToPsAxiLpd generate AxiPlToPs(config.exposePlToPsAxiCounters)
		val plToPsAxiAcp = config.plToPsAxiAcp generate AxiPlToPsAcp()
		//		val plToPsAxiAce = config.plToPsAxiAce generate AxiPlToPsAce

		val psToPlDisplayPortTiming = config.psToPlDisplayPortTiming generate DisplayPortPsToPlTiming()
		val psToPlDisplayPortVideo  = config.psToPlDisplayPortVideo generate DisplayPortPsToPlVideo()
		val psToPlDisplayPortAudio  = config.psToPlDisplayPortAudio generate DisplayPortPsToPlAudio()

		val plToPsDisplayPortTiming = config.plToPsDisplayPortTiming generate DisplayPortPlToPsTiming()
		val plToPsDisplayPortVideo  = config.plToPsDisplayPortVideo generate DisplayPortPlToPsVideo()
		val plToPsDisplayPortAudio  = config.plToPsDisplayPortAudio generate DisplayPortPlToPsAudio()

		val displayAux        = config.displayPortAux generate DisplayPortAux()
		val displayPortEvents = config.displayPortEvents generate DisplayPortEvents()

		val i2c  = for (i <- 0 until 2) yield config.i2c(i) generate I2C()
		val uart = for (i <- 0 until 2) yield config.uart(i) generate UART()
		val ttc  = for (i <- 0 until 4) yield config.ttc(i) generate TTC()
		val wdt  = for (i <- 0 until 2) yield config.wdt(i) generate WDT()
		val spi  = for (i <- 0 until 2) yield config.spi(i) generate SPI()
		val sdio = for (i <- 0 until 2) yield config.sdio(i) generate SDIO()
		val can  = for (i <- 0 until 2) yield config.can(i) generate CAN()
		val usb  = for (i <- 0 until 2) yield config.usb(i) generate USB()

		val plClocks = for (i <- 0 until 4) yield config.plClocks(i) generate PlClockDomain()

		val trace = config.trace generate TRACE()

		val gpio = config.gpio generate GPIO()

		val enetGemTsu = config.enetGemTsu generate ENET_GEMTSU()
		val enet       = ENET(config)

	}

	noIoPrefix()

	val rawPS8 = PS8()

	// ps to pl hookups
	if (config.psToPlAxiHpm0) connectMAXI(0, io.psToPlAxiHpm0) else capMAXI(0)
	if (config.psToPlAxiHpm1) connectMAXI(1, io.psToPlAxiHpm1) else capMAXI(1)
	if (config.psToPlAxiLpd) connectMAXI(2, io.psToPlAxiLpd) else capMAXI(2)

	// pl to ps hookups
	if (config.plToPsAxiHpc0) connectSAXI(0, io.plToPsAxiHpc0) else capSAXI(0)
	if (config.plToPsAxiHpc1) connectSAXI(1, io.plToPsAxiHpc1) else capSAXI(1)
	if (config.plToPsAxiHp0) connectSAXI(2, io.plToPsAxiHp0) else capSAXI(2)
	if (config.plToPsAxiHp1) connectSAXI(3, io.plToPsAxiHp1) else capSAXI(3)
	if (config.plToPsAxiHp2) connectSAXI(4, io.plToPsAxiHp2) else capSAXI(4)
	if (config.plToPsAxiHp3) connectSAXI(5, io.plToPsAxiHp3) else capSAXI(5)
	if (config.plToPsAxiLpd) connectSAXI(6, io.plToPsAxiLpd) else capSAXI(6)

	if (config.plToPsAxiAcp) connectSAXIACP(io.plToPsAxiAcp) else capSAXIACP()
	capSAXIACE()

	rawPS8.io.MISC.DDRCEXTREFRESHRANK0REQ := False
	rawPS8.io.MISC.DDRCEXTREFRESHRANK1REQ := False
	rawPS8.io.MISC.DDRCREFRESHPLCLK := False
	rawPS8.io.MISC.PLACPINACT := False

	if (config.gpio) {
		rawPS8.io.GPIO.I(0 until 92) := io.gpio.gpio.read
		io.gpio.gpio.write := rawPS8.io.GPIO.O(0 until 92)
		io.gpio.gpio.writeEnable := rawPS8.io.GPIO.TN(0 until 92)
	} else for (i <- 0 until 92) {
		rawPS8.io.GPIO.I(i) := False
		False := rawPS8.io.GPIO.O(i)
		False := rawPS8.io.GPIO.TN(i)
	}
	for (i <- config.plClocks.zipWithIndex) if (i._1) {
		if (config.plClocksBuffered(i._2)) {
			val bufg = BUFG_PS()
			bufg.io.I := rawPS8.io.PLCLK(i._2)
			io.plClocks(i._2).clk := bufg.io.O
		} else io.plClocks(i._2).clk := rawPS8.io.PLCLK(i._2)

		io.plClocks(i._2).rst_n := rawPS8.io.GPIO.O(i._2 + 92)
		rawPS8.io.GPIO.I(i._2 + 92) := False
		False := rawPS8.io.GPIO.TN(i._2 + 92)
	} else {
		False := rawPS8.io.PLCLK(i._2)
		False := rawPS8.io.GPIO.O(i._2 + 92)
		rawPS8.io.GPIO.I(i._2 + 92) := False
		False := rawPS8.io.GPIO.TN(i._2 + 92)

	}
	capPSS_ALTOC_CORE_PAD()

	connectDisplayPort()

	connectI2C()

	connectUART()

	connectTTC()

	connectWTD()

	connectSPI()

	connectSDIO()

	connectCAN()

	connectTRACE()

	connectENET()

	connectUSB()

	private def capMAXI(i: Integer): Unit = {
		rawPS8.io.MAXIGP(i).ACLK := False

		0 := rawPS8.io.MAXIGP(i).AWID
		0 := rawPS8.io.MAXIGP(i).AWADDR
		0 := rawPS8.io.MAXIGP(i).AWLEN
		0 := rawPS8.io.MAXIGP(i).AWSIZE
		0 := rawPS8.io.MAXIGP(i).AWBURST
		0 := rawPS8.io.MAXIGP(i).AWLOCK
		0 := rawPS8.io.MAXIGP(i).AWCACHE
		0 := rawPS8.io.MAXIGP(i).AWPROT
		False := rawPS8.io.MAXIGP(i).AWVALID
		0 := rawPS8.io.MAXIGP(i).AWUSER
		rawPS8.io.MAXIGP(i).AWREADY := False
		0 := rawPS8.io.MAXIGP(i).AWQOS

		0 := rawPS8.io.MAXIGP(i).WDATA
		0 := rawPS8.io.MAXIGP(i).WSTRB
		False := rawPS8.io.MAXIGP(i).WLAST
		False := rawPS8.io.MAXIGP(i).WVALID
		rawPS8.io.MAXIGP(i).WREADY := False

		rawPS8.io.MAXIGP(i).BID := 0
		rawPS8.io.MAXIGP(i).BRESP := 0
		rawPS8.io.MAXIGP(i).BVALID := False
		False := rawPS8.io.MAXIGP(i).BREADY

		0 := rawPS8.io.MAXIGP(i).ARID
		0 := rawPS8.io.MAXIGP(i).ARADDR
		0 := rawPS8.io.MAXIGP(i).ARLEN
		0 := rawPS8.io.MAXIGP(i).ARSIZE
		0 := rawPS8.io.MAXIGP(i).ARBURST
		0 := rawPS8.io.MAXIGP(i).ARLOCK
		0 := rawPS8.io.MAXIGP(i).ARCACHE
		0 := rawPS8.io.MAXIGP(i).ARPROT
		False := rawPS8.io.MAXIGP(i).ARVALID
		0 := rawPS8.io.MAXIGP(i).ARUSER
		rawPS8.io.MAXIGP(i).ARREADY := False
		0 := rawPS8.io.MAXIGP(i).ARQOS

		rawPS8.io.MAXIGP(i).RID := 0
		rawPS8.io.MAXIGP(i).RDATA := 0
		rawPS8.io.MAXIGP(i).RRESP := 0
		rawPS8.io.MAXIGP(i).RLAST := False
		rawPS8.io.MAXIGP(i).RVALID := False
		False := rawPS8.io.MAXIGP(i).RREADY
	}

	private def capSAXI(i: Integer): Unit = {
		rawPS8.io.SAXIGP(i).RCLK := False
		rawPS8.io.SAXIGP(i).WCLK := False

		rawPS8.io.SAXIGP(i).AWID := 0
		rawPS8.io.SAXIGP(i).AWADDR := 0
		rawPS8.io.SAXIGP(i).AWLEN := 0
		rawPS8.io.SAXIGP(i).AWSIZE := 0
		rawPS8.io.SAXIGP(i).AWBURST := 0
		rawPS8.io.SAXIGP(i).AWLOCK := 0
		rawPS8.io.SAXIGP(i).AWCACHE := 0
		rawPS8.io.SAXIGP(i).AWPROT := 0
		rawPS8.io.SAXIGP(i).AWVALID := False
		rawPS8.io.SAXIGP(i).AWQOS := 0
		rawPS8.io.SAXIGP(i).AWUSER := 0

		rawPS8.io.SAXIGP(i).WDATA := 0
		rawPS8.io.SAXIGP(i).WSTRB := 0
		rawPS8.io.SAXIGP(i).WLAST := False
		rawPS8.io.SAXIGP(i).WVALID := False

		rawPS8.io.SAXIGP(i).BREADY := False

		rawPS8.io.SAXIGP(i).ARID := 0
		rawPS8.io.SAXIGP(i).ARADDR := 0
		rawPS8.io.SAXIGP(i).ARLEN := 0
		rawPS8.io.SAXIGP(i).ARSIZE := 0
		rawPS8.io.SAXIGP(i).ARBURST := 0
		rawPS8.io.SAXIGP(i).ARLOCK := 0
		rawPS8.io.SAXIGP(i).ARCACHE := 0
		rawPS8.io.SAXIGP(i).ARPROT := 0
		rawPS8.io.SAXIGP(i).ARVALID := False
		rawPS8.io.SAXIGP(i).ARQOS := 0
		rawPS8.io.SAXIGP(i).ARUSER := 0

		rawPS8.io.SAXIGP(i).RREADY := False

	}

	private def capSAXIACP(): Unit = {
		rawPS8.io.SAXIACP.ACLK := False

		rawPS8.io.SAXIACP.AWID := 0
		rawPS8.io.SAXIACP.AWADDR := 0
		rawPS8.io.SAXIACP.AWLEN := 0
		rawPS8.io.SAXIACP.AWSIZE := 0
		rawPS8.io.SAXIACP.AWBURST := 0
		rawPS8.io.SAXIACP.AWLOCK := 0
		rawPS8.io.SAXIACP.AWCACHE := 0
		rawPS8.io.SAXIACP.AWPROT := 0
		rawPS8.io.SAXIACP.AWVALID := False
		rawPS8.io.SAXIACP.AWQOS := 0
		rawPS8.io.SAXIACP.AWUSER := 0

		rawPS8.io.SAXIACP.WDATA := 0
		rawPS8.io.SAXIACP.WSTRB := 0
		rawPS8.io.SAXIACP.WLAST := False
		rawPS8.io.SAXIACP.WVALID := False

		rawPS8.io.SAXIACP.BREADY := False

		rawPS8.io.SAXIACP.ARID := 0
		rawPS8.io.SAXIACP.ARADDR := 0
		rawPS8.io.SAXIACP.ARLEN := 0
		rawPS8.io.SAXIACP.ARSIZE := 0
		rawPS8.io.SAXIACP.ARBURST := 0
		rawPS8.io.SAXIACP.ARLOCK := 0
		rawPS8.io.SAXIACP.ARCACHE := 0
		rawPS8.io.SAXIACP.ARPROT := 0
		rawPS8.io.SAXIACP.ARVALID := False
		rawPS8.io.SAXIACP.ARQOS := 0
		rawPS8.io.SAXIACP.ARUSER := 0

		rawPS8.io.SAXIACP.RREADY := False
	}

	private def capSAXIACE(): Unit = {
		rawPS8.io.SAXIACE.ACLK := False

		rawPS8.io.SAXIACE.AWID := 0
		rawPS8.io.SAXIACE.AWADDR := 0
		rawPS8.io.SAXIACE.AWLEN := 0
		rawPS8.io.SAXIACE.AWSIZE := 0
		rawPS8.io.SAXIACE.AWBURST := 0
		rawPS8.io.SAXIACE.AWLOCK := 0
		rawPS8.io.SAXIACE.AWCACHE := 0
		rawPS8.io.SAXIACE.AWPROT := 0
		rawPS8.io.SAXIACE.AWREGION := 0
		rawPS8.io.SAXIACE.AWVALID := False
		False := rawPS8.io.SAXIACE.AWREADY
		rawPS8.io.SAXIACE.AWQOS := 0
		rawPS8.io.SAXIACE.AWDOMAIN := 0
		rawPS8.io.SAXIACE.AWSNOOP := 0
		rawPS8.io.SAXIACE.AWBAR := 0

		rawPS8.io.SAXIACE.WDATA := 0
		rawPS8.io.SAXIACE.WSTRB := 0
		rawPS8.io.SAXIACE.WLAST := False
		rawPS8.io.SAXIACE.WVALID := False
		False := rawPS8.io.SAXIACE.WREADY
		rawPS8.io.SAXIACE.WUSER := 0

		0 := rawPS8.io.SAXIACE.BID
		0 := rawPS8.io.SAXIACE.BRESP
		False := rawPS8.io.SAXIACE.BVALID
		rawPS8.io.SAXIACE.BREADY := False
		rawPS8.io.SAXIACE.BUSER := 0

		rawPS8.io.SAXIACE.ARID := 0
		rawPS8.io.SAXIACE.ARADDR := 0
		rawPS8.io.SAXIACE.ARLEN := 0
		rawPS8.io.SAXIACE.ARSIZE := 0
		rawPS8.io.SAXIACE.ARBURST := 0
		rawPS8.io.SAXIACE.ARLOCK := 0
		rawPS8.io.SAXIACE.ARCACHE := 0
		rawPS8.io.SAXIACE.ARPROT := 0
		rawPS8.io.SAXIACE.ARREGION := 0
		rawPS8.io.SAXIACE.ARVALID := False
		False := rawPS8.io.SAXIACE.ARREADY
		rawPS8.io.SAXIACE.ARQOS := 0
		rawPS8.io.SAXIACE.ARDOMAIN := 0
		rawPS8.io.SAXIACE.ARSNOOP := 0
		rawPS8.io.SAXIACE.ARBAR := 0

		0 := rawPS8.io.SAXIACE.RID
		0 := rawPS8.io.SAXIACE.RRESP
		False := rawPS8.io.SAXIACE.RVALID
		rawPS8.io.SAXIACE.RREADY := False
		0 := rawPS8.io.SAXIACE.RUSER

		False := rawPS8.io.SAXIACE.ACVALID
		rawPS8.io.SAXIACE.ACREADY := False
		0 := rawPS8.io.SAXIACE.ACADDR
		0 := rawPS8.io.SAXIACE.ACSNOOP
		0 := rawPS8.io.SAXIACE.ACPROT
		rawPS8.io.SAXIACE.CRVALID := False
		False := rawPS8.io.SAXIACE.CRREADY
		rawPS8.io.SAXIACE.CRRESP := 0
		rawPS8.io.SAXIACE.CDVALID := False
		False := rawPS8.io.SAXIACE.CDREADY
		rawPS8.io.SAXIACE.CDDATA := 0
		rawPS8.io.SAXIACE.CDLAST := False
		rawPS8.io.SAXIACE.WACK := False
		False := rawPS8.io.SAXIACE.RACK
	}

	private def connectMAXI(i: Int, bundle: AxiPsToPl): Unit = {
		rawPS8.io.MAXIGP(i).ACLK := bundle.aclk

		val axi = bundle.axi

		axi.aw.id <> rawPS8.io.MAXIGP(i).AWID
		axi.aw.addr <> rawPS8.io.MAXIGP(i).AWADDR
		axi.aw.len <> rawPS8.io.MAXIGP(i).AWLEN
		axi.aw.size <> rawPS8.io.MAXIGP(i).AWSIZE
		axi.aw.burst <> rawPS8.io.MAXIGP(i).AWBURST
		axi.aw.lock <> rawPS8.io.MAXIGP(i).AWLOCK
		axi.aw.cache <> rawPS8.io.MAXIGP(i).AWCACHE
		axi.aw.prot <> rawPS8.io.MAXIGP(i).AWPROT
		axi.aw.valid <> rawPS8.io.MAXIGP(i).AWVALID
		axi.aw.user <> rawPS8.io.MAXIGP(i).AWUSER
		axi.aw.ready <> rawPS8.io.MAXIGP(i).AWREADY
		axi.aw.qos <> rawPS8.io.MAXIGP(i).AWQOS

		axi.w.data <> rawPS8.io.MAXIGP(i).WDATA
		axi.w.strb <> rawPS8.io.MAXIGP(i).WSTRB
		axi.w.last <> rawPS8.io.MAXIGP(i).WLAST
		axi.w.valid <> rawPS8.io.MAXIGP(i).WVALID
		axi.w.ready <> rawPS8.io.MAXIGP(i).WREADY

		axi.b.id <> rawPS8.io.MAXIGP(i).BID
		axi.b.resp <> rawPS8.io.MAXIGP(i).BRESP
		axi.b.valid <> rawPS8.io.MAXIGP(i).BVALID
		axi.b.ready <> rawPS8.io.MAXIGP(i).BREADY

		axi.ar.id <> rawPS8.io.MAXIGP(i).ARID
		axi.ar.addr <> rawPS8.io.MAXIGP(i).ARADDR
		axi.ar.len <> rawPS8.io.MAXIGP(i).ARLEN
		axi.ar.size <> rawPS8.io.MAXIGP(i).ARSIZE
		axi.ar.burst <> rawPS8.io.MAXIGP(i).ARBURST
		axi.ar.lock <> rawPS8.io.MAXIGP(i).ARLOCK
		axi.ar.cache <> rawPS8.io.MAXIGP(i).ARCACHE
		axi.ar.prot <> rawPS8.io.MAXIGP(i).ARPROT
		axi.ar.valid <> rawPS8.io.MAXIGP(i).ARVALID
		axi.ar.user <> rawPS8.io.MAXIGP(i).ARUSER
		axi.ar.ready <> rawPS8.io.MAXIGP(i).ARREADY
		axi.ar.qos <> rawPS8.io.MAXIGP(i).ARQOS

		axi.r.id <> rawPS8.io.MAXIGP(i).RID
		axi.r.data <> rawPS8.io.MAXIGP(i).RDATA
		axi.r.resp <> rawPS8.io.MAXIGP(i).RRESP
		axi.r.last <> rawPS8.io.MAXIGP(i).RLAST
		axi.r.valid <> rawPS8.io.MAXIGP(i).RVALID
		axi.r.ready <> rawPS8.io.MAXIGP(i).RREADY
	}

	private def connectSAXI(i: Int, bundle: AxiPlToPs): Unit = {
		rawPS8.io.SAXIGP(i).RCLK := bundle.read_clk
		rawPS8.io.SAXIGP(i).WCLK := bundle.write_clk

		val axi = bundle.axi

		axi.aw.id <> rawPS8.io.SAXIGP(i).AWID
		axi.aw.addr <> rawPS8.io.SAXIGP(i).AWADDR
		axi.aw.len <> rawPS8.io.SAXIGP(i).AWLEN
		axi.aw.size <> rawPS8.io.SAXIGP(i).AWSIZE
		axi.aw.burst <> rawPS8.io.SAXIGP(i).AWBURST
		axi.aw.lock <> rawPS8.io.SAXIGP(i).AWLOCK
		axi.aw.cache <> rawPS8.io.SAXIGP(i).AWCACHE
		axi.aw.prot <> rawPS8.io.SAXIGP(i).AWPROT
		axi.aw.valid <> rawPS8.io.SAXIGP(i).AWVALID
		axi.aw.ready <> rawPS8.io.SAXIGP(i).AWREADY
		axi.aw.qos <> rawPS8.io.SAXIGP(i).AWQOS
		axi.aw.user := rawPS8.io.SAXIGP(i).AWUSER

		axi.w.data <> rawPS8.io.SAXIGP(i).WDATA
		axi.w.strb <> rawPS8.io.SAXIGP(i).WSTRB
		axi.w.last <> rawPS8.io.SAXIGP(i).WLAST
		axi.w.valid <> rawPS8.io.SAXIGP(i).WVALID
		axi.w.ready <> rawPS8.io.SAXIGP(i).WREADY

		axi.b.id <> rawPS8.io.SAXIGP(i).BID
		axi.b.resp <> rawPS8.io.SAXIGP(i).BRESP
		axi.b.valid <> rawPS8.io.SAXIGP(i).BVALID
		axi.b.ready <> rawPS8.io.SAXIGP(i).BREADY

		axi.ar.id <> rawPS8.io.SAXIGP(i).ARID
		axi.ar.addr <> rawPS8.io.SAXIGP(i).ARADDR
		axi.ar.len <> rawPS8.io.SAXIGP(i).ARLEN
		axi.ar.size <> rawPS8.io.SAXIGP(i).ARSIZE
		axi.ar.burst <> rawPS8.io.SAXIGP(i).ARBURST
		axi.ar.lock <> rawPS8.io.SAXIGP(i).ARLOCK
		axi.ar.cache <> rawPS8.io.SAXIGP(i).ARCACHE
		axi.ar.prot <> rawPS8.io.SAXIGP(i).ARPROT
		axi.ar.valid <> rawPS8.io.SAXIGP(i).ARVALID
		axi.ar.ready <> rawPS8.io.SAXIGP(i).ARREADY
		axi.ar.qos <> rawPS8.io.SAXIGP(i).ARQOS
		axi.ar.user := rawPS8.io.SAXIGP(i).ARUSER

		axi.r.id <> rawPS8.io.SAXIGP(i).RID
		axi.r.data <> rawPS8.io.SAXIGP(i).RDATA
		axi.r.resp <> rawPS8.io.SAXIGP(i).RRESP
		axi.r.last <> rawPS8.io.SAXIGP(i).RLAST
		axi.r.valid <> rawPS8.io.SAXIGP(i).RVALID
		axi.r.ready <> rawPS8.io.SAXIGP(i).RREADY

		if (bundle.exposeCounters) {
			bundle.counters.read <> rawPS8.io.SAXIGP(i).RCOUNT
			bundle.counters.write <> rawPS8.io.SAXIGP(i).WCOUNT
			bundle.counters.addressRead <> rawPS8.io.SAXIGP(i).ARCOUNT
			bundle.counters.addressWrite <> rawPS8.io.SAXIGP(i).AWCOUNT
		}
	}

	private def connectSAXIACP(bundle: AxiPlToPsAcp): Unit = {
		rawPS8.io.SAXIACP.ACLK := bundle.aclk

		val axi = bundle.axi
		axi.aw.id <> rawPS8.io.SAXIACP.AWID
		axi.aw.addr <> rawPS8.io.SAXIACP.AWADDR
		axi.aw.len <> rawPS8.io.SAXIACP.AWLEN
		axi.aw.size <> rawPS8.io.SAXIACP.AWSIZE
		axi.aw.burst <> rawPS8.io.SAXIACP.AWBURST
		axi.aw.lock <> rawPS8.io.SAXIACP.AWLOCK
		axi.aw.cache <> rawPS8.io.SAXIACP.AWCACHE
		axi.aw.prot <> rawPS8.io.SAXIACP.AWPROT
		axi.aw.valid <> rawPS8.io.SAXIACP.AWVALID
		axi.aw.ready <> rawPS8.io.SAXIACP.AWREADY
		axi.aw.qos <> rawPS8.io.SAXIACP.AWQOS
		axi.aw.user <> rawPS8.io.SAXIACP.AWUSER

		axi.w.data <> rawPS8.io.SAXIACP.WDATA
		axi.w.strb <> rawPS8.io.SAXIACP.WSTRB
		axi.w.last <> rawPS8.io.SAXIACP.WLAST
		axi.w.valid <> rawPS8.io.SAXIACP.WVALID
		axi.w.ready <> rawPS8.io.SAXIACP.WREADY

		axi.b.id <> rawPS8.io.SAXIACP.BID
		axi.b.resp <> rawPS8.io.SAXIACP.BRESP
		axi.b.valid <> rawPS8.io.SAXIACP.BVALID
		axi.b.ready <> rawPS8.io.SAXIACP.BREADY

		axi.ar.id <> rawPS8.io.SAXIACP.ARID
		axi.ar.addr <> rawPS8.io.SAXIACP.ARADDR
		axi.ar.len <> rawPS8.io.SAXIACP.ARLEN
		axi.ar.size <> rawPS8.io.SAXIACP.ARSIZE
		axi.ar.burst <> rawPS8.io.SAXIACP.ARBURST
		axi.ar.lock <> rawPS8.io.SAXIACP.ARLOCK
		axi.ar.cache <> rawPS8.io.SAXIACP.ARCACHE
		axi.ar.prot <> rawPS8.io.SAXIACP.ARPROT
		axi.ar.valid <> rawPS8.io.SAXIACP.ARVALID
		axi.ar.ready <> rawPS8.io.SAXIACP.ARREADY
		axi.ar.qos <> rawPS8.io.SAXIACP.ARQOS
		axi.ar.user <> rawPS8.io.SAXIACP.AWUSER

		axi.r.id <> rawPS8.io.SAXIACP.RID
		axi.r.data <> rawPS8.io.SAXIACP.RDATA
		axi.r.resp <> rawPS8.io.SAXIACP.RRESP
		axi.r.last <> rawPS8.io.SAXIACP.RLAST
		axi.r.valid <> rawPS8.io.SAXIACP.RVALID
		axi.r.ready <> rawPS8.io.SAXIACP.RREADY
	}

	private def connectSAXIACE(bundle: AxiPlToPsAcp): Unit = ???

	private def connectDisplayPort(): Unit = {
		if (config.psToPlDisplayPortTiming) {
			io.psToPlDisplayPortTiming.vsync := rawPS8.io.DP.VIDEOOUTVSYNC
			io.psToPlDisplayPortTiming.hsync := rawPS8.io.DP.VIDEOOUTHSYNC
			io.psToPlDisplayPortTiming.displayEnable := rawPS8.io.DP.LIVEVIDEODEOUT
		} else {
			False := rawPS8.io.DP.VIDEOOUTVSYNC
			False := rawPS8.io.DP.VIDEOOUTHSYNC
			False := rawPS8.io.DP.LIVEVIDEODEOUT
		}
		if (config.psToPlDisplayPortVideo) {
			val bufg = BUFG_PS()
			io.psToPlDisplayPortVideo.clk := bufg.io.O
			bufg.io.I := rawPS8.io.DP.VIDEOREFCLK
			io.psToPlDisplayPortVideo.videoColour := rawPS8.io.DP.VIDEOOUTPIXEL1
		} else {
			False := rawPS8.io.DP.VIDEOREFCLK
			0 := rawPS8.io.DP.VIDEOOUTPIXEL1
		}
		if (config.psToPlDisplayPortAudio) {
			val bufg = BUFG_PS()
			io.psToPlDisplayPortAudio.clk := bufg.io.O
			bufg.io.I := rawPS8.io.DP.AUDIOREFCLK

			io.psToPlDisplayPortAudio.audioStream.payload.data := rawPS8.io.DP.MAXISMIXEDAUDIOTDATA
			io.psToPlDisplayPortAudio.audioStream.payload.id := rawPS8.io.DP.MAXISMIXEDAUDIOTID
			io.psToPlDisplayPortAudio.audioStream.valid := rawPS8.io.DP.MAXISMIXEDAUDIOTVALID
			rawPS8.io.DP.MAXISMIXEDAUDIOTREADY := io.psToPlDisplayPortAudio.audioStream.ready
		} else {
			False := rawPS8.io.DP.AUDIOREFCLK
			0 := rawPS8.io.DP.MAXISMIXEDAUDIOTDATA
			0 := rawPS8.io.DP.MAXISMIXEDAUDIOTID
			False := rawPS8.io.DP.MAXISMIXEDAUDIOTVALID
			rawPS8.io.DP.MAXISMIXEDAUDIOTREADY := False
		}

		if (config.plToPsDisplayPortTiming) {
			rawPS8.io.DP.LIVEVIDEOINHSYNC := io.plToPsDisplayPortTiming.hsync
			rawPS8.io.DP.LIVEVIDEOINVSYNC := io.plToPsDisplayPortTiming.vsync
			rawPS8.io.DP.LIVEVIDEOINDE := io.plToPsDisplayPortTiming.displayEnable
		} else {
			rawPS8.io.DP.LIVEVIDEOINHSYNC := False
			rawPS8.io.DP.LIVEVIDEOINVSYNC := False
			rawPS8.io.DP.LIVEVIDEOINDE := False
		}

		if (config.plToPsDisplayPortVideo) {
			rawPS8.io.DP.VIDEOINCLK := io.plToPsDisplayPortVideo.clk
			rawPS8.io.DP.LIVEVIDEOINPIXEL1 := io.plToPsDisplayPortVideo.videoColour
			rawPS8.io.DP.LIVEGFXALPHAIN := io.plToPsDisplayPortVideo.gfxAlpha
			rawPS8.io.DP.LIVEGFXPIXEL1IN := io.plToPsDisplayPortVideo.gfxColour
		} else {
			rawPS8.io.DP.VIDEOINCLK := False
			rawPS8.io.DP.LIVEVIDEOINPIXEL1 := 0
			rawPS8.io.DP.LIVEGFXALPHAIN := 0
			rawPS8.io.DP.LIVEGFXPIXEL1IN := 0
		}
		if (config.plToPsDisplayPortAudio) {
			rawPS8.io.DP.SAXISAUDIOCLK := io.plToPsDisplayPortAudio.clk
			rawPS8.io.DP.SAXISAUDIOTDATA := io.plToPsDisplayPortAudio.audioStream.data
			rawPS8.io.DP.SAXISAUDIOTID := io.plToPsDisplayPortAudio.audioStream.id
			rawPS8.io.DP.SAXISAUDIOTVALID := io.plToPsDisplayPortAudio.audioStream.valid
			io.plToPsDisplayPortAudio.audioStream.ready := rawPS8.io.DP.SAXISAUDIOTREADY

		} else {
			rawPS8.io.DP.SAXISAUDIOCLK := False
			rawPS8.io.DP.SAXISAUDIOTDATA := 0
			rawPS8.io.DP.SAXISAUDIOTID := 0
			rawPS8.io.DP.SAXISAUDIOTVALID := False
			False := rawPS8.io.DP.SAXISAUDIOTREADY
		}
		if (config.displayPortAux) {
			rawPS8.io.DP.AUXDATAIN := io.displayAux.dataIn
			io.displayAux.dataOut := rawPS8.io.DP.AUXDATAOUT
			io.displayAux.dataOutputEnable_n := rawPS8.io.DP.AUXDATAOEN
		} else {
			rawPS8.io.DP.AUXDATAIN := False
			False := rawPS8.io.DP.AUXDATAOUT
			False := rawPS8.io.DP.AUXDATAOEN
		}
		if (config.displayPortEvents) {
			rawPS8.io.DP.HOTPLUGDETECT := io.displayPortEvents.hotPlugDetect
			rawPS8.io.DP.EXTERNALCUSTOMEVENT1 := io.displayPortEvents.customEvent0
			rawPS8.io.DP.EXTERNALCUSTOMEVENT2 := io.displayPortEvents.customEvent1
			rawPS8.io.DP.EXTERNALVSYNCEVENT := io.displayPortEvents.vsyncEvent
		} else {
			rawPS8.io.DP.HOTPLUGDETECT := False
			rawPS8.io.DP.EXTERNALCUSTOMEVENT1 := False
			rawPS8.io.DP.EXTERNALCUSTOMEVENT2 := False
			rawPS8.io.DP.EXTERNALVSYNCEVENT := False
		}
	}

	private def connectI2C(): Unit = {
		for (i <- config.i2c.zipWithIndex) if (i._1) {
			rawPS8.io.I2C(i._2).SCLI := io.i2c(i._2).scl.read
			io.i2c(i._2).scl.write := rawPS8.io.I2C(i._2).SCLO
			io.i2c(i._2).scl.writeEnable := rawPS8.io.I2C(i._2).SCLTN
			rawPS8.io.I2C(i._2).SDAI := io.i2c(i._2).sda.read
			io.i2c(i._2).sda.write := rawPS8.io.I2C(i._2).SDAO
			io.i2c(i._2).sda.writeEnable := rawPS8.io.I2C(i._2).SDATN
		} else {
			rawPS8.io.I2C(i._2).SCLI := False
			False := rawPS8.io.I2C(i._2).SCLO
			False := rawPS8.io.I2C(i._2).SCLTN
			rawPS8.io.I2C(i._2).SDAI := False
			False := rawPS8.io.I2C(i._2).SDAO
			False := rawPS8.io.I2C(i._2).SDATN
		}
	}

	private def connectUART(): Unit = {
		for (i <- config.uart.zipWithIndex) if (i._1) {
			io.uart(i._2).tx := rawPS8.io.UART(i._2).TX
			rawPS8.io.UART(i._2).RX := io.uart(i._2).rx
			rawPS8.io.UART(i._2).CTSN := io.uart(i._2).cts_n
			io.uart(i._2).rts_n := rawPS8.io.UART(i._2).RTSN
			rawPS8.io.UART(i._2).DSRN := io.uart(i._2).dsr_n
			rawPS8.io.UART(i._2).DCDN := io.uart(i._2).dcd_n
			rawPS8.io.UART(i._2).RIN := io.uart(i._2).ri_n
			io.uart(i._2).dtr_n := rawPS8.io.UART(i._2).DTRN
		} else {
			False := rawPS8.io.UART(i._2).TX
			rawPS8.io.UART(i._2).RX := False
			rawPS8.io.UART(i._2).CTSN := False
			False := rawPS8.io.UART(i._2).RTSN
			rawPS8.io.UART(i._2).DSRN := False
			rawPS8.io.UART(i._2).DCDN := False
			rawPS8.io.UART(i._2).RIN := False
			False := rawPS8.io.UART(i._2).DTRN
		}
	}

	private def connectTTC(): Unit = {
		for (i <- config.ttc.zipWithIndex) if (i._1) {
			rawPS8.io.TTC(i._2).CLKI := io.ttc(i._2).clk
			io.ttc(i._2).wave := rawPS8.io.TTC(i._2).WAVEO
		} else {
			rawPS8.io.TTC(i._2).CLKI := Bits(3 bits).setAllTo(False)
			0 := rawPS8.io.TTC(i._2).WAVEO
		}
	}

	private def connectWTD(): Unit = {
		for (i <- config.wdt.zipWithIndex) if (i._1) {
			rawPS8.io.WDT(i._2).CLKI := io.wdt(i._2).clk
			io.wdt(i._2).reset := rawPS8.io.WDT(i._2).RSTO
		} else {
			rawPS8.io.WDT(i._2).CLKI := False
			False := rawPS8.io.WDT(i._2).RSTO
		}
	}

	private def connectSPI(): Unit = {
		for (i <- config.spi.zipWithIndex) if (i._1) {
			rawPS8.io.SPI(i._2).SCLKI := io.spi(i._2).sclk.read
			io.spi(i._2).sclk.write := rawPS8.io.SPI(i._2).SCLKO
			io.spi(i._2).sclk.writeEnable := rawPS8.io.SPI(i._2).SCLKTN
			rawPS8.io.SPI(i._2).MI := io.spi(i._2).supplier.read
			io.spi(i._2).supplier.write := rawPS8.io.SPI(i._2).MO
			io.spi(i._2).supplier.writeEnable := rawPS8.io.SPI(i._2).MOTN
			rawPS8.io.SPI(i._2).SI := io.spi(i._2).consumer.read
			io.spi(i._2).consumer.write := rawPS8.io.SPI(i._2).SO
			io.spi(i._2).consumer.writeEnable := rawPS8.io.SPI(i._2).STN
			rawPS8.io.SPI(i._2).SSIN := io.spi(i._2).selectIn
			io.spi(i._2).selectOut := rawPS8.io.SPI(i._2).SSON
			io.spi(i._2).selectT_n := rawPS8.io.SPI(i._2).SSNTN
		} else {
			rawPS8.io.SPI(i._2).SCLKI := False
			False := rawPS8.io.SPI(i._2).SCLKO
			False := rawPS8.io.SPI(i._2).SCLKTN
			rawPS8.io.SPI(i._2).MI := False
			False := rawPS8.io.SPI(i._2).MO
			False := rawPS8.io.SPI(i._2).MOTN
			rawPS8.io.SPI(i._2).SI := False
			False := rawPS8.io.SPI(i._2).SO
			False := rawPS8.io.SPI(i._2).STN
			rawPS8.io.SPI(i._2).SSIN := False
			0 := rawPS8.io.SPI(i._2).SSON
			False := rawPS8.io.SPI(i._2).SSNTN
		}
	}


	private def connectSDIO(): Unit = {
		for (i <- config.sdio.zipWithIndex) if (i._1) {
			io.sdio(i._2).clkOut := rawPS8.io.SDIO(i._2).CLKOUT
			rawPS8.io.SDIO(i._2).FBCLKIN := io.sdio(i._2).clkIn
			io.sdio(i._2).commandOut := rawPS8.io.SDIO(i._2).CMDOUT
			rawPS8.io.SDIO(i._2).CMDIN := io.sdio(i._2).commandIn
			io.sdio(i._2).commandEnable := rawPS8.io.SDIO(i._2).CMDENA
			rawPS8.io.SDIO(i._2).DATAIN := io.sdio(i._2).dataIn
			io.sdio(i._2).dataOut := rawPS8.io.SDIO(i._2).DATAOUT
			io.sdio(i._2).dataEnable := rawPS8.io.SDIO(i._2).DATAENA
			rawPS8.io.SDIO(i._2).CDN := io.sdio(i._2).cardDetect
			rawPS8.io.SDIO(i._2).WP := io.sdio(i._2).writeProtect
			io.sdio(i._2).led := rawPS8.io.SDIO(i._2).LEDCONTROL
			io.sdio(i._2).busPower := rawPS8.io.SDIO(i._2).BUSPOWER
			io.sdio(i._2).busVolt := rawPS8.io.SDIO(i._2).BUSVOLT
		} else {
			False := rawPS8.io.SDIO(i._2).CLKOUT
			rawPS8.io.SDIO(i._2).FBCLKIN := False
			False := rawPS8.io.SDIO(i._2).CMDOUT
			rawPS8.io.SDIO(i._2).CMDIN := False
			False := rawPS8.io.SDIO(i._2).CMDENA
			rawPS8.io.SDIO(i._2).DATAIN := 0
			0 := rawPS8.io.SDIO(i._2).DATAOUT
			0 := rawPS8.io.SDIO(i._2).DATAENA
			rawPS8.io.SDIO(i._2).CDN := False
			rawPS8.io.SDIO(i._2).WP := False
			False := rawPS8.io.SDIO(i._2).LEDCONTROL
			False := rawPS8.io.SDIO(i._2).BUSPOWER
			0 := rawPS8.io.SDIO(i._2).BUSVOLT
		}
	}

	private def connectCAN(): Unit = {
		for (i <- config.can.zipWithIndex) if (i._1) {
			io.can(i._2).phyTx := rawPS8.io.CAN(i._2).PHYTX
			rawPS8.io.CAN(i._2).PHYRX := io.can(i._2).phyRx
		} else {
			False := rawPS8.io.CAN(i._2).PHYTX
			rawPS8.io.CAN(i._2).PHYRX := False
		}
	}

	private def connectTRACE(): Unit = {
		if (config.trace) {
			io.trace.clk := rawPS8.io.TRACE.CLK
			io.trace.control := rawPS8.io.TRACE.CTL
			io.trace.data := rawPS8.io.TRACE.DATA
		} else {
			False := rawPS8.io.TRACE.CLK
			False := rawPS8.io.TRACE.CTL
			0 := rawPS8.io.TRACE.DATA
		}
	}

	private def connectENET(): Unit = {
		if (config.enetGemTsu) {
			rawPS8.io.ENET_GEM_TSU.CLKFROMPL := io.enetGemTsu.clockFromPl
			io.enetGemTsu.clockToPlBufG := rawPS8.io.ENET_GEM_TSU.CLKTOPLBUFG
			rawPS8.io.ENET_GEM_TSU.CLK := io.enetGemTsu.clk
			io.enetGemTsu.timerCount := rawPS8.io.ENET_GEM_TSU.TIMERCNT
		} else {
			rawPS8.io.ENET_GEM_TSU.CLKFROMPL := False
			False := rawPS8.io.ENET_GEM_TSU.CLKTOPLBUFG
			rawPS8.io.ENET_GEM_TSU.CLK := False
			0 := rawPS8.io.ENET_GEM_TSU.TIMERCNT
		}

		for (i <- config.enetGmii.zipWithIndex) if (i._1) {
			rawPS8.io.ENET(i._2).GMII.RXCLK := io.enet.gmii(i._2).receiveClk
			io.enet.gmii(i._2).speedMode := rawPS8.io.ENET(i._2).GMII.SPEEDMODE
			rawPS8.io.ENET(i._2).GMII.CRS := io.enet.gmii(i._2).carrierSend
			rawPS8.io.ENET(i._2).GMII.COL := io.enet.gmii(i._2).collisionDetect
			rawPS8.io.ENET(i._2).GMII.RXD := io.enet.gmii(i._2).receiveData
			rawPS8.io.ENET(i._2).GMII.RXER := io.enet.gmii(i._2).receiveError
			rawPS8.io.ENET(i._2).GMII.RXDV := io.enet.gmii(i._2).receiveDataValid
			rawPS8.io.ENET(i._2).GMII.TXCLK := io.enet.gmii(i._2).transmitClk
			io.enet.gmii(i._2).transmitData := rawPS8.io.ENET(i._2).GMII.TXD
			io.enet.gmii(i._2).transmitEnable := rawPS8.io.ENET(i._2).GMII.TXEN
			io.enet.gmii(i._2).transmitError := rawPS8.io.ENET(i._2).GMII.TXER
		} else {
			rawPS8.io.ENET(i._2).GMII.RXCLK := False
			0 := rawPS8.io.ENET(i._2).GMII.SPEEDMODE
			rawPS8.io.ENET(i._2).GMII.CRS := False
			rawPS8.io.ENET(i._2).GMII.COL := False
			rawPS8.io.ENET(i._2).GMII.RXD := 0
			rawPS8.io.ENET(i._2).GMII.RXER := False
			rawPS8.io.ENET(i._2).GMII.RXDV := False
			rawPS8.io.ENET(i._2).GMII.TXCLK := False
			0 := rawPS8.io.ENET(i._2).GMII.TXD
			False := rawPS8.io.ENET(i._2).GMII.TXEN
			False := rawPS8.io.ENET(i._2).GMII.TXER
		}

		for (i <- config.enetMdio.zipWithIndex) if (i._1) {
			io.enet.mdio(i._2).mdc := rawPS8.io.ENET(i._2).MDIO.MDC
			rawPS8.io.ENET(i._2).MDIO.I := io.enet.mdio(i._2).mdio.read
			io.enet.mdio(i._2).mdio.write := rawPS8.io.ENET(i._2).MDIO.O
			io.enet.mdio(i._2).mdio.writeEnable := rawPS8.io.ENET(i._2).MDIO.TN
		} else {
			False := rawPS8.io.ENET(i._2).MDIO.MDC
			rawPS8.io.ENET(i._2).MDIO.I := False
			False := rawPS8.io.ENET(i._2).MDIO.O
			False := rawPS8.io.ENET(i._2).MDIO.TN
		}

		for (i <- config.enetFifo.zipWithIndex) if (i._1) {
			rawPS8.io.ENET(i._2).FIFO.TXRDATARDY := io.enet.fifo(i._2).transmitDataReady
			io.enet.fifo(i._2).transmitReady := rawPS8.io.ENET(i._2).FIFO.TXRRD
			rawPS8.io.ENET(i._2).FIFO.TXRVALID := io.enet.fifo(i._2).transmitValid
			rawPS8.io.ENET(i._2).FIFO.TXRDATA := io.enet.fifo(i._2).transmitData
			rawPS8.io.ENET(i._2).FIFO.TXRSOP := io.enet.fifo(i._2).transmitStartOfPacket
			rawPS8.io.ENET(i._2).FIFO.TXREOP := io.enet.fifo(i._2).transmitEndOfPacket
			rawPS8.io.ENET(i._2).FIFO.TXRERR := io.enet.fifo(i._2).transmitError
			rawPS8.io.ENET(i._2).FIFO.TXRUNDERFLOW := io.enet.fifo(i._2).transmitUnderFlow
			rawPS8.io.ENET(i._2).FIFO.TXRFLUSHED := io.enet.fifo(i._2).transmitFlushed
			rawPS8.io.ENET(i._2).FIFO.TXRCONTROL := io.enet.fifo(i._2).transmitControl
			io.enet.fifo(i._2).transmitDmaEndToggle := rawPS8.io.ENET(i._2).FIFO.DMATXENDTOG
			rawPS8.io.ENET(i._2).FIFO.DMATXSTATUSTOG := io.enet.fifo(i._2).transmitDmaStatusToggle
			io.enet.fifo(i._2).transmitStatus := rawPS8.io.ENET(i._2).FIFO.TXRSTATUS
			io.enet.fifo(i._2).transmitFixedLat := rawPS8.io.ENET(i._2).FIFO.TXRFIXEDLAT

			io.enet.fifo(i._2).receiveReady := rawPS8.io.ENET(i._2).FIFO.RXWWR
			io.enet.fifo(i._2).receiveData := rawPS8.io.ENET(i._2).FIFO.RXWDATA
			io.enet.fifo(i._2).receiveStartOfPacket := rawPS8.io.ENET(i._2).FIFO.RXWSOP
			io.enet.fifo(i._2).receiveEndOfPacket := rawPS8.io.ENET(i._2).FIFO.RXWEOP
			io.enet.fifo(i._2).receiveStatus := rawPS8.io.ENET(i._2).FIFO.RXWSTATUS
			io.enet.fifo(i._2).receiveError := rawPS8.io.ENET(i._2).FIFO.RXWERR
			io.enet.fifo(i._2).receiveOverflow := rawPS8.io.ENET(i._2).FIFO.RXWOVERFLOW
			io.enet.fifo(i._2).receiveFlush := rawPS8.io.ENET(i._2).FIFO.RXWFLUSH

			rawPS8.io.ENET(i._2).FIFO.SIGNALDETECT := io.enet.fifo(i._2).signalDetect
		} else {
			rawPS8.io.ENET(i._2).FIFO.TXRDATARDY := False
			False := rawPS8.io.ENET(i._2).FIFO.TXRRD
			rawPS8.io.ENET(i._2).FIFO.TXRVALID := False
			rawPS8.io.ENET(i._2).FIFO.TXRDATA := 0
			rawPS8.io.ENET(i._2).FIFO.TXRSOP := False
			rawPS8.io.ENET(i._2).FIFO.TXREOP := False
			rawPS8.io.ENET(i._2).FIFO.TXRERR := False
			rawPS8.io.ENET(i._2).FIFO.TXRUNDERFLOW := False
			rawPS8.io.ENET(i._2).FIFO.TXRFLUSHED := False
			rawPS8.io.ENET(i._2).FIFO.TXRCONTROL := False
			False := rawPS8.io.ENET(i._2).FIFO.DMATXENDTOG
			rawPS8.io.ENET(i._2).FIFO.DMATXSTATUSTOG := False
			0 := rawPS8.io.ENET(i._2).FIFO.TXRSTATUS
			False := rawPS8.io.ENET(i._2).FIFO.TXRFIXEDLAT

			False := rawPS8.io.ENET(i._2).FIFO.RXWWR
			0 := rawPS8.io.ENET(i._2).FIFO.RXWDATA
			False := rawPS8.io.ENET(i._2).FIFO.RXWSOP
			False := rawPS8.io.ENET(i._2).FIFO.RXWEOP
			0 := rawPS8.io.ENET(i._2).FIFO.RXWSTATUS
			False := rawPS8.io.ENET(i._2).FIFO.RXWERR
			False := rawPS8.io.ENET(i._2).FIFO.RXWOVERFLOW
			False := rawPS8.io.ENET(i._2).FIFO.RXWFLUSH

			rawPS8.io.ENET(i._2).FIFO.SIGNALDETECT := False
		}
		for (i <- config.enetGemFmio.zipWithIndex) if (i._1) {
			rawPS8.io.ENET(i._2).GEM.FMIO.TXCLKFROMPL := io.enet.gemFmio(i._2).transmitClockFromPl
			rawPS8.io.ENET(i._2).GEM.FMIO.RXCLKFROMPL := io.enet.gemFmio(i._2).recieveClockFromPl
			io.enet.gemFmio(i._2).transmitClockToPlBufG := rawPS8.io.ENET(i._2).GEM.FMIO.TXCLKTOPLBUFG
			io.enet.gemFmio(i._2).receiveClockToPlBufG := rawPS8.io.ENET(i._2).GEM.FMIO.RXCLKTOPLBUFG
		} else {
			rawPS8.io.ENET(i._2).GEM.FMIO.TXCLKFROMPL := False
			rawPS8.io.ENET(i._2).GEM.FMIO.RXCLKFROMPL := False
			False := rawPS8.io.ENET(i._2).GEM.FMIO.TXCLKTOPLBUFG
			False := rawPS8.io.ENET(i._2).GEM.FMIO.RXCLKTOPLBUFG
		}
		for (i <- config.enetGem1588.zipWithIndex) if (i._1) {
			io.enet.gem1588(i._2).transmitStartOfFrame := rawPS8.io.ENET(i._2).GEM.GEM1588.TXSOF
			io.enet.gem1588(i._2).transmitSyncFrame := rawPS8.io.ENET(i._2).GEM.GEM1588.SYNCFRAMETX
			io.enet.gem1588(i._2).transmitDelayReq := rawPS8.io.ENET(i._2).GEM.GEM1588.DELAYREQTX
			io.enet.gem1588(i._2).transmitPDelayReq := rawPS8.io.ENET(i._2).GEM.GEM1588.PDELAYREQTX
			io.enet.gem1588(i._2).transmitPDelayResp := rawPS8.io.ENET(i._2).GEM.GEM1588.PDELAYRESPTX
			io.enet.gem1588(i._2).receiveStartOfFrame := rawPS8.io.ENET(i._2).GEM.GEM1588.RXSOF
			io.enet.gem1588(i._2).receiveSyncFrame := rawPS8.io.ENET(i._2).GEM.GEM1588.SYNCFRAMERX
			io.enet.gem1588(i._2).receiveDelayReq := rawPS8.io.ENET(i._2).GEM.GEM1588.DELAYREQRX
			io.enet.gem1588(i._2).receivePDelayReq := rawPS8.io.ENET(i._2).GEM.GEM1588.PDELAYREQRX
			io.enet.gem1588(i._2).receivePDelayResp := rawPS8.io.ENET(i._2).GEM.GEM1588.PDELAYRESPRX

			rawPS8.io.ENET(i._2).GEM.GEM1588.TSUINCCTRL := io.enet.gem1588(i._2).tsuIncControl
			io.enet.gem1588(i._2).tsuTimerCompareValue := rawPS8.io.ENET(i._2).GEM.GEM1588.TSUTIMERCMPVAL
		} else {
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.TXSOF
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.SYNCFRAMETX
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.DELAYREQTX
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.PDELAYREQTX
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.PDELAYRESPTX
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.RXSOF
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.SYNCFRAMERX
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.DELAYREQRX
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.PDELAYREQRX
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.PDELAYRESPRX

			rawPS8.io.ENET(i._2).GEM.GEM1588.TSUINCCTRL := 0
			False := rawPS8.io.ENET(i._2).GEM.GEM1588.TSUTIMERCMPVAL
		}
		for (i <- config.enetGemMisc.zipWithIndex) if (i._1) {
			rawPS8.io.ENET(i._2).GEM.MISC.EXTINTIN := io.enet.gemMisc(i._2).externalIntIn
			io.enet.gemMisc(i._2).dmaBusWidth := rawPS8.io.ENET(i._2).GEM.MISC.DMABUSWIDTH
		} else {
			rawPS8.io.ENET(i._2).GEM.MISC.EXTINTIN := False
			0 := rawPS8.io.ENET(i._2).GEM.MISC.DMABUSWIDTH
		}
	}

	private def connectUSB(): Unit = {
		for (i <- config.usb.zipWithIndex) if (i._1) {
			rawPS8.io.USB(i._2).HUBPORTOVERCRNTUSB2 := io.usb(i._2).hubPortOverCurrentUSB2
			rawPS8.io.USB(i._2).HUBPORTOVERCRNTUSB3 := io.usb(i._2).hubPortOverCurrentUSB2
			io.usb(i._2).u2dsVBusCtrl := rawPS8.io.USB(i._2).U2DSPORTVBUSCTRLUSB3
			io.usb(i._2).u3dsVBusCtrl := rawPS8.io.USB(i._2).U3DSPORTVBUSCTRLUSB3
		} else {
			rawPS8.io.USB(i._2).HUBPORTOVERCRNTUSB2 := False
			rawPS8.io.USB(i._2).HUBPORTOVERCRNTUSB3 := False
			False := rawPS8.io.USB(i._2).U2DSPORTVBUSCTRLUSB3
			False := rawPS8.io.USB(i._2).U3DSPORTVBUSCTRLUSB3
		}
	}

	private def capPSS_ALTOC_CORE_PAD(): Unit = {
		rawPS8.io.PSS_ALTO_CORE_PAD.MGTRXN0IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.MGTRXN1IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.MGTRXN2IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.MGTRXN3IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.MGTRXP0IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.MGTRXP1IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.MGTRXP2IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.MGTRXP3IN := False

		rawPS8.io.PSS_ALTO_CORE_PAD.REFN0IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.REFN1IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.REFN2IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.REFN3IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.REFP0IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.REFP1IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.REFP2IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.REFP3IN := False
		rawPS8.io.PSS_ALTO_CORE_PAD.PADI := False
	}

	// Function used to rename all signals of the blackbox
	private def renameIO(): Unit = {
		io.flatten.foreach(bt => {
			bt.setName(bt.getName().replace("_axi", ""))
			bt.setName(bt.getName().replace("_payload", ""))
			bt.setName(bt.getName().replace("_aw_", "_aw"))
			bt.setName(bt.getName().replace("_ar_", "_ar"))
			bt.setName(bt.getName().replace("_r_", "_r"))
			bt.setName(bt.getName().replace("_w_", "_w"))
			bt.setName(bt.getName().replace("_b_", "_b"))
		})
	}
	// Execute the function renameIO after the creation of the component
	addPrePopTask(() => renameIO())
}

object ZynqMpPs8Config {
	private val possibleBusNames = Array[(String, Boolean)](("hpm0", true),
	                                                        ("hpm1", true),
	                                                        ("lpd", true),
	                                                        ("hpc0", false),
	                                                        ("hpc1", false),
	                                                        ("hp0", false),
	                                                        ("hp1", false),
	                                                        ("hp2", false),
	                                                        ("hp3", false),
	                                                        ("lpd", false),
	                                                        ("acp", false),
	                                                        ("ace", false))

	def createFromTable(table: Map[String, Variant]): ZynqMpPs8Config = {
		var config = ZynqMpPs8Config()
		table.foreach(tuple => tuple._1 match {
			case "buses"                        => Utils.toArray(tuple._2).foreach(t => config = decodeBusVariant(Utils.toTable(t), config))
			case "axi_counter"                  => config = config.copy(exposePlToPsAxiCounters = Utils.toBoolean(tuple._2))
			case "display_port_ps_to_pl_timing" => config = config.copy(psToPlDisplayPortTiming = Utils.toBoolean(tuple._2))
			case "display_port_ps_to_pl_video"  => config = config.copy(psToPlDisplayPortVideo = Utils.toBoolean(tuple._2))
			case "display_port_ps_to_pl_audio"  => config = config.copy(psToPlDisplayPortAudio = Utils.toBoolean(tuple._2))
			case "display_port_pl_to_ps_timing" => config = config.copy(plToPsDisplayPortTiming = Utils.toBoolean(tuple._2))
			case "display_port_pl_to_ps_video"  => config = config.copy(plToPsDisplayPortVideo = Utils.toBoolean(tuple._2))
			case "display_port_pl_to_ps_audio"  => config = config.copy(plToPsDisplayPortAudio = Utils.toBoolean(tuple._2))
			case "display_port_aux"             => config = config.copy(displayPortAux = Utils.toBoolean(tuple._2))
			case "display_port_events"          => config = config.copy(displayPortEvents = Utils.toBoolean(tuple._2))
			case "trace"                        => config = config.copy(trace = Utils.toBoolean(tuple._2))
			case "enet_gemtsu"                  => config = config.copy(enetGemTsu = Utils.toBoolean(tuple._2))

			case "i2c0" | "i2c1"                                       => {
				val index = tuple._1.replace("i2c", "").toInt
				config.i2c(index) = Utils.toBoolean(tuple._2)
			}
			case "uart0" | "uart1"                                     => {
				val index = tuple._1.replace("uart", "").toInt
				config.uart(index) = Utils.toBoolean(tuple._2)
			}
			case "ttc0" | "ttc1" | "ttc2" | "ttc3"                     => {
				val index = tuple._1.replace("ttc", "").toInt
				config.ttc(index) = Utils.toBoolean(tuple._2)
			}
			case "wdt0" | "wdt1"                                       => {
				val index = tuple._1.replace("wdt", "").toInt
				config.wdt(index) = Utils.toBoolean(tuple._2)
			}
			case "spi0" | "spi1"                                       => {
				val index = tuple._1.replace("spi", "").toInt
				config.spi(index) = Utils.toBoolean(tuple._2)
			}
			case "sdio0" | "sdio1"                                     => {
				val index = tuple._1.replace("sdio", "").toInt
				config.sdio(index) = Utils.toBoolean(tuple._2)
			}
			case "can0" | "can1"                                       => {
				val index = tuple._1.replace("can", "").toInt
				config.can(index) = Utils.toBoolean(tuple._2)
			}
			case "pl_clock0" | "pl_clock1" | "pl_clock2" | "pl_clock3" => {
				val index = tuple._1.replace("pl_clock", "").toInt
				config.plClocks(index) = Utils.toBoolean(tuple._2)
			}
			case "pl_clock_buffered0" | "pl_clock_buffered1" |
			     "pl_clock_buffered2" | "pl_clock_buffered3"           => {
				val index = tuple._1.replace("pl_clock_buffered", "").toInt
				config.plClocksBuffered(index) = Utils.toBoolean(tuple._2)
			}
			case "enet_gmii0" | "enet_gmii1" |
			     "enet_gmii2" | "enet_gmii3"                           => {
				val index = tuple._1.replace("enet_gmii", "").toInt
				config.enetGmii(index) = Utils.toBoolean(tuple._2)
			}
			case "enet_fifo0" | "enet_fifo1" |
			     "enet_fifo2" | "enet_fifo3"                           => {
				val index = tuple._1.replace("enet_fifo", "").toInt
				config.enetFifo(index) = Utils.toBoolean(tuple._2)
			}
			case "enet_mdio0" | "enet_mdio1" |
			     "enet_mdio2" | "enet_mdio3"                           => {
				val index = tuple._1.replace("enet_mdio", "").toInt
				config.enetMdio(index) = Utils.toBoolean(tuple._2)
			}
			case "enet_gemfmio0" | "enet_gemfmio1" |
			     "enet_gemfmio2" | "enet_gemfmio3"                     => {
				val index = tuple._1.replace("enet_gemfmio", "").toInt
				config.enetGemFmio(index) = Utils.toBoolean(tuple._2)
			}
			case "enet_gem15880" | "enet_gem15881" |
			     "enet_gem15882" | "enet_gem15883"                     => {
				val index = tuple._1.replace("enet_gem1588", "").toInt
				config.enetGem1588(index) = Utils.toBoolean(tuple._2)
			}
			case "enet_gemmisc0" | "enet_gemmisc1" |
			     "enet_gemmisc2" | "enet_gemmisc3"                     => {
				val index = tuple._1.replace("enet_gemmisc", "").toInt
				config.enetGemMisc(index) = Utils.toBoolean(tuple._2)
			}
			case "usb0" | "usb1"                                       => {
				val index = tuple._1.replace("usb", "").toInt
				config.usb(index) = Utils.toBoolean(tuple._2)
			}
			case _                                                     =>
		})
		config
	}

	private def decodeBusVariant(t: Map[String, Variant], config: ZynqMpPs8Config): ZynqMpPs8Config = {
		val name     = Utils.lookupString(t, "name", "")
		val protocol = Utils.lookupString(t, "protocol", "")
		val supplier = Utils.lookupBoolean(t, "supplier", false)

		if (name.nonEmpty) setBus((name, supplier), config) else if (protocol == "axi4") {
			possibleBusNames.foreach { case (n, _) => if (!isBusUsed((n, supplier), config)) {
				return setBus((n, supplier), config)
			}
			}
			config
		} else {
			println("ERROR: Unknown bus protocol")
			config
		}
	}

	private def setBus(v: (String, Boolean), config: ZynqMpPs8Config): ZynqMpPs8Config = {
		val (name, supplier) = possibleBusNames.find(_ == v).getOrElse {
			val stxt = if (v._2) "supplier" else "consumer"
			println(s"ERROR: Unknown $stxt bus named $v._1")
			return config
		}
		if (supplier) name match {
			case "hpm0" => config.copy(psToPlAxiHpm0 = true)
			case "hpm1" => config.copy(psToPlAxiHpm1 = true)
			case "lpd"  => config.copy(psToPlAxiLpd = true)
		} else name match {
			case "hpc0" => config.copy(plToPsAxiHpc0 = true)
			case "hpc1" => config.copy(plToPsAxiHpc1 = true)
			case "hp0"  => config.copy(plToPsAxiHp0 = true)
			case "hp1"  => config.copy(plToPsAxiHp1 = true)
			case "hp2"  => config.copy(plToPsAxiHp2 = true)
			case "hp3"  => config.copy(plToPsAxiHp3 = true)
			case "lpd"  => config.copy(plToPsAxiLpd = true)
			case "acp"  => config.copy(plToPsAxiAcp = true)
			case "ace"  => config.copy(plToPsAxiAce = true)
		}
	}

	private def isBusUsed(v: (String, Boolean), config: ZynqMpPs8Config): Boolean = {
		val (name, supplier) = possibleBusNames.find(_ == v).getOrElse {
			return true
		}
		if (supplier) name match {
			case "hpm0" => config.psToPlAxiHpm0
			case "hpm1" => config.psToPlAxiHpm1
			case "lpd"  => config.psToPlAxiLpd
		} else name match {
			case "hpc0" => config.plToPsAxiHpc0
			case "hpc1" => config.plToPsAxiHpc1
			case "hp0"  => config.plToPsAxiHp0
			case "hp1"  => config.plToPsAxiHp1
			case "hp2"  => config.plToPsAxiHp2
			case "hp3"  => config.plToPsAxiHp3
			case "lpd"  => config.plToPsAxiLpd
			case "acp"  => config.plToPsAxiAcp
			case "ace"  => config.plToPsAxiAce
		}
	}
}

object Ps8Wrapper {

	def main(args: Array[String]): Unit = {
		println(s"Building ZynqMP Ps8 SOC wrapper")

		// read config file
		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "PS8Soc"

		val table = if (tomlFile.isEmpty) {
			println(s"No toml config file provided, defaults will be used")
			Map[String, Variant]()
		} else Utils.readToml(Paths.get(tomlFile.get))

		println(Paths.get(tomlFile.get).toString)

		val ps8Config = ZynqMpPs8Config.createFromTable(table)

		// setup spinalHDL
		val spinalConfig = SpinalConfig(defaultConfigForClockDomains = ClockDomainConfig(resetKind = spinal.core.SYNC),
		                                targetDirectory = targetDir,
		                                netlistFileName = name + ".v")

		// generate verilog
		val report = spinalConfig.generateVerilog {
			ZynqMpPs8(ps8Config).setDefinitionName(name)
		}
		//		report.printPrunedIo()
		//		report.printRtl()
	}
}