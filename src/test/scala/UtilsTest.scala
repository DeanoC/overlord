package gagameos

import org.scalatest.funsuite.AnyFunSuite
import java.nio.file.{Files, Paths}

class UtilsTest extends AnyFunSuite {

  test("toBigInt should convert IntV to BigInt") {
    val intVariant = IntV(42)
    assert(Utils.toBigInt(intVariant) == BigInt(42))
  }

  test("toBigInt should convert StringV to BigInt with multiplier") {
    val stringVariant = StringV("2kb")
    assert(Utils.toBigInt(stringVariant) == BigInt(2048))
  }

  test("toVariant should convert String to StringV") {
    val string = "hello"
    val variant = Utils.toVariant(string)
    assert(variant.isInstanceOf[StringV])
    assert(variant.asInstanceOf[StringV].value == "hello")
  }

  test("readYaml should parse valid YAML file") {
    val yamlContent = """key1: value1
key2: 42
key3: true"""
    val tempFile = Files.createTempFile("test", ".yaml")
    Files.write(tempFile, yamlContent.getBytes)

    val result = Utils.readYaml(tempFile)
    assert(result("key1").isInstanceOf[StringV])
    assert(result("key1").asInstanceOf[StringV].value == "value1")
    assert(result("key2").isInstanceOf[IntV])
    assert(result("key2").asInstanceOf[IntV].value == 42)
    assert(result("key3").isInstanceOf[BooleanV])
    assert(result("key3").asInstanceOf[BooleanV].value)

    Files.delete(tempFile)
  }

  test("readYaml should return empty map for invalid file") {
    val tempFile = Files.createTempFile("test", ".yaml")
    Files.write(tempFile, "invalid: [".getBytes)

    val result = Utils.readYaml(tempFile)
    assert(result.isEmpty)

    Files.delete(tempFile)
  }
}