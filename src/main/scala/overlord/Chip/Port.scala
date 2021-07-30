package overlord.Chip

import ikuy_utils._
import toml.Value

sealed trait WireDirection {
	override def toString: String = {
		this match {
			case InWireDirection()    => "input"
			case OutWireDirection()   => "output"
			case InOutWireDirection() => "inout"
		}
	}

	def flip: WireDirection = this match {
		case InWireDirection()    => OutWireDirection()
		case OutWireDirection()   => InWireDirection()
		case InOutWireDirection() => this
	}
}

case class InWireDirection() extends WireDirection

case class OutWireDirection() extends WireDirection

case class InOutWireDirection() extends WireDirection

sealed case class BitsDesc(text: String) {
	private val txt        : String  = text.filterNot(c => c == '[' || c == ']')
	private val singleDigit: Boolean = txt.split(":").length < 2
	val hi       : Int     = Integer.parseInt(txt.split(":")(0))
	val lo       : Int     =
		if (singleDigit) hi
		else Integer.parseInt(txt.split(":")(1))
	val bitCount : Int     = (hi - lo) + 1
	val singleBit: Boolean = singleDigit || bitCount == 1
	lazy val mask: Long = ((1L << bitCount.toLong) - 1L) << lo.toLong
}

object BitsDesc {
	def apply(width: Int): BitsDesc = BitsDesc(s"[${width - 1}:0]")
}


object WireDirection {
	def apply(s: String): WireDirection = {
		s.toLowerCase match {
			case "in" | "input"   => InWireDirection()
			case "out" | "output" => OutWireDirection()
			case "inout" | "bi"   => InOutWireDirection()
			case _                => println(s"$s is an invalid port direction")
				InOutWireDirection()
		}
	}
}

case class Port(name: String,
                width: BitsDesc,
                direction: WireDirection = InOutWireDirection())

object Ports {
	def apply(array: Array[Variant]): Seq[Port] =
		array.flatMap(_ match {
			              case TableV(tbl) => if (tbl.contains("name")) {
				              val name  = Utils.toString(tbl("name"))
				              val width = Utils.lookupInt(tbl, "width", 1)
				              val dir   = WireDirection(
					              Utils.lookupString(tbl, "direction", "inout"))
				              Some(Port(name, BitsDesc(width), dir))
			              }
			              else {
				              println(s"$array is a port inline table without a name")
				              None
			              }
			              case StringV(s)  => Some(Port(s, BitsDesc(1)))
			              case _           => None
		              })
}