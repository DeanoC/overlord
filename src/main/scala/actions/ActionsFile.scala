package actions

import ikuy_utils.{ArrayV, Utils, Variant}
import overlord.Game
import overlord.Instances.{ChipInstance, InstanceTrait}

import java.nio.file.Path

sealed trait ActionPathOp

case class ActionPathOp_Noop() extends ActionPathOp

case class ActionPathOp_Push() extends ActionPathOp

case class ActionPathOp_Pop() extends ActionPathOp


trait Action {
	val pathOp: ActionPathOp
	val phase : Int

	def updatePath(path: Path): Unit = {
		pathOp match {
			case ActionPathOp_Noop() =>
			case ActionPathOp_Push() => Game.pathStack.push(path)
			case ActionPathOp_Pop()  => Game.pathStack.pop()
		}
	}

	def execute(instance: InstanceTrait,
	            parameters: Map[String, () => Variant],
	            outPath: Path): Unit
}

trait GatewareAction extends Action {
	def execute(instance: ChipInstance,
	            parameters: Map[String, () => Variant],
	            outPath: Path): Unit

}

trait SoftwareAction extends Action

case class ActionsFile(actions: Seq[Action])

object ActionsFile {
	def apply(name: String, parsed: Map[String, Variant]): Option[ActionsFile] = {
		if (!parsed.contains("actions") ||
		    !parsed("actions").isInstanceOf[ArrayV]) {
			println(s"$name doesn't have an actions field")
			return None
		}

		if (!parsed.contains("process") ||
		    !parsed("process").isInstanceOf[ArrayV]) {
			println(s"$name doesn't have process fields")
			return None
		}

		val processes =
			(for {tprocess <- Utils.toArray(parsed("process"))
			      tbl = Utils.toTable(tprocess)} yield {
				if (tbl.contains("name"))
					Some(Utils.toString(tbl("name")) -> tbl)
				else None
			}).flatten.toMap

		val actions =
			(for {taction <- Utils.toArray(parsed("actions"))
			      action = Utils.toString(taction)} yield {

				if (!processes.contains(action)) {
					println(s"$action process not found in $name")
					return None
				}
				val process = processes(action)

				val pathOp: ActionPathOp = if (process.contains("path_op"))
					Utils.toString(process("path_op")) match {
						case "push" => ActionPathOp_Push()
						case "pop"  => ActionPathOp_Pop()
						case _      => ActionPathOp_Noop()
					}
				else ActionPathOp_Noop()

				Utils.toString(process("processor")) match {
					case "copy"                => CopyAction(name, process, pathOp)
					case "sources"             => SourcesAction(name, process, pathOp)
					case "git"                 => GitCloneAction(name, process, pathOp)
					case "python"              => PythonAction(name, process, pathOp)
					case "yaml"                => YamlAction(name, process, pathOp)
					case "toml"                => TomlAction(name, process, pathOp)
					case "sbt"                 => SbtAction(name, process, pathOp)
					case "read_verilog_top"    => ReadVerilogTopAction(name, process, pathOp)
					case "read_toml_registers" => ReadTomlRegistersAction(name, process, pathOp)
					case "templates"           => TemplateAction(name, process, pathOp)
					case "software_sources"    => SoftSourceAction(name, process, pathOp)
					case _                     =>
						println(f"Unknown action processor in $name")
						None
				}
			}).flatten

		Some(ActionsFile(actions.toSeq))
	}
}

