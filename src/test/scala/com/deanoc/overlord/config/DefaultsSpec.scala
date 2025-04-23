package com.deanoc.overlord.config

import io.circe.yaml.parser
import io.circe.Decoder
import io.circe.HCursor
import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

case class SimpleConfig(
  foo: String,
  bar: Int,
  baz: Boolean
)

object SimpleConfig {
  import com.deanoc.overlord.config.DefaultsAwareDecoders._
  implicit val decoder: Decoder[SimpleConfig] = Decoder.instance { c =>
    for {
      foo <- c.downField("foo").as[String]
      bar <- c.downField("bar").as[Int]
      baz <- c.downField("baz").as[Boolean]
    } yield SimpleConfig(foo, bar, baz)
  }
}

class DefaultsSpec extends AnyFlatSpec with Matchers {

  "Defaults" should "give precedence to keys pushed earlier in the stack" in {
    // Clear the stack to ensure a clean test environment
    Defaults.clear()

    // Push two maps onto the stack
    Defaults.push(Map("Bob" -> true))
    Defaults.push(Map("Bob" -> false))

    // Flatten the stack and verify the precedence
    Defaults.flatten("Bob") shouldEqual true
  }

  it should "return an empty map when the stack is empty" in {
    // Clear the stack
    Defaults.clear()

    // Verify flatten returns an empty map
    Defaults.flatten shouldBe empty
  }

  it should "maintain precedence when popping from the stack" in {
    // Clear the stack to ensure a clean test environment
    Defaults.clear()

    // Push three maps onto the stack
    Defaults.push(Map("Alice" -> true))
    Defaults.push(Map("Alice" -> false))
    Defaults.push(Map("Alice" -> "override"))

    // Pop the top map
    Defaults.pop()

    // Flatten the stack and verify the precedence
    Defaults.flatten("Alice") shouldEqual true

    // Pop again
    Defaults.pop()

    // Flatten the stack and verify the precedence
    Defaults.flatten("Alice") shouldEqual true
  }

  it should "handle popping until the stack is empty" in {
    // Clear the stack
    Defaults.clear()

    // Push a map onto the stack
    Defaults.push(Map("Charlie" -> 42))

    // Pop the map
    Defaults.pop()

    // Verify the stack is empty
    Defaults.isEmpty shouldBe true

    // Flatten should return an empty map
    Defaults.flatten shouldBe empty
  }
  
  it should "exclude keys in the excludes list when flattening" in {
    // Clear the stack and excludes to ensure a clean test environment
    Defaults.clear()
    
    // Push maps onto the stack
    Defaults.push(Map("Alice" -> true, "Bob" -> false, "Charlie" -> 42))
    
    // Add keys to the excludes list
    Defaults.addExclude("Bob")
    
    // Flatten the stack and verify excluded keys are not returned
    val flattened = Defaults.flatten
    flattened should contain ("Alice" -> true)
    flattened should contain ("Charlie" -> 42)
    flattened should not contain key ("Bob")
    
    // Verify direct key access also respects excludes
    Defaults.flatten("Alice") shouldEqual true
    // For the excluded key, we should get None when trying to access it
    Defaults.flatten.get("Bob") shouldBe None
  }
  
  it should "allow removing keys from the excludes list" in {
    // Clear the stack and excludes to ensure a clean test environment
    Defaults.clear()
    
    // Push maps onto the stack
    Defaults.push(Map("Alice" -> true, "Bob" -> false))
    
    // Add and then remove a key from the excludes list
    Defaults.addExclude("Bob")
    Defaults.removeExclude("Bob")
    
    // Verify the key is no longer excluded
    val flattened = Defaults.flatten
    flattened should contain ("Alice" -> true)
    flattened should contain ("Bob" -> false)
  }
  
  it should "clear excludes list when clear() is called" in {
    // Clear the stack to ensure a clean test environment
    Defaults.clear()
    
    // Push a map onto the stack
    Defaults.push(Map("Alice" -> true, "Bob" -> false))
    
    // Add a key to the excludes list
    Defaults.addExclude("Bob")
    
    // Clear everything
    Defaults.clear()
    
    // Push a new map
    Defaults.push(Map("Bob" -> true))
    
    // Verify the excludes list was cleared
    Defaults.flatten should contain ("Bob" -> true)
  }
  
  it should "allow clearing only the excludes list" in {
    // Clear the stack to ensure a clean test environment
    Defaults.clear()
    
    // Push a map onto the stack
    Defaults.push(Map("Alice" -> true, "Bob" -> false))
    
    // Add a key to the excludes list
    Defaults.addExclude("Bob")
    
    // Clear only the excludes list
    Defaults.clearExcludes()
    
    // Verify the excludes list was cleared but the stack remains
    val flattened = Defaults.flatten
    flattened should contain ("Alice" -> true)
    flattened should contain ("Bob" -> false)
  }
  
