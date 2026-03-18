#!/usr/bin/env bash
set -euo pipefail

# Build the native SerialTransport library for Linux (ARM/ARM64) using the Mercury API sources.
# Run this on the Raspberry Pi (or other target platform) where you want to use the ThingMagic reader.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/mercuryapi-1.37.0.80/c/proj/jni"

# Build the native library. Adjust Makefile.jni if you need to point at a different JDK.
make -f Makefile.jni

# Copy the result into the java/ directory and name it as the Mercury API expects.
# On ARM64, the API looks for "linux-aarch64.lib".
cp -f libSerialTransportNative.so.0 "$SCRIPT_DIR/mercuryapi-1.37.0.80/java/linux-aarch64.lib"

echo "Built native library and installed as linux-aarch64.lib"
