@echo off
setlocal

:: Check if the first parameter is provided
if "%~1"=="" (
    echo Please provide a file path.
    exit /b 1
)

:: Set the parameters
set FILE_PATH=%~1

:: Call the executable with the provided parameter
C:\Users\Calxf\Documents\CallumDocs\git\GameEngine\cmake-build-debug-visual-studio\Tools\PackageHDRI.exe "%FILE_PATH%" C:\Users\Calxf\Documents\CallumDocs\git\GameEngine\Sandbox\assets\packaged

endlocal
