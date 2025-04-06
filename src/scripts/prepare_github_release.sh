#!/bin/bash
#
# prepare_github_release.sh - Script to prepare Overlord for GitHub release
#
# This script builds all distribution formats of Overlord and prepares
# them for upload to GitHub releases.
#

set -e  # Exit immediately if a command exits with non-zero status

# Configuration
PROJECT_NAME="overlord"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RELEASE_DIR="${PROJECT_ROOT}/release"
VERSION=$(grep "ThisBuild / version" "${PROJECT_ROOT}/build.sbt" | sed -E 's/.*"([^"]+)".*/\1/')
DATE=$(date +%Y-%m-%d)

# Print header
echo "====================================================="
echo "  Preparing ${PROJECT_NAME} v${VERSION} for GitHub Release"
echo "====================================================="
echo "Project root: ${PROJECT_ROOT}"
echo "Release directory: ${RELEASE_DIR}"
echo "Date: ${DATE}"
echo

# Clean up any previous build and release directory
echo "Step 1: Cleaning previous builds..."
cd "${PROJECT_ROOT}"
sbt clean
rm -rf "${RELEASE_DIR}"
mkdir -p "${RELEASE_DIR}"
echo "✓ Clean completed"
echo

# Compile project
echo "Step 2: Compiling project..."
cd "${PROJECT_ROOT}"
sbt compile
echo "✓ Compilation completed"
echo

# Create Debian package
echo "Step 3: Creating Debian package..."
cd "${PROJECT_ROOT}"
sbt "Debian / packageBin"
DEB_PATH=$(find "${PROJECT_ROOT}/target" -name "*.deb" -type f)
DEB_FILENAME=$(basename "${DEB_PATH}")
echo "  Found Debian package: ${DEB_FILENAME}"
cp "${DEB_PATH}" "${RELEASE_DIR}/${DEB_FILENAME}"
chmod a+r "${RELEASE_DIR}/${DEB_FILENAME}"
echo "✓ Debian package created and copied to release directory"
echo

# Create Universal ZIP package
echo "Step 4: Creating Universal ZIP package..."
cd "${PROJECT_ROOT}"
sbt "Universal / packageBin"
ZIP_PATH=$(find "${PROJECT_ROOT}/target/universal" -name "*.zip" -type f)
ZIP_FILENAME=$(basename "${ZIP_PATH}")
echo "  Found ZIP package: ${ZIP_FILENAME}"
cp "${ZIP_PATH}" "${RELEASE_DIR}/${ZIP_FILENAME}"
echo "✓ Universal ZIP package created and copied to release directory"
echo

# Create standalone executable directory
echo "Step 5: Creating standalone executable package..."
cd "${PROJECT_ROOT}"
sbt stage
STANDALONE_DIR="${RELEASE_DIR}/${PROJECT_NAME}-${VERSION}-standalone"
mkdir -p "${STANDALONE_DIR}/bin"
mkdir -p "${STANDALONE_DIR}/lib"
cp -r "${PROJECT_ROOT}/target/universal/stage/bin/${PROJECT_NAME}" "${STANDALONE_DIR}/bin/"
cp -r "${PROJECT_ROOT}/target/universal/stage/lib/"* "${STANDALONE_DIR}/lib/"
chmod +x "${STANDALONE_DIR}/bin/${PROJECT_NAME}"
cd "${RELEASE_DIR}"
tar -czf "${PROJECT_NAME}-${VERSION}-standalone.tar.gz" "$(basename "${STANDALONE_DIR}")"
rm -rf "${STANDALONE_DIR}"
echo "✓ Standalone executable package created"
echo

# Create SHA256 checksums
echo "Step 6: Generating SHA256 checksums..."
cd "${RELEASE_DIR}"
sha256sum * > SHA256SUMS
echo "✓ SHA256 checksums generated"
echo

# Create version information file
echo "Step 7: Creating version information file..."
cat > "${RELEASE_DIR}/version-info.txt" <<EOL
${PROJECT_NAME} ${VERSION}
Release date: ${DATE}

This release contains:
- ${DEB_FILENAME}: Debian package for system-wide installation
- ${ZIP_FILENAME}: Universal ZIP package
- ${PROJECT_NAME}-${VERSION}-standalone.tar.gz: Standalone executable package

For installation instructions, see docs/cli-installation.md
EOL
echo "✓ Version information file created"
echo

# Copy documentation
echo "Step 8: Copying documentation..."
mkdir -p "${RELEASE_DIR}/docs"
cp -r "${PROJECT_ROOT}/docs/"* "${RELEASE_DIR}/docs/"
cp "${PROJECT_ROOT}/README.md" "${RELEASE_DIR}/"
cp "${PROJECT_ROOT}/LICENSE" "${RELEASE_DIR}/"
echo "✓ Documentation copied"
echo

# Final message
echo "====================================================="
echo "Release preparation complete!"
echo "====================================================="
echo
echo "GitHub release artifacts are ready in: ${RELEASE_DIR}"
echo
echo "Files prepared for release:"
ls -la "${RELEASE_DIR}"
echo
echo "Next steps:"
echo "1. Review the contents of the release directory"
echo "2. Create a new GitHub release"
echo "3. Upload all files from the release directory"
echo