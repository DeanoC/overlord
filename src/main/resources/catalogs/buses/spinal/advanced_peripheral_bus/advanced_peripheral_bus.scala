import toml.Value
import spinal.core._
import spinal.lib._
import spinal.lib.bus.misc.SizeMapping
import spinal.lib.bus.amba3.apb._
import ikuy_utils._
import java.nio.file.Path

object AdvancedPeripheralBus {

	var consumerCount  : Int    = 1
	var addressWidth   : Int    = 20
	var dataWidth      : Int    = 32
	var useSlaveError  : Int    = 1
	var baseAddress    : BigInt = 0x00000l
	var addressBankSize: BigInt = 4096l

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

		val luInt  = new Function2[String, Int, Int] {
			override def apply(k: String, default: Int): Int =
				Utils.lookupInt(table, k, default)
		}
		val luBInt = new Function2[String, BigInt, BigInt] {
			override def apply(k: String, default: BigInt): BigInt =
				Utils.lookupBigInt(table, k, default)
		}

		consumerCount = luInt("bus_count", consumerCount)
		dataWidth = luInt("bus_data_width", dataWidth)
		addressWidth = luInt("bus_address_width", addressWidth)
		baseAddress = luBInt("bus_base", baseAddress)
		addressBankSize = luBInt("bus_bank_size", addressBankSize)

		useSlaveError = luInt("use_slave_error", useSlaveError)

		println(s"bus has $dataWidth data with $addressWidth bit address")
		println(s"$consumerCount consumers attached to bus")

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
			val consumers = Array.fill(consumerCount) {
				master(Apb3(busConfig))
			}
		}
		noIoPrefix()

		val tmp = for (i <- 0 until consumerCount) yield Apb3(busConfig)

		val busMapping = for (i <- 0 until consumerCount) yield
			(tmp(i), SizeMapping(baseAddress + i * addressBankSize, addressBankSize))

		Apb3Decoder(io.supplier, busMapping)

		for (i <- 0 until consumerCount) tmp(i) <> io.consumers(i)
	}

}