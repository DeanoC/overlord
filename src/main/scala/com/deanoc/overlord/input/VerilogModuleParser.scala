package com.deanoc.overlord.input

import com.deanoc.overlord.Hardware.BitsDesc
import com.deanoc.overlord.utils.Logging

import scala.util.parsing.combinator._

import java.nio.file.{Files, Path}
import scala.collection.mutable
import scala.util.boundary, boundary.break
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

object VerilogModuleParser extends Logging {
  // Regular expressions for parsing Verilog syntax
  private val bitRegEx = "\\[\\d+:\\d+\\]".r
  // Update the module regex pattern to better match different module declaration styles
  private val moduleRegEx = "\\s*module\\s+(\\w+)\\s*[#(]?.*".r
  private val endPortsRegEx = "\\);".r
  private val blockCommentRegEx =
    "(/\\*([^*]|[\\r\\n]|(\\*+([^*/]|[\\r\\n])))*\\*+/)|(//.*)".r
  // Add regex for localparam declarations
  private val localParamRegEx = "\\s*localparam\\s+(\\w+).*".r
  // Add regex for multi-dimensional bit vectors
  private val multiBitRegEx = "\\[\\d+:\\d+\\]\\s*\\[\\d+:\\d+\\]".r

  // Parses a Verilog file and extracts modules
  def apply(
      absolutePath: Path,
      name: String
  ): Either[String, Seq[VerilogModule]] = {
    load(absolutePath) match {
      case Left(error) => Left(error)
      case Right(content) =>
        val txt = content
        val modules = extractModules(txt)
        
        modules.foreach { module =>
          debug(s"Parsed module: ${module.name}")
          module.module_boundary.foreach {
            case VerilogPort(direction, bits, name, knownWidth) =>
              debug(s"  Port - Direction: $direction, Name: $name, Bits: $bits, Known Width: $knownWidth")
            case VerilogParameterKey(parameter) =>
              debug(s"  Parameter - Name: $parameter")
          }
        }

        debug("Completed parsing modules.")
        Right(modules.toSeq)
    }
  }

