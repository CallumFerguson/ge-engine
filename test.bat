@echo off
setlocal

:: Check for argument
if "%~1"=="" (
    echo Usage: %0 directory_path
    exit /b 1
)

:: Store and resolve the full directory path
set "DIR_PATH=%~f1"

:: Check if the directory exists
if not exist "%DIR_PATH%\\" (
    echo The directory does not exist: %DIR_PATH%
    exit /b 1
)

:: Use PowerShell to calculate the hash of all files in the directory and subdirectories
powershell -NoProfile -Command ^
    "$path = '%DIR_PATH%';" ^
    "$hash = Get-ChildItem -Path $path -Recurse -File | Sort-Object FullName | Get-FileHash -Algorithm SHA256 | ForEach-Object { $_.Hash }; $finalHash = [System.BitConverter]::ToString((New-Object Security.Cryptography.SHA256Managed).ComputeHash([System.Text.Encoding]::UTF8.GetBytes(($hash -join '')))) -replace '-';" ^
    "Write-Output 'Final SHA256 hash of all files: ' $finalHash"

endlocal
