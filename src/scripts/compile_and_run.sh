#!/bin/bash

# compile_and_run.sh
# Script to compile the overlord project and install it to local ~/bin directory

set -e

# Define colors for output
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RED="\033[0;31m"
RESET="\033[0m"

# Define paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"  # Go up two directories to reach project root
LOCAL_BIN_DIR="$HOME/bin"
OVERLORD_BIN_PATH="$LOCAL_BIN_DIR/overlord"

# Parse command-line options
CLEAN=false
while [[ $# -gt 0 ]]; do
  case "$1" in
    --clean)
      CLEAN=true
      shift
      ;;
    *)
      echo -e "${RED}Unknown option: $1${RESET}"
      echo -e "Usage: $0 [--clean]"
      exit 1
      ;;
  esac
done

echo -e "${YELLOW}Starting compilation of overlord project...${RESET}"

# Step 1: Change to the project directory
cd "$PROJECT_DIR"
echo -e "${YELLOW}Working from project directory: $PROJECT_DIR${RESET}"

# Step 2: Clean and compile with SBT
if [ "$CLEAN" = true ]; then
  echo -e "${YELLOW}Cleaning previous build...${RESET}"
  sbt clean
else
  echo -e "${YELLOW}Skipping clean (use --clean to clean previous build)${RESET}"
fi

echo -e "${YELLOW}Compiling project...${RESET}"
sbt compile

# Step 3: Stage the application (creates executable scripts)
echo -e "${YELLOW}Staging application...${RESET}"
sbt stage

# Step 4: Create local bin directory if it doesn't exist
if [ ! -d "$LOCAL_BIN_DIR" ]; then
    echo -e "${YELLOW}Creating local bin directory at $LOCAL_BIN_DIR...${RESET}"
    mkdir -p "$LOCAL_BIN_DIR"
fi

# Step 5: Copy the staged executable to the local bin directory
STAGED_BIN="$PROJECT_DIR/target/universal/stage/bin/overlord"
if [ -f "$STAGED_BIN" ]; then
    echo -e "${YELLOW}Copying overlord executable to $OVERLORD_BIN_PATH...${RESET}"
    cp "$STAGED_BIN" "$OVERLORD_BIN_PATH"
    chmod +x "$OVERLORD_BIN_PATH"
    
    # Also copy the lib directory needed by the executable
    echo -e "${YELLOW}Copying lib directory to $LOCAL_BIN_DIR/../lib...${RESET}"
    mkdir -p "$LOCAL_BIN_DIR/../lib"
    cp -r "$PROJECT_DIR/target/universal/stage/lib/"* "$LOCAL_BIN_DIR/../lib/"
else
    echo -e "${RED}Error: Could not find staged overlord executable.${RESET}"
    exit 1
fi

# Step 6: Check if local bin directory is in PATH, if not suggest adding it
if [[ ":$PATH:" != *":$LOCAL_BIN_DIR:"* ]]; then
    echo -e "${YELLOW}Warning: $LOCAL_BIN_DIR is not in your PATH.${RESET}"
    echo -e "${YELLOW}Consider adding the following line to your ~/.bashrc or ~/.profile:${RESET}"
    echo -e "${GREEN}export PATH=\"\$HOME/bin:\$PATH\"${RESET}"
    echo -e "${YELLOW}Then run 'source ~/.bashrc' or 'source ~/.profile' to update your current session.${RESET}"
    
    # Temporarily add the bin directory to PATH for this session
    echo -e "${YELLOW}Temporarily adding $LOCAL_BIN_DIR to your PATH for this session.${RESET}"
    export PATH="$LOCAL_BIN_DIR:$PATH"
fi

# Step 7: Verify installation
if [ -x "$OVERLORD_BIN_PATH" ]; then
    echo -e "${GREEN}Overlord successfully installed at $OVERLORD_BIN_PATH${RESET}"
    
    # Check if the execution will work based on PATH
    if [[ ":$PATH:" == *":$LOCAL_BIN_DIR:"* ]]; then
        echo -e "${GREEN}You can now run 'overlord' from anywhere.${RESET}"
    else
        echo -e "${YELLOW}You can now run overlord using the full path: $OVERLORD_BIN_PATH${RESET}"
    fi
    
    # Automatically run the template project without asking
    TEMPLATE_PROJECT_DIR="$PROJECT_DIR/overlord_template_project"
    cd "$TEMPLATE_PROJECT_DIR"
    echo -e "${YELLOW}Running template project from: $TEMPLATE_PROJECT_DIR${RESET}"
    
    # Run the overlord command directly with the template parameters
    echo -e "${YELLOW}Running: overlord create template_project.over --board kv260${RESET}"
    overlord create template_project.over --board kv260
else
    echo -e "${RED}Installation failed.${RESET}"
    exit 1
fi

echo -e "${GREEN}Done!${RESET}"
