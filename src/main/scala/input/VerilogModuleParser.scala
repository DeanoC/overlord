package input

import overlord.Hardware.BitsDesc

import scala.util.parsing.combinator._

import java.nio.file.{Files, Path}
import scala.collection.mutable

// Represents different boundaries within a Verilog module
sealed trait VerilogBoundary

// Represents a Verilog parameter
case class VerilogParameterKey(parameter: String) extends VerilogBoundary

// Represents a Verilog port with direction, bit width, name, and width knowledge
case class VerilogPort(
    direction: String,
    bits: BitsDesc,
    name: String,
    knownWidth: Boolean
) extends VerilogBoundary

// Represents a Verilog module with a name and its boundaries
case class VerilogModule(name: String, module_boundary: Seq[VerilogBoundary])

object VerilogModuleParser {
  // Regular expressions for parsing Verilog syntax
  private val bitRegEx = "\\[\\d+:\\d+\\]".r
  private val moduleRegEx = "\\s*module\\s+(\\w+)[\\s|#(]*".r
  private val endPortsRegEx = "\\);".r
  private val blockCommentRegEx =
    "(/\\*([^*]|[\\r\\n]|(\\*+([^*/]|[\\r\\n])))*\\*+/)|(//.*)".r

  // Parses a Verilog file and extracts modules
  def apply(absolutePath: Path, name: String): Seq[VerilogModule] = {
    println(s"parsing $name verilog for ports and parameter")

    val modules = mutable.ArrayBuffer[VerilogModule]()
    var txt = load(absolutePath)

    // Process the file line by line to extract modules
    while (txt.nonEmpty) {
      val (nl, module) = extractModule(txt)
      txt = txt.drop(nl)
      if (module.nonEmpty) modules += module.get
    }

    modules.toSeq
  }

  // Extracts a single module from the Verilog file
  private def extractModule(
      txt: Seq[String]
  ): (Integer, Option[VerilogModule]) = {
    import scala.util.boundary, boundary.break
    boundary {
      var moduleName = ""
      val module_boundary = mutable.ArrayBuffer[VerilogBoundary]()

      // Locate the module definition
      for {
        i <- txt.indices
        if txt(i).contains("module") && !txt(i).contains("endmodule")
        moduleRegEx(n) = txt(i): @unchecked
      } {
        moduleName = n
        boundary {
          // Parse the module's ports and parameters
          for (j <- i + 1 until txt.length) {
            if (
              txt(j).nonEmpty &&
              !txt(j).trim().startsWith("//") &&
              !blockCommentRegEx.matches(txt(j))
            ) {

              // Tokenize the line and filter out unnecessary words
              val words = txt(j)
                .trim()
                .split("\\s")
                .filterNot(w => w == "wire" || w == "reg" || w == "integer")
                .map(_.filter(c => c.isLetterOrDigit || c == '_'))
                .filterNot(_.isEmpty)
                .filterNot(_(0).isDigit)

              // Identify and process ports or parameters
              if (words.length == 2 || words.length == 3) {
                val t = words(0)
                val b = bitRegEx.findFirstIn(txt(j)) match {
                  case Some(value) => BitsDesc(value)
                  case None        => BitsDesc(1)
                }
                t match {
                  case "input" | "output" | "inout" =>
                    val n = words.last
                    module_boundary += VerilogPort(t, b, n, words.length == 2)
                  case "parameter" =>
                    val n =
                      words(1) // Parameters always have their name at index 1
                    module_boundary += VerilogParameterKey(n)
                  case _ =>
                }
              }

              // Break when the end of the port list is reached
              if (endPortsRegEx.matches(txt(j))) {
                boundary.break(
                  (j, Some(VerilogModule(moduleName, module_boundary.toSeq)))
                )
              }
            }
          }
        }
      }
      (txt.length, None)
    }
  }

  // Loads the Verilog file and returns its content as a sequence of lines
  private def load(absolutePath: Path): Seq[String] = {
    if (!Files.exists(absolutePath)) {
      println(s"$absolutePath does not exist")
      Seq()
    } else {
      val file = absolutePath.toFile
      val sourcetext = io.Source.fromFile(file)
      sourcetext.getLines().toSeq
    }
  }
}
