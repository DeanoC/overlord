import ikuy_utils._

import java.nio.file.Path

object PMBToAPBBridge {

	var apbAddressWidth : Int = 20
	var apbDataWidth    : Int = 32
	var apbUseSlaveError: Int = 1
	var apbPipeline     : Int = 1

	var pmbAddressWidth: Int = 32
	var pmbDataWidth   : Int = 32

	def main(args: Array[String]): Unit = {
		println(s"Building Spinal PMB to APB Bridge")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "pmb_to_apb_bridge"

		val table = if (tomlFile.isEmpty) {
			println(s"No toml config file provided, defaults will be used")
			Map[String, Variant]()
		}
		else Utils.readToml(name, Path.of(tomlFile.get), getClass)

		val luInt = new Function2[String, Int, Int] {
			override def apply(k: String, default: Int): Int =
				Utils.lookupInt(table, k, default)
		}

		val luBInt = new Function2[String, BigInt, BigInt] {
			override def apply(k: String, default: BigInt): BigInt =
				Utils.lookupBigInt(table, k, default)
		}

		apbAddressWidth = luInt("apb_address_width", apbAddressWidth)
		apbDataWidth = luInt("apb_data_width", apbDataWidth)
		apbUseSlaveError = luInt("apb_use_slave_error", apbUseSlaveError)
		apbPipeline = luInt("apb_pipeline", apbPipeline)

		pmbAddressWidth = luInt("pmb_address_width", pmbAddressWidth)
		pmbDataWidth = luInt("pmb_data_width", pmbDataWidth)

		println(s"bridge to APB of $apbDataWidth bits & $apbAddressWidth address")

		val config = SpinalConfig(
			defaultConfigForClockDomains =
				ClockDomainConfig(resetKind = spinal.core.SYNC),
			targetDirectory = targetDir,
			netlistFileName = name + ".v"
			)

		config
			.withPrivateNamespace
			.generateVerilog {
				PMBToAPB().setDefinitionName(name)
			}.printPrunedIo()
	}

	val pmbBusConfig = PipelinedMemoryBusConfig(pmbAddressWidth, pmbDataWidth)

	val apbBusConfig = Apb3Config(apbAddressWidth,
	                              apbDataWidth,
	                              selWidth = 1,
	                              apbUseSlaveError == 1)

	case class PMBToAPB() extends Component {

		val io = new Bundle {
			val pipelinedMemoryBus = slave(PipelinedMemoryBus(pmbBusConfig))
			val apb                = master(Apb3(apbBusConfig))
		}
		noIoPrefix()

		val apbBridge = new PipelinedMemoryBusToApbBridge(
			apb3Config = apbBusConfig,
			pipelinedMemoryBusConfig = pmbBusConfig,
			pipelineBridge = apbPipeline == 1
			)

		io.pipelinedMemoryBus <> apbBridge.io.pipelinedMemoryBus
		io.apb <> apbBridge.io.apb
	}

}