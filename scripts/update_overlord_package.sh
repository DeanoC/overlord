#!/bin/bash

# Default to dry-run mode (no replacements)
DO_REPLACE=0

# Process parameters
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <source_package> <destination_package> [--replace]"
    echo "Example: $0 Connections connections --replace"
    echo ""
    echo "Without --replace flag, shows what would change without making changes (dry run)"
    exit 1
fi

SOURCE_PKG=$1
DEST_PKG=$2

# Check for --replace flag
if [ "$#" -ge 3 ] && [ "$3" = "--replace" ]; then
    DO_REPLACE=1
    echo "REPLACE MODE ACTIVE: Changes will be applied!"
else
    echo "DRY RUN MODE: Showing what would change without making changes."
    echo "Add --replace parameter to actually perform the replacements."
fi

echo "Converting package 'com.deanoc.overlord.$SOURCE_PKG' to 'com.deanoc.overlord.$DEST_PKG'"

# Search in both main and test directories
echo "Files with package declaration using $SOURCE_PKG:"
grep -r "package com.deanoc.overlord.$SOURCE_PKG" --include="*.scala" ./src/main ./src/test || echo "No files found"

# Find all files importing from the source package
echo -e "\nFiles importing from $SOURCE_PKG package:"
grep -r "import com.deanoc.overlord.$SOURCE_PKG" --include="*.scala" ./src/main ./src/test || echo "No files found"

# Also search for wildcard imports
echo -e "\nFiles with wildcard imports from $SOURCE_PKG:"
grep -r "import com.deanoc.overlord.$SOURCE_PKG._" --include="*.scala" ./src/main ./src/test || echo "No files found"

# Fixed replacements
echo -e "\nPerforming replacements..."

# First find the files that need updating
FILES_TO_UPDATE=$(grep -r -l "com\.deanoc\.overlord\.$SOURCE_PKG" --include="*.scala" ./src/main ./src/test)

# Only run sed if we found files to update
if [ -n "$FILES_TO_UPDATE" ]; then
    if [ "$DO_REPLACE" -eq 1 ]; then
        # Actually perform replacements
        sed -i "s/com\.deanoc\.overlord\.$SOURCE_PKG/com.deanoc.overlord.$DEST_PKG/g" $FILES_TO_UPDATE
        echo "Updated package references in files."
        
        # This checks for cases where the package name might appear multiple times in a file
        DOUBLE_CHECK=$(grep -r -l "package com\.deanoc\.overlord\.$DEST_PKG" --include="*.scala" ./src/main ./src/test)
        if [ -n "$DOUBLE_CHECK" ]; then
            sed -i "s/package com\.deanoc\.overlord\.$DEST_PKG/package com.deanoc.overlord.$DEST_PKG/g" $DOUBLE_CHECK
            echo "Fixed any potential duplicate occurrences."
        fi
    else
        # Just show what would be changed
        echo "Would update these files (add --replace to actually change them):"
        for file in $FILES_TO_UPDATE; do
            echo "  $file"
        done
        echo "Would replace 'com.deanoc.overlord.$SOURCE_PKG' with 'com.deanoc.overlord.$DEST_PKG'"
    fi
else
    echo "No files found that need updating."
fi

if [ "$DO_REPLACE" -eq 1 ]; then
    echo "Package update complete."
else
    echo "Dry run complete. No changes were made."
fi
