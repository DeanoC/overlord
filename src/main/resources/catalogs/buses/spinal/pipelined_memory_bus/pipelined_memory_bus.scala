import toml.Value
import spinal.core._
import spinal.lib._
import spinal.lib.bus.misc.SizeMapping
import spinal.lib.bus.simple.{PipelinedMemoryBus => pmb}
import spinal.lib.bus.simple.PipelinedMemoryBusConfig
import ikuy_utils._
import java.nio.file.Path

object PipelinedMemoryBus {

	private var instructionWidth: Int = 32
	private var iBusDataWidth   : Int = 32
	private var iBusAddressWidth: Int = 32
	private var dBusDataWidth   : Int = 32
	private var dBusAddressWidth: Int = 32
	private var busDataWidth    : Int = 32
	private var busAddressWidth : Int = 32

	private var consumerBuses = Array[(BigInt, BigInt)]()

	def main(args: Array[String]): Unit = {
		println(s"Building Spinal Pipelined Memory Bus")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "pipelined_memory_bus"

		val table = if (tomlFile.isEmpty) {
			println(s"No toml config file provided, defaults will be used")
			Map[String, Variant]()
		}
		else Utils.readToml(name, Path.of(tomlFile.get), getClass)

		val luInt  = new Function2[String, Int, Int] {
			override def apply(k: String, default: Int): Int =
				Utils.lookupInt(table, k, default)
		}
		val luBInt = new Function2[String, BigInt, BigInt] {
			override def apply(k: String, default: BigInt): BigInt =
				Utils.lookupBigInt(table, k, default)
		}

		busDataWidth = luInt("bus_data_width", busDataWidth)
		busAddressWidth = luInt("bus_address_width", busAddressWidth)
		consumerBuses = {
			val cons = Utils.toArray(table("consumers"))
			for (i <- 0 until cons.length by 2) yield
				(Utils.toBigInt(cons(i)), Utils.toBigInt(cons(i + 1)))
		}.toArray

		iBusDataWidth = luInt("ibus_data_width", busDataWidth)
		iBusAddressWidth = luInt("ibus_address_width", busAddressWidth)
		instructionWidth = luInt("instruction_width", iBusAddressWidth)
		dBusDataWidth = luInt("dbus_data_width", busDataWidth)
		dBusAddressWidth = luInt("dbus_address_width", busAddressWidth)

		if (iBusDataWidth != instructionWidth) {
			println("Currently iBus Data Width and instruction Width must be equal")
			return
		}

		println(s"bus has $busDataWidth data with $busAddressWidth bit address")
		println(s"${consumerBuses.length} consumers attached to bus")

		val config = SpinalConfig(
			defaultConfigForClockDomains =
				ClockDomainConfig(resetKind = spinal.core.SYNC),
			targetDirectory = targetDir,
			netlistFileName = name + ".v"
			)

		config
			.withPrivateNamespace
			.generateVerilog {
				MuraxBus().setDefinitionName(name)
			}.printPrunedIo()
	}

	case class DBusSimpleCmd() extends Bundle {
		val wr     : Bool = Bool
		val address: UInt = UInt(dBusAddressWidth bits)
		val data   : Bits = Bits(dBusDataWidth bit)
		val size   : UInt = UInt(2 bit)
	}

	case class DBusSimpleRsp() extends Bundle with IMasterSlave {
		val ready: Bool = Bool
		val error: Bool = Bool
		val data : Bits = Bits(dBusDataWidth bit)

		override def asMaster(): Unit = {
			out(ready, error, data)
		}
	}

	case class IBusSimpleCmd() extends Bundle {
		val pc = UInt(dBusAddressWidth bits)
	}

	case class IBusSimpleRsp() extends Bundle with IMasterSlave {
		val error: Bool = Bool
		val inst : Bits = Bits(instructionWidth bits)

		override def asMaster(): Unit = {
			out(error, inst)
		}
	}

