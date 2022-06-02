import ikuy_utils._

import java.nio.file.Path


case class ZynqMpPs8Config(psToPlAxiHpmCount: Int = 0,
                           psToPlAxiLpdCount: Int = 0,

                           plToPsAxiHpcCount: Int = 0,
                           plToPsAxiHpCount: Int = 0,
                           plToPsAxiLpdCount: Int = 0,
                           plToPsAxiAcpCount: Int = 0,
                           plToPsAxiAceCount: Int = 0, // TODO not supported yet
                           plToPsDisplayPort: Boolean = false,
                           exposePlToPsAxiCounters: Boolean = false,
                          ) {
	val maxHpmBuses       = 2
	val maxHpcBuses       = 2
	val maxHpBuses        = 4
	val maxPstoPlLpdBuses = 1
	val maxPltoPsLpdBuses = 1
	val maxAcpBuses       = 1
	val maxAceBuses       = 0

	require(psToPlAxiHpmCount >= 0)
	require(psToPlAxiHpmCount <= maxHpmBuses)

	require(plToPsAxiHpcCount >= 0)
	require(plToPsAxiHpcCount <= maxHpcBuses)
	require(plToPsAxiHpCount >= 0)
	require(plToPsAxiHpCount <= maxHpBuses)

	require(psToPlAxiLpdCount <= maxPstoPlLpdBuses)
	require(plToPsAxiLpdCount <= maxPltoPsLpdBuses)
	require(plToPsAxiAcpCount <= maxAcpBuses)
	require(plToPsAxiAceCount <= maxAceBuses)

}

class Axi4Counters extends Bundle {
	val read         = out UInt (8 bits)
	val write        = out UInt (8 bits)
	val addressRead  = out UInt (4 bits)
	val addressWrite = out UInt (4 bits)
}

