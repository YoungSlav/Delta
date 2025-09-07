## Cross-platform Makefile for Delta

# Configuration
CONFIG ?= Debug
OS := $(shell uname -s)
ARCH := $(shell uname -m)

# Map ARCH to folder names similar to VS
ifeq ($(ARCH),x86_64)
  PLATFORM := x64
else ifeq ($(ARCH),arm64)
  PLATFORM := arm64
else
  PLATFORM := $(ARCH)
endif

OUT_DIR := Build/$(PLATFORM)/$(CONFIG)
INT_DIR := Intermediate/$(PLATFORM)/$(CONFIG)/Delta

TARGET := $(OUT_DIR)/Delta

# Sources and includes
SRC_DIR := Delta/Source
SRC := $(wildcard $(SRC_DIR)/Private/*.cpp)

INCLUDES := \
  -IDelta \
  -IDelta/Source \
  -IDelta/Source/Public \
  -IThirdParty/glm \
  -IThirdParty/glfw/include \
  -IThirdParty/assimp/include \
  -IThirdParty/include

# Vulkan SDK detection
GLSLC_PATH := $(shell command -v glslc 2>/dev/null)
ifeq ($(VULKAN_SDK),)
  ifneq ($(GLSLC_PATH),)
    VULKAN_SDK_C1 := $(abspath $(dir $(dir $(GLSLC_PATH))))
    VULKAN_SDK_C2 := $(abspath $(dir $(VULKAN_SDK_C1)))
    VULKAN_SDK := $(shell [ -f "$(VULKAN_SDK_C1)/Include/vulkan/vulkan.h" ] && echo $(VULKAN_SDK_C1) || ([ -f "$(VULKAN_SDK_C2)/Include/vulkan/vulkan.h" ] && echo $(VULKAN_SDK_C2)))
  endif
endif

# Vulkan SDK include/library if available (env or auto-detected)
ifneq ($(VULKAN_SDK),)
  INCLUDES += -I$(VULKAN_SDK)/Include -I$(VULKAN_SDK)/include
  LDFLAGS += -L$(VULKAN_SDK)/Lib -L$(VULKAN_SDK)/lib
endif

OBJ := $(patsubst $(SRC_DIR)/Private/%.cpp,$(INT_DIR)/%.o,$(SRC))

CXX ?= c++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wno-unused-parameter $(INCLUDES)
LDFLAGS += -LThirdParty/lib/$(CONFIG)

# Platform specific flags
ifeq ($(CONFIG),Debug)
  CXXFLAGS += -g -O0 -D_DEBUG
else
  CXXFLAGS += -O2 -DNDEBUG
endif

LIBS :=

ifeq ($(OS),Darwin)
  # macOS: frameworks needed by GLFW
  LIBS += -framework Cocoa -framework IOKit -framework CoreVideo
  # Try both names to support different GLFW builds
  LIBS += -lglfw -lglfw3
  LIBS += -lassimp
  # Vulkan loader on macOS typically via Vulkan SDK / MoltenVK
  LIBS += -lvulkan
else ifeq ($(OS),Linux)
  # Linux: X11 dependencies for GLFW
  LIBS += -lglfw -ldl -lpthread -lX11 -lXrandr -lXi -lXxf86vm -lXcursor
  LIBS += -lassimp
  LIBS += -lvulkan
else
  # Other POSIX-like; leave as-is and let linker report missing pieces
  LIBS += -lglfw -lassimp -lvulkan
endif

.PHONY: all debug release clean deps shaders

all: $(TARGET)

debug:
	$(MAKE) CONFIG=Debug all

release:
	$(MAKE) CONFIG=Release all

# Link
$(TARGET): $(OBJ) | $(OUT_DIR)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LIBS)

# Compile
$(INT_DIR)/%.o: $(SRC_DIR)/Private/%.cpp | $(INT_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(INT_DIR):
	@mkdir -p $(INT_DIR)

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

clean:
	rm -rf Build/$(PLATFORM)/$(CONFIG) Intermediate/$(PLATFORM)/$(CONFIG)/Delta

# Build third-party dependencies via CMake and copy artifacts
deps:
	@bash scripts/build_deps.sh $(CONFIG)

# Compile GLSL shaders to SPIR-V
shaders:
	@bash scripts/compile_shaders.sh

# =====================
# MSVC (Windows) targets
# =====================
# Build using Microsoft Visual C++ toolchain. Run these from a Developer Command Prompt
# or any shell where vcvars are already set (CL, LINK, LIB environment configured).

ifeq ($(OS),Windows_NT)

MSVC_INT_DIR := $(INT_DIR)
MSVC_OBJ := $(patsubst $(SRC_DIR)/Private/%.cpp,$(MSVC_INT_DIR)/%.obj,$(SRC))
MSVC_INCLUDES := $(INCLUDES: -I%=/I%)

# Common MSVC flags
CL_FLAGS_COMMON := /nologo /std:c++20 /EHsc /W3 /MD
CL_FLAGS_DEBUG := /Zi /Od /D _DEBUG
CL_FLAGS_RELEASE := /O2 /DNDEBUG

LINK_FLAGS_COMMON := /nologo /SUBSYSTEM:CONSOLE
LINK_LIBPATHS := /LIBPATH:ThirdParty\\lib\\$(CONFIG)

# Library names (assimp name may vary; adjust if needed)
MSVC_LIBS_DEBUG := vulkan-1.lib glfw3.lib assimp-vc143-mtd.lib
MSVC_LIBS_RELEASE := vulkan-1.lib glfw3.lib assimp-vc143-mt.lib

.PHONY: msvc-debug msvc-release

msvc-debug: CONFIG=Debug
msvc-debug: $(TARGET).exe

msvc-release: CONFIG=Release
msvc-release: $(TARGET).exe

$(TARGET).exe: $(MSVC_OBJ) | $(OUT_DIR)
	link $(LINK_FLAGS_COMMON) $(LINK_LIBPATHS) /OUT:$@ $(MSVC_OBJ) $(MSVC_LIBS_$(CONFIG))

$(MSVC_INT_DIR)/%.obj: $(SRC_DIR)/Private/%.cpp | $(MSVC_INT_DIR)
	cl $(CL_FLAGS_COMMON) $(CL_FLAGS_$(CONFIG)) $(MSVC_INCLUDES) /Fo$@ /c $<

$(MSVC_INT_DIR):
	@mkdir -p $(MSVC_INT_DIR)

endif
