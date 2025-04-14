package com.deanoc.overlord

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.mockito.Mockito

/** A simple test to verify that Mockito is working correctly with Scala 3
  */
class MockitoTest extends AnyFlatSpec with Matchers {

  trait TestTrait {
    def getValue: String
  }

  "Mockito" should "work correctly with Scala 3" in {
    val mockObject = Mockito.mock(classOf[TestTrait])
    Mockito.when(mockObject.getValue).thenReturn("test value")

    mockObject.getValue should be("test value")
  }
}
