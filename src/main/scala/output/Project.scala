package output

import ikuy_utils.Utils
import overlord.Game

import java.nio.file.Path

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
//              generate_compilers.sh
//       gate
//

object Project {
	def apply(game: Game, out: Path): Unit = {
		println(s"Creating project at ${out.toRealPath()}")

		val softPath = out.resolve("soft")
		val gatePath = out.resolve("gate")

		Utils.ensureDirectories(gatePath)

		output.Report(game, out)
		output.Xdc(game, gatePath)
		output.Top(game, gatePath)
//		output.Edalize(game, gatePath)
		output.Software(game, softPath)
	}
}

object UpdateProject {
	def apply(game: Game, out: Path, instance: Option[String]): Unit = {
		instance match {
			case Some(inst) =>

			case None        =>
				// TODO for now just call create Project
				Project(game, out)
		}
	}

}
