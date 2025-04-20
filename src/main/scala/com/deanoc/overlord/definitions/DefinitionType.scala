package com.deanoc.overlord.definitions

import com.deanoc.overlord.utils.Logging

/**
 * Represents the type of a definition in the Overlord system.
 * Refactored to use Scala 3's enum system.
 */
enum DefinitionType:
  // Chip definitions
  case RamDefinition(ident: Seq[String])
  case CpuDefinition(ident: Seq[String])
  case GraphicDefinition(ident: Seq[String])
  case StorageDefinition(ident: Seq[String])
  case NetDefinition(ident: Seq[String])
  case IoDefinition(ident: Seq[String])
  case OtherDefinition(ident: Seq[String])
  case SocDefinition(ident: Seq[String])
  case SwitchDefinition(ident: Seq[String])
  
  // Board definition
  case BoardDefinition(ident: Seq[String])
  
  // Port definitions
  case PinGroupDefinition(ident: Seq[String])
  case ClockDefinition(ident: Seq[String])
  
  // Software definitions
  case ProgramDefinition(ident: Seq[String])
  case LibraryDefinition(ident: Seq[String])
  
  // Component definition
  case ComponentDefinition(ident: Seq[String])
  
  // All definition types have an identifier
  def ident: Seq[String]

/**
 * Companion object for DefinitionType with factory methods and extensions
 */
object DefinitionType extends Logging:
  /**
   * Categories for definition types
   */
  enum Category:
    case Chip, Port, Software, Board, Component
  
  /**
   * Get the category of a definition type
   */
  def categoryOf(defType: DefinitionType): Category = defType match
    case _: RamDefinition | _: CpuDefinition | _: GraphicDefinition |
         _: StorageDefinition | _: NetDefinition | _: IoDefinition |
         _: OtherDefinition | _: SocDefinition | _: SwitchDefinition => 
      Category.Chip
    
    case _: PinGroupDefinition | _: ClockDefinition => 
      Category.Port
    
    case _: ProgramDefinition | _: LibraryDefinition => 
      Category.Software
    
    case _: BoardDefinition => 
      Category.Board
    
    case _: ComponentDefinition => 
      Category.Component
  
  /**
   * Creates a DefinitionType from a string representation.
   * Uses Scala 3's enhanced pattern matching capabilities.
   *
   * @param in The string representation of the definition type
   * @return The corresponding DefinitionType
   */
  def apply(in: String): DefinitionType =
    val defTypeName = in.split('.')
    val tt = defTypeName.map(_.toLowerCase).toSeq
    
    defTypeName.headOption.map(_.toLowerCase) match
      case Some("ram")       => RamDefinition(tt)
      case Some("cpu")       => CpuDefinition(tt)
      case Some("storage")   => StorageDefinition(tt)
      case Some("graphic")   => GraphicDefinition(tt)
      case Some("net")       => NetDefinition(tt)
      case Some("io")        => IoDefinition(tt)
      case Some("board")     => BoardDefinition(tt)
      case Some("soc")       => SocDefinition(tt)
      case Some("switch")    => SwitchDefinition(tt)
      case Some("other")     => OtherDefinition(tt)
      
      case Some("pin") | Some("pingroup") => PinGroupDefinition(tt)
      case Some("clock")                  => ClockDefinition(tt)
      
      case Some("program")   => ProgramDefinition(tt)
      case Some("library")   => LibraryDefinition(tt)
      case Some("component") => ComponentDefinition(tt)
      
      case Some(unknown) =>
        warn(s"Unknown definition type: $unknown")
        OtherDefinition(tt)
      
      case None =>
        error(s"Empty definition type string")
        OtherDefinition(tt)

  // Extension methods for common operations
  extension (defType: DefinitionType)
    /**
     * Gets the category of this definition type.
     */
    def category: Category = categoryOf(defType)
    
    /**
     * Checks if this definition type is a chip definition.
     */
    def isChipDefinition: Boolean = category == Category.Chip
    
    /**
     * Checks if this definition type is a port definition.
     */
    def isPortDefinition: Boolean = category == Category.Port
    
    /**
     * Checks if this definition type is a software definition.
     */
    def isSoftwareDefinition: Boolean = category == Category.Software
    
    /**
     * Checks if this definition type is a board definition.
     */
    def isBoardDefinition: Boolean = category == Category.Board
    
    /**
     * Checks if this definition type is a component definition.
     */
    def isComponentDefinition: Boolean = category == Category.Component
    
    /**
     * Gets the type name (first part of the identifier).
     */
    def typeName: String = defType.ident.headOption.getOrElse("")
