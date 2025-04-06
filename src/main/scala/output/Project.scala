package output

import overlord.Project as OverlordProject

// This object is responsible for generating the actual project file structure
// based on the provided overlord.Project instance. It organizes the output
// into directories and files for both hardware ("gate") and software ("soft") components.

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
	def apply(game: OverlordProject): Unit = {
		val out = OverlordProject.outPath
		println(s"Creating project at ${out.toRealPath()}")

		output.Report(game)

		OverlordProject.pushOutPath("gate")
		output.Xdc(game)
		output.Top(game)
		output.Edalize(game)
		OverlordProject.popOutPath()

		OverlordProject.pushOutPath("soft")
		output.Software(game)
		OverlordProject.popOutPath()
	}
}

object UpdateProject {
	def apply(game: OverlordProject, instance: Option[String]): Unit = {
		instance match {
			case Some(inst) =>

			case None =>
				// TODO for now just call create Project
				Project(game)
		}
	}
}
