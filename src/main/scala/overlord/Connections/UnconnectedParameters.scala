package overlord.Connections

import gagameos.{Utils, Variant}
import overlord.Instances.{ChipInstance, InstanceTrait}
import overlord.ConnectionDirection
import overlord._
import overlord.Interfaces._

sealed trait ParameterType

case class ConstantParameterType(value: Variant) extends ParameterType

case class FrequencyParameterType(freq: Double) extends ParameterType

case class Parameter(name: String, parameterType: ParameterType)

case class UnconnectedParameters(
    direction: ConnectionDirection,
    instanceName: String,
    parameters: Seq[Parameter]
) extends Unconnected {

  override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = Seq()

  override def collectConstants(
      unexpanded: Seq[InstanceTrait]
  ): Seq[Constant] = {
    val instances = matchInstances(instanceName, unexpanded)
    if (instances.isEmpty) return Seq()

    // constants hookup before gateware has generated its ports, so we assume that the port specified is valid
    for {
      instance <- instances
      param <- parameters
    } yield Constant(instance.instance, param)
  }

  override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

  override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}

object UnconnectedParameters {
  def apply(
      direction: ConnectionDirection,
      secondFullName: String,
      parametersV: Array[Variant]
  ): UnconnectedLike = {
    val parameters = parametersV.flatMap { v =>
      val table = Utils.toTable(v)
      if (!table.contains("name")) {
        println(
          s"ERROR: parameter table entry does have a name _ -> $secondFullName"
        )
        None
      } else {
        val name = Utils.lookupString(table, "name", "NO_NAME")
        Utils.lookupString(table, "type", "_") match {
          case "constant" =>
            Some(Parameter(name, ConstantParameterType(table("value"))))
          case "frequency" =>
            Some(
              Parameter(
                name,
                FrequencyParameterType(Utils.toFrequency(table("value")))
              )
            )
          case _ => None
        }
      }
    }

    new UnconnectedParameters(
      direction,
      secondFullName,
      parameters.toIndexedSeq
    )
  }
}
