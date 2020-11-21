package overlord.Gateware

import overlord.Definitions.{DefinitionType, GatewareTrait}
import overlord.Gateware.GatewareAction._

import java.nio.file.{Files, Path}
import overlord.{GameBuilder, Utils}
import toml.Value

case class Gateware(actions: Seq[GatewareAction],
                    moduleName: String,
                    ports: Seq[String],
                    parameters: Map[String, String]
                   ) extends GatewareTrait

object Gateware {
	def apply(name: String, spath: Path): Option[Gateware] = {
		val path = GameBuilder.pathStack.top.resolve(spath)
		if (!Files.exists(path.toAbsolutePath)) {
			println(s"${path.toAbsolutePath} gateware file not found");
			return None
		}
		GameBuilder.pathStack.push(path.getParent)
		val file   = path.toAbsolutePath.toFile
		val source = scala.io.Source.fromFile(file)

		val result = parse(name, source.getLines().mkString("\n"))
		GameBuilder.pathStack.pop()
		result
	}

	private def parse(name: String, data: String)
	: Option[Gateware] = {

		val parsed = {
			val parsed = toml.Toml.parse(data)
			parsed match {
				case Right(value) => value.values
				case Left(value)  => println(s"$name has failed to parse with " +
				                             s"error ${Left(parsed)}")
					return None
			}
		}

		if (!parsed.contains("actions") ||
		    !parsed("actions").isInstanceOf[Value.Arr]) {
			println(s"$name doesn't have an actions field")
			return None
		}

		if (!parsed.contains("process") ||
		    !parsed("process").isInstanceOf[Value.Arr]) {
			println(s"$name doesn't have process fields")
			return None
		}

		if (!parsed.contains("ports") ||
		    !parsed("ports").isInstanceOf[Value.Arr]) {
			println(s"$name doesn't have a ports field")
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
					println(s"$action process not found in ${name}")
					return None
				}
				val process = processes(action)

				val pathOp: GatewarePathOp = if (process.contains("path_op"))
					Utils.toString(process("path_op")) match {
						case "push" => GatewarePathOp_Push()
						case "pop"  => GatewarePathOp_Pop()
						case _      => GatewarePathOp_Noop()
					}
				else GatewarePathOp_Noop()

				Utils.toString(process("processor")) match {
					case "copy"   => CopyAction(name, process, pathOp)
					case "source" => SourcesAction(name, process, pathOp)
					case "git"    => GitCloneAction(name, process, pathOp)
					case "python" => PythonAction(name, process, pathOp)
					case "yaml"   => YamlAction(name, process, pathOp)
					case "sbt"    => SbtAction(name, process, pathOp)
					case _        => None
				}
			}).flatten

		val moduleName =
			(if (parsed.contains("module_name"))
				Utils.toString(parsed("module_name"))
			else name).split('.').last

		val ports = Utils.toArray(parsed("ports")).map(Utils.toString)

		val parameters = if (parsed.contains("parameters"))
			Utils.toArray(parsed("parameters")).map(Utils.toTable)
				.map(t => (Utils.toString(t("key")) -> Utils.toString(t("value"))))
				.toMap
		else Map[String, String]()

		val attribs: Map[String, Value] = parsed.filter(
			_._1 match {
				case "actions" | "ports" | "parameters" | "process" => false
				case _                                              => true
			})


		Some(Gateware(actions,
		              moduleName,
		              ports,
		              parameters))
	}

}
