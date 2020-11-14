package overlord

import java.nio.file.Path

import toml.Value

sealed trait GatewarePathOp

case class GatewarePathOp_Noop() extends GatewarePathOp

case class GatewarePathOp_Push() extends GatewarePathOp

case class GatewarePathOp_Pop() extends GatewarePathOp

trait GatewareAction {
	def execute(gateware: GatewareInstance,
	            parameters: Map[String, String],
	            outPath: Path): Unit

	val pathOp: GatewarePathOp

	def updatePath(path: Path): Unit = {
		pathOp match {
			case GatewarePathOp_Noop() =>
			case GatewarePathOp_Push() => GameBuilder.pathStack.push(path)
			case GatewarePathOp_Pop()  => GameBuilder.pathStack.pop()
		}
	}
}

object GatewareAction {
	def createCopy(name: String,
	               process: Map[String, Value],
	               pathOp: GatewarePathOp): Seq[GatewareAction] = {
		val srcs = process("sources").asInstanceOf[Value.Arr].values
			.map(_.asInstanceOf[Value.Tbl].values)

		for (entry <- srcs) yield {
			val filename = entry("file").asInstanceOf[Value.Str].value
			val srcPath  = if (filename.contains("${src}")) {
				val tmp = filename.replace("${src}", "")
				GameBuilder.pathStack.top + tmp
			} else filename

			GatewareCopyAction(
				filename,
				entry("language").asInstanceOf[Value.Str].value,
				srcPath,
				pathOp)
		}
	}

	def createGit(name: String,
	              process: Map[String, Value],
	              pathOp: GatewarePathOp): Seq[GatewareAction] = {
		Seq(GatewareGitCloneAction(process("url").asInstanceOf[Value.Str].value,
		                           pathOp))
	}

	def createYaml(name: String,
	               process: Map[String, Value],
	               pathOp: GatewarePathOp): Seq[GatewareAction] = {
		if (!process.contains("parameters")) {
			println(s"Yaml process $name doesn't have a parameters field")
			None
		}
		if (!process("parameters").isInstanceOf[Value.Arr]) {
			println(s"Yaml process $name parameters isn't an array")
			None
		}
		if (!process.contains("filename")) {
			println(s"Yaml process $name doesn't have a filename field")
			None
		}
		if (!process("filename").isInstanceOf[Value.Str]) {
			println(s"Yaml process $name filename isn't a string")
			None
		}

		val filename   = process("filename").asInstanceOf[Value.Str].value
		val parameters = if (process.contains("parameters") &&
		                     process("parameters").isInstanceOf[Value.Arr])
			process("parameters").asInstanceOf[Value.Arr].values
				.map(_.asInstanceOf[Value.Str].value)
		else Seq[String]()

		Seq(GatewareYamlAction(parameters, filename, pathOp))
	}

	def createPython(name: String,
	                 process: Map[String, Value],
	                 pathOp: GatewarePathOp): Seq[GatewareAction] = {
		if (!process.contains("script")) {
			println(s"Python process $name doesn't have a script field")
			None
		}
		if (!process("script").isInstanceOf[Value.Str]) {
			println(s"Python process $name script isn't a string")
			None
		}
		if (!process.contains("args")) {
			println(s"Python process $name doesn't have a args field")
			None
		}
		if (!process("args").isInstanceOf[Value.Str]) {
			println(s"Python process $name args isn't a string")
			None
		}
		val script = Utils.toString(process("script"))
		val args   = Utils.toString(process("args"))
		Seq(GatewarePythonAction(script, args, pathOp))

	}
}

case class GatewareCopyAction(filename: String,
                              language: String,
                              srcPath: String,
                              pathOp: GatewarePathOp)
	extends GatewareAction {

	private var dstAbsPath: Path = Path.of("")

	override def execute(gateware: GatewareInstance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = {
		val fn         = filename.split('/').last
		val pathString = s"${gateware.ident}/$fn"
		dstAbsPath = outPath.resolve(pathString.replace("${dest}/", ""))

		val srcAbsPath = if (srcPath.contains("${dest}/")) {
			GameBuilder.pathStack.top.resolve(
				srcPath.replace("${dest}/", ""))
		} else Path.of(srcPath)

		Utils.ensureDirectories(dstAbsPath.getParent)
		val source = Utils.readFile(srcAbsPath)
		Utils.writeFile(dstAbsPath, source.toString)

		updatePath(dstAbsPath.getParent)

	}

	def getDestPath: String =
		dstAbsPath.toString.replace('\\', '/')

}

case class GatewareGitCloneAction(url: String,
                                  pathOp: GatewarePathOp)
	extends GatewareAction {
	override def execute(gateware: GatewareInstance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = {
		import scala.language.postfixOps
		import scala.sys.process._

		val path = outPath.resolve(url.split('/').last)
		if (!path.toFile.exists()) {
			val result = s"git clone --recursive $url $path" !

			if (result != 0)
				println(s"FAILED git clone of $url")
		}
		updatePath(path)
	}
}

case class GatewareYamlAction(parameterKeys: Seq[String],
                              filename: String,
                              pathOp: GatewarePathOp)
	extends GatewareAction {
	override def execute(gateware: GatewareInstance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = {
		val sb = new StringBuilder()
		for (k <- parameterKeys) {
			if (parameters.contains(k))
				sb ++= s"$k: ${parameters(k)}\n"
		}
		Utils.writeFile(outPath.resolve(filename), sb.result())

		updatePath(outPath)
	}
}

case class GatewarePythonAction(script: String,
                                args: String,
                                pathOp: GatewarePathOp)
	extends GatewareAction {
	override def execute(instance: GatewareInstance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = {
		import scala.language.postfixOps

		val result = sys.process.Process(
			Seq("python3", s"$script", s"$args"), outPath.toFile).!

		if (result != 0)
			println(s"FAILED python3 of $script $args")

	}
}

case class GatewareShellAction(script: String,
                               args: String,
                               pathOp: GatewarePathOp)
	extends GatewareAction {
	override def execute(gateware: GatewareInstance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = ???
}
