package actions

import ikuy_utils.{Utils, Variant}
import overlord.Game
import overlord.Instances.{InstanceTrait, ProgramInstance, SoftwareInstance}
import java.nio.file.{Path, Paths}

case class TemplateAction(override val phase: Int,
                          cpus: Seq[String],
													catalog_path: Path,
                          in: String,
                          out: String)
	extends Action() {
	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {
		var ifn = in
		var ofn = out

		val cpuName = if (phase == 1) "" else if (cpus.nonEmpty) {
			val cpuName = Utils.toString(parameters("${cpuName}"))
			if (!cpus.contains(cpuName)) return
			cpuName
		} else Utils.toString(parameters("${cpuName}"))

		for ((k, v) <- parameters) {
			ifn = ifn.replace(s"$k", v.toCString)
			ofn = ofn.replace(s"$k", v.toCString)
		}

		val iPath = catalog_path.resolve(ifn)

		val source = Utils.readFile(iPath)
		if (source.isEmpty) {
			println(f"Template source file $iPath not found%n")
			return
		}

		var sourceString = source.get

		for ((k, v) <- parameters) {
			sourceString = sourceString.replace(s"$k", v.toCString)
		}

		val oPath = instance match {
			case si: SoftwareInstance =>
				val folder = if (instance.isInstanceOf[ProgramInstance]) {
					if (cpuName.nonEmpty) s"${si.folder}_$cpuName"
					else si.folder
				} else si.folder

				Game.outPath
					.resolve(folder)
					.resolve(si.name.replace('.','/'))
					.resolve(ofn)
			case _                    =>
				Game.outPath.resolve(ofn)
		}
		Utils.ensureDirectories(oPath.getParent)
		Utils.writeFile(oPath, sourceString)
	}
}

object TemplateAction {
	private val cpuRegEx = "\\s*,\\s*".r

	def apply(name: String, process: Map[String, Variant]): Seq[TemplateAction] = {
		if (!process.contains("sources")) {
			println(s"Template process $name doesn't have a sources field")
			return Seq()
		}
		val srcs = Utils.toArray(process("sources")).map(Utils.toTable)

		val all_cpus = if (process.contains("cpus")) {
			val cpusString = Utils.toString(process("cpus"))
			if (cpusString == "_") Some(Seq())
			else Some(cpuRegEx.split(cpusString).toSeq)
		} else None

		for (entry <- srcs.toIndexedSeq) yield {
			val (phase, cpus) =
				if (entry.contains("cpus")) {
					val cpusString = Utils.toString(entry("cpus"))
					if (cpusString == "_") (2, Seq())
					else {
						val cpus = cpuRegEx.split(cpusString)
						if (all_cpus.isDefined) (2, cpus.intersect(all_cpus.get).toSeq.map(_.toLowerCase()))
						else (2, cpus.toSeq)
					}
				} else if (all_cpus.isDefined) (2, all_cpus.get)
				else (1, Seq())

			val inFilename  = Utils.toString(entry("in"))
			val outFilename = Utils.toString(entry("out"))

			TemplateAction(phase,
			               cpus,
										 Game.catalogPath,
			               inFilename,
			               outFilename)
		}

	}
}