package overlord.Instances

import gagameos._
import overlord.Interfaces.UnconnectedLike
import overlord.{ChipDefinitionTrait, Definition}
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
    override val definition: ChipDefinitionTrait,
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
      definition: ChipDefinitionTrait,
      iattribs: Map[String, Variant]
  ): Either[String, BoardInstance] = {

    // Merge instance attributes with the definition's attributes.
    val attribs = Utils.mergeAintoB(iattribs, definition.attributes)

    // Ensure required attributes are present.
    if (!attribs.contains("board_type")) {
      return Left(s"${name} board requires a type value")
    }

    if (!attribs.contains("clocks")) {
      return Left(s"${name} board requires some clocks")
    }

    if (!attribs.contains("pingroups")) {
      return Left(s"${name} board requires some pingroups")
    }      
    // Determine the type of board based on attributes.
    val boardTypeResult = Utils.toString(attribs("board_type")) match {
      case "Xilinx" =>
        if (
          !attribs.contains("board_family") ||
          !attribs.contains("board_device")
        ) {
          Left(
            s"$name Xilinx board requires a board_family AND " +
              s"board_device field"
          )
        } else {
          Right(XilinxBoard(
            Utils.toString(attribs("board_family")),
            Utils.toString(attribs("board_device"))
          ))
        }
      case "Altera"  => Right(AlteraBoard())
      case "Lattice" => Right(LatticeBoard())
      case "Gowin" =>
        if (
          !attribs.contains("board_family") ||
          !attribs.contains("board_device")
        ) {
          Left(
            s"$name Gowin board requires a board_family AND " +
              s"board_device field"
          )
        } else {
          Right(GowinBoard(
            Utils.toString(attribs("board_family")),
            Utils.toString(attribs("board_device"))
          ))
        }
      case _ =>
        Left(s"$name board has an unknown board_type")
    }
    
    if (boardTypeResult.isLeft) {
      return Left(boardTypeResult.left.getOrElse("Unknown board type error"))
    }
      
    val boardType = boardTypeResult.toOption.get
      
    // Extract default attributes if present.
    val defaults = if (attribs.contains("defaults")) {
      Utils.toTable(attribs("defaults"))
    } else Map[String, Variant]()
      
    // Instantiate all clocks defined in the attributes.
      val clocksResult = Utils.toArray(attribs("clocks")).foldLeft[Either[String, Seq[InstanceTrait]]](Right(Seq.empty)) { 
        case (Left(error), _) => Left(error)
        case (Right(clocksAcc), pinv) => 
          val table = Utils.toTable(pinv)
          if (table.contains("name")) {
            val name = Utils.toString(table("name"))
            val clock = table ++ Map[String, Variant]("type" -> StringV(name))
            Definition(TableV(clock), defaults).createInstance(s"$name", clock) match {
              case Right(instance: InstanceTrait) => Right(clocksAcc :+ instance)
              case Left(error) => Left(s"Error creating clock $name: $error")
            }
          } else {
            Left(s"clocks must have a name field")
          }
      }
      
      if (clocksResult.isLeft) {
        return Left(clocksResult.left.getOrElse("Error creating clocks"))
      }
      
      val clocks = clocksResult.toOption.get      // Instantiate all pingroups defined in the attributes.
      val pingroupsResult = Utils.toArray(attribs("pingroups")).foldLeft[Either[String, Seq[InstanceTrait]]](Right(Seq.empty)) {
        case (Left(error), _) => Left(error)
        case (Right(pingroupsAcc), pinv) =>
          val table = Utils.toTable(pinv)
          if (table.contains("name")) {
            val name = Utils.toString(table("name"))
            val pingroup =
              table ++ Map[String, Variant]("type" -> StringV(s"pingroup.$name"))
            Definition(TableV(pingroup), defaults)
              .createInstance(s"$name", pingroup) match {
                case Right(instance: InstanceTrait) => Right(pingroupsAcc :+ instance)
                case Left(error) => Left(s"Error creating pingroup $name: $error")
              }
          } else if (table.contains("names")) {
            val names = Utils.toArray(table("names"))
            names.foldLeft[Either[String, Seq[InstanceTrait]]](Right(pingroupsAcc)) {
              case (Left(error), _) => Left(error)
              case (Right(acc), nameV) =>
                val name = Utils.toString(nameV)
                val pingroup = table ++
                  Map[String, Variant]("type" -> StringV(s"pingroup.$name"))
                Definition(TableV(pingroup), defaults)
                  .createInstance(s"$name", pingroup) match {
                    case Right(instance: InstanceTrait) => Right(acc :+ instance)
                    case Left(error) => Left(s"Error creating pingroup $name: $error")
                  }
            }
          } else {
            Left(s"pin groups must either have a name or names field")
          }
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
  }
}
