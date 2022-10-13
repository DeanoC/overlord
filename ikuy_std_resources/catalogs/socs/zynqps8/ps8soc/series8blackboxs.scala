import spinal.core._


case class BUFG_PS() extends BlackBox {
	val io = new Bundle {
		val I = in Bool()
		val O = out Bool()
	}
	noIoPrefix()
}
