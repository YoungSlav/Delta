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
  if (WIN32)
    # Try to import zlib built inside Assimp's build tree
    set(ZLIB_DEBUG_PATH   ${CMAKE_SOURCE_DIR}/ThirdParty/build/assimp/Debug/contrib/zlib/Debug/zlibstaticd.lib)
    set(ZLIB_RELEASE_PATH ${CMAKE_SOURCE_DIR}/ThirdParty/build/assimp/Release/contrib/zlib/Release/zlibstatic.lib)
    if (EXISTS "${ZLIB_DEBUG_PATH}" OR EXISTS "${ZLIB_RELEASE_PATH}")
      add_library(ZLIB::ZLIB STATIC IMPORTED)
        if(CMAKE_CONFIGURATION_TYPES)
          # For multi-config generators we must provide IMPORTED_LOCATION for each config.
          if(NOT (EXISTS "${ZLIB_DEBUG_PATH}" OR EXISTS "${ZLIB_RELEASE_PATH}"))
            message(FATAL_ERROR "Zlib static lib not found in ThirdParty build. Run scripts/build_deps.sh to build third-party libs for Windows.")
          endif()
          if(EXISTS "${ZLIB_DEBUG_PATH}")
            set(_dbg "${ZLIB_DEBUG_PATH}")
          else()
            set(_dbg "${ZLIB_RELEASE_PATH}")
          endif()
          if(EXISTS "${ZLIB_RELEASE_PATH}")
            set(_rel "${ZLIB_RELEASE_PATH}")
          else()
            set(_rel "${ZLIB_DEBUG_PATH}")
          endif()
          set_property(TARGET ZLIB::ZLIB PROPERTY IMPORTED_LOCATION_DEBUG "${_dbg}")
          set_property(TARGET ZLIB::ZLIB PROPERTY IMPORTED_LOCATION_RELEASE "${_rel}")
          set_property(TARGET ZLIB::ZLIB PROPERTY IMPORTED_LOCATION_RelWithDebInfo "${_rel}")
          set_property(TARGET ZLIB::ZLIB PROPERTY IMPORTED_LOCATION_MinSizeRel "${_rel}")
          set_property(TARGET ZLIB::ZLIB PROPERTY IMPORTED_LOCATION "${_rel}")
      else()
        if (EXISTS "${ZLIB_DEBUG_PATH}")
          set_property(TARGET ZLIB::ZLIB PROPERTY IMPORTED_LOCATION_DEBUG "${ZLIB_DEBUG_PATH}")
        endif()
        if (EXISTS "${ZLIB_RELEASE_PATH}")
          set_property(TARGET ZLIB::ZLIB PROPERTY IMPORTED_LOCATION_RELEASE "${ZLIB_RELEASE_PATH}")
        endif()
      endif()
      target_link_libraries(Delta PRIVATE ZLIB::ZLIB)
    else()
      message(WARNING "Zlib not found; ensure Assimp's contrib zlib is built, or install zlib and reconfigure.")
    endif()
  else()
    # POSIX fallback system name
    target_link_libraries(Delta PRIVATE z)
  endif()
endif()

# GLFW (import static lib from ThirdParty/lib)
if(NOT TARGET glfw)
  add_library(glfw STATIC IMPORTED)
    if(CMAKE_CONFIGURATION_TYPES)
      # For multi-config generators we must provide IMPORTED_LOCATION for each config or fail fast.
      if (WIN32)
        set(_debug_path "${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/glfw3.lib")
        set(_release_path "${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/glfw3.lib")
        if(NOT (EXISTS "${_debug_path}" OR EXISTS "${_release_path}"))
          message(FATAL_ERROR "GLFW static lib not found in ThirdParty/lib. Run scripts/build_deps.sh to build third-party libs for Windows.")
        endif()
        if(EXISTS "${_debug_path}")
          set(_dbg "${_debug_path}")
        else()
          set(_dbg "${_release_path}")
        endif()
        if(EXISTS "${_release_path}")
          set(_rel "${_release_path}")
        else()
          set(_rel "${_debug_path}")
        endif()
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION_DEBUG "${_dbg}")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION_RELEASE "${_rel}")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION_RelWithDebInfo "${_rel}")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION_MinSizeRel "${_rel}")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION "${_rel}")
      else()
        set(_debug_path "${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/libglfw3.a")
        set(_release_path "${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/libglfw3.a")
        if(NOT (EXISTS "${_debug_path}" OR EXISTS "${_release_path}"))
          message(FATAL_ERROR "GLFW static lib not found in ThirdParty/lib. Ensure libc builds are present.")
        endif()
        if(EXISTS "${_debug_path}")
          set(_dbg "${_debug_path}")
        else()
          set(_dbg "${_release_path}")
        endif()
        if(EXISTS "${_release_path}")
          set(_rel "${_release_path}")
        else()
          set(_rel "${_debug_path}")
        endif()
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION_DEBUG "${_dbg}")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION_RELEASE "${_rel}")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION_RelWithDebInfo "${_rel}")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION_MinSizeRel "${_rel}")
        set_target_properties(glfw PROPERTIES IMPORTED_LOCATION "${_rel}")
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
        set(_debug_path "${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/assimp-vc143-mtd.lib")
        set(_release_path "${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/assimp-vc143-mt.lib")
        if(NOT (EXISTS "${_debug_path}" OR EXISTS "${_release_path}"))
          message(FATAL_ERROR "Assimp static lib not found in ThirdParty/lib. Run scripts/build_deps.sh to build third-party libs for Windows.")
        endif()
        if(EXISTS "${_debug_path}")
          set(_dbg "${_debug_path}")
        else()
          set(_dbg "${_release_path}")
        endif()
        if(EXISTS "${_release_path}")
          set(_rel "${_release_path}")
        else()
          set(_rel "${_debug_path}")
        endif()
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_DEBUG "${_dbg}")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_RELEASE "${_rel}")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_RelWithDebInfo "${_rel}")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_MinSizeRel "${_rel}")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION "${_rel}")
      else()
        set(_debug_path "${CMAKE_SOURCE_DIR}/ThirdParty/lib/Debug/libassimpd.a")
        set(_release_path "${CMAKE_SOURCE_DIR}/ThirdParty/lib/Release/libassimp.a")
        if(NOT (EXISTS "${_debug_path}" OR EXISTS "${_release_path}"))
          message(FATAL_ERROR "Assimp static lib not found in ThirdParty/lib. Ensure libs are present.")
        endif()
        if(EXISTS "${_debug_path}")
          set(_dbg "${_debug_path}")
        else()
          set(_dbg "${_release_path}")
        endif()
        if(EXISTS "${_release_path}")
          set(_rel "${_release_path}")
        else()
          set(_rel "${_debug_path}")
        endif()
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_DEBUG "${_dbg}")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_RELEASE "${_rel}")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_RelWithDebInfo "${_rel}")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_MinSizeRel "${_rel}")
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION "${_rel}")
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
  message(WARNING "glslc not found; shader compilation target will be skipped")
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
