package com.deanoc.overlord.connections

import org.scalatest.funsuite.AnyFunSuite
import org.scalatest.matchers.should.Matchers

class ConnectionPriorityTest extends AnyFunSuite with Matchers {
  
  test("ConnectionPriority comparison operators work correctly") {
    ConnectionPriority.Explicit > ConnectionPriority.Group shouldBe true
    ConnectionPriority.Group > ConnectionPriority.WildCard shouldBe true
    ConnectionPriority.WildCard < ConnectionPriority.Group shouldBe true
    ConnectionPriority.Group < ConnectionPriority.Explicit shouldBe true
    ConnectionPriority.Explicit >= ConnectionPriority.Group shouldBe true
    ConnectionPriority.Group <= ConnectionPriority.Group shouldBe true
  }
  
  test("ConnectionPriority.fromString correctly parses valid strings") {
    ConnectionPriority.fromString("explicit") shouldBe Some(ConnectionPriority.Explicit)
    ConnectionPriority.fromString("Explicit") shouldBe Some(ConnectionPriority.Explicit)
    ConnectionPriority.fromString("group") shouldBe Some(ConnectionPriority.Group)
    ConnectionPriority.fromString("wildcard") shouldBe Some(ConnectionPriority.WildCard)
  }
  
  test("ConnectionPriority.fromString returns None for invalid strings") {
    ConnectionPriority.fromString("invalid") shouldBe None
    ConnectionPriority.fromString("") shouldBe None
  }
  
  test("ConnectionPriority.default returns WildCard") {
    ConnectionPriority.default shouldBe ConnectionPriority.WildCard
  }
  
  test("ConnectionPriority enum values are correctly defined") {
    ConnectionPriority.Explicit.toInt shouldBe 4
    ConnectionPriority.Group.toInt shouldBe 3
    ConnectionPriority.WildCard.toInt shouldBe 2
    ConnectionPriority.Fake.toInt shouldBe 1
  }
}
