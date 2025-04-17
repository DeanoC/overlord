# Build Instructions for Overlord

Follow these steps to set up and build the Overlord project on a Linux system.

## Prerequisites

1. **Java Development Kit (JDK)**: Install JDK 11 or newer.
   ```bash
   sudo apt update
   sudo apt install openjdk-11-jdk -y
   ```

2. **Scala Build Tool (SBT)**: Install SBT for building the project.
   ```bash
   echo "deb https://repo.scala-sbt.org/scalasbt/debian all main" | sudo tee /etc/apt/sources.list.d/sbt.list
   echo "deb https://repo.scala-sbt.org/scalasbt/debian /" | sudo tee /etc/apt/sources.list.d/sbt_old.list
   curl -sL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x99e82a75642ac823" | sudo apt-key add
   sudo apt update
   sudo apt install sbt -y
   ```

3. **Git**: Required for cloning the repository.
   ```bash
   sudo apt install git -y
   ```

## Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/deanoc/overlord.git
   cd overlord
   ```

2. Build the project using SBT:
   ```bash
   sbt clean compile
   ```

3. Optional: Build the Debian package:
   ```bash
   sbt debian:packageBin
   ```
   This will generate a `.deb` file in the `target` directory.

4. Optional: Run the project:
   ```bash
   sbt run
   ```

You can now use the `overlord` command as described in the documentation.

## Embedded Resources

### Baremetal Toolchain Script

The `build_baremetal_toolchain.sh` script (used for creating GCC toolchains) is now embedded as a resource in the Overlord JAR file:

- During the build process, the script is automatically copied from the `scripts/` directory into the JAR resources.
- At runtime, when you run the `overlord create gcc-toolchain` command, the application extracts the script from its internal resources and executes it.
- Users no longer need to have the `scripts/` directory present on their system.
- The build process ensures the embedded resource is always up to date with the source script.

This change simplifies distribution and ensures all users have access to the latest version of the script without manual updates.
