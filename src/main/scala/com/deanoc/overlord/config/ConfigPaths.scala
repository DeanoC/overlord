package com.deanoc.overlord.config

import java.nio.file.{Files, Path, Paths}
import scala.collection.mutable
import com.deanoc.overlord.utils.Logging

object ConfigPaths extends Logging {
  private val catalogPathStack: mutable.Stack[Path] = mutable.Stack()
  private val instancePathStack: mutable.Stack[Path] = mutable.Stack()
  private val outPathStack: mutable.Stack[Path] = mutable.Stack()

  private var baseProjectPath: Path = Paths.get("")

  def getCatalogPathStackForTesting: mutable.Stack[Path] = catalogPathStack

  def resetPaths(): Unit = {
    catalogPathStack.clear()
    instancePathStack.clear()
    outPathStack.clear()
  }

  def setupPaths(projectPath: Path): Unit = {
    baseProjectPath = projectPath.toAbsolutePath
    resetPaths()

    catalogPathStack.push(projectPath)
    instancePathStack.push(projectPath)
    outPathStack.push(projectPath)

    info(s"Setting up paths with project path: $projectPath, all stacks are set to it")
  }

  def projectPath: Path = baseProjectPath

  def pushCatalogPath(path: String): Unit = pushCatalogPath(Paths.get(path))
  def pushCatalogPath(path: Path): Unit = {
    val pot = if (catalogPathStack.isEmpty) path else catalogPath.resolve(path)
    if (pot.toFile.isFile) catalogPathStack.push(pot.getParent)
    else catalogPathStack.push(pot)
  }
  def catalogPath: Path = catalogPathStack.top

  def setInstancePath(path: String): Unit = setInstancePath(Paths.get(path))
  def setInstancePath(path: Path): Unit = instancePathStack.push(path)

  def pushInstancePath(path: String): Unit = pushInstancePath(Paths.get(path))
  def pushInstancePath(path: Path): Unit = {
    val pot = instancePath.resolve(path)
    if (pot.toFile.isFile) instancePathStack.push(pot.getParent)
    else instancePathStack.push(pot)
  }
  def instancePath: Path = instancePathStack.top

  def pushOutPath(path: String): Unit = pushOutPath(Paths.get(path))
  def pushOutPath(path: Path): Unit = {
    val pot = outPath.resolve(path)
    if (pot.toFile.isFile) outPathStack.push(pot.getParent)
    else outPathStack.push(pot)
  }
  def outPath: Path = outPathStack.top

  def popCatalogPath(): Unit = if (catalogPathStack.nonEmpty) catalogPathStack.pop()
  def popInstancePath(): Unit = if (instancePathStack.nonEmpty) instancePathStack.pop()
  def popOutPath(): Unit = if (outPathStack.nonEmpty) outPathStack.pop()
}
