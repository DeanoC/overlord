package com.deanoc.overlord.connections

import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord.connections.ConnectionTypes._
import scala.language.implicitConversions

/** Provides utility methods and extensions for testing with opaque types.
  */
object TestUtils extends Matchers {
  // Import implicit conversions from the ConnectionTypes object
  import ConnectionTypes.given

  /** Extension methods for BusName to make testing easier
    */
  implicit class BusNameTestOps(busName: BusName) {
    def shouldEqual(expected: String): Unit = {
      busName.toString shouldBe expected
    }
  }

  /** Extension methods for ConnectionName to make testing easier
    */
  implicit class ConnectionNameTestOps(name: ConnectionName) {
    def shouldEqual(expected: String): Unit = {
      name.toString shouldBe expected
    }
  }

  /** Extension methods for PortName to make testing easier
    */
  implicit class PortNameTestOps(name: PortName) {
    def shouldEqual(expected: String): Unit = {
      name.toString shouldBe expected
    }
  }

  /** Extension methods for InstanceName to make testing easier
    */
  implicit class InstanceNameTestOps(name: InstanceName) {
    def shouldEqual(expected: String): Unit = {
      name.toString shouldBe expected
    }
  }
}
