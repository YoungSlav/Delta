#!/usr/bin/env bash
set -euo pipefail

CONFIG="${1:-Debug}"
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

echo "Building third-party dependencies for config: $CONFIG"

mkdir -p "$ROOT_DIR/ThirdParty/lib/$CONFIG"

# Build GLFW
echo "==> Building GLFW"
GLFW_SRC="$ROOT_DIR/ThirdParty/glfw"
GLFW_BUILD="$ROOT_DIR/ThirdParty/build/glfw/$CONFIG"
mkdir -p "$GLFW_BUILD"
cmake -S "$GLFW_SRC" -B "$GLFW_BUILD" \
  -DCMAKE_BUILD_TYPE=$CONFIG \
  -DBUILD_SHARED_LIBS=OFF \
  -DGLFW_BUILD_TESTS=OFF \
  -DGLFW_BUILD_EXAMPLES=OFF \
  -DGLFW_BUILD_DOCS=OFF
cmake --build "$GLFW_BUILD" --config $CONFIG --parallel

# Copy resulting library (name differs across platforms)
echo "-- Copying GLFW libs"
find "$GLFW_BUILD" \( -name "libglfw*.a" -o -name "glfw3.lib" -o -name "libglfw*.dylib" -o -name "glfw3.dll" \) \
  -exec cp {} "$ROOT_DIR/ThirdParty/lib/$CONFIG" \; || true

# Build Assimp
echo "==> Building Assimp"
ASSIMP_SRC="$ROOT_DIR/ThirdParty/assimp"
ASSIMP_BUILD="$ROOT_DIR/ThirdParty/build/assimp/$CONFIG"
mkdir -p "$ASSIMP_BUILD"
cmake -S "$ASSIMP_SRC" -B "$ASSIMP_BUILD" \
  -DCMAKE_BUILD_TYPE=$CONFIG \
  -DBUILD_SHARED_LIBS=OFF \
  -DASSIMP_BUILD_TESTS=OFF \
  -DASSIMP_BUILD_SAMPLES=OFF \
  -DASSIMP_BUILD_ZLIB=ON
cmake --build "$ASSIMP_BUILD" --config $CONFIG --parallel

echo "-- Copying Assimp libs"
find "$ASSIMP_BUILD" \( -name "libassimp*.a" -o -name "assimp*.lib" -o -name "libassimp*.dylib" -o -name "assimp*.dll" \) \
  -exec cp {} "$ROOT_DIR/ThirdParty/lib/$CONFIG" \; || true

echo "Dependencies built and copied to ThirdParty/lib/$CONFIG"
