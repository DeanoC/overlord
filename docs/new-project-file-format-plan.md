# Overlord Project/Component File Format (.yaml)

## Introduction

The Overlord framework uses YAML-based configuration files with the `.yaml` extension to define projects, components, and their connections. These files are the primary input for the Overlord.Project class and define how hardware and software components are instantiated, configured, and connected.

Projects can consist of subprojects known as components, creating a hierarchical structure. A top-level component is considered a Project. Structurally they are the same, allowing reuse and composition as projects grow larger.

## File Format

Overlord project files are written in YAML and have the `.yaml` extension.

## Main Sections

A typical `.yaml` file consists of the following main sections:

### 1. Info/Description

Project and Components *must* have an info section with at least a name, which is how this component will be instantiated. If the name doesn't start with "component.", this prefix will be assumed.

```yaml
info:
  name: "deano.testproject"
  version: "1.0.0"
  author: "Your Name"
  description: "Project description"
```

### 2. Catalogs

Catalogs contain sets of definitions that can be used to create instances within this project. Catalogs can include their own catalogs.
  
```yaml
catalogs:
  - type: "git"
    url: "https://github.com/example/hardware-catalog.git"
  - type: "local"
    path: "/path/to/local/catalog"
  - type: "fetch"
    url: "https://example.com/api/catalog"
```

### 3. Components

References pre-configured instances and connections that can be added to a project with a single name. Components are structurally the same as Projects but require an info.name, which allows parts to be reused.

```yaml
components:
  - type: local
    path: /home/deano/local_prefab.yaml
  - type: fetch
    url: https://gagameos.com/gagameos_base_prefab.yaml
  - type: git
    url: https://github.com/DeanoC/bl616.git
```

### 4. Defaults

Optional section for specifying default values that will be applied to instances if specific values are not provided.

```yaml
defaults:
  target_frequency: "100MHz"
```

### 5. Instances

Defines components to be instantiated in the project.
They can be definitions included from catalogs, project components, or 'one-shot' instances with embedded definitions.

```yaml
instances:
  - name: instanceName
    type: component.type  # e.g., cpu.riscv.test, ram.ddr4.test, component.deano.testproject etc.
    config:
      # Optional parameters for the instance
      parameter1: value1
      parameter2: value2
```

### 6. Connections

Defines connections between instances, such as buses, signals, or other interfaces. Connections have a `type` and a `connection` string, along with type-specific parameters.

```yaml
connections:
  # Bus connections
  - type: "bus"
    connection: "instanceName1 -> instanceName2"
    bus_protocol: "axi4" # Optional
    bus_name: "bus_name" # Optional
    silent: true # Optional

  # Port connections
  - type: "port"
    connection: "instanceName1.port_name -> instanceName2.port_name"

  # Port group connections
  - type: "port_group"
    connection: "instanceName1.group_name -> instanceName2.group_name"
    first_prefix: "prefix1_" # Optional
    second_prefix: "prefix2_" # Optional
    excludes: ["port_to_exclude"] # Optional

  # Clock connections
  - type: "clock"
    connection: "clock_source_instance -> clock_sink_instance.clock_port"

  # Logical connections
  - type: "logical"
    connection: "instanceName1.signal_name -> instanceName2.signal_name"

  # Parameters connections
  - type: "parameters"
    connection: "instanceName1 -> instanceName2"
    parameters:
      - name: "parameter_name"
        value: "parameter_value"
        type: "parameter_type" # e.g., "integer", "boolean", "string"

  # Constant connections
  - type: "constant"
    connection: "instanceName.parameter_name"
    value: "constant_value"
```
### 7. Definitions
```yaml
definitions:
  - name: "Cortex-A53"
    type: "cpu"
    config:
      core_count: 4
      triple: "aarch64-none-elf"
  
  - name: "MainMemory"
    type: "ram"
    config:
      ranges:
        - address: "0x80000000"
          size: "0x40000000"
        - address: "0xC0000000" 
          size: "0x20000000"
  
  - name: "SystemClock"
    type: "clock"
    config:
      frequency: "100MHz"
```
## Processing Order

The Overlord.Project class processes these sections in a specific order:

