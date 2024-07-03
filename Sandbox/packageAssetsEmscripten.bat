@echo off
setlocal enabledelayedexpansion

set exe_path="../cmake-build-debug-visual-studio/Tools/PackageAssets.exe"
set assets_dir="assets"
set cache_dir="../cache"
set engine_assets_dir="../GameEngine/engineAssets"

%exe_path% %assets_dir% %cache_dir% %engine_assets_dir%

REM Get the directory of the batch file
set "batch_dir=%~dp0"

REM Load environment variables from .env file
for /f "tokens=1,* delims==" %%a in ('type "%batch_dir%..\.env"') do (
    set "%%a=%%b"
)

if "!emsdk_path!"=="" (
    echo emsdk_path is not set in the .env file
    exit /b 1
)

cd %batch_dir%../cache

python !emsdk_path!/upstream/emscripten/tools/file_packager.py %batch_dir%../cmake-build-debug-emscripten/Sandbox/dist/assets.data --preload assets --js-output=%batch_dir%../cmake-build-debug-emscripten/Sandbox/dist/assets.js --quiet
