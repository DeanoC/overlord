package overlord.Gateware

import overlord.Definitions.GatewareTrait
import overlord.Gateware.GatewareAction._

import java.nio.file.{Files, Path}
import overlord.{Game, Utils}
import toml.Value

import scala.collection.mutable

case class Gateware(actions: Seq[GatewareAction],
                    moduleName: String,
                    ports: mutable.HashMap[String, Port],
                    parameters: mutable.HashMap[String, Parameter],
                    verilog_parameters: mutable.HashSet[String]
                   ) extends GatewareTrait

object Gateware {
	def apply(name: String, spath: Path): Option[Gateware] = {
		val path = Game.pathStack.top.resolve(spath)
		if (!Files.exists(path.toAbsolutePath)) {
			println(s"${path.toAbsolutePath} gateware file not found");
			return None
		}
		Game.pathStack.push(path.getParent)
		val file   = path.toAbsolutePath.toFile
		val source = scala.io.Source.fromFile(file)

		val result = parse(name, source.getLines().mkString("\n"))
		Game.pathStack.pop()
		result
	}

	private def parse(name: String, data: String): Option[Gateware] = {

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

				val pathOp: GatewareActionPathOp = if (process.contains("path_op"))
					Utils.toString(process("path_op")) match {
						case "push" => GatewareActionPathOp_Push()
						case "pop"  => GatewareActionPathOp_Pop()
						case _      => GatewareActionPathOp_Noop()
					}
				else GatewareActionPathOp_Noop()

				Utils.toString(process("processor")) match {
					case "copy"             => CopyAction(name, process, pathOp)
					case "source"           => SourcesAction(name, process, pathOp)
					case "git"              => GitCloneAction(name, process, pathOp)
					case "python"           => PythonAction(name, process, pathOp)
					case "yaml"             => YamlAction(name, process, pathOp)
					case "toml"             => TomlAction(name, process, pathOp)
					case "sbt"              => SbtAction(name, process, pathOp)
					case "read_verilog_top" => ReadVerilogTopAction(name, process,
					                                                pathOp)
					case _                  => None
				}
			}).flatten

		val moduleName =
			(if (parsed.contains("module_name"))
				Utils.toString(parsed("module_name"))
			else name).split('.').last

		val ports = if (parsed.contains("ports"))
			Ports(Utils.toArray(parsed("ports"))).map(t => (t.name -> t)).toMap
		else Map()

		val parameters = if (parsed.contains("parameters"))
			Parameters(Utils.toArray(parsed("parameters")))
				.map(t => (t.key -> t)).toMap
		else Map()

		val portHash = mutable.HashMap[String, Port]()
		ports.foreach(p => portHash += (p._1 -> p._2))

		val parameterHash = mutable.HashMap[String, Parameter]()
		parameters.foreach(p => parameterHash += (p._1 -> p._2))

		Some(Gateware(actions,
		              moduleName,
		              portHash,
		              parameterHash,
		              mutable.HashSet()))
	}
}
