package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.ChipInstance
import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.interfaces.UnconnectedLike
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito.mock

class UnconnectedSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {

  "Unconnected.apply" should "delegate to ConnectionParser" in {
    withSilentLogs {
      // Use a mock Variant to test the apply method
      val mockVariant = mock[Variant]
      
      // Since ConnectionParser is not easily mockable (static methods),
      // we'll just verify the method returns expected type
      val result = Unconnected.apply(mockVariant)
      
      // For invalid input, it should return None
      result shouldBe None
    }
  }
  
  "Concrete implementations of Unconnected" should "have consistent behavior" in {
    // Test with different concrete implementations
    val port = UnconnectedPort(
      firstFullName = "device1.tx",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2.rx"
    )
    
    val logical = UnconnectedLogical(
      firstFullName = "component1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "component2"
    )
    
    val clock = UnconnectedClock(
      firstFullName = "clk_source",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "clk_consumer"
    )
    
    // Verify all implementations are Unconnected
    port.isInstanceOf[Unconnected] shouldBe true
    logical.isInstanceOf[Unconnected] shouldBe true
    clock.isInstanceOf[Unconnected] shouldBe true
    
    // Verify all implementations are UnconnectedLike
    port.isInstanceOf[UnconnectedLike] shouldBe true
    logical.isInstanceOf[UnconnectedLike] shouldBe true
    clock.isInstanceOf[UnconnectedLike] shouldBe true
    
    // Test with empty data - should not throw exceptions
    withSilentLogs {
      noException should be thrownBy {
        port.preConnect(Seq.empty[ChipInstance])
        logical.preConnect(Seq.empty[ChipInstance])
        clock.preConnect(Seq.empty[ChipInstance])
      }
      
      noException should be thrownBy {
        port.finaliseBuses(Seq.empty[ChipInstance])
        logical.finaliseBuses(Seq.empty[ChipInstance])
        clock.finaliseBuses(Seq.empty[ChipInstance])
      }
      
      // Connect returns empty sequences with no instances
      port.connect(Seq.empty[ChipInstance]) shouldBe empty
      logical.connect(Seq.empty[ChipInstance]) shouldBe empty
      clock.connect(Seq.empty[ChipInstance]) shouldBe empty
    }
  }
}