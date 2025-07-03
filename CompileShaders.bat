@echo off
setlocal enabledelayedexpansion

REM Get the directory of the running batch file
set "BASE_DIR=%~dp0"
set "GLSLC=c:\VulkanSDK\1.4.309.0\Bin\glslc.exe"
set "INPUT_DIR=%BASE_DIR%Delta\Source\Shaders"
set "OUTPUT_DIR=%BASE_DIR%Resources\Shaders"

for %%f in ("%INPUT_DIR%\*") do (
    set "filename=%%~nxf.spv"
    "%GLSLC%" "%%f" -o "%OUTPUT_DIR%\!filename!"
)

echo Done.
pause
