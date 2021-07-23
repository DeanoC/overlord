package overlord.Gateware.GatewareAction

import overlord.Instances.Instance
import overlord.Game
import ikuy_utils._

import java.nio.file.Path

case class ReadTomlRegistersAction(filename: String,
                                   pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase2()

	override def execute(instance: Instance,
	                     parameters: Map[String, Variant],
	                     outPath: Path): Unit = {
		val name = filename.replace("${name}", instance.ident)
		input.TomlRegistersParser(outPath, name.split("/").last) match {
			case Some(v) => instance.instanceRegisterLists ++= v.registerLists
			case None    =>
		}
	}
}

object ReadTomlRegistersAction {
	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: GatewareActionPathOp): Option[ReadTomlRegistersAction] = {
		if (!process.contains("source")) {
			println(s"Read Toml Registers process $name doesn't have a source field")
			return None
		}

		val filename = process("source") match {
			case s: StringV => s.value
			case t: TableV  => Utils.toString(t.value("file"))
			case _ =>
				println("Read Toml Register source field is malformed")
				return None
		}

		val srcPath = if (filename.contains("${src}")) {
			val tmp = filename.replace("${src}", "")
			Game.pathStack.top.toString + tmp
		} else filename
		Some(ReadTomlRegistersAction(srcPath, pathOp))
	}
}
