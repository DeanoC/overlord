package overlord.Interfaces

trait MultiBusLike extends ChipLike {
  def numberOfBuses: Int
  def getBus(index: Int): Option[BusLike]
  def getFirstSupplierBusByName(name: String): Option[SupplierBusLike]
  def getFirstSupplierBusOfProtocol(protocol: String): Option[SupplierBusLike]
  def getFirstConsumerBusByName(name: String): Option[BusLike]
  def getFirstConsumerBusOfProtocol(protocol: String): Option[BusLike]
}