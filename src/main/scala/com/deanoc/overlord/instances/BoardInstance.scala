package com.deanoc.overlord.instances

import com.deanoc.overlord.utils._
import com.deanoc.overlord.interfaces.UnconnectedLike
import com.deanoc.overlord.definitions.{HardwareDefinition, Definition}
import com.deanoc.overlord.instances.Instance.variantToAny
import scala.util.boundary, boundary.break

import scala.collection.immutable

// Represents a generic type of board with default attributes.
sealed trait BoardType {
  val defaults: Map[String, Variant] // Default attributes for the board type.
}

// Represents a Xilinx board with specific family and device attributes.
case class XilinxBoard(family: String, device: String) extends BoardType {
  // Default attributes for Xilinx boards.
  override val defaults: Map[String, Variant] =
    immutable.Map[String, Variant](
      ("pullup" -> BooleanV(false)),
      ("slew" -> StringV("Slow")),
      ("drive" -> IntV(8)),
      ("direction" -> StringV("None")),
      ("standard" -> StringV("LVCMOS33"))
    )
}

// Represents an Altera board with no specific default attributes.
case class AlteraBoard() extends BoardType {
  // Default attributes for Altera boards.
  override val defaults: Map[String, Variant] =
    immutable.Map[String, Variant]()
}

// Represents a Lattice board with no specific default attributes.
case class LatticeBoard() extends BoardType {
  // Default attributes for Lattice boards.
  override val defaults: Map[String, Variant] =
    immutable.Map[String, Variant]()
}

// Represents a Gowin board with specific family and device attributes.
case class GowinBoard(family: String, device: String) extends BoardType {
  // Default attributes for Gowin boards.
  override val defaults: Map[String, Variant] =
    immutable.Map[String, Variant](
      ("pullup" -> BooleanV(false)),
      ("slew" -> StringV("Slow")),
      ("drive" -> IntV(8)),
      ("direction" -> StringV("None")),
      ("standard" -> StringV("LVCMOS33"))
    )
}

// Represents an instance of a board with its type, definition, and children.
case class BoardInstance(
    name: String,
    boardType: BoardType,
    override val definition: HardwareDefinition,
    override val children: Seq[InstanceTrait] = Seq()
) extends ChipInstance
    with Container {
  // Indicates whether the board is physical.
  override val physical: Boolean = true
  // Represents unconnected elements on the board.
  override val unconnected: Seq[UnconnectedLike] = Seq()

  // Determines if the board is visible to software.
  override def isVisibleToSoftware: Boolean = true
}

object BoardInstance {

