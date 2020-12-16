import spinal.core._
import spinal.lib._
import spinal.lib.bus.amba3.apb._
import spinal.lib.com.uart._
import ikuy_utils.Utils
import toml.Value

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
			Map[String, Value]()
		}
		else {
			println(s"Reading $tomlFile for UartApbCtrl config")
			Utils.readToml(tomlFile.get)
		}

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
			targetDirectory = targetDir,
			netlistFileName = name + ".v"
			)

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

		val io = new Bundle{
			val apb =  slave(Apb3(busConfig))
			val uart = master(Uart())
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
		val bridge = uartCtrl.driveFrom32(busCtrl,config)

		io.interrupt := bridge.interruptCtrl.interrupt
		io.uart <> uartCtrl.io.uart

	}

}