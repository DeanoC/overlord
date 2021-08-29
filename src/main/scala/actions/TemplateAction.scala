package actions

import ikuy_utils.{Utils, Variant}
import overlord.Game
import overlord.Instances.{InstanceTrait, ProgramInstance, SoftwareInstance}

import java.nio.file.Path

case class TemplateAction(override val phase: Int,
                          cpus: Seq[String],
                          srcPath: Path,
                          in: String,
                          out: String,
                          pathOp: ActionPathOp)
	extends Action() {
	override def execute(instance: InstanceTrait,
	                     parameters: Map[String, () => Variant],
	                     outPath: Path): Unit = {
		var ifn = in
		var ofn = out

		val cpuName = if (phase == 1) "" else if (cpus.nonEmpty) {
			val cpuName = Utils.toString(parameters("${cpuName}")())
			if (!cpus.contains(cpuName)) return
			cpuName
		} else Utils.toString(parameters("${cpuName}")())

		for ((k, v) <- parameters) {
			ifn = ifn.replace(s"$k", v().toCString)
			ofn = ofn.replace(s"$k", v().toCString)
		}

		val source = Utils.readFile(ifn, srcPath.resolve(ifn), getClass)
		if (source.isEmpty) {
			println(f"Template source file $ifn not found%n")
			return
		}

		var sourceString = source.get

		for ((k, v) <- parameters) {
			sourceString = sourceString.replace(s"$k", v().toCString)
		}

		val oPath = instance match {
			case si: SoftwareInstance =>
				val folder = if (instance.isInstanceOf[ProgramInstance]) {
					if (cpuName.nonEmpty) s"${si.folder}_$cpuName"
					else si.folder
				} else si.folder

				outPath
					.resolve(folder)
					.resolve(si.name)
					.resolve(ofn)
			case _                    =>
				outPath.resolve(ofn)
		}
		Utils.ensureDirectories(oPath.getParent)
		Utils.writeFile(oPath, sourceString)

		updatePath(oPath.getParent)
	}
}

object TemplateAction {
	private val cpuRegEx = "\\s*,\\s*".r

	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Seq[TemplateAction] = {
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

		for (entry <- srcs) yield {
			val (phase, cpus) =
				if (entry.contains("cpus")) {
					val cpusString = Utils.toString(entry("cpus"))
					if (cpusString == "_") (2, Seq())
					else {
						val cpus = cpuRegEx.split(cpusString)
						if (all_cpus.isDefined) (2, cpus.intersect(all_cpus.get).toSeq)
						else (2, cpus.toSeq)
					}
				} else if (all_cpus.isDefined) (2, all_cpus.get)
				else (1, Seq())

			val inFilename  = Utils.toString(entry("in"))
			val outFilename = Utils.toString(entry("out"))

			TemplateAction(phase,
			               cpus,
			               Game.pathStack.top,
			               inFilename,
			               outFilename,
			               pathOp)
		}

	}
}