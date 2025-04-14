package com.deanoc.overlord

import scala.reflect.ClassTag

trait QueryInterface {
  def hasInterface[T](implicit tag: ClassTag[T]): Boolean =
    getInterface[T](tag).nonEmpty

  def getInterface[T](implicit tag: ClassTag[T]): Option[T] = None

  }
