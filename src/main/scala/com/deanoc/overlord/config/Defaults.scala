package com.deanoc.overlord.config

import scala.collection.mutable

/**
 * This object implements a stack-based mechanism where keys are processed
 * with an unusual precedence rule: earlier keys in the stack take precedence 
 * over later ones. This is the opposite of the typical stack behavior where 
 * the most recently added (top) element is given priority. The logic ensures 
 * that the first keys pushed onto the stack maintain their dominance in 
 * determining outcomes or resolving conflicts.
 * 
 * E.g.
 * Defaults.push(Map("Bob" -> true))
 * Defaults.push(Map("Bob" -> false))
 * Defaults.flatten("Bob") // returns true
 */
object Defaults {
  private val stack: mutable.Stack[Map[String, Any]] = mutable.Stack()
  private val excludes: mutable.Set[String] = mutable.Set()
  private var cachedFlattened: Option[Map[String, Any]] = None
  // Make cacheValid accessible within the 'config' package
  private[config] var cacheValid: Boolean = false

  private def invalidateCache(): Unit = {
    cacheValid = false
    cachedFlattened = None
  }

  def push(map: Map[String, Any]): Unit = {
    stack.push(map)
    invalidateCache()
  }

  def pop(): Option[Map[String, Any]] = {
    val popped = if (stack.nonEmpty) Some(stack.pop()) else None
    invalidateCache()
    popped
  }

  def peek(): Option[Map[String, Any]] = stack.headOption

  def isEmpty: Boolean = stack.isEmpty

  def flatten: Map[String, Any] = {
    if (!cacheValid) {
      cachedFlattened = Some(computeFlatten())
      cacheValid = true
    }
    cachedFlattened.get
  }

  def contains(key: String): Boolean = flatten.contains(key)

  def apply(key: String): Any = 
    flatten.getOrElse(key, throw new NoSuchElementException(s"Key '$key' not found in Defaults"))

  private def computeFlatten(): Map[String, Any] = {
    val flattened = stack.foldRight(Map.empty[String, Any]) { (map, acc) =>
      val result = map ++ acc
      result
    }
    flattened.filterNot { case (key, _) => excludes.contains(key) }
  }

  def flatten(key: String): Any = flatten.getOrElse(key, null)
    
  def addExclude(key: String): Unit = {
    excludes.add(key)
    invalidateCache()
  }
  
  def removeExclude(key: String): Unit = {
    excludes.remove(key)
    invalidateCache()
  }
  
  def clearExcludes(): Unit = {
    excludes.clear()
    invalidateCache()
  }
  
  def clear(): Unit = {
    invalidateCache()
    stack.clear()
    excludes.clear()
  }
}
