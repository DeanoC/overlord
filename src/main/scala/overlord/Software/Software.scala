package overlord.Software

import java.nio.file.{Files, Path}
import overlord.Definitions.{SoftwareGroup, SoftwareTrait}
import overlord.{Game, Utils}
import toml.Value

import scala.collection.mutable

case class RegisterBank(name: String,
                        address: String)

case class Register(name: String,
                    regType: String,
                    width: Int,
                    default: String,
                    offset: String,
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
	def apply(registerDefs: Seq[toml.Value],
	          registerPath: Path): Option[Software] = {
		val groups = for (inlineTable <- registerDefs) yield {
			val item = inlineTable.asInstanceOf[Value.Tbl].values
			val name = item("registers").asInstanceOf[Value.Str].value

			val path = registerPath.resolve(Path.of(s"$name" + s".toml"))
			if (!Files.exists(path.toAbsolutePath)) {
				println(s"${path} register file not found");
				return None
			}

			Game.pathStack.push(path.getParent)
			val file   = path.toAbsolutePath.toFile
			val source = scala.io.Source.fromFile(file)
			val result = parse(path.toUri.toString,
			                   source.getLines().mkString("\n"))
			Game.pathStack.pop()
			result.toList
		}
		Some(Software(groups.flatten))
	}

	private def parse(name: String, src: String): Option[SoftwareGroup] = {
		import toml.Value

		val parsed = {
			val parsed = toml.Toml.parse(src)
			parsed match {
				case Right(value) => value.values
				case Left(value)  => println(s"$name has failed to parse with " +
				                             s"error ${Left(parsed)}")
					return None
			}
		}
		val desc   = if (parsed.contains("description"))
			parsed("description").asInstanceOf[Value.Str].value
		else "No Description"

		val tbanks = Utils.toArray(parsed("bank"))
		val banks  = for (bank <- tbanks) yield {
			val table   = Utils.toTable(bank)
			val name    = Utils.toString(table("name"))
			val address = Utils.toString(table("address"))
			RegisterBank(name, address)
		}

		val tregisters = parsed("register").asInstanceOf[Value.Arr].values
		val registers  = for (reg <- tregisters) yield {
			val table   = Utils.toTable(reg)
			val name    = Utils.toString(table("name"))
			val regType = Utils.toString(table("type"))
			val width   = Utils.toInt(table("width"))
			val default = Utils.toString(table("default"))
			val offset  = Utils.toString(table("offset"))
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
			}.toArray else Array[RegisterField]()

			Register(name, regType, width, default, offset, desc, fields)
		}

		Some(SoftwareGroup(desc, banks.toArray,
		                   registers.sortBy(p => (p.offset, p.name)).toArray))
	}
}
