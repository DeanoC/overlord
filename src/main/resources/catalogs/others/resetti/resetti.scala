import ikuy_utils._

import java.nio.file.Path
import spinal.core._
import spinal.lib._

object Resetti {
	var resetClocks: Int = 64

	def main(args: Array[String]): Unit = {
		println(s"Building Resetti")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "Resetti"

		val table = if (tomlFile.isEmpty) {
			println(s"No toml config file provided, defaults will be used")
			Map[String, Variant]()
		}
		else Utils.readToml(Paths.get(tomlFile.get))


		val luInt  = new Function2[String, Int, Int] {
			override def apply(k: String, default: Int): Int =
				Utils.lookupInt(table, k, default)
		}
		val luBInt = new Function2[String, BigInt, BigInt] {
			override def apply(k: String, default: BigInt): BigInt =
				Utils.lookupBigInt(table, k, default)
		}

		resetClocks = luInt("reset_clocks", resetClocks)
		println(s"holds reset for $resetClocks")

		val config = SpinalConfig(
			targetDirectory = targetDir,
			netlistFileName = name + ".v"
			)

		config
			.withPrivateNamespace
			.generateVerilog(
				ResettiComponent().setDefinitionName(name)
				)
			.printPrunedIo()
	}

	case class ResettiComponent()
		extends Component {
		val io = new Bundle {
			val clk        = in Bool()
			val asyncReset = in Bool()
			val syncReset  = out Bool()
		}
		noIoPrefix()

		val resetCtrlClockDomain = ClockDomain(
			clock = io.clk,
			config = ClockDomainConfig(resetKind = BOOT),
			frequency = FixedFrequency(33 MHz) // TODO
			)

		val resetCtrl = new ClockingArea(resetCtrlClockDomain) {
			val systemResetUnbuffered  = False

			//Implement an counter to keep the reset axiResetOrder high 64 cycles
			// Also this counter will automaticly do a reset when the system boot.
			val systemResetCounter = Reg(UInt(log2Up(resetClocks-1) bits)) init(0)
			when(systemResetCounter =/= U(systemResetCounter.bitsRange -> true)) {
				systemResetCounter := systemResetCounter + 1
				systemResetUnbuffered := True
			}
			when(BufferCC(io.asyncReset)) {
				systemResetCounter := 0
			}

			//Create all reset used later in the design
			val systemReset  = RegNext(systemResetUnbuffered)
		}
		io.syncReset := resetCtrl.systemReset
	}

}