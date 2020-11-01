package overlord

import overlord.cmdline._

object Main {
  def main(args: Array[String]): Unit = {
    Cmdline.go(args)
  }
}