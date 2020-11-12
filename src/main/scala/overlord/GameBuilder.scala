package overlord

import java.nio.file.{Files, Path, Paths}

import toml.Value

import scala.collection.mutable

object GameBuilder {
	def gameFrom(gameName: String,
	             gameText: String,
	             catalogs: DefinitionCatalogs,
	             board: Board): Option[Game] = {
		(new GameBuilder(gameName, gameText, catalogs, board)).toGame
	}

	val pathStack     : mutable.Stack[Path]           = mutable.Stack[Path]()
	val containerStack: mutable.Stack[Option[String]] = mutable
		.Stack[Option[String]]()

	def parseConnection(connection: Value,
	                    catalogs: DefinitionCatalogs): Option[Connection] = {
		val table = connection.asInstanceOf[Value.Tbl].values
		if (!table.contains("type")) {
			println(s"connection ${connection} requires a type field")
			return None
		}
		val conntype = table("type").asInstanceOf[Value.Str].value

		if (!table.contains("connection")) {
			println(s"connection ${conntype} requires a connection field")
			return None
		}

		val cons = table("connection").asInstanceOf[Value.Str].value
		val con  = cons.split(' ')
		if (con.length != 3) {
			println(s"$conntype has an invalid connection field: $cons")
			return None
		}
		val (first, secondary) = con(1) match {
			case "->" | "<->" => (con(0), con(2))
			case "<-"         => (con(2), con(0))
			case _            =>
				println(s"$conntype has an invalid connection ${con(1)} : $cons")
				return None
		}

		(catalogs.FindDefinition(conntype) match {
			case Some(b) => Some(UnconnectedBetween(b, first, secondary))
			case None    =>
				conntype match {
					case "port" | "clock" =>
						Some(UnconnectedBetween(
							Def("Connection",
							    None,
							    Map[String, toml.Value](),
							    Seq[Software]()),
							first,
							secondary))
					case "constant"       =>
						Some(UnconnectedConstant(
							Def("Connection",
							    None,
							    Map[String, toml.Value](),
							    Seq[Software]()),
							first,
							secondary
							))
					case _                =>
						println(s"$conntype isn't in any definition catalog")
						return None
				}
		})
	}

	def parseInstance(instance: toml.Value,
	                  catalogs: DefinitionCatalogs
	                 ): Option[Instance[_]] = {
		val table = instance.asInstanceOf[Value.Tbl].values
		if (!table.contains("type")) {
			println(s"$instance doesn't have a type")
			return None
		}
		val defType = table("type").asInstanceOf[Value.Str].value
		val name    = tableStringGetOrElse(table, "name", defType)

		val attribs: Map[String, Value] = table.filter(
			_._1 match {
				case "type" | "name" => false
				case _               => true
			})

		catalogs.FindDefinition(defType) match {
			case Some(d) => {
				defType.split('.').head match {
					case "ram"             => Some(RamInstance(name, d, attribs))
					case "cpu"             => Some(CpuInstance(name, d, attribs))
					case "NxMinterconnect" => Some(NxMInstance(name, d, attribs))
					case "storage"         => Some(StorageInstance(name, d, attribs))
					case "soc"             => Some(SocInstance(name, d, attribs))
					case "bridge"          => Some(BridgeInstance(name, d, attribs))
					case "net"             => Some(NetInstance(name, d, attribs))
					case "gateware"        =>
						Some(GatewareInstance(name,
						                      d.asInstanceOf[GatewareDef],
						                      attribs))
					case _                 =>
						println(s"$defType has unknown class")
						None
				}
			}
			case None    =>
				// some types be defined directly in the instance
				defType.split('.').head match {
					case "gateware" =>
						Gateware.defFromFile(defType,
						                     Path.of(s"$name/$name" + s".toml")) match {
							case Some(d) => Some(GatewareInstance(name, d, attribs))
							case None    => None
						}
					case _          =>
						println(s"${defType} not found in any definition catalogs")
						None
				}
		}
	}

	def fromFile(name: String, path: Path): Option[String] = {
		println(s"Reading $name")

		if (!Files.exists(path.toAbsolutePath)) {
			println(s"$name catalog at $path not found");
			return None
		}

		val file   = path.toAbsolutePath.toFile
		val source = scala.io.Source.fromFile(file)

		Some(source.getLines().mkString("\n"))
	}

