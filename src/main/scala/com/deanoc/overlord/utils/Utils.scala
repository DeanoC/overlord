package com.deanoc.overlord.utils

import io.circe.{Decoder, Json} // Import Decoder and Json explicitly
import io.circe.yaml.parser // Import parser for YAML parsing
import org.yaml.snakeyaml.Yaml

import java.io._
import java.nio.file.{Files, Path}
import scala.collection.compat.immutable.LazyList
import scala.collection.mutable
import scala.util.{Failure, Success, Try}
import scala.jdk.CollectionConverters.*

sealed trait Variant {
  def toYamlString: String

  def toCString: String
}

case class ArrayV(arr: Array[Variant]) extends Variant {
  override def toYamlString: String =
    arr.map(v => s"- ${v.toYamlString.replaceAll("(?m)^", "  ").trim}").mkString("\n")

  override def toCString: String = {
    arr.map(_.toYamlString).mkString("[ ", ", ", " ]")
  }

  def value: Array[Variant] = arr
}

case class BigIntV(bigInt: BigInt) extends Variant {
  def value: BigInt = bigInt

  override def toYamlString: String = {
    Try {
      bigInt.toLong
    } match {
      case Failure(exception) => s"'${bigInt.toString(16)}'"
      case Success(value)     => value.toString
    }
  }

  override def toCString: String = {
    Try {
      bigInt.toLong
    } match {
      case Failure(exception) => s"'${bigInt.toString(16)}'"
      case Success(value)     => value.toString
    }
  }
}

case class BooleanV(boolean: Boolean) extends Variant {
  override def toYamlString: String = value.toString

  override def toCString: String = if (value) "TRUE" else "FALSE"

  def value: Boolean = boolean
}

case class IntV(int: Int) extends Variant {
  override def toYamlString: String = value.toString

  override def toCString: String = value.toString

  def value: Int = int
}

case class TableV(table: Map[String, Variant]) extends Variant {
  override def toYamlString: String = {
    value.map { case (k, v) => 
      val valueStr = v.toYamlString
      if (v.isInstanceOf[TableV] || v.isInstanceOf[ArrayV]) {
        s"$k:\n${valueStr.replaceAll("(?m)^", "  ")}"
      } else {
        s"$k: $valueStr"
      }
    }.mkString("\n")
  }

  override def toCString: String = {
    value
      .map { case (k, v) => s"$k, ${v.toString}\n" }
      .mkString("[ ", ", ", " ]")
  }

  def value: Map[String, Variant] = table

}

case class StringV(string: String) extends Variant {
  def value: String = string

  override def toYamlString: String = {
    if (value.contains("\n") || value.contains("\"") || value.contains("'") || 
        value.contains(":") || value.contains("{") || value.contains("}") ||
        value.trim.isEmpty) {
      "\"" + value.replace("\"", "\\\"") + "\""
    } else {
      value
    }
  }

  override def toCString: String = s"""$value"""
}

case class DoubleV(dbl: Double) extends Variant {
  override def toYamlString: String = value.toString

  override def toCString: String = value.toString

  def value: Double = dbl
}

object Utils extends Logging {

  type VariantTable = Map[String, Variant]
  type MutableVariantTable = mutable.HashMap[String, Variant]

  def pow2(x: Double): Double = scala.math.pow(2, x)

  def log2(x: Double): Double = scala.math.log10(x) / scala.math.log10(2.0)

  def mergeAintoB(a: VariantTable, b: VariantTable): VariantTable =
    a ++ (for { s <- b } yield if (!a.contains(s._1)) Some(s) else None).flatten

  def copy(srcAbsPath: Path, dstAbsPath: Path): Unit = {
    Option(dstAbsPath.getParent).foreach { parent =>
      parent match {
        case null => // Do nothing
        case nonNullParent =>
          Utils.ensureDirectories(nonNullParent.asInstanceOf[Path])
      }
    }
    val source = Utils.readFile(srcAbsPath)
    source match {
      case Some(value) =>
        Utils.writeFile(dstAbsPath, value)
      case None =>
        warn(f"$srcAbsPath does not exist%n")
    }
  }

  def ensureDirectories(path: Path): Unit = {
    val directory = path.toFile
    if (!directory.exists()) directory.mkdirs()
  }

