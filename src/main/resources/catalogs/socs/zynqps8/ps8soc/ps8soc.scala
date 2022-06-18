import spinal.core._
import spinal.lib.bus.amba4.axi.Axi4Config
import spinal.lib.bus.amba4.axis.Axi4StreamConfig
import scala.language.postfixOps

case class PS8() extends BlackBox {

	val io = new Bundle {
		val MAXIGP = for (i <- 0 until 3) yield new Bundle {
			val ACLK = in Bool()

			val AWID    = out UInt (PS8.PsToPLConfig.idWidth bits)
			val AWADDR  = out UInt (PS8.PsToPLConfig.addressWidth bits)
			val AWLEN   = out UInt (8 bits)
			val AWSIZE  = out UInt (3 bits)
			val AWBURST = out Bits (2 bits)
			val AWLOCK  = out Bits (1 bits)
			val AWCACHE = out Bits (4 bits)
			val AWPROT  = out Bits (3 bits)
			val AWVALID = out Bool()
			val AWUSER  = out Bits (PS8.PsToPLConfig.awUserWidth bits)
			val AWREADY = in Bool()
			val AWQOS   = out Bits (4 bits)

			val WDATA  = out Bits (128 bits)
			val WSTRB  = out Bits (16 bits)
			val WLAST  = out Bool()
			val WVALID = out Bool()
			val WREADY = in Bool()

			val BID    = in UInt (PS8.PsToPLConfig.idWidth bits)
			val BRESP  = in Bits (2 bits)
			val BVALID = in Bool()
			val BREADY = out Bool()

			val ARID    = out UInt (PS8.PsToPLConfig.idWidth bits)
			val ARADDR  = out UInt (PS8.PsToPLConfig.addressWidth bits)
			val ARLEN   = out UInt (8 bits)
			val ARSIZE  = out UInt (3 bits)
			val ARBURST = out Bits (2 bits)
			val ARLOCK  = out Bits (1 bits)
			val ARCACHE = out Bits (4 bits)
			val ARPROT  = out Bits (3 bits)
			val ARVALID = out Bool()
			val ARUSER  = out Bits (PS8.PsToPLConfig.arUserWidth bits)
			val ARREADY = in Bool()
			val ARQOS   = out Bits (4 bits)

			val RID    = in UInt (PS8.PsToPLConfig.idWidth bits)
			val RDATA  = in Bits (128 bits)
			val RRESP  = in Bits (2 bits)
			val RLAST  = in Bool()
			val RVALID = in Bool()
			val RREADY = out Bool()
		}
		val SAXIGP = for (i <- 0 until 7) yield new Bundle {
			val RCLK = in Bool()
			val WCLK = in Bool()

			val AWID    = in UInt (PS8.PlToPsConfig.idWidth bits)
			val AWADDR  = in UInt (PS8.PlToPsConfig.addressWidth bits)
			val AWLEN   = in UInt (8 bits)
			val AWSIZE  = in UInt (3 bits)
			val AWBURST = in Bits (2 bits)
			val AWLOCK  = in Bits (1 bits)
			val AWCACHE = in Bits (4 bits)
			val AWPROT  = in Bits (3 bits)
			val AWVALID = in Bool()
			val AWUSER  = in Bits (PS8.PlToPsConfig.awUserWidth bits)
			val AWREADY = out Bool()
			val AWQOS   = in Bits (4 bits)

			val WDATA  = in Bits (128 bits)
			val WSTRB  = in Bits (16 bits)
			val WLAST  = in Bool()
			val WVALID = in Bool()
			val WREADY = out Bool()

			val BID    = out UInt (PS8.PlToPsConfig.idWidth bits)
			val BRESP  = out Bits (2 bits)
			val BVALID = out Bool()
			val BREADY = in Bool()

			val ARID    = in UInt (PS8.PlToPsConfig.idWidth bits)
			val ARADDR  = in UInt (PS8.PlToPsConfig.addressWidth bits)
			val ARLEN   = in UInt (8 bits)
			val ARSIZE  = in UInt (3 bits)
			val ARBURST = in Bits (2 bits)
			val ARLOCK  = in Bits (1 bits)
			val ARCACHE = in Bits (4 bits)
			val ARPROT  = in Bits (3 bits)
			val ARVALID = in Bool()
			val ARUSER  = in Bits (PS8.PlToPsConfig.arUserWidth bits)
			val ARREADY = out Bool()
			val ARQOS   = in Bits (4 bits)

			val RID    = out UInt (PS8.PlToPsConfig.idWidth bits)
			val RDATA  = out Bits (128 bits)
			val RRESP  = out Bits (2 bits)
			val RLAST  = out Bool()
			val RVALID = out Bool()
			val RREADY = in Bool()

			val RCOUNT  = out UInt (8 bits)
			val WCOUNT  = out UInt (8 bits)
			val ARCOUNT = out UInt (4 bits)
			val AWCOUNT = out UInt (4 bits)
		}

		val SAXIACP = new Bundle {
			val ACLK = in Bool()

			val AWID    = in UInt (PS8.PlToPsAcpConfig.idWidth bits)
			val AWADDR  = in UInt (PS8.PlToPsAcpConfig.addressWidth bits)
			val AWLEN   = in UInt (8 bits)
			val AWSIZE  = in UInt (3 bits)
			val AWBURST = in Bits (2 bits)
			val AWLOCK  = in Bits (1 bits)
			val AWCACHE = in Bits (4 bits)
			val AWPROT  = in Bits (3 bits)
			val AWVALID = in Bool()
			val AWUSER  = in Bits (PS8.PlToPsAcpConfig.awUserWidth bits)
			val AWREADY = out Bool()
			val AWQOS   = in Bits (4 bits)

			val WDATA  = in Bits (128 bits)
			val WSTRB  = in Bits (16 bits)
			val WLAST  = in Bool()
			val WVALID = in Bool()
			val WREADY = out Bool()

			val BID    = out UInt (PS8.PlToPsAcpConfig.idWidth bits)
			val BRESP  = out Bits (2 bits)
			val BVALID = out Bool()
			val BREADY = in Bool()

			val ARID    = in UInt (PS8.PlToPsAcpConfig.idWidth bits)
			val ARADDR  = in UInt (PS8.PlToPsAcpConfig.addressWidth bits)
			val ARLEN   = in UInt (8 bits)
			val ARSIZE  = in UInt (3 bits)
			val ARBURST = in Bits (2 bits)
			val ARLOCK  = in Bits (1 bits)
			val ARCACHE = in Bits (4 bits)
			val ARPROT  = in Bits (3 bits)
			val ARVALID = in Bool()
			val ARUSER  = in Bits (PS8.PlToPsAcpConfig.arUserWidth bits)
			val ARREADY = out Bool()
			val ARQOS   = in Bits (4 bits)

			val RID    = out UInt (PS8.PlToPsAcpConfig.idWidth bits)
			val RDATA  = out Bits (128 bits)
			val RRESP  = out Bits (2 bits)
			val RLAST  = out Bool()
			val RVALID = out Bool()
			val RREADY = in Bool()
		}
		val SAXIACE = new Bundle {
			val ACLK = in Bool()

			val AWID     = in UInt (PS8.PlToPsAceConfig.idWidth bits)
			val AWADDR   = in UInt (PS8.PlToPsAceConfig.addressWidth bits)
			val AWLEN    = in UInt (8 bits)
			val AWSIZE   = in UInt (3 bits)
			val AWBURST  = in Bits (2 bits)
			val AWLOCK   = in Bits (1 bits)
			val AWCACHE  = in Bits (4 bits)
			val AWPROT   = in Bits (3 bits)
			val AWREGION = in Bits (4 bits)
			val AWVALID  = in Bool()
			val AWREADY  = out Bool()
			val AWQOS    = in Bits (4 bits)
			val AWDOMAIN = in Bits (2 bits)
			val AWSNOOP  = in Bits (3 bits)
			val AWBAR    = in Bits (2 bits)

			val WDATA  = in Bits (128 bits)
			val WSTRB  = in Bits (16 bits)
			val WLAST  = in Bool()
			val WVALID = in Bool()
			val WREADY = out Bool()
			val WUSER  = in Bits (PS8.PlToPsAceConfig.wUserWidth bits)

			val BID    = out UInt (PS8.PlToPsAceConfig.idWidth bits)
			val BRESP  = out Bits (2 bits)
			val BVALID = out Bool()
			val BREADY = in Bool()
			val BUSER  = in Bits (PS8.PlToPsAceConfig.bUserWidth bits)

			val ARID     = in UInt (PS8.PlToPsAceConfig.idWidth bits)
			val ARADDR   = in UInt (PS8.PlToPsAceConfig.addressWidth bits)
			val ARLEN    = in UInt (8 bits)
			val ARSIZE   = in UInt (3 bits)
			val ARBURST  = in Bits (2 bits)
			val ARLOCK   = in Bits (1 bits)
			val ARCACHE  = in Bits (4 bits)
			val ARPROT   = in Bits (3 bits)
			val ARVALID  = in Bool()
			val ARREADY  = out Bool()
			val ARQOS    = in Bits (4 bits)
			val ARREGION = in Bits (4 bits)
			val ARDOMAIN = in Bits (2 bits)
			val ARSNOOP  = in Bits (4 bits)
			val ARBAR    = in Bits (2 bits)

			val RID    = out UInt (PS8.PlToPsAceConfig.idWidth bits)
			val RDATA  = out Bits (128 bits)
			val RRESP  = out Bits (4 bits)
			val RLAST  = out Bool()
			val RVALID = out Bool()
			val RREADY = in Bool()
			val RUSER  = out Bits (PS8.PlToPsAceConfig.rUserWidth bits)

			val ACVALID = out Bool()
			val ACREADY = in Bool()
			val ACADDR  = out Bits (44 bits)
			val ACSNOOP = out Bits (4 bits)
			val ACPROT  = out Bits (3 bits)
			val CRVALID = in Bool()
			val CRREADY = out Bool()
			val CRRESP  = in Bits (5 bits)
			val CDVALID = in Bool()
			val CDREADY = out Bool()
			val CDDATA  = in Bits (128 bits)
			val CDLAST  = in Bool()
			val WACK    = in Bool()
			val RACK    = out Bool()

		}

		val DP = new Bundle {

			// axi stream for ps to pl audio
			val AUDIOREFCLK           = out Bool()
			val MAXISMIXEDAUDIOTDATA  = out Bits (32 bits)
			val MAXISMIXEDAUDIOTID    = out UInt (1 bit)
			val MAXISMIXEDAUDIOTVALID = out Bool()
			val MAXISMIXEDAUDIOTREADY = in Bool()

			// axi stream for pl to ps audio
			val SAXISAUDIOCLK    = in Bool()
			val SAXISAUDIOTDATA  = in Bits (32 bits)
			val SAXISAUDIOTID    = in UInt (1 bit)
			val SAXISAUDIOTVALID = in Bool()
			val SAXISAUDIOTREADY = out Bool()

			// PL generated video
			val VIDEOINCLK        = in Bool()
			val LIVEVIDEOINPIXEL1 = in Bits (36 bits)
			val LIVEGFXALPHAIN    = in Bits (8 bits)
			val LIVEGFXPIXEL1IN   = in Bits (36 bits)

			// PS generated video timing
			val VIDEOOUTHSYNC  = out Bool()
			val VIDEOOUTVSYNC  = out Bool()
			val LIVEVIDEODEOUT = out Bool()

			// PL generated video timing
			val LIVEVIDEOINHSYNC = in Bool()
			val LIVEVIDEOINVSYNC = in Bool()
			val LIVEVIDEOINDE    = in Bool()

			// display port video from PS
			val VIDEOREFCLK    = out Bool()
			val VIDEOOUTPIXEL1 = out Bits (36 bits)

			// Display port aux data in and out
			val AUXDATAIN  = in Bool()
			val AUXDATAOUT = out Bool()
			val AUXDATAOEN = out Bool()

			// PL generate events
			val HOTPLUGDETECT        = in Bool()
			val EXTERNALCUSTOMEVENT1 = in Bool()
			val EXTERNALCUSTOMEVENT2 = in Bool()
			val EXTERNALVSYNCEVENT   = in Bool()
		}

		val I2C = for (i <- 0 until 2) yield new Bundle {
			val SCLI  = in Bool()
			val SCLO  = out Bool()
			val SCLTN = out Bool()
			val SDAI  = in Bool()
			val SDAO  = out Bool()
			val SDATN = out Bool()
		}

		val UART = for (i <- 0 until 2) yield new Bundle {
			val TX   = out Bool()
			val RX   = in Bool()
			val CTSN = in Bool()
			val RTSN = out Bool()
			val DSRN = in Bool()
			val DCDN = in Bool()
			val RIN  = in Bool()
			val DTRN = out Bool()
		}

		val TTC = for (i <- 0 until 4) yield new Bundle {
			val CLKI  = in Bits (3 bits)
			val WAVEO = out Bits (3 bits)
		}
		val WDT = for (i <- 0 until 2) yield new Bundle {
			val CLKI = in Bool()
			val RSTO = out Bool()
		}
		val SPI = for (i <- 0 until 2) yield new Bundle {
			val SCLKI  = in Bool()
			val SCLKO  = out Bool()
			val SCLKTN = out Bool()
			val MI     = in Bool()
			val MO     = out Bool()
			val MOTN   = out Bool()
			val SI     = in Bool()
			val SO     = out Bool()
			val STN    = out Bool()
			val SSIN   = in Bool()
			val SSON   = out Bits (3 bits)
			val SSNTN  = out Bool()

		}

		val SDIO = for (i <- 0 until 2) yield new Bundle {
			val CLKOUT     = out Bool()
			val FBCLKIN    = in Bool()
			val CMDOUT     = out Bool()
			val CMDIN      = in Bool()
			val CMDENA     = out Bool()
			val DATAIN     = in Bits (8 bits)
			val DATAOUT    = out Bits (8 bits)
			val DATAENA    = out Bits (8 bits)
			val CDN        = in Bool()
			val WP         = in Bool()
			val LEDCONTROL = out Bool()
			val BUSPOWER   = out Bool()
			val BUSVOLT    = out Bits (3 bits)
		}

		val CAN = for (i <- 0 until 2) yield new Bundle {
			val PHYTX = out Bool()
			val PHYRX = in Bool()
		}

		val GPIO = new Bundle {
			val I  = in Bits (96 bits)
			val O  = out Bits (96 bits)
			val TN = out Bits (96 bits)
		}

		val PLCLK = out Bits (4 bits)

		val TRACE = new Bundle {
			val CLK  = out Bool()
			val CTL  = out Bool()
			val DATA = out Bits (32 bits)
		}

		val USB = for (i <- 0 until 2) yield new Bundle {
			val HUBPORTOVERCRNTUSB3  = in Bool()
			val HUBPORTOVERCRNTUSB2  = in Bool()
			val U2DSPORTVBUSCTRLUSB3 = out Bool()
			val U3DSPORTVBUSCTRLUSB3 = out Bool()
		}

		// shared by all ENET instances
		val ENET_GEM_TSU = new Bundle {
			val CLKFROMPL   = in Bool()
			val CLKTOPLBUFG = out Bool()
			val CLK         = in Bool()
			val TIMERCNT    = out Bits (94 bits)
		}

		val ENET = for (i <- 0 until 4) yield new Bundle {
			val GMII = new Bundle {
				val RXCLK     = in Bool()
				val SPEEDMODE = out Bits (3 bits)
				val CRS       = in Bool()
				val COL       = in Bool()
				val RXD       = in Bits (8 bits)
				val RXER      = in Bool()
				val RXDV      = in Bool()
				val TXCLK     = in Bool()
				val TXD       = out Bits (8 bits)
				val TXEN      = out Bool()
				val TXER      = out Bool()
			}
			val MDIO = new Bundle {
				val MDC = out Bool()
				val I   = in Bool()
				val O   = out Bool()
				val TN  = out Bool()
			}
			val FIFO = new Bundle {
				val TXRDATARDY     = in Bool()
				val TXRRD          = out Bool()
				val TXRVALID       = in Bool()
				val TXRDATA        = in Bits (8 bits)
				val TXRSOP         = in Bool()
				val TXREOP         = in Bool()
				val TXRERR         = in Bool()
				val TXRUNDERFLOW   = in Bool()
				val TXRFLUSHED     = in Bool()
				val TXRCONTROL     = in Bool()
				val DMATXENDTOG    = out Bool()
				val DMATXSTATUSTOG = in Bool()
				val TXRSTATUS      = out Bits (4 bits)

				val RXWWR       = out Bool()
				val RXWDATA     = out Bits (8 bits)
				val RXWSOP      = out Bool()
				val RXWEOP      = out Bool()
				val RXWSTATUS   = out Bits (45 bits)
				val RXWERR      = out Bool()
				val RXWOVERFLOW = out Bool()
				val RXWFLUSH    = out Bool()
				val TXRFIXEDLAT = out Bool()

				val SIGNALDETECT = in Bool()
			}
			val GEM  = new Bundle {
				val FMIO    = new Bundle {
					val TXCLKFROMPL   = in Bool()
					val RXCLKFROMPL   = in Bool()
					val TXCLKTOPLBUFG = out Bool()
					val RXCLKTOPLBUFG = out Bool()
				}
				val GEM1588 = new Bundle {

					val TXSOF        = out Bool()
					val SYNCFRAMETX  = out Bool()
					val DELAYREQTX   = out Bool()
					val PDELAYREQTX  = out Bool()
					val PDELAYRESPTX = out Bool()

					val RXSOF        = out Bool()
					val SYNCFRAMERX  = out Bool()
					val DELAYREQRX   = out Bool()
					val PDELAYREQRX  = out Bool()
					val PDELAYRESPRX = out Bool()

					val TSUINCCTRL     = in Bits (2 bits)
					val TSUTIMERCMPVAL = out Bool()
				}

				val MISC = new Bundle {
					val EXTINTIN    = in Bool()
					val DMABUSWIDTH = out Bits (2 bits)
				}
			}
		}

		val MISC = new Bundle {
			val DDRCEXTREFRESHRANK0REQ = in Bool()
			val DDRCEXTREFRESHRANK1REQ = in Bool()
			val DDRCREFRESHPLCLK       = in Bool()
			val PLACPINACT             = in Bool()
		}

		val PSS_ALTO_CORE_PAD = new Bundle {
			val MGTTXN0OUT = out Bool()
			val MGTTXN1OUT = out Bool()
			val MGTTXN2OUT = out Bool()
			val MGTTXN3OUT = out Bool()
			val MGTTXP0OUT = out Bool()
			val MGTTXP1OUT = out Bool()
			val MGTTXP2OUT = out Bool()
			val MGTTXP3OUT = out Bool()
			val MGTRXN0IN  = in Bool()
			val MGTRXN1IN  = in Bool()
			val MGTRXN2IN  = in Bool()
			val MGTRXN3IN  = in Bool()
			val MGTRXP0IN  = in Bool()
			val MGTRXP1IN  = in Bool()
			val MGTRXP2IN  = in Bool()
			val MGTRXP3IN  = in Bool()

			val PADO        = out Bool()
			val BOOTMODE    = out Bits (4 bits)
			val CLK         = out Bool()
			val DONEB       = out Bool()
			val DRAMA       = out Bits (18 bits)
			val DRAMACTN    = out Bool()
			val DRAMALERTN  = out Bool()
			val DRAMBA      = out Bits (2 bits)
			val DRAMBG      = out Bits (2 bits)
			val DRAMCK      = out Bits (2 bits)
			val DRAMCKE     = out Bits (2 bits)
			val DRAMCKN     = out Bits (2 bits)
			val DRAMCSN     = out Bits (2 bits)
			val DRAMDM      = out Bits (9 bits)
			val DRAMDQ      = out Bits (72 bits)
			val DRAMDQS     = out Bits (9 bits)
			val DRAMDQSN    = out Bits (9 bits)
			val DRAMODT     = out Bits (2 bits)
			val DRAMPARITY  = out Bool()
			val DRAMRAMRSTN = out Bool()
			val ERROROUT    = out Bool()
			val ERRORSTATUS = out Bool()
			val INITB       = out Bool()
			val JTAGTCK     = out Bool()
			val JTAGTDI     = out Bool()
			val JTAGTDO     = out Bool()
			val JTAGTMS     = out Bool()
			val MIO         = out Bool()
			val PORB        = out Bool()
			val PROGB       = out Bool()
			val RCALIBINOUT = out Bool()
			val SRSTB       = out Bool()
			val ZQ          = out Bool()
			val PADI        = in Bool()
			val REFN0IN     = in Bool()
			val REFN1IN     = in Bool()
			val REFN2IN     = in Bool()
			val REFN3IN     = in Bool()
			val REFP0IN     = in Bool()
			val REFP1IN     = in Bool()
			val REFP2IN     = in Bool()
			val REFP3IN     = in Bool()
		}

	}
	noIoPrefix()

	// Function used to rename all signals of the blackbox
	private def renameIO(): Unit = {
		io.flatten.foreach(bt => {
			for (i <- 0 until 10) {
				val replaceSrc = s"_${i}_"
				val replaceDst = s"$i"
				bt.setName(bt.getName().replace(replaceSrc, replaceDst))
			}
			bt.setName(bt.getName().replace("DP_", "DP"))
			bt.setName(bt.getName().replace("SAXIACP_", "SAXIACP"))
			bt.setName(bt.getName().replace("SAXIACE_", "SACEFPD"))
			bt.setName(bt.getName().replace("SAXIGP0ARCOUNT", "SAXIGP0RACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP1ARCOUNT", "SAXIGP1RACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP2ARCOUNT", "SAXIGP2RACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP3ARCOUNT", "SAXIGP3RACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP4ARCOUNT", "SAXIGP4RACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP5ARCOUNT", "SAXIGP5RACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP6ARCOUNT", "SAXIGP6RACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP0AWCOUNT", "SAXIGP0WACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP1AWCOUNT", "SAXIGP1WACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP2AWCOUNT", "SAXIGP2WACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP3AWCOUNT", "SAXIGP3WACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP4AWCOUNT", "SAXIGP4WACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP5AWCOUNT", "SAXIGP5WACOUNT"))
			bt.setName(bt.getName().replace("SAXIGP6AWCOUNT", "SAXIGP6WACOUNT"))
			bt.setName(bt.getName().replace("SACEFPDACLK", "PLACECLK"))

			bt.setName {
				if (bt.getName().contains("USB")) {
					bt.getName()
						.replace("USB0", "EMIO")
						.replace("USB1", "EMIO")
						.appended(bt.getName()(3))
				} else bt.getName()
			}
			bt.setName(bt.getName().replace("GPIO_", "EMIOGPIO"))
			bt.setName(bt.getName().replace("I2C", "EMIOI2C"))
			bt.setName(bt.getName().replace("UART", "EMIOUART"))
			bt.setName(bt.getName().replace("TTC", "EMIOTTC"))
			bt.setName(bt.getName().replace("WDT", "EMIOWDT"))
			bt.setName(bt.getName().replace("SPI", "EMIOSPI"))
			bt.setName(bt.getName().replace("SDIO", "EMIOSDIO"))
			bt.setName(bt.getName().replace("CAN", "EMIOCAN"))
			bt.setName(bt.getName().replace("TRACE_CLK", "PLPSTRACECLK"))
			bt.setName(bt.getName().replace("TRACE_CTL", "PSPLTRACECTL"))
			bt.setName(bt.getName().replace("TRACE_DATA", "PSPLTRACEDATA"))
			bt.setName(bt.getName().replace("MISC_", ""))


			bt.setName(bt.getName().replace("GMII_", "GMII"))
			bt.setName(bt.getName().replace("MDIO_", "MDIO"))
			bt.setName(bt.getName().replace("FIFO_", ""))
			bt.setName(bt.getName().replace("GEM_GEM1588_", "GEM_"))
			bt.setName(bt.getName().replace("_GEM_TSU_", "TSU"))
			bt.setName(bt.getName().replace("GEM_MISC_", ""))
			bt.setName(bt.getName().replace("ENET0GEM_", "EMIOGEM0"))
			bt.setName(bt.getName().replace("ENET1GEM_", "EMIOGEM1"))
			bt.setName(bt.getName().replace("ENET2GEM_", "EMIOGEM2"))
			bt.setName(bt.getName().replace("ENET3GEM_", "EMIOGEM3"))
			bt.setName(bt.getName().replace("EMIOGEM0FMIO_", "FMIOGEM0FIFO"))
			bt.setName(bt.getName().replace("EMIOGEM1FMIO_", "FMIOGEM1FIFO"))
			bt.setName(bt.getName().replace("EMIOGEM2FMIO_", "FMIOGEM2FIFO"))
			bt.setName(bt.getName().replace("EMIOGEM3FMIO_", "FMIOGEM3FIFO"))
			bt.setName(bt.getName().replace("ENET", "EMIOENET"))
			bt.setName(bt.getName().replace("GMIISPEEDMODE", "SPEEDMODE"))

			bt.setName(bt.getName().replace("EMIOENETTSUTIMERCNT", "EMIOENET0GEMTSUTIMERCNT"))
			bt.setName(bt.getName().replace("EMIOENETTSUCLKFROM", "FMIOGEMTSUCLKFROM"))
			bt.setName(bt.getName().replace("EMIOENETTSUCLKTO", "FMIOGEMTSUCLKTO"))
			bt.setName(bt.getName().replace("EMIOENET0TXRFIXEDLAT", "EMIOGEM0TXRFIXEDLAT"))
			bt.setName(bt.getName().replace("EMIOENET1TXRFIXEDLAT", "EMIOGEM1TXRFIXEDLAT"))
			bt.setName(bt.getName().replace("EMIOENET2TXRFIXEDLAT", "EMIOGEM2TXRFIXEDLAT"))
			bt.setName(bt.getName().replace("EMIOENET3TXRFIXEDLAT", "EMIOGEM3TXRFIXEDLAT"))
			bt.setName(bt.getName().replace("EMIOENET0SIGNALDETECT", "FMIOGEM0SIGNALDETECT"))
			bt.setName(bt.getName().replace("EMIOENET1SIGNALDETECT", "FMIOGEM1SIGNALDETECT"))
			bt.setName(bt.getName().replace("EMIOENET2SIGNALDETECT", "FMIOGEM2SIGNALDETECT"))
			bt.setName(bt.getName().replace("EMIOENET3SIGNALDETECT", "FMIOGEM3SIGNALDETECT"))
			bt.setName(bt.getName().replace("EMIOGEM0DMABUSWIDTH", "EMIOENET0DMABUSWIDTH"))
			bt.setName(bt.getName().replace("EMIOGEM1DMABUSWIDTH", "EMIOENET1DMABUSWIDTH"))
			bt.setName(bt.getName().replace("EMIOGEM2DMABUSWIDTH", "EMIOENET2DMABUSWIDTH"))
			bt.setName(bt.getName().replace("EMIOGEM3DMABUSWIDTH", "EMIOENET3DMABUSWIDTH"))
			bt.setName(bt.getName().replace("EMIOGEM0EXTINTIN", "EMIOENET0EXTINTIN"))
			bt.setName(bt.getName().replace("EMIOGEM1EXTINTIN", "EMIOENET1EXTINTIN"))
			bt.setName(bt.getName().replace("EMIOGEM2EXTINTIN", "EMIOENET2EXTINTIN"))
			bt.setName(bt.getName().replace("EMIOGEM3EXTINTIN", "EMIOENET3EXTINTIN"))


		})
	}

	// Execute the function renameIO after the creation of the component
	addPrePopTask(() => renameIO())
}

