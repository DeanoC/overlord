package com.deanoc.overlord

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._
import scala.reflect.ClassTag
import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.utils.SilentLogger

/**
 * A test to demonstrate the Mockito issue with Scala's ClassTag and type parameters
 */
class MockitoFixTest extends AnyFlatSpec with Matchers with MockitoSugar with Logging with SilentLogger {
  withSilentLogs{
    trait TestInterface {
      def getValue: String
    }
    trait OtherTestInterface {
      def getValue: String
    }

    trait QueryInterfaceTest {
      def hasInterface[T](implicit tag: ClassTag[T]): Boolean = {
        // Add this line for debugging
        trace(s"hasInterface called with ClassTag: ${tag}")
        getInterface[T](tag).nonEmpty
      }

      def getInterface[T](implicit tag: ClassTag[T]): Option[T] = None
    }

    "Mockito" should "work with Scala's ClassTag and type parameters using doAnswer" in {
      val mockObject = mock[QueryInterfaceTest]
      val testInterface = mock[TestInterface]
      when(testInterface.getValue).thenReturn("test value")
      
      // Use more specific matchers for the ClassTag parameter
      val testInterfaceTag = implicitly[ClassTag[TestInterface]]
      
      // Print the actual ClassTag values for debugging
      trace(s"TestInterface ClassTag: ${testInterfaceTag}")
      
      // First set a default behavior for any class tag - use any() without type parameters
      doAnswer(invocation => {
        val classTag = invocation.getArgument[ClassTag[_]](0)
        classTag == testInterfaceTag // Return true only if it matches TestInterface
      }).when(mockObject).hasInterface(any())
      
      // Set up the getInterface behavior
      doAnswer(invocation => {
        val classTag = invocation.getArgument[ClassTag[_]](0)
        if(classTag == testInterfaceTag) Some(testInterface) else None
      }).when(mockObject).getInterface(any())
      
      // Test the mocked methods
      mockObject.hasInterface[TestInterface] should be(true)
      mockObject.hasInterface[OtherTestInterface] should be(false)
      // Let's also test with another random class type
      mockObject.hasInterface[String] should be(false)
      mockObject.getInterface[TestInterface].get.getValue should be("test value")
    }
  }
}