  // Extracts all modules from the Verilog file
  private def extractModules(
      txt: Seq[String]
  ): Seq[VerilogModule] = {
    val modules = mutable.ArrayBuffer[VerilogModule]()
    var i = 0
    
    // Loop through all lines to find module definitions
    while (i < txt.length) {
      val line = txt(i).trim()
      // Check if the line contains a module definition
      if (line.contains("module") && !line.contains("endmodule")) {
        debug(s"START ${txt(i)}")
        // Extract module name using regex
        moduleRegEx.findFirstMatchIn(line) match {
          case Some(m) =>
            val moduleName = m.group(1)
            debug(s"Found module definition: $moduleName at line $i")
            val module_boundary = mutable.ArrayBuffer[VerilogBoundary]()
            
            // Process the first line of the module for potential port definitions
            val firstLineWords = line
              .split("\\s+|,")  // Split by whitespace or comma
              .filterNot(w => w == "wire" || w == "reg" || w == "integer" || w == "module")
              .map(_.trim)
              .filterNot(_.isEmpty)
              
            // Quickly parse simple ports defined on the module line itself
            if (firstLineWords.length >= 3) { // module name input/output name
              for (j <- 1 until firstLineWords.length - 1) {
                if (firstLineWords(j) == "input" || firstLineWords(j) == "output" || firstLineWords(j) == "inout") {
                  val t = firstLineWords(j)
                  val n = firstLineWords(j+1).replaceAll("[();]", "")
                  debug(s"Found port on module line: $t $n")
                  module_boundary += VerilogPort(t, BitsDesc(1), n, true)
                }
              }
            }
            
            // Continue processing the module body
            var j = i + 1
            var moduleEnded = false
            
            // Process module contents until we find the end
            while (j < txt.length && !moduleEnded) {
              val line = txt(j).trim()
              if (
                line.nonEmpty &&
                !line.startsWith("//") &&
                !blockCommentRegEx.matches(txt(j))
              ) {
                debug(s"Processing line $j: ${txt(j)}")
                
                // Special handling for parameter lines
                if (line.startsWith("parameter")) {
                  val paramPattern = "parameter\\s+(\\w+).*".r
                  line match {
                    case paramPattern(paramName) => 
                      debug(s"Found parameter by direct match: $paramName")
                      module_boundary += VerilogParameterKey(paramName)
                    case _ => 
                      debug(s"Parameter line found but couldn't extract name: $line")
                  }
                } else {
                  // Tokenize the line and filter out unnecessary words
                  val words = line
                    .split("\\s+|,")  // Split by whitespace or comma
                    .filterNot(w => w == "wire" || w == "reg" || w == "integer")
                    .map(_.trim)
                    .filterNot(_.isEmpty)
                    .take(2)  // Only take the first two tokens (direction and name)
                    .map(_.filter(c => c.isLetterOrDigit || c == '_'))
                    .filterNot(_.isEmpty)
                    .filterNot(w => w.nonEmpty && w(0).isDigit)

                  // Identify and process ports
                  if (words.length >= 2 && (words(0) == "input" || words(0) == "output" || words(0) == "inout")) {
                    val t = words(0)
                    val n = words(1)
                    val b = bitRegEx.findFirstIn(txt(j)) match {
                      case Some(value) => BitsDesc(value)
                      case None => BitsDesc(1)
                    }
                    debug(s"Found port: $t $n")
                    module_boundary += VerilogPort(t, b, n, words.length == 2)
                  }
                }

                // Check for end of port list
                if (endPortsRegEx.matches(txt(j))) {
                  debug(s"End of port list reached at line $j")
                  
                  // Now scan ahead for parameters and port declarations after the port list
                  var k = j + 1
                  var foundDefs = false
                  
                  while (k < txt.length && !txt(k).contains("endmodule")) {
                    val line = txt(k).trim()
                    if (
                      line.nonEmpty &&
                      !line.startsWith("//") &&
                      !blockCommentRegEx.matches(txt(k))
                    ) {
                      // Check for parameter declarations
                      if (line.startsWith("parameter")) {
                        foundDefs = true
                        val paramPattern = "parameter\\s+(\\w+).*".r
                        line match {
                          case paramPattern(paramName) => 
                            debug(s"Found parameter after port list: $paramName at line $k")
                            module_boundary += VerilogParameterKey(paramName)
                          case _ =>
                            debug(s"Parameter line after port list couldn't be parsed: $line")
                        }
                      } 
                      // Check for localparam declarations - they're often used like parameters
                      else if (line.startsWith("localparam")) {
                        foundDefs = true
                        line match {
                          case localParamRegEx(paramName) => 
                            debug(s"Found localparam after port list: $paramName at line $k")
                            // We treat localparams like parameters for simplicity
                            module_boundary += VerilogParameterKey(paramName)
                          case _ =>
                            debug(s"Localparam line couldn't be parsed: $line")
                        }
                      }
                      // Check for port declarations (input, output, inout)
                      else if (line.startsWith("input") || line.startsWith("output") || line.startsWith("inout")) {
                        foundDefs = true
                        
                        // Enhanced regex to handle more port declaration formats
                        val portPattern = "(input|output|inout)\\s+(?:(?:wire|reg)\\s+)?(?:\\[.*?\\]\\s*)?(\\w+).*".r
                        
                        line match {
                          case portPattern(direction, portName) =>
                            debug(s"Found port after port list: $direction $portName at line $k")
                            
                            // Handle multi-dimensional ports
                            val b = if (multiBitRegEx.findFirstIn(line).isDefined) {
                              debug(s"Found multi-dimensional port: $portName")
                              // For multi-dimensional ports, just use the first dimension for now
                              bitRegEx.findFirstIn(line) match {
                                case Some(value) => BitsDesc(value)
                                case None => BitsDesc(1)
                              }
                            } else {
                              bitRegEx.findFirstIn(line) match {
                                case Some(value) => BitsDesc(value)
                                case None => BitsDesc(1)
                              }
                            }
                            
                            module_boundary += VerilogPort(direction, b, portName, true)
                          case _ =>
                            // Try to handle array-style port declarations
                            val arrayPortPattern = "(input|output|inout)\\s+(?:(?:wire|reg)\\s+)?(?:\\[.*?\\]\\s*)?(\\w+)\\s*\\[.*?\\].*".r
                            line match {
                              case arrayPortPattern(direction, portName) =>
                                debug(s"Found array port: $direction $portName at line $k")
                                val b = bitRegEx.findFirstIn(line) match {
                                  case Some(value) => BitsDesc(value)
                                  case None => BitsDesc(1)
                                }
                                module_boundary += VerilogPort(direction, b, portName, true)
                              case _ =>
                                debug(s"Port line after port list couldn't be parsed: $line")
                            }
                        }
                      }
                      // Also check for wire/reg declarations with bit widths
                      else if (line.startsWith("wire") || line.startsWith("reg")) {
                        // These might be internal nets but could still be useful for analysis
                        debug(s"Found internal net declaration: $line")
                      }
                    }
                    k += 1
                  }
                  
                  debug(s"END ${txt(k)}")
                  if (foundDefs) {
                    debug(s"Added definitions from after port list")
                  }
                  
                  modules += VerilogModule(moduleName, module_boundary.toSeq)
                  moduleEnded = true
                  
                  // Skip to after this module to look for more modules
                  j = k + 1
                }
              }
              j += 1
              
              // Check if we've reached the end of the module
              if (j < txt.length && txt(j).contains("endmodule")) {
                debug(s"Found endmodule at line $j")
                moduleEnded = true
                // Add module if we haven't added it already
                if (!modules.exists(_.name == moduleName)) {
                  modules += VerilogModule(moduleName, module_boundary.toSeq)
                }
                j += 1 // Skip past the endmodule line
              }
            }
            
            // If we found the module end, resume outer scanning from there
            if (moduleEnded) {
              i = j  // j is already positioned after endmodule
            } else {
              // If we didn't find a proper end, we still add the module with what we found
              debug(s"No explicit end marker found for module $moduleName, adding anyway")
              if (!modules.exists(_.name == moduleName)) {
                modules += VerilogModule(moduleName, module_boundary.toSeq)
              }
              i = j
            }
          case None => 
            debug(s"Module keyword found but couldn't extract module name from: ${txt(i)}")
            i += 1
        }
      } else if (line.contains("endmodule")) {
        debug(s"END ${txt(i)}")
        i += 1
      } else {
        debug(s"END ${txt(i)}")
        i += 1
      }
    }
    
    debug(s"Found ${modules.length} modules: ${modules.map(_.name).mkString(", ")}")
    modules.toSeq
  }

  // Loads the Verilog file and returns its content as a sequence of lines
  private def load(absolutePath: Path): Either[String, Seq[String]] = {
    if (!Files.exists(absolutePath)) {
      Left(s"File not found: $absolutePath does not exist")
    } else {
      try {
        val file = absolutePath.toFile
        val sourcetext = io.Source.fromFile(file)
        val lines = sourcetext.getLines().toSeq
        sourcetext.close()
        Right(lines)
      } catch {
        case e: Exception =>
          Left(s"Error reading file $absolutePath: ${e.getMessage}")
      }
    } 
  }
}