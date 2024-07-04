@echo off

set "batch_dir=%~dp0"

cd %batch_dir%../../../../cache

python %2/upstream/emscripten/tools/file_packager.py %1/dist/assets.data --preload assets --js-output=%1/dist/assets.js --quiet
