package overlord

import actions.ActionsFile
import gagameos.{Utils, Variant}
import overlord.Hardware.{HardwareDefinition, Port, RegisterBank, Registers}
import overlord.Instances._
import overlord.Software.SoftwareDefinition
import overlord.Project

import java.nio.file.Path
import scala.sys.exit

trait DefinitionTrait {
	val defType     : DefinitionType
	val attributes  : Map[String, Variant]
	val sourcePath  : Path
	val dependencies: Seq[String]

	def createInstance(name: String, attribs: Map[String, Variant]): Option[InstanceTrait]
}

object Definition {
	def apply(defi: Variant,
	          defaults: Map[String, Variant]): DefinitionTrait = {

		val path = Project.catalogPath

		val table   = Utils.mergeAintoB(Utils.toTable(defi), defaults)
		val deftype = DefinitionType(Utils.toString(table("type"))) match {
			case _: ChipDefinitionType     => HardwareDefinition(table, path)
			case _: SoftwareDefinitionType => SoftwareDefinition(table, path)

			// TODO these aren't really chips, but piggyback off HardwareDefinition for now
			case _: PinGroupDefinitionType => HardwareDefinition(table, path)
			case _: ClockDefinitionType    => HardwareDefinition(table, path)
			case _: BoardDefinitionType    => HardwareDefinition(table, path)
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
	val ports       : Map[String, Port]
	val maxInstances: Int
	val defType     : DefinitionType
	val attributes  : Map[String, Variant]
	protected val registersV: Seq[Variant]
	var registers: Seq[RegisterBank] = Seq()

	def createInstance(name: String, attribs: Map[String, Variant]): Option[InstanceTrait] = {
		val instance = defType match {
			case _: RamDefinitionType     => RamInstance(name, this, attribs)
			case _: CpuDefinitionType     => CpuInstance(name, this, attribs)
			case _: GraphicDefinitionType => GraphicInstance(name, this, attribs)
			case _: StorageDefinitionType => StorageInstance(name, this, attribs)
			case _: NetDefinitionType     => NetInstance(name, this, attribs)
			case _: IoDefinitionType      => IoInstance(name, this, attribs)
			case _: SocDefinitionType     => SocInstance(name, this, attribs)
			case _: SwitchDefinitionType  => SwitchInstance(name, this, attribs)
			case _: OtherDefinitionType   => OtherInstance(name, this, attribs)

			case _: PinGroupDefinitionType => PinGroupInstance(name, this, attribs)
			case _: ClockDefinitionType    => ClockInstance(name, this, attribs)
			case _: BoardDefinitionType    => BoardInstance(name, this, attribs)
			case _                         =>
				println(s"$defType is invalid for chip\n")
				None
		}
		if (instance.nonEmpty) registers = Registers(instance.get, registersV)
		instance
	}
}

trait GatewareDefinitionTrait extends ChipDefinitionTrait {
	val actionsFile: ActionsFile
	val parameters : Map[String, Variant]
}

trait SoftwareDefinitionTrait extends DefinitionTrait {
	val actionsFilePath: Path
	val actionsFile    : ActionsFile
	val parameters     : Map[String, Variant]


	def createInstance(name: String,
	                   attribs: Map[String, Variant]
	                  ): Option[InstanceTrait] = {
		defType match {
			case _: LibraryDefinitionType => LibraryInstance(name, this, attribs)
			case _: ProgramDefinitionType => ProgramInstance(name, this, attribs)
			case _                        => println(s"$defType is invalid for software\n")
				None
		}
	}
}

trait HardwareDefinitionTrait extends ChipDefinitionTrait