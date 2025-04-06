# Testing and Validating Overlord Project Files

## Introduction

This guide provides tips and techniques for testing and validating your Overlord project files (.over) to ensure they work correctly with the Overlord.Project system.

## Validation Methods

### Project Report

The Overlord framework includes a "report" command that analyzes your project file and generates a summary of instances, connections, and potential issues:

```bash
sbt "run report path/to/your/project.over"
```

This command provides an overview of your project structure without generating output files, making it useful for quick validation.

### Common Issues and Solutions

#### Missing Dependencies

**Symptom:** Error messages about missing components or definitions.

**Solution:** Ensure all referenced components have their definitions in the catalog, and all software dependencies are available. The Project class will try to automatically resolve software dependencies, but hardware dependencies must be explicitly included.

#### Connection Errors

**Symptom:** Error messages about failed connections, usually with incompatible interfaces.

**Solution:** Verify that the connected interfaces are compatible. Check the instance types and their available interfaces in the catalog.

#### Path Resolution Problems

**Symptom:** File not found errors, especially for included resources.

**Solution:** Remember that paths are resolved relative to the current path context. The Project class maintains separate stacks for catalog, instance, and output paths, which can sometimes lead to confusion.

## Testing Strategies

### Incremental Development

Build your project file incrementally:

1. Start with a minimal project with just one or two instances
2. Validate using the report command
3. Add connections between the instances
4. Validate again
5. Continue adding instances and connections in small batches

### Testing Component Configurations

To test specific component configurations without creating a full project:

1. Create a minimal test.over file that includes only the component you want to test
2. Use a simplified board definition if possible
3. Run with the report command to validate the configuration

### Using Prefabs for Testing

Create test prefabs that represent common subsystems in your design. This allows you to:

1. Test subsystems in isolation
2. Reuse validated configurations
3. Simplify your main project file

## Debugging Tools

### Log Messages

The Project class outputs various log messages during processing. Pay attention to:

- Warnings about duplicate definitions
- Path resolution messages
- Connection establishment messages

### Inspecting Generated Files

After project generation, examine the generated files in the output directory:

- Check hardware configuration files for correct component instantiation
- Verify register maps match your expectations
- Review generated software interfaces for accuracy

### Manual YAML Validation

Use a YAML validator to ensure your .over file has valid YAML syntax before processing it with Overlord:

```bash
python -c 'import yaml; yaml.safe_load(open("your_project.over"))'
```

## Advanced Validation

### Hardware Validation

For validating hardware aspects of your project:

1. Generate the project with `sbt "run create path/to/your/project.over"`
2. Check the generated Verilog/VHDL in the output directory
3. Use simulation tools to validate hardware behavior

### Software Validation

For validating software aspects:

1. Examine generated libraries and driver code
2. Compile the generated software using provided build scripts
3. Test software interfaces to hardware components

## Versioning and Backup

Always version control your .over files and create backups before making substantial changes. This allows you to revert to a working configuration if needed.

## Conclusion

Proper testing and validation of .over files ensures your Overlord projects will build correctly and function as expected. Start with small, incremental changes, and use the available validation tools to catch issues early in the development process.