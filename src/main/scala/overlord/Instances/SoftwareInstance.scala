package overlord.Instances

import overlord.SoftwareDefinitionTrait

trait SoftwareInstance extends InstanceTrait {
  val folder: String = ""

  override def definition: SoftwareDefinitionTrait
}
