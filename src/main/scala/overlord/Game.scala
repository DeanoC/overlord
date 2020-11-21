package overlord

import overlord.Connections.{Connected, ConnectedBetween, ConnectedConstant, Connection}
import overlord.Definitions.GatewareTrait
import overlord.Instances._

case class Game(name: String,
                connections: List[Connection],
                instances: List[Instance],
                board: Board) {

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

	def peripherals: Seq[Instance] = storages ++ nets

	def gatewares: Seq[(Instance,GatewareTrait)] =
		instances.filter(_.definition.gateware.nonEmpty)
			.map( g =>(g, g.definition.gateware.get))

	def constraints: Seq[ConstraintInstance] =
		instances.filter(_.isInstanceOf[ConstraintInstance])
			.map(_.asInstanceOf[ConstraintInstance])

	def constants: Seq[ConnectedConstant] =
		connections.filter(_.isInstanceOf[ConnectedConstant])
			.map(_.asInstanceOf[ConnectedConstant])

	def constraintConnecteds: Set[Connected] = {
		val cb = connections.filter(f => f.isInstanceOf[ConnectedBetween])
			.map(_.asInstanceOf[ConnectedBetween])
		val cc = connections.filter(f => f.isInstanceOf[ConnectedConstant])
			.map(_.asInstanceOf[ConnectedConstant])

		val cbm = cb.filter(_.main.isInstanceOf[ConstraintInstance])
		val cbs = cb.filter(_.secondary.isInstanceOf[ConstraintInstance])
		val cct = cc.filter(_.to.isInstanceOf[ConstraintInstance])

		(cbm ++ cbs ++ cct).toSet
	}

	def connectedConstraints: Seq[ConstraintInstance] = {
		val cb = connections.filter(f => f.isInstanceOf[ConnectedBetween])
			.map(_.asInstanceOf[ConnectedBetween])
		val cc = connections.filter(f => f.isInstanceOf[ConnectedConstant])
			.map(_.asInstanceOf[ConnectedConstant])

		val cbm = cb.filter(_.main.isInstanceOf[ConstraintInstance])
			.map(_.main.asInstanceOf[ConstraintInstance])
		val cbs = cb.filter(_.second.isInstanceOf[ConstraintInstance])
			.map(_.second.asInstanceOf[ConstraintInstance])
		val cct = cc.filter(_.to.isInstanceOf[ConstraintInstance])
			.map(_.to.asInstanceOf[ConstraintInstance])
		cbm ++ cbs ++ cct
	}
}

object Game {
	def newGame(gameName: String,
	            gameText: String,
	            catalogs: DefinitionCatalog,
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
