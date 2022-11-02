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
        case "libraries" => gitLibraryAction()
        case _           => println(s"Unknown type of software ${id.id(1)}")

  private def gitLibraryAction(): Unit =
    assert(id.id(1) == "libraries")
    val branch = if args.size >= 3 then args(2) else "main"
    val cutId = Identifier(id.id.drop(2))

    // we use the last part of the id unless optionally specified
    val dir = if args.size >= 4 then args(3) else cutId.id.mkString("/")
    val folder = paths.libPath / dir
    println(folder)
    if os.exists(folder) then
      println(s"$folder exists, updating")
      if pullBeforeFetch then
        println("Pulling before fetch (AKA sync)")
        gitPushLibSubTree(paths, args(1), dir.toString(), branch)
      println(s"Updating $cutId")
      gitUpdateLibrary(paths, args(1), dir.toString(), branch)
    else
      println(s"$folder does not exists, git fetching ${args(1)}")
      gitAddLibSubTree(paths, args(1), dir.toString(), branch)
