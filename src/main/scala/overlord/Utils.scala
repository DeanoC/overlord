package overlord

import java.io.{BufferedReader, BufferedWriter, FileReader, FileWriter}
import java.nio.file.Path

object Utils {
	def ensureDirectories(path: Path): Unit = {
		val directory = path.toFile
		if (!directory.exists()) {
			directory.mkdirs()
		}
	}

	def writeFile(path: Path, s: String): Unit = {
		val file = path.toFile
		val bw   = new BufferedWriter(new FileWriter(file))
		bw.write(s)
		bw.close()
	}

	def readFile(path: Path): String = {
		val file = path.toFile
		val br   = new BufferedReader(new FileReader(file))
		val s    = Stream.continually(br.readLine())
			.takeWhile(_ != null)
			.mkString("\n")
		br.close()
		s
	}

	def toString(t: toml.Value): String = {
		if (t.isInstanceOf[toml.Value.Str])
			t.asInstanceOf[toml.Value.Str].value
		else s"ERROR $t isn't a string"
	}

}
