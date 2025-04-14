package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Logging, SilentLogger}
import com.deanoc.overlord.instances.ChipInstance
import scala.language.implicitConversions

/**
 * Additional tests for UnconnectedBus focused on edge cases and refactored code
 */
class UnconnectedBusAdditionalSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with Logging
    with SilentLogger {

  // Simplified tests that don't rely on complex mocking

  "UnconnectedBus.preConnect" should "handle an empty instance list gracefully" in {
    val bus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2"
    )

    withSilentLogs {
      noException should be thrownBy {
        bus.preConnect(Seq.empty)
      }
    }
  }

  "UnconnectedBus.connect" should "return empty when instances are missing" in {
    val bus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2"
    )

    withSilentLogs {
      val result = bus.connect(Seq.empty)
      result shouldBe empty
    }
  }

  it should "handle silent mode correctly" in {
    val silentBus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2",
      silent = true
    )

    withSilentLogs {
      val result = silentBus.connect(Seq.empty)
      result shouldBe empty
    }
  }

  it should "respect the connection direction" in {
    val firstToSecond = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory"
    )

    val secondToFirst = UnconnectedBus(
      firstFullName = "memory",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "cpu"
    )

    val bidirectional = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.BiDirectional,
      secondFullName = "device2"
    )

    firstToSecond.direction shouldBe ConnectionDirection.FirstToSecond
    secondToFirst.direction shouldBe ConnectionDirection.SecondToFirst
    bidirectional.direction shouldBe ConnectionDirection.BiDirectional
  }

  it should "use the correct bus protocol" in {
    val withProtocol = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "axi4"
    )

    val defaultProtocol = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory"
    )

    withProtocol.busProtocol.toString shouldBe "axi4"
    defaultProtocol.busProtocol.toString shouldBe "default"
  }
}
