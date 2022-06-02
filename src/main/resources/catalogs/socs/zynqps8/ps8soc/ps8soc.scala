

case class PS8() extends BlackBox {

	val io = new Bundle {
		val MAXIGP = for (i <- 0 until 3) yield new Bundle {
			val ACLK = out Bool()

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

		val SAXIACP = for (i <- 0 until 1) yield new Bundle {
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
			val AWUSER  = out Bits (PS8.PlToPsAcpConfig.awUserWidth bits)
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
			val ARUSER  = out Bits (PS8.PlToPsAcpConfig.arUserWidth bits)
			val ARREADY = out Bool()
			val ARQOS   = in Bits (4 bits)

			val RID    = out UInt (PS8.PlToPsAcpConfig.idWidth bits)
			val RDATA  = out Bits (128 bits)
			val RRESP  = out Bits (2 bits)
			val RLAST  = out Bool()
			val RVALID = out Bool()
			val RREADY = in Bool()
		}

		val DP = new Bundle {
			val VIDEOREFCLK = out Bool()
			val AUDIOREFCLK = out Bool()

			val VIDEOINCLK    = in Bool()
			val SAXISAUDIOCLK = in Bool()

			// axi streams for ps <> pl audio
			val SAXISAUDIOTDATA  = in Bits (32 bits)
			val SAXISAUDIOTID    = in Bool()
			val SAXISAUDIOTVALID = in Bool()
			val SAXISAUDIOTREADY = out Bool()

			val MAXISMIXEDAUDIOTDATA  = out Bits (32 bits)
			val MAXISMIXEDAUDIOTID    = out Bool()
			val MAXISMIXEDAUDIOTVALID = out Bool()
			val MAXISMIXEDAUDIOTREADY = in Bool()

			// PL generated video into the PS
			val LIVEVIDEOINVSYNC  = in Bool()
			val LIVEVIDEOINHSYNC  = in Bool()
			val LIVEVIDEOINDE     = in Bool()
			val LIVEVIDEOINPIXEL1 = in Bits (36 bits)
			val LIVEVIDEODEOUT    = out Bool()
			val LIVEGFXALPHAIN    = in Bits (8 bits)
			val LIVEGFXPIXEL1IN   = in Bits (36 bits)

			// display port video from PS
			val VIDEOOUTHSYNC  = out Bool()
			val VIDEOOUTVSYNC  = out Bool()
			val VIDEOOUTPIXEL1 = out Bool()

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
			bt.setName(bt.getName().replace("SAXIACP0", "SAXIACP"))
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
}


/*
val PLACECLK = in Bool
val SACEFPDAWVALID = in Bool
val SACEFPDAWREADY = in Bool
val SACEFPDAWID = in Bool
val SACEFPDAWADDR = in Bool
val SACEFPDAWREGION = in Bool
val SACEFPDAWLEN = in Bool
val SACEFPDAWSIZE = in Bool
val SACEFPDAWBURST = in Bool
val SACEFPDAWLOCK = in Bool
val SACEFPDAWCACHE = in Bool
val SACEFPDAWPROT = in Bool
val SACEFPDAWDOMAIN = in Bool
val SACEFPDAWSNOOP = in Bool
val SACEFPDAWBAR = in Bool
val SACEFPDAWQOS = in Bool
val SACEFPDAWUSER = in Bool
val SACEFPDWVALID = in Bool
val SACEFPDWREADY = in Bool
val SACEFPDWDATA = in Bool
val SACEFPDWSTRB = in Bool
val SACEFPDWLAST = in Bool
val SACEFPDWUSER = in Bool
val SACEFPDBVALID = in Bool
val SACEFPDBREADY = in Bool
val SACEFPDBID = in Bool
val SACEFPDBRESP = in Bool
val SACEFPDBUSER = in Bool
val SACEFPDARVALID = in Bool
val SACEFPDARREADY = in Bool
val SACEFPDARID = in Bool
val SACEFPDARADDR = in Bool
val SACEFPDARREGION = in Bool
val SACEFPDARLEN = in Bool
val SACEFPDARSIZE = in Bool
val SACEFPDARBURST = in Bool
val SACEFPDARLOCK = in Bool
val SACEFPDARCACHE = in Bool
val SACEFPDARPROT = in Bool
val SACEFPDARDOMAIN = in Bool
val SACEFPDARSNOOP = in Bool
val SACEFPDARBAR = in Bool
val SACEFPDARQOS = in Bool
val SACEFPDARUSER = in Bool
val SACEFPDRVALID = in Bool
val SACEFPDRREADY = in Bool
val SACEFPDRID = in Bool
val SACEFPDRDATA = in Bool
val SACEFPDRRESP = in Bool
val SACEFPDRLAST = in Bool
val SACEFPDRUSER = in Bool
val SACEFPDACVALID = in Bool
val SACEFPDACREADY = in Bool
val SACEFPDACADDR = in Bool
val SACEFPDACSNOOP = in Bool
val SACEFPDACPROT = in Bool
val SACEFPDCRVALID = in Bool
val SACEFPDCRREADY = in Bool
val SACEFPDCRRESP = in Bool
val SACEFPDCDVALID = in Bool
val SACEFPDCDREADY = in Bool
val SACEFPDCDDATA = in Bool
val SACEFPDCDLAST = in Bool
val SACEFPDWACK = in Bool
val SACEFPDRACK = in Bool
val EMIOCAN0PHYTX = in Bool
val EMIOCAN0PHYRX = in Bool
val EMIOCAN1PHYTX = in Bool
val EMIOCAN1PHYRX = in Bool
val EMIOENET0GMIIRXCLK = in Bool
val EMIOENET0SPEEDMODE = in Bool
val EMIOENET0GMIICRS = in Bool
val EMIOENET0GMIICOL = in Bool
val EMIOENET0GMIIRXD = in Bool
val EMIOENET0GMIIRXER = in Bool
val EMIOENET0GMIIRXDV = in Bool
val EMIOENET0GMIITXCLK = in Bool
val EMIOENET0GMIITXD = in Bool
val EMIOENET0GMIITXEN = in Bool
val EMIOENET0GMIITXER = in Bool
val EMIOENET0MDIOMDC = in Bool
val EMIOENET0MDIOI = in Bool
val EMIOENET0MDIOO = in Bool
val EMIOENET0MDIOTN = in Bool
val EMIOENET1GMIIRXCLK = in Bool
val EMIOENET1SPEEDMODE = in Bool
val EMIOENET1GMIICRS = in Bool
val EMIOENET1GMIICOL = in Bool
val EMIOENET1GMIIRXD = in Bool
val EMIOENET1GMIIRXER = in Bool
val EMIOENET1GMIIRXDV = in Bool
val EMIOENET1GMIITXCLK = in Bool
val EMIOENET1GMIITXD = in Bool
val EMIOENET1GMIITXEN = in Bool
val EMIOENET1GMIITXER = in Bool
val EMIOENET1MDIOMDC = in Bool
val EMIOENET1MDIOI = in Bool
val EMIOENET1MDIOO = in Bool
val EMIOENET1MDIOTN = in Bool
val EMIOENET2GMIIRXCLK = in Bool
val EMIOENET2SPEEDMODE = in Bool
val EMIOENET2GMIICRS = in Bool
val EMIOENET2GMIICOL = in Bool
val EMIOENET2GMIIRXD = in Bool
val EMIOENET2GMIIRXER = in Bool
val EMIOENET2GMIIRXDV = in Bool
val EMIOENET2GMIITXCLK = in Bool
val EMIOENET2GMIITXD = in Bool
val EMIOENET2GMIITXEN = in Bool
val EMIOENET2GMIITXER = in Bool
val EMIOENET2MDIOMDC = in Bool
val EMIOENET2MDIOI = in Bool
val EMIOENET2MDIOO = in Bool
val EMIOENET2MDIOTN = in Bool
val EMIOENET3GMIIRXCLK = in Bool
val EMIOENET3SPEEDMODE = in Bool
val EMIOENET3GMIICRS = in Bool
val EMIOENET3GMIICOL = in Bool
val EMIOENET3GMIIRXD = in Bool
val EMIOENET3GMIIRXER = in Bool
val EMIOENET3GMIIRXDV = in Bool
val EMIOENET3GMIITXCLK = in Bool
val EMIOENET3GMIITXD = in Bool
val EMIOENET3GMIITXEN = in Bool
val EMIOENET3GMIITXER = in Bool
val EMIOENET3MDIOMDC = in Bool
val EMIOENET3MDIOI = in Bool
val EMIOENET3MDIOO = in Bool
val EMIOENET3MDIOTN = in Bool
val EMIOENET0TXRDATARDY = in Bool
val EMIOENET0TXRRD = in Bool
val EMIOENET0TXRVALID = in Bool
val EMIOENET0TXRDATA = in Bool
val EMIOENET0TXRSOP = in Bool
val EMIOENET0TXREOP = in Bool
val EMIOENET0TXRERR = in Bool
val EMIOENET0TXRUNDERFLOW = in Bool
val EMIOENET0TXRFLUSHED = in Bool
val EMIOENET0TXRCONTROL = in Bool
val EMIOENET0DMATXENDTOG = in Bool
val EMIOENET0DMATXSTATUSTOG = in Bool
val EMIOENET0TXRSTATUS = in Bool
val EMIOENET0RXWWR = in Bool
val EMIOENET0RXWDATA = in Bool
val EMIOENET0RXWSOP = in Bool
val EMIOENET0RXWEOP = in Bool
val EMIOENET0RXWSTATUS = in Bool
val EMIOENET0RXWERR = in Bool
val EMIOENET0RXWOVERFLOW = in Bool
val FMIOGEM0SIGNALDETECT = in Bool
val EMIOENET0RXWFLUSH = in Bool
val EMIOGEM0TXRFIXEDLAT = in Bool
val FMIOGEM0FIFOTXCLKFROMPL = in Bool
val FMIOGEM0FIFORXCLKFROMPL = in Bool
val FMIOGEM0FIFOTXCLKTOPLBUFG = in Bool
val FMIOGEM0FIFORXCLKTOPLBUFG = in Bool
val EMIOENET1TXRDATARDY = in Bool
val EMIOENET1TXRRD = in Bool
val EMIOENET1TXRVALID = in Bool
val EMIOENET1TXRDATA = in Bool
val EMIOENET1TXRSOP = in Bool
val EMIOENET1TXREOP = in Bool
val EMIOENET1TXRERR = in Bool
val EMIOENET1TXRUNDERFLOW = in Bool
val EMIOENET1TXRFLUSHED = in Bool
val EMIOENET1TXRCONTROL = in Bool
val EMIOENET1DMATXENDTOG = in Bool
val EMIOENET1DMATXSTATUSTOG = in Bool
val EMIOENET1TXRSTATUS = in Bool
val EMIOENET1RXWWR = in Bool
val EMIOENET1RXWDATA = in Bool
val EMIOENET1RXWSOP = in Bool
val EMIOENET1RXWEOP = in Bool
val EMIOENET1RXWSTATUS = in Bool
val EMIOENET1RXWERR = in Bool
val EMIOENET1RXWOVERFLOW = in Bool
val FMIOGEM1SIGNALDETECT = in Bool
val EMIOENET1RXWFLUSH = in Bool
val EMIOGEM1TXRFIXEDLAT = in Bool
val FMIOGEM1FIFOTXCLKFROMPL = in Bool
val FMIOGEM1FIFORXCLKFROMPL = in Bool
val FMIOGEM1FIFOTXCLKTOPLBUFG = in Bool
val FMIOGEM1FIFORXCLKTOPLBUFG = in Bool
val EMIOENET2TXRDATARDY = in Bool
val EMIOENET2TXRRD = in Bool
val EMIOENET2TXRVALID = in Bool
val EMIOENET2TXRDATA = in Bool
val EMIOENET2TXRSOP = in Bool
val EMIOENET2TXREOP = in Bool
val EMIOENET2TXRERR = in Bool
val EMIOENET2TXRUNDERFLOW = in Bool
val EMIOENET2TXRFLUSHED = in Bool
val EMIOENET2TXRCONTROL = in Bool
val EMIOENET2DMATXENDTOG = in Bool
val EMIOENET2DMATXSTATUSTOG = in Bool
val EMIOENET2TXRSTATUS = in Bool
val EMIOENET2RXWWR = in Bool
val EMIOENET2RXWDATA = in Bool
val EMIOENET2RXWSOP = in Bool
val EMIOENET2RXWEOP = in Bool
val EMIOENET2RXWSTATUS = in Bool
val EMIOENET2RXWERR = in Bool
val EMIOENET2RXWOVERFLOW = in Bool
val FMIOGEM2SIGNALDETECT = in Bool
val EMIOENET2RXWFLUSH = in Bool
val EMIOGEM2TXRFIXEDLAT = in Bool
val FMIOGEM2FIFOTXCLKFROMPL = in Bool
val FMIOGEM2FIFORXCLKFROMPL = in Bool
val FMIOGEM2FIFOTXCLKTOPLBUFG = in Bool
val FMIOGEM2FIFORXCLKTOPLBUFG = in Bool
val EMIOENET3TXRDATARDY = in Bool
val EMIOENET3TXRRD = in Bool
val EMIOENET3TXRVALID = in Bool
val EMIOENET3TXRDATA = in Bool
val EMIOENET3TXRSOP = in Bool
val EMIOENET3TXREOP = in Bool
val EMIOENET3TXRERR = in Bool
val EMIOENET3TXRUNDERFLOW = in Bool
val EMIOENET3TXRFLUSHED = in Bool
val EMIOENET3TXRCONTROL = in Bool
val EMIOENET3DMATXENDTOG = in Bool
val EMIOENET3DMATXSTATUSTOG = in Bool
val EMIOENET3TXRSTATUS = in Bool
val EMIOENET3RXWWR = in Bool
val EMIOENET3RXWDATA = in Bool
val EMIOENET3RXWSOP = in Bool
val EMIOENET3RXWEOP = in Bool
val EMIOENET3RXWSTATUS = in Bool
val EMIOENET3RXWERR = in Bool
val EMIOENET3RXWOVERFLOW = in Bool
val FMIOGEM3SIGNALDETECT = in Bool
val EMIOENET3RXWFLUSH = in Bool
val EMIOGEM3TXRFIXEDLAT = in Bool
val FMIOGEM3FIFOTXCLKFROMPL = in Bool
val FMIOGEM3FIFORXCLKFROMPL = in Bool
val FMIOGEM3FIFOTXCLKTOPLBUFG = in Bool
val FMIOGEM3FIFORXCLKTOPLBUFG = in Bool
val EMIOGEM0TXSOF = in Bool
val EMIOGEM0SYNCFRAMETX = in Bool
val EMIOGEM0DELAYREQTX = in Bool
val EMIOGEM0PDELAYREQTX = in Bool
val EMIOGEM0PDELAYRESPTX = in Bool
val EMIOGEM0RXSOF = in Bool
val EMIOGEM0SYNCFRAMERX = in Bool
val EMIOGEM0DELAYREQRX = in Bool
val EMIOGEM0PDELAYREQRX = in Bool
val EMIOGEM0PDELAYRESPRX = in Bool
val EMIOGEM0TSUINCCTRL = in Bool
val EMIOGEM0TSUTIMERCMPVAL = in Bool
val EMIOGEM1TXSOF = in Bool
val EMIOGEM1SYNCFRAMETX = in Bool
val EMIOGEM1DELAYREQTX = in Bool
val EMIOGEM1PDELAYREQTX = in Bool
val EMIOGEM1PDELAYRESPTX = in Bool
val EMIOGEM1RXSOF = in Bool
val EMIOGEM1SYNCFRAMERX = in Bool
val EMIOGEM1DELAYREQRX = in Bool
val EMIOGEM1PDELAYREQRX = in Bool
val EMIOGEM1PDELAYRESPRX = in Bool
val EMIOGEM1TSUINCCTRL = in Bool
val EMIOGEM1TSUTIMERCMPVAL = in Bool
val EMIOGEM2TXSOF = in Bool
val EMIOGEM2SYNCFRAMETX = in Bool
val EMIOGEM2DELAYREQTX = in Bool
val EMIOGEM2PDELAYREQTX = in Bool
val EMIOGEM2PDELAYRESPTX = in Bool
val EMIOGEM2RXSOF = in Bool
val EMIOGEM2SYNCFRAMERX = in Bool
val EMIOGEM2DELAYREQRX = in Bool
val EMIOGEM2PDELAYREQRX = in Bool
val EMIOGEM2PDELAYRESPRX = in Bool
val EMIOGEM2TSUINCCTRL = in Bool
val EMIOGEM2TSUTIMERCMPVAL = in Bool
val EMIOGEM3TXSOF = in Bool
val EMIOGEM3SYNCFRAMETX = in Bool
val EMIOGEM3DELAYREQTX = in Bool
val EMIOGEM3PDELAYREQTX = in Bool
val EMIOGEM3PDELAYRESPTX = in Bool
val EMIOGEM3RXSOF = in Bool
val EMIOGEM3SYNCFRAMERX = in Bool
val EMIOGEM3DELAYREQRX = in Bool
val EMIOGEM3PDELAYREQRX = in Bool
val EMIOGEM3PDELAYRESPRX = in Bool
val EMIOGEM3TSUINCCTRL = in Bool
val EMIOGEM3TSUTIMERCMPVAL = in Bool
val FMIOGEMTSUCLKFROMPL = in Bool
val FMIOGEMTSUCLKTOPLBUFG = in Bool
val EMIOENETTSUCLK = in Bool
val EMIOENET0GEMTSUTIMERCNT = in Bool
val EMIOENET0EXTINTIN = in Bool
val EMIOENET1EXTINTIN = in Bool
val EMIOENET2EXTINTIN = in Bool
val EMIOENET3EXTINTIN = in Bool
val EMIOENET0DMABUSWIDTH = in Bool
val EMIOENET1DMABUSWIDTH = in Bool
val EMIOENET2DMABUSWIDTH = in Bool
val EMIOENET3DMABUSWIDTH = in Bool
val EMIOGPIOI = in Bool
val EMIOGPIOO = in Bool
val EMIOGPIOTN = in Bool
val EMIOI2C0SCLI = in Bool
val EMIOI2C0SCLO = in Bool
val EMIOI2C0SCLTN = in Bool
val EMIOI2C0SDAI = in Bool
val EMIOI2C0SDAO = in Bool
val EMIOI2C0SDATN = in Bool
val EMIOI2C1SCLI = in Bool
val EMIOI2C1SCLO = in Bool
val EMIOI2C1SCLTN = in Bool
val EMIOI2C1SDAI = in Bool
val EMIOI2C1SDAO = in Bool
val EMIOI2C1SDATN = in Bool
val EMIOUART0TX = in Bool
val EMIOUART0RX = in Bool
val EMIOUART0CTSN = in Bool
val EMIOUART0RTSN = in Bool
val EMIOUART0DSRN = in Bool
val EMIOUART0DCDN = in Bool
val EMIOUART0RIN = in Bool
val EMIOUART0DTRN = in Bool
val EMIOUART1TX = in Bool
val EMIOUART1RX = in Bool
val EMIOUART1CTSN = in Bool
val EMIOUART1RTSN = in Bool
val EMIOUART1DSRN = in Bool
val EMIOUART1DCDN = in Bool
val EMIOUART1RIN = in Bool
val EMIOUART1DTRN = in Bool
val EMIOSDIO0CLKOUT = in Bool
val EMIOSDIO0FBCLKIN = in Bool
val EMIOSDIO0CMDOUT = in Bool
val EMIOSDIO0CMDIN = in Bool
val EMIOSDIO0CMDENA = in Bool
val EMIOSDIO0DATAIN = in Bool
val EMIOSDIO0DATAOUT = in Bool
val EMIOSDIO0DATAENA = in Bool
val EMIOSDIO0CDN = in Bool
val EMIOSDIO0WP = in Bool
val EMIOSDIO0LEDCONTROL = in Bool
val EMIOSDIO0BUSPOWER = in Bool
val EMIOSDIO0BUSVOLT = in Bool
val EMIOSDIO1CLKOUT = in Bool
val EMIOSDIO1FBCLKIN = in Bool
val EMIOSDIO1CMDOUT = in Bool
val EMIOSDIO1CMDIN = in Bool
val EMIOSDIO1CMDENA = in Bool
val EMIOSDIO1DATAIN = in Bool
val EMIOSDIO1DATAOUT = in Bool
val EMIOSDIO1DATAENA = in Bool
val EMIOSDIO1CDN = in Bool
val EMIOSDIO1WP = in Bool
val EMIOSDIO1LEDCONTROL = in Bool
val EMIOSDIO1BUSPOWER = in Bool
val EMIOSDIO1BUSVOLT = in Bool
val EMIOSPI0SCLKI = in Bool
val EMIOSPI0SCLKO = in Bool
val EMIOSPI0SCLKTN = in Bool
val EMIOSPI0MI = in Bool
val EMIOSPI0MO = in Bool
val EMIOSPI0MOTN = in Bool
val EMIOSPI0SI = in Bool
val EMIOSPI0SO = in Bool
val EMIOSPI0STN = in Bool
val EMIOSPI0SSIN = in Bool
val EMIOSPI0SSON = in Boolemio_spi0_ss1_o_n,emio_spi0_ss_o_n}),
val EMIOSPI0SSNTN = in Bool
val EMIOSPI1SCLKI = in Bool
val EMIOSPI1SCLKO = in Bool
val EMIOSPI1SCLKTN = in Bool
val EMIOSPI1MI = in Bool
val EMIOSPI1MO = in Bool
val EMIOSPI1MOTN = in Bool
val EMIOSPI1SI = in Bool
val EMIOSPI1SO = in Bool
val EMIOSPI1STN = in Bool
val EMIOSPI1SSIN = in Bool
val EMIOSPI1SSON = in Boolemio_spi1_ss1_o_n,emio_spi1_ss_o_n}),
val EMIOSPI1SSNTN = in Bool
val PLPSTRACECLK = in Bool
val PSPLTRACECTL = in Bool
val PSPLTRACEDATA = in Bool
val EMIOTTC0WAVEO = in Bool
val EMIOTTC0CLKI = in Bool
val EMIOTTC1WAVEO = in Bool
val EMIOTTC1CLKI = in Bool
val EMIOTTC2WAVEO = in Bool
val EMIOTTC2CLKI = in Bool
val EMIOTTC3WAVEO = in Bool
val EMIOTTC3CLKI = in Bool
val EMIOWDT0CLKI = in Bool
val EMIOWDT0RSTO = in Bool
val EMIOWDT1CLKI = in Bool
val EMIOWDT1RSTO = in Bool
val EMIOHUBPORTOVERCRNTUSB30 = in Bool
val EMIOHUBPORTOVERCRNTUSB31 = in Bool
val EMIOHUBPORTOVERCRNTUSB20 = in Bool
val EMIOHUBPORTOVERCRNTUSB21 = in Bool
val EMIOU2DSPORTVBUSCTRLUSB30 = in Bool
val EMIOU2DSPORTVBUSCTRLUSB31 = in Bool
val EMIOU3DSPORTVBUSCTRLUSB30 = in Bool
val EMIOU3DSPORTVBUSCTRLUSB31 = in Bool
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
val DPSAXISAUDIOTDATA = in Bool
val DPSAXISAUDIOTID = in Bool
val DPSAXISAUDIOTVALID = in Bool
val DPSAXISAUDIOTREADY = in Bool
val DPMAXISMIXEDAUDIOTDATA = in Bool
val DPMAXISMIXEDAUDIOTID = in Bool
val DPMAXISMIXEDAUDIOTVALID = in Bool
val DPMAXISMIXEDAUDIOTREADY = in Bool
val DPSAXISAUDIOCLK = in Bool
val DPLIVEVIDEOINVSYNC = in Bool
val DPLIVEVIDEOINHSYNC = in Bool
val DPLIVEVIDEOINDE = in Bool
val DPLIVEVIDEOINPIXEL1 = in Bool
val DPVIDEOINCLK = in Bool
val DPVIDEOOUTHSYNC = in Bool
val DPVIDEOOUTVSYNC = in Bool
val DPVIDEOOUTPIXEL1 = in Bool
val DPAUXDATAIN = in Bool
val DPAUXDATAOUT = in Bool
val DPAUXDATAOEN = in Bool
val DPLIVEGFXALPHAIN = in Bool
val DPLIVEGFXPIXEL1IN = in Bool
val DPHOTPLUGDETECT = in Bool
val DPEXTERNALCUSTOMEVENT1 = in Bool
val DPEXTERNALCUSTOMEVENT2 = in Bool
val DPEXTERNALVSYNCEVENT = in Bool
val DPLIVEVIDEODEOUT = in Bool
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
val PLACPINACT  = in Bool
val PLCLK  = in Bool
val DPVIDEOREFCLK  = in Bool
val DPAUDIOREFCLK  = in Bool
val PSS_ALTO_CORE_PAD_MGTTXN0OUT  = in Bool
val PSS_ALTO_CORE_PAD_MGTTXN1OUT  = in Bool
val PSS_ALTO_CORE_PAD_MGTTXN2OUT  = in Bool
val PSS_ALTO_CORE_PAD_MGTTXN3OUT  = in Bool
val PSS_ALTO_CORE_PAD_MGTTXP0OUT  = in Bool
val PSS_ALTO_CORE_PAD_MGTTXP1OUT  = in Bool
val PSS_ALTO_CORE_PAD_MGTTXP2OUT  = in Bool
val PSS_ALTO_CORE_PAD_MGTTXP3OUT  = in Bool
val PSS_ALTO_CORE_PAD_PADO  = in Bool
val PSS_ALTO_CORE_PAD_BOOTMODE  = in Bool
val PSS_ALTO_CORE_PAD_CLK  = in Bool
val PSS_ALTO_CORE_PAD_DONEB  = in Bool
val PSS_ALTO_CORE_PAD_DRAMA  = in Bool
val PSS_ALTO_CORE_PAD_DRAMACTN  = in Bool
val PSS_ALTO_CORE_PAD_DRAMALERTN  = in Bool
val PSS_ALTO_CORE_PAD_DRAMBA  = in Bool
val PSS_ALTO_CORE_PAD_DRAMBG  = in Bool
val PSS_ALTO_CORE_PAD_DRAMCK  = in Bool
val PSS_ALTO_CORE_PAD_DRAMCKE  = in Bool
val PSS_ALTO_CORE_PAD_DRAMCKN  = in Bool
val PSS_ALTO_CORE_PAD_DRAMCSN  = in Bool
val PSS_ALTO_CORE_PAD_DRAMDM  = in Bool
val PSS_ALTO_CORE_PAD_DRAMDQ  = in Bool
val PSS_ALTO_CORE_PAD_DRAMDQS  = in Bool
val PSS_ALTO_CORE_PAD_DRAMDQSN  = in Bool
val PSS_ALTO_CORE_PAD_DRAMODT  = in Bool
val PSS_ALTO_CORE_PAD_DRAMPARITY  = in Bool
val PSS_ALTO_CORE_PAD_DRAMRAMRSTN  = in Bool
val PSS_ALTO_CORE_PAD_ERROROUT  = in Bool
val PSS_ALTO_CORE_PAD_ERRORSTATUS  = in Bool
val PSS_ALTO_CORE_PAD_INITB  = in Bool
val PSS_ALTO_CORE_PAD_JTAGTCK  = in Bool
val PSS_ALTO_CORE_PAD_JTAGTDI  = in Bool
val PSS_ALTO_CORE_PAD_JTAGTDO  = in Bool
val PSS_ALTO_CORE_PAD_JTAGTMS  = in Bool
val PSS_ALTO_CORE_PAD_MIO  = in Bool
val PSS_ALTO_CORE_PAD_PORB  = in Bool
val PSS_ALTO_CORE_PAD_PROGB  = in Bool
val PSS_ALTO_CORE_PAD_RCALIBINOUT  = in Bool
val PSS_ALTO_CORE_PAD_SRSTB  = in Bool
val PSS_ALTO_CORE_PAD_ZQ  = in Bool
val PSS_ALTO_CORE_PAD_MGTRXN0IN  = in Bool
val PSS_ALTO_CORE_PAD_MGTRXN1IN  = in Bool
val PSS_ALTO_CORE_PAD_MGTRXN2IN  = in Bool
val PSS_ALTO_CORE_PAD_MGTRXN3IN  = in Bool
val PSS_ALTO_CORE_PAD_MGTRXP0IN  = in Bool
val PSS_ALTO_CORE_PAD_MGTRXP1IN  = in Bool
val PSS_ALTO_CORE_PAD_MGTRXP2IN  = in Bool
val PSS_ALTO_CORE_PAD_MGTRXP3IN  = in Bool
val PSS_ALTO_CORE_PAD_PADI  = in Bool
val PSS_ALTO_CORE_PAD_REFN0IN  = in Bool
val PSS_ALTO_CORE_PAD_REFN1IN  = in Bool
val PSS_ALTO_CORE_PAD_REFN2IN  = in Bool
val PSS_ALTO_CORE_PAD_REFN3IN  = in Bool
val PSS_ALTO_CORE_PAD_REFP0IN  = in Bool
val PSS_ALTO_CORE_PAD_REFP1IN  = in Bool
val PSS_ALTO_CORE_PAD_REFP2IN  = in Bool
val PSS_ALTO_CORE_PAD_REFP3IN  = input Bool
*/