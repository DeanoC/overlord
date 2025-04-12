package com.deanoc.overlord.input

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.utils.{TableV, StringV, BigIntV}
import com.deanoc.overlord.Hardware.{RegisterBank, Registers}
import com.deanoc.overlord.Instances.{ChipInstance, InstanceTrait}
import com.deanoc.overlord.Project

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
        Left(
          s"Error parsing $name yaml for register definitions: ${e.getMessage}"
        )
    }
  }
}
