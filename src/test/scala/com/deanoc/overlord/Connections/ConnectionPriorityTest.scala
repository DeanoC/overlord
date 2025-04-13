package com.deanoc.overlord.Connections

import org.scalatest.funsuite.AnyFunSuite
import org.scalatest.matchers.should.Matchers

class ConnectionPriorityTest extends AnyFunSuite with Matchers {
  
  test("ConnectionPriority comparison operators work correctly") {
    ConnectionPriority.High > ConnectionPriority.Medium shouldBe true
    ConnectionPriority.Medium > ConnectionPriority.Low shouldBe true
    ConnectionPriority.Low < ConnectionPriority.Medium shouldBe true
    ConnectionPriority.Medium < ConnectionPriority.High shouldBe true
    ConnectionPriority.High >= ConnectionPriority.Medium shouldBe true
    ConnectionPriority.Medium <= ConnectionPriority.Medium shouldBe true
  }
  
  test("ConnectionPriority.fromString correctly parses valid strings") {
    ConnectionPriority.fromString("high") shouldBe Some(ConnectionPriority.High)
    ConnectionPriority.fromString("High") shouldBe Some(ConnectionPriority.High)
    ConnectionPriority.fromString("medium") shouldBe Some(ConnectionPriority.Medium)
    ConnectionPriority.fromString("low") shouldBe Some(ConnectionPriority.Low)
  }
  
  test("ConnectionPriority.fromString returns None for invalid strings") {
    ConnectionPriority.fromString("invalid") shouldBe None
    ConnectionPriority.fromString("") shouldBe None
  }
  
  test("ConnectionPriority.default returns Medium") {
    ConnectionPriority.default shouldBe ConnectionPriority.Medium
  }
}
