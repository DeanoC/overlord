#!/bin/bash
# Make the script more robust when called from other applications
set -e  # Exit immediately if a command exits with a non-zero status

# Print debug information about the environment
echo "Running as user: $(whoami)"
echo "Current directory: $(pwd)"
echo "PATH: $PATH"

# Function to display error messages and exit
error_exit() {
    echo "ERROR: $1" >&2
    exit 1
}

# Function to display info messages
info() {
    echo "INFO: $1"
}

# Function to display warn messages
warn() {
    echo "WARN: $1"
}

# Function to check if a command exists
check_command() {
    command -v "$1" >/dev/null 2>&1 || error_exit "Required command '$1' not found. Please install it and try again."
}

# Function to download a file with resume capability
download_file() {
    local url="$1"
    local output_file="$2"

    # Ensure the directory exists
    local output_dir=$(dirname "$output_file")
    mkdir -p "$output_dir" || error_exit "Failed to create directory for download: $output_dir"

    # Check if we have write permission to the output file/directory
    if [ ! -w "$output_dir" ]; then
        error_exit "No write permission to download directory: $output_dir"
    fi

    # Print debug info
    info "Download URL: $url"
    info "Output file: $output_file"
    info "Output directory: $output_dir"

    # Try to use a local copy first if it exists in the current directory
    local filename=$(basename "$output_file")
    if [ -f "./$filename" ] && [ -s "./$filename" ]; then
        info "Found local copy of $filename, copying instead of downloading"
        cp "./$filename" "$output_file" || error_exit "Failed to copy local file $filename to $output_file"
        return 0
    fi

    # Try to use a local copy from common locations
    for dir in "/tmp" "$HOME/Downloads" "/var/tmp"; do
        if [ -f "$dir/$filename" ] && [ -s "$dir/$filename" ]; then
            info "Found local copy of $filename in $dir, copying instead of downloading"
            cp "$dir/$filename" "$output_file" || error_exit "Failed to copy local file $dir/$filename to $output_file"
            return 0
        fi
    done

    if [ -f "$output_file" ]; then
        # Check if file is partially downloaded
        if [ -s "$output_file" ]; then
            info "Resuming download of $output_file"
            wget -v -c "$url" -O "$output_file" || error_exit "Failed to download $url to $output_file"
        else
            # Empty file, restart download
            info "Downloading $output_file (empty file exists)"
            rm -f "$output_file" # Remove empty file
            wget -v "$url" -O "$output_file" || error_exit "Failed to download $url to $output_file"
        fi
    else
        info "Downloading $output_file (new file)"
        wget -v "$url" -O "$output_file" || error_exit "Failed to download $url to $output_file"
    fi

    # Verify the download was successful
    if [ ! -s "$output_file" ]; then
        error_exit "Download completed but file is empty: $output_file"
    fi

    info "Download completed successfully: $output_file"
}

# Function to extract a tarball
extract_tarball() {
    local tarball="$1"
    local target_dir="$2"

    if [ ! -d "$target_dir" ]; then
        info "Extracting $tarball"
        tar -xzf "$tarball" || error_exit "Failed to extract $tarball"
    else
        info "Directory $target_dir already exists, skipping extraction"
    fi
}

# Function to clean up on error
cleanup() {
    info "Cleaning up..."
    if [ -n "$BUILD_DIR" ] && [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    exit 1
}

# Set up trap for cleanup on error
trap cleanup ERR INT TERM

# Check for required tools
check_command wget
check_command tar
check_command make
check_command gcc
check_command g++

# Define variables with defaults
GCC_VERSION=${GCC_VERSION:-"10.2.0"}
BINUTILS_VERSION=${BINUTILS_VERSION:-"2.35"}
TARGET=${TARGET:-"riscv64-unknown-elf"}
INSTALL_DIR=${INSTALL_DIR:-"$(pwd)/${TARGET}-gcc"}

# Convert INSTALL_DIR to absolute path and ensure it's not duplicated
INSTALL_DIR=$(readlink -f "$INSTALL_DIR")
# Check if we're already in the install directory to avoid duplication
CURRENT_DIR=$(pwd)
if [[ "$INSTALL_DIR" == "$CURRENT_DIR"* ]]; then
    # If INSTALL_DIR is a subdirectory of current dir, use just the directory name
    RELATIVE_PATH=${INSTALL_DIR#"$CURRENT_DIR/"}
    if [[ "$RELATIVE_PATH" != "$INSTALL_DIR" ]]; then
        info "Detected nested install path, adjusting to avoid duplication"
        INSTALL_DIR="$CURRENT_DIR"
    fi
fi

# Set URLs for downloads with mirrors
# Primary URLs
GCC_PRIMARY_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"
BINUTILS_PRIMARY_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz"

# Fallback URLs (mirror sites)
GCC_FALLBACK_URL="https://mirrors.kernel.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"
BINUTILS_FALLBACK_URL="https://mirrors.kernel.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz"

# Set initial URLs to primary
GCC_URL="$GCC_PRIMARY_URL"
BINUTILS_URL="$BINUTILS_PRIMARY_URL"

# Determine number of CPU cores for parallel build
if command -v nproc >/dev/null 2>&1; then
    NUM_CORES=$(nproc)
elif command -v sysctl >/dev/null 2>&1 && sysctl -n hw.ncpu >/dev/null 2>&1; then
    # For macOS
    NUM_CORES=$(sysctl -n hw.ncpu)
else
    # Default to 2 cores if we can't determine
    NUM_CORES=2
fi

# Use the GCC_CONFIGURE_OPTIONS environment variable with defaults
# Note: We disable various optional components to make the build more robust
GCC_CONFIGURE_OPTIONS=${GCC_CONFIGURE_OPTIONS:-"\
    --disable-shared \
    --disable-threads \
    --disable-libmudflap \
    --disable-libssp \
    --disable-libgomp \
    --disable-libquadmath \
    --disable-libatomic \
    --disable-libitm \
    --disable-libvtv \
    --enable-languages=c,c++ \
    --without-newlib \
    --disable-nls \
    --disable-bootstrap \
    --enable-multilib \
    --disable-libstdcxx \
    --with-headers \
    --disable-libcc1 \
    --disable-werror \
    --disable-libsanitizer \
    --disable-plugin \
    --disable-gdb \
    --disable-libdecnumber"}

# Use the BINUTILS_CONFIGURE_OPTIONS environment variable with defaults
# Note: --disable-debuginfod is important to prevent build failures when libdebuginfod is missing
BINUTILS_CONFIGURE_OPTIONS=${BINUTILS_CONFIGURE_OPTIONS:-"\
    --disable-nls \
    --enable-multilib \
    --disable-werror \
    --disable-dependency-tracking \
    --disable-debuginfod \
    --with-isl"}

info "${TARGET} toolchain build starting."
info "GCC version: ${GCC_VERSION}"
info "Binutils version: ${BINUTILS_VERSION}"
info "Target: ${TARGET}"
info "Install directory: ${INSTALL_DIR}"
info "Using ${NUM_CORES} CPU cores for parallel build"

# Create base directories
WORK_DIR="$(pwd)"
SRC_DIR="${WORK_DIR}/src"
BUILD_DIR="${WORK_DIR}/build"

mkdir -p "${SRC_DIR}" || error_exit "Failed to create source directory"
mkdir -p "${BUILD_DIR}" || error_exit "Failed to create build directory"
mkdir -p "${INSTALL_DIR}" || error_exit "Failed to create install directory"

# Download and extract source code
cd "${SRC_DIR}" || error_exit "Failed to change to source directory"

# Function to try downloading from primary URL, then fallback if needed
try_download() {
    local primary_url="$1"
    local fallback_url="$2"
    local output_file="$3"
    local name="$4"

    # Check if the file already exists in the output location
    if [ -f "$output_file" ] && [ -s "$output_file" ]; then
        info "File $output_file already exists and is not empty, skipping download"
        return 0
    fi

    # Try to use a local copy first if it exists in the current directory
    local filename=$(basename "$output_file")
    if [ -f "./$filename" ] && [ -s "./$filename" ]; then
        info "Found local copy of $filename, copying instead of downloading"
        cp "./$filename" "$output_file" || error_exit "Failed to copy local file $filename to $output_file"
        return 0
    fi

    # Try to use a local copy from common locations
    for dir in "/tmp" "$HOME/Downloads" "/var/tmp"; do
        if [ -f "$dir/$filename" ] && [ -s "$dir/$filename" ]; then
            info "Found local copy of $filename in $dir, copying instead of downloading"
            cp "$dir/$filename" "$output_file" || error_exit "Failed to copy local file $dir/$filename to $output_file"
            return 0
        fi
    done

    # Try direct download without checking first (sometimes --spider fails but direct download works)
    info "Attempting to download $name directly from primary URL"
    if wget -v -t 2 -T 30 "$primary_url" -O "$output_file" 2>&1; then
        info "Successfully downloaded $name from primary URL"
        return 0
    fi

    info "Primary URL direct download failed, trying fallback URL for $name"
    if wget -v -t 2 -T 30 "$fallback_url" -O "$output_file" 2>&1; then
        info "Successfully downloaded $name from fallback URL"
        return 0
    fi

    # If both direct downloads failed, try older versions
    info "Both primary and fallback URLs failed, trying older versions"
    local version_parts=(${5//./ })
    local major=${version_parts[0]}
    local minor=${version_parts[1]}
    local patch=${version_parts[2]:-0}

    # Try a few older versions
    for ((i=0; i<3; i++)); do
        if ((patch > 0)); then
            patch=$((patch - 1))
        elif ((minor > 0)); then
            minor=$((minor - 1))
            patch=9  # Assume max patch is 9
        else
            major=$((major - 1))
            minor=9  # Assume max minor is 9
            patch=9  # Assume max patch is 9
        fi

        local older_version="${major}.${minor}"
        if [ -n "$patch" ] && [ "$patch" -ne 0 ]; then
            older_version="${older_version}.${patch}"
        fi

        local older_url=""
        if [[ "$name" == "binutils" ]]; then
            older_url="https://ftp.gnu.org/gnu/binutils/binutils-${older_version}.tar.gz"
        else
            older_url="https://ftp.gnu.org/gnu/gcc/gcc-${older_version}/gcc-${older_version}.tar.gz"
        fi

        info "Trying older version: $older_version"
        if wget -v -t 2 -T 30 "$older_url" -O "$output_file" 2>&1; then
            info "Successfully downloaded $name version $older_version"
            if [[ "$name" == "binutils" ]]; then
                BINUTILS_VERSION="$older_version"
                BINUTILS_TARBALL="binutils-${BINUTILS_VERSION}.tar.gz"
                BINUTILS_SRC_DIR="${SRC_DIR}/binutils-${BINUTILS_VERSION}"
            else
                GCC_VERSION="$older_version"
                GCC_TARBALL="gcc-${GCC_VERSION}.tar.gz"
                GCC_SRC_DIR="${SRC_DIR}/gcc-${GCC_VERSION}"
            fi
            return 0
        fi
    done

    # If we get here, all download attempts failed
    error_exit "Failed to download $name from all URLs and versions tried"
}

# Try using curl if wget fails
if ! command -v wget &> /dev/null; then
    info "wget not found, checking for curl"
    if command -v curl &> /dev/null; then
        info "Using curl instead of wget"
        # Redefine download_file to use curl
        download_file() {
            local url="$1"
            local output_file="$2"

            # Ensure the directory exists
            local output_dir=$(dirname "$output_file")
            mkdir -p "$output_dir" || error_exit "Failed to create directory for download: $output_dir"

            info "Download URL: $url"
            info "Output file: $output_file"

            if [ -f "$output_file" ] && [ -s "$output_file" ]; then
                info "Resuming download of $output_file"
                curl -L -C - "$url" -o "$output_file" || error_exit "Failed to download $url to $output_file"
            else
                info "Downloading $output_file"
                curl -L "$url" -o "$output_file" || error_exit "Failed to download $url to $output_file"
            fi

            # Verify the download was successful
            if [ ! -s "$output_file" ]; then
                error_exit "Download completed but file is empty: $output_file"
            fi

            info "Download completed successfully: $output_file"
        }
    else
        error_exit "Neither wget nor curl is available. Please install one of them and try again."
    fi
fi

# Use known working versions if specified versions fail
BINUTILS_KNOWN_WORKING_VERSION="2.40"
GCC_KNOWN_WORKING_VERSION="12.3.0"

# Handle binutils
BINUTILS_TARBALL="binutils-${BINUTILS_VERSION}.tar.gz"
BINUTILS_SRC_DIR="${SRC_DIR}/binutils-${BINUTILS_VERSION}"

# Check if binutils directory already exists
if [ -d "${BINUTILS_SRC_DIR}" ]; then
    info "Binutils directory ${BINUTILS_SRC_DIR} already exists, skipping download and extraction"
else
    # Check if we have a different version already extracted
    existing_binutils_dir=$(find "${SRC_DIR}" -maxdepth 1 -type d -name "binutils-*" | sort -V | tail -n 1)
    if [ -n "$existing_binutils_dir" ]; then
        # Use existing directory
        existing_binutils_version=$(basename "$existing_binutils_dir" | sed 's/binutils-//')
        info "Found existing binutils version $existing_binutils_version"
        BINUTILS_VERSION="$existing_binutils_version"
        BINUTILS_TARBALL="binutils-${BINUTILS_VERSION}.tar.gz"
        BINUTILS_SRC_DIR="${SRC_DIR}/binutils-${BINUTILS_VERSION}"
        info "Using existing binutils directory: ${BINUTILS_SRC_DIR}"
    else
        # Check if we have the tarball already
        if [ -f "${SRC_DIR}/${BINUTILS_TARBALL}" ]; then
            info "Binutils tarball ${BINUTILS_TARBALL} already exists, skipping download"
        else
            # Try to download binutils
            info "Attempting to download binutils version ${BINUTILS_VERSION}"
            if ! try_download "$BINUTILS_PRIMARY_URL" "$BINUTILS_FALLBACK_URL" "${SRC_DIR}/${BINUTILS_TARBALL}" "binutils" "$BINUTILS_VERSION"; then
                # If all attempts failed, try the known working version
                info "All download attempts failed, trying known working binutils version ${BINUTILS_KNOWN_WORKING_VERSION}"
                BINUTILS_VERSION="${BINUTILS_KNOWN_WORKING_VERSION}"
                BINUTILS_TARBALL="binutils-${BINUTILS_VERSION}.tar.gz"
                BINUTILS_SRC_DIR="${SRC_DIR}/binutils-${BINUTILS_VERSION}"
                BINUTILS_PRIMARY_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz"
                BINUTILS_FALLBACK_URL="https://mirrors.kernel.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz"

                if ! try_download "$BINUTILS_PRIMARY_URL" "$BINUTILS_FALLBACK_URL" "${SRC_DIR}/${BINUTILS_TARBALL}" "binutils" "$BINUTILS_VERSION"; then
                    error_exit "Failed to download binutils even with known working version"
                fi
            fi
        fi

        # Extract binutils
        extract_tarball "${SRC_DIR}/${BINUTILS_TARBALL}" "${BINUTILS_SRC_DIR}"
    fi
fi

# Handle GCC
GCC_TARBALL="gcc-${GCC_VERSION}.tar.gz"
GCC_SRC_DIR="${SRC_DIR}/gcc-${GCC_VERSION}"

# Check if GCC directory already exists
if [ -d "${GCC_SRC_DIR}" ]; then
    info "GCC directory ${GCC_SRC_DIR} already exists, skipping download and extraction"
else
    # Check if we have a different version already extracted
    existing_gcc_dir=$(find "${SRC_DIR}" -maxdepth 1 -type d -name "gcc-*" | sort -V | tail -n 1)
    if [ -n "$existing_gcc_dir" ]; then
        # Use existing directory
        existing_gcc_version=$(basename "$existing_gcc_dir" | sed 's/gcc-//')
        info "Found existing GCC version $existing_gcc_version"
        GCC_VERSION="$existing_gcc_version"
        GCC_TARBALL="gcc-${GCC_VERSION}.tar.gz"
        GCC_SRC_DIR="${SRC_DIR}/gcc-${GCC_VERSION}"
        info "Using existing GCC directory: ${GCC_SRC_DIR}"
    else
        # Check if we have the tarball already
        if [ -f "${SRC_DIR}/${GCC_TARBALL}" ]; then
            info "GCC tarball ${GCC_TARBALL} already exists, skipping download"
        else
            # Try to download GCC
            info "Attempting to download GCC version ${GCC_VERSION}"
            if ! try_download "$GCC_PRIMARY_URL" "$GCC_FALLBACK_URL" "${SRC_DIR}/${GCC_TARBALL}" "gcc" "$GCC_VERSION"; then
                # If all attempts failed, try the known working version
                info "All download attempts failed, trying known working GCC version ${GCC_KNOWN_WORKING_VERSION}"
                GCC_VERSION="${GCC_KNOWN_WORKING_VERSION}"
                GCC_TARBALL="gcc-${GCC_VERSION}.tar.gz"
                GCC_SRC_DIR="${SRC_DIR}/gcc-${GCC_VERSION}"
                GCC_PRIMARY_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"
                GCC_FALLBACK_URL="https://mirrors.kernel.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"

                if ! try_download "$GCC_PRIMARY_URL" "$GCC_FALLBACK_URL" "${SRC_DIR}/${GCC_TARBALL}" "gcc" "$GCC_VERSION"; then
                    error_exit "Failed to download GCC even with known working version"
                fi
            fi
        fi

        # Extract GCC
        extract_tarball "${SRC_DIR}/${GCC_TARBALL}" "${GCC_SRC_DIR}"
    fi
fi

# Handle GCC prerequisites
info "Checking GCC prerequisites"
cd "${GCC_SRC_DIR}" || error_exit "Failed to change to GCC source directory"
contrib/download_prerequisites || error_exit "Failed to download GCC prerequisites"

# Verify prerequisites
missing_prereqs=0
for prereq in mpfr gmp mpc isl; do
    if [ ! -d "./$prereq" ]; then
        error_exit "Missing required prerequisite: $prereq"
    else
        info "Found prerequisite: $prereq"
    fi
done

if [ $missing_prereqs -eq 1 ]; then
    warn "Some prerequisites are missing. The build may fail."
else
    info "All required prerequisites are present."
fi

# Build and install binutils
info "Building binutils"
BINUTILS_BUILD_DIR="${BUILD_DIR}/binutils-build"
mkdir -p "${BINUTILS_BUILD_DIR}" || error_exit "Failed to create binutils build directory"
cd "${BINUTILS_BUILD_DIR}" || error_exit "Failed to change to binutils build directory"

# Check if we have sudo access
HAS_SUDO=0
if command -v sudo &> /dev/null; then
    if sudo -n true 2>/dev/null; then
        HAS_SUDO=1
    fi
fi

# Function to install a package
install_package() {
    local package_name="$1"

    if command -v apt-get &> /dev/null; then
        if [ $HAS_SUDO -eq 1 ]; then
            sudo apt-get update -qq && sudo apt-get install -y "$package_name" || warn "Failed to install $package_name with sudo, configure may fail"
        else
            apt-get update -qq && apt-get install -y "$package_name" || warn "Failed to install $package_name, configure may fail"
        fi
    elif command -v yum &> /dev/null; then
        if [ $HAS_SUDO -eq 1 ]; then
            sudo yum install -y "$package_name" || warn "Failed to install $package_name with sudo, configure may fail"
        else
            yum install -y "$package_name" || warn "Failed to install $package_name, configure may fail"
        fi
    elif command -v brew &> /dev/null; then
        brew install "$package_name" || warn "Failed to install $package_name, configure may fail"
    else
        warn "Could not install $package_name, configure may fail"
    fi
}

# Install required packages if they're missing
MISSING_PACKAGES=""

# Check for makeinfo
if ! command -v makeinfo &> /dev/null; then
    MISSING_PACKAGES="$MISSING_PACKAGES texinfo"
fi

# Check for file command
if ! command -v file &> /dev/null; then
    MISSING_PACKAGES="$MISSING_PACKAGES file"
fi

# Check for other potentially required tools
for cmd in bison flex m4; do
    if ! command -v $cmd &> /dev/null; then
        MISSING_PACKAGES="$MISSING_PACKAGES $cmd"
    fi
done

# Always try to install ISL development package
if command -v apt-get &> /dev/null; then
    info "Attempting to install libisl-dev package"
    if [ $HAS_SUDO -eq 1 ]; then
        sudo apt-get update -qq && sudo apt-get install -y libisl-dev || warn "Failed to install libisl-dev with sudo"
    else
        apt-get update -qq && apt-get install -y libisl-dev || warn "Failed to install libisl-dev"
    fi
elif command -v yum &> /dev/null; then
    info "Attempting to install isl-devel package"
    if [ $HAS_SUDO -eq 1 ]; then
        sudo yum install -y isl-devel || warn "Failed to install isl-devel with sudo"
    else
        yum install -y isl-devel || warn "Failed to install isl-devel"
    fi
elif command -v brew &> /dev/null; then
    info "Attempting to install isl package"
    brew install isl || warn "Failed to install isl"
fi

# Check for other missing packages
if ! pkg-config --exists isl 2>/dev/null; then
    if command -v apt-get &> /dev/null; then
        MISSING_PACKAGES="$MISSING_PACKAGES libisl-dev"
    elif command -v yum &> /dev/null; then
        MISSING_PACKAGES="$MISSING_PACKAGES isl-devel"
    elif command -v brew &> /dev/null; then
        MISSING_PACKAGES="$MISSING_PACKAGES isl"
    fi
fi

# Install missing packages
if [ -n "$MISSING_PACKAGES" ]; then
    warn "Missing required tools: $MISSING_PACKAGES. Attempting to install..."
    for package in $MISSING_PACKAGES; do
        install_package "$package"
    done
fi

# Create a dummy makeinfo script if it's still missing
if ! command -v makeinfo &> /dev/null; then
    warn "Creating dummy makeinfo script"
    DUMMY_MAKEINFO="${BUILD_DIR}/makeinfo"
    echo '#!/bin/sh' > "${DUMMY_MAKEINFO}"
    echo 'echo "Dummy makeinfo, ignoring request to generate documentation"' >> "${DUMMY_MAKEINFO}"
    echo 'exit 0' >> "${DUMMY_MAKEINFO}"
    chmod +x "${DUMMY_MAKEINFO}"
    export PATH="${BUILD_DIR}:${PATH}"
fi

# Try to find system ISL
ISL_SYSTEM_PATH=""
if pkg-config --exists isl; then
    ISL_SYSTEM_PATH=$(pkg-config --variable=prefix isl)
    ISL_VERSION=$(pkg-config --modversion isl)
    info "Found system ISL version $ISL_VERSION at $ISL_SYSTEM_PATH"

    # Create a symlink to the system ISL in the build directory
    mkdir -p "${BUILD_DIR}/isl"
    ln -sf "${ISL_SYSTEM_PATH}/include/isl" "${BUILD_DIR}/isl/include"
    ln -sf "${ISL_SYSTEM_PATH}/lib/libisl.so" "${BUILD_DIR}/isl/lib"

    # Add the ISL directory to the library path
    export LD_LIBRARY_PATH="${ISL_SYSTEM_PATH}/lib:${LD_LIBRARY_PATH}"
    export LIBRARY_PATH="${ISL_SYSTEM_PATH}/lib:${LIBRARY_PATH}"
    export C_INCLUDE_PATH="${ISL_SYSTEM_PATH}/include:${C_INCLUDE_PATH}"
    export CPLUS_INCLUDE_PATH="${ISL_SYSTEM_PATH}/include:${CPLUS_INCLUDE_PATH}"

    # Use pkg-config to get ISL CFLAGS and LDFLAGS
    ISL_CFLAGS=$(pkg-config --cflags isl)
    ISL_LDFLAGS=$(pkg-config --libs isl)

    # Add ISL flags to CFLAGS and LDFLAGS
    export CFLAGS="${CFLAGS} ${ISL_CFLAGS}"
    export LDFLAGS="${LDFLAGS} ${ISL_LDFLAGS}"

    info "Set up environment for ISL: CFLAGS=${CFLAGS}, LDFLAGS=${LDFLAGS}"
fi

# Configure binutils
"${BINUTILS_SRC_DIR}/configure" --target="${TARGET}" --prefix="${INSTALL_DIR}" ${BINUTILS_CONFIGURE_OPTIONS} || {
    # If configure fails, try again with additional options
    warn "Binutils configure failed, trying with additional options"
    "${BINUTILS_SRC_DIR}/configure" --target="${TARGET}" --prefix="${INSTALL_DIR}" ${BINUTILS_CONFIGURE_OPTIONS} --disable-werror || error_exit "Failed to configure binutils"
}
make -j"${NUM_CORES}" || error_exit "Failed to build binutils"
make install || error_exit "Failed to install binutils"

# Add binutils to PATH for GCC build
export PATH="${INSTALL_DIR}/bin:${PATH}"

# Build and install GCC
info "Building GCC"
GCC_BUILD_DIR="${BUILD_DIR}/gcc-build"
mkdir -p "${GCC_BUILD_DIR}" || error_exit "Failed to create GCC build directory"
cd "${GCC_BUILD_DIR}" || error_exit "Failed to change to GCC build directory"

# Configure GCC with explicit ISL paths
if [ -n "$ISL_SYSTEM_PATH" ]; then
    "${GCC_SRC_DIR}/configure" --target="${TARGET}" --prefix="${INSTALL_DIR}" ${GCC_CONFIGURE_OPTIONS} --with-isl="$ISL_SYSTEM_PATH" || {
        # If configure fails, try again with additional options
        warn "GCC configure failed, trying with additional options"
        "${GCC_SRC_DIR}/configure" --target="${TARGET}" --prefix="${INSTALL_DIR}" ${GCC_CONFIGURE_OPTIONS} --with-isl="$ISL_SYSTEM_PATH" --disable-werror || error_exit "Failed to configure GCC"
    }
else
    "${GCC_SRC_DIR}/configure" --target="${TARGET}" --prefix="${INSTALL_DIR}" ${GCC_CONFIGURE_OPTIONS} || {
        # If configure fails, try again with additional options
        warn "GCC configure failed, trying with additional options"
        "${GCC_SRC_DIR}/configure" --target="${TARGET}" --prefix="${INSTALL_DIR}" ${GCC_CONFIGURE_OPTIONS} --disable-werror || error_exit "Failed to configure GCC"
    }
fi
make -j"${NUM_CORES}" || error_exit "Failed to build GCC"
make install || error_exit "Failed to install GCC"

info "${TARGET} toolchain installation completed successfully."
info "Toolchain installed to: ${INSTALL_DIR}"
info "You can add it to your PATH with: export PATH=\"${INSTALL_DIR}/bin:\$PATH\""

# Return to original directory
cd "${WORK_DIR}" || true