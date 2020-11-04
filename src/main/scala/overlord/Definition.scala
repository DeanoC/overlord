package overlord


case class Software(val registers : Array[Registers])

case class Gateware()

case class Definition(val chipType: String,
                      val attributes: Map[String, toml.Value],
                      val software: Option[Software] = None,
                      val gateware: Option[Gateware] = None,
)