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
  ): Either[String, Seq[RegisterBank]] = {
    try {
      val regSeq = Seq(
        TableV(
          Map(
            "resource" -> StringV(filename),
            "name" -> StringV(name),
            "base_address" -> BigIntV(0)
          )
        )
      )
      Right(Registers(instance, regSeq))
    } catch {
      case e: Exception => 
        Left(s"Error parsing $name yaml for register definitions: ${e.getMessage}")
    }
  }
}
