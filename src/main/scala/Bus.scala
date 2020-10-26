package overlord

class Bus( override val name : String, override val isHard : Boolean)
extends BoardFeature(name, isHard)

class InternalBus(override val name : String, override val isHard : Boolean)
extends Bus(name, isHard)

class Axi3Bus(override val name : String, override val isHard : Boolean)
extends Bus(name, isHard)

class Axi4Bus(override val name : String, override val isHard : Boolean)
extends Bus(name, isHard)
