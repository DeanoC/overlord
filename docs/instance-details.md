# Instance Documentation

This document provides an overview of the `instance` section in the Overlord system, detailing its structure, usage, and examples.

## Structure of an Instance

```yaml
instance:
  - name: instanceName
    type: component.type  # e.g., cpu.riscv, memory.ram, etc.
    # Optional parameters for the instance
    parameter1: value1
    parameter2: value2
```

## Explanation of Items

### `name`
- **Description**: The unique identifier for the instance.
- **Type**: String
- **Example**:
  ```yaml
  name: cpu0
  ```

### `type`
- **Description**: Specifies the type of the instance, which corresponds to a definition in the catalog.
- **Type**: String
- **Example**:
  ```yaml
  type: cpu.riscv
  ```

### Parameters (Optional)
- **Description**: Additional configuration options for the instance.
- **Type**: Key-value pairs
- **Example**:
  ```yaml
  parameter1: value1
  parameter2: value2
  ```

## Notes
- Each instance must have a unique `name`.
- The `type` field must match a valid definition in the catalog.
- Parameters are optional but can be used to customize the instance's behavior.

## Example

```yaml
instance:
  - name: cpu0
    type: cpu.riscv
    clockFrequency: 200000000  # Override default

  - name: ram0
    type: memory.ram
    size: 0x10000  # 64KB

  - name: uart0
    type: peripheral.uart
    baudRate: 115200
```

## Supported Instance Types

### `CpuInstance`
- **Description**: Represents a CPU component in the system.
- **Attributes**:
  - `triple`: The target triple for the CPU.
  - `maxAtomicWidth`: Maximum width for atomic operations.
  - `core_count`: Number of cores in the CPU.
- **Example**:
  ```yaml
  instance:
    - name: cpu0
      type: cpu.riscv
      triple: riscv64-unknown-elf
      core_count: 4
  ```

### `RamInstance`
- **Description**: Represents a RAM component in the system.
- **Attributes**:
  - `ranges`: Memory ranges for the RAM.
  - `fixed_address`: Whether the address is fixed.
- **Example**:
  ```yaml
  instance:
    - name: ram0
      type: memory.ram
      ranges:
        - address: 0x80000000
          size: 0x10000
  ```

### `IoInstance`
- **Description**: Represents an I/O component in the system.
- **Attributes**:
  - `visible_to_software`: Whether the I/O is visible to software.
- **Example**:
  ```yaml
  instance:
    - name: uart0
      type: peripheral.uart
      baudRate: 115200
  ```

### `ClockInstance`
- **Description**: Represents a clock component in the system.
- **Attributes**:
  - `pin`: The pin associated with the clock.
  - `frequency`: The frequency of the clock.
- **Example**:
  ```yaml
  instance:
    - name: clk0
      type: clock
      frequency: 100MHz
  ```

### `PinGroupInstance`
- **Description**: Represents a group of pins in the system.
- **Attributes**:
  - `pins`: List of pins in the group.
  - `direction`: Direction of the pins (e.g., input, output).
- **Example**:
  ```yaml
  instance:
    - name: gpio0
      type: pingroup
      pins: [pin1, pin2, pin3]
      direction: output
  ```

### `BoardInstance`
- **Description**: Represents a board in the system.
- **Attributes**:
  - `board_type`: Type of the board (e.g., Xilinx, Altera).
  - `clocks`: List of clocks on the board.
- **Example**:
  ```yaml
  instance:
    - name: board0
      type: board
      board_type: Xilinx
      clocks:
        - name: clk0
          frequency: 100MHz
  ```