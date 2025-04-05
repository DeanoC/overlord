package actions

import ikuy_utils.{ArrayV, Utils, Variant}
import overlord.Instances.{ChipInstance, InstanceTrait}
import scala.util.boundary, boundary.break

trait Action {
	val phase : Int

	def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit
}

trait GatewareAction extends Action {
	def execute(instance: ChipInstance, parameters: Map[String, Variant]): Unit
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

				boundary {
					if (!processes.contains(action)) {
						println(s"$action process not found in $name")
						break(None)
					}

					val process = processes(action)
					Utils.toString(process("processor")) match {
						case "copy"                => CopyAction(name, process)
						case "sources"             => SourcesAction(name, process)
						case "git"                 => GitCloneAction(name, process)
						case "python"              => PythonAction(name, process)
						case "yaml"                => YamlAction(name, process)
						case "toml"                => TomlAction(name, process)
						case "sbt"                 => SbtAction(name, process)
						case "read_verilog_top"    => ReadVerilogTopAction(name, process)
						case "read_toml_registers" => ReadTomlRegistersAction(name, process)
						case "templates"           => TemplateAction(name, process)
						case "software_sources"    => SoftSourceAction(name, process)
						case _                     =>
							println(f"Unknown action processor (${Utils.toString(process("processor"))}) in $name")
							break(None)
					}
				}
			}).flatten

		Some(ActionsFile(actions.toSeq))
	}
}

