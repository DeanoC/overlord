#!/usr/bin/env bash

set -e

# Usage: ./build_baremetal_toolchain.sh <target-triple> <install-dir>
# Example: ./build_baremetal_toolchain.sh arm-none-eabi /opt/toolchains/arm-none-eabi

# Helper functions first (so they're available to all code paths)

# Helper: print error and exit
die() {
  echo "Error: $1" >&2
  exit 1
}

# Helper: print info
info() {
  echo -e "\\033[1;34m$1\\033[0m"
}

# Helper: clean everything except downloaded tarballs
clean_build() {
  local install_dir="$1"

  # Make install_dir absolute
  if command -v realpath &>/dev/null; then
    install_dir="$(realpath "$install_dir")"
  else
    install_dir="$(cd "$install_dir" 2>/dev/null && pwd || echo "$install_dir")"
  fi

  # Set up directories
  local work_dir="$install_dir"
  local tarballs_dir="$work_dir/tarballs"
  local build_dir="$work_dir/build"

  info "Cleaning build environment..."
  info "Working directory: $work_dir"
  info "Tarballs directory: $tarballs_dir"

  # Create directories if they don't exist
  mkdir -p "$work_dir"
  mkdir -p "$tarballs_dir"

  # Keep tarballs directory content but remove everything else
  find "$work_dir" -mindepth 1 -maxdepth 1 -not -path "$tarballs_dir" -exec rm -rf {} \;

  info "Cleaned build environment. Kept downloaded tarballs."
}

# Parse arguments - First check for the clean command
if [[ "$1" == "clean" ]]; then
  if [[ -z "$2" ]]; then
    echo "Usage for clean: $0 clean <install-dir>"
    exit 1
  fi
  clean_build "$2"
  exit 0
fi

# Check for help or no arguments
if [[ "$1" == "--help" || "$1" == "-h" || -z "$1" ]]; then
  echo "Usage: $0 <target-triple> <install-dir> [options]"
  echo "       $0 clean <install-dir>  # Clean build environment"
  echo ""
  echo "Options:"
  echo "  --use-local-tarballs <dir>   Use tarballs from the specified directory instead of downloading"
  echo "  --skip-download              Skip downloading tarballs (assumes they are already in the tarballs directory)"
  echo "  --help, -h                   Show this help message"
  exit 0
fi

# Check if we have the required positional arguments
if [[ -z "$1" || -z "$2" ]]; then
  echo "Error: Missing required arguments"
  echo "Usage: $0 <target-triple> <install-dir> [options]"
  echo "       $0 clean <install-dir>  # Clean build environment"
  echo "Try '$0 --help' for more information."
  exit 1
fi

# Set positional arguments
TARGET_TRIPLE="$1"
INSTALL_DIR="$2"
LOCAL_TARBALLS_DIR=""
SKIP_DOWNLOAD=false

# Parse additional options
shift 2
while [[ $# -gt 0 ]]; do
  case "$1" in
    --use-local-tarballs)
      if [[ -z "$2" ]]; then
        die "Option --use-local-tarballs requires a directory argument"
      fi
      LOCAL_TARBALLS_DIR="$2"
      shift 2
      ;;
    --skip-download)
      SKIP_DOWNLOAD=true
      shift
      ;;
    --help|-h)
      echo "Usage: $0 <target-triple> <install-dir> [options]"
      echo "       $0 clean <install-dir>  # Clean build environment"
      echo ""
      echo "Options:"
      echo "  --use-local-tarballs <dir>   Use tarballs from the specified directory instead of downloading"
      echo "  --skip-download              Skip downloading tarballs (assumes they are already in the tarballs directory)"
      echo "  --help, -h                   Show this help message"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      echo "Try '$0 --help' for more information."
      exit 1
      ;;
  esac
done

BINUTILS_VERSION="2.42"
GCC_VERSION="13.2.0"
BINUTILS_TARBALL="binutils-${BINUTILS_VERSION}.tar.xz"
GCC_TARBALL="gcc-${GCC_VERSION}.tar.xz"

# Primary URLs
BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/${BINUTILS_TARBALL}"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/${GCC_TARBALL}"

# Mirror URLs (used if primary fails)
BINUTILS_MIRRORS=(
  "https://ftpmirror.gnu.org/binutils/${BINUTILS_TARBALL}"
  "https://mirrors.kernel.org/gnu/binutils/${BINUTILS_TARBALL}"
  "https://mirror.csclub.uwaterloo.ca/gnu/binutils/${BINUTILS_TARBALL}"
)

