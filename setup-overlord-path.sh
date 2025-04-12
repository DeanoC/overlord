#!/bin/bash

# Script to create symbolic links to overlord executables in ~/bin
# and add ~/bin to the PATH in .bashrc if needed

# Colors for output
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
RED="\033[0;31m"
RESET="\033[0m"

# Function to display help/usage
show_help() {
  echo "Usage: $0 [OPTIONS]"
  echo "Set up Overlord in your PATH by creating symbolic links in ~/bin"
  echo ""
  echo "Options:"
  echo "  -h, --help       Display this help message"
  echo "  -f, --force      Force creation of symbolic links even if they already exist"
  echo ""
}

# Default settings
FORCE_LINKS=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    -h|--help)
      show_help
      exit 0
      ;;
    -f|--force)
      FORCE_LINKS=true
      shift
      ;;
    *)
      echo "Unknown option: $key"
      show_help
      exit 1
      ;;
  esac
done

echo "=== Setting up Overlord in PATH ==="

# Set project directory path
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
OVERLORD_EXEC="${PROJECT_DIR}/target/universal/stage/bin/overlord"

# Print debug information
echo -e "${YELLOW}Debug information:${RESET}"
echo "- Project directory: ${PROJECT_DIR}"
echo "- Expected Overlord executable: ${OVERLORD_EXEC}"
echo "- Current PATH: $PATH"

# Check if overlord executable exists
if [ ! -f "$OVERLORD_EXEC" ]; then
  echo -e "${RED}Error: Overlord executable not found at ${OVERLORD_EXEC}${RESET}"
  echo "Have you built the project? Run ./build-local.sh first."
  echo "Running build-local.sh now..."

  # Try to build the project first
  ./build-local.sh

  if [ ! -f "$OVERLORD_EXEC" ]; then
    echo -e "${RED}Build failed or did not produce the expected executable.${RESET}"
    echo "Please build the project manually with './build-local.sh' and try again."
    exit 1
  else
    echo -e "${GREEN}Build successful, continuing with setup...${RESET}"
  fi
fi

# Create ~/bin directory if it doesn't exist
if [ ! -d ~/bin ]; then
  echo "Creating ~/bin directory..."
  mkdir -p ~/bin
  echo -e "${GREEN}✓ Created ~/bin directory${RESET}"
else
  echo "~/bin directory already exists."
fi

# Create symbolic link to overlord
if [ -L ~/bin/overlord ] && [ "$FORCE_LINKS" != true ]; then
  echo "Symbolic link for overlord already exists in ~/bin."
  echo "Use -f or --force to overwrite."
else
  # Remove existing link if it exists
  if [ -L ~/bin/overlord ]; then
    rm ~/bin/overlord
  fi

  # Create new link and verify it
  ln -sf "$OVERLORD_EXEC" ~/bin/overlord
  if [ -L ~/bin/overlord ]; then
    echo -e "${GREEN}✓ Created symbolic link: ~/bin/overlord -> ${OVERLORD_EXEC}${RESET}"
  else
    echo -e "${RED}Failed to create symbolic link.${RESET}"
    exit 1
  fi
fi

# Add ~/bin to PATH in .bashrc if not already there
if ! grep -q 'export PATH="$HOME/bin:$PATH"' ~/.bashrc; then
  echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
  echo -e "${GREEN}✓ Added ~/bin to PATH in ~/.bashrc${RESET}"
  echo "To apply this change immediately, run: source ~/.bashrc"
else
  echo "~/bin is already in PATH (in ~/.bashrc)."
fi

# Make the script executable
chmod +x "$OVERLORD_EXEC" 2>/dev/null
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ Made overlord executable${RESET}"
else
  echo -e "${YELLOW}Warning: Could not set executable permissions on ${OVERLORD_EXEC}${RESET}"
fi

chmod +x ~/bin/overlord 2>/dev/null
if [ $? -ne 0 ]; then
  echo -e "${YELLOW}Warning: Could not set executable permissions on ~/bin/overlord${RESET}"
fi

# Test if overlord is in PATH now
echo "Testing if overlord is available in PATH..."
if command -v overlord &>/dev/null; then
  echo -e "${GREEN}✓ overlord is now available in your PATH${RESET}"
else
  echo -e "${YELLOW}Warning: overlord is not yet in your PATH.${RESET}"
  echo "You need to run 'source ~/.bashrc' or restart your terminal."
  echo "Running 'source ~/.bashrc' for you now..."
  source ~/.bashrc

  # Test again after sourcing .bashrc
  if command -v overlord &>/dev/null; then
    echo -e "${GREEN}✓ overlord is now available in your PATH${RESET}"
  else
    echo -e "${YELLOW}Warning: overlord is still not in your PATH.${RESET}"
    echo "This may be because the script can't modify the current shell environment."
    echo "Please run 'source ~/.bashrc' manually in your terminal."
  fi
fi

echo -e "${GREEN}=== Setup completed successfully ===${RESET}"
echo "You can now run 'overlord' from anywhere after:"
echo "1. Ensuring a build is complete (run ./build-local.sh if needed)"
echo "2. Applying PATH changes with 'source ~/.bashrc' or opening a new terminal"
echo ""
echo -e "${YELLOW}Important note in VS Code Dev Containers:${RESET}"
echo "If you're using this in a Dev Container, note that ~/bin may not persist between sessions."
echo "You might need to run this setup script again in new container sessions."