  it should "reuse cached results when no changes occur" in {
    Defaults.clear()
    
    Defaults.push(Map("Alice" -> true, "Bob" -> false))
    
    // First call should compute and cache
    val result1 = Defaults.flatten
    // Second call should use cache
    val result2 = Defaults.flatten
    
    result1 should be theSameInstanceAs result2
  }

  it should "invalidate cache when stack changes" in {
    Defaults.clear()
    
    Defaults.push(Map("Alice" -> true))
    val result1 = Defaults.flatten
    
    // Modifying stack should invalidate cache
    Defaults.push(Map("Bob" -> false))
    val result2 = Defaults.flatten
    
    result1 should not be theSameInstanceAs(result2)
  }

  it should "invalidate cache when excludes list changes" in {
    Defaults.clear()
    
    Defaults.push(Map("Alice" -> true, "Bob" -> false))
    val result1 = Defaults.flatten
    
    // Modifying excludes should invalidate cache
    Defaults.addExclude("Bob")
    val result2 = Defaults.flatten
    
    result1 should not be theSameInstanceAs(result2)
    result2 should not contain key("Bob")
  }

  it should "maintain cache validity appropriately" in {
    Defaults.clear()
    Defaults.cacheValid shouldBe false // Should start invalid

    Defaults.push(Map("test" -> true))
    Defaults.cacheValid shouldBe false // Should be invalid after push

    Defaults.flatten // Should compute and cache
    Defaults.cacheValid shouldBe true // Should now be valid

    // Reading shouldn't invalidate
    Defaults.flatten
    Defaults.peek()
    Defaults.isEmpty
    Defaults.cacheValid shouldBe true

    // Modifications should invalidate
    Defaults.push(Map("new" -> false))
    Defaults.cacheValid shouldBe false

    Defaults.flatten // Recompute
    Defaults.cacheValid shouldBe true

    Defaults.pop()
    Defaults.cacheValid shouldBe false // Pop should invalidate

    Defaults.flatten // Recompute
    Defaults.cacheValid shouldBe true

    Defaults.addExclude("test")
    Defaults.cacheValid shouldBe false // Adding exclude should invalidate

    Defaults.flatten // Recompute
    Defaults.cacheValid shouldBe true

    Defaults.removeExclude("test")
    Defaults.cacheValid shouldBe false // Removing exclude should invalidate

    Defaults.flatten // Recompute
    Defaults.cacheValid shouldBe true

    Defaults.clearExcludes()
    Defaults.cacheValid shouldBe false // Clearing excludes should invalidate

    Defaults.flatten // Recompute
    Defaults.cacheValid shouldBe true

    Defaults.clear()
    Defaults.cacheValid shouldBe false // Clear should invalidate
  }

  "Defaults YAML decoding" should "override value-type keys from the defaults stack" in {
    Defaults.clear()
    // Push defaults that should override YAML
    Defaults.push(Map("foo" -> "from-defaults", "bar" -> 123, "baz" -> true))

    val yaml =
      """
      |foo: original
      |bar: 42
      |baz: false
      """.stripMargin

    val doc = parser.parse(yaml).toOption.get
    val decoded = SimpleConfig.decoder.decodeJson(doc)
    decoded shouldBe Right(SimpleConfig("from-defaults", 123, true))
  }

  it should "fall back to YAML values if not in defaults stack" in {
    Defaults.clear()
    // Only override 'foo'
    Defaults.push(Map("foo" -> "from-defaults"))

    val yaml =
      """
      |foo: original
      |bar: 42
      |baz: false
      """.stripMargin

    val doc = parser.parse(yaml).toOption.get
    val decoded = SimpleConfig.decoder.decodeJson(doc)
    decoded shouldBe Right(SimpleConfig("from-defaults", 42, false))
  }

  it should "decode YAML values if defaults stack is empty" in {
    Defaults.clear()

    val yaml =
      """
      |foo: original
      |bar: 42
      |baz: false
      """.stripMargin

    val doc = parser.parse(yaml).toOption.get
    val decoded = SimpleConfig.decoder.decodeJson(doc)
    decoded shouldBe Right(SimpleConfig("original", 42, false))
  }

  it should "respect excludes when decoding" in {
    Defaults.clear()
    Defaults.push(Map("foo" -> "from-defaults", "bar" -> 123, "baz" -> true))
    Defaults.addExclude("bar")

    val yaml =
      """
      |foo: original
      |bar: 42
      |baz: false
      """.stripMargin

    val doc = parser.parse(yaml).toOption.get
    val decoded = SimpleConfig.decoder.decodeJson(doc)
    // 'bar' should come from YAML, others from defaults
    decoded shouldBe Right(SimpleConfig("from-defaults", 42, true))
  }
}
