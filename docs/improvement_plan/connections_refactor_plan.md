# Connections Module Refactoring Plan

# test code lives in 
overlord/src/test/scala/com/deanoc/overlord
# main code lives 
overlord/src/main/scala/com/deanoc/overlord

## Goals
- Improve code maintainability and readability
- Apply Scala 3 features and idioms where appropriate
- Fix package declaration inconsistency (lowercase "connections" directory vs. uppercase "Connections" package)
- Ensure backward compatibility (no breaking changes)
- Add proper tests to verify behavior

## Files to Refactor
1. Connected.scala
2. ConnectedBus.scala
3. ConnectedLogical.scala
4. ConnectedBetween.scala
5. ConnectedConstant.scala
6. ConnectedPortGroup.scala
7. Unconnected.scala
8. UnconnectedPort.scala
9. UnconnectedPortGroup.scala
10. UnconnectedBus.scala
11. UnconnectedLogical.scala
12. UnconnectedClock.scala
13. UnconnectedParameters.scala
14. Wire.scala

## Scala 3 Improvements to Apply
1. Replace traits with `enum` where appropriate (e.g., ConnectionPriority)
2. Use `extension methods` for related functionality
3. Apply `opaque types` for type safety
4. Use `given instances` instead of implicit parameters
5. Replace pattern matching with more concise expressions
6. Apply indentation-based syntax for better readability
7. Use `export` clauses to reduce import clutter
8. Apply `union types` where appropriate
9. Use `optional braces` for cleaner code
10. Apply `transparent traits` where applicable

## Refactoring Steps
2. Convert ConnectionPriority to an enum
   - Replace the sealed trait and case classes with a proper Scala 3 enum
   
3. Improve type safety
   - Apply opaque types for domain-specific concepts
   - Ensure proper type checking
   
4. Apply extension methods
   - Move related functionality to extension methods
   - Group related functionality together
   
5. Improve error handling
   - Use proper error propagation
   - Add validation methods
   
6. Clean up code style
   - Use consistent formatting
   - Apply indentation-based syntax where appropriate
   - Remove unnecessary code
   
7. Update tests
   - Ensure tests verify all refactoring changes
   - Add tests for edge cases

## Code Style Improvements
1. Consistent naming conventions
2. Better documentation (scaladoc)
3. Remove redundant or unused code
4. Improve error messages
5. Better type safety

## Testing Strategy
1. Create tests that verify current behavior before refactoring
2. Add tests for edge cases and error conditions
3. Run tests after each refactoring step
4. Run sbt test after each code change to ensure edits compile
5. Only move onto the next test when the one we are working on passes
6. When testing a particular test file, use testOnly syntax to sbt test
7. Focus on behavior verification rather than implementation details
