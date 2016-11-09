This branch will be used to contribute support for Standalone MM on ARMv8-A platforms in the AArch64 execution state.

The branch owner: Achin Gupta <achin.gupta@arm.com>

# Feature Introduction
The "Volume 4: Platform Initialization Specification, Management Mode Core
Interface" introduces the concept of MM Standalone Mode. Initialization of MM in
this mode can be done during the SEC phase. On ARMv8-A systems, [ARM Trusted
Firmware](https://github.com/ARM-software/arm-trusted-firmware) is responsible
for launching the normal world firmware e.g. UEFI. It can be seen as running in
the PI SEC phase. The aim of this branch is to add support in EDK2 for:

1. Initialisation of the Standalone MM environment during the SEC phase in
   collaboration with ARM Trusted Firmware
2. Accessing MM services instantiated in the secure world from the normal world
   i.e. UEFI, OS in collaboration with ARM Trusted Firmware.

## Details
The Standalone MM environment is instantiated in Secure EL0 and runs under
control of ARM TF. This branch builds upon the StandaloneSmmPkg module
originally contributed by Intel.

* Platform: FVPv7.7 FVP_Base_AEMv8A-AEMv8A
* Toolchain: aarch64-linux-gnu-gcc (Linaro GCC 2014.11) 4.9.3 20141031 (prerelease)

####

## Related Modules
The following modules are related to Standalone MM:

StandaloneSmmPkg - Standalone MM package. This package provides the MM Foundation, MM CPU driver and related libraries/modules required for instantiating a MM environment in the secure world
