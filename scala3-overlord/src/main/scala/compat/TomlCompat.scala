package compat

import scala.util.{Try, Success, Failure}
import scala.collection.mutable
import scala.jdk.CollectionConverters.*
import com.electronwill.nightconfig.core.Config
import com.electronwill.nightconfig.toml.TomlParser

/**
 * Compatibility layer for TOML parsing.
 * This provides a bridge between the original tech.sparse.toml API used in Scala 2
 * and the night-config library used for Scala 3.
 */
object TomlCompat:
  // Mimicking the structure of the original toml.Value
  sealed trait Value
  object Value:
    case class Str(v: String) extends Value
    case class Bool(v: Boolean) extends Value
    case class Real(v: Double) extends Value
    case class Num(v: BigInt) extends Value
    case class Tbl(v: Map[String, Value]) extends Value
    case class Arr(v: Seq[Value]) extends Value

  // Main TOML processing class
  object Toml:
    def parse(content: String): Either[String, ValuesResult] =
      try
        val parser = new TomlParser()
        val config = parser.parse(content)
        val values = convertConfig(config)
        Right(ValuesResult(values))
      catch
        case e: Exception => Left(e.getMessage)

  // Result class similar to the original toml library
  case class ValuesResult(values: Map[String, Value])
  
  // Helper method to convert night-config Config objects to our Value type
  private def convertConfig(config: Config): Map[String, Value] =
    val result = mutable.Map[String, Value]()
    
    // Process all entries in the config
    for key <- config.entrySet().asScala.map(_.getKey) do
      val value = convertValue(config.get[Object](key))
      result.put(key, value)
    
    result.toMap
  
  // Convert a night-config value to our Value type
  private def convertValue(value: Object): Value = value match
    case s: String => Value.Str(s)
    case b: java.lang.Boolean => Value.Bool(b)
    case d: java.lang.Double => Value.Real(d)
    case l: java.lang.Long => Value.Num(BigInt(l))
    case i: java.lang.Integer => Value.Num(BigInt(i.toLong))
    case c: Config => Value.Tbl(convertConfig(c))
    case list: java.util.List[?] => 
      val values = list.asScala.map(v => convertValue(v.asInstanceOf[Object])).toSeq
      Value.Arr(values)
    case null => Value.Str("null")
    case other => Value.Str(other.toString)