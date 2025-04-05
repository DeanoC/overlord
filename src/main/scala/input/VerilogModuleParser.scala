package input

import overlord.Chip.BitsDesc

import java.nio.file.{Files, Path, Paths}
import scala.collection.mutable

sealed trait VerilogBoundary

case class VerilogParameterKey(parameter: String) extends VerilogBoundary

case class VerilogPort(direction: String, bits: BitsDesc, name: String, knownWidth: Boolean)
	extends VerilogBoundary

case class VerilogModule(name: String,
                         boundary: Seq[VerilogBoundary])

object VerilogModuleParser {
	private val bitRegEx          = "\\[\\d+:\\d+\\]".r
	private val moduleRegEx       = "\\s*module\\s+(\\w+)[\\s|#(]*".r
	private val endPortsRegEx     = "\\);".r
	private val blockCommentRegEx = "(/\\*([^*]|[\\r\\n]|(\\*+([^*/]|[\\r\\n])))*\\*+/)|(//.*)".r

	def apply(absolutePath: Path, name: String): Seq[VerilogModule] = {
		println(s"parsing $name verilog for ports and parameter")

		val modules = mutable.ArrayBuffer[VerilogModule]()

		var txt = load(absolutePath)
		while (txt.nonEmpty) {
			val (nl, module) = extractModule(txt)
			txt = txt.drop(nl)
			if (module.nonEmpty) modules += module.get
		}

		modules.toSeq
	}

	private def extractModule(txt: Seq[String]): (Integer, Option[VerilogModule]) = {
		var moduleName = ""
		val boundary   = mutable.ArrayBuffer[VerilogBoundary]()
		for {i <- txt.indices;
		     if txt(i).contains("module") && !txt(i).contains("endmodule")
		     moduleRegEx(n) = txt(i): @unchecked
		     } {
			moduleName = n
			for {j <- i + 1 until txt.length} {
				if (txt(j).nonEmpty &&
				    !txt(j).trim().startsWith("""//""") &&
				    !blockCommentRegEx.matches(txt(j))) {
					val words = {
						txt(j).trim().split("\\s")
							.filterNot(w => w == "wire" || w == "reg" || w == "integer")
							.map(_.filter(c => (c.isLetterOrDigit || c == '_')))
							.filterNot(_.isEmpty)
							.filterNot(_ (0).isDigit)
					}
					if (words.length == 2 || words.length == 3) {
						val t = words(0)
						val b = bitRegEx.findFirstIn(txt(j)) match {
							case Some(value) => BitsDesc(value)
							case None        => BitsDesc(1)
						}
						t match {
							case "input" | "output" | "inout" =>
								val n = words.last
								boundary += VerilogPort(t, b, n, words.length == 2)
							case "parameter"                  =>
								val n = words(1) // always word 1 for parameters as there may be an = XXX part we can ignore
								boundary += VerilogParameterKey(n)
							case _                            =>
						}
					}
					if (endPortsRegEx.matches(txt(j))) {
						return (j, Some(VerilogModule(moduleName, boundary.toSeq)))
					}
				}
			}
		}

		(txt.length, None)
	}

	private def load(absolutePath: Path): Seq[String] = {
		if (!Files.exists(absolutePath)) {
			println(s"$absolutePath does not exists");
			Seq()
		} else {
			val file       = absolutePath.toFile
			val sourcetext = io.Source.fromFile(file)
			sourcetext.getLines().toSeq
		}
	}
}