package input

import overlord.Chip.BitsDesc

import java.nio.file.{Files, Path}
import scala.collection.mutable

sealed trait VerilogBoundary

case class VerilogParameterKey(parameter: String) extends VerilogBoundary

case class VerilogPort(direction: String, bits: BitsDesc, name: String)
	extends VerilogBoundary

object VerilogModuleParser {
	private val bitRegEx      = "\\[\\d+:\\d+\\]".r
	private val moduleRegEx   = "\\s*module\\s+(\\w+)[\\s|#(]*".r
	private val endPortsRegEx = "\\);".r

	def apply(absolutePath: Path, name: String): Seq[VerilogBoundary] = {
		println(s"parsing $name verilog for ports and parameter")
		val txt      = load(absolutePath)
		val boundary = mutable.ArrayBuffer[VerilogBoundary]()
		for {i <- txt.indices;
		     if txt(i).contains("module") && !txt(i).contains("endmodule")
		     moduleRegEx(n) = txt(i)
		     if n == name
		     } {
			for {j <- i + 1 until txt.length} {
				val words = txt(j).split("\\s")
					.filterNot(w => w == "wire" || w == "reg" || w == "integer")
					.map(_.filter(c => (c.isLetterOrDigit || c == '_')))
					.filterNot(_.isBlank)
					.filterNot(_ (0).isDigit)
				if (words.length == 2) {
					val t = words(0)
					val b = bitRegEx.findFirstIn(txt(j)) match {
						case Some(value) => BitsDesc(value)
						case None        => BitsDesc(1)
					}
					val n = words(1)
					t match {
						case "input" | "output" | "inout" =>
							boundary += VerilogPort(t, b, n)
						case "parameter"                  =>
							boundary += VerilogParameterKey(n)
						case _                            =>
					}
				}
				if (endPortsRegEx.matches(txt(j))) {
					return boundary.toSeq
				}
			}
		}

		boundary.toSeq
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