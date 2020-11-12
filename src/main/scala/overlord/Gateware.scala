package overlord

import java.nio.file.{Files, Path}

import toml.Value

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
                       override val provision: String,
                       override val sources: Seq[GatewareSourceFile],
                       override val ports: Seq[String],
                       override val parameters: Seq[String])
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
		if (!parsed.contains("provision") ||
		    !parsed("provision").isInstanceOf[Value.Str]) {
			println(s"$name doesn't have a provision field")
			return None
		}
		if (!parsed.contains("sources") ||
		    !parsed("sources").isInstanceOf[Value.Arr]) {
			println(s"$name doesn't have a sources field")
			return None
		}

		if (!parsed.contains("ports") ||
		    !parsed("ports").isInstanceOf[Value.Arr]) {
			println(s"$name doesn't have a ports field")
			return None
		}

		val provision = parsed("provision").asInstanceOf[Value.Str].value

		val src   = {
			val srcs = parsed("sources").asInstanceOf[Value.Arr].values
				.map(_.asInstanceOf[Value.Tbl].values)

			for (entry <- srcs) yield {

				val filename = entry("src").asInstanceOf[Value.Str].value

				val data = if (provision == "copy") {
					val path   = GameBuilder.pathStack.top.resolve(filename)
					val file   = path.toAbsolutePath.toFile
					val source = scala.io.Source.fromFile(file)
					source.getLines().mkString("\n")
				} else ""

				GatewareSourceFile(
					filename,
					entry("language").asInstanceOf[Value.Str].value,
					data)
			}
		}
		val ports = parsed("ports").asInstanceOf[Value.Arr].values
			.map(_.asInstanceOf[Value.Str].value)

		val parameters =
			if (parsed.contains("parameters") &&
			    parsed("parameters").isInstanceOf[Value.Arr])
				parsed("parameters").asInstanceOf[Value.Arr].values
					.map(_.asInstanceOf[Value.Str].value)
			else Seq[String]()

		val attribs: Map[String, Value] = parsed.filter(
			_._1 match {
				case "provision" | "sources" | "ports" | "parameters" => false
				case _                                                => true
			})

		Some(GatewareDef(defType,
		                 GameBuilder.containerStack.top,
		                 attribs, provision, src, ports, parameters))
	}

}