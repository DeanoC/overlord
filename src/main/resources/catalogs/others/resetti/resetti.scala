import spinal.core._
import spinal.lib._
import ikuy_utils.Utils
import toml.Value

object Resetti {
	var resetClocks: Int    = 64

	def main(args: Array[String]): Unit = {
		println(s"Building Resetti")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "Resetti"

		val table = if (tomlFile.isEmpty) {
			println(s"No toml config file provided, defaults will be used")
			Map[String, Value]()
		}
		else {
			println(s"Reading $tomlFile for resetti config")
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
			val mainClk = in Bool
			val asyncReset = in Bool
			val syncReset = out Bool
		}
		noIoPrefix()

		val resetCtrlClockDomain = ClockDomain(
			clock = io.mainClk,
			config = ClockDomainConfig(
				resetKind = BOOT
				)
			)

		val resetCtrl = new ClockingArea(resetCtrlClockDomain) {
			val syncResetUnbuffered  = False

			//Implement an counter to keep the reset reset high for resetClocks cycles
			// Also this counter will automatically do a reset when the system boot.
			val systemClkResetCounter = Reg(UInt(log2Up(resetClocks-1) bits)) init(0)
			when(systemClkResetCounter =/= U(systemClkResetCounter.range -> true)){
				systemClkResetCounter := systemClkResetCounter + 1
				syncResetUnbuffered := True
			}
			when(BufferCC(io.asyncReset)){
				systemClkResetCounter := 0
			}

			io.syncReset := RegNext(syncResetUnbuffered)
		}
	}

}