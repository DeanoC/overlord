# Scala 3 Features for Overlord Refactoring

This document outlines how Scala 3 features can be leveraged to improve the Connections part of the Overlord project, which was originally written in Scala 2.

## Table of Contents

1. [Key Scala 3 Features](#key-scala-3-features)
2. [Detailed Refactoring Examples](#detailed-refactoring-examples)
3. [Addressing Code Smells with Scala 3](#addressing-code-smells-with-scala-3)
4. [Migration Considerations](#migration-considerations)
5. [Some Code Examples](#code-examples)

## Key Scala 3 Features

### Enums and ADTs

Scala 3 introduces a more concise and powerful enum syntax that can replace the verbose sealed trait + case class pattern commonly used in Scala 2.

```scala
// Scala 2
sealed trait ConnectionDirection
case object FirstToSecondConnection extends ConnectionDirection
case object SecondToFirstConnection extends ConnectionDirection
case object BiDirectionConnection extends ConnectionDirection

// Scala 3
enum ConnectionDirection:
  case FirstToSecond, SecondToFirst, BiDirection
```

### Extension Methods

Extension methods allow adding functionality to existing types without modifying their source code or using implicit classes.

```scala
// Scala 2
implicit class InstanceOps(instance: InstanceTrait) {
  def getMatchNameAndPort(nameToMatch: String): (Option[String], Option[Port]) = {
    // Implementation
  }
}

// Scala 3
extension (instance: InstanceTrait)
  def getMatchNameAndPort(nameToMatch: String): (Option[String], Option[Port]) = 
    // Implementation
```

### Opaque Type Aliases

Opaque types provide type safety without runtime overhead, making them perfect for domain modeling.

```scala
// Scala 3
object ConnectionTypes:
  opaque type ConnectionId = String
  
  object ConnectionId:
    def apply(value: String): ConnectionId = value
    extension (id: ConnectionId) def value: String = id
```

### Union Types

Union types allow expressing that a value can be one of several types, which is useful for handling different connection types.

```scala
// Scala 3
def processConnection(conn: Port | Bus | Logical): Unit = 
  conn match
    case p: Port => // handle port
    case b: Bus => // handle bus
    case l: Logical => // handle logical
```

### Contextual Abstractions

Scala 3 replaces implicits with a clearer given/using syntax for dependency injection.

```scala
// Scala 2
def connect(implicit matcher: ConnectionMatcher): Seq[Connected] = {
  // Implementation using matcher
}

// Scala 3
def connect(using matcher: ConnectionMatcher): Seq[Connected] = 
  // Implementation using matcher
```

### Match Types

Match types enable complex type-level pattern matching, useful for type-safe connection handling.

```scala
// Scala 3
type ConnectionResult[T] = T match
  case Port => PortConnection
  case Bus => BusConnection
  case _ => GenericConnection
```

### Indentation-based Syntax

Scala 3's optional braces-free syntax can make code more readable, especially for deeply nested match expressions.

```scala
// Scala 3
def process(value: Int): String =
  if value > 0 then
    "positive"
  else if value < 0 then
    "negative"
  else
    "zero"
```

### Improved Pattern Matching

Scala 3 offers more powerful pattern matching capabilities that simplify connection matching logic.

## Detailed Refactoring Examples

### Example 1: Refactoring Unconnected.apply Method

#### Current Code (Scala 2)

```scala
object Unconnected {
  def apply(connection: Variant): Option[UnconnectedLike] = {
    val table = Utils.toTable(connection)

    if (!table.contains("type")) {
      println(s"connection $connection requires a type field")
      return None
    }
    val conntype = Utils.toString(table("type"))

    if (!table.contains("connection")) {
      println(s"connection $conntype requires a connection field")
      return None
    }

    val cons = Utils.toString(table("connection"))
    val con = cons.split(' ')
    if (con.length != 3) {
      println(s"$conntype has an invalid connection field: $cons")
      return None
    }
    val (first, dir, secondary) = con(1) match {
      case "->"         => (con(0), FirstToSecondConnection(), con(2))
      case "<->" | "<>" => (con(0), BiDirectionConnection(), con(2))
      case "<-"         => (con(0), SecondToFirstConnection(), con(2))
      case _ =>
        println(s"$conntype has an invalid connection ${con(1)} : $cons")
        return None
    }

    conntype match {
      case "port"  => Some(UnconnectedPort(first, dir, secondary))
      case "clock" => Some(UnconnectedClock(first, dir, secondary))
      case "parameters" =>
        if (first != "_") None
        else
          Some(
            UnconnectedParameters(
              dir,
              secondary,
              Utils.lookupArray(table, "parameters")
            )
          )
      case "port_group" =>
        Some(
          UnconnectedPortGroup(
            first,
            dir,
            secondary,
            Utils.lookupString(table, "first_prefix", ""),
            Utils.lookupString(table, "second_prefix", ""),
            Utils.lookupArray(table, "excludes").toSeq.map(Utils.toString)
          )
        )
      case "bus" => {
        val supplierBusName = Utils.lookupString(table, "bus_name", "")
        val consumerBusName =
          Utils.lookupString(table, "consumer_bus_name", supplierBusName)
        Some(
          UnconnectedBus(
            first,
            dir,
            secondary,
            Utils.lookupString(table, "bus_protocol", "internal"),
            supplierBusName,
            consumerBusName,
            Utils.lookupBoolean(table, "silent", or = false)
          )
        )
      }
      case "logical" => Some(UnconnectedLogical(first, dir, secondary))
      case _ =>
        println(s"$conntype is an unknown connection type")
        None
    }
  }
}
```

#### Refactored Code (Scala 3)

```scala
// Define ADTs for connection types and errors
enum ConnectionType:
  case Port, Clock, Parameters, PortGroup, Bus, Logical

enum ConnectionError:
  case MissingField(fieldName: String, connection: Variant)
  case InvalidConnectionFormat(connectionStr: String)
  case InvalidDirectionSymbol(symbol: String, connectionStr: String)
  case UnknownConnectionType(typeName: String)
  case InvalidParametersFormat(first: String)

// Define a connection configuration
case class ConnectionConfig(
  first: String,
  direction: ConnectionDirection,
  second: String,
  additionalParams: Map[String, Variant] = Map.empty
)

object ConnectionParser:
  def parseConnectionType(typeStr: String): Either[ConnectionError, ConnectionType] =
    typeStr match
      case "port" => Right(ConnectionType.Port)
      case "clock" => Right(ConnectionType.Clock)
      case "parameters" => Right(ConnectionType.Parameters)
      case "port_group" => Right(ConnectionType.PortGroup)
      case "bus" => Right(ConnectionType.Bus)
      case "logical" => Right(ConnectionType.Logical)
      case _ => Left(ConnectionError.UnknownConnectionType(typeStr))
  
  def parseDirection(dirSymbol: String): Either[ConnectionError, ConnectionDirection] =
    dirSymbol match
      case "->" => Right(ConnectionDirection.FirstToSecond)
      case "<->" | "<>" => Right(ConnectionDirection.BiDirection)
      case "<-" => Right(ConnectionDirection.SecondToFirst)
      case _ => Left(ConnectionError.InvalidDirectionSymbol(dirSymbol, dirSymbol))
  
  def parseConnectionString(connectionStr: String): Either[ConnectionError, (String, ConnectionDirection, String)] =
    val parts = connectionStr.split(' ')
    if parts.length != 3 then
      Left(ConnectionError.InvalidConnectionFormat(connectionStr))
    else
      parseDirection(parts(1)).map((parts(0), _, parts(2)))

object Unconnected:
  def apply(connection: Variant): Either[ConnectionError, UnconnectedLike] =
    for
      // Extract and validate required fields
      table <- Right(Utils.toTable(connection))
      typeStr <- table.get("type").toRight(ConnectionError.MissingField("type", connection))
      connType <- parseConnectionType(Utils.toString(typeStr))
      connStr <- table.get("connection").toRight(ConnectionError.MissingField("connection", connection))
      (first, dir, second) <- parseConnectionString(Utils.toString(connStr))
      
      // Create the appropriate connection type
      result <- createConnection(connType, first, dir, second, table)
    yield result
  
  private def createConnection(
    connType: ConnectionType,
    first: String,
    dir: ConnectionDirection,
    second: String,
    table: Map[String, Variant]
  ): Either[ConnectionError, UnconnectedLike] =
    connType match
      case ConnectionType.Port => 
        Right(UnconnectedPort(first, dir, second))
      
      case ConnectionType.Clock => 
        Right(UnconnectedClock(first, dir, second))
      
      case ConnectionType.Parameters =>
        if first != "_" then 
          Left(ConnectionError.InvalidParametersFormat(first))
        else
          Right(UnconnectedParameters(
            dir,
            second,
            Utils.lookupArray(table, "parameters")
          ))
      
      case ConnectionType.PortGroup =>
        Right(UnconnectedPortGroup(
          first,
          dir,
          second,
          Utils.lookupString(table, "first_prefix", ""),
          Utils.lookupString(table, "second_prefix", ""),
          Utils.lookupArray(table, "excludes").toSeq.map(Utils.toString)
        ))
      
      case ConnectionType.Bus =>
        val supplierBusName = Utils.lookupString(table, "bus_name", "")
        val consumerBusName = Utils.lookupString(table, "consumer_bus_name", supplierBusName)
        Right(UnconnectedBus(
          first,
          dir,
          second,
          Utils.lookupString(table, "bus_protocol", "internal"),
          supplierBusName,
          consumerBusName,
          Utils.lookupBoolean(table, "silent", or = false)
        ))
      
      case ConnectionType.Logical =>
        Right(UnconnectedLogical(first, dir, second))
```

### Example 2: Refactoring Connection Logic with Extension Methods

#### Current Code (Scala 2)

```scala
trait ConnectedBetween extends Connected {
  val connectionPriority: ConnectionPriority
  val direction: ConnectionDirection

  override def connectedBetween(
      s: ChipInstance,
      e: ChipInstance,
      d: ConnectionDirection
  ): Boolean = {
    if (first.isEmpty || second.isEmpty) false
    else {
      d match {
        case FirstToSecondConnection() => (
          first.get.instance == s && second.get.instance == e
        )
        case SecondToFirstConnection() => (
          first.get.instance == e && second.get.instance == s
        )
        case BiDirectionConnection() => (
          (first.get.instance == s && second.get.instance == e) || (first.get.instance == e && second.get.instance == s)
        )
      }
    }
  }
}

case class ConnectedPortGroup(
    connectionPriority: ConnectionPriority,
    main: InstanceLoc,
    direction: ConnectionDirection,
    secondary: InstanceLoc
) extends Connected {
  // Other methods...

  override def connectedBetween(
      s: ChipInstance,
      e: ChipInstance,
      d: ConnectionDirection
  ): Boolean = {
    d match {
      case FirstToSecondConnection() => (
        main.instance == s && secondary.instance == e
      )
      case SecondToFirstConnection() => (
        main.instance == e && secondary.instance == s
      )
      case BiDirectionConnection() => (
        (main.instance == s && secondary.instance == e) || (main.instance == e && secondary.instance == s)
      )
    }
  }
}
```

#### Refactored Code (Scala 3)

```scala
// Define the core behavior once
trait Connected:
  def first: Option[InstanceLoc]
  def second: Option[InstanceLoc]
  def direction: ConnectionDirection

// Add the behavior to all Connected instances
extension (c: Connected)
  def connectedBetween(s: ChipInstance, e: ChipInstance, d: ConnectionDirection): Boolean =
    if c.first.isEmpty || c.second.isEmpty then false
    else
      d match
        case ConnectionDirection.FirstToSecond => 
          c.first.get.instance == s && c.second.get.instance == e
        case ConnectionDirection.SecondToFirst => 
          c.first.get.instance == e && c.second.get.instance == s
        case ConnectionDirection.BiDirection => 
          (c.first.get.instance == s && c.second.get.instance == e) || 
          (c.first.get.instance == e && c.second.get.instance == s)

// Simplified implementation that doesn't need to repeat the logic
case class ConnectedPortGroup(
  connectionPriority: ConnectionPriority,
  main: InstanceLoc,
  direction: ConnectionDirection,
  secondary: InstanceLoc
) extends Connected:
  override def first: Option[InstanceLoc] = Some(main)
  override def second: Option[InstanceLoc] = Some(secondary)
  // Other methods...
```

## Addressing Code Smells with Scala 3

### 1. Duplicate Connection Logic

**Code Smell**: Similar connection logic repeated across multiple classes.

**Scala 3 Solution**: Extension methods allow defining behavior once and applying it to all instances of a type.

```scala
extension (c: Connected)
  def connectedBetween(s: ChipInstance, e: ChipInstance, d: ConnectionDirection): Boolean =
    // Implementation that works for all Connected instances
```

### 2. Complex Conditional Logic

**Code Smell**: Complex nested conditionals and early returns.

**Scala 3 Solution**: Pattern matching, for-comprehensions with Either, and indentation-based syntax.

```scala
// Before (Scala 2)
if (!table.contains("type")) {
  println(s"connection $connection requires a type field")
  return None
}

// After (Scala 3)
for
  typeStr <- table.get("type").toRight(ConnectionError.MissingField("type", connection))
  // More processing
yield result
```

### 3. Inconsistent Error Handling

**Code Smell**: Mix of println statements and None returns.

**Scala 3 Solution**: ADTs for errors and Either for consistent error handling.

```scala
enum ConnectionError:
  case MissingField(fieldName: String, connection: Variant)
  case InvalidConnectionFormat(connectionStr: String)
  // More error types

def apply(connection: Variant): Either[ConnectionError, UnconnectedLike] =
  // Implementation using Either for error handling
```

### 4. Mutable State

**Code Smell**: Mutable collections and side effects.

**Scala 3 Solution**: Immutable collections with improved type inference.

```scala
// Before (Scala 2)
val wires = mutable.ArrayBuffer[Wire]()
wires ++= fanoutTmpWires.map { case (sl, (pr, els)) =>
  // Create wire
}

// After (Scala 3)
val fanoutWires = ghosts
  .groupBy(_.sloc)
  .collect {
    case (sl, gws) if gws.length > 1 => 
      Wire(sl, gws.map(_.eloc), gws.head.priority, sl.port.fold(true)(_.knownWidth))
  }
  .toSeq
```

### 5. YAML Processing Issues

**Code Smell**: YAML processing mixed with business logic.

**Scala 3 Solution**: Separation of concerns with dedicated parser objects and ADTs.

```scala
object ConnectionParser:
  def parseConnection(connection: Variant): Either[ConnectionError, UnconnectedLike] =
    // Parsing logic separated from business logic
```

## Migration Considerations

When migrating from Scala 2 to Scala 3 for the Overlord project, consider these steps:

1. **Update Build Configuration**: Update your build.sbt to use Scala 3.

```scala
scalaVersion := "3.2.2" // Or the latest Scala 3 version
```

2. **Incremental Migration**: Scala 3 allows for incremental migration. You can:
   - Start with the most problematic parts of the codebase
   - Refactor one module at a time
   - Use the `-source:3.0-migration` compiler flag to help with the transition

3. **Address Deprecated Features**: Some Scala 2 features are deprecated in Scala 3:
   - Replace implicit classes with extension methods
   - Update implicit parameters to use the given/using syntax
   - Replace type projections with type lambdas

4. **Leverage New Features**: Gradually introduce new Scala 3 features:
   - Start with enums for ADTs
   - Introduce extension methods for common functionality
   - Use indentation-based syntax for improved readability
   - Implement union types and match types where appropriate

5. **Testing Strategy**: Ensure comprehensive test coverage before and after migration to catch any issues.

By following these recommendations, you can successfully migrate the Overlord project to Scala 3 while improving code maintainability and readability.


# Code Examples
```scala
enum BusError:
  case UndirectedBusConnection(first: String, second: String)
  case MissingPortsInterface(instance: String)
  case NoMultiBusInterface(first: String, second: String)
  case NoSupportedBus(first: String, second: String, protocol: String)
  case NotChipInstance(instance: String)

def getBus(
  mainIL: InstanceLoc,
  otherIL: InstanceLoc,
  direction: ConnectionDirection
): Either[BusError, (SupplierBusLike, ChipInstance)] =
  // Check direction
  if direction == ConnectionDirection.BiDirection then
    return Left(BusError.UndirectedBusConnection(mainIL.fullName, otherIL.fullName))
  
  // Check ports interfaces
  for
    _ <- Either.cond(
      mainIL.instance.hasInterface[PortsLike], 
      (), 
      BusError.MissingPortsInterface(mainIL.fullName)
    )
    _ <- Either.cond(
      otherIL.instance.hasInterface[PortsLike], 
      (), 
      BusError.MissingPortsInterface(otherIL.fullName)
    )
    
    // Get multi buses based on direction
    (supplierInstance, consumerInstance) = direction match
      case ConnectionDirection.FirstToSecond => (mainIL.instance, otherIL.instance)
      case ConnectionDirection.SecondToFirst => (otherIL.instance, mainIL.instance)
      case _ => ??? // Already handled above
    
    supplierMultiBus <- supplierInstance.getInterface[MultiBusLike].toRight(
      BusError.NoMultiBusInterface(mainIL.fullName, otherIL.fullName)
    )
    
    // Get the appropriate bus
    supplierBus <- findSupplierBus(supplierMultiBus)
    
    // Ensure consumer is a chip instance
    consumer <- consumerInstance match
      case chip: ChipInstance => Right(chip)
      case _ => Left(BusError.NotChipInstance(
        if direction == ConnectionDirection.FirstToSecond then otherIL.fullName else mainIL.fullName
      ))
  yield (supplierBus, consumer)
```