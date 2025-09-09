#!/bin/bash

set -e # Exit on first error

# Variables
PLUGIN_NAME="Modulee"
APP_BUNDLE="inputs/${PLUGIN_NAME}.app"
VST3_BUNDLE="inputs/${PLUGIN_NAME}.vst3"
IDENTIFIER="com.yolisses.${PLUGIN_NAME}"
VERSION="0.0.1"
INSTALLER_NAME="${PLUGIN_NAME}Installer"
OUTPUT_DIR="dist"
PKG_DIR="pkg_temp"
LOG_FILE="log.txt"

# Redirect output to log file and terminal
exec > >(tee -a "${LOG_FILE}") 2>&1

echo "Starting installer creation for ${PLUGIN_NAME}"

# Create temporary directories
echo "Creating directories..."
mkdir -p "${PKG_DIR}/Applications" || { echo "Failed to create Applications directory"; exit 1; }
mkdir -p "${PKG_DIR}/Library/Audio/Plug-Ins/VST3" || { echo "Failed to create VST3 directory"; exit 1; }
mkdir -p "${OUTPUT_DIR}" || { echo "Failed to create output directory"; exit 1; }

# Copy the standalone app and VST3 bundle
echo "Copying ${APP_BUNDLE} to Applications..."
cp -R "${APP_BUNDLE}" "${PKG_DIR}/Applications/" || { echo "Failed to copy ${APP_BUNDLE}"; exit 1; }

echo "Copying ${VST3_BUNDLE} to VST3 directory..."
cp -R "${VST3_BUNDLE}" "${PKG_DIR}/Library/Audio/Plug-Ins/VST3/" || { echo "Failed to copy ${VST3_BUNDLE}"; exit 1; }

# Build component packages with different names
echo "Building application package..."
pkgbuild --root "${PKG_DIR}/Applications" \
         --identifier "${IDENTIFIER}.app" \
         --version "${VERSION}" \
         --install-location /Applications \
         "${OUTPUT_DIR}/${PLUGIN_NAME}_app.pkg" || { echo "Failed to build application package"; exit 1; }

echo "Building VST3 package..."
pkgbuild --root "${PKG_DIR}/Library/Audio/Plug-Ins/VST3" \
         --identifier "${IDENTIFIER}.vst3" \
         --version "${VERSION}" \
         --install-location /Library/Audio/Plug-Ins/VST3 \
         "${OUTPUT_DIR}/${PLUGIN_NAME}_vst3.pkg" || { echo "Failed to build VST3 package"; exit 1; }

# Create distribution.xml
echo "Creating distribution.xml..."
cat << EOF > distribution.xml || { echo "Failed to create distribution.xml"; exit 1; }
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>${PLUGIN_NAME} Installer</title>
    <pkg-ref id="${IDENTIFIER}.app" version="${VERSION}" onConclusion="none">${PLUGIN_NAME}_app.pkg</pkg-ref>
    <pkg-ref id="${IDENTIFIER}.vst3" version="${VERSION}" onConclusion="none">${PLUGIN_NAME}_vst3.pkg</pkg-ref>
    <options customize="never" require-scripts="false"/>
    <choices-outline>
        <line choice="app_choice"/>
        <line choice="vst3_choice"/>
    </choices-outline>
    <choice id="app_choice" title="${PLUGIN_NAME} Application" description="Standalone application">
        <pkg-ref id="${IDENTIFIER}.app"/>
    </choice>
    <choice id="vst3_choice" title="${PLUGIN_NAME} VST3 Plugin" description="VST3 plugin for DAWs">
        <pkg-ref id="${IDENTIFIER}.vst3"/>
    </choice>
</installer-gui-script>
EOF

# Create the final installer
echo "Creating final installer..."
cd "${OUTPUT_DIR}"
productbuild --distribution ../distribution.xml \
             --package-path . \
             "${INSTALLER_NAME}.pkg" || { echo "Failed to create final installer"; exit 1; }
cd ..

# Clean up
echo "Cleaning up temporary files..."
rm -rf "${PKG_DIR}" || { echo "Failed to clean up ${PKG_DIR}"; exit 1; }
rm distribution.xml || { echo "Failed to clean up distribution.xml"; exit 1; }
rm "${OUTPUT_DIR}/${PLUGIN_NAME}_app.pkg" "${OUTPUT_DIR}/${PLUGIN_NAME}_vst3.pkg" || { echo "Failed to clean up component packages"; exit 1; }

echo "Installer created at ${OUTPUT_DIR}/${INSTALLER_NAME}.pkg"
echo "Log saved to ${LOG_FILE}"
