package Overlord
import scala.collection.mutable
import org.virtuslab.yaml.StringOps

case class Software(
    paths: Paths,
    zigTargets: ZigTargets,
    catalog: Catalog,
    software: Seq[SoftwareDef],
    dictionary: Map[String, String],
    gccTriples: Seq[Triple],
    llvmTriples: Seq[Triple],
    board: Seq[Board],
    cpus: Seq[Cpu]
):
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
  software.foreach(sw => produceSoftware(sw))

//  private def getAllDependencies(sw: SoftwareDef, currentDeps: List[String]): List[String]

  private def makeIdentifier(p: String) = Identifier(Seq("software", "libs") ++ p.split('.') ++ Seq("lib"))

  private def getAllLibraries(sw: SoftwareDef, libsByPath: mutable.HashMap[os.Path, SoftwareDef]): Unit =
    if sw.libraries.contains(sw.name) then
      println(s"ERROR: ${sw.name} has itself as a library dependency")
      return

    sw.libraries
      .flatMap(p =>
        catalog.fetch(makeIdentifier(p)) match
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
            text.as[SoftwareDef] match
              case Left(err)    => println(err); None
              case Right(value) => libsByPath(c.filePath) = value; Some(value)
        else None
      )
      .flatten
      .toSet
      .map(c => getAllLibraries(c, libsByPath))

  private def produceLibs(libs: Map[Identifier, SoftwareDef]): Unit =
    val actions = libs.flatMap((id, sw) =>
      println(s"${id.toString()}: ")
      // lets through each action
      sw.actions.flatMap(a =>
        val action = a.split(" ")
        action(0) match
          case "fetch" => Some(FetchSoftwareAction(paths, id, sw, action.tail))
          case _       => println(s"Unknown action ${action(0)}"); None
      )
    )

  private def produceSoftware(swd: SoftwareDef): Unit =
    val cpusRequired = swd.cpus.map(_.replace("host", zigTargets.native.cpu.arch))
    for
      cpu <- cpusRequired
      if !cpus.exists(_.arch == cpu)
    do println(s"$cpu is required for ${swd.name} software")
    val libsByPath = mutable.HashMap[os.Path, SoftwareDef]()
    getAllLibraries(swd, libsByPath)
    produceLibs(libsByPath.map(l => (makeIdentifier(l._2.name) -> l._2)).toMap);
