package overlord.Software

import java.nio.file.{Files, Path}
import overlord.Definitions.{SoftwareGroup, SoftwareTrait}
import overlord.Game
import ikuy_utils._

case class RegisterBank(name: String,
                        address: BigInt)

case class Register(name: String,
                    regType: String,
                    width: Int,
                    default: BigInt,
                    offset: BigInt,
                    desc: String,
                    fields: Array[RegisterField])

case class RegisterField(name: String,
                         bits: String,
                         accessType: String,
                         shortDesc: Option[String],
                         longDesc: Option[String])

// registers should be sorted!
case class Software(groups: Seq[SoftwareGroup]) extends SoftwareTrait

object Software {
	def apply(registerDefs: Seq[Variant],
	          registerPath: Path): Option[Software] = {
		val groups = for (inlineTable <- registerDefs) yield {
			val item = inlineTable.asInstanceOf[TableV].value
			val name = item("registers").asInstanceOf[StringV].value

			val path = registerPath.resolve(Path.of(s"$name" + s".toml"))
			if (!Files.exists(path.toAbsolutePath)) {
				println(s"${path} register file not found");
				return None
			}

			Game.pathStack.push(path.getParent)
			val source = Utils.readToml(name, path, getClass)
			val result = parse(name, source)
			Game.pathStack.pop()
			result.toList
		}
		Some(Software(groups.flatten))
	}

	private def parse(name: String, parsed: Map[String, Variant])
	: Option[SoftwareGroup] = {

		val desc   = if (parsed.contains("description"))
			parsed("description").asInstanceOf[StringV].value
		else "No Description"

		val tbanks = Utils.toArray(parsed("bank"))
		val banks  = for (bank <- tbanks) yield {
			val table   = Utils.toTable(bank)
			val name    = Utils.toString(table("name"))
			val address = Utils.toBigInt(table("address"))
			RegisterBank(name, address)
		}

		val tregisters = Utils.toArray(parsed("register"))
		val registers  = for (reg <- tregisters) yield {
			val table   = Utils.toTable(reg)
			val name    = Utils.toString(table("name"))
			val regType = Utils.toString(table("type"))
			val width   = Utils.toInt(table("width"))
			val default = Utils.toBigInt(table("default"))
			val offset  = Utils.toBigInt(table("offset"))
			val desc    = Utils.toString(table("description"))

			val fields = if (table.contains("field")) {
				for (field <- Utils.toArray(table("field"))) yield {
					val table     = Utils.toTable(field)
					val fieldName = Utils.toString(table("name"))
					val fieldBits = Utils.toString(table("bits"))
					val fieldType = Utils.toString(table("type"))

					val shortDesc = if (table.contains("shortdesc"))
						Some(Utils.toString(table("shortdesc")))
					else None

					val longDesc = if (table.contains("longdesc"))
						Some(Utils.toString(table("longdesc")))
					else None

					RegisterField(fieldName, fieldBits, fieldType, shortDesc, longDesc)
				}
			} else Array[RegisterField]()

			Register(name, regType, width, default, offset, desc, fields)
		}

		Some(SoftwareGroup(desc, banks, registers.sortBy(p => (p.offset, p.name))))
	}
}
