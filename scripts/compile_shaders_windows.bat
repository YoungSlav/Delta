@echo off
setlocal enabledelayedexpansion

set BASE_DIR=%~dp0..\
set INPUT_DIR=%BASE_DIR%Delta\Source\Shaders
set OUTPUT_DIR=%BASE_DIR%Resources\Shaders

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

set GLSLC=%GLSLC%
if "%GLSLC%"=="" (
  if exist "%VULKAN_SDK%\Bin\glslc.exe" set GLSLC=%VULKAN_SDK%\Bin\glslc.exe
)

if not exist "%GLSLC%" (
  echo Error: glslc.exe not found. Set VULKAN_SDK or GLSLC.
  exit /b 1
)

for %%f in ("%INPUT_DIR%\*") do (
  set "name=%%~nxf.spv"
  "%GLSLC%" "%%f" -o "%OUTPUT_DIR%\!name!"
)

echo Shaders compiled to %OUTPUT_DIR%

