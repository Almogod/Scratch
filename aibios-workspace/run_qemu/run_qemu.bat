@echo off
echo ==============================================
echo  aiBIOS Emulation Harness
echo ==============================================

:: Check if QEMU is installed
where qemu-system-x86_64 >nul 2>nul
if %ERRORLEVEL% neq 0 (
    if exist "C:\Program Files\qemu\qemu-system-x86_64.exe" (
        set "PATH=C:\Program Files\qemu;%PATH%"
    ) else (
        echo [ERROR] qemu-system-x86_64 is not found in PATH!
        echo Please install QEMU. You can use winget from an administrator prompt:
        echo winget install SoftwareFreedomConservancy.QEMU
        pause
        exit /b 1
    )
)

:: Find OVMF.fd provided natively by the QEMU installation
set "QEMU_PATH=C:\Program Files\qemu"
set "OVMF_CODE=%QEMU_PATH%\share\edk2-x86_64-code.fd"

if not exist "%OVMF_CODE%" (
    echo [ERROR] Expected OVMF firmware not found at %OVMF_CODE%
    echo Make sure you have a complete QEMU installation with the edk2 bios images.
    pause
    exit /b 1
)

echo [INFO] Emulating a UEFI system running AiBiosMain.efi...
echo [INFO] Press CTRL+C in the terminal to forcefully kill QEMU if it hangs.
echo.

:: Launch QEMU:
:: -m 2G: 2GB of memory since LLMs need memory (even quantized ones)
:: -L : Bios search path
:: -drive if=pflash: Load OVMF EFI firmware natively
:: -drive file=fat:rw:fat_dir : Map the local folder as a FAT32 filesystem holding our EFI and script
:: -net none: No network, secure boot simulation
qemu-system-x86_64 ^
    -m 2G ^
    -machine q35 ^
    -drive if=pflash,format=raw,readonly=on,file="%OVMF_CODE%" ^
    -drive file=fat:rw:"%~dp0fat_dir",format=raw ^
    -net none ^
    -serial stdio

echo.
echo [INFO] Emulation Session Ended.
pause
