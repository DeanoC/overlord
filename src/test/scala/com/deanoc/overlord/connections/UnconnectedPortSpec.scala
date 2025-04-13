package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.ChipInstance
import org.scalatestplus.mockito.MockitoSugar

class UnconnectedPortSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {

  "UnconnectedPort" should "store connection properties correctly" in {
    val port = UnconnectedPort(
      firstFullName = "device1.tx",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2.rx"
    )
    
    port.firstFullName shouldBe "device1.tx"
    port.direction shouldBe ConnectionDirection.FirstToSecond
    port.secondFullName shouldBe "device2.rx"
  }
  
  it should "support all connection directions" in {
    val firstToSecond = UnconnectedPort(
      firstFullName = "device1.tx",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2.rx"
    )
    
    val secondToFirst = UnconnectedPort(
      firstFullName = "device2.rx",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "device1.tx"
    )
    
    val biDirectional = UnconnectedPort(
      firstFullName = "device1.io",
      direction = ConnectionDirection.BiDirectional,
      secondFullName = "device2.io"
    )
    
    firstToSecond.direction shouldBe ConnectionDirection.FirstToSecond
    secondToFirst.direction shouldBe ConnectionDirection.SecondToFirst
    biDirectional.direction shouldBe ConnectionDirection.BiDirectional
  }
  
  it should "not throw exceptions with empty data" in {
    val port = UnconnectedPort(
      firstFullName = "device1.tx",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2.rx"
    )
    
    // Test with empty data - methods should not throw exceptions
    withSilentLogs {
      noException should be thrownBy {
        port.preConnect(Seq.empty[ChipInstance])
      }
      
      noException should be thrownBy {
        port.finaliseBuses(Seq.empty[ChipInstance])
      }
      
      val connections = port.connect(Seq.empty[ChipInstance])
      connections shouldBe empty
      
      val constants = port.collectConstants(Seq.empty)
      constants shouldBe empty
    }
  }
  
  it should "handle port name patterns correctly" in {
    val port = UnconnectedPort(
      firstFullName = "device1.data_out",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2.data_in"
    )
    
    // Test that the port name contains the expected parts
    port.firstFullName should include ("data_out")
    port.secondFullName should include ("data_in")
    
    // Check that the format is correct - component.port_name
    port.firstFullName.split('.').length shouldBe 2
    port.secondFullName.split('.').length shouldBe 2
  }
}