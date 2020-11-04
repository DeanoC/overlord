package overlord


case class Game(name: String,
                connections: List[Connection],
                instances: List[Instance[_]]) {

	def cpus: Seq[CpuInstance] =
		instances.filter(_.isInstanceOf[CpuInstance])
			.map(_.asInstanceOf[CpuInstance])

	def rams: Seq[RamInstance] = instances.filter(_.isInstanceOf[RamInstance])
		.map(_.asInstanceOf[RamInstance])

	def storages: Seq[StorageInstance] =
		instances.filter(_.isInstanceOf[StorageInstance])
			.map(_.asInstanceOf[StorageInstance])

	def nets: Seq[NetInstance] =
		instances.filter(_.isInstanceOf[NetInstance])
			.map(_.asInstanceOf[NetInstance])

	def peripherals: Seq[Instance[_]] = storages ++ nets

}

object Game {

	def newGame(gameName: String,
	            gameText: String,
	            catalogs: DefinitionCatalogs,
	            boards: BoardCatalog): Option[Game] = {

		import toml.Value

		if (gameText.isEmpty) return None

		val parsed = {
			val parsed = toml.Toml.parse(gameText)
			parsed match {
				case Right(value) => value.values
				case Left(value)  => println(s"game.over has failed to parse with " +
				                             s"error ${Left(parsed)}")
					return None
			}
		}

		if (!parsed.contains("board")) {
			println(s"game.over requires a board value")
			return None
		}

		// fix name (remove '-')
		val boardName = parsed("board").asInstanceOf[Value.Str]
			.value.filterNot(c => c == '-')

		val board = (boards.FindBoard(boardName) match {
			case Some(b) => b
			case None    => println(s"$boardName board wasn't found in board " +
			                        s"catalog")
				return None
		})

		val name = gameName.split('/').last.split('.').head

		GameBuilder.gameFrom(name, gameText, catalogs, board)
	}
}
