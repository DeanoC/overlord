# Overlord Project File Format (.yaml)

## Introduction

The Overlord framework uses YAML-based configuration files with the `.yaml` extension to define projects, components, and their connections. These files are the main input for the Overlord.Project class and define how your hardware and software components are instantiated, configured, and connected.

## File Format

Overlord project files are written in YAML and have the `.yaml` extension. The framework includes a utility script `convert_toml_to_yaml.py` that can convert TOML files to YAML if you prefer writing in TOML.

## Main Sections

A typical `.yaml` file consists of the following main sections:

### 1. Defaults

Optional section for specifying default values that will be applied to instances if specific values are not provided.

```yaml
defaults:
  key1: value1
  key2: value2
```

### 2. Include

Allows including other `.yaml` files to promote modularity and reuse.

```yaml
include:
  - resource: path/to/another/file.yaml
  - resource: another/include.yaml
```

### 3. Instance

Defines hardware or software components to be instantiated in the project.

```yaml
instance:
  - name: instanceName
    type: component.type  # e.g., cpu.riscv, memory.ram, etc.
    # Optional parameters for the instance
    parameter1: value1
    parameter2: value2
```

### 4. Connection

Defines connections between instances, such as buses, signals, or other interfaces.

```yaml
connection:
  - first: instanceName1.interfaceName
    second: instanceName2.interfaceName
    # Optional parameters for the connection
    parameter1: value1
    parameter2: value2
```

### 5. Prefab

References preconfigured component templates from prefab catalogs.

```yaml
prefab:
  - name: prefabName  # e.g., boards.xilinx_zcu104
```

## Processing Order

The Overlord.Project class processes these sections in a specific order:

1. Process `include` sections first, recursively including other files.
2. Process `instance` sections to create component instances.
3. Process `connection` sections to connect those instances.
4. Process `prefab` sections to include predefined configurations.

## Special Macros

When defining paths or other string values, you can use special macros that will be resolved by the Project class:

- `${name}` - Replaced with the instance name
- `${projectPath}` - Replaced with the project path
- `${definitionPath}` - Replaced with the instance definition path
- `${outPath}` - Replaced with the configured output path
- `${instancePath}` - Replaced with the instance path

## Example

Here's a simple example of an Overlord project file:

```yaml
# Define default values
defaults:
  clockFrequency: 100000000  # 100MHz

# Include other configuration files
include:
  - resource: components/peripherals.yaml

# Define instances
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

# Define connections
connection:
  - first: cpu0.data_bus
    second: ram0.bus

  - first: cpu0.peripheral_bus
    second: uart0.bus

# Include board-specific configuration
prefab:
  - name: boards.xilinx_zcu104
```

## Board Specification

Each project must specify a board, which is automatically injected as if it had been a prefab in the main project file. The board definition provides critical information about the physical hardware platform.

## Output and Paths

The Project class manages several stacks of paths:

- `catalogPathStack` - For finding component definitions
- `instancePathStack` - For finding instance configurations
- `outPathStack` - For placing generated output files

These path stacks help the framework locate resources and organize outputs while processing nested includes and hierarchical components.

## Advanced Features

### Constants

Constants can be defined in the file and used throughout the project. They are collected from unconnected elements and made available to components.

### Distance Matrix

The framework computes a "distance matrix" between components to understand their connectivity topology. This is used for routing and software generation.

### Software Dependencies

Software components can declare dependencies on other components, which the Project class resolves automatically. This ensures all necessary drivers and libraries are included.

## File Processing

The Project class processes `.yaml` files through the following steps:

1. Read the YAML structure using `Utils.readYaml()`
2. Process includes and imports recursively
3. Create instances based on definitions from catalogs
4. Establish connections between instances
5. Resolve software dependencies
6. Generate hardware configuration
7. Generate software components

## Best Practices

1. Use includes to organize complex projects into smaller, manageable files
2. Use prefabs for common configurations
3. Group related instances and connections in the same file
4. Use meaningful names that reflect the function of components
5. Document your configuration with comments