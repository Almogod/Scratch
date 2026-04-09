# aiBIOS Installation & Flash Guide

This guide describes how to sideload the aiBIOS AI-driven UEFI module onto a physical motherboard using the standard EFI Shell. Note: Modifying UEFI DXE drivers is an advanced topic. **Proceed with caution.**

## Prerequisites
1. A USB Flash Drive formatted to **FAT32**.
2. A motherboard with UEFI version 2.4 or higher (most boards produced after 2014).
3. The compiled aiBIOS binary (`AiBiosMain.efi`).
4. EDK II Shell (often built into the motherboard `Boot / Tool` menus, or downloadable as `Shell.efi`).

## Step 1: Prepare the USB Drive
1. Plug the FAT32 formatted USB drive into your PC.
2. Create a folder named `EFI` at the root directory of the drive.
3. Inside `EFI`, create a folder named `BOOT`.
4. Copy `AiBiosMain.efi` directly to the ROOT of the USB drive (not inside `BOOT`, unless you are setting it up as the primary bootloader). Let's keep it at root `FS0:\AiBiosMain.efi`.

## Step 2: Disable Secure Boot
Because you are loading a custom (and potentially self-signed or unsigned) EFI module, you must temporarily disable Secure Boot.
1. Reboot your PC and repeatedly press `DEL` or `F2` to enter the BIOS menu.
2. Navigate to **Security** -> **Secure Boot**.
3. Set Secure Boot state to **Disabled**.
4. Save and Exit (`F10`).

## Step 3: Launch EFI Shell
1. Enter the BIOS menu again.
2. Most modern boards (ASUS, MSI, Gigabyte) have an option "Launch EFI Shell from filesystem device" or present the USB drive in the Boot Override list as "UEFI: Built-in EFI Shell".
3. Select the EFI Shell option.

## Step 4: Loading aiBIOS
Once inside the EFI Shell:
1. Identify your USB drive. Type `map -r` to refresh the mappings. Your USB is usually mapped to `FS0:` or `FS1:`.
2. Type `FS0:` (or your specific FS number) and hit Enter.
3. Type `ls` to verify `AiBiosMain.efi` is present.
4. Load the module into DXE space by typing:
   ```bash
   load AiBiosMain.efi
   ```
5. You should immediately see standard output:
   ```text
   aiBIOS Prototype v0.1 Online.
   NL Interface ready. Enter command:
   ```
6. You can now use the Natural Language hardware tuning interface directly from the Shell.

## Step 5: Rollback and Recovery
If a severe crash occurs or an MSR register lockup triggers a boot failure:
1. **CMOS Reset**: Disconnect the power cable. Hold down the Clear CMOS button on your rear I/O panel for 5 seconds (or short the two CLRTC pins on your motherboard with a screwdriver).
2. **BIOS Flashback**: Use your motherboard's physical USB Flashback port to flash a clean, stock BIOS binary using a USB drive if corruption somehow touches NVRAM variables heavily.

**Warning**: Do not experiment with extreme voltage profiling via aiBIOS unless you have adequate motherboard protection mechanisms. The agent enforces safe parameter limits via `MSR_SAFE_RANGE`, but thermal protections should not be disabled.
