package com.deanoc.overlord.connections

import com.deanoc.overlord.utils.{Utils, Logging}
import com.deanoc.overlord.instances._
import com.deanoc.overlord._
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.connections.ConnectionTypes.{BusName, InstanceName}

import com.deanoc.overlord.interfaces.{
  PortsLike,
  RamLike,
  SupplierBusLike,
  MultiBusLike
}

/** Represents an unconnected bus between two components.
  *
  * This case class defines the properties and methods for managing unconnected
  * bus connections, including pre-connection checks, finalization, and
  * establishing connections.
  *
  * @param firstFullName
  *   The full name of the first component in the connection.
  * @param direction
  *   The direction of the connection.
  * @param secondFullName
  *   The full name of the second component in the connection.
  * @param busProtocol
  *   The protocol used by the bus (default is "default").
  * @param supplierBusName
  *   The name of the supplier bus (optional).
  * @param consumerBusName
  *   The name of the consumer bus (optional).
  * @param silent
  *   Whether to suppress error messages (default is false).
  */
case class UnconnectedBus(
    firstFullName: String,
    direction: ConnectionDirection,
    secondFullName: String,
    busProtocol: BusName = BusName(""),
    supplierBusName: BusName = BusName(""),
    consumerBusName: BusName = BusName(""),
    silent: Boolean = false
) extends Unconnected
    with Logging {

  /** Performs pre-connection checks for the bus connection.
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def preConnect(unexpanded: Seq[HardwareInstance]): Unit = {
    if (direction == ConnectionDirection.BiDirectional) {
      error(
        s"connection between ${firstFullName} and $secondFullName is a undirected bus connection"
      )
      return
    }

    val mo = matchInstances(firstFullName, unexpanded)
    val so = matchInstances(secondFullName, unexpanded)

    if (mo.isEmpty) {
      if (!silent) error(s"$firstFullName instance can't be found");
      return
    }

    if (so.isEmpty) {
      if (!silent) error(s"$secondFullName instance can't be found");
      return
    }

    if (mo.length != 1 || so.length != 1) {
      error(
        s"connection $firstFullName between $secondFullName count error"
      );
      return
    }

    val mainIL = mo.head
    val otherIL = so.head

    // hardware buses require no pre-connecting
    //		if (mainIL.isHardware || otherIL.isHardware) return None

    val (bus: SupplierBusLike, other: HardwareInstance) =
      getBus(mainIL, otherIL, direction) match {
        case Some(result) => result
        case None         => return
      }

    supplierToConsumerHookup(bus, other)
  }

  /** Collects constants associated with the unconnected bus.
    *
    * @param unexpanded
    *   A sequence of unexpanded instance traits.
    * @return
    *   A sequence of constants (empty for this implementation).
    */
  override def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant] =
    Seq()

  private def supplierToConsumerHookup(
      bus: SupplierBusLike,
      other: HardwareInstance
  ): Unit = {
    other.getInterface[RamLike].exists { ram =>
      if (ram.getRanges.isEmpty) {
        error(s"ram ${other.name} has no ranges, so isn't a valid range")
        false
      } else {
        ram.getRanges.foreach {
          case (address, size, _, _) =>
            if (address == -1) bus.addVariableAddressConsumer(other, size)
            else bus.addFixedRelativeAddressConsumer(other, address, size)
        }
        true
      }
    }
  }

  override def finaliseBuses(unexpanded: Seq[HardwareInstance]): Unit = {
    val mo = matchInstances(firstFullName, unexpanded)
    val so = matchInstances(secondFullName, unexpanded)

    if (mo.length != 1 || so.length != 1) return

    val mainIL = mo.head
    val otherIL = so.head

    val (bus: SupplierBusLike, other: HardwareInstance) =
      getBus(mainIL, otherIL, direction) match {
        case Some(result) => result
        case None         => return
      }

    bus.computeConsumerAddresses()

  }

  /** Establishes the connection between the supplier and consumer buses.
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    * @return
    *   A sequence of connected components.
    */
  override def connect(unexpanded: Seq[HardwareInstance]): Seq[Connected] = {
    val mo = matchInstances(firstFullName, unexpanded)
    val so = matchInstances(secondFullName, unexpanded)

    if (mo.length != 1 || so.length != 1) {
      if (!silent)
        error(
          s"connection $firstFullName between $secondFullName count error"
        )
      return Seq[ConnectedBetween]()
    }

    val mainIL = mo.head
    val otherIL = so.head

    val (bus: SupplierBusLike, other: HardwareInstance) =
      getBus(mainIL, otherIL, direction) match {
        case Some(result) => result
        case None         => return Seq()
      }

    val busConnection = Seq[Connected](
      ConnectedBus(
        ConnectionPriority.Group,
        mainIL,
        direction,
        otherIL,
        bus,
        other
      )
    )

    trace(s"bus connection $firstFullName to $secondFullName")

    // pure hardware buses require no pre-connecting
    if (mainIL.isHardware && otherIL.isHardware) return busConnection

    val busPortsOpt = bus.getInterface[PortsLike]
    val otherPortsOpt = other.getInterface[PortsLike]

    if (busPortsOpt.isEmpty || otherPortsOpt.isEmpty) {
      val missingInterfaces = Seq(
        if (busPortsOpt.isEmpty) Some("bus") else None,
        if (otherPortsOpt.isEmpty) Some("other") else None
      ).flatten.mkString(" and ")

      warn(s"$missingInterfaces does not have PortsLike interface, returning empty sequence")
      return Seq()
    }

    val busPorts = busPortsOpt.get
    val otherPorts = otherPortsOpt.get

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

  /** Retrieves the bus connection between two instance locations.
    *
    * @param mainIL
    *   The main instance location.
    * @param otherIL
    *   The other instance location.
    * @param direction
    *   The direction of the connection.
    * @return
    *   An optional tuple containing the supplier bus and the other chip
    *   instance.
    */
  private def getBus(
      mainIL: InstanceLoc,
      otherIL: InstanceLoc,
      direction: ConnectionDirection
  ): Option[(SupplierBusLike, HardwareInstance)] = {
    /*
		// hardware can't be connected to another type
		if (mainIL.isHardware != otherIL.isHardware) {
			println(s"ERROR: connection $firstFullName between $secondFullName is connecting hardware to non hardware");
			return None
		}*/

    // check both instance have ports
    if (!mainIL.instance.hasInterface[PortsLike]) {
      error(
        s"$firstFullName doesn't expose a ports interface, so can't be connected to a bus"
      );
      return None
    }
    if (!otherIL.instance.hasInterface[PortsLike]) {
      error(
        s"$secondFullName doesn't expose a ports interface, so can't be connected to a bus"
      );
      return None
    }

    val multiBuses0: (Option[MultiBusLike], Option[MultiBusLike]) =
      direction match {
        case ConnectionDirection.FirstToSecond =>
          (
            mainIL.instance.getInterface[MultiBusLike],
            otherIL.instance.getInterface[MultiBusLike]
          )
        case ConnectionDirection.SecondToFirst =>
          (
            otherIL.instance.getInterface[MultiBusLike],
            mainIL.instance.getInterface[MultiBusLike]
          )
        case ConnectionDirection.BiDirectional =>
          error(
            s"connection between ${firstFullName} and $secondFullName is a undirected bus connection"
          )
          return None
      }
    if (multiBuses0._1.isEmpty && multiBuses0._2.isEmpty) {
      error(
        s"neither ${firstFullName} and $secondFullName has no multi bus interface, so can't be connected as a bus"
      )
      return None
    }

    val (supplierMultiBusO, consumerMultiBusesO) = multiBuses0
    if (supplierMultiBusO.isEmpty) {
      error(s"Supplier bus connected must always be multi bus like")
      return None
    }
    val supplierMultiBus = supplierMultiBusO.get
    if (supplierMultiBus.numberOfBuses < 1) {
      error(
        s"No buses available on supplier MultiBus so no connection possible"
      );
      return None
    }

    // no specified name or protocol, we could try port matching of buses???
    if (supplierBusName.isEmpty && busProtocol.isEmpty) {
      return None
    }

    if (
      direction == ConnectionDirection.SecondToFirst && !mainIL.instance
        .isInstanceOf[HardwareInstance]
    ) {
      error(
        s"$firstFullName isn't a chip instance, so can't be connected to a bus"
      );
      return None
    } else if (
      direction == ConnectionDirection.FirstToSecond && !otherIL.instance
        .isInstanceOf[HardwareInstance]
    ) {
      error(
        s"$secondFullName isn't a chip instance, so can't be connected to a bus"
      );
      return None
    }

    val other: HardwareInstance =
      (if (direction == ConnectionDirection.SecondToFirst) mainIL.instance
       else otherIL.instance).asInstanceOf[HardwareInstance]
    // search through suppliers name and protocols for a match
    val supplierBus: SupplierBusLike = {
      trace(s"looking for bus $supplierBusName and protocol $busProtocol on $firstFullName")
      
      // Check if supplierBusName is invalid
      if (supplierBusName.isEmpty || supplierBusName.toString.trim.isEmpty) {
        warn(s"$firstFullName has an invalid or empty supplier bus name")
        return None
      }

      val byName =
        supplierMultiBus.getFirstSupplierBusByName(supplierBusName.toString)
      if (byName.nonEmpty) byName.get
      else if (supplierBusName.isEmpty) {
        val byProtocol =
          supplierMultiBus.getFirstSupplierBusOfProtocol(busProtocol.toString)
        if (byProtocol.nonEmpty) byProtocol.get
        else {
          error(
            s"$firstFullName and $secondFullName are trying to connect but don't have a $busProtocol protocol in common"
          )
          return None
        }
      } else {
        error(
          s"$firstFullName and $secondFullName supplier doesn't have a $supplierBusName named bus"
        )
        return None
      }
    }

    // now check the selected supplier bus is supported on the consumer side it is also a multi bus
    if (consumerMultiBusesO.nonEmpty) {
      val consumerMultiBus = consumerMultiBusesO.get
      trace(s"looking for bus $consumerBusName and protocol $busProtocol on $secondFullName")
      val byName =
        consumerMultiBus.getFirstConsumerBusByName(consumerBusName.toString)
      if (byName.isEmpty && busProtocol.nonEmpty) {
        val byProtocol =
          consumerMultiBus.getFirstConsumerBusOfProtocol(busProtocol.toString)
        if (byProtocol.isEmpty) {
          error(
            s"$firstFullName and $secondFullName are trying to connect but consumer doesn't support $busProtocol protocol"
          )
          return None
        }
      } else if (byName.isEmpty) {
        error(
          s"$firstFullName and $secondFullName consumer doesn't have a $consumerBusName named bus"
        )
        return None
      }
    }

    Some(supplierBus, other)
  }

}
