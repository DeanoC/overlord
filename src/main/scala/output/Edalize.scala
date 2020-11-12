package output

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path

import overlord.{AlteraBoard, Game, LatticeBoard, XilinxBoard}

object Edalize {
	def apply(game: Game, out: Path): Unit = {
		println(s"Creating Edalize script at ${out.toRealPath()}")

		val sb = new StringBuilder()

		sb ++=
		s"""from edalize import *
			 |import os
			 |work_root = 'build'
			 |name = '${game.name}'
			 |""".stripMargin

		game.board.boardType match {
			case XilinxBoard(_, _) =>
				sb ++= "tool = 'vivado'\n"
			case AlteraBoard()     =>
			case LatticeBoard()    =>
		}

		sb ++= "files = [\n"
		for (gateware <- game.gatewares) {
			gateware.definition.sources.foreach(
				f => {
					val filename = s"${gateware.ident}/${f.filename}"
					sb ++=
					s"""  {
						 |    'name': os.path.relpath('$filename', work_root),
						 |    'file_type': '${f.language}Source'
						 |  },
						 |""".stripMargin

					ensureDirectories(out.resolve(filename).getParent)
					if (f.data.nonEmpty) writeFile(out.resolve(filename), f.data)
				})
		}
		sb ++=
		s""" {
			 |     'name': os.path.relpath('${game.name}_top.v', work_root),
			 |     'file_type': 'verilogSource'
			 | },
			 |""".stripMargin
		sb ++=
		s""" {
			|     'name': os.path.relpath('${game.name}.xdc', work_root),
			|     'file_type': 'xdc'
			| }
			|""".stripMargin
		sb ++= "]\n"

		sb ++= "parameters = {\n"
		for (gateware <- game.gatewares)
			gateware.definition.parameters.foreach(
				p => sb ++=
				     s""" '${p}':
					      |   {'datatype': 'int', 'default': 1000,
								|   'paramtype': 'vlogparam'},
					      |""".stripMargin)
		sb ++= "}\n"
		game.board.boardType match {
			case XilinxBoard(_, device) =>
				sb ++= s"tool_options = {'part': '$device'}\n"
			case AlteraBoard()          =>
			case LatticeBoard()         =>
		}

		sb ++=
		s"""edam = {
			 |  'files': files,
			 |  'name': name,
			 |  'parameters': parameters,
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

		writeFile(out.resolve("edalize_build.py"), sb.result)
	}

	private def ensureDirectories(path: Path): Unit = {
		val directory = path.toFile
		if (!directory.exists()) {
			directory.mkdirs()
		}
	}

	private def writeFile(path: Path, s: String): Unit = {
		val file = path.toFile
		val bw   = new BufferedWriter(new FileWriter(file))
		bw.write(s)
		bw.close()
	}

}
