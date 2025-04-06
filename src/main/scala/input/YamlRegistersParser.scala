package input

import gagameos._
import overlord.Hardware.{RegisterBank, Registers}
import overlord.Instances.{ChipInstance, InstanceTrait}
import overlord.Project

object YamlRegistersParser {
  def apply(
      instance: InstanceTrait,
      filename: String,
      name: String
  ): Seq[RegisterBank] = {
    println(s"parsing $name yaml for register definitions")
    val regSeq = Seq(
      TableV(
        Map(
          "resource" -> StringV(filename),
          "name" -> StringV(name),
          "base_address" -> BigIntV(0)
        )
      )
    )
    Registers(instance, regSeq)
  }
}
