package Overlord

import scala.collection.IndexedSeqView.Id

case class Identifier(id: Seq[String]):
  def +:(name: String): Identifier = Identifier(name +: id)
  def contains(name: String): Boolean = id.contains(name)
  override def toString(): String = id.mkString(".")

case class Catalog(
    catalogPath: os.Path
):

  trait Item:
    def parent: Option[Item]
    def filePath: os.Path
    def depth: Int
    lazy val name = filePath.last.split('.').head
    lazy val ident: Identifier = identInternal(Identifier(Seq()))
    protected def identInternal(id: Identifier): Identifier = parent match
      case None        => id
      case Some(value) => value.identInternal(name +: id)

    def ==(matcher: Identifier): Boolean = ident == matcher
    def deepestMatch(matcher: Identifier): Option[Identifier]
    def fetch(matcher: Identifier): Option[Item]

  case class File(
      parent: Option[Item],
      filePath: os.Path,
      depth: Int
  ) extends Item:
    override def toString(): String = parent match
      case None => filePath.toString
      case Some(par) =>
        s"\n${" " * depth} ${ident.id.mkString(".")} (${filePath.relativeTo(par.filePath)})"
    override def deepestMatch(matcher: Identifier): Option[Identifier] =
      if ident == matcher then Some(ident) else None
    override def fetch(matcher: Identifier): Option[Item] = if ident == matcher then Some(this) else None

  case class Folder(
      parent: Option[Item],
      filePath: os.Path,
      depth: Int
  ) extends Item:
    lazy val contents: Array[Item] = os
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

    override def deepestMatch(matcher: Identifier): Option[Identifier] =
      var result: Option[Identifier] = if ident == matcher then Some(ident) else None
      for
        id <- contents
        if result.isEmpty
      do result = id.deepestMatch(matcher)
      result
    override def fetch(matcher: Identifier): Option[Item] =
      var result: Option[Item] = if ident == matcher then Some(this) else None
      for
        id <- contents
        if result.isEmpty
      do result = id.fetch(matcher)
      result

  val root = Folder(None, catalogPath, 0)

  def matchIdentifier(toMatch: Identifier): Option[Identifier] = root.deepestMatch(toMatch)
  def fetch(id: Identifier): Option[Item] = root.fetch(id)
