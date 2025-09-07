#!/usr/bin/env bash
set -euo pipefail

BASE_DIR="$(cd "$(dirname "$0")/.." && pwd)"
INPUT_DIR="$BASE_DIR/Delta/Source/Shaders"
OUTPUT_DIR="$BASE_DIR/Resources/Shaders"

mkdir -p "$OUTPUT_DIR"

# Prefer glslc from PATH; fallback to VULKAN_SDK
GLSLC_BIN="${GLSLC:-}" 
if [ -z "$GLSLC_BIN" ]; then
  GLSLC_BIN="$(command -v glslc || true)"
fi
if [ -z "$GLSLC_BIN" ] && [ -n "${VULKAN_SDK:-}" ]; then
  if [ -x "$VULKAN_SDK/bin/glslc" ]; then
    GLSLC_BIN="$VULKAN_SDK/bin/glslc"
  elif [ -x "$VULKAN_SDK/Bin/glslc" ]; then
    GLSLC_BIN="$VULKAN_SDK/Bin/glslc"
  elif [ -x "$VULKAN_SDK/macOS/bin/glslc" ]; then
    GLSLC_BIN="$VULKAN_SDK/macOS/bin/glslc"
  fi
fi

if [ -z "$GLSLC_BIN" ]; then
  echo "Error: glslc not found. Install Vulkan SDK or provide GLSLC in PATH."
  exit 1
fi

echo "Compiling shaders from $INPUT_DIR to $OUTPUT_DIR using $GLSLC_BIN"

shopt -s nullglob
for f in "$INPUT_DIR"/*; do
  fname="$(basename "$f")"
  out="$OUTPUT_DIR/$fname.spv"
  "$GLSLC_BIN" "$f" -o "$out"
  echo "  $fname -> $(basename "$out")"
done

echo "Done."
