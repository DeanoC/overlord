import toml.Value
import spinal.core._
import spinal.lib._
import spinal.lib.bus.misc.SizeMapping
import spinal.lib.bus.amba3.apb._
import ikuy_utils._
import java.nio.file.Path

object AdvancedPeripheralBus {

	private var addressWidth   : Int    = 20
	private var dataWidth      : Int    = 32
	private var useSlaveError  : Int    = 1
	private var consumerBuses = Array[(BigInt, BigInt)]()

	def main(args: Array[String]): Unit = {
		println(s"Building Spinal Advanced Peripheral Bus")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else
			"advanced_peripheral_bus"

		val table = if (tomlFile.isEmpty) {
			println(s"No toml config file provided, defaults will be used")
			Map[String, Variant]()
		}
		else Utils.readToml(name, Path.of(tomlFile.get), getClass)

		if(!table.contains("consumers")) {
			println("No consumers for bus, nothing to do")
			return
		}

		val luInt  = new Function2[String, Int, Int] {
			override def apply(k: String, default: Int): Int =
				Utils.lookupInt(table, k, default)
		}
		val luBInt = new Function2[String, BigInt, BigInt] {
			override def apply(k: String, default: BigInt): BigInt =
				Utils.lookupBigInt(table, k, default)
		}

		dataWidth = luInt("bus_data_width", dataWidth)
		addressWidth = luInt("bus_address_width", addressWidth)
		consumerBuses = {
			val cons = Utils.toArray(table("consumers"))
			for (i <- 0 until cons.length by 2) yield
				(Utils.toBigInt(cons(i)), Utils.toBigInt(cons(i + 1)))
		}.toArray

		useSlaveError = luInt("use_slave_error", useSlaveError)

		println(s"bus has $dataWidth data with $addressWidth bit address")
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
				APB().setDefinitionName(name)
			}.printPrunedIo()
	}

	val busConfig = Apb3Config(addressWidth,
	                           dataWidth,
	                           selWidth = 1,
	                           useSlaveError == 1)

	case class APB() extends Component {

		val io = new Bundle {
			val supplier  = slave(Apb3(busConfig))
			val consumers = Array.fill(consumerBuses.length) {
				master(Apb3(busConfig))
			}
		}
		noIoPrefix()

		val busMapping = for (i <- 0 until consumerBuses.length) yield
			(Apb3(busConfig), SizeMapping(consumerBuses(i)._1, consumerBuses(i)._2))

		Apb3Decoder(io.supplier, busMapping)

		for (i <- 0 until consumerBuses.length)
			busMapping(i)._1 <> io.consumers(i)
	}

}