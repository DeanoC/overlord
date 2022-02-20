package overlord

import actions.ActionsFile
import ikuy_utils.{Utils, Variant}
import overlord.Chip.{ChipDefinition, Port, Registers}
import overlord.Instances._
import overlord.Software.SoftwareDefinition

import java.nio.file.Path
import scala.sys.exit

trait DefinitionTrait {
	val defType   : DefinitionType
	val attributes: Map[String, Variant]

	def createInstance(name: String, attribs: Map[String, Variant]): Option[InstanceTrait]
}

object Definition {
	def apply(defi: Variant,
	          path: Path,
	          defaults: Map[String, Variant]): DefinitionTrait = {
		val table   = Utils.mergeAintoB(Utils.toTable(defi), defaults)
		val deftype = DefinitionType(Utils.toString(table("type"))) match {
			case _: RamDefinitionType      => ChipDefinition(table, path)
			case _: CpuDefinitionType      => ChipDefinition(table, path)
			case _: BusDefinitionType      => ChipDefinition(table, path)
			case _: StorageDefinitionType  => ChipDefinition(table, path)
			case _: BridgeDefinitionType   => ChipDefinition(table, path)
			case _: NetDefinitionType      => ChipDefinition(table, path)
			case _: IoDefinitionType       => ChipDefinition(table, path)
			case _: OtherDefinitionType    => ChipDefinition(table, path)
			case _: PinGroupDefinitionType => ChipDefinition(table, path)
			case _: ClockDefinitionType    => ChipDefinition(table, path)
			case _: BoardDefinitionType    => ChipDefinition(table, path)
			case _: ProgramDefinitionType  => SoftwareDefinition(table, path)
			case _: LibraryDefinitionType  => SoftwareDefinition(table, path)
		}
		deftype match {
			case Some(value) => value
			case None        =>
				println(s"Invalid definition type $path")
				exit()
		}

	}
}

trait ChipDefinitionTrait extends DefinitionTrait {
	val ports    : Map[String, Port]
	val registers: Option[Registers]

	def createInstance(name: String,
	                   attribs: Map[String, Variant]): Option[ChipInstance] = {
		defType match {
			case _: RamDefinitionType      => RamInstance(name, this, attribs)
			case _: CpuDefinitionType      => CpuInstance(name, this, attribs)
			case _: BusDefinitionType      => BusInstance(name, this, attribs)
			case _: StorageDefinitionType  => StorageInstance(name, this, attribs)
			case _: BridgeDefinitionType   => BridgeInstance(name, this, attribs)
			case _: NetDefinitionType      => NetInstance(name, this, attribs)
			case _: IoDefinitionType       => IoInstance(name, this, attribs)
			case _: OtherDefinitionType    => OtherInstance(name, this, attribs)
			case _: PinGroupDefinitionType => PinGroupInstance(name, this, attribs)
			case _: ClockDefinitionType    => ClockInstance(name, this, attribs)
			case _: BoardDefinitionType    => BoardInstance(name, this, attribs)
			case _                         =>
				println(s"$defType is invalid for chip\n")
				None
		}
	}
}

trait GatewareDefinitionTrait extends ChipDefinitionTrait {
	val actionsFile: ActionsFile
	val parameters : Map[String, Variant]
}

trait SoftwareDefinitionTrait extends DefinitionTrait {
	val actionsFile: ActionsFile
	val parameters : Map[String, Variant]

	def createInstance(name: String,
	                   attribs: Map[String, Variant]
	                  ): Option[InstanceTrait] = {
		defType match {
			case _: LibraryDefinitionType => LibraryInstance(name, this, attribs)
			case _: ProgramDefinitionType => ProgramInstance(name, this, attribs)
			case _                        => println(s"$defType is invalid for software\n");
				None
		}
	}
}