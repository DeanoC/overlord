package overlord.Instances

import overlord.Interfaces.UnconnectedLike

trait Container {
	var children   : Seq[InstanceTrait]
	var unconnected: Seq[UnconnectedLike]
	val physical   : Boolean

	def flatChildren: Seq[InstanceTrait] =
		(children.filter(_.isInstanceOf[Container])
			 .map(_.asInstanceOf[Container]).flatMap(_.flatChildren) ++ children).toSeq

	def chipChildren: Seq[ChipInstance] =
		children.filter(_.isInstanceOf[ChipInstance])
			.map(_.asInstanceOf[ChipInstance]).toSeq
}

case class RootContainer() extends Container {
	override val physical   : Boolean              = true
	override var children   : Seq[InstanceTrait]   = Seq()
	override var unconnected: Seq[UnconnectedLike] = Seq()
}

