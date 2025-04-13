package com.deanoc.overlord

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord.connections.ConnectionDirection

class ConnectionDirectionSpec extends AnyFlatSpec with Matchers {

  "ConnectionDirection" should "provide correct string representation" in {
    ConnectionDirection.FirstToSecond.toString shouldBe "first to second"
    ConnectionDirection.SecondToFirst.toString shouldBe "second to first"
    ConnectionDirection.BiDirectional.toString shouldBe "bi direction"
  }

  it should "flip directions correctly" in {
    ConnectionDirection.FirstToSecond.flip shouldBe ConnectionDirection.SecondToFirst
    ConnectionDirection.SecondToFirst.flip shouldBe ConnectionDirection.FirstToSecond
    ConnectionDirection.BiDirectional.flip shouldBe ConnectionDirection.BiDirectional // BiDirectional remains unchanged
  }

  it should "allow pattern matching" in {
    def checkDirection(dir: ConnectionDirection): String = dir match {
      case ConnectionDirection.FirstToSecond => "Forward"
      case ConnectionDirection.SecondToFirst => "Backward"
      case ConnectionDirection.BiDirectional   => "Both"
    }

    checkDirection(ConnectionDirection.FirstToSecond) shouldBe "Forward"
    checkDirection(ConnectionDirection.SecondToFirst) shouldBe "Backward"
    checkDirection(ConnectionDirection.BiDirectional) shouldBe "Both"
  }
}