# Definition Types Documentation

This document provides an overview of the various `DefinitionType` categories in the Overlord system, detailing their structure, purpose, and examples.

## Overview
`DefinitionType` is a sealed trait that represents different types of definitions in the Overlord system. These are categorized into three main groups:

- **ChipDefinitionType**: Represents hardware components.
- **PortDefinitionType**: Represents ports or connections.
- **SoftwareDefinitionType**: Represents software components.

## Chip Definition Types

### `RamDefinitionType`
- **Description**: Represents RAM components.
- **Attributes**: Memory ranges, fixed address.
- **Example**:
  ```yaml
  type: ram
  ranges:
    - address: 0x80000000
      size: 0x10000
  ```

### `CpuDefinitionType`
- **Description**: Represents CPU components.
- **Attributes**: Core count, target triple.
- **Example**:
  ```yaml
  type: cpu
  core_count: 4
  triple: riscv64-unknown-elf
  ```

### `GraphicDefinitionType`
- **Description**: Represents graphic processing units.
- **Attributes**: TBD.

### `StorageDefinitionType`
- **Description**: Represents storage components.
- **Attributes**: TBD.

### `NetDefinitionType`
- **Description**: Represents network components.
- **Attributes**: TBD.

### `IoDefinitionType`
- **Description**: Represents input/output components.
- **Attributes**: Visibility to software.
- **Example**:
  ```yaml
  type: io
  visible_to_software: true
  ```

### `SocDefinitionType`
- **Description**: Represents System-on-Chip components.
- **Attributes**: TBD.

### `SwitchDefinitionType`
- **Description**: Represents switch components.
- **Attributes**: TBD.

### `OtherDefinitionType`
- **Description**: Represents other miscellaneous components.
- **Attributes**: TBD.

## Port Definition Types

### `PinGroupDefinitionType`
- **Description**: Represents groups of pins.
- **Attributes**: Pin list, direction.
- **Example**:
  ```yaml
  type: pingroup
  pins: [pin1, pin2, pin3]
  direction: output
  ```

### `ClockDefinitionType`
- **Description**: Represents clock components.
- **Attributes**: Frequency, associated pin.
- **Example**:
  ```yaml
  type: clock
  frequency: 100MHz
  ```

## Software Definition Types

### `ProgramDefinitionType`
- **Description**: Represents software programs.
- **Attributes**: Dependencies, parameters.
- **Example**:
  ```yaml
  type: program
  dependencies:
    - library1
    - library2
  ```

### `LibraryDefinitionType`
- **Description**: Represents software libraries.
- **Attributes**: Dependencies, parameters.
- **Example**:
  ```yaml
  type: library
  dependencies:
    - corelib
  ```

## Board Definition Type

### `BoardDefinitionType`
- **Description**: Represents physical hardware boards.
- **Attributes**: Board type, clocks.
- **Example**:
  ```yaml
  type: board
  board_type: Xilinx
  clocks:
    - name: clk0
      frequency: 100MHz
  ```