	private def tableStringGetOrElse(table: Map[String, toml.Value],
	                                 key: String,
	                                 default: String) =
		if (table.contains(key)) table(key)
			.asInstanceOf[toml.Value.Str]
			.value else default


}

private class GameBuilder(gameName: String,
                          gameText: String,
                          catalogs: DefinitionCatalogs,
                          board: Board) {

	private val unconnected = mutable.ArrayBuffer[Connection]()
	private val unexpanded  = mutable.ArrayBuffer[Instance[_]]()

	unexpanded ++= board.instances
	includeOver(gameText, catalogs)

	// TODO scala generic code for these
	private def tableIntGetOrElse(table: Map[String, toml.Value],
	                              key: String,
	                              default: Int) =
		if (table.contains(key)) table(key)
			.asInstanceOf[toml.Value.Num]
			.value
			.toInt else default

	private def tableStringGetOrElse(table: Map[String, toml.Value],
	                                 key: String,
	                                 default: String) =
		if (table.contains(key)) table(key)
			.asInstanceOf[toml.Value.Str]
			.value else default

	def includeOver(data: String, catalogs: DefinitionCatalogs): Unit = {
		val parsed = {
			val parsed = toml.Toml.parse(data)
			parsed match {
				case Right(value) => value.values
				case Left(value)  => println(s"game.over has failed to parse with " +
				                             s"error ${Left(parsed)}")
					return
			}
		}

		// includes
		if (parsed.contains("includes")) {
			val tincs = parsed("includes").asInstanceOf[Value.Arr].values
			for (include <- tincs) {
				val table       = include.asInstanceOf[Value.Tbl].values
				val incResource = table("resource").asInstanceOf[Value.Str].value

				val path = Paths.get(s"src/main/resources/overs")
				GameBuilder.pathStack.push(path)

				val file = path.resolve(s"$incResource.over")
				GameBuilder.fromFile(incResource, file) match {
					case Some(d) =>
						includeOver(d, catalogs)
						GameBuilder.pathStack.pop()
					case _       => println(
						"Include resource file ${incResource} not found")
						GameBuilder.pathStack.pop()
						return
				}
			}
		}

		// extract instances
		if (parsed.contains("instance")) {
			val instances = parsed("instance").asInstanceOf[Value.Arr].values
			for (instance <- instances) {
				GameBuilder.parseInstance(instance, catalogs) match {
					case Some(value) => unexpanded += value
					case None        =>
				}
			}
		}

		// extract connections
		if (parsed.contains("connection")) {
			val connections = parsed("connection").asInstanceOf[Value.Arr].values
			for (connection <- connections)
				GameBuilder.parseConnection(connection, catalogs) match {
					case Some(v) => this.unconnected += v
					case None    =>
				}
		}
	}


	private val validUnconnected: Boolean = {
		var okay = true
		for {c <- unconnected.filter(_.isInstanceOf[Unconnected])
		     uncon = c.asInstanceOf[Unconnected]} {
			uncon match {
				case v: UnconnectedBetween  =>
					val main      = unexpanded.find(_.matchIdent(v.main))
					val secondary = unexpanded.find(_.matchIdent(v.secondary))
					if (main.isEmpty) {
						println(s"main connection ${v.main} could not be " +
						        s"found in the instances")
						okay = false
					}
					if (secondary.isEmpty) {
						println(s"second connection ${v.secondary} could " +
						        s"not be found in the instances")
						okay = false
					}
				case v: UnconnectedConstant =>
					val secondary = unexpanded.find(_.matchIdent(v.to))
					if (secondary.isEmpty) {
						println(s"to connection ${v.to} could " +
						        s"not be found in the instances")
						okay = false
					}
				case _                      =>
			}
		}
		okay
	}

	private val connected = (for {
		c <- unconnected.filter(_.isUnconnected).map(_.asUnconnected)
	} yield {
		unconnected -= c
		c match {
			case v: UnconnectedBetween  =>
				val main      = unexpanded.find(_.matchIdent(v.main))
				val secondary = unexpanded.find(_.matchIdent(v.secondary))
				if (main.nonEmpty && secondary.nonEmpty)
					ConnectedBetween[Instance[_], Instance[_]](c.definition,
					                                           main.get,
					                                           secondary.get,
					                                           v)
				else {
					if (main.isEmpty)
						println(s"main ${v.main} not found")

					if (secondary.isEmpty)
						println(s"second ${v.secondary} not found")

					c
				}
			case v: UnconnectedConstant =>
				val secondary = unexpanded.find(_.matchIdent(v.to))
				if (secondary.nonEmpty)
					ConnectedConstant[Instance[_]](c.definition,
					                               v.constant,
					                               secondary.get,
					                               v)
				else {
					println(s"to ${v.to} not found")
					c
				}

			case _ =>
				println(s"Unknown Unconnected type $c")
				c
		}
	}).toSeq

	private val expandableInstances = unexpanded.filter(
		p => p.attributes.contains("count") && {
			p.attributes("count").isInstanceOf[Value.Num] &&
			p.attributes("count").asInstanceOf[Value.Num].value.toInt > 1
		}).toSeq

	private val expanded = (for {
		toExpand <- expandableInstances
		index <- 0 until {
			val tomlCount = toExpand.attributes("count")
			tomlCount.asInstanceOf[Value.Num].value.toInt
		}} yield {
		toExpand.copyMutate(
			nid = s"${toExpand.ident}.$index",
			nattribs = toExpand.attributes.filterNot(_._1 == "count"))
	}).toSeq ++ unexpanded.diff(expandableInstances)

	private val connectionsNeedExpanding = for {
		toExpand <- expandableInstances
		con <- connected.filter(_.isConnected).map(_.asConnected)
		if (con.connectsToInstance(toExpand))
	} yield con

	private val expandedConnections = {
		val result = mutable.ArrayBuffer[Connection]()
		val done   = mutable.ArrayBuffer[Connection]()

		for {
			con <- connectionsNeedExpanding
			if con.areConnectionCountsCompatible
			if !done.contains(con)
			i <- 0 until 2
		} {
			if (i == 1) done += con

			// expansion of connections requires equal counts on both sides
			// OR one side to be shared (NxMConnect are always shared)
			// we also have to ensure we dont double count when replicated both sides
			val doReplicate = (i == 0 && con.firstCount == con.secondaryCount) ||
			                  ((con.firstCount != con.secondaryCount) &&
			                   ((i == 0 && con.firstCount != 1) ||
			                    (i == 1 && con.secondaryCount != 1)))

			if (doReplicate) {
				val replicator = (if (i == 0) con.first else con.second)
				val count      = replicator
					.asInstanceOf[Instance[_]].attributes("count")
					.asInstanceOf[Value.Num].value.toInt
				for (index <- 0 until count) yield {
					val m: Instance[_] = {
						val mident = if (con.firstCount == 1) con.first.ident
						else s"${
							con.first.ident
						}.${
							index
						}"
						(expanded.find(p => p.ident == s"$mident") match {
							case Some(value) => value
							case None        => println(s"$mident isn't a instance name")
								con.first
						})
					}

					val s: Instance[_] = {
						val sident = if (con.secondaryCount == 1) con.second.ident
						else s"${
							con.second.ident
						}.${
							index
						}"
						(expanded.find(p => p.ident == s"$sident") match {
							case Some(value) => value
							case None        => println(s"$sident isn't a instance name")
								con.second
						})
					}

					result += (con match {
						case ConnectedBetween(d, _, _, u)  =>
							ConnectedBetween[Instance[_], Instance[_]](d, m, s, u)
						case ConnectedConstant(d, c, t, u) =>
							ConnectedConstant[Instance[_]](d, c, t, u)
						case v                             =>
							println(s"Expansion of unknown Connected Type?? $con")
							v
					})
				}
			}
		}
		result ++ connected.diff(connectionsNeedExpanding)
	}.toSeq

	def toGame: Option[Game] = {
		if (!validUnconnected) return None
		Some(Game(gameName, expandedConnections.toList, expanded.toList, board))
	}
}
