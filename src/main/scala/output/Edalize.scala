package output

import java.nio.file.Path

import overlord._

object Edalize {
	def apply(game: Game, out: Path): Unit = {
		println(s"Creating Edalize script at ${out.toRealPath()}")
		Utils.ensureDirectories(out.toRealPath())

		GameBuilder.pathStack.push(out.toRealPath())

		val sb = new StringBuilder()

		sb ++=
		s"""from edalize import *
			 |import os
			 |work_root = 'build'
			 |name = '${game.name}'
			 |""".stripMargin

		for {gateware <- game.gatewares; action <- gateware.definition.actions} {
			val parameters = gateware.definition.parameters
			val merged     = game.constants
				.map(_.asParameter)
				.fold(parameters)((o, n) => o ++ n)

			action.execute(gateware, merged, GameBuilder.pathStack.top)
		}

		game.board.boardType match {
			case XilinxBoard(_, _) => sb ++= "tool = 'vivado'\n"
			case AlteraBoard()     =>
			case LatticeBoard()    =>
		}

		sb ++= "files = [\n"
		for (gateware <- game.gatewares) {
			gateware.definition.actions.foreach(
				_ match {
					case action: GatewareCopyAction =>
						sb ++=
						// @formatter:off
s"""    {'name': os.path.relpath('${action.getDestPath}', work_root), 'file_type': '${action.language}Source'},\n"""
						// @formatter:on
					case GatewareGitCloneAction(git, _) =>
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
		for (gateware <- game.gatewares)
			gateware.definition.parameters.foreach(
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

		Utils.writeFile(out.resolve("edalize_build.py"), sb.result)
	}

}
