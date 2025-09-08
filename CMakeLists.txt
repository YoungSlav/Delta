cmake_minimum_required(VERSION 3.20)
project(Delta LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/Source)

file(GLOB_RECURSE SRC_FILES
  ${SRC_DIR}/Private/*.cpp
)

add_executable(Delta ${SRC_FILES})

target_include_directories(Delta PRIVATE
  ${CMAKE_SOURCE_DIR}/Source/Public
  ${CMAKE_SOURCE_DIR}/Source
  # Third-party headers (delegates/, stb/, etc.) now live directly under ThirdParty/
  ${CMAKE_SOURCE_DIR}/ThirdParty
  ${CMAKE_SOURCE_DIR}/ThirdParty/glm
)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(Delta PRIVATE Vulkan::Vulkan)

### Third-party: build from subdirectories for portability
# GLFW
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/glfw EXCLUDE_FROM_ALL)
target_link_libraries(Delta PRIVATE glfw)

# Assimp (build static, no tools/tests)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/ThirdParty/assimp EXCLUDE_FROM_ALL)
target_link_libraries(Delta PRIVATE assimp)

if(APPLE)
  # Frameworks needed by GLFW
  target_link_libraries(Delta PRIVATE
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
    "-framework QuartzCore"
    "-framework Metal"
  )
  # Embed rpaths so libvulkan.dylib can be found without DYLD envs
  if(DEFINED ENV{VULKAN_SDK})
    set(CMAKE_BUILD_RPATH "$ENV{VULKAN_SDK}/macOS/lib;$ENV{VULKAN_SDK}/lib")
  endif()
endif()

# Shader compilation (glslc)
set(SHADER_SRC_DIR ${SRC_DIR}/Shaders)
set(SHADER_OUT_DIR ${CMAKE_SOURCE_DIR}/Resources/Shaders)
file(GLOB SHADERS ${SHADER_SRC_DIR}/*)

# Locate glslc reliably (PATH + SDK hints)
if(WIN32)
  find_program(GLSLC_TOOL glslc.exe
    HINTS
      "$ENV{VULKAN_SDK}/Bin"
    PATHS
      "$ENV{VULKAN_SDK}/Bin"
    PATH_SUFFIXES bin Bin
  )
else()
  find_program(GLSLC_TOOL glslc
    HINTS
      "$ENV{VULKAN_SDK}/macOS/bin"
      "$ENV{VULKAN_SDK}/bin"
    PATHS
      "$ENV{VULKAN_SDK}/macOS/bin"
      "$ENV{VULKAN_SDK}/bin"
    PATH_SUFFIXES bin
  )
endif()

if(NOT GLSLC_TOOL)
  message(WARNING "glslc not found in PATH or VULKAN_SDK; shader compilation will be skipped")
else()
  add_custom_target(compile_shaders ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUT_DIR})
  foreach(SH ${SHADERS})
    get_filename_component(SH_NAME ${SH} NAME)
    add_custom_command(TARGET compile_shaders POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "Compiling ${SH_NAME}"
      COMMAND ${GLSLC_TOOL} ${SH} -o ${SHADER_OUT_DIR}/${SH_NAME}.spv
      VERBATIM
    )
  endforeach()
  add_dependencies(Delta compile_shaders)
endif()

if(WIN32)
  # Static GLFW on Windows requires linking Win32 system libs.
  target_link_libraries(Delta PRIVATE
    user32
    gdi32
    shell32
    kernel32
    ws2_32
    winmm
  )

  # Ensure Vulkan loader is available next to the executable when VULKAN_SDK is set.
  if(DEFINED ENV{VULKAN_SDK})
    set(VK_DLL "$ENV{VULKAN_SDK}/Bin/vulkan-1.dll")
    if(EXISTS ${VK_DLL})
      # Place runtime DLL in the same output dir as the executable for each config
      if(CMAKE_CONFIGURATION_TYPES)
        foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
          string(TOUPPER ${cfg} CFG_UPPER)
          add_custom_command(TARGET Delta POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${VK_DLL}"
              "$<TARGET_FILE_DIR:Delta>"
            VERBATIM
            COMMENT "Copying Vulkan loader (vulkan-1.dll) to output directory for ${cfg}"
          )
        endforeach()
      else()
        add_custom_command(TARGET Delta POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VK_DLL}"
            "$<TARGET_FILE_DIR:Delta>"
          VERBATIM
          COMMENT "Copying Vulkan loader (vulkan-1.dll) to output directory"
        )
      endif()
    endif()
  endif()
endif()
