package com.deanoc.overlord.output

import java.nio.file.Path
import scala.collection.mutable

import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.instances.CpuInstance
import com.deanoc.overlord.{Project, Resources}

object Compiler {
  def apply(game: Project, out: Path): Unit = {
    if (game.cpus.isEmpty) return

    println(s"Creating Compiler scripts at $out")

    if (generateMakeCompilersScript(game, out)) return

    game.cpus.foreach(genCMakeToolChains(_, out))
  }

  private def generateMakeCompilersScript(game: Project, out: Path): Boolean = {
    val compilerScriptBuilder = new StringBuilder
    compilerScriptBuilder ++= (Utils.readFile(
      Resources.stdResourcePath().resolve("catalogs/software/make_compilers.sh")
    ) match {
      case Some(script) => script
      case None =>
        println("ERROR: resource make_compilers.sh not found!")
        return true
    })
    game.cpus.foreach({ cpu =>
      val (triple, gccFlags) = (cpu.triple, cpu.gccFlags)

      compilerScriptBuilder ++=
        s"""
				 |build_binutils $triple $$PWD/compilers
				 |build_gcc $triple $$PWD/compilers "$gccFlags"
				 |""".stripMargin

    })

    Utils.writeFile(
      out.resolve("make_compilers.sh"),
      compilerScriptBuilder.result()
    )
    Utils.setFileExecutable(out.resolve(s"make_compilers.sh"))
    false
  }

  private def sanatizeTriple(triple: String): String = {
    triple.replace("-", "_")
  }

  private def genCMakeToolChains(cpu: CpuInstance, out: Path): Unit = {
    val (cpuType, triple, gccFlags) = (cpu.cpuType, cpu.triple, cpu.gccFlags)

    val template = Utils.readFile(
      Resources
        .stdResourcePath()
        .resolve("catalogs/software/toolchain_template.cmake")
    ) match {
      case Some(script) => script
      case None =>
        println("ERROR: resource make_compilers.sh not found!")
        return
    }

    // try to read a specialist toolchain file, if none exist use template
    val specialist = Resources
      .stdResourcePath()
      .resolve(s"catalogs/software/toolchain_$cpuType.cmake")
    val tt = Utils.readFile(specialist) match {
      case Some(s) => s
      case None =>
        template
          .replace("""${triple}""", triple)
          .replace("""${GCC_FLAGS}""", gccFlags)
    }

    Utils.writeFile(out.resolve(cpuType + "_toolchain.cmake"), tt)
  }
}
