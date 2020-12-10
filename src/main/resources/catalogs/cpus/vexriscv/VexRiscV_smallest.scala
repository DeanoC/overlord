import vexriscv.plugin._
import vexriscv.{VexRiscv, VexRiscvConfig, plugin}
import spinal.core._

object VexRiscV_smallest {
	def main(args: Array[String]): Unit = {
		println(s"Building VexRiscV smallest cpu")

		val targetDir = if (args.length >= 1) args(0)
		else "."

		val config = SpinalConfig(
			targetDirectory = targetDir,
			netlistFileName = "VexRiscV_smallest.v"
			)

		config.generateVerilog({
			val cpu = GenSmallest.cpu().setDefinitionName("VexRiscV_smallest")
			cpu
		})
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

