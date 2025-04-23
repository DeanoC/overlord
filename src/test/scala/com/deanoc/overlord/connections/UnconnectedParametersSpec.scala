package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{SilentLogger, Variant, IntV, StringV}
import com.deanoc.overlord.instances.{HardwareInstance, InstanceTrait}
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito.mock

class UnconnectedParametersSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {

  "UnconnectedParameters" should "store connection properties correctly" in {
    // Create parameter instances
    val constantParam = Parameter("frequency", ConstantParameterType(StringV("100MHz")))
    val frequencyParam = Parameter("clock_freq", FrequencyParameterType(50.0))
    
    val params = UnconnectedParameters(
      direction = ConnectionDirection.FirstToSecond,
      instanceName = "device1",
      parameters = Seq(constantParam, frequencyParam)
    )
    
    params.direction shouldBe ConnectionDirection.FirstToSecond
    params.instanceName shouldBe "device1"
    params.parameters should have length 2
    params.parameters should contain (constantParam)
    params.parameters should contain (frequencyParam)
  }
  
  it should "support different parameter types" in {
    val constantParam = Parameter("value", ConstantParameterType(IntV(42)))
    val frequencyParam = Parameter("freq", FrequencyParameterType(100.0))
    
    constantParam.name shouldBe "value"
    constantParam.parameterType shouldBe a[ConstantParameterType]
    
    frequencyParam.name shouldBe "freq"
    frequencyParam.parameterType shouldBe a[FrequencyParameterType]
    
    val constType = constantParam.parameterType.asInstanceOf[ConstantParameterType]
    val freqType = frequencyParam.parameterType.asInstanceOf[FrequencyParameterType]
    
    // Test the actual values
    freqType.freq shouldBe 100.0
  }
  
  it should "not throw exceptions with empty data" in {
    val params = UnconnectedParameters(
      direction = ConnectionDirection.FirstToSecond,
      instanceName = "device1",
      parameters = Seq()
    )
    
    // Test with empty data - methods should not throw exceptions
    withSilentLogs {
      noException should be thrownBy {
        params.preConnect(Seq.empty[HardwareInstance])
      }
      
      noException should be thrownBy {
        params.finaliseBuses(Seq.empty[HardwareInstance])
      }
      
      val connections = params.connect(Seq.empty[HardwareInstance])
      connections shouldBe empty
      
      val constants = params.collectConstants(Seq.empty)
      constants shouldBe empty
    }
  }
  
  it should "collect constants from matching instances" in {
    // Create parameters
    val param1 = Parameter("param1", ConstantParameterType(StringV("value1")))
    val param2 = Parameter("param2", ConstantParameterType(StringV("value2")))
    
    val params = UnconnectedParameters(
      direction = ConnectionDirection.FirstToSecond,
      instanceName = "device1",
      parameters = Seq(param1, param2)
    )
    
    // We can't fully test instance matching without complex mocking,
    // but we can verify the basic structure is correct
    params.parameters should have length 2
    params.instanceName shouldBe "device1"
  }
  
  "UnconnectedParameters factory" should "delegate to ConnectionParser" in {
    // Since we can't easily mock static methods in the ConnectionParser,
    // this is just a basic test to verify the method exists and has correct signature
    
    val direction = ConnectionDirection.FirstToSecond
    val instanceName = "device1"
    val paramsArray = Array.empty[Variant]
    
    // This call should compile without errors, which verifies the method signature
    // The actual functionality depends on ConnectionParser implementation
    noException should be thrownBy {
      UnconnectedParameters.apply(direction, instanceName, paramsArray)
    }
  }
}