package overlord

class Interconnect( override val name : String, override val isHard : Boolean,
                    val busses : List[String])
extends BoardFeature(name, isHard)