GCC_MIRRORS=(
  "https://ftpmirror.gnu.org/gcc/gcc-${GCC_VERSION}/${GCC_TARBALL}"
  "https://mirrors.kernel.org/gnu/gcc/gcc-${GCC_VERSION}/${GCC_TARBALL}"
  "https://mirror.csclub.uwaterloo.ca/gnu/gcc/gcc-${GCC_VERSION}/${GCC_TARBALL}"
)

NUM_CORES=$(nproc)
WGET_OPTS="--timeout=30 --tries=3 --retry-connrefused"

# Helper function to download with fallback to mirrors
download_with_mirrors() {
  local file="$1"
  local primary_url="$2"
  shift 2
  local mirrors=("$@")

  # Try primary URL first
  info "Downloading $file from primary URL..."
  if wget $WGET_OPTS "$primary_url"; then
    return 0
  fi

  # Try mirrors if primary fails
  for mirror in "${mirrors[@]}"; do
    info "Primary URL failed. Trying mirror: $mirror"
    if wget $WGET_OPTS "$mirror"; then
      return 0
    fi
  done

  # All URLs failed
  die "Failed to download $file from all sources"
}

# 1. Check arguments
# Make INSTALL_DIR absolute
if command -v realpath &>/dev/null; then
  INSTALL_DIR="$(realpath "$INSTALL_DIR")"
else
  INSTALL_DIR="$(cd "$INSTALL_DIR" 2>/dev/null && pwd || echo "$INSTALL_DIR")"
fi

# 2. Check required tools
REQUIRED_TOOLS=(wget tar make gcc g++ bzip2 xz perl)
MISSING_TOOLS=()
for tool in "${REQUIRED_TOOLS[@]}"; do
  if ! command -v "$tool" &>/dev/null; then
    MISSING_TOOLS+=("$tool")
  fi
done

