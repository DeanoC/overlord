import ikuy_utils._

import java.nio.file.Path
import scala.collection.mutable.ArrayBuffer

object UartApbCtrl {
	var addressWidth : Int = 20
	var dataWidth    : Int = 32
	var useSlaveError: Int = 1

	var dataWidthMax     : Int = 8
	var clockDividerWidth: Int = 20
	var preSamplingSize  : Int = 1
	var samplingSize     : Int = 5
	var postSamplingSize : Int = 2

	var txFifoDepth = 16
	var rxFifoDepth = 16

	def main(args: Array[String]): Unit = {
		println(s"Building Uart Apb Ctrl")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "UartApbCtrl"

		val table = if (tomlFile.isEmpty) {
			println(s"No toml config file provided, defaults will be used")
			Map[String, Variant]()
		}
		else Utils.readToml(name, Paths.get(tomlFile.get), getClass)


		val luInt  = new Function2[String, Int, Int] {
			override def apply(k: String, default: Int): Int =
				Utils.lookupInt(table, k, default)
		}
		val luBInt = new Function2[String, BigInt, BigInt] {
			override def apply(k: String, default: BigInt): BigInt =
				Utils.lookupBigInt(table, k, default)
		}

		addressWidth = luInt("address_width", addressWidth)
		dataWidth = luInt("data_width", dataWidth)
		useSlaveError = luInt("use_slave_error", useSlaveError)

		dataWidthMax = luInt("data_width_max", dataWidthMax)
		clockDividerWidth = luInt("clock_divider_width", clockDividerWidth)
		preSamplingSize = luInt("presampling_size", preSamplingSize)
		samplingSize = luInt("sampling_size", samplingSize)
		postSamplingSize = luInt("postsampling_size", postSamplingSize)
		txFifoDepth = luInt("tx_fifo_depth", txFifoDepth)
		rxFifoDepth = luInt("rx_fifo_depth", rxFifoDepth)

		val config = SpinalConfig(
			defaultConfigForClockDomains =
				ClockDomainConfig(resetKind = spinal.core.SYNC),
			targetDirectory = targetDir,
			netlistFileName = name + ".v"
			)

		val regsToml = writeRegistersToToml
		Utils.writeFile(Paths.get(targetDir).resolve(s"${name}_regs.toml"),
		                regsToml)

		config
			.withPrivateNamespace
			.generateVerilog(
				UartApbCtrl().setDefinitionName(name)
				)
			.printPrunedIo()
	}

	val busConfig = Apb3Config(addressWidth,
	                           dataWidth,
	                           selWidth = 1,
	                           useSlaveError == 1)

	case class UartApbCtrl()
		extends Component {

		val io = new Bundle {
			val apb       = slave(Apb3(busConfig))
			val uart      = master(Uart())
			val interrupt = out Bool
		}
		noIoPrefix()

		val config = UartCtrlMemoryMappedConfig(
			uartCtrlConfig = UartCtrlGenerics(
				dataWidthMax, clockDividerWidth,
				preSamplingSize, samplingSize, postSamplingSize),
			txFifoDepth = txFifoDepth,
			rxFifoDepth = rxFifoDepth
			)

		val uartCtrl = new UartCtrl(config.uartCtrlConfig)

		val busCtrl = Apb3SlaveFactory(io.apb)
		val bridge  = uartCtrl.driveFrom(busCtrl, config)


		io.interrupt := bridge.interruptCtrl.interrupt
		io.uart <> uartCtrl.io.uart
	}

	private def writeRegistersToToml = {
		// there is no data from spinal Slave Factory
		// about what each register does, so we have to code
		// it here manually

		import toml.Node
		import toml.Node._
		import toml.Value.{Arr, Num, Str, Tbl}

		val items = ArrayBuffer[Node](
			Pair("description", Str("APB Uart Control registers")),
			NamedArray(List("register"),
			           List(("name", Str("DATA")),
			                ("type", Str("rw")),
			                ("width", Num(32)),
			                ("description", Str("Data")),
			                ("default", Str("0")),
			                ("offset", Str("0x0"))
			                )
			           ),
			NamedArray(List("register"),
			           List(("name", Str("STATUS")),
			                ("type", Str("ro")),
			                ("width", Num(32)),
			                ("default", Str("0")),
			                ("offset", Str("0x4")),
			                ("field", Arr(List(
				                Tbl(Map(
					                ("name", Str("RESERVED")),
					                ("bits", Num(16))
					                )),
				                Tbl(Map(
					                ("name", Str("WRITE_AVAIL")),
					                ("bits", Str("8")),
					                ("type", Str("r")),
					                ("shortdesc", Str("Tx Fifo availibility"))
					                )),
				                Tbl(Map(
					                ("name", Str("READ_OCCUP")),
					                ("bits", Str("8")),
					                ("type", Str("r")),
					                ("shortdesc", Str("Rx Fifo occupancy"))
					                ))
				                ))
			                )
			                )
			           ),
			NamedArray(List("register"),
			           List(("name", Str("CLOCK_DIVIDER")),
			                ("type", Str("rw")),
			                ("width", Num(32)),
			                ("default", Str("0")),
			                ("offset", Str("0x8")),
			                ("field", Arr(List(
				                Tbl(Map(
					                ("name", Str("DIVIDER")),
					                ("bits", Str(clockDividerWidth.toString)),
					                ("type", Str("rw")),
					                ("shortdesc", Str("Divider for uart comms"))
					                )),
				                Tbl(Map(
					                ("name", Str("RESERVED")),
					                ("bits", Str((32 - clockDividerWidth).toString))
					                ))
				                ))
			                ))
			           ),
			NamedArray(List("register"),
			           List(("name", Str("FRAME_CONFIG")),
			                ("type", Str("rw")),
			                ("width", Num(32)),
			                ("default", Str("0")),
			                ("offset", Str("0xC")),
			                ("field", Arr(List(
				                Tbl(Map(
					                ("name", Str("DATA_LENGTH")),
					                ("bits", Str(dataWidthMax.toString)),
					                ("type", Str("rw")),
					                ("shortdesc", Str("UART data bits -1"))
					                )),
				                Tbl(Map(
					                ("name", Str("PARITY")),
					                ("bits", Num(8)),
					                ("type", Str("rw")),
					                ("shortdesc", Str("Parity Bits for uart comms")),
					                ("longdesc", Str(
						                """NONE = 0
							                |EVEN = 1
							                |ODD = 2""".stripMargin))
					                ))
				                ,
				                Tbl(Map(
					                ("name", Str("STOP")),
					                ("bits", Num(8)),
					                ("type", Str("rw")),
					                ("shortdesc", Str("Stop Bits for uart comms")),
					                ("longdesc", Str(
						                """ONE = 0
							                |TWO = 1
							                |""".stripMargin))
					                ))
				                ))
			                ))
			           )
			)

		val root = toml.Root(items.toList)

		toml.Toml.generate(root)
	}
}