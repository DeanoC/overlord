package overlord

import scala.collection.immutable.Map

case class BoardCatalog(val catalog: Map[String, Board]) {
	def FindBoard(boardName: String): Option[Board] =
		if (catalog.contains(boardName)) Some(catalog(boardName))
		else {
			println(s"${boardName} not found in the board catalog");
			return None
		}
}
