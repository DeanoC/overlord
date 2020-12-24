import spinal.core._
import spinal.lib._
import spinal.lib.bus.simple.PipelinedMemoryBus
import spinal.lib.bus.simple.PipelinedMemoryBusConfig
import ikuy_utils._
import toml.Value
import java.nio.file.Path

object PmbBram {
	var sizeInBytes    : BigInt = 4096
	var bitsPerByte    : Int    = 8
	var busDataWidth   : Int    = 32
	var busAddressWidth: Int    = 32

	def main(args: Array[String]): Unit = {
		println(s"Building Spinal Pipelined Memory Bus RAM")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "pmb_bram"

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

		sizeInBytes = luBInt("size_in_bytes", sizeInBytes)
		bitsPerByte = luInt("bits_per_byte", bitsPerByte)
		busDataWidth = luInt("data_width", busDataWidth)
		busAddressWidth = luInt("address_width", busAddressWidth)

		println(s"mem has $busDataWidth data bus with $busAddressWidth address " +
		        s"bus")
		println(s"$bitsPerByte bits per byte and $sizeInBytes bytes capacity")

		val config = SpinalConfig(
			defaultConfigForClockDomains =
				ClockDomainConfig(resetKind = spinal.core.SYNC),
			targetDirectory = targetDir,
			netlistFileName = name + ".v"
			)

		config
			.withPrivateNamespace
			.generateVerilog(
				PmbBramComponent().setDefinitionName(name)
				)
			.printPrunedIo()
	}

	case class PmbBramComponent()
		extends Component {

		val busConfig = PipelinedMemoryBusConfig(busAddressWidth, busDataWidth)

		val io = new Bundle {
			val bus = slave(PipelinedMemoryBus(busConfig))
		}
		noIoPrefix()

		val ram = Mem(Bits(busDataWidth bits),
		              sizeInBytes / (busDataWidth / bitsPerByte))

		io.bus.rsp.valid :=
		RegNext(io.bus.cmd.fire && !io.bus.cmd.write) init (False)

		io.bus.rsp.data := ram.readWriteSync(
			address = (io.bus.cmd.address >> 2).resized,
			data = io.bus.cmd.data,
			enable = io.bus.cmd.valid,
			write = io.bus.cmd.write,
			mask = io.bus.cmd.mask
			)

		io.bus.cmd.ready := True
	}

}