if [[ ${#MISSING_TOOLS[@]} -ne 0 ]]; then
  echo "The following required tools are missing:"
  for t in "${MISSING_TOOLS[@]}"; do echo "  - $t"; done
  echo "Please install them using your package manager. For example:"
  echo "  sudo apt-get update && sudo apt-get install build-essential wget bzip2 xz-utils perl"
  exit 1
fi

# 3. Check for required libraries (headers)
REQUIRED_LIBS=(gmp mpfr mpc isl)
MISSING_LIBS=()
for lib in "${REQUIRED_LIBS[@]}"; do
  if ! ldconfig -p | grep -q "$lib"; then
    MISSING_LIBS+=("$lib")
  fi
done

if [[ ${#MISSING_LIBS[@]} -ne 0 ]]; then
  echo "The following required libraries are missing:"
  for l in "${MISSING_LIBS[@]}"; do echo "  - $l (dev package)"; done
  echo "Please install them using your package manager. For example:"
  echo "  sudo apt-get install libgmp-dev libmpfr-dev libmpc-dev libisl-dev"
  exit 1
fi

# 4. Set up directories with absolute paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK_DIR="$INSTALL_DIR"
TARBALLS_DIR="$WORK_DIR/tarballs"
BUILD_DIR="$WORK_DIR/build"

info "Using work directory: $WORK_DIR"
info "Using tarballs directory: $TARBALLS_DIR"
info "Using build directory: $BUILD_DIR"

# Create directories
mkdir -p "$TARBALLS_DIR"
mkdir -p "$BUILD_DIR"

# 5. Download or copy tarballs if missing
cd "$TARBALLS_DIR"

# If using local tarballs, copy them instead of downloading
if [[ -n "$LOCAL_TARBALLS_DIR" ]]; then
  info "Using local tarballs from $LOCAL_TARBALLS_DIR"

  if [[ ! -f "$BINUTILS_TARBALL" && -f "$LOCAL_TARBALLS_DIR/$BINUTILS_TARBALL" ]]; then
    info "Copying $BINUTILS_TARBALL from local directory..."
    cp "$LOCAL_TARBALLS_DIR/$BINUTILS_TARBALL" .
  fi

  if [[ ! -f "$GCC_TARBALL" && -f "$LOCAL_TARBALLS_DIR/$GCC_TARBALL" ]]; then
    info "Copying $GCC_TARBALL from local directory..."
    cp "$LOCAL_TARBALLS_DIR/$GCC_TARBALL" .
  fi
elif [[ "$SKIP_DOWNLOAD" != "true" ]]; then
  # Download tarballs if not skipping download
  if [[ ! -f "$BINUTILS_TARBALL" ]]; then
    download_with_mirrors "$BINUTILS_TARBALL" "$BINUTILS_URL" "${BINUTILS_MIRRORS[@]}"
  fi

  if [[ ! -f "$GCC_TARBALL" ]]; then
    download_with_mirrors "$GCC_TARBALL" "$GCC_URL" "${GCC_MIRRORS[@]}"
  fi
else
  info "Skipping download step as requested"
fi

# Check if tarballs exist
if [[ ! -f "$BINUTILS_TARBALL" ]]; then
  die "Binutils tarball not found: $BINUTILS_TARBALL"
fi

if [[ ! -f "$GCC_TARBALL" ]]; then
  die "GCC tarball not found: $GCC_TARBALL"
fi

# 6. Extract tarballs
cd "$BUILD_DIR"

# Ensure ISL include directory exists to avoid symlink errors
mkdir -p isl/include

if [[ ! -d "binutils-${BINUTILS_VERSION}" ]]; then
  info "Extracting binutils..."
  tar -xf "$TARBALLS_DIR/$BINUTILS_TARBALL"
fi

if [[ ! -d "gcc-${GCC_VERSION}" ]]; then
  info "Extracting gcc..."
  tar -xf "$TARBALLS_DIR/$GCC_TARBALL"
fi

# 7. Build and install binutils
BINUTILS_BUILD_DIR="$BUILD_DIR/build-binutils"
mkdir -p "$BINUTILS_BUILD_DIR"
cd "$BINUTILS_BUILD_DIR"
if [[ ! -f ".built" || ! -f "$INSTALL_DIR/bin/${TARGET_TRIPLE}-as" ]]; then
  info "Configuring binutils..."
  "$BUILD_DIR/binutils-${BINUTILS_VERSION}/configure" --target="$TARGET_TRIPLE" --prefix="$INSTALL_DIR" $BINUTILS_CONFIGURE_OPTIONS
  info "Building binutils..."
  make -j"$NUM_CORES"
  info "Installing binutils..."
  make install
  touch .built
else
  info "Binutils already built, skipping."
fi

# 8. Build and install gcc (C & C++, bare-metal)
GCC_BUILD_DIR="$BUILD_DIR/build-gcc"
mkdir -p "$GCC_BUILD_DIR"
cd "$GCC_BUILD_DIR"
if [[ ! -f ".built" || ! -f "$INSTALL_DIR/bin/${TARGET_TRIPLE}-gcc" ]]; then
  info "Downloading GCC prerequisites..."
  cd "$BUILD_DIR/gcc-${GCC_VERSION}"
  ./contrib/download_prerequisites
  cd "$GCC_BUILD_DIR"

  info "Configuring gcc..."
  "$BUILD_DIR/gcc-${GCC_VERSION}/configure" --target="$TARGET_TRIPLE" --prefix="$INSTALL_DIR" $GCC_CONFIGURE_OPTIONS
  info "Building gcc (this may take a while)..."
  make all-gcc -j"$NUM_CORES"
  info "Installing gcc..."
  make install-gcc
  touch .built
else
  info "GCC already built, skipping."
fi

# 9. Test the toolchain
info "Testing the new toolchain..."
TEST_PREFIX="/tmp/toolchain_test_${USER}_${TARGET_TRIPLE}"
TEST_C="${TEST_PREFIX}.c"
TEST_ELF="${TEST_PREFIX}.elf"
TEST_LOG="${TEST_PREFIX}.log"

cat > "$TEST_C" <<EOF
int main(void) { return 0; }
EOF

"$INSTALL_DIR/bin/${TARGET_TRIPLE}-gcc" -nostdlib -o "$TEST_ELF" "$TEST_C" 2> "$TEST_LOG"
RESULT=$?

if [[ $RESULT -eq 0 && -f "$TEST_ELF" ]]; then
  echo -e "\\033[1;32mToolchain build and test succeeded!\\033[0m"
  echo "Test binary: $TEST_ELF"
else
  echo -e "\\033[1;31mToolchain build or test failed.\\033[0m"
  echo "See $TEST_LOG for details."
  exit 1
fi

info "Done."