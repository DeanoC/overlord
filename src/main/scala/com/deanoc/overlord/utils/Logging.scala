package com.deanoc.overlord.utils

import com.typesafe.scalalogging.LazyLogging
import org.slf4j.event.Level

// A singleton object to manage module-specific logging levels
object ModuleLogger {
  private val moduleLogLevels = scala.collection.mutable.Map[String, Level]()
  private var defaultLevel: Level = Level.INFO
  private var exitOnErrorEnabled: Boolean = false
  private var exitCode: Int = 1
  private var silentMode: Boolean = false // Flag to enable/disable all logging output
  
  // Set the logging level for a specific module
  def setModuleLogLevel(moduleName: String, level: Level): Unit = {
    moduleLogLevels.put(moduleName, level)
  }

  // Get the logging level for a specific module
  def getModuleLogLevel(moduleName: String): Level = {
    moduleLogLevels.getOrElse(moduleName, defaultLevel)
  }

  // Set the default logging level
  def setDefaultLogLevel(level: Level): Unit = {
    defaultLevel = level
  }

  // Enable or disable silent mode for testing
  def setSilentMode(silent: Boolean): Unit = {
    silentMode = silent
  }
  
  // Check if silent mode is enabled
  def isSilentMode(): Boolean = silentMode

  // Check if a specific logging level is enabled for a module
  def isLevelEnabled(moduleName: String, level: Level): Boolean = {
    if (silentMode) {
      // When in silent mode, don't log anything
      return false
    }
    
    val moduleLevel = getModuleLogLevel(moduleName)

    // Check if the requested level is less than or equal to the module's level
    // Level.TRACE(0) < Level.DEBUG(10) < Level.INFO(20) < Level.WARN(30) < Level.ERROR(40)
    level.toInt() >= moduleLevel.toInt()
  }

  // Configure whether to exit on error
  def setExitOnError(enabled: Boolean, code: Int = 1): Unit = {
    exitOnErrorEnabled = enabled
    exitCode = code
  }

  // Check if exit on error is enabled
  def isExitOnErrorEnabled(): Boolean = exitOnErrorEnabled

  // Get the exit code to use when exiting on error
  def getExitCode(): Int = exitCode

  // Method to exit the application
  def exitApplication(): Unit = {
    if (exitOnErrorEnabled) {
      System.exit(exitCode)
    }
  }
}

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
  // Get the module name from the class
  protected val moduleName: String = this.getClass.getName

  // Convenience methods that delegate to the underlying logger
  def trace(message: => String): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.TRACE)) {
      logger.trace(message)
    }
  }
  
  def debug(message: => String): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.DEBUG)) {
      logger.debug(message)
    }
  }
  
  def info(message: => String): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.INFO)) {
      logger.info(message)
    }
  }
  
  def warn(message: => String): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.WARN)) {
      logger.warn(message)
      // Note: warnings are automatically silenced by isLevelEnabled when in silent mode
    }
  }
  
  def error(message: => String): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.ERROR)) {
      logger.error(message)
      // Even in silent mode, we should respect exit on error
      if (!ModuleLogger.isSilentMode()) {
        ModuleLogger.exitApplication()
      }
    }
  }
  
  // Versions with interpolation parameters
  def trace(message: => String, arg: Any): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.TRACE)) {
      logger.trace(message, arg)
    }
  }
  
  def debug(message: => String, arg: Any): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.DEBUG)) {
      logger.debug(message, arg)
    }
  }
  
  def info(message: => String, arg: Any): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.INFO)) {
      logger.info(message, arg)
    }
  }
  
  def warn(message: => String, arg: Any): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.WARN)) {
      logger.warn(message, arg)
      // Note: warnings are automatically silenced by isLevelEnabled when in silent mode
    }
  }
  
  def error(message: => String, arg: Any): Unit = {
    if (ModuleLogger.isLevelEnabled(moduleName, Level.ERROR)) {
      logger.error(message, arg)
      // Even in silent mode, we should respect exit on error
      if (!ModuleLogger.isSilentMode()) {
        ModuleLogger.exitApplication()
      }
    }
  }
}
