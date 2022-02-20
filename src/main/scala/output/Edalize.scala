package output

import actions.{CopyAction, SourcesAction}
import ikuy_utils._
import overlord.Instances._
import overlord._

import java.nio.file.Path

object Edalize {
	def apply(game: Game, out: Path): Unit = {
		println(s"Creating Edalize script at $out")
		Utils.ensureDirectories(out)

		val board = if (game.board.nonEmpty) game.board.get else {
			print(s"No board instance found, aborting Edalize process")
			return
		}

		//@formatter:off
		val sb = new StringBuilder()
		sb ++=
		s"""from edalize import *
			 |import os
			 |from bitstring import BitArray, BitStream
			 |
			 |
			 |""".stripMargin

		sb ++=
		s"""def prep_rom(name, in_path, out_path, bits_per_byte, data_width, size_in_bytes):
			 |    "turns roms into form for bitstream"
			 |    assert ((data_width % bits_per_byte) == 0)
			 |    bytes_per_data_element = int(data_width // bits_per_byte)
			 |
			 |    out_data = []
			 |    for i in range(0, bytes_per_data_element):
			 |        out_data.append(BitStream())
			 |
			 |    with open(in_path, 'rb') as ifile:
			 |        data = BitStream(ifile.read())
			 |        data.append((size_in_bytes * bits_per_byte) - data.length)
			 |
			 |        while data.pos < data.length:
			 |            for i in range(0, bytes_per_data_element):
			 |                out_data[i].append(data.read(bits_per_byte))
			 |
			 |        for i in range(0, bytes_per_data_element):
			 |            with open(out_path + str(i) + ".bin", "wt") as f:
			 |                while out_data[i].pos < out_data[i].length:
			 |                    d = out_data[i].read(8).bin
			 |                    f.write(str(d) + '\\n')
	     |"""
			.stripMargin
		//@formatter:on

		for {ram <- game.rams} {
			ram.fillType match {
				case PrimaryBootFillType() =>

					val luBInt = new Function2[String, BigInt, BigInt] {
						override def apply(k: String, default: BigInt): BigInt =
							Utils.lookupBigInt(ram.attributes, k, default)
					}
					val luInt  = new Function2[String, Int, Int] {
						override def apply(k: String, default: Int): Int =
							Utils.lookupInt(ram.attributes, k, default)
					}

					val sizeInBytes  = ram.getSizeInBytes
					val bitsPerByte  = luInt("bits_per_byte", 0)
					val busDataWidth = luInt("data_width", 0)
					if (sizeInBytes <= 0 ||
					    bitsPerByte <= 0 ||
					    busDataWidth <= 0) {
						println("invalid RAM setup")
						return
					}


					val inPath =
						s"../soft/build/bootroms/riscv32noneelf_primary_boot" +
						s"/riscv32noneelf_boot.bin"

					val outPath = s"${ram.ident}.v_toplevel_ram_symbol"


					sb ++= s"\n\n" +
					       s"prep_rom('${ram.ident}', \n" +
					       s"         r'$inPath', \n" +
					       s"         r'$outPath', \n" +
					       s"         $bitsPerByte, \n" +
					       s"         $busDataWidth, \n" +
					       s"         $sizeInBytes)\n"

				case _ | ZeroFillType() =>
			}

		}
		sb ++= "work_root = 'build'\n"
		sb ++= "name = 'xmasdemo'\n"

		board.boardType match {
			case XilinxBoard(_, _) => sb ++= "tool = 'vivado'\n"
			case AlteraBoard()     =>
			case LatticeBoard()    =>
		}

		sb ++= "files = [\n"
		for {gw <- game.gatewares
		     defi = gw.definition.asInstanceOf[GatewareDefinitionTrait]} {
			defi.actionsFile.actions.foreach {
				case action: CopyAction =>
					sb ++=
					// @formatter:off
s"""    {'name': os.path.relpath('${action.getDestPath}', work_root), 'file_type': '${action.language}Source'},\n"""
						// @formatter:on
				case action: SourcesAction =>
					sb ++=
					// @formatter:off
s"""    {'name': os.path.relpath('${action.getSrcPath}', work_root), 'file_type': '${action.language}Source'},\n"""
						// @formatter:on
				case _ =>
			}
		}

		sb ++=
		// @formatter:off
s"""    {'name': os.path.relpath('${game.name}_top.v', work_root), 'file_type': 'verilogSource'},\n"""
		sb ++=
s"""    {'name': os.path.relpath('${game.name}.xdc', work_root), 'file_type': 'xdc'}\n"""
		// @formatter:on

		sb ++= "]\n"

		board.boardType match {
			case XilinxBoard(_, device) =>
				sb ++= s"tool_options = {'part': '$device'}\n"
			case AlteraBoard()          =>
			case LatticeBoard()         =>
		}

		//  'parameters': parameters,
		sb ++=
		s"""edam = {
			 |  'files': files,
			 |  'name': name,
			 |  'tool_options': {'vivado': tool_options},
			 |  'toplevel': '${game.name}_top'
			 |}
			 |
			 |backend = get_edatool(tool)(edam=edam, work_root=work_root)
			 |""".stripMargin


		sb ++=
		s"""|
			  |if not os.path.isdir(work_root):
		    |  os.makedirs(work_root)
		    |
		    |backend.configure()
		    |backend.build()
		    |
		    |""".stripMargin

		Utils.writeFile(out.resolve("edalize_build.py"), sb.result())
	}

}
