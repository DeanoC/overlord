package output

import overlord.Game

// Projects structure
// build
//       soft
//              src
//                 $triple
//                     hw
//                        include
//                               registers
//                        src
//                        CMakeLists.txt
//                     core
//                        include
//                               core
//                        src
//                        CMakeLists.txt
//              etc
//                     CMSIS-SVD.xsd
//              compilers
//              ${cpu}_toolchain.cmake
//              ${gamename}.svd
//              make_compilers.sh
//       gate
//

object Project {
	def apply(game: Game): Unit = {
		val out = Game.outPath
		println(s"Creating project at ${out.toRealPath()}")

		output.Report(game)

		Game.pushOutPath("gate")
		output.Xdc(game)
		output.Top(game)
		output.Edalize(game)
		Game.popOutPath()

		Game.pushOutPath("soft")
		output.Software(game)
		Game.popOutPath()
	}
}

object UpdateProject {
	def apply(game: Game, instance: Option[String]): Unit = {
		instance match {
			case Some(inst) =>

			case None =>
				// TODO for now just call create Project
				Project(game)
		}
	}

}
