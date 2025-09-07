@echo off
setlocal enabledelayedexpansion

REM Usage: build_deps_windows.bat Debug|Release
set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Debug

set ROOT_DIR=%~dp0..\
set GLFW_SRC=%ROOT_DIR%ThirdParty\glfw
set ASSIMP_SRC=%ROOT_DIR%ThirdParty\assimp
set GLFW_BUILD=%ROOT_DIR%ThirdParty\build\glfw\%CONFIG%
set ASSIMP_BUILD=%ROOT_DIR%ThirdParty\build\assimp\%CONFIG%
set OUT_LIB=%ROOT_DIR%ThirdParty\lib\%CONFIG%

echo Building third-party dependencies for %CONFIG%
if not exist "%OUT_LIB%" mkdir "%OUT_LIB%"

REM Build GLFW (static)
echo ===> Building GLFW
cmake -S "%GLFW_SRC%" -B "%GLFW_BUILD" -G "Visual Studio 17 2022" -A x64 ^
  -DBUILD_SHARED_LIBS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_DOCS=OFF || exit /b 1
cmake --build "%GLFW_BUILD" --config %CONFIG% -- /m || exit /b 1
for %%f in ("%GLFW_BUILD%\src\*.lib") do copy /Y "%%f" "%OUT_LIB%" >nul 2>&1

REM Build Assimp (static)
echo ===> Building Assimp
cmake -S "%ASSIMP_SRC%" -B "%ASSIMP_BUILD" -G "Visual Studio 17 2022" -A x64 ^
  -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_ZLIB=ON || exit /b 1
cmake --build "%ASSIMP_BUILD" --config %CONFIG% -- /m || exit /b 1
for %%f in ("%ASSIMP_BUILD%\lib\*.lib") do copy /Y "%%f" "%OUT_LIB%" >nul 2>&1

echo Done. Copied libraries to %OUT_LIB%

