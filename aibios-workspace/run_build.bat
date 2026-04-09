@echo off
set "NASM_PREFIX=%~dp0"
set "GCC_BIN=C:\Users\Hp\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin\"
set "PATH=%~dp0BaseTools\Bin\Win64;%PATH%"
set "EDK_TOOLS_BIN=%~dp0BaseTools\Bin\Win64"
call edksetup.bat >nul 2>&1
build -p AiBiosPackage/AiBiosPackage.dsc -a X64 -t GCC -b DEBUG -D RUN_TESTS=TRUE
