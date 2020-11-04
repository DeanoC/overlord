package overlord

import toml.Value

trait Instance[T <: Instance[T]] {
	val ident     : String
	val definition: Definition
	val attributes: Map[String, toml.Value]

	def copyAndMutate(nid: String,
	                  nattribs: Map[String, toml.Value]): Instance[T]

	def count: Int =
		if (attributes.contains("count"))
			attributes("count").asInstanceOf[toml.Value.Num].value.toInt
		else 1

	def shared: Boolean = attributes.contains("shared")
}

case class RamInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, toml.Value],
                      ) extends Instance[RamInstance] {
	override def copyAndMutate(nid: String, nattribs: Map[String, Value])
	: Instance[RamInstance] = copy(ident = nid, attributes = nattribs)

}

case class CpuInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, toml.Value],
                      ) extends Instance[CpuInstance] {
	override def copyAndMutate(nid: String, nattribs: Map[String, Value])
	: Instance[CpuInstance] = copy(ident = nid, attributes = nattribs)

}

case class NxMInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, toml.Value]
                      ) extends Instance[NxMInstance] {
	override def copyAndMutate(nid: String, nattribs: Map[String, Value])
	: Instance[NxMInstance] = copy(ident = nid, attributes = nattribs)

	override def shared: Boolean = true
}

case class StorageInstance(ident: String,
                           definition: Definition,
                           attributes: Map[String, toml.Value]
                          ) extends Instance[StorageInstance] {
	override def copyAndMutate(nid: String, nattribs: Map[String, Value])
	: Instance[StorageInstance] = copy(ident = nid, attributes = nattribs)

}

case class SocInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, toml.Value]
                      ) extends Instance[SocInstance] {
	override def copyAndMutate(nid: String, nattribs: Map[String, Value])
	: Instance[SocInstance] = copy(ident = nid, attributes = nattribs)

}

case class BridgeInstance(ident: String,
                          definition: Definition,
                          attributes: Map[String, toml.Value]
                         ) extends Instance[BridgeInstance] {
	override def copyAndMutate(nid: String, nattribs: Map[String, Value])
	: Instance[BridgeInstance] = copy(ident = nid, attributes = nattribs)

}

case class NetInstance(ident: String,
                       definition: Definition,
                       attributes: Map[String, toml.Value]
                      ) extends Instance[NetInstance] {
	override def copyAndMutate(nid: String, nattribs: Map[String, Value])
	: Instance[NetInstance] = copy(ident = nid, attributes = nattribs)
}

case class UnknownInstance(ident: String,
                           definition: Definition,
                           attributes: Map[String, toml.Value]
                          ) extends Instance[UnknownInstance] {
	override def copyAndMutate(nid: String, nattribs: Map[String, Value])
	: Instance[UnknownInstance] = copy(ident = nid, attributes = nattribs)

}
