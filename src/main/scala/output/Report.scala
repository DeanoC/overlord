package output

import java.nio.file.Path

import overlord._

object Report {

	def apply(game: Game, out : Path): Unit = {
		import toml.Value

		val cpus     = game.cpus
		val cpuTypes = cpus.map(_.definition).toSet
		println(f"------------------")
		println(f"${cpus.length} CPU cores of ${cpuTypes.size} types")
		println(f"------------------")
		for (cput <- cpuTypes) {
			val chipType = cput.defType.toString
			val arch     = Utils.toString(cput.attributes("arch"))
			val bw       = Utils.toInt(cput.attributes("width"))

			println(s"${chipType} are ${bw} bit $arch CPUs")
			if(cput.software.nonEmpty)
				cput.software.get.groups.foreach(r => println(s"  ${r.description}"))
		}

		for (cpu <- cpus) {
			val name = cpu.ident
		}
		//		game.instances.foreach(println)
		//		game.connections.foreach(println)
	}

}
