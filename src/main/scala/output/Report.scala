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
			val chipType = cput.chipType.split('.')(1)
			val arch     = cput.attributes("arch").asInstanceOf[Value.Str].value
			val bw       = cput
				.attributes("width")
				.asInstanceOf[Value.Num]
				.value
				.toInt
			println(s"${chipType} are ${bw} bit $arch CPUs")
			cput.software match {
				case Some(sw) =>
					sw.registers.foreach(r => println(s"  ${r.description}"))
				case None     =>
			}
		}
		for (cpu <- cpus) {
			val name = cpu.ident
		}
		//		game.instances.foreach(println)
		//		game.connections.foreach(println)
	}

}
