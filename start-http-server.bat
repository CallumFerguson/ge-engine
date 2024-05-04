@echo off
if "%1"=="" (
    echo Usage: %0 directory
    goto :eof
)
python quiet_http_server.py "%~1"
