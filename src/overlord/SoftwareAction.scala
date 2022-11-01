package Overlord

sealed trait SoftwareAction(paths: Paths, id: Identifier, sw: SoftwareDef):
  def doAction(): Unit

case class FetchSoftwareAction(
    paths: Paths,
    id: Identifier,
    sw: SoftwareDef,
    args: Seq[String],
    pullBeforeFetch: Boolean
) extends SoftwareAction(paths, id, sw):

  override def doAction(): Unit =
    args(0) match
      case "git" => gitAction()
      case _     => println(s"Unknown fetch subaction ${args.mkString(" ")}")

    def gitAction(): Unit =
      // check we are a valid software action
      assert(id.id(0) == "software")
      id.id(1) match
        case "libs" => gitLibraryAction()
        case _      => println(s"Unknown type of software ${id.id(1)}")

  private def gitLibraryAction(): Unit =
    assert(id.id(1) == "libs")
    val branch = if args.size == 3 then args(2) else "main"

    // and get the rest
    val cutId = Identifier(id.id.drop(2))
    val folder = paths.libPath / cutId.id.mkString("/")
    if os.exists(folder) then
      println(s"$folder exists, updating")
      if pullBeforeFetch then
        println("Pulling before fetch (AKA sync)")
        gitPushLibSubTree(paths, args(1), cutId.toString(), branch)
      println(s"Updating $cutId")
      gitUpdateLibrary(paths, args(1), cutId.toString(), branch)
    else
      println(s"$folder does not exists, git fetching ${args(1)}")
      gitAddLibSubTree(paths, args(1), cutId.toString(), branch)
