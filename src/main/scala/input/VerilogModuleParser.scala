package input

import overlord.Gateware.BitsDesc

import java.nio.file.{Files, Path}
import scala.collection.mutable

sealed trait VerilogBoundary

case class VerilogParameter(parameter: String) extends VerilogBoundary

case class VerilogPort(direction: String, bits: BitsDesc, name: String)
	extends VerilogBoundary

object VerilogModuleParser {
	private val bitRegEx = "\\[\\d+:\\d+\\]".r

	def apply(absolutePath: Path, name: String): Seq[VerilogBoundary] = {
		println(s"parsing $name verilog for ports and parameter")
		val txt      = load(absolutePath)
		val boundary = mutable.ArrayBuffer[VerilogBoundary]()
		for {i <- txt.indices
		     if txt(i).contains("module")
		     if txt(i).split(" ").exists(_ == name)
		     } {
			for {j <- i + 1 until txt.length} {
				val words = txt(j).split(" ")
					.filterNot(w => w == "wire" || w == "reg")
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
							boundary += VerilogParameter(n)
						case _                            =>
					}
				}
				if (txt(j).contains(");")) {
					return boundary.toSeq
				}
			}
		}

		boundary.toSeq
	}

	private def load(absolutePath: Path): Seq[String] = {
		if (!Files.exists(absolutePath)) {
			println(s"$absolutePath does't not exists");
			Seq()
		} else {
			val file       = absolutePath.toFile
			val sourcetext = io.Source.fromFile(file)
			sourcetext.getLines().toSeq
		}
	}
}