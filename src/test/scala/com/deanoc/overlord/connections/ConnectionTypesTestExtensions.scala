package com.deanoc.overlord.connections

import scala.language.implicitConversions
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord.connections.ConnectionTypes._

/** Extension methods for ConnectionTypes to support testing with ScalaTest.
  *
  * This object provides extension methods that enhance the testability of
  * connection-related types when using ScalaTest. These extensions are only
  * available in the test scope.
  */
object ConnectionTypesTestExtensions {

  /** Helper method to assert that a BusName equals a string value */
  def assertBusNameEquals(busName: BusName, expected: String): Unit = {
    assert(
      busName.value == expected,
      s"BusName was '${busName.value}' but expected '$expected'"
    )
  }

  /** Implicit conversion from BusName to String for testing */
  implicit def busNameToString(name: BusName): String = name.asString
}
