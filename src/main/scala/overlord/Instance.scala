package overlord

import toml.Value

trait Instance[T <: Instance[T]] {
	val ident     : String
	val definition: Definition
	val attributes: Map[String, Value]

	def matchIdent(a: String): Boolean = (a == ident)

	def copyMutate(nid: String, nattribs: Map[String, Value]): Instance[T]

	def count: Int =
		if (attributes.contains("count"))
			attributes("count").asInstanceOf[Value.Num].value.toInt
		else 1

	def shared: Boolean = attributes.contains("shared")
}

case class RamInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, Value],
                      ) extends Instance[RamInstance] {
	def copyMutate(nid: String, nattribs: Map[String, Value]): RamInstance =
		copy(ident = nid, attributes = nattribs)

}

case class CpuInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, Value],
                      ) extends Instance[CpuInstance] {
	def copyMutate(nid: String, nattribs: Map[String, Value]): CpuInstance =
		copy(ident = nid, attributes = nattribs)

}

case class NxMInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, Value]
                      ) extends Instance[NxMInstance] {
	def copyMutate(nid: String, nattribs: Map[String, Value]): NxMInstance =
		copy(ident = nid, attributes = nattribs)

	override def shared: Boolean = true
}

case class StorageInstance(ident: String,
                           definition: Definition,
                           attributes: Map[String, Value]
                          ) extends Instance[StorageInstance] {
	def copyMutate(nid: String, nattribs: Map[String, Value]): StorageInstance =
		copy(ident = nid, attributes = nattribs)

}

case class SocInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, Value]
                      ) extends Instance[SocInstance] {
	def copyMutate(nid: String, nattribs: Map[String, Value]): SocInstance =
		copy(ident = nid, attributes = nattribs)

}

case class BridgeInstance(ident: String,
                          definition: Definition,
                          attributes: Map[String, Value]
                         ) extends Instance[BridgeInstance] {
	def copyMutate(nid: String, nattribs: Map[String, Value]): BridgeInstance =
		copy(ident = nid, attributes = nattribs)

}

case class NetInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, Value]
                      ) extends Instance[NetInstance] {
	def copyMutate(nid: String, nattribs: Map[String, Value]): NetInstance =
		copy(ident = nid, attributes = nattribs)
}


case class UnknownInstance(ident: String,
                           definition: Definition,
                           attributes: Map[String, toml.Value]
                          ) extends Instance[UnknownInstance] {
	def copyMutate(nid: String, nattribs: Map[String, Value]): UnknownInstance =
		copy(ident = nid, attributes = nattribs)

}
