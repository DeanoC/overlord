#!/bin/bash

# Define variables
GCC_VERSION=${GCC_VERSION:-"10.2.0"}
BINUTILS_VERSION=${BINUTILS_VERSION:-"2.35"}
TARGET=${TARGET:-"riscv64-unknown-elf"}
INSTALL_DIR=${INSTALL_DIR:-"./${TARGET}-gcc"}

GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz"
BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz"
NUM_CORES=$(nproc)

# Use the GCC_CONFIGURE_OPTIONS environment variable
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
    --disable-libcc1"}

# Use the BINUTILS_CONFIGURE_OPTIONS environment variable
BINUTILS_CONFIGURE_OPTIONS=${BINUTILS_CONFIGURE_OPTIONS:-"\
    --disable-nls \
    --enable-multilib \
    --disable-werror"}

echo ${TARGET} "toolchain build starting."

# Create necessary directories
mkdir -p src
mkdir -p build

pushd src

if [ ! -f  binutils-${BINUTILS_VERSION}.tar.gz ]; then
    wget ${BINUTILS_URL}
fi
if [ ! -f  gcc-${GCC_VERSION}.tar.gz ]; then
    wget ${GCC_URL}
fi

# extract the source
if [ ! -d binutils-${BINUTILS_VERSION} ]; then
    tar -xzf binutils-${BINUTILS_VERSION}.tar.gz
fi
if [ ! -d gcc-${GCC_VERSION} ]; then
    tar -xzf gcc-${GCC_VERSION}.tar.gz
fi

# Download GCC prerequisites
pushd gcc-${GCC_VERSION}
./contrib/download_prerequisites
popd

# Build and install binutils
mkdir -p build/binutils-build
pushd build/binutils-build
binutils-${BINUTILS_VERSION}/configure --target=${TARGET} --prefix=${INSTALL_DIR} ${BINUTILS_CONFIGURE_OPTIONS}
make -j${NUM_CORES}
make install
popd

# Configure GCC
mkdir -p build/gcc-build
pushd build/gcc-build
gcc-${GCC_VERSION}/configure --target=${TARGET} --prefix=${INSTALL_DIR} ${GCC_CONFIGURE_OPTIONS}
# Build and install GCC
make -j${NUM_CORES}
make install
popd

echo ${TARGET} "toolchain installation completed."
popd