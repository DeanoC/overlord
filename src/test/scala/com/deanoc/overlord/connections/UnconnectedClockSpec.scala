package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.ChipInstance
import org.scalatestplus.mockito.MockitoSugar

class UnconnectedClockSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {

  "UnconnectedClock" should "store connection properties correctly" in {
    val clock = UnconnectedClock(
      firstFullName = "clk_source",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "clk_consumer"
    )
    
    clock.firstFullName shouldBe "clk_source"
    clock.direction shouldBe ConnectionDirection.FirstToSecond
    clock.secondFullName shouldBe "clk_consumer"
  }
  
  it should "support all connection directions" in {
    val firstToSecond = UnconnectedClock(
      firstFullName = "clk_source",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "clk_consumer"
    )
    
    val secondToFirst = UnconnectedClock(
      firstFullName = "clk_consumer",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "clk_source"
    )
    
    val biDirectional = UnconnectedClock(
      firstFullName = "clk_a",
      direction = ConnectionDirection.BiDirectional,
      secondFullName = "clk_b"
    )
    
    firstToSecond.direction shouldBe ConnectionDirection.FirstToSecond
    secondToFirst.direction shouldBe ConnectionDirection.SecondToFirst
    biDirectional.direction shouldBe ConnectionDirection.BiDirectional
  }
  
  it should "not throw exceptions with empty data" in {
    val clock = UnconnectedClock(
      firstFullName = "clk_source",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "clk_consumer"
    )
    
    // Test with empty data - methods should not throw exceptions
    withSilentLogs {
      noException should be thrownBy {
        clock.preConnect(Seq.empty[ChipInstance])
      }
      
      noException should be thrownBy {
        clock.finaliseBuses(Seq.empty[ChipInstance])
      }
      
      val connections = clock.connect(Seq.empty[ChipInstance])
      connections shouldBe empty
      
      val constants = clock.collectConstants(Seq.empty)
      constants shouldBe empty
    }
  }
}