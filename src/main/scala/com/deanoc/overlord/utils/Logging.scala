package com.deanoc.overlord.utils

import com.typesafe.scalalogging.LazyLogging

/** Trait that provides logging capabilities to any class that mixes it in.
  *
  * Usage:
  * {{{
  *   class MyClass extends Logging {
  *     def doSomething(): Unit = {
  *       info("Starting operation")
  *       // ...
  *       if (problem) {
  *         error("Something went wrong", exception)
  *       }
  *       debug("Operation details: {}", details)
  *     }
  *   }
  * }}}
  */
trait Logging extends LazyLogging {
  // Convenience methods that delegate to the underlying logger
  def debug(message: => String): Unit = logger.debug(message)
  def debug(message: => String, cause: Throwable): Unit =
    logger.debug(message, cause)
  def debug(message: => String, args: Any*): Unit =
    logger.debug(message, args: _*)

  def info(message: => String): Unit = logger.info(message)
  def info(message: => String, cause: Throwable): Unit =
    logger.info(message, cause)
  def info(message: => String, args: Any*): Unit =
    logger.info(message, args: _*)

  def warn(message: => String): Unit = logger.warn(message)
  def warn(message: => String, cause: Throwable): Unit =
    logger.warn(message, cause)
  def warn(message: => String, args: Any*): Unit =
    logger.warn(message, args: _*)

  def error(message: => String): Unit = logger.error(message)
  def error(message: => String, cause: Throwable): Unit =
    logger.error(message, cause)
  def error(message: => String, args: Any*): Unit =
    logger.error(message, args: _*)

  def trace(message: => String): Unit = logger.trace(message)
  def trace(message: => String, cause: Throwable): Unit =
    logger.trace(message, cause)
  def trace(message: => String, args: Any*): Unit =
    logger.trace(message, args: _*)
}
