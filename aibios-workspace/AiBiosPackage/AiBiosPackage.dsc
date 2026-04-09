[Defines]
  PLATFORM_NAME            = AiBiosPackage
  PLATFORM_GUID            = a1b2c3d4-e5f6-7890-abcd-ef1234567890
  PLATFORM_VERSION         = 0.1
  DSC_SPECIFICATION        = 0x0001001C
  OUTPUT_DIRECTORY         = Build/AiBios
  SUPPORTED_ARCHITECTURES  = X64
  BUILD_TARGETS            = DEBUG|RELEASE
  SKUID_IDENTIFIER         = DEFAULT

[BuildOptions]
  GCC:*_*_X64_CC_FLAGS = -fno-stack-protector -mno-red-zone
  MSFT:*_*_X64_CC_FLAGS = /O2 /GS- /D MDE_CPU_X64
!if $(RUN_TESTS) == TRUE
  GCC:*_*_X64_CC_FLAGS = -fno-stack-protector -mno-red-zone -D RUN_TESTS=1
!endif

[LibraryClasses]
  # Basic Libraries
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  TimerLib|MdePkg/Library/SecPeiDxeTimerLibCpu/SecPeiDxeTimerLibCpu.inf
  
  # UEFI Libraries
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  StackCheckLib|MdePkg/Library/StackCheckLibNull/StackCheckLibNull.inf
  StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf

[Components]
  AiBiosPackage/AiBiosMain/AiBiosMain.inf
