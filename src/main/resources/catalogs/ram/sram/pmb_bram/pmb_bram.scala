import ikuy_utils._

import java.nio.file.Path

object PmbBram {
	var sizeInBytes    : BigInt = 0
	var bitsPerByte    : Int    = 0
	var busDataWidth   : Int    = 0
	var busAddressWidth: Int    = 0

	def main(args: Array[String]): Unit = {
		println(s"Building Spinal Pipelined Memory Bus RAM")

		val tomlFile  = if (args.length >= 1) Some(args(0)) else None
		val targetDir = if (args.length >= 2) args(1) else "."
		val name      = if (args.length >= 3) args(2) else "pmb_bram"

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

		sizeInBytes = luBInt("size_in_bytes", sizeInBytes)
		bitsPerByte = luInt("bits_per_byte", bitsPerByte)
		busDataWidth = luInt("data_width", busDataWidth)
		busAddressWidth = luInt("address_width", busAddressWidth)

		println(s"mem has $busDataWidth data bus with $busAddressWidth address " +
		        s"bus")
		println(s"$bitsPerByte bits per byte and $sizeInBytes bytes capacity")

		if (sizeInBytes <= 0 ||
		    bitsPerByte <= 0 ||
		    busDataWidth <= 0 ||
		    busAddressWidth <= 0) {
			println("invalid RAM setup")
			return
		}

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

		val wordCount = sizeInBytes / (busDataWidth / bitsPerByte)

		val ram = Mem(wordType = Bits(busDataWidth bits), wordCount)
		ram.init(
			for (i <- 0 until wordCount.toInt) yield B(0, busDataWidth bits)
			)

		io.bus.rsp.valid := RegNext(io.bus.cmd.fire &&
		                            !io.bus.cmd.write) init (False)

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