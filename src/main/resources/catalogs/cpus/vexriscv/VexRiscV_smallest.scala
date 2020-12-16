import vexriscv.plugin._
import vexriscv.{VexRiscv, VexRiscvConfig, plugin}
import spinal.core._
import ikuy_utils.Utils
import toml.Value

object VexRiscV_smallest {
	def main(args: Array[String]): Unit = {
		println(s"Building VexRiscV smallest cpu")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "VexRiscv"

		val table = if (tomlFile.isEmpty) {
			println(s"No toml config file provided, defaults will be used")
			Map[String, Value]()
		}
		else {
			println(s"Reading $tomlFile for pmb_bram config")
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

		val config = SpinalConfig(
			targetDirectory = targetDir,
			netlistFileName = name + ".v"
			)

		config
			.withPrivateNamespace
			.generateVerilog {
				GenSmallest.cpu().setDefinitionName(name)
			}
	}

	object GenSmallest {
		def cpu() = new VexRiscv(
			config = VexRiscvConfig(
				plugins = List(
					new IBusSimplePlugin(
						resetVector = 0x80000000l,
						cmdForkOnSecondStage = false,
						cmdForkPersistence = false,
						prediction = NONE,
						catchAccessFault = false,
						compressedGen = false
						),
					new DBusSimplePlugin(
						catchAddressMisaligned = false,
						catchAccessFault = false
						),
					new CsrPlugin(CsrPluginConfig.smallest),
					new DecoderSimplePlugin(
						catchIllegalInstruction = false
						),
					new RegFilePlugin(
						regFileReadyKind = plugin.SYNC,
						zeroBoot = false
						),
					new IntAluPlugin,
					new SrcPlugin(
						separatedAddSub = false,
						executeInsertion = false
						),
					new LightShifterPlugin,
					new HazardSimplePlugin(
						bypassExecute = false,
						bypassMemory = false,
						bypassWriteBack = false,
						bypassWriteBackBuffer = false,
						pessimisticUseSrc = false,
						pessimisticWriteRegFile = false,
						pessimisticAddressMatch = false
						),
					new BranchPlugin(
						earlyBranch = false,
						catchAddressMisaligned = false
						),
					new YamlPlugin("cpu0.yaml")
					)
				)
			)
	}

}

