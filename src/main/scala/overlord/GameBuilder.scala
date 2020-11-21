package overlord

import java.nio.file.{Files, Path, Paths}

import overlord.Connections._
import overlord.Instances.Instance
import toml.Value

import scala.collection.mutable

object GameBuilder {
	def gameFrom(
		            gameName: String,
		            gameText: String,
		            catalogs: DefinitionCatalog,
		            board: Board): Option[Game] = {
		(new GameBuilder(gameName, gameText, catalogs, board)).toGame
	}

	var pathStack     : mutable.Stack[Path]           =
		mutable.Stack[Path]()
	var containerStack: mutable.Stack[Option[String]] =
		mutable.Stack[Option[String]]()

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

}

private class GameBuilder(gameName: String,
                          gameText: String,
                          catalogs: DefinitionCatalog,
                          board: Board) {

	private val unconnected = mutable.ArrayBuffer[Connection]()
	private val unexpanded  = mutable.ArrayBuffer[Instance]()

	unexpanded ++= board.instances
	includeOver(gameText, catalogs)

	private def includeOver(data: String, catalogs: DefinitionCatalog): Unit = {
		val parsed = {
			val parsed = toml.Toml.parse(data)
			parsed match {
				case Right(value) => value.values
				case Left(_)      => println(s"game.over has failed to parse with " +
				                             s"error ${Left(parsed)}")
					return
			}
		}

		// includes
		if (parsed.contains("includes")) {
			val tincs = Utils.toArray(parsed("includes"))
			for (include <- tincs) {
				val table       = include.asInstanceOf[Value.Tbl].values
				val incResource = Utils.toString(table("resource"))

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
			val instances = Utils.toArray(parsed("instance"))
			unexpanded ++= instances.flatMap(Instance(_, catalogs))
		}

		// extract connections
		if (parsed.contains("connection")) {
			val connections = Utils.toArray(parsed("connection"))
			unconnected ++= connections.flatMap(Connection(_, catalogs))
		}
	}

	private val (expanded, expandedConnections) =
		Connection.expandAndConnect(unconnected.toSeq, unexpanded.toSeq)

	def toGame: Option[Game] = {
		Some(Game(gameName, expandedConnections.toList, expanded.toList, board))
	}
}
