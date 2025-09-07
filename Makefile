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
  -IThirdParty/include \
  -IThirdParty/glm \
  -IThirdParty/glfw/include \
  -IThirdParty/assimp/include

# Vulkan SDK include/library if available
ifdef VULKAN_SDK
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

