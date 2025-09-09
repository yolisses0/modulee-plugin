#!/bin/bash
set -e
exec > log.txt 2>&1  # All output goes to log.txt

# Configuration
PLUGIN_NAME="Modulee"
BUNDLE_ID="com.yolisses.${PLUGIN_NAME}"
VERSION="0.0.1"
TEAM_ID="ModuleeTeam"
INSTALLER_NAME="${PLUGIN_NAME}Installer"

# Directories
BUILD_DIR="build"
OUTPUT_DIR="output"

# Plugin formats and their installation paths
declare -a PLUGINS
PLUGINS[VST3]="Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"
PLUGINS[AU]="Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"

# Create output directories
mkdir -p $OUTPUT_DIR

# Build .pkg for each plugin format
for format in VST3 AU; do
    pkgbuild --root "${BUILD_DIR}/${format}" \
             --identifier "${BUNDLE_ID}.${format}" \
             --version "${VERSION}" \
             --install-location "/${PLUGINS[$format]}" \
             "${OUTPUT_DIR}/${PLUGIN_NAME}_${format}.pkg"
done

# Combine into final .pkg
productbuild --distribution "distribution.xml" \
             --package-path "${OUTPUT_DIR}" \
             --resources "${OUTPUT_DIR}" \
             "${OUTPUT_DIR}/${INSTALLER_NAME}.pkg"


# Create DMG
hdiutil create -volname "${PLUGIN_NAME} Installer" \
               -srcfolder "${OUTPUT_DIR}/${INSTALLER_NAME}.pkg" \
               -ov -format UDZO \
               "${OUTPUT_DIR}/${INSTALLER_NAME}.dmg"

echo "Installer created at ${OUTPUT_DIR}/${INSTALLER_NAME}.dmg"