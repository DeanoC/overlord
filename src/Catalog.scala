package Overlord

case class Catalog(
    catalogPath: os.Path
):
  trait Item:
    def parent: Option[Item]
    def filePath: os.Path
    def depth: Int
    lazy val name = filePath.lastOpt

  case class File(
      parent: Option[Item],
      filePath: os.Path,
      depth: Int
  ) extends Item:
    override def toString(): String = parent match
      case None      => filePath.toString
      case Some(par) => s"\n${" " * depth} ${filePath.relativeTo(par.filePath)}"

  case class Folder(
      parent: Option[Item],
      filePath: os.Path,
      depth: Int
  ) extends Item:
    val contents: Array[Item] = os
      .list(filePath)
      .map(f =>
        if os.isDir(f) then Folder(Some(this), f, depth + 1)
        else File(Some(this), f, depth + 1)
      )
      .toArray

    override def toString(): String = {
      s"\n${parent match
          case None      => filePath.toString
          case Some(par) => " " * depth + filePath.relativeTo(par.filePath)
        }: ${{ contents.map(_.toString) }.mkString}"
    }.mkString

  val root = Folder(None, catalogPath, 0)

  println(root.toString())
