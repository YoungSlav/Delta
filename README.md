# Delta

Overview
- Cross-platform Vulkan app built with CMake. VS Code tasks/launch use CMake presets for macOS and Windows.
- Third-party dependencies (GLFW, Assimp, zlib via Assimp) are built with the provided script.
- GLSL shaders compile to SPIR-V during the CMake build.

Requirements
- CMake 3.20+
- A C++20 compiler
  - macOS: Xcode Command Line Tools (clang) or full Xcode
  - Windows: Visual Studio 2022 (Desktop development with C++)
- Vulkan SDK installed
  - macOS: MoltenVK via LunarG Vulkan SDK
  - Windows: LunarG Vulkan SDK (installer sets environment variables)
- glslc (comes with Vulkan SDK)

Getting Started (VS Code)
- Open the repo in VS Code. The provided tasks/launch configs are under `.vscode`.
- All launches use LLDB (macOS and Windows) for a consistent debugger experience and colored logs.
- macOS
  - Use launch: `Run (macOS, Debug)` or `Run (macOS, Release)`.
  - Pre-launch does: build deps → `cmake --preset macos-<config>` → build.
  - Debugger output: launch uses the Integrated Terminal and sets `FORCE_COLOR=1`, so ANSI log colors render. View output in the Terminal panel (not Debug Console).
  - Vulkan env: `.vscode/vulkan.macos.env` (absolute SDK paths).
- Windows
  - Use launch: `Run (Windows x64, Debug)` or `Run (Windows x64, Release)`.
  - Pre-launch does: build deps → `cmake --preset windows-vs-<config>` → build.
  - Debugger output: LLDB runs in the Integrated Terminal and sets `FORCE_COLOR=1`, so ANSI log colors render. View output in the Terminal panel (not Debug Console).
  - Vulkan env: `.vscode/vulkan.win.env` (usually empty; the SDK installer sets env vars).

Dependencies
- GLFW and Assimp are built automatically by CMake (via `add_subdirectory(ThirdParty/...)`).
- No separate scripts are needed. First build per config will compile them.

- Building from Command Line (without VS Code)
- Configure + build with presets:
  - macOS Debug: `cmake --preset macos-debug && cmake --build --preset macos-debug`
  - macOS Release: `cmake --preset macos-release && cmake --build --preset macos-release`
  - Windows Debug: `cmake --preset windows-vs-debug && cmake --build --preset windows-vs-debug`
  - Windows Release: `cmake --preset windows-vs-release && cmake --build --preset windows-vs-release`

Shaders
- CMake compiles GLSL shaders in `Source/Shaders` to SPIR-V into `Resources/Shaders` during the build.
- If `glslc` is not found, the shader step is skipped and CMake prints a warning.

Environment
- macOS: edit `.vscode/vulkan.macos.env` to match your Vulkan SDK install path.
- Windows: `.vscode/vulkan.win.env` can be left empty if the SDK installer set `VULKAN_SDK` and PATH.

Notes
- The project previously supported building the app via a Makefile; it now uses CMake exclusively. The dependency build still uses CMake under the hood.
