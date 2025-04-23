package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.HardwareInstance
import org.scalatestplus.mockito.MockitoSugar

class UnconnectedPortGroupSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {

  "UnconnectedPortGroup" should "store connection properties correctly" in {
    val excludes = Seq("clock", "reset")
    val portGroup = UnconnectedPortGroup(
      firstFullName = "uart0",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "uart1",
      first_prefix = "tx_",
      second_prefix = "rx_",
      excludes = excludes
    )
    
    portGroup.firstFullName shouldBe "uart0"
    portGroup.direction shouldBe ConnectionDirection.FirstToSecond
    portGroup.secondFullName shouldBe "uart1"
    portGroup.first_prefix shouldBe "tx_"
    portGroup.second_prefix shouldBe "rx_"
    portGroup.excludes should contain theSameElementsAs excludes
  }
  
  it should "support all connection directions" in {
    val firstToSecond = UnconnectedPortGroup(
      firstFullName = "uart0",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "uart1",
      first_prefix = "tx_",
      second_prefix = "rx_",
      excludes = Seq()
    )
    
    val secondToFirst = UnconnectedPortGroup(
      firstFullName = "uart1",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "uart0",
      first_prefix = "rx_",
      second_prefix = "tx_",
      excludes = Seq()
    )
    
    val biDirectional = UnconnectedPortGroup(
      firstFullName = "i2c0",
      direction = ConnectionDirection.BiDirectional,
      secondFullName = "i2c1",
      first_prefix = "io_",
      second_prefix = "io_",
      excludes = Seq()
    )
    
    firstToSecond.direction shouldBe ConnectionDirection.FirstToSecond
    secondToFirst.direction shouldBe ConnectionDirection.SecondToFirst
    biDirectional.direction shouldBe ConnectionDirection.BiDirectional
  }
  
  it should "handle excludes correctly" in {
    val excludes = Seq("clock", "reset", "enable")
    val portGroup = UnconnectedPortGroup(
      firstFullName = "uart0",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "uart1",
      first_prefix = "tx_",
      second_prefix = "rx_",
      excludes = excludes
    )
    
    portGroup.excludes should contain allOf("clock", "reset", "enable")
    portGroup.excludes.size shouldBe 3
    
    val emptyExcludes = UnconnectedPortGroup(
      firstFullName = "uart0",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "uart1",
      first_prefix = "tx_",
      second_prefix = "rx_",
      excludes = Seq()
    )
    
    emptyExcludes.excludes shouldBe empty
  }
  
  it should "not throw exceptions with empty data" in {
    val portGroup = UnconnectedPortGroup(
      firstFullName = "uart0",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "uart1",
      first_prefix = "tx_",
      second_prefix = "rx_",
      excludes = Seq()
    )
    
    // Test with empty data - methods should not throw exceptions
    withSilentLogs {
      noException should be thrownBy {
        portGroup.preConnect(Seq.empty[HardwareInstance])
      }
      
      noException should be thrownBy {
        portGroup.finaliseBuses(Seq.empty[HardwareInstance])
      }
      
      val connections = portGroup.connect(Seq.empty[HardwareInstance])
      connections shouldBe empty
      
      val constants = portGroup.collectConstants(Seq.empty)
      constants shouldBe empty
    }
  }
  
  it should "handle prefix patterns correctly" in {
    val portGroup = UnconnectedPortGroup(
      firstFullName = "uart0",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "uart1",
      first_prefix = "tx_",
      second_prefix = "rx_",
      excludes = Seq()
    )
    
    // Test that the prefixes are set correctly
    portGroup.first_prefix shouldBe "tx_"
    portGroup.second_prefix shouldBe "rx_"
    
    // Test with empty prefixes
    val emptyPrefixes = UnconnectedPortGroup(
      firstFullName = "uart0",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "uart1",
      first_prefix = "",
      second_prefix = "",
      excludes = Seq()
    )
    
    emptyPrefixes.first_prefix shouldBe ""
    emptyPrefixes.second_prefix shouldBe ""
  }
}