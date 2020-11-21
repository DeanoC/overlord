package output

import java.nio.file.Path
import overlord.Gateware.GatewareAction.{CopyAction, GitCloneAction, SourcesAction}
import overlord._

object Edalize {
	def apply(game: Game, out: Path): Unit = {
		println(s"Creating Edalize script at ${out.toRealPath()}")
		Utils.ensureDirectories(out.toRealPath())

		val sb = new StringBuilder()
		sb ++=
		s"""from edalize import *
			 |import os
			 |work_root = 'build'
			 |name = '${game.name}'
			 |""".stripMargin

		game.board.boardType match {
			case XilinxBoard(_, _) => sb ++= "tool = 'vivado'\n"
			case AlteraBoard()     =>
			case LatticeBoard()    =>
		}

		sb ++= "files = [\n"
		for ((gateware, defi) <- game.gatewares) {
			defi.actions.foreach(
				_ match {
					case action: CopyAction =>
						sb ++=
						// @formatter:off
s"""    {'name': os.path.relpath('${action.getDestPath}', work_root), 'file_type': '${action.language}Source'},\n"""
						// @formatter:on
						case action:SourcesAction =>
							sb ++=
						// @formatter:off
s"""    {'name': os.path.relpath('${action.srcPath}', work_root), 'file_type': '${action.language}Source'},\n"""
						// @formatter:on
					case _                              =>
				})
		}

		sb ++=
		// @formatter:off
s"""    {'name': os.path.relpath('${game.name}_top.v', work_root), 'file_type': 'verilogSource'},\n"""
		sb ++=
s"""    {'name': os.path.relpath('${game.name}.xdc', work_root), 'file_type': 'xdc'}\n"""
		// @formatter:on

		sb ++= "]\n"

		sb ++= "parameters = {\n"
		for ((gateware, defi) <- game.gatewares)
			defi.parameters.foreach(
				// @formatter:off
				p => sb ++=
s"""    '${p._1}': {'datatype': 'string', 'default': "${p._2}", 'paramtype': 'vlogparam'},\n"""
				// @formatter:on
				)
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

		Utils.writeFile(out.resolve("edalize_build.py"), sb.result())
	}

}
