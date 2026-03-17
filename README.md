# Overlord

[![Scala CI](https://github.com/deanoc/overlord/actions/workflows/scala.yml/badge.svg)](https://github.com/deanoc/overlord/actions/workflows/scala.yml)

Framework for building FPGA-based systems across hardware, software, and bare-metal runtime.

Spans hardware configuration, system generation, and operating environments, exploring full-stack design from silicon to software.

Part of a broader focus on constructing complex systems from first principles.

Example packagesat https://github.com/DeanoC/ikuy_std_resources

## Features

- **Project Management**: Automates the creation, update, and reporting of projects.
- **Hardware Integration**: Supports a variety of FPGA boards with detailed hardware specifications.
- **Software Generation**: Generates software components, including libraries and programs, tailored for the board's architecture.
- **FPGA Support**: Provides tools for generating FPGA configurations and integrating custom hardware designs.
- **Resource Management**: Handles catalogs, prefabs, and resources for streamlined development.

## Supported Hardware

The framework is optimized for a wide range of FPGA boards, featuring:

- **SoC / FPGA**:
  - Multi-core Arm processors
  - FPGA logic cells, DSPs, and BRAM
  - GPU and display capabilities (if available)

- **Memory**:
  - High-speed DDR RAM

- **Storage**:
  - eMMC, SD Card slots, QSPI, and EEPROM

- **I/O**:
  - USB, Ethernet, Camera ports, and other interfaces

## Getting Started

Refer to the following documents to get started:

- [CLI Installation Guide](docs/cli-installation.md)
- [Build Instructions](docs/build-instructions.md)

## Debugging in Visual Studio Code

To debug Overlord in Visual Studio Code, you can use the following `launch.json` configuration:

```json
// filepath: /home/deano/overlord/.vscode/launch.json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "scala",
      "request": "launch",
      "name": "Debug Overlord",
      "mainClass": "Main",
      "buildTarget": "overlord",
      "args": [
        "create",
        "~/overlord/overlord_template_project/template_project.yaml",
        "--out",
        "~/overlord/overlord_template_project/",
        "--board",
        "tangnano9k"
      ]
    }
  ]
}
```

This configuration assumes you are using the Metals extension for Scala in Visual Studio Code. Replace `tangnano9k` and `template_project.yaml` with your specific board and `.yaml` file.

## Documentation

For detailed documentation, refer to the [wiki](<wiki-url>) or the `docs/` directory in the repository.

## Contributing

Contributions are welcome! Please read the [CONTRIBUTING.md](<contributing-url>) file for guidelines on how to contribute to this project.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
