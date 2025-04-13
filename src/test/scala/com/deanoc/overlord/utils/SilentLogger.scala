package com.deanoc.overlord.utils

/**
 * Helper trait for tests that need to silence log output.
 * Use this when testing code that would normally produce expected errors or warnings.
 *
 * Usage:
 * {{{
 *   class MyTest extends AnyFlatSpec with SilentLogger {
 *     it should "handle error cases silently" in {
 *       withSilentLogs {
 *         // Code that will generate error/warning logs
 *         // Logs will be suppressed but assertions will still work
 *       }
 *     }
 *   }
 * }}}
 */
trait SilentLogger {
  /**
   * Executes the given block with logging silenced.
   * Logs will be re-enabled and shown if the test fails.
   * 
   * @param block The code block to execute with silent logging
   * @tparam T The return type of the block
   * @return The result of the block execution
   */
  def withSilentLogs[T](block: => T): T = {
    val wasSilent = ModuleLogger.isSilentMode()
    try {
      ModuleLogger.setSilentMode(true)
      block
    } catch {
      case e: Throwable =>
        // If an exception occurs (test failure), restore logging to see what happened
        ModuleLogger.setSilentMode(false)
        println("Test failed - showing logs for debugging")
        throw e
    } finally {
      // Only reset to previous value if no exception was thrown
      if (!Thread.currentThread.isInterrupted) {
        ModuleLogger.setSilentMode(wasSilent)
      }
    }
  }
}
