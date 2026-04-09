@echo off
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do set VSPATH=%%i
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat"
call edksetup.bat >nul 2>&1
build -p AiBiosPackage/AiBiosPackage.dsc -a X64 -t CLANGDWARF -b DEBUG -D RUN_TESTS=TRUE
