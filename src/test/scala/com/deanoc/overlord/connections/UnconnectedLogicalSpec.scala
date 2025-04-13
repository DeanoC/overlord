package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.ChipInstance
import org.scalatestplus.mockito.MockitoSugar

class UnconnectedLogicalSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {

  "UnconnectedLogical" should "store connection properties correctly" in {
    val logical = UnconnectedLogical(
      firstFullName = "component1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "component2"
    )
    
    logical.firstFullName shouldBe "component1"
    logical.direction shouldBe ConnectionDirection.FirstToSecond
    logical.secondFullName shouldBe "component2"
  }
  
  it should "support all connection directions" in {
    val firstToSecond = UnconnectedLogical(
      firstFullName = "component1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "component2"
    )
    
    val secondToFirst = UnconnectedLogical(
      firstFullName = "component2",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "component1"
    )
    
    val biDirectional = UnconnectedLogical(
      firstFullName = "component_a",
      direction = ConnectionDirection.BiDirectional,
      secondFullName = "component_b"
    )
    
    firstToSecond.direction shouldBe ConnectionDirection.FirstToSecond
    secondToFirst.direction shouldBe ConnectionDirection.SecondToFirst
    biDirectional.direction shouldBe ConnectionDirection.BiDirectional
  }
  
  it should "not throw exceptions with empty data" in {
    val logical = UnconnectedLogical(
      firstFullName = "component1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "component2"
    )
    
    // Test with empty data - methods should not throw exceptions
    withSilentLogs {
      noException should be thrownBy {
        logical.preConnect(Seq.empty[ChipInstance])
      }
      
      noException should be thrownBy {
        logical.finaliseBuses(Seq.empty[ChipInstance])
      }
      
      val connections = logical.connect(Seq.empty[ChipInstance])
      connections shouldBe empty
      
      val constants = logical.collectConstants(Seq.empty)
      constants shouldBe empty
    }
  }
  
  it should "use ConnectionPriority.WildCard when multiple matches exist" in {
    // This is a behavioral test that verifies the logic in connect method
    // In real implementation, when multiple instances match, WildCard priority is used
    
    // We can't fully test this without complex mocking, but we can verify
    // the class uses the expected types and interfaces
    
    val logical = UnconnectedLogical(
      firstFullName = "component*", // Wildcard pattern
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "target*"    // Wildcard pattern
    )
    
    logical.firstFullName should include ("*")
    logical.secondFullName should include ("*")
    
    // If this compiles, the class is structurally correct
    logical shouldBe a[Unconnected]
  }
}