  def deleteDirectories(path: Path): Unit = {
    val directory = path.toFile
    deleteRecursively(directory)
  }

  def deleteFileIfExists(path: Path): Unit = {
    if (Files.exists(path)) {
      val file = path.toFile
      file.delete()
    }
  }

  def rename(src: Path, dst: Path): Unit = {
    val srcFile = src.toFile
    val dstFile = dst.toFile
    srcFile.renameTo(dstFile)
  }

  def doesFileOrDirectoryExist(path: Path): Boolean = {
    Files.exists(path)
  }

  def createSymbolicLink(src: Path, dst: Path): Unit = {
    Files.createSymbolicLink(dst.toAbsolutePath, src.toAbsolutePath)
  }

  def setFileExecutable(file: Path): Unit = {
    val fileFile = file.toFile
    fileFile.setExecutable(true)
  }

  def toBigInt(t: Variant): BigInt = t match {
    case b: BigIntV => b.value
    // string to int included some postfix
    case s: StringV =>
      parseBigInt(s.value) match {
        case Some(value) => value
        case None =>
          val numString = "([0-9]+)".r.findFirstIn(s.value)
          val multString = "([a-zA-Z-]+)".r.findFirstIn(s.value)

          val num = numString match {
            case Some(s) =>
              parseBigInt(s) match {
                case Some(value) => value
                case None        => error(s"ERR $t not a big int"); return 0
              }
            case None => error(s"ERR $t not a big int"); return 0
          }

          val mult = multString match {
            case Some(s) =>
              s.toLowerCase match {
                case "kb" | "kib" => BigInt(1024)
                case "mb" | "mib" => BigInt(1024 * 1024)
                case "gb" | "gib" => BigInt(1024 * 1024 * 1024)
                case _            => BigInt(1)
              }
            case None => BigInt(1)
          }

          num * mult
      }
    case i: IntV => BigInt(i.value)
    case _       => error(s"ERR $t not a big int"); 0
  }

  def toFrequency(t: Variant): Double = t match {
    case b: BigIntV => b.value.toDouble
    // string to int included some postfix
    case s: StringV =>
      parseBigInt(s.value) match {
        case Some(value) => value.toDouble
        case None =>
          val numString = "([0-9]+)".r.findFirstIn(s.value)
          val multString = "([a-zA-Z-]+)".r.findFirstIn(s.value)

          val num = numString match {
            case Some(s) =>
              parseBigInt(s) match {
                case Some(value) => value.toFloat
                case None        => error(s"ERR $t not a big int"); return 0
              }
            case None => error(s"ERR $t not a big int"); return 0
          }

          val mult = multString match {
            case Some(s) =>
              s.toLowerCase match {
                case "khz" => 1024.0
                case "mhz" => 1024.0 * 1024.0
                case "ghz" => 1024.0 * 1024.0 * 1024.0
                case _     => 1.0
              }
            case None => 1.0
          }
          num * mult
      }
    case _ => error(s"ERR $t not a big int"); 0
  }

  def writeFile(path: Path, s: String): Unit = {
    val file = path.toFile
    val bw = new BufferedWriter(new FileWriter(file))
    bw.write(s)
    bw.close()
  }

  def readFile(path: Path): Option[String] = {
    val file = path.toAbsolutePath.toFile
    if (!file.exists()) return None;
    val br = new BufferedReader(new FileReader(file))
    val s = LazyList
      .continually(br.readLine())
      .takeWhile(_ != null)
      .mkString("\n")
    br.close()
    Some(s)
  }

  def readYaml(yamlPath: Path): Map[String, Variant] = {
    val source = readFile(yamlPath) match {
      case Some(value) => value
      case None        =>
        // Log and return an empty map if the file cannot be read
        warn(s"$yamlPath YAML file could not be read.")
        return Map[String, Variant]()
    }

    fromYaml(source)
  }
  
  def fromYaml(yamlContent: String): Map[String, Variant] = {
    val yaml = new Yaml()
    try {
      val parsed = yaml.load(yamlContent).asInstanceOf[java.util.Map[String, Any]]
      parsed.asScala.map { case (k, v) => k -> toVariant(v) }.toMap
    } catch {
      case e: Exception =>
        // Log the warning instead of treating it as an error
        trace(s"Parsing YAML content: $yamlContent")
        warn(s"Error parsing YAML content: ${e.getMessage}")
        Map[String, Variant]()
    }
  }