	object IBusSimpleBus {
		def getPipelinedMemoryBusConfig: PipelinedMemoryBusConfig =
			PipelinedMemoryBusConfig(iBusAddressWidth, iBusDataWidth)
	}

	case class IBusSimpleBus() extends Bundle with IMasterSlave {
		var cmd: Stream[IBusSimpleCmd] = Stream(IBusSimpleCmd())
		var rsp: Flow[IBusSimpleRsp]   = Flow(IBusSimpleRsp())

		override def asMaster(): Unit = {
			master(cmd)
			slave(rsp)
		}

		def cmdS2mPipe(): IBusSimpleBus = {
			val s = IBusSimpleBus()
			s.cmd << this.cmd.s2mPipe()
			this.rsp << s.rsp
			s
		}

		def toPipelinedMemoryBus: pmb = {
			val pipelinedMemoryBusConfig = IBusSimpleBus.getPipelinedMemoryBusConfig

			val bus = pmb(pipelinedMemoryBusConfig)
			bus.cmd.arbitrationFrom(cmd)
			bus.cmd.address := cmd.pc.resized
			bus.cmd.write := False
			bus.cmd.mask.assignDontCare()
			bus.cmd.data.assignDontCare()
			rsp.valid := bus.rsp.valid
			rsp.inst := bus.rsp.payload.data
			rsp.error := False
			bus
		}
	}

	object DBusSimpleBus {
		def getPipelinedMemoryBusConfig: PipelinedMemoryBusConfig =
			PipelinedMemoryBusConfig(dBusAddressWidth, dBusDataWidth)
	}

	case class DBusSimpleBus() extends Bundle with IMasterSlave {
		val cmd: Stream[DBusSimpleCmd] = Stream(DBusSimpleCmd())
		val rsp: DBusSimpleRsp         = DBusSimpleRsp()

		override def asMaster(): Unit = {
			master(cmd)
			slave(rsp)
		}

		def cmdS2mPipe(): DBusSimpleBus = {
			val s = DBusSimpleBus()
			s.cmd << this.cmd.s2mPipe()
			this.rsp := s.rsp
			s
		}

		def genMask(cmd: DBusSimpleCmd): Bits = {
			cmd.size.mux(
				U(0) -> B"0001",
				U(1) -> B"0011",
				default -> B"1111"
				) |<< cmd.address(1 downto 0)
		}

		def toPipelinedMemoryBus: pmb = {
			val pipelinedMemoryBusConfig = DBusSimpleBus.getPipelinedMemoryBusConfig

			val bus = pmb(pipelinedMemoryBusConfig)
			bus.cmd.valid := cmd.valid
			bus.cmd.write := cmd.wr
			bus.cmd.address := cmd.address.resized
			bus.cmd.data := cmd.data
			bus.cmd.mask := genMask(cmd)
			cmd.ready := bus.cmd.ready

			rsp.ready := bus.rsp.valid
			rsp.data := bus.rsp.data

			bus
		}
	}

	val busConfig = PipelinedMemoryBusConfig(busAddressWidth, busDataWidth)

	case class MuraxBus() extends Component {

		val io = new Bundle {
			val iBus      = slave(IBusSimpleBus())
			val dBus      = slave(DBusSimpleBus())
			val consumers = Array.fill(consumerBuses.length) {
				master(pmb(busConfig))
			}
		}
		noIoPrefix()

		val arb = PipelinedMemoryBusArbiter()

		val mainBusMapping = for (i <- 0 until consumerBuses.length) yield
			(pmb(busConfig), SizeMapping(consumerBuses(i)._1, consumerBuses(i)._2))

		PipelinedMemoryBusDecoder(arb.io.bus,
		                          mainBusMapping,
		                          pipelineMaster = true)

		for (i <- 0 until consumerBuses.length)
			mainBusMapping(i)._1 <> io.consumers(i)

		arb.io.iBus <> io.iBus
		arb.io.dBus <> io.dBus
	}

