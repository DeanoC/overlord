package output

import java.nio.file.Path
import overlord.Gateware.GatewareAction.{CopyAction, GitCloneAction, SourcesAction}
import overlord.Instances.{AlteraBoard, LatticeBoard, XilinxBoard}
import overlord._
import ikuy_utils._

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

		val board = if(game.board.nonEmpty) game.board.get else {
			print(s"No board instance found, aborting Edalize process")
			return
		}

		board.boardType match {
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
s"""    {'name': os.path.relpath('${action.getSrcPath}', work_root), 'file_type': '${action.language}Source'},\n"""
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

		/* Currently parameters are handled by Overlord not Edalize
		Its quite likely in future this will change so here is my original code

		sb ++= "parameters = {\n"
		for ((gateware, defi) <- game.gatewares)
			defi.parameters.foreach(
				// @formatter:off
				p => sb ++=
s"""    '${p._1}': {'datatype': 'string', 'default': "${p._2}", 'paramtype': 'vlogparam'},\n"""
				// @formatter:on
				)
		sb ++= "}\n"
*/

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
