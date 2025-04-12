package com.deanoc.overlord.Connections

import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.Instances._
import com.deanoc.overlord._

import com.deanoc.overlord.Interfaces.{
  PortsLike,
  RamLike,
  SupplierBusLike,
  MultiBusLike
}

case class UnconnectedBus(
    firstFullName: String,
    direction: ConnectionDirection,
    secondFullName: String,
    busProtocol: String = "default",
    supplierBusName: String = "",
    consumerBusName: String = "",
    silent: Boolean = false
) extends Unconnected {

  override def preConnect(unexpanded: Seq[ChipInstance]): Unit = {
    if (direction == BiDirectionConnection()) {
      println(
        s"ERROR: connection between ${firstFullName} and $secondFullName is a undirected bus connection"
      )
      return
    }

    val mo = matchInstances(firstFullName, unexpanded)
    val so = matchInstances(secondFullName, unexpanded)

    if (mo.isEmpty) {
      if (!silent) println(s"ERROR: $firstFullName instance can't be found");
      return
    }

    if (so.isEmpty) {
      if (!silent) println(s"ERROR: $secondFullName instance can't be found");
      return
    }

    if (mo.length != 1 || so.length != 1) {
      println(
        s"ERROR: connection $firstFullName between $secondFullName count error"
      );
      return
    }

    val mainIL = mo.head
    val otherIL = so.head

    // hardware buses require no pre-connecting
    //		if (mainIL.isHardware || otherIL.isHardware) return None

    val (bus: SupplierBusLike, other: ChipInstance) =
      getBus(mainIL, otherIL, direction) match {
        case Some(result) => result
        case None         => return
      }

    supplierToConsumerHookup(bus, other)
  }

  override def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant] =
    Seq()

  private def supplierToConsumerHookup(
      bus: SupplierBusLike,
      other: ChipInstance
  ): Unit = {
    if (other.hasInterface[RamLike]) {
      val ram = other.getInterfaceUnwrapped[RamLike]
      if (ram.getRanges.isEmpty) {
        println(
          s"ERROR: ram ${other.name} has no ranges, so isn't a valid range"
        )
      }
      ram.getRanges.foreach {
        case (address, size, _, _) => {
          if (address == -1) bus.addVariableAddressConsumer(other, size)
          else bus.addFixedRelativeAddressConsumer(other, address, size)
        }
      }
    }
  }

  override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = {
    val mo = matchInstances(firstFullName, unexpanded)
    val so = matchInstances(secondFullName, unexpanded)

    if (mo.length != 1 || so.length != 1) return

    val mainIL = mo.head
    val otherIL = so.head

    val (bus: SupplierBusLike, other: ChipInstance) =
      getBus(mainIL, otherIL, direction) match {
        case Some(result) => result
        case None         => return
      }

    bus.computeConsumerAddresses()

  }

  private def getBus(
      mainIL: InstanceLoc,
      otherIL: InstanceLoc,
      direction: ConnectionDirection
  ): Option[(SupplierBusLike, ChipInstance)] = {
    /*
		// hardware can't be connected to another type
		if (mainIL.isHardware != otherIL.isHardware) {
			println(s"ERROR: connection $firstFullName between $secondFullName is connecting hardware to non hardware");
			return None
		}*/

    // check both instance have ports
    if (!mainIL.instance.hasInterface[PortsLike]) {
      println(
        s"$firstFullName doesn't expose a ports interface, so can't be connected to a bus"
      );
      return None
    }
    if (!otherIL.instance.hasInterface[PortsLike]) {
      println(
        s"$secondFullName doesn't expose a ports interface, so can't be connected to a bus"
      );
      return None
    }

    val multiBuses0: (Option[MultiBusLike], Option[MultiBusLike]) =
      direction match {
        case FirstToSecondConnection() =>
          (
            mainIL.instance.getInterface[MultiBusLike],
            otherIL.instance.getInterface[MultiBusLike]
          )
        case SecondToFirstConnection() =>
          (
            otherIL.instance.getInterface[MultiBusLike],
            mainIL.instance.getInterface[MultiBusLike]
          )
        case BiDirectionConnection() =>
          println(
            s"ERROR: connection between ${firstFullName} and $secondFullName is a undirected bus connection"
          )
          return None
      }
    if (multiBuses0._1.isEmpty && multiBuses0._2.isEmpty) {
      println(
        s"ERROR: neither ${firstFullName} and $secondFullName has no multi bus interface, so can't be connected as a bus"
      )
      return None
    }

    val (supplierMultiBusO, consumerMultiBusesO) = multiBuses0
    if (supplierMultiBusO.isEmpty) {
      println(s"ERROR: Supplier bus connected must always be multi bus like")
      return None
    }
    val supplierMultiBus = supplierMultiBusO.get
    if (supplierMultiBus.numberOfBuses < 1) {
      println(
        s"No buses available on supplier MultiBus so no connection possible"
      );
      return None
    }

    // no specified name or protocol, we could try port matching of buses???
    if (supplierBusName.isEmpty && busProtocol.isEmpty) {
      return None
    }

    if (
      direction == SecondToFirstConnection() && !mainIL.instance
        .isInstanceOf[ChipInstance]
    ) {
      println(
        s"$firstFullName isn't a chip instance, so can't be connected to a bus"
      );
      return None
    } else if (
      direction == FirstToSecondConnection() && !otherIL.instance
        .isInstanceOf[ChipInstance]
    ) {
      println(
        s"$secondFullName isn't a chip instance, so can't be connected to a bus"
      );
      return None
    }

    val other: ChipInstance =
      (if (direction == SecondToFirstConnection()) mainIL.instance
       else otherIL.instance).asInstanceOf[ChipInstance]
    // search through suppliers name and protocols for a match
    val supplierBus: SupplierBusLike = {
      val byName = supplierMultiBus.getFirstSupplierBusByName(supplierBusName)
      if (byName.nonEmpty) byName.get
      else if (supplierBusName.isEmpty) {
        val byProtocol =
          supplierMultiBus.getFirstSupplierBusOfProtocol(busProtocol)
        if (byProtocol.nonEmpty) byProtocol.get
        else {
          println(
            s"ERROR: $firstFullName and $secondFullName are trying to connect but don't have a $busProtocol protocol in common"
          )
          return None
        }
      } else {
        println(
          s"ERROR: $firstFullName and $secondFullName supplioer doesn't have a $supplierBusName named bus"
        )
        return None
      }
    }

    // now check the selected supplier bus is supported on the consumer side it is also a multi bus
    if (consumerMultiBusesO.nonEmpty) {
      val consumerMultiBus = consumerMultiBusesO.get
      val byName = consumerMultiBus.getFirstConsumerBusByName(consumerBusName)
      if (byName.isEmpty && busProtocol.nonEmpty) {
        val byProtocol =
          consumerMultiBus.getFirstConsumerBusOfProtocol(busProtocol)
        if (byProtocol.isEmpty) {
          println(
            s"ERROR: $firstFullName and $secondFullName are trying to connect but consumer doesn't support $busProtocol protocol"
          )
          return None
        }
      } else if (byName.isEmpty) {
        println(
          s"ERROR: $firstFullName and $secondFullName consumer doesn't have a $consumerBusName named bus"
        )
        return None
      }
    }

    Some(supplierBus, other)
  }

  override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = {
    val mo = matchInstances(firstFullName, unexpanded)
    val so = matchInstances(secondFullName, unexpanded)

    if (mo.length != 1 || so.length != 1) {
      if (!silent)
        println(
          s"connection $firstFullName between $secondFullName count error"
        )
      return Seq[ConnectedBetween]()
    }

    val mainIL = mo.head
    val otherIL = so.head

    val (bus: SupplierBusLike, other: ChipInstance) =
      getBus(mainIL, otherIL, direction) match {
        case Some(result) => result
        case None         => return Seq()
      }

    val busConnection = Seq[Connected](
      ConnectedBus(
        GroupConnectionPriority(),
        mainIL,
        direction,
        otherIL,
        bus,
        other
      )
    )

    // pure hardware buses require no pre-connecting
    if (mainIL.isHardware && otherIL.isHardware) return busConnection

    val busPorts: PortsLike = bus.getInterfaceUnwrapped[PortsLike]
    val otherPorts: PortsLike = other.getInterfaceUnwrapped[PortsLike]
    val busPrefix = bus.getPrefix
    val otherPrefix = Utils.lookupString(other.attributes, "bus_prefix", "bus_")

    val cpgs: Seq[Connected] = for {
      fp <- busPorts.getPortsStartingWith(busPrefix)
      otherName = s"${otherPrefix}${fp.name.replace(busPrefix, "")}"
      sp <- otherPorts.getPortsMatchingName(otherName)
    } yield ConnectedPortGroup(
      busPorts,
      fp,
      mainIL.fullName,
      otherPorts,
      sp,
      direction
    )

    busConnection ++ cpgs
  }

}
