package Overlord

sealed trait SoftwareAction(paths: Paths, id: Identifier, sw: SoftwareDef)

case class FetchSoftwareAction(paths: Paths, id: Identifier, sw: SoftwareDef, args: Seq[String])
    extends SoftwareAction(paths, id, sw):
  args(0) match
    case "git" => gitAction()
    case _     => println(s"Unknown fetch subaction ${args.mkString(" ")}")

  def gitAction(): Unit =
    // check we are a valid lib.software action and get the rest minus the end lib
    assert(id.id(0) == "software")
    assert(id.id(1) == "libs")
    val cutId = Identifier(id.id.drop(2).dropRight(1))
    val folder = paths.libPath / cutId.id.mkString("/")
    if os.exists(folder) then
      println(s"$folder exists, updating")
      gitUpdateLibrary(paths, s"libs_${cutId.toString()}")
    else
      println(s"$folder does not exists, git fetching ${args(1)}")
      gitAddLibSubTree(paths, args(1), cutId.toString())