  def loadAndParseYamlFile[T](
      filePath: Path
  )(implicit decoder: Decoder[T]): Either[String, T] = {
    if (!Files.exists(filePath.toAbsolutePath)) {
      return Left(s"Local catalog file not found at $filePath")
    }

    if (Files.size(filePath.toAbsolutePath) == 0) {
      return Left(s"Local catalog file is empty: $filePath")
    }

    val yamlString = scala.util.Try(scala.io.Source.fromFile(filePath.toFile).mkString) match {
      case scala.util.Success(content) => content
      case scala.util.Failure(exception) =>
        return Left(s"Failed to read file $filePath.toAbsolutePath: ${exception.getMessage}")
    }

    val json = parser.parse(yamlString) match {
      case Right(parsedJson) => parsedJson
      case Left(err) =>
        return Left(s"Failed to parse YAML to JSON from $filePath: ${err.getMessage}")
    }

    json.as[T] match {
      case Right(config) => Right(config)
      case Left(err) =>
        val errorDetails = err match {
          case decodingFailure: io.circe.DecodingFailure =>
            val historyPath = decodingFailure.history.map {
              case io.circe.CursorOp.DownField(field) => s"Field: '$field'"
              case io.circe.CursorOp.DownArray        => "Array element"
              case other                              => other.toString
            }.mkString(" -> ")
            s"""
               |Decoding Failure:
               |  Message: ${decodingFailure.message}
               |  History: $historyPath
               |""".stripMargin.trim
          case null =>
            s"Unexpected error: null"
        }
        Left(s"Failed to decode JSON to ${implicitly[Decoder[T]].getClass.getSimpleName} from $filePath:\n$errorDetails")
    }
  }

  def lookupString(tbl: VariantTable, key: String, or: String): String =
    if (tbl.contains(key)) {
      Utils.toString(tbl(key))
    } else {
      or
    }

  def toTable(t: Variant): Map[String, Variant] = t match {
    case tbl: TableV => tbl.value
    case _ =>
      error(s"$t not a Table")
      Map[String, Variant]()
  }

  def toVariant(t: Any): Variant = t match {
    case null                 => StringV("")
    case v: String            => StringV(v)
    case v: java.lang.Boolean => BooleanV(v)
    case v: java.lang.Double  => DoubleV(v)
    case v: java.lang.Float   => DoubleV(v.toDouble)
    case v: java.lang.Integer => IntV(v)
    case v: java.lang.Long    => BigIntV(BigInt(v))
    case v: java.lang.Short   => IntV(v.toInt)
    case v: java.lang.Byte    => IntV(v.toInt)
    case v: BigInt            => BigIntV(v)
    case v: scala.math.BigDecimal => DoubleV(v.toDouble)
    case v: java.math.BigDecimal  => DoubleV(v.doubleValue())
    case v: java.math.BigInteger  => BigIntV(BigInt(v))
    case v: TableV =>
      v // Return the TableV instance as it is already a Variant
    case v: java.util.Map[_, _] =>
      TableV(
        v.asInstanceOf[java.util.Map[String, Any]]
          .asScala
          .map { case (k, v) => k -> toVariant(v) }
          .toMap
      )
    case v: scala.collection.Map[_, _] =>
      TableV(
        v.asInstanceOf[scala.collection.Map[String, Any]]
          .map { case (k, v) => k -> toVariant(v) }
          .toMap
      )
    case v: java.util.List[_] => ArrayV(v.asScala.map(toVariant).toArray)
    case v: scala.collection.Seq[_] => ArrayV(v.map(toVariant).toArray)
    case _ =>
      error(s"Unsupported type encountered in toVariant: ${t.getClass}")
      throw new IllegalArgumentException(s"Unsupported type: ${t.getClass}")
  }

  def lookupStrings(
      tbl: MutableVariantTable,
      key: String,
      or: String
  ): Seq[String] = lookupStrings(tbl.toMap, key, or)

  def lookupStrings(tbl: VariantTable, key: String, or: String): Seq[String] =
    if (tbl.contains(key))
      tbl(key) match {
        case ArrayV(arr) =>
          arr.flatMap {
            case StringV(str) => Some(str)
            case _            => None
          }.toSeq
        case StringV(str) => Seq(str)
        case _            => Seq(or)
      }
    else Seq(or)

  def lookupString(tbl: MutableVariantTable, key: String, or: String): String =
    lookupString(tbl.toMap, key, or)

  def toString(t: Variant): String = t match {
    case i: IntV      => i.value.toString
    case str: StringV => str.value
    case b: BigIntV   => b.value.toString
    case _ =>
      error(s"$t not a string")
      "ERROR"
  }

  def lookupInt(tbl: MutableVariantTable, key: String, or: Int): Int =
    lookupInt(tbl.toMap, key, or)

  def lookupInt(tbl: VariantTable, key: String, or: Int): Int =
    if (tbl.contains(key)) Utils.toInt(tbl(key)) else or

  def toInt(t: Variant): Int = t match {
    case b: IntV => b.value
    case b: BigIntV =>
      if (b.value < Int.MaxValue && b.value > Int.MinValue) b.value.toInt
      else {
        error(s"$t not within int range")
        0
      }
    case _ =>
      error(s"$t not an int")
      0
  }

  def lookupDouble(tbl: MutableVariantTable, key: String, or: Double): Double =
    lookupDouble(tbl.toMap, key, or)

  def lookupDouble(tbl: VariantTable, key: String, or: Double): Double =
    if (tbl.contains(key)) Utils.toDouble(tbl(key)) else or

  def toDouble(t: Variant): Double = t match {
    case d: DoubleV => d.value
    case _ =>
      error(s"$t not a double")
      0.0
  }

  def lookupBoolean(
      tbl: MutableVariantTable,
      key: String,
      or: Boolean
  ): Boolean =
    lookupBoolean(tbl.toMap, key, or)

  def lookupBoolean(tbl: VariantTable, key: String, or: Boolean): Boolean =
    if (tbl.contains(key)) Utils.toBoolean(tbl(key)) else or

  def toBoolean(t: Variant): Boolean = t match {
    case b: BooleanV => b.value
    case _ =>
      error(s"$t not a bool")
      false
  }

  def lookupBoolean(tbl: MutableVariantTable, key: String): Array[Variant] =
    lookupArray(tbl.toMap, key)

  def lookupArray(tbl: VariantTable, key: String): Array[Variant] =
    if (tbl.contains(key)) Utils.toArray(tbl(key)) else Array()

  def toArray(t: Variant): Array[Variant] = t match {
    case arr: ArrayV => arr.value
    case _ =>
      error(s"$t not an Array")
      Array()
  }

  def lookupBigInt(tbl: MutableVariantTable, key: String, or: BigInt): BigInt =
    lookupBigInt(tbl.toMap, key, or)

  def lookupBigInt(tbl: VariantTable, key: String, or: BigInt): BigInt =
    if (tbl.contains(key)) Utils.toBigInt(tbl(key)) else or

  private def deleteRecursively(file: File): Unit = {
    if (Files.isSymbolicLink(file.toPath)) {
      file.delete()
    } else if (file.isDirectory) {
      file.listFiles.foreach(deleteRecursively)
    }

    if (file.exists && !file.delete) {
      error(s"Unable to delete ${file.getAbsolutePath}")
      throw new Exception(s"Unable to delete ${file.getAbsolutePath}")
    }
  }

  def parseBigInt(s: String): Option[BigInt] = Try {
    val ns = s.replace("_", "")

    if (ns.startsWith("0x")) Some(BigInt(ns.substring(2), 16))
    else if (ns.startsWith("0b")) Some(BigInt(ns.substring(2), 2))
    else Some(BigInt(ns))
  } match {
    case Failure(_)     => None
    case Success(value) => value
  }

  def stringToVariant(s: String): Variant = {
    val ns = s.toLowerCase
    if (ns.contains("'")) StringV(s)
    else if (ns.contains('"')) StringV(s)
    else if (ns == "true") BooleanV(true)
    else if (ns == "false") BooleanV(false)
    else
      parseBigInt(ns) match {
        case Some(v) => BigIntV(v)
        case None =>
          Try {
            ns.toDouble
          } match {
            case Failure(_)     => StringV(s)
            case Success(value) => DoubleV(value)
          }
      }
  }
}
