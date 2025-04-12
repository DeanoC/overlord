#!/bin/bash

# Script to build the overlord project for local development
# This script runs compile, assembly, and stage tasks in sequence
# It checks the result of each step and only continues if the previous step was successful

# Function to display help/usage
show_help() {
  echo "Usage: $0 [OPTIONS]"
  echo "Build the Overlord project for local development"
  echo ""
  echo "Options:"
  echo "  -h, --help       Display this help message"
  echo "  -a, --with-assembly  Build the fat JAR with assembly"
  echo ""
}

# Function to handle errors
handle_error() {
  echo -e "\033[0;31mERROR: $1 failed\033[0m"
  echo "Build process stopped due to errors."
  exit 1
}

# Default settings
ENABLE_ASSEMBLY=false
ENABLE_DOCS=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    -h|--help)
      show_help
      exit 0
      ;;
    -a|--with-assembly)
      ENABLE_ASSEMBLY=true
      shift
      ;;
    --with-docs)
      ENABLE_DOCS=true
      shift
      ;;
    *)
      echo "Unknown option: $key"
      show_help
      exit 1
      ;;
  esac
done

echo "=== Building Overlord for local development ==="

# Set the working directory to the script's location
cd "$(dirname "$0")"

# Build steps with status checking
echo "1. Compiling the project..."
sbt compile
if [ $? -ne 0 ]; then
  handle_error "Compilation"
fi
echo -e "\033[0;32m✓ Compilation successful\033[0m"

if [ "$ENABLE_DOCS" = true ]; then
  echo "2. Generating Scala documentation..."
  sbt doc
  if [ $? -ne 0 ]; then
    handle_error "Documentation"
  fi
  echo -e "\033[0;32m✓ Documentation generation successful\033[0m"
else
  echo "2. Skipping documentation generation..."
fi

if [ "$ENABLE_ASSEMBLY" = true ]; then
  echo "3. Building fat JAR with assembly..."
  sbt assembly
  if [ $? -ne 0 ]; then
    handle_error "Assembly"
  fi
  echo -e "\033[0;32m✓ Assembly successful\033[0m"
else
  echo "3. Skipping assembly step (fat JAR will not be built)..."
fi

echo "4. Preparing staged application..."
if [ "$ENABLE_DOCS" = false ]; then
  echo "Skipping documentation during stage..."
  sbt 'set Compile / doc / sources := Seq()' stage
else
  sbt stage
fi
if [ $? -ne 0 ]; then
  handle_error "Stage"
fi
echo -e "\033[0;32m✓ Stage successful\033[0m"

echo "=== Build completed successfully ==="
echo "You can find:"
if [ "$ENABLE_ASSEMBLY" = true ]; then
  echo " - Fat JAR in: target/scala-*/overlord-assembly-*.jar"
fi
echo " - Staged application in: target/universal/stage/"
echo " - Run the application with: target/universal/stage/bin/overlord"
