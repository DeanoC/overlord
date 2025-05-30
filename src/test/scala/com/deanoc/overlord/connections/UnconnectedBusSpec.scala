package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.ChipInstance
import org.scalatestplus.mockito.MockitoSugar
import scala.language.implicitConversions
import org.mockito.Mockito.{when, doAnswer}
import org.mockito.ArgumentMatchers.any
import com.deanoc.overlord.connections.ConnectionTypesTestExtensions._

class UnconnectedBusSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with SilentLogger {

  "UnconnectedBus" should "handle silent mode correctly" in {
    val silentBus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2",
      silent = true
    )

    val regularBus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2",
      silent = false
    )

    silentBus.silent shouldBe true
    regularBus.silent shouldBe false
  }

  it should "store correct connection direction" in {
    val biDirectionalBus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.BiDirectional,
      secondFullName = "device2"
    )

    // Verify it's created with the right direction
    biDirectionalBus.direction shouldBe ConnectionDirection.BiDirectional
  }

  it should "use the specified bus protocol" in {
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

    ConnectionTypesTestExtensions.assertBusNameEquals(
      withProtocol.busProtocol,
      "axi4"
    )
    ConnectionTypesTestExtensions.assertBusNameEquals(
      defaultProtocol.busProtocol,
      "default"
    )
  }

  it should "use the specified bus names" in {
    val withNames = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      supplierBusName = "main_bus",
      consumerBusName = "mem_bus"
    )

    val defaultNames = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory"
    )

    ConnectionTypesTestExtensions.assertBusNameEquals(
      withNames.supplierBusName,
      "main_bus"
    )
    ConnectionTypesTestExtensions.assertBusNameEquals(
      withNames.consumerBusName,
      "mem_bus"
    )
    ConnectionTypesTestExtensions.assertBusNameEquals(
      defaultNames.supplierBusName,
      ""
    )
    ConnectionTypesTestExtensions.assertBusNameEquals(
      defaultNames.consumerBusName,
      ""
    )
  }

  it should "not throw exceptions with empty data" in {
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory"
    )

    // Test with empty data - methods should not throw exceptions
    withSilentLogs {
      noException should be thrownBy {
        bus.preConnect(Seq.empty[ChipInstance])
      }

      noException should be thrownBy {
        bus.finaliseBuses(Seq.empty[ChipInstance])
      }

      val connections = bus.connect(Seq.empty[ChipInstance])
      connections shouldBe empty
    }
  }

  it should "properly handle the connection direction" in {
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

    firstToSecond.direction shouldBe ConnectionDirection.FirstToSecond
    secondToFirst.direction shouldBe ConnectionDirection.SecondToFirst
  }

}