	case class PipelinedMemoryBusArbiter()
		extends Component {

		val io = new Bundle {
			val iBus: IBusSimpleBus = slave(IBusSimpleBus())
			val dBus: DBusSimpleBus = slave(DBusSimpleBus())
			val bus : pmb           = master(pmb(busConfig))
		}
		noIoPrefix()

		io.bus.cmd.valid := io.iBus.cmd.valid || io.dBus.cmd.valid
		io.bus.cmd.write := io.dBus.cmd.valid && io.dBus.cmd.wr
		io.bus.cmd.address :=
		io.dBus.cmd.valid ? io.dBus.cmd.address | io.iBus.cmd.pc
		io.bus.cmd.data := io.dBus.cmd.data
		io.bus.cmd.mask := io.dBus.genMask(io.dBus.cmd)
		io.iBus.cmd.ready := io.bus.cmd.ready && !io.dBus.cmd.valid
		io.dBus.cmd.ready := io.bus.cmd.ready

		val rspPending: Bool = RegInit(False) clearWhen (io.bus.rsp.valid)
		val rspTarget : Bool = RegInit(False)

		when(io.bus.cmd.fire && !io.bus.cmd.write) {
			rspTarget := io.dBus.cmd.valid
			rspPending := True
		}

		when(rspPending && !io.bus.rsp.valid) {
			io.iBus.cmd.ready := False
			io.dBus.cmd.ready := False
			io.bus.cmd.valid := False
		}

		io.iBus.rsp.valid := io.bus.rsp.valid && !rspTarget
		io.iBus.rsp.inst := io.bus.rsp.data
		io.iBus.rsp.error := False

		io.dBus.rsp.ready := io.bus.rsp.valid && rspTarget
		io.dBus.rsp.data := io.bus.rsp.data
		io.dBus.rsp.error := False
	}

	case class PipelinedMemoryBusDecoder(producer: pmb,
	                                     specification: Seq[(pmb, SizeMapping)],
	                                     pipelineMaster: Boolean)
		extends Area {

		val producerPipelined: pmb = pmb(producer.config)
		if (!pipelineMaster) {
			producerPipelined.cmd << producer.cmd
			producerPipelined.rsp >> producer.rsp
		} else {
			producerPipelined.cmd <-< producer.cmd
			producerPipelined.rsp >> producer.rsp
		}

		val (consumers, memorySpaces) = specification.unzip

		val hits: Seq[Bool] =
			for ((slaveBus, memorySpace) <- specification) yield {
				val hit = memorySpace.hit(producerPipelined.cmd.address)
				slaveBus.cmd.valid := producerPipelined.cmd.valid && hit
				slaveBus.cmd.payload := producerPipelined.cmd.payload.resized
				hit
			}

		val noHit: Bool = !hits.orR

		producerPipelined.cmd.ready := (hits, consumers).zipped
			                               .map(_ && _.cmd.ready).orR || noHit

		val rspPending: Bool = RegInit(False)
			.clearWhen(producerPipelined.rsp.valid)
			.setWhen(producerPipelined.cmd.fire && !producerPipelined.cmd.write)

		val rspNoHit   : Bool = RegNext(False) init (False) setWhen (noHit)
		val rspSourceId: UInt =
			RegNextWhen(OHToUInt(hits), producerPipelined.cmd.fire)

		producerPipelined.rsp.valid :=
		consumers.map(_.rsp.valid).orR || (rspPending && rspNoHit)

		producerPipelined.rsp.payload :=
		consumers.map(_.rsp.payload).read(rspSourceId)

		when(rspPending && !producerPipelined.rsp.valid) {
			//Only one pending read request is allowed
			producerPipelined.cmd.ready := False
			consumers.foreach(_.cmd.valid := False)
		}
	}

}