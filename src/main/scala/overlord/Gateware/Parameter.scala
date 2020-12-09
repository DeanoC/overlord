package overlord.Gateware

import overlord.Utils
import toml.Value

case class Parameter(key: String,
                     value: toml.Value)

object Parameters {
	def apply(array: Seq[toml.Value]): Seq[Parameter] = {
		(for (v <- array) yield {
			v match {
				case Value.Tbl(tbl) => if (tbl.contains("key")) {
					val key  = Utils.toString(tbl("key"))
					val value = if(tbl.contains("value")) tbl("value")
					else toml.Value.Str("NO_VALUE")

					Some(Parameter(key, value))
				}
				else {
					println(s"$array is a parameter inline table without a key")
					None
				}
				case Value.Str(s)   => Some(Parameter(s, toml.Value.Str("NO_VALUE")))
				case _              => None
			}
		}).flatten

	}
}