package Overlord
import scala.collection.mutable
import org.virtuslab.yaml.StringOps

case class Software(
    paths: Paths,
    zigTargets: ZigTargets,
    catalog: Catalog,
    software: Seq[SoftwareDef],
    localProgramNames: Seq[Identifier],
    localLibNames: Seq[Identifier],
    dictionary: Map[String, String],
    gccTriples: Seq[Triple],
    llvmTriples: Seq[Triple],
    board: Seq[Board],
    cpus: Seq[Cpu],
    pushBeforeFetch: Boolean,
    skipGit: Boolean
):
  private def makeLibraryIdentifier(p: String) = Identifier(Seq("software", "libs") ++ p.split('.') ++ Seq("lib"))

  private lazy val baseTemplatePath = paths.targetPath / "ikuy_std_resources" / "templates"
  // produce GCC build files
  if gccTriples.nonEmpty then
    val builder = mutable.StringBuilder()
    builder ++= os.read(baseTemplatePath / "c++" / "make_compilers.sh")
    val gccFlags = "" // TODO
    builder ++= {
      for triple <- gccTriples
      yield s"""
        |build_binutils ${triple.toString} $${bin_path}
        |build_gcc ${triple.toString} $${bin_path} "$gccFlags"
        |""".stripMargin
    }.mkString
    val builderResult = builder.result().overlordStringInterpolate(dictionary)
    os.write.over(paths.binPath / "make_compilers.sh", builderResult, perms = os.PermSet.fromString("rwxr-xr-x"))
    // TODO CMAKE toolchain files
  end if

  val localPrograms = for
    sw <- software
    progName <- localProgramNames
    if progName.contains(sw.name)
  yield sw

  // TODO local libraries?

  // recursive get all libraries needed by all local programs
  val libsById = {
    val libsByPath = mutable.HashMap[os.Path, SoftwareDef]()
    localPrograms.foreach(getAllLibraries(_, libsByPath))
    libsByPath.map(l => (makeLibraryIdentifier(l._2.name) -> l._2)).toMap
  }
  val actions = produceSoftwareActions(libsById)
  actions.foreach(_.doAction())

  val zigLocalPrograms = localPrograms.filter(sw => sw.builder.contains("zig"))
  val zigTop = ZigSoftware.programsTop(paths, zigLocalPrograms, libsById)
  os.write.over(paths.targetPath / "build.zig", zigTop)

  // val cmakeLocalSoftware = localPrograms.filter(sw => sw.builder.contains("cmake"))

  private def getAllLibraries(sw: SoftwareDef, libsByPath: mutable.HashMap[os.Path, SoftwareDef]): Unit =
    if sw.libraries.contains(sw.name) then
      println(s"ERROR: ${sw.name} has itself as a library dependency")
      return

    sw.libraries
      .flatMap(p =>
        catalog.fetch(makeLibraryIdentifier(p)) match
          case None        => println(s"ERROR: Unknown library ${p}"); None
          case Some(value) => Some(value)
      )
      .map(c =>
        if !libsByPath.contains(c.filePath) then
          val text = os.read(c.filePath)
          if text.isEmpty() then
            println(s"ERROR: library ${c.name} has no body");
            None
          else
            text.as[LibSoftwareDef] match
              case Left(err)    => println(s"ERROR ${c.filePath} with $err"); None
              case Right(value) => libsByPath(c.filePath) = value; Some(value)
        else None
      )
      .flatten
      .toSet
      .map(c => getAllLibraries(c, libsByPath))

  private def produceSoftwareActions(libs: Map[Identifier, SoftwareDef]): Seq[FetchSoftwareAction] =
    libs
      .flatMap((id, sw) =>
        println(s"${id.toString()}: ")
        // lets go through each action
        sw.actions.flatMap(a =>
          val action = a.split(" ")
          action(0) match
            case "fetch" =>
              if action(1) == "git" && skipGit then None
              else Some(FetchSoftwareAction(paths, id, sw, action.tail, pushBeforeFetch))
            case _ => println(s"Unknown action ${action(0)}"); None
        )
      )
      .to(Seq)
