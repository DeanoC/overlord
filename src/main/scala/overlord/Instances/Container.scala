package overlord.Instances

import overlord.Connections.Connection

import scala.collection.mutable

trait Container {
	val children: Seq[InstanceTrait]
	val physical: Boolean

	def flatChildren: Seq[InstanceTrait] =
		children.filter(_.isInstanceOf[Container])
			.map(_.asInstanceOf[Container]).flatMap(_.flatChildren) ++ children

	def chipChildren: Seq[ChipInstance] =
		children.filter(_.isInstanceOf[ChipInstance])
			.map(_.asInstanceOf[ChipInstance])

	def copyMutateContainer(copy: MutContainer): Container
}

class MutContainer(var children: mutable.Seq[InstanceTrait] = mutable.Seq(),
                   var connections: mutable.Seq[Connection] = mutable.Seq())

