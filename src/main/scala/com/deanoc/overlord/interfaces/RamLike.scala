package com.deanoc.overlord.interfaces

trait RamLike extends ChipLike {
  // sequence of start and size in bytes with cpu filtering
  def getRanges: Seq[(BigInt, BigInt, Boolean, Seq[String])]
}
