# Connection Details

The `connection` field in the allows you to define relationships between instances. Below is a detailed explanation of its structure and usage.

## Syntax

- `bus_name`: The name of the bus or connection.
- `connection`: Specifies the connection in the format `Source -> Target`, `Source <-> Target`, or `Source <- Target`.
  - `->`: Represents a unidirectional connection from `Source` to `Target`.
  - `<->` or `<>`: Represents a bidirectional connection between `Source` and `Target`.
  - `<-`: Represents a unidirectional connection from `Target` to `Source`.
- `type`: The type of connection. Supported types include:
  - **`bus`**: Represents a hardware bus connection. Additional fields:
    - `bus_protocol`: Specifies the protocol (e.g., `internal`).
    - `consumer_bus_name`: Optional, specifies the consumer bus name.
  - **`logical`**: Represents a logical connection between instances.
  - **`port`**: Represents a connection between ports.
  - **`clock`**: Represents a clock signal connection.
  - **`parameters`**: Represents parameter connections. Additional fields:
    - `parameters`: A list of parameter definitions.
  - **`port_group`**: Represents grouped port connections. Additional fields:
    - `first_prefix`: Prefix for the first group of ports.
    - `second_prefix`: Prefix for the second group of ports.
    - `excludes`: A list of ports to exclude.
- `[optional] consumer_bus_name`: Specifies an additional consumer bus name (optional).

## Example

```yaml
connection:
  - bus_name: busA
    connection: A -> B
    consumer_bus_name: in0
    type: bus
  - connection: B <-> C
    type: logical
  - connection: clk -> cpu
    type: clock
  - connection: A -> B
    type: port
  - connection: _ -> cpu
    type: parameters
    parameters:
      - name: param1
        type: constant
        value: 42
  - connection: groupA -> groupB
    type: port_group
    first_prefix: "groupA_"
    second_prefix: "groupB_"
    excludes:
      - groupA_exclude
      - groupB_exclude
```

## Notes

- The `connection` field uses the format `Source [direction] Target` to define the direction of the connection.
- The `type` field determines the nature of the connection and may require additional fields.
- Invalid or missing fields (e.g., `type` or `connection`) will result in errors during parsing.