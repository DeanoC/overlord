package overlord

import java.nio.file.{Files, Path}

import toml.Value

import scala.collection.mutable

case class RegisterBank(val name: String,
                        val address: String)

case class Register(val name: String,
                    val regType: String,
                    val width: Int,
                    val default: String,
                    val offset: String,
                    val desc: String,
                    val fields: Array[RegisterField])

case class RegisterField(val name: String,
                         val bits: String,
                         val accessType: String,
                         val shortDesc: Option[String],
                         val longDesc: Option[String])

// registers should be sorted!
case class Software(description: String,
                    banks: Array[RegisterBank],
                    registers: Array[Register]) {
	def bankSize: String = registers.last.offset
}

object Software {
	def defFromFile(registerDefs: Seq[toml.Value],
	                registerPath: Path
	               ): Seq[Software] =
		(for (inlineTable <- registerDefs) yield {
			val item = inlineTable.asInstanceOf[Value.Tbl].values
			val name = item("registers").asInstanceOf[Value.Str].value

			val path = registerPath.resolve(Path.of(s"$name" + s".toml"))
			if (!Files.exists(path.toAbsolutePath)) {
				println(s"${path} register file not found");
				return Seq[Software]()
			}

			GameBuilder.pathStack.push(path.getParent)
			val file   = path.toAbsolutePath.toFile
			val source = scala.io.Source.fromFile(file)
			val result = parse(path.toUri.toString,
			                   source.getLines().mkString("\n"))
			GameBuilder.pathStack.pop()
			result.toList
		}).fold(List[Software]())({ (s, n) => s ++ n })

	private def parse(name: String, src: String): Option[Software] = {
		val banks     = mutable.ArrayBuffer[RegisterBank]()
		val registers = mutable.ArrayBuffer[Register]()
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

		val tbanks = parsed("bank").asInstanceOf[Value.Arr].values
		for (bank <- tbanks) {
			val table   = bank.asInstanceOf[Value.Tbl].values
			val name    = table("name").asInstanceOf[Value.Str].value
			val address = table("address").asInstanceOf[Value.Str].value
			banks += RegisterBank(name, address)
		}

		val tregisters = parsed("register").asInstanceOf[Value.Arr].values
		for (reg <- tregisters) {
			val table   = reg.asInstanceOf[Value.Tbl].values
			val name    = table("name").asInstanceOf[Value.Str].value
			val regType = table("type").asInstanceOf[Value.Str].value
			val width   = table("width").asInstanceOf[Value.Num].value.toInt
			val default = table("default").asInstanceOf[Value.Str].value
			val offset  = table("offset").asInstanceOf[Value.Str].value
			val desc    = table("description").asInstanceOf[Value.Str].value

			val fields = if (table.contains("field")) {
				for (field <- table("field").asInstanceOf[Value.Arr].values) yield {
					val table     = field.asInstanceOf[Value.Tbl].values
					val fieldName = table("name").asInstanceOf[Value.Str].value
					val fieldBits = table("bits").asInstanceOf[Value.Str].value
					val fieldType = table("type").asInstanceOf[Value.Str].value

					val shortDesc = if (table.contains("shortdesc"))
						Some(table("shortdesc").asInstanceOf[Value.Str].value)
					else None

					val longDesc = if (table.contains("longdesc"))
						Some(table("longdesc").asInstanceOf[Value.Str].value)
					else None

					RegisterField(fieldName, fieldBits, fieldType, shortDesc, longDesc)
				}
			}.toArray else Array[RegisterField]()
			registers += Register(name, regType, width, default, offset, desc,
			                      fields.toArray)
		}

		Some(Software(desc, banks.toArray,
		              registers.sortBy(p => (p.offset, p.name)).toArray))
	}
}