1. Process `info` section
2. Process `defaults` section
3. Process `catalogs` section
4. Process `definitions` section
5. Process `instances` sections to create component instances
6. Process `connections` sections to connect those instances
7. Process `components` section

## Special Macros

When defining paths or other string values, you can use special macros that will be resolved by the Project class:

- `${name}` - Replaced with the instance name
- `${projectPath}` - Replaced with the project path
- `${definitionPath}` - Replaced with the instance definition path
- `${outPath}` - Replaced with the configured output path
- `${instancePath}` - Replaced with the instance path

## Example

Here's an example of an Overlord project file:

```yaml
# Project information
info:
  name: "example.soc"
  version: "1.0.0"
  author: "Overlord Development Team"
  description: "Example SoC design with RISC-V cores"

# External catalog sources
catalogs:
  - type: "git"
    url: "https://github.com/example/hardware-catalog.git"
  - type: "local"
    path: "/path/to/local/catalog"
  - type: "fetch"
    url: "https://example.com/api/catalog"

# Referenced components
components:
  - type: local
    path: /home/deano/local_prefab.yaml
  - type: fetch
    url: https://gagameos.com/gagameos_base_prefab.yaml
  - type: git
    url: https://github.com/DeanoC/bl616.git

# Default settings for the project
defaults:
  target_frequency: "100MHz"

# Hardware and software instances
instances:
  # CPU instances
  - name: "cpu0"
    type: "cpu.riscv.test"
    config:
      core_count: 4
      triple: "riscv64-unknown-elf"

  - name: "cpu1"
    type: "cpu.arm.test"
    config:
      core_count: 2
      triple: "aarch64-none-elf"

  # Memory instances
  - name: "main_memory"
    type: "ram.ddr4.test"
    config:
      ranges:
        - address: "0x80000000"
          size: "0x40000000"

  - name: "sram0"
    type: "ram.sram.test"
    config:
      ranges:
        - address: "0x00100000"
          size: "0x00010000"

  # Clock instance
  - name: "system_clock"
    type: "clock.test.100mhz"
    config:
      frequency: "100MHz"

  # IO instance
  - name: "uart0"
    type: "io.uart.test"
    config:
      visible_to_software: true

  # component instance
  - name: "sub_module"
    type: "component.zynqps7.test"

# Connections between instances
connections:
  # Bus connections
  - type: "bus"
    connection: "cpu0 -> main_memory"
    bus_protocol: "axi4"
    bus_name: "cpu0_mem_bus"

  - type: "bus"
    connection: "cpu1 -> sram0"
    bus_protocol: "axi4-lite"
    silent: true

  # Port connections
  - type: "port"
    connection: "uart0.tx -> board.uart_tx"

  # Port group connections
  - type: "port_group"
    connection: "cpu0.gpio -> board.leds"
    first_prefix: "gpio_"
    excludes: ["gpio_3", "gpio_4"]

  # Clock connections
  - type: "clock"
    connection: "system_clock -> cpu0.clk"

  # Logical connections
  - type: "logical"
    connection: "cpu0.reset -> board.reset_button"

  # Parameters connections
  - type: "parameters"
    connection: "cpu0 -> cpu1"
    parameters:
      - name: "cache_size"
        value: 8192
        type: "integer"
      - name: "debug_mode"
        value: true

  # Constant connections
  - type: "constant"
    connection: "cpu0.vector_base"
    value: "0x00000000"

definitions:
  - name: "Cortex-A53"
    type: "cpu.arm.a53"
    config:
      core_count: 4
      triple: "aarch64-none-elf"
  
  - name: "MainMemory"
    type: "ram.sram.main"
    config:
      ranges:
        - address: "0x80000000"
          size: "0x40000000"
        - address: "0xC0000000" 
          size: "0x20000000"
  
  - name: "SystemClock"
    type: "clock.system.100mhz"
    config:
      frequency: "100MHz"    
```

## Advanced Features

### Constants

Constants can be defined in the file and used throughout the project. They are collected from unconnected elements and made available to components.

### Software Dependencies

Software components can declare dependencies on other components, which the Project class resolves automatically. This ensures all necessary drivers and libraries are included.

## Best Practices

1. Use catalogs and component to organize complex projects into smaller, manageable files
2. Group related instances and connections in the same file
3. Use meaningful names that reflect the function of components
4. Document your configuration with comments