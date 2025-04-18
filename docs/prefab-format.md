# YAML Prefab Format

A YAML Prefab file defines a reusable component in the Overlord system. Below is an explanation of the structure and possible items in a YAML Prefab file.

## Structure of a YAML Prefab

```yaml
catalogs:               # (Optional) A list of resources used by the prefab.
  - type: local
    path: /home/deano/catalog.yaml
  - type: fetch
    URL: https://gagameos.com/gagameos_base_catalog
  - type: git
    URL: https://github.com/DeanoC/bl616.git
prefabs:                # (Optional) A list of included prefabs.
  - included_prefab1
    included_prefab2
instance:               # (Optional) A list of instances defined in the prefab.
  - name: instance1
    type: a.type.name
  - name: instance2
    type: b.type.name
connection:             # (Optional) A list of connections between instances.
  - name: busA
    connection: A -> B
    type: bus
  - name: busB
    connection: B -> C
    type: logical
```

## Explanation of Items

### `catalogs` (Optional)
- **Description**: A list of other catalogs used by the prefab.
- **Type**: List
- **Example**:
  ```yaml
  catelogs:
    - type: local
      path: /home/deano/catalog.yaml
    - type: fetch
      URL: https://gagameos.com/gagameos_base_catalog
    - type: git
      URL: https://github.com/DeanoC/bl616.git
  ```

### `prefabs` (Optional)
- **Description**: A list of other prefabs included as if they were in this prefab.
- **Type**: List
- **Example**:
  ```yaml
  include:
    - included_prefab1
      included_prefab2
  ```

### `instance` (Optional)
- **Description**: A list of instances defined in the prefab. see [here](instance-details.md) for details.
- **Type**: List
- **Example**:
  ```yaml
  instance:
    - name: instance1
      type: a.type.name
    - name: instance2
      type: b.type.name
  ```

### `connection` (Optional)
- **Description**: A list of connections between instances. see [here](connection-details.md) for details.
- **Type**: List
- **Example**:
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
- **Notes**:
  - The `connection` field uses the format `Source [direction] Target` to define the direction of the connection.
  - The `type` field determines the nature of the connection and may require additional fields.
  - Invalid or missing fields (e.g., `type` or `connection`) will result in errors during parsing.

## Notes
- All fields (`resource`, `include`, `instance`, `connection`) are optional.
- The structure is flexible and can be customized based on the prefab's requirements.
