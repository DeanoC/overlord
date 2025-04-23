package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.hardware.Port
import com.deanoc.overlord.config.BitsDesc
import com.deanoc.overlord.config.WireDirection
import com.deanoc.overlord.instances.{HardwareInstance, InstanceTrait}
import com.deanoc.overlord.interfaces.{BusLike, SupplierBusLike}
import org.mockito.Mockito
import com.deanoc.overlord.connections.ConnectedExtensions._
import scala.language.implicitConversions

/** Test suite for the ConnectedBus class. This test suite focuses on the
  * behavior of bus connections to establish a baseline before refactoring.
  */
class ConnectedBusSpec extends AnyFlatSpec with Matchers with SilentLogger {

  // Create mock objects
  val mockSupplierBusLike = Mockito.mock(classOf[SupplierBusLike])

  // Test basic ConnectedBus properties
  "ConnectedBus" should "store and retrieve basic properties correctly" in {
    // Create mock instances
    val srcInstance = Mockito.mock(classOf[HardwareInstance])
    val destInstance = Mockito.mock(classOf[HardwareInstance])

    Mockito.when(srcInstance.name).thenReturn("source")
    Mockito.when(destInstance.name).thenReturn("destination")

    // Create InstanceLocs
    val srcLoc = InstanceLoc(srcInstance, None, "top.source")
    val destLoc = InstanceLoc(destInstance, None, "top.destination")

    // Create ConnectedBus
    val busConn = ConnectedBus(
      ConnectionPriority.Explicit,
      srcLoc,
      ConnectionDirection.FirstToSecond,
      destLoc,
      mockSupplierBusLike,
      destInstance
    )

    // Test basic properties
    busConn.connectionPriority shouldBe ConnectionPriority.Explicit
    busConn.first.get shouldBe srcLoc
    busConn.second.get shouldBe destLoc
    (busConn.direction == ConnectionDirection.FirstToSecond) shouldBe true
    busConn.firstFullName shouldBe "top.source"
    busConn.secondFullName shouldBe "top.destination"
    busConn.firstLastName shouldBe "source"
    busConn.secondLastName shouldBe "destination"

    // Verify that bus and other properties have correct references
    busConn.bus shouldBe mockSupplierBusLike
    busConn.other shouldBe destInstance
  }

