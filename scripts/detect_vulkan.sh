#!/usr/bin/env bash
set -euo pipefail

# Try to locate Vulkan SDK and export VULKAN_SDK

detect_from_glslc() {
  if command -v glslc >/dev/null 2>&1; then
    local gp
    gp="$(command -v glslc)"
    # Try two directories up
    local c1
    c1="$(cd "$(dirname "$gp")/.." && pwd)"
    if [ -f "$c1/Include/vulkan/vulkan.h" ]; then
      echo "$c1"
      return 0
    fi
    # Try three directories up (handles macOS SDK layout)
    local c2
    c2="$(cd "$(dirname "$gp")/../.." && pwd)"
    if [ -f "$c2/Include/vulkan/vulkan.h" ]; then
      echo "$c2"
      return 0
    fi
  fi
  return 1
}

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

if [ "${VULKAN_SDK:-}" != "" ] && [ -f "$VULKAN_SDK/Include/vulkan/vulkan.h" ]; then
  SDK="$VULKAN_SDK"
else
  SDK="$(detect_from_glslc || true)"
fi

if [ -z "$SDK" ]; then
  echo "Could not detect Vulkan SDK. Ensure it is installed and glslc is in PATH."
  exit 1
fi

echo "Detected Vulkan SDK at: $SDK"
ENV_FILE="$ROOT_DIR/.vulkan.env"
echo "export VULKAN_SDK=\"$SDK\"" > "$ENV_FILE"
echo "Wrote $ENV_FILE. You can source it: 'source .vulkan.env'"

