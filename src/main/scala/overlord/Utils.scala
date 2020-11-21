package overlord

import java.io.{BufferedReader, BufferedWriter, FileReader, FileWriter}
import java.nio.file.Path

import toml.Value

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
		val s    = LazyList.continually(br.readLine())
			.takeWhile(_ != null)
			.mkString("\n")
		br.close()
		s
	}

	def toString(t: toml.Value): String = t match {
		case str: Value.Str => str.value
		case _              => println(s"ERR $t not a string"); "ERROR"
	}

	def toArray(t: Value): Seq[Value] = t match {
		case arr: Value.Arr => arr.values
		case _              => println(s"ERR $t not an Array"); Seq[Value]()
	}

	def toTable(t: Value): Map[String, Value] = t match {
		case tbl: Value.Tbl => tbl.values
		case _              => println(s"ERR $t not a Table"); Map[String, Value]()
	}

	def toBoolean(t: Value): Boolean = t match {
		case b: Value.Bool => b.value
		case _             => println(s"ERR $t not a bool"); false
	}

	def toInt(t: Value): Int = t match {
		case b: Value.Num => b.value.toInt
		case _            => println(s"ERR $t not a number"); 0
	}

	def lookupString(tbl: Map[String, Value], key: String, or: String): String =
		if (tbl.contains(key)) Utils.toString(tbl(key)) else or

	def lookupInt(tbl: Map[String, Value], key: String, or: Int): Int =
		if (tbl.contains(key)) Utils.toInt(tbl(key)) else or

}
