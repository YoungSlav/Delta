cmake_minimum_required(VERSION 3.20)
project(Delta LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/Delta/Source)

file(GLOB_RECURSE SRC_FILES
  ${SRC_DIR}/Private/*.cpp
)

add_executable(Delta ${SRC_FILES})

target_include_directories(Delta PRIVATE
  ${CMAKE_SOURCE_DIR}/Delta/Source/Public
  ${CMAKE_SOURCE_DIR}/Delta/Source
  ${CMAKE_SOURCE_DIR}/ThirdParty/include
  ${CMAKE_SOURCE_DIR}/ThirdParty/assimp/include
  ${CMAKE_SOURCE_DIR}/ThirdParty/glfw/include
  ${CMAKE_SOURCE_DIR}/ThirdParty/glm
)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(Delta PRIVATE Vulkan::Vulkan)

# zlib (needed by Assimp static lib)
find_package(ZLIB)
if (ZLIB_FOUND)
  target_link_libraries(Delta PRIVATE ZLIB::ZLIB)
else()
  # Fallback to system lib name
  target_link_libraries(Delta PRIVATE z)
endif()

# GLFW (import static lib from ThirdParty/lib)
if(NOT TARGET glfw)
  add_library(glfw STATIC IMPORTED)
  if(CMAKE_CONFIGURATION_TYPES)
    if (WIN32)
      set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION_DEBUG   ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/glfw3.lib
        IMPORTED_LOCATION_RELEASE ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/glfw3.lib)
    else()
      set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION_DEBUG   ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/libglfw3.a
        IMPORTED_LOCATION_RELEASE ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/libglfw3.a)
    endif()
  else()
    if (WIN32)
      if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/glfw3.lib)
      else()
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/glfw3.lib)
      endif()
    else()
      if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/libglfw3.a)
      else()
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/libglfw3.a)
      endif()
    endif()
  endif()
endif()
target_link_libraries(Delta PRIVATE glfw)

# Assimp (import from ThirdParty/lib)
if(NOT TARGET assimp)
  add_library(assimp STATIC IMPORTED)
  if(CMAKE_CONFIGURATION_TYPES)
    if (WIN32)
      set_target_properties(assimp PROPERTIES
        IMPORTED_LOCATION_DEBUG   ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/assimp-vc143-mtd.lib
        IMPORTED_LOCATION_RELEASE ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/assimp-vc143-mt.lib)
    else()
      set_target_properties(assimp PROPERTIES
        IMPORTED_LOCATION_DEBUG   ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/libassimpd.a
        IMPORTED_LOCATION_RELEASE ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/libassimp.a)
    endif()
  else()
    if (WIN32)
      if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/assimp-vc143-mtd.lib)
      else()
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/assimp-vc143-mt.lib)
      endif()
    else()
      if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/libassimpd.a)
      else()
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/libassimp.a)
      endif()
    endif()
  endif()
endif()
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

if(WIN32)
  set(GLSLC_CANDIDATES
    "$ENV{VULKAN_SDK}/Bin/glslc.exe"
    glslc.exe
  )
else()
  set(GLSLC_CANDIDATES
    "$ENV{VULKAN_SDK}/macOS/bin/glslc"
    "$ENV{VULKAN_SDK}/bin/glslc"
    glslc
  )
endif()

set(GLSLC_TOOL "")
foreach(c ${GLSLC_CANDIDATES})
  if(EXISTS ${c})
    set(GLSLC_TOOL ${c})
    break()
  endif()
endforeach()
if(GLSLC_TOOL STREQUAL "")
  message(WARNING "glslc not found; shader compilation target will be empty")
endif()

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
