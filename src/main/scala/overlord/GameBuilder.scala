package overlord

import java.nio.file.{Files, Paths}

import toml.Value

import scala.collection.mutable

object GameBuilder {
	def gameFrom(gameName: String,
	             gameText: String,
	             catalogs: DefinitionCatalogs,
	             board: Board): Option[Game] = {
		(new GameBuilder(gameName, gameText, catalogs, board)).toGame
	}
}

private class GameBuilder(gameName: String,
                          gameText: String,
                          catalogs: DefinitionCatalogs,
                          board: Board) {

	private val unconnected = mutable.ArrayBuffer[Connection]()
	private val unexpanded  = mutable.ArrayBuffer[Instance[_]]()

	parseInstances(board.instances, catalogs)
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

	private def parseConnections(connections: List[toml.Value],
	                             catalogs: DefinitionCatalogs): Unit = {
		import toml.Value
		for (connection <- connections) {
			val table     = connection.asInstanceOf[Value.Tbl].values
			val name      = table("name").asInstanceOf[Value.Str].value
			val conntype  = table("type").asInstanceOf[Value.Str].value
			val main      = table("main").asInstanceOf[Value.Str].value
			val secondary = table("secondary").asInstanceOf[Value.Str].value

			catalogs.FindDefinition(conntype) match {
				case Some(value) => this.unconnected += Unconnected(name,
				                                                    value,
				                                                    main,
				                                                    secondary)

				case None => println(s"$name connection type $conntype isn't in" +
				                     s" any definition catalog")
					return
			}

		}
	}

	def parseInstances(instances: List[toml.Value],
	                   catalogs: DefinitionCatalogs): Unit = {

		for (instance <- instances) {
			val table    = instance.asInstanceOf[Value.Tbl].values
			val chipType = table("type").asInstanceOf[Value.Str].value
			val name     = tableStringGetOrElse(table, "name", chipType)

			val attribs: Map[String, Value] = table.filter(
				_._1 match {
					case "type" | "name" => false
					case _               => true
				})

			this.unexpanded += {
				catalogs.FindDefinition(chipType) match {
					case Some(c) => {
						chipType.split('.').head match {
							case "ram"             => RamInstance(name, c, attribs)
							case "cpu"             => CpuInstance(name, c, attribs)
							case "NxMinterconnect" => NxMInstance(name, c, attribs)
							case "storage"         => StorageInstance(name, c, attribs)
							case "soc"             => SocInstance(name, c, attribs)
							case "bridge"          => BridgeInstance(name, c, attribs)
							case "net"             => NetInstance(name, c, attribs)
							case _                 => println(s"$chipType has unknown class")
								UnknownInstance(name, c, attribs)
						}
					}
					case None    => println(s"${chipType} not found in any chip " +
					                        s"catalogs")
						return
				}
			}
		}
	}

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

				FromResourceFile(incResource) match {
					case Some(d) => includeOver(d, catalogs)
					case _       => println(
						"Include resource file ${incResource} not found")
						return
				}
			}
		}

		// buses this over has if any
		if (parsed.contains("connection")) {
			val connections = parsed("connection").asInstanceOf[Value.Arr].values
			parseConnections(connections, catalogs)
		}

		// extract instances
		if (parsed.contains("instance")) {
			val instances = parsed("instance").asInstanceOf[Value.Arr].values
			parseInstances(instances, catalogs)
		}
	}

	private def FromResourceFile(name: String): Option[String] = {
		println(s"Reading $name over")

		val path = Paths.get(s"src/main/resources/overs/${name}.over")
		if (!Files.exists(path.toAbsolutePath)) {
			println(s"${name} catalog at ${path} not found");
			return None
		}

		val chipFile = path.toAbsolutePath.toFile
		val source   = scala.io.Source.fromFile(chipFile)

		Some(source.getLines().mkString("\n"))
	}

	private val validUnconnected: Boolean = {
		var okay = true
		for {c <- unconnected.filter(_.isInstanceOf[Unconnected])
		     uncon = c.asInstanceOf[Unconnected]} {
			val main      = unexpanded.find(_.ident == uncon.main)
			val secondary = unexpanded.find(_.ident == uncon.secondary)
			if (main.isEmpty) {
				println(s"${uncon.ident} main connection ${uncon.main} could not be " +
				        s"found in the instances")
				okay = false
			}
			if (secondary.isEmpty) {
				println(s"${uncon.ident} main connection ${uncon.secondary} could " +
				        s"not be found in the instances")
				okay = false
			}
		}
		okay
	}

	private val connected: Seq[Connection] = {
		val newConnected = for {
			c <- unconnected.filter(_.isInstanceOf[Unconnected])
				.map(_.asInstanceOf[Unconnected])
			main = unexpanded.find(_.ident == c.main)
			secondary = unexpanded.find(_.ident == c.secondary)
			if main.isDefined
			if secondary.isDefined
		} yield {
			unconnected -= c
			Connected[Instance[_], Instance[_]](c.ident,
			                                    c.definition,
			                                    main.get,
			                                    secondary.get)
		}
		newConnected.toSeq
	}

	private val expandableInstances = unexpanded.filter(
		p => p.attributes.contains("count") && {
			val tomlCount = p.attributes("count")
			tomlCount.isInstanceOf[Value.Num] &&
			tomlCount.asInstanceOf[Value.Num].value.toInt > 1
		})

	private val expanded: Seq[Instance[_]] = {
		val accum = mutable.ArrayBuffer[Instance[_]]()
		for (toExpand <- expandableInstances) {
			val count = {
				val tomlCount = toExpand.attributes("count")
				tomlCount.asInstanceOf[Value.Num].value.toInt
			}
			for (index <- 0 until count) {
				accum += toExpand.copyAndMutate(
					nid = s"${toExpand.ident}.$index",
					nattribs = toExpand.attributes.filterNot(_._1 == "count"))
			}
		}
		accum.toSeq ++ unexpanded.diff(expandableInstances)
	}

	val connectionsNeedExpanding: mutable.Seq[Connected[_, _]] =
		for {
			toExpand <- expandableInstances
			con <- connected
			if (con.connectsToInstance(toExpand))
		} yield con.asInstanceOf[Connected[_, _]]

	private val expandedConnections: Seq[Connection] = {
		val result = mutable.ArrayBuffer[Connection]()
		val done   = mutable.ArrayBuffer[Connection]()

		for {con <- connectionsNeedExpanding
		     if (con.areConnectionCountsCompatible)
		     if !done.contains(con)
		     i <- 0 until 2} {
			if (i == 1) done += con

			// expansion of connections requires equal counts on both sides
			// OR one side to be shared (NxMConnect are always shared)
			// we also have to ensure we dont double count when replicated both sides
			val doReplicate = (i == 0 && con.mainCount == con.secondaryCount) ||
			                  ((con.mainCount != con.secondaryCount) &&
			                   ((i == 0 && con.mainCount != 1) ||
			                    (i == 1 && con.secondaryCount != 1)))

			if (doReplicate) {
				val replicator = (if (i == 0) con.main else con.secondary)
				val count      = replicator
					.asInstanceOf[Instance[_]].attributes("count")
					.asInstanceOf[Value.Num].value.toInt
				for (index <- 0 until count) yield {
					val mident = if (con.mainCount == 1)
						con.main.asInstanceOf[Instance[_]].ident
					else
						s"${con.main.asInstanceOf[Instance[_]].ident}.${index}"
					val sident = if (con.secondaryCount == 1)
						con.secondary.asInstanceOf[Instance[_]].ident
					else
						s"${con.secondary.asInstanceOf[Instance[_]].ident}.${index}"

					val m: Instance[_] =
						(expanded.find(p => p.ident == s"$mident") match {
							case Some(value) => value
							case None        => println(s"$mident " +
							                            s"isn't a instance name")
								con.main
						}).asInstanceOf[Instance[_]]
					val s: Instance[_] =
						(expanded.find(p => p.ident == s"$sident")
						match {
							case Some(value) => value
							case None        => println(s"$sident " +
							                            s"isn't a instance name")
								con.secondary
						}).asInstanceOf[Instance[_]]
					result +=
					Connected[Instance[_], Instance[_]](con.ident, con.definition, m, s)
				}
			}
		}
		result ++ connected.diff(result ++ connectionsNeedExpanding)
	}.toSeq

	def toGame: Option[Game] = {
		if (!validUnconnected) return None
		Some(Game(gameName, expandedConnections.toList, expanded.toList))
	}
}