  // Factory method to create a BoardInstance from attributes and a definition.
  def apply(
      name: String,
      definition: HardwareDefinition,
      config: com.deanoc.overlord.config.BoardConfig // Accept BoardConfig
  ): Either[String, BoardInstance] = {

    // Determine the type of board based on the config.board_type field.
    val boardTypeResult = config.board_type match {
      case "Xilinx" =>
        // Assuming board_family and board_device are now part of BoardConfig or definition attributes
        // For now, I'll assume they are still in definition.attributes and will need to be passed or looked up.
        // This part might need further refinement based on the actual structure of BoardConfig and definition attributes.
        // If board_family and board_device are moved to BoardConfig, update this logic.
        if (
          !definition.config.attributes.contains("board_family") ||
          !definition.config.attributes.contains("board_device")
        ) {
          Left(
            s"$name Xilinx board definition requires a board_family AND " +
              s"board_device field in its attributes"
          )
        } else {
          Right(
            XilinxBoard(
              Utils.toString(definition.config.attributesAsVariant("board_family")),
              Utils.toString(definition.config.attributesAsVariant("board_device"))
            )
          )
        }
      case "Altera"  => Right(AlteraBoard())
      case "Lattice" => Right(LatticeBoard())
      case "Gowin" =>
        // Similar assumption as Xilinx regarding board_family and board_device
        if (
          !definition.config.attributes.contains("board_family") ||
          !definition.config.attributes.contains("board_device")
        ) {
          Left(
            s"$name Gowin board definition requires a board_family AND " +
              s"board_device field in its attributes"
          )
        } else {
          Right(
            GowinBoard(
              Utils.toString(definition.config.attributesAsVariant("board_family")),
              Utils.toString(definition.config.attributesAsVariant("board_device"))
            )
          )
        }
      case _ =>
        Left(s"$name board has an unknown board_type: ${config.board_type}")
    }

    if (boardTypeResult.isLeft) {
      return Left(boardTypeResult.left.getOrElse("Unknown board type error"))
    }

    val boardType = boardTypeResult.toOption.get

    Left("Not implemented yet")
    /*
    // Clocks are now directly available in config.clocks
    val clocksResult = config.clocks.foldLeft[Either[String, Seq[InstanceTrait]]](Right(Seq.empty)) {
      case (Left(error), _) => Left(error)
      case (Right(clocksAcc), boardClockConfig) =>
        // Create a dummy VariantTable for now, as Definition still expects it.
        // This will be refactored later when Definition is updated.
        val clockTable = Map[String, Variant](
          "name" -> StringV(boardClockConfig.name),
          "type" -> StringV(boardClockConfig.name), // Assuming type is the same as name for clocks
          "frequency" -> StringV(boardClockConfig.frequency)
          // Add other relevant fields from BoardClockConfig if needed
        )
        // Definition.apply now returns Either
        // Create a map of Any values for the clock config
        val clockConfigMap = Map[String, Any](
          "name" -> boardClockConfig.name,
          "type" -> boardClockConfig.name,
          "frequency" -> boardClockConfig.frequency
        )
        
        val clockDefConfig = OtherDefinitionConfig(
          boardClockConfig.name,
          boardClockConfig.name,
          clockConfigMap
        )
        
        Definition(clockDefConfig, Map()) match {
          case Right(clockDefinition) =>
            // Pass the same config map as Option[Map[String, Any]]
            clockDefinition.createInstance(boardClockConfig.name, clockConfigMap).asInstanceOf[Either[String, InstanceTrait]] match {
              case Right(instance: InstanceTrait) =>
                Right(clocksAcc :+ instance)
              case Left(error) => Left(s"Error creating clock ${boardClockConfig.name}: $error")
            }
          case Left(error) => Left(s"Error getting definition for clock ${boardClockConfig.name}: $error")
        }
    }

    if (clocksResult.isLeft) {
      return Left(clocksResult.left.getOrElse("Error creating clocks"))
    }

    val clocks = clocksResult.toOption.get

    // Pingroups are not directly in BoardConfig, they are likely handled elsewhere
    // For now, I will keep the existing logic for pingroups, assuming they are still
    // processed from the definition's attributes or a separate part of the config.
    // This might need adjustment based on the overall refactoring plan.
    // If pingroups are moved to BoardConfig, update this logic.
    val pingroupsResult = definition.attributes.get("pingroups") match {
      case Some(pingroupsVariant) =>
        Utils
          .toArray(pingroupsVariant)
          .foldLeft[Either[String, Seq[InstanceTrait]]](Right(Seq.empty)) {
            case (Left(error), _) => Left(error)
            case (Right(pingroupsAcc), pinv) =>
              val table = Utils.toTable(pinv)
              if (table.contains("name")) {
                val name = Utils.toString(table("name"))
                val pingroup =
                  table ++ Map[String, Variant](
                    "type" -> StringV(s"pingroup.$name")
                  )
                // Definition.apply now returns Either
                // Convert Variant map to Any map
                val pingroupConfigMap = pingroup.map { case (k, v) =>
                  k -> Instance.variantToAny(v)
                }.toMap
                
                // Create a DefinitionConfig with the Any values
                val pingroupDefConfig = com.deanoc.overlord.config.OtherDefinitionConfig(
                  name,
                  s"pingroup.$name",
                  pingroupConfigMap
                )
                
                Definition(pingroupDefConfig, Map()) match {
                  case Right(pingroupDefinition) =>
                    pingroupDefinition.createInstance(s"$name", pingroupConfigMap).asInstanceOf[Either[String, InstanceTrait]] match {
                      case Right(instance: InstanceTrait) => Right(pingroupsAcc :+ instance)
                      case Left(error) => Left(s"Error creating pingroup $name: $error")
                    }
                  case Left(error) => Left(s"Error getting definition for pingroup $name: $error")
                }
              } else if (table.contains("names")) {
                val names = Utils.toArray(table("names"))
                names.foldLeft[Either[String, Seq[InstanceTrait]]](
                  Right(pingroupsAcc)
                ) {
                  case (Left(error), _) => Left(error)
                  case (Right(acc), nameV) =>
                    val name = Utils.toString(nameV)
                    val pingroup = table ++
                      Map[String, Variant]("type" -> StringV(s"pingroup.$name"))
                    // Definition.apply now returns Either
                    // Convert Variant map to Any map
                    val pingroupConfigMap = pingroup.map { case (k, v) =>
                      k -> Instance.variantToAny(v)
                    }.toMap
                    
                    // Create a DefinitionConfig with the Any values
                    val pingroupDefConfig = com.deanoc.overlord.config.OtherDefinitionConfig(
                      name,
                      s"pingroup.$name",
                      pingroupConfigMap
                    )
                    
                    Definition(pingroupDefConfig, Map()) match {
                      case Right(pingroupDefinition) =>
                        pingroupDefinition.createInstance(s"$name", pingroupConfigMap).asInstanceOf[Either[String, InstanceTrait]] match {
                          case Right(instance: InstanceTrait) => Right(acc :+ instance)
                          case Left(error) =>
                            Left(s"Error creating pingroup $name: $error")
                        }
                      case Left(error) => Left(s"Error getting definition for pingroup $name: $error")
                    }
                }
              } else {
                Left(s"pin groups must either have a name or names field")
              }
          }
      case None => Right(Seq.empty) // No pingroups defined
    }


    if (pingroupsResult.isLeft) {
      return Left(pingroupsResult.left.getOrElse("Error creating pingroups"))
    }

    val pingroups = pingroupsResult.toOption.get

    // Create and return the BoardInstance.
    Right(
      BoardInstance(
        name,
        boardType = boardType,
        definition = definition,
        children = clocks ++ pingroups
      )
    )
      */
  }
}
