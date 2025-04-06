# Installing Overlord as a Linux CLI Application

This document describes how to build, package, and install Overlord as a system-wide command-line tool on Linux systems.

## Prerequisites

- SBT (Scala Build Tool)
- JDK 11 or newer
- Debian-based Linux distribution (for .deb package installation)

## Building the Debian Package

The project is configured to use `sbt-native-packager`, which allows us to create various package formats including Debian (.deb) packages.

To build the Debian package:

```bash
# Navigate to the project root
cd /path/to/overlord

# Clean and compile the project
sbt clean compile

# Create the Debian package
sbt debian:packageBin
```

This will generate a `.deb` file in the `target` directory, typically named `overlord_1.0_all.deb` (where version number may vary according to the build.sbt configuration).

## Installing the Package

Once the Debian package is built, you need to ensure it has the right permissions before installing:

```bash
# Fix file permissions (important!)
sudo chmod a+r target/overlord_1.0_all.deb
```

Then you can install it using one of the following methods:

### Using dpkg

```bash
sudo dpkg -i target/overlord_1.0_all.deb
```

### Using apt

```bash
sudo apt install ./target/overlord_1.0_all.deb
```

After installation, the `overlord` command will be available system-wide, and you can run it from any directory.

## Standalone Executable (No System-wide Installation)

If you prefer not to install the application system-wide, you can create a standalone executable that can be placed in your personal bin directory:

```bash
# Navigate to the project root
cd /path/to/overlord

# Create the stage directory with all needed files
sbt stage
```

This will generate the executable script and all dependencies in `target/universal/stage/`:
- `target/universal/stage/bin/overlord` - The executable script
- `target/universal/stage/lib/` - Directory containing all required JAR files

To use this as a personal command:

```bash
# Create a bin directory in your home if it doesn't exist
mkdir -p ~/bin

# Copy the executable script and the lib directory
cp -r target/universal/stage/bin/overlord ~/bin/
cp -r target/universal/stage/lib ~/bin/

# Make the script executable
chmod +x ~/bin/overlord
```

Ensure your `~/bin` directory is in your PATH by adding this line to your `~/.bashrc` or `~/.zshrc`:

```bash
export PATH="$HOME/bin:$PATH"
```

After updating your PATH, reload your shell configuration:

```bash
source ~/.bashrc  # or source ~/.zshrc if using zsh
```

You can now run `overlord` from anywhere without requiring system-wide installation.

Alternatively, you can create a symbolic link to the executable in the stage directory:

```bash
ln -s "$(pwd)/target/universal/stage/bin/overlord" ~/bin/overlord
```

This approach requires keeping the project directory intact.

## Uninstalling

To remove the installed package:

```bash
sudo apt remove overlord
```

## Usage

After installation, you can use the `overlord` command from anywhere on your system:

```bash
# Display help
overlord

# Create a new project
overlord create --board <board_name> --out <output_directory> <filename.over>

# Update an existing project
overlord update --board <board_name> --out <output_directory> <filename.over>

# Generate a report
overlord report --board <board_name> <filename.over>

# Generate SVD file
overlord svd --board <board_name> <filename.over>
```

## Troubleshooting

If you encounter issues with the Overlord CLI tool:

1. **Command not found**: Ensure the installation completed successfully and that `/usr/bin` is in your PATH.
2. **Permission issues**: The installation requires sudo privileges to install system-wide.
3. **Java errors**: Ensure you have a compatible JRE installed.

## Development and Rebuilding

If you make changes to the Overlord codebase, rebuild and reinstall the package:

```bash
sbt clean compile debian:packageBin
sudo dpkg -i target/overlord_1.0_all.deb
```

## Packaging Configuration

The packaging configuration is defined in `build.sbt` through the sbt-native-packager plugin. Key settings include:

```scala
// Enable plugins
enablePlugins(JavaAppPackaging)
enablePlugins(UniversalPlugin)
enablePlugins(DebianPlugin)

// Application packaging settings
Compile / mainClass := Some("Main")
maintainer := "deano@github.com"
packageSummary := "Overlord CLI Tool"
packageDescription := "A command-line tool for handling Overlord projects"

// Native packager settings
executableScriptName := "overlord"
```

These settings control aspects like the executable name, package metadata, and the main class to run.