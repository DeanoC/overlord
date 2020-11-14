package overlord

import java.nio.file.{Files, Path}

import toml.Value

trait GatewareTrait {
	val actions   : Seq[GatewareAction]
	val moduleName: String
	val ports     : Seq[String]
	val parameters: Map[String, String]
}

case class GatewareInstance(ident: String,
                            definition: GatewareDef,
                            attributes: Map[String, toml.Value],
                           ) extends Instance[GatewareInstance] {

	override def copyMutate(nid: String,
	                        nattribs: Map[String, toml.Value])
	: Instance[GatewareInstance] = copy(ident = nid, attributes = nattribs)

	override def matchIdent(a: String): Boolean = {
		if (a == ident) true else {
			val s = a.split('.')

			(s.length >= 2 &&
			 (s(0) == ident &&
			  (definition.ports.contains(s(1)) ||
			   definition.parameters.contains(s(1)))))
		}
	}

}

case class GatewareDef(override val chipType: String,
                       override val container: Option[String],
                       override val attributes: Map[String, Value],
                       override val actions: Seq[GatewareAction],
                       override val moduleName: String,
                       override val ports: Seq[String],
                       override val parameters: Map[String, String])
	extends Definition with GatewareTrait {
	override val softwares = Seq[Software]()
}

object Gateware {
	def defFromFile(defType: String, spath: Path): Option[GatewareDef] = {
		val typePath = defType.split('.')
		if (typePath.length < 2 ||
		    typePath(0) != "gateware") {
			println(s"$defType is an invalid gateware type")
			return None
		}

		val path = GameBuilder.pathStack.top.resolve(spath)
		if (!Files.exists(path.toAbsolutePath)) {
			println(s"${path.toAbsolutePath} gateware file not found");
			return None
		}
		GameBuilder.pathStack.push(path.getParent)
		val file   = path.toAbsolutePath.toFile
		val source = scala.io.Source.fromFile(file)

		val result = parse(typePath(1), defType, source.getLines().mkString("\n"))
		GameBuilder.pathStack.pop
		result
	}

	private def parse(name: String,
	                  defType: String,
	                  data: String)
	: Option[GatewareDef] = {
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
			(for (tprocess <- parsed("process").asInstanceOf[Value.Arr].values)
				yield {
					tprocess match {
						case tbl: Value.Tbl =>
							if (tbl.values.contains("name")) {
								val name = tbl.values("name").asInstanceOf[Value.Str].value
								Some(name -> tbl.values)
							} else None
						case _              => None
					}
				}).flatten.toMap

		val actions =
			(for {taction <- parsed("actions").asInstanceOf[Value.Arr].values
			      action = taction.asInstanceOf[Value.Str].value} yield {

				if (!processes.contains(action)) {
					println(s"$action process not found in ${name}")
					return None
				}
				val process = processes(action)

				val pathOp: GatewarePathOp = {
					if (process.contains("path_op")) {
						val pathOp = process("path_op").asInstanceOf[Value.Str].value
						pathOp match {
							case "push" => GatewarePathOp_Push()
							case "pop"  => GatewarePathOp_Pop()
							case _      => GatewarePathOp_Noop()
						}
					} else GatewarePathOp_Noop()
				}

				process("processor").asInstanceOf[Value.Str].value match {
					case "copy"   => GatewareAction.createCopy(name, process, pathOp)
					case "git"    => GatewareAction.createGit(name, process, pathOp)
					case "python" => GatewareAction.createPython(name, process, pathOp)
					case "yaml"   => GatewareAction.createYaml(name, process, pathOp)
					case _        => None
				}
			}).flatten

		val moduleName =
			if (parsed.contains("module_name"))
				parsed("module_name").asInstanceOf[Value.Str].value
			else name

		val ports = parsed("ports").asInstanceOf[Value.Arr].values
			.map(_.asInstanceOf[Value.Str].value)

		val parameters = if (parsed.contains("parameters")) {
			parsed("parameters").asInstanceOf[Value.Arr].values
				.map(_.asInstanceOf[Value.Tbl].values)
				.map(t => (Utils.toString(t("key")) ->
				           Utils.toString(t("value"))))
				.toMap
		} else Map[String, String]()

		val attribs: Map[String, Value] = parsed.filter(
			_._1 match {
				case "actions" | "ports" | "parameters" | "process" => false
				case _                                              => true
			})


		Some(GatewareDef(defType,
		                 GameBuilder.containerStack.top,
		                 attribs,
		                 actions,
		                 moduleName,
		                 ports,
		                 parameters))
	}

}