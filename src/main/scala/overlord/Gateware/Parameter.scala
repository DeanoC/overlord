package overlord.Gateware

import ikuy_utils._
import toml.Value

case class Parameter(key: String,
                     value: BigInt)

object Parameters {
	def apply(array: Seq[toml.Value]): Seq[Parameter] = {
		(for (v <- array) yield {
			v match {
				case Value.Tbl(tbl) => if (tbl.contains("key")) {
					val key  = Utils.toString(tbl("key"))
					val value = Utils.lookupBigInt(tbl,"value",0)

					Some(Parameter(key, value))
				}
				else {
					println(s"$array is a parameter inline table without a key")
					None
				}
				case Value.Str(s)   => Some(Parameter(s, 0))
				case _              => None
			}
		}).flatten

	}
}