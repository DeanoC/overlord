package overlord.Chip

import ikuy_utils.{StringV, TableV, Utils, Variant}
import overlord.Game

import java.nio.file.{Files, Path}
import scala.collection.mutable

case class RegisterBank(name: String,
                        address: BigInt,
                        registerListName: String)

case class Register(name: String,
                    regType: String,
                    width: Int,
                    default: BigInt,
                    offset: BigInt,
                    desc: String,
                    fields: Array[RegisterField])

case class RegisterList(name: String,
                        description: String,
                        registers: Array[Register]) {
	def sizeInBytes: BigInt = registers.last.offset

}

case class RegisterField(name: String,
                         bits: String,
                         accessType: String,
                         enums: Array[RegisterFieldEnum],
                         shortDesc: Option[String],
                         longDesc: Option[String])

case class RegisterFieldEnum(name: String,
                             value: BigInt,
                             description: Option[String])

case class Registers(banks: Seq[RegisterBank], lists: Seq[RegisterList])

object Registers {
	def apply(registerDefs: Seq[Variant],
	          registerPath: Path): Registers = {

		val tomls = mutable.ArrayBuffer[(String, Map[String, Variant])]()

		for (inlineTable <- registerDefs) yield {
			val item = inlineTable.asInstanceOf[TableV].value
			if(!item.contains("resource")) {
				println(s"No resource in register table\n")
				return Registers(Seq(),Seq())
			}
			val resource = item("resource").asInstanceOf[StringV].value

			val path = registerPath.resolve(Path.of(s"$resource" + s".toml"))
			if (!Files.exists(path.toAbsolutePath)) {
				println(s"$path register file not found")
				return Registers(Seq(),Seq())
			}

			Game.pathStack.push(path.getParent)
			val source = Utils.readToml(resource, path, getClass)
			tomls += ((resource, source))
			Game.pathStack.pop()
		}

		val registerLists = for ((name, source) <- tomls) yield {
			parseRegisterList(name, source)
		}
		val registerBanks = for ((name, source) <- tomls) yield {
			parseRegisterBanks(name, source)
		}

		Registers(registerBanks.flatten.toSeq, registerLists.flatten.toSeq)
	}

	private def parseRegisterList(name: String, parsed: Map[String, Variant])
	: Option[RegisterList] = {
		if (!parsed.contains("register")) return None

		val tregisters = Utils.toArray(parsed("register"))
		val registers  = for (reg <- tregisters) yield {
			val table   = Utils.toTable(reg)
			val regName = Utils.toString(table("name"))
			val regType = Utils.toString(table("type"))
			val width   = Utils.toInt(table("width"))
			val default = Utils.toBigInt(table("default"))
			val offset  = Utils.toBigInt(table("offset"))
			val desc    = if (table.contains("description")) {
				Utils.toString(table("description"))
			} else { "" }

			val fields = if (table.contains("field")) {
				for (field <- Utils.toArray(table("field"))) yield {
					val table     = Utils.toTable(field)
					val fieldName = Utils.toString(table("name"))
					val fieldBits = Utils.toString(table("bits"))
					val fieldType = if (table.contains("type")) {
						Utils.toString(table("type"))
					} else {
						""
					}

					val shortDesc = if (table.contains("shortdesc"))
						Some(Utils.toString(table("shortdesc")))
					else None

					val longDesc = if (table.contains("longdesc"))
						Some(Utils.toString(table("longdesc")))
					else None

					val enums = if (table.contains("enum")) {
						for (enum <- Utils.toArray(table("enum"))) yield {
							val table     = Utils.toTable(enum)
							val enumName  = Utils.toString(table("name"))
							val enumValue = Utils.toBigInt(table("value"))
							val enumDesc  = if (table.contains("description"))
								Some(Utils.toString(table("description")))
							else None
							RegisterFieldEnum(enumName, enumValue, enumDesc)
						}
					} else Array[RegisterFieldEnum]()

					RegisterField(fieldName, fieldBits, fieldType, enums, shortDesc, longDesc)
				}
			} else Array[RegisterField]()

			Register(regName, regType, width, default, offset, desc, fields)
		}
		val desc       = if (parsed.contains("description"))
			parsed("description").asInstanceOf[StringV].value
		else "No Description"

		Some(RegisterList(name, desc, registers))
	}

	private def parseRegisterBanks(name: String, parsed: Map[String, Variant])
	: Seq[RegisterBank] = {
		if (!parsed.contains("bank")) return Seq[RegisterBank]()

		val tbanks = Utils.toArray(parsed("bank"))
		val banks  = for (bank <- tbanks) yield {
			val table    = Utils.toTable(bank)
			val bankName = Utils.toString(table("name"))
			val address  = Utils.toBigInt(table("address"))
			RegisterBank(bankName, address, name)
		}
		banks.toSeq
	}
}