case class ZynqMpPs8(config: ZynqMpPs8Config) extends Component {
	val io = new Bundle {

		val psToPlAxiHpm = for (i <- 0 until config.psToPlAxiHpmCount) yield new Bundle {
			val clk = out Bool()
			val axi = master(Axi4(PS8.PsToPLConfig))
		}

		val psToPlAxiLpd = for (i <- 0 until config.psToPlAxiLpdCount) yield new Bundle {
			val clk = out Bool()
			val axi = master(Axi4(PS8.PsToPLConfig))
		}

		val plToPsAxiHpc = for (i <- 0 until config.plToPsAxiHpcCount) yield new Bundle {
			val read_clk  = in Bool()
			val write_clk = in Bool()

			val counters = config.exposePlToPsAxiCounters generate (new Axi4Counters)

			val axi = slave(Axi4(PS8.PlToPsConfig))
		}
		val plToPsAxiHp  = for (i <- 0 until config.plToPsAxiHpCount) yield new Bundle {
			val read_clk  = in Bool()
			val write_clk = in Bool()

			val counters = config.exposePlToPsAxiCounters generate (new Axi4Counters)

			val axi = slave(Axi4(PS8.PlToPsConfig))
		}
		val plToPsAxiLpd = for (i <- 0 until config.plToPsAxiLpdCount) yield new Bundle {
			val read_clk  = in Bool()
			val write_clk = in Bool()

			val counters = config.exposePlToPsAxiCounters generate (new Axi4Counters)

			val axi = slave(Axi4(PS8.PlToPsConfig))
		}

		val plToPsAxiAcp = for (i <- 0 until config.plToPsAxiAcpCount) yield new Bundle {
			val clk = in Bool()
			val axi = slave(Axi4(PS8.PlToPsAcpConfig))
		}

		/*	val plToPsAxiAce = config.plToPsAxiAce generate(new Bundle {
				val clk = in Bool()
				val axi = slave( Axi4(zynqMpAxiConfig) )
			})*/
	}

	noIoPrefix()

	val rawPS8 = PS8();

	// ps to pl hookups
	var maxiCount = 0
	for (i <- 0 until config.maxHpmBuses)
		if (i >= config.psToPlAxiHpmCount) capMAXI(i + maxiCount)
		else connectMAXI(i + maxiCount,
		                 io.psToPlAxiHpm(i).clk,
		                 io.psToPlAxiHpm(i).axi)
	maxiCount += config.maxHpmBuses

	for (i <- 0 until config.maxPstoPlLpdBuses)
		if (i >= config.psToPlAxiLpdCount) capMAXI(i + maxiCount)
		else connectMAXI(i + maxiCount,
		                 io.psToPlAxiLpd(i).clk,
		                 io.psToPlAxiLpd(i).axi)
	maxiCount += config.maxPstoPlLpdBuses

	// pl to ps hookups
	var saxiCount = 0
	for (i <- 0 until config.maxHpcBuses)
		if (i >= config.plToPsAxiHpcCount) capSAXI(i + saxiCount)
		else connectSAXI(i + saxiCount,
		                 io.plToPsAxiHpc(i).read_clk,
		                 io.plToPsAxiHpc(i).write_clk,
		                 io.plToPsAxiHpc(i).axi,
		                 io.plToPsAxiHpc(i).counters)
	saxiCount += config.maxHpcBuses

	for (i <- 0 until config.maxHpBuses)
		if (i >= config.plToPsAxiHpCount) capSAXI(i + saxiCount)
		else connectSAXI(i + saxiCount,
		                 io.plToPsAxiHp(i).read_clk,
		                 io.plToPsAxiHp(i).write_clk,
		                 io.plToPsAxiHp(i).axi,
		                 io.plToPsAxiHp(i).counters)
	saxiCount += config.maxHpBuses

	for (i <- 0 until config.maxPltoPsLpdBuses)
		if (i >= config.plToPsAxiLpdCount) capSAXI(i + saxiCount)
		else connectSAXI(i + saxiCount,
		                 io.plToPsAxiLpd(i).read_clk,
		                 io.plToPsAxiLpd(i).write_clk,
		                 io.plToPsAxiLpd(i).axi,
		                 io.plToPsAxiLpd(i).counters)
	saxiCount += config.maxPltoPsLpdBuses

	// hook up or cap FPD ACP interface to saxiacp
	var saciacpCount = 0
	for (i <- 0 until config.maxAcpBuses)
		if (i >= config.plToPsAxiAcpCount) capSAXIACP(i + saciacpCount)
		else connectSAXIACP(i + saciacpCount,
		                    io.plToPsAxiAcp(i).clk,
		                    io.plToPsAxiAcp(i).axi)
	saciacpCount += config.maxAcpBuses

	if (!config.plToPsDisplayPort) {
		rawPS8.io.DP.VIDEOINCLK := False
		rawPS8.io.DP.SAXISAUDIOCLK := False
		rawPS8.io.DP.SAXISAUDIOTDATA := 0
		rawPS8.io.DP.SAXISAUDIOTID := False
		rawPS8.io.DP.SAXISAUDIOTVALID := False
		rawPS8.io.DP.MAXISMIXEDAUDIOTREADY := False
		rawPS8.io.DP.LIVEVIDEOINVSYNC := False
		rawPS8.io.DP.LIVEVIDEOINHSYNC := False
		rawPS8.io.DP.LIVEVIDEOINDE := False
		rawPS8.io.DP.LIVEVIDEOINPIXEL1 := 0
		rawPS8.io.DP.LIVEGFXALPHAIN := 0
		rawPS8.io.DP.LIVEGFXPIXEL1IN := 0
		rawPS8.io.DP.AUXDATAIN := False
		rawPS8.io.DP.HOTPLUGDETECT := False
		rawPS8.io.DP.EXTERNALCUSTOMEVENT1 := False
		rawPS8.io.DP.EXTERNALCUSTOMEVENT2 := False
		rawPS8.io.DP.EXTERNALVSYNCEVENT := False
	}

	private def capMAXI(i: Integer): Unit = {
		rawPS8.io.MAXIGP(i).AWREADY := False
		rawPS8.io.MAXIGP(i).WREADY := False

		rawPS8.io.MAXIGP(i).BID := 0
		rawPS8.io.MAXIGP(i).BRESP := 0
		rawPS8.io.MAXIGP(i).BVALID := False

		rawPS8.io.MAXIGP(i).ARREADY := False

		rawPS8.io.MAXIGP(i).RID := 0
		rawPS8.io.MAXIGP(i).RDATA := 0
		rawPS8.io.MAXIGP(i).RRESP := 0
		rawPS8.io.MAXIGP(i).RLAST := False
		rawPS8.io.MAXIGP(i).RVALID := False
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

		rawPS8.io.SAXIGP(i).RREADY := False

	}

	private def capSAXIACP(i: Integer): Unit = {
		rawPS8.io.SAXIACP(i).ACLK := False

		rawPS8.io.SAXIACP(i).AWID := 0
		rawPS8.io.SAXIACP(i).AWADDR := 0
		rawPS8.io.SAXIACP(i).AWLEN := 0
		rawPS8.io.SAXIACP(i).AWSIZE := 0
		rawPS8.io.SAXIACP(i).AWBURST := 0
		rawPS8.io.SAXIACP(i).AWLOCK := 0
		rawPS8.io.SAXIACP(i).AWCACHE := 0
		rawPS8.io.SAXIACP(i).AWPROT := 0
		rawPS8.io.SAXIACP(i).AWVALID := False
		rawPS8.io.SAXIACP(i).AWQOS := 0

		rawPS8.io.SAXIACP(i).WDATA := 0
		rawPS8.io.SAXIACP(i).WSTRB := 0
		rawPS8.io.SAXIACP(i).WLAST := False
		rawPS8.io.SAXIACP(i).WVALID := False

		rawPS8.io.SAXIACP(i).BREADY := False

		rawPS8.io.SAXIACP(i).ARID := 0
		rawPS8.io.SAXIACP(i).ARADDR := 0
		rawPS8.io.SAXIACP(i).ARLEN := 0
		rawPS8.io.SAXIACP(i).ARSIZE := 0
		rawPS8.io.SAXIACP(i).ARBURST := 0
		rawPS8.io.SAXIACP(i).ARLOCK := 0
		rawPS8.io.SAXIACP(i).ARCACHE := 0
		rawPS8.io.SAXIACP(i).ARPROT := 0
		rawPS8.io.SAXIACP(i).ARVALID := False
		rawPS8.io.SAXIACP(i).ARQOS := 0

		rawPS8.io.SAXIACP(i).RREADY := False
	}

	private def connectMAXI(i: Int,
	                        clk: Bool,
	                        axi: Axi4): Unit = {
		clk := rawPS8.io.MAXIGP(i).ACLK

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

	private def connectSAXI(i: Int,
	                        read_clk: Bool,
	                        write_clk: Bool,
	                        axi: Axi4,
	                        counters: Axi4Counters): Unit = {
		rawPS8.io.SAXIGP(i).RCLK := read_clk
		rawPS8.io.SAXIGP(i).WCLK := write_clk

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

		axi.r.id <> rawPS8.io.SAXIGP(i).RID
		axi.r.data <> rawPS8.io.SAXIGP(i).RDATA
		axi.r.resp <> rawPS8.io.SAXIGP(i).RRESP
		axi.r.last <> rawPS8.io.SAXIGP(i).RLAST
		axi.r.valid <> rawPS8.io.SAXIGP(i).RVALID
		axi.r.ready <> rawPS8.io.SAXIGP(i).RREADY

		if (config.exposePlToPsAxiCounters) {
			counters.read <> rawPS8.io.SAXIGP(i).RCOUNT
			counters.write <> rawPS8.io.SAXIGP(i).WCOUNT
			counters.addressRead <> rawPS8.io.SAXIGP(i).ARCOUNT
			counters.addressWrite <> rawPS8.io.SAXIGP(i).AWCOUNT
		}
	}

	private def connectSAXIACP(i: Int,
	                           clk: Bool,
	                           axi: Axi4): Unit = {
		rawPS8.io.SAXIACP(i).ACLK := clk

		axi.aw.id <> rawPS8.io.SAXIACP(i).AWID
		axi.aw.addr <> rawPS8.io.SAXIACP(i).AWADDR
		axi.aw.len <> rawPS8.io.SAXIACP(i).AWLEN
		axi.aw.size <> rawPS8.io.SAXIACP(i).AWSIZE
		axi.aw.burst <> rawPS8.io.SAXIACP(i).AWBURST
		axi.aw.lock <> rawPS8.io.SAXIACP(i).AWLOCK
		axi.aw.cache <> rawPS8.io.SAXIACP(i).AWCACHE
		axi.aw.prot <> rawPS8.io.SAXIACP(i).AWPROT
		axi.aw.valid <> rawPS8.io.SAXIACP(i).AWVALID
		axi.aw.ready <> rawPS8.io.SAXIACP(i).AWREADY
		axi.aw.qos <> rawPS8.io.SAXIACP(i).AWQOS

		axi.w.data <> rawPS8.io.SAXIACP(i).WDATA
		axi.w.strb <> rawPS8.io.SAXIACP(i).WSTRB
		axi.w.last <> rawPS8.io.SAXIACP(i).WLAST
		axi.w.valid <> rawPS8.io.SAXIACP(i).WVALID
		axi.w.ready <> rawPS8.io.SAXIACP(i).WREADY

		axi.b.id <> rawPS8.io.SAXIACP(i).BID
		axi.b.resp <> rawPS8.io.SAXIACP(i).BRESP
		axi.b.valid <> rawPS8.io.SAXIACP(i).BVALID
		axi.b.ready <> rawPS8.io.SAXIACP(i).BREADY

		axi.ar.id <> rawPS8.io.SAXIACP(i).ARID
		axi.ar.addr <> rawPS8.io.SAXIACP(i).ARADDR
		axi.ar.len <> rawPS8.io.SAXIACP(i).ARLEN
		axi.ar.size <> rawPS8.io.SAXIACP(i).ARSIZE
		axi.ar.burst <> rawPS8.io.SAXIACP(i).ARBURST
		axi.ar.lock <> rawPS8.io.SAXIACP(i).ARLOCK
		axi.ar.cache <> rawPS8.io.SAXIACP(i).ARCACHE
		axi.ar.prot <> rawPS8.io.SAXIACP(i).ARPROT
		axi.ar.valid <> rawPS8.io.SAXIACP(i).ARVALID
		axi.ar.ready <> rawPS8.io.SAXIACP(i).ARREADY
		axi.ar.qos <> rawPS8.io.SAXIACP(i).ARQOS

		axi.r.id <> rawPS8.io.SAXIACP(i).RID
		axi.r.data <> rawPS8.io.SAXIACP(i).RDATA
		axi.r.resp <> rawPS8.io.SAXIACP(i).RRESP
		axi.r.last <> rawPS8.io.SAXIACP(i).RLAST
		axi.r.valid <> rawPS8.io.SAXIACP(i).RVALID
		axi.r.ready <> rawPS8.io.SAXIACP(i).RREADY
	}

	// Function used to rename all signals of the blackbox
	private def renameIO(): Unit = {
		io.flatten.foreach(bt => {
			if (bt.getName().contains("_axi")) bt.setName(bt.getName().replace("_axi", ""))
		})
	}

	// Execute the function renameIO after the creation of the component
	addPrePopTask(() => renameIO())

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
		}
		else Utils.readToml(Path.of(tomlFile.get))

		println(Path.of(tomlFile.get).toString)

		val luInt      = new Function2[String, Int, Int] {
			override def apply(k: String, default: Int): Int =
				Utils.lookupInt(table, k, default)
		}
		val luBInt     = new Function2[String, BigInt, BigInt] {
			override def apply(k: String, default: BigInt): BigInt =
				Utils.lookupBigInt(table, k, default)
		}
		val luBusArray = new Function1[String, Array[(BigInt, BigInt)]] {
			override def apply(k: String): Array[(BigInt, BigInt)] =
				if (!table.contains(k)) {
					Array[(BigInt, BigInt)]()
				}
				else {
					val cons = Utils.toArray(table(k))
					for (i <- 0 until cons.length by 2) yield
						(Utils.toBigInt(cons(i)), Utils.toBigInt(cons(i + 1)))
				}.toArray
		}

		val hpmSupplierBuses = luBusArray("hpm_suppliers")
		val lpdSupplierBuses = luBusArray("lpd_suppliers")

		val hpcConsumerBuses = luBusArray("hpc_consumers")
		val hpConsumerBuses  = luBusArray("hp_consumers")
		val lpdConsumerBuses = luBusArray("lpd_consumers")
		val acpConsumerBuses = luBusArray("acp_consumers")

		val ps8Config = ZynqMpPs8Config(
			psToPlAxiHpmCount = 1, //hpmSupplierBuses.length,
			psToPlAxiLpdCount = lpdSupplierBuses.length,

			plToPsAxiHpcCount = hpcConsumerBuses.length,
			plToPsAxiHpCount = hpConsumerBuses.length,
			plToPsAxiLpdCount = lpdConsumerBuses.length,
			plToPsAxiAcpCount = acpConsumerBuses.length,
			plToPsDisplayPort = false,
			exposePlToPsAxiCounters = false,
			)


		// setup spinalHDL
		val spinalConfig = SpinalConfig(
			defaultConfigForClockDomains =
				ClockDomainConfig(resetKind = spinal.core.SYNC),
			targetDirectory = targetDir,
			netlistFileName = name + ".v"
			)

		// generate verilog
		val report = spinalConfig
			.generateVerilog {
				ZynqMpPs8(ps8Config)
					.setDefinitionName(name)
			}
		//		report.printPrunedIo()
		//		report.printRtl()
	}

}