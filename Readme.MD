# Cadence IP Support Branch

This branch will be used to contribute code to support Cadence
hardware IP for AArch64-based platforms.

## edk2-staging branch owners
* Scott Telford <stelford@cadence.com>

## Introduction 

Cadence develops a variety of peripheral IP modules suitable for ARM
SoC architectures. This branch is intended to provide a staging area
for drivers and libraries developed to support Cadence peripherals
that may be incorporated into various SoC architectures from other
vendors, for later integration into the appropriate EDK2
platforms. Initially, the focus will be support for the Cadence PCIe
4.0 Root Port. For internal development and testing purposes, an EDK2
platform has been developed for a minimal AArch64 system
configuration, comprising a single Cortex-A53 processor, GIC-500,
NIC-400, Cadence PCIe Root Port, and Cadence UART. This will be
included in the branch to provide an example of driver/platform
integration.

## Related Modules
The following modules are related to this branch:
* CadencePkg   - Package containing Cadence-specific drivers and libraries
* MdeModulePkg - Package containing generic drivers relevant to Cadence peripheral IP