  it should "identify connection between instances correctly" in {
    // Create mock instances
    val srcInstance = Mockito.mock(classOf[HardwareInstance])
    val destInstance = Mockito.mock(classOf[HardwareInstance])
    val otherInstance = Mockito.mock(classOf[HardwareInstance])

    Mockito.when(srcInstance.name).thenReturn("source")
    Mockito.when(destInstance.name).thenReturn("destination")
    Mockito.when(otherInstance.name).thenReturn("other")

    // Create InstanceLocs
    val srcLoc = InstanceLoc(srcInstance, None, "top.source")
    val destLoc = InstanceLoc(destInstance, None, "top.destination")

    // Create ConnectedBus with FirstToSecond direction
    val forwardBus = ConnectedBus(
      ConnectionPriority.Explicit,
      srcLoc,
      ConnectionDirection.FirstToSecond,
      destLoc,
      mockSupplierBusLike,
      destInstance
    )

    // Create ConnectedBus with bidirectional connection
    val biBus = ConnectedBus(
      ConnectionPriority.Explicit,
      srcLoc,
      ConnectionDirection.BiDirectional,
      destLoc,
      mockSupplierBusLike,
      destInstance
    )

    // Test instance connection checks
    forwardBus.connectedTo(srcInstance) shouldBe true
    forwardBus.connectedTo(destInstance) shouldBe true
    forwardBus.connectedTo(otherInstance) shouldBe false

    // Test directional connections
    forwardBus.connectedBetween(
      srcInstance,
      destInstance
    ) shouldBe true // default is bidirectional
    forwardBus.connectedBetween(
      destInstance,
      srcInstance
    ) shouldBe true // default is bidirectional

    // Test with explicit directions
    forwardBus.connectedBetween(
      srcInstance,
      destInstance,
      ConnectionDirection.FirstToSecond
    ) shouldBe true
    forwardBus.connectedBetween(
      destInstance,
      srcInstance,
      ConnectionDirection.FirstToSecond
    ) shouldBe false
    forwardBus.connectedBetween(
      srcInstance,
      destInstance,
      ConnectionDirection.SecondToFirst
    ) shouldBe false
    forwardBus.connectedBetween(
      destInstance,
      srcInstance,
      ConnectionDirection.SecondToFirst
    ) shouldBe true
    forwardBus.connectedBetween(
      srcInstance,
      destInstance,
      ConnectionDirection.BiDirectional
    ) shouldBe true
    forwardBus.connectedBetween(
      destInstance,
      srcInstance,
      ConnectionDirection.BiDirectional
    ) shouldBe true

    // Test bidirectional bus connections
    biBus.connectedBetween(
      srcInstance,
      destInstance,
      ConnectionDirection.FirstToSecond
    ) shouldBe true
    biBus.connectedBetween(
      destInstance,
      srcInstance,
      ConnectionDirection.FirstToSecond
    ) shouldBe false
    biBus.connectedBetween(
      srcInstance,
      destInstance,
      ConnectionDirection.SecondToFirst
    ) shouldBe false
    biBus.connectedBetween(
      destInstance,
      srcInstance,
      ConnectionDirection.SecondToFirst
    ) shouldBe true
    biBus.connectedBetween(
      srcInstance,
      destInstance,
      ConnectionDirection.BiDirectional
    ) shouldBe true
    biBus.connectedBetween(
      destInstance,
      srcInstance,
      ConnectionDirection.BiDirectional
    ) shouldBe true
  }

  it should "correctly identify chip-to-chip connections" in {
    // Create mock instances
    val srcInstance = Mockito.mock(classOf[HardwareInstance])
    val destInstance = Mockito.mock(classOf[HardwareInstance])

    Mockito.when(srcInstance.name).thenReturn("source")
    Mockito.when(destInstance.name).thenReturn("destination")

    // Create InstanceLocs
    val srcLoc = InstanceLoc(srcInstance, None, "top.source")
    val destLoc = InstanceLoc(destInstance, None, "top.destination")

    // Create ConnectedBus
    val busConn = ConnectedBus(
      ConnectionPriority.Explicit,
      srcLoc,
      ConnectionDirection.FirstToSecond,
      destLoc,
      mockSupplierBusLike,
      destInstance
    )

    // Test connection type identification
    busConn.isChipToChip shouldBe true
    busConn.isPinToChip shouldBe false
    busConn.isChipToPin shouldBe false
    busConn.isClock shouldBe false
  }

  it should "handle name operations correctly" in {
    // Create mock instances
    val srcInstance = Mockito.mock(classOf[HardwareInstance])
    val destInstance = Mockito.mock(classOf[HardwareInstance])

    Mockito.when(srcInstance.name).thenReturn("source")
    Mockito.when(destInstance.name).thenReturn("destination")

    // Create InstanceLocs with hierarchical names
    val srcLoc = InstanceLoc(srcInstance, None, "soc.cpu.core")
    val destLoc = InstanceLoc(destInstance, None, "soc.memory.controller")

    // Create ConnectedBus
    val busConn = ConnectedBus(
      ConnectionPriority.Explicit,
      srcLoc,
      ConnectionDirection.FirstToSecond,
      destLoc,
      mockSupplierBusLike,
      destInstance
    )

    // Test name operations
    busConn.firstFullName shouldBe "soc.cpu.core"
    busConn.secondFullName shouldBe "soc.memory.controller"
    busConn.firstLastName shouldBe "core"
    busConn.secondLastName shouldBe "controller"
    busConn.firstHeadName shouldBe "soc"
    busConn.secondHeadName shouldBe "soc"
  }
}