object PS8 {

	val PsToPLConfig = Axi4Config(
		addressWidth = 40,
		dataWidth = 128,
		idWidth = 16,
		arUserWidth = 16,
		awUserWidth = 16,
		useRegion = false,
		)

	val PlToPsConfig = Axi4Config(
		addressWidth = 48,
		dataWidth = 128,
		idWidth = 6,
		arUserWidth = 1,
		awUserWidth = 1,
		useRegion = false,
		)

	val PlToPsAcpConfig = Axi4Config(
		addressWidth = 40,
		dataWidth = 128,
		idWidth = 5,
		arUserWidth = 2,
		awUserWidth = 2,
		useRegion = false,
		)
	val PlToPsAceConfig = Axi4Config(
		addressWidth = 44,
		dataWidth = 128,
		idWidth = 6,
		rUserWidth = 1,
		wUserWidth = 1,
		bUserWidth = 1,
		useRegion = true,
		)

	val AudioStreamConfig = Axi4StreamConfig(
		dataWidth = 4,
		idWidth = 1,
		useId = true
		)
}


/*


val ADMAFCICLK = in Bool
val PL2ADMACVLD = in Bool
val PL2ADMATACK = in Bool
val ADMA2PLCACK = in Bool
val ADMA2PLTVLD = in Bool

val GDMAFCICLK = in Bool
val PL2GDMACVLD = in Bool
val PL2GDMATACK = in Bool
val GDMA2PLCACK = in Bool
val GDMA2PLTVLD = in Bool

val PLFPGASTOP = in Bool
val PLLAUXREFCLKLPD = in Bool
val PLLAUXREFCLKFPD = in Bool

val PLPSEVENTI = in Bool
val PSPLEVENTO = in Bool
val PSPLSTANDBYWFE = in Bool
val PSPLSTANDBYWFI = in Bool

val PLPSAPUGICIRQ = in Bool
val PLPSAPUGICFIQ = in Bool

val RPUEVENTI0 = in Bool
val RPUEVENTI1 = in Bool
val RPUEVENTO0 = in Bool
val RPUEVENTO1 = in Bool
val NFIQ0LPDRPU = in Bool
val NFIQ1LPDRPU = in Bool
val NIRQ0LPDRPU = in Bool
val NIRQ1LPDRPU = in Bool

val STMEVENT = in Bool
val PLPSTRIGACK = in Bool
val PLPSTRIGGER = in Bool
val PSPLTRIGACK = in Bool
val PSPLTRIGGER = in Bool
val FTMGPO = in Bool
val FTMGPI = in Bool

val PLPSIRQ0 = in Bool
val PLPSIRQ1 = in Bool

.PSPLIRQLPD ({irq_lpd_dev_null[18 = in8],
ps_to_pl_irq_xmpu_lpd,
ps_to_pl_irq_efuse,
ps_to_pl_irq_csu_dma,
ps_to_pl_irq_csu,
ps_to_pl_irq_adma_chan,
ps_to_pl_irq_usb3_0_pmu_wakeup,
ps_to_pl_irq_usb3_1_otg,
ps_to_pl_irq_usb3_1_endpoint,
ps_to_pl_irq_usb3_0_otg,
ps_to_pl_irq_usb3_0_endpoint,
ps_to_pl_irq_enet3_wake,
ps_to_pl_irq_enet3,
ps_to_pl_irq_enet2_wake,
ps_to_pl_irq_enet2,
ps_to_pl_irq_enet1_wake,
ps_to_pl_irq_enet1,
ps_to_pl_irq_enet0_wake,
ps_to_pl_irq_enet0,
ps_to_pl_irq_ams,
ps_to_pl_irq_aib_axi,
ps_to_pl_irq_atb_err_lpd,
ps_to_pl_irq_csu_pmu_wdt,
ps_to_pl_irq_lp_wdt,
ps_to_pl_irq_sdio1_wake,
ps_to_pl_irq_sdio0_wake,
ps_to_pl_irq_sdio1,
ps_to_pl_irq_sdio0,
ps_to_pl_irq_ttc3_2,
ps_to_pl_irq_ttc3_1,
ps_to_pl_irq_ttc3_0,
ps_to_pl_irq_ttc2_2,
ps_to_pl_irq_ttc2_1,
ps_to_pl_irq_ttc2_0,
ps_to_pl_irq_ttc1_2,
ps_to_pl_irq_ttc1_1,
ps_to_pl_irq_ttc1_0,
ps_to_pl_irq_ttc0_2,
ps_to_pl_irq_ttc0_1,
ps_to_pl_irq_ttc0_0,
ps_to_pl_irq_ipi_channel0,
ps_to_pl_irq_ipi_channel1,
ps_to_pl_irq_ipi_channel2,
ps_to_pl_irq_ipi_channel7,
ps_to_pl_irq_ipi_channel8,
ps_to_pl_irq_ipi_channel9,
ps_to_pl_irq_ipi_channel10,
ps_to_pl_irq_clkmon,
ps_to_pl_irq_rtc_seconds,
ps_to_pl_irq_rtc_alaram,
ps_to_pl_irq_lpd_apm,
ps_to_pl_irq_can1,
ps_to_pl_irq_can0,
ps_to_pl_irq_uart1,
ps_to_pl_irq_uart0,
ps_to_pl_irq_spi1,
ps_to_pl_irq_spi0,
ps_to_pl_irq_i2c1,
ps_to_pl_irq_i2c0,
ps_to_pl_irq_gpio,
ps_to_pl_irq_qspi,
ps_to_pl_irq_nand,
ps_to_pl_irq_r5_core1_ecc_error,
ps_to_pl_irq_r5_core0_ecc_error,
ps_to_pl_irq_lpd_apb_intr,
ps_to_pl_irq_ocm_error,
ps_to_pl_irq_rpu_pm,
irq_lpd_dev_null[7 = in0]}),
.PSPLIRQFPD ({irq_fpd_dev_null[19 = in12],
ps_to_pl_irq_intf_fpd_smmu,
ps_to_pl_irq_intf_ppd_cci,
ps_to_pl_irq_apu_regs,
ps_to_pl_irq_apu_exterr,
ps_to_pl_irq_apu_l2err,
ps_to_pl_irq_apu_comm,
ps_to_pl_irq_apu_pmu,
ps_to_pl_irq_apu_cti,
ps_to_pl_irq_apu_cpumnt,
ps_to_pl_irq_xmpu_fpd,
ps_to_pl_irq_sata,
ps_to_pl_irq_gpu,
ps_to_pl_irq_gdma_chan,
ps_to_pl_irq_apm_fpd,
ps_to_pl_irq_dpdma,
ps_to_pl_irq_fpd_atb_error,
ps_to_pl_irq_fpd_apb_int,
ps_to_pl_irq_dport,
ps_to_pl_irq_pcie_msc,
ps_to_pl_irq_pcie_dma,
ps_to_pl_irq_pcie_legacy,
ps_to_pl_irq_pcie_msi,
ps_to_pl_irq_fp_wdt,
ps_to_pl_irq_ddr_ss,
irq_fpd_dev_null[11 = in0]}),

val OSCRTCCLK  = in Bool

val PLPMUGPI  = in Bool
val PMUPLGPO  = in Bool
val AIBPMUAFIFMFPDACK  = in Bool
val AIBPMUAFIFMLPDACK  = in Bool
val PMUAIBAFIFMFPDREQ  = in Bool
val PMUAIBAFIFMLPDREQ  = in Bool
val PMUERRORTOPL  = in Bool
val PMUERRORFROMPL  = in Bool

val DDRCEXTREFRESHRANK0REQ  = in Bool
val DDRCEXTREFRESHRANK1REQ  = in Bool
val DDRCREFRESHPLCLK  = in Bool


*/