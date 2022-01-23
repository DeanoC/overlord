package overlord.Instances

import overlord.Connections.Connection

trait Container {
	var children   : Seq[InstanceTrait]
	var connections: Seq[Connection]
	val physical   : Boolean

	def flatChildren: Seq[InstanceTrait] =
		(children.filter(_.isInstanceOf[Container])
			 .map(_.asInstanceOf[Container]).flatMap(_.flatChildren) ++ children).toSeq

	def chipChildren: Seq[ChipInstance] =
		children.filter(_.isInstanceOf[ChipInstance])
			.map(_.asInstanceOf[ChipInstance]).toSeq
}

class RootContainer extends Container {
	override val physical   : Boolean            = true
	override var children   : Seq[InstanceTrait] = Seq()
	override var connections: Seq[Connection]    = Seq()
}

