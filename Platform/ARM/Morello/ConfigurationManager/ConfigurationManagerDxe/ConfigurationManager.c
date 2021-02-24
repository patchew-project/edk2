/** @file
  Configuration Manager Dxe

  Copyright (c) 2021, ARM Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Cm or CM   - Configuration Manager
    - Obj or OBJ - Object
**/

#include <IndustryStandard/DebugPort2Table.h>
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/ConfigurationManagerProtocol.h>

#include "ConfigurationManagerFvp.h"
#include "ConfigurationManager.h"
#include "Platform.h"

extern EDKII_PLATFORM_REPOSITORY_INFO MorelloRepositoryInfo;

// The platform configuration repository information.
EDKII_COMMON_PLATFORM_REPOSITORY_INFO CommonPlatformInfo = {
  // Configuration Manager information
  { CONFIGURATION_MANAGER_REVISION, CFG_MGR_OEM_ID },

  // Boot architecture information
  { EFI_ACPI_6_3_ARM_PSCI_COMPLIANT },              // BootArchFlags

#ifdef HEADLESS_PLATFORM
  // Fixed feature flag information
  { EFI_ACPI_6_3_HEADLESS },                        // Fixed feature flags
#endif

  // Power management profile information
  { EFI_ACPI_6_3_PM_PROFILE_ENTERPRISE_SERVER },    // PowerManagement Profile

  /* GIC CPU Interface information
     GIC_ENTRY (CPUInterfaceNumber, Mpidr, PmuIrq, VGicIrq, GicRedistBase,
                EnergyEfficiency, SpeIrq, ProximityDomain, ClockDomain)
  */
  {
    GICC_ENTRY (0, GET_MPID3 (0x0, 0x0, 0x0, 0x0), 23, 25, 0, 0, 21, 0, 0),
    GICC_ENTRY (1, GET_MPID3 (0x0, 0x0, 0x1, 0x0), 23, 25, 0, 0, 21, 0, 0),
    GICC_ENTRY (2, GET_MPID3 (0x0, 0x1, 0x0, 0x0), 23, 25, 0, 0, 21, 0, 0),
    GICC_ENTRY (3, GET_MPID3 (0x0, 0x1, 0x1, 0x0), 23, 25, 0, 0, 21, 0, 0),
  },

  // GIC Distributor Info
  {
    FixedPcdGet64 (PcdGicDistributorBase),  // UINT64  PhysicalBaseAddress
    0,                                      // UINT32  SystemVectorBase
    3                                       // UINT8   GicVersion
  },

  // GIC Re-Distributor Info
  {
    // UINT64  DiscoveryRangeBaseAddress
    FixedPcdGet64 (PcdGicRedistributorsBase),
    // UINT32  DiscoveryRangeLength
    SIZE_1MB
  },

  // Generic Timer Info
  {
    // The physical base address for the counter control frame
    MORELLO_SYSTEM_TIMER_BASE_ADDRESS,
    // The physical base address for the counter read frame
    MORELLO_CNT_READ_BASE_ADDRESS,
    // The secure PL1 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerSecIntrNum),
    // The secure PL1 timer flags
    MORELLO_GTDT_GTIMER_FLAGS,
    // The non-secure PL1 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerIntrNum),
    // The non-secure PL1 timer flags
    MORELLO_GTDT_GTIMER_FLAGS,
    // The virtual timer interrupt
    FixedPcdGet32 (PcdArmArchTimerVirtIntrNum),
    // The virtual timer flags
    MORELLO_GTDT_GTIMER_FLAGS,
    // The non-secure PL2 timer interrupt
    FixedPcdGet32 (PcdArmArchTimerHypIntrNum),
    // The non-secure PL2 timer flags
    MORELLO_GTDT_GTIMER_FLAGS
  },

  // Generic Timer Block Information
  {
    {
     // The physical base address for the GT Block Timer structure
      MORELLO_GT_BLOCK_CTL_BASE,
      // The number of timer frames implemented in the GT Block
      MORELLO_TIMER_FRAMES_COUNT,
      // Reference token for the GT Block timer frame list
      REFERENCE_TOKEN (GTBlock0TimerInfo)
    }
  },

  // GT Block Timer Frames
  {
    // Frame 0
    {
      0,                                    // UINT8   FrameNumber
      MORELLO_GT_BLOCK_FRAME0_CTL_BASE,     // UINT64  PhysicalAddressCntBase
      MORELLO_GT_BLOCK_FRAME0_CTL_EL0_BASE, // UINT64  PhysicalAddressCntEL0Base
      MORELLO_GT_BLOCK_FRAME0_GSIV,         // UINT32  PhysicalTimerGSIV
      MORELLO_GTX_TIMER_FLAGS,              // UINT32  PhysicalTimerFlags
      0,                                    // UINT32  VirtualTimerGSIV
      0,                                    // UINT32  VirtualTimerFlags
      MORELLO_GTX_COMMON_FLAGS_NS           // UINT32  CommonFlags
    },
    // Frame 1
    {
      1,                                    // UINT8   FrameNumber
      MORELLO_GT_BLOCK_FRAME1_CTL_BASE,     // UINT64  PhysicalAddressCntBase
      MORELLO_GT_BLOCK_FRAME1_CTL_EL0_BASE, // UINT64  PhysicalAddressCntEL0Base
      MORELLO_GT_BLOCK_FRAME1_GSIV,         // UINT32  PhysicalTimerGSIV
      MORELLO_GTX_TIMER_FLAGS,              // UINT32  PhysicalTimerFlags
      0,                                    // UINT32  VirtualTimerGSIV
      0,                                    // UINT32  VirtualTimerFlags
      MORELLO_GTX_COMMON_FLAGS_S            // UINT32  CommonFlags
    },
  },

  // Watchdog Info
  {
    // The physical base address of the SBSA Watchdog control frame
    FixedPcdGet64 (PcdGenericWatchdogControlBase),
    // The physical base address of the SBSA Watchdog refresh frame
    FixedPcdGet64 (PcdGenericWatchdogRefreshBase),
    // The watchdog interrupt
    FixedPcdGet32 (PcdGenericWatchdogEl2IntrNum),
    // The watchdog flags
    MORELLO_SBSA_WATCHDOG_FLAGS
  },

  // SPCR Serial Port
  {
    FixedPcdGet64 (PcdSerialRegisterBase),                  // BaseAddress
    FixedPcdGet32 (PL011UartInterrupt),                     // Interrupt
    FixedPcdGet64 (PcdUartDefaultBaudRate),                 // BaudRate
    FixedPcdGet32 (PL011UartClkInHz),                       // Clock
    EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_ARM_SBSA_GENERIC_UART // Port subtype
  },

  // Debug Serial Port
  {
    FixedPcdGet64 (PcdSerialDbgRegisterBase),               // BaseAddress
    0,                                                      // Interrupt -unused
    FixedPcdGet64 (PcdSerialDbgUartBaudRate),               // BaudRate
    FixedPcdGet32 (PcdSerialDbgUartClkInHz),                // Clock
    EFI_ACPI_DBG2_PORT_SUBTYPE_SERIAL_ARM_SBSA_GENERIC_UART // Port subtype
  },

  // Processor Hierarchy Nodes
  {
    // Package
    {
      // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (ProcHierarchyInfo[0]),
      // UINT32  Flags
      PROC_NODE_FLAGS (
        EFI_ACPI_6_3_PPTT_PACKAGE_PHYSICAL,
        EFI_ACPI_6_3_PPTT_PROCESSOR_ID_INVALID,
        EFI_ACPI_6_3_PPTT_PROCESSOR_IS_NOT_THREAD,
        EFI_ACPI_6_3_PPTT_NODE_IS_NOT_LEAF,
        EFI_ACPI_6_3_PPTT_IMPLEMENTATION_IDENTICAL
      ),
      // CM_OBJECT_TOKEN  ParentToken
      CM_NULL_TOKEN,
      // CM_OBJECT_TOKEN  GicCToken
      CM_NULL_TOKEN,
      // UINT32  NoOfPrivateResources
      SOC_RESOURCE_COUNT,
      // CM_OBJECT_TOKEN  PrivateResourcesArrayToken
      REFERENCE_TOKEN (SocResources)
    },

    // Cluster0
    {
      // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (ProcHierarchyInfo[1]),
      // UINT32  Flags
      PROC_NODE_FLAGS (
        EFI_ACPI_6_3_PPTT_PACKAGE_NOT_PHYSICAL,
        EFI_ACPI_6_3_PPTT_PROCESSOR_ID_INVALID,
        EFI_ACPI_6_3_PPTT_PROCESSOR_IS_NOT_THREAD,
        EFI_ACPI_6_3_PPTT_NODE_IS_NOT_LEAF,
        EFI_ACPI_6_3_PPTT_IMPLEMENTATION_IDENTICAL
      ),
      // CM_OBJECT_TOKEN  ParentToken
      REFERENCE_TOKEN (ProcHierarchyInfo[0]), // -> Package
      // CM_OBJECT_TOKEN  GicCToken
      CM_NULL_TOKEN,
      // UINT32  NoOfPrivateResources
      CLUSTER_RESOURCE_COUNT,
      // CM_OBJECT_TOKEN  PrivateResourcesArrayToken
      REFERENCE_TOKEN (ClusterResources)
    },
    // Cluster1
    {
      // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (ProcHierarchyInfo[2]),
      // UINT32  Flags
      PROC_NODE_FLAGS (
        EFI_ACPI_6_3_PPTT_PACKAGE_NOT_PHYSICAL,
        EFI_ACPI_6_3_PPTT_PROCESSOR_ID_INVALID,
        EFI_ACPI_6_3_PPTT_PROCESSOR_IS_NOT_THREAD,
        EFI_ACPI_6_3_PPTT_NODE_IS_NOT_LEAF,
        EFI_ACPI_6_3_PPTT_IMPLEMENTATION_IDENTICAL
      ),
      // CM_OBJECT_TOKEN  ParentToken
      REFERENCE_TOKEN (ProcHierarchyInfo[0]), // -> Package
      // CM_OBJECT_TOKEN  GicCToken
      CM_NULL_TOKEN,
      // UINT32  NoOfPrivateResources
      CLUSTER_RESOURCE_COUNT,
      // CM_OBJECT_TOKEN  PrivateResourcesArrayToken
      REFERENCE_TOKEN (ClusterResources)
    },
    // Cluster0 - Cpu0
    {
      // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (ProcHierarchyInfo[3]),
      // UINT32  Flags
      PROC_NODE_FLAGS (
        EFI_ACPI_6_3_PPTT_PACKAGE_NOT_PHYSICAL,
        EFI_ACPI_6_3_PPTT_PROCESSOR_ID_VALID,
        EFI_ACPI_6_3_PPTT_PROCESSOR_IS_NOT_THREAD,
        EFI_ACPI_6_3_PPTT_NODE_IS_LEAF,
        EFI_ACPI_6_3_PPTT_IMPLEMENTATION_NOT_IDENTICAL
      ),
      // CM_OBJECT_TOKEN  ParentToken
      REFERENCE_TOKEN (ProcHierarchyInfo[1]), // -> 'cluster in Cluster0
      // CM_OBJECT_TOKEN  GicCToken
      REFERENCE_TOKEN (GicCInfo[0]),
      // UINT32  NoOfPrivateResources
      CORE_RESOURCE_COUNT,
      // CM_OBJECT_TOKEN  PrivateResourcesArrayToken
      REFERENCE_TOKEN (CoreResources)
    },
    // Cluster0 - Cpu1
    {
      // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (ProcHierarchyInfo[4]),
      // UINT32  Flags
      PROC_NODE_FLAGS (
        EFI_ACPI_6_3_PPTT_PACKAGE_NOT_PHYSICAL,
        EFI_ACPI_6_3_PPTT_PROCESSOR_ID_VALID,
        EFI_ACPI_6_3_PPTT_PROCESSOR_IS_NOT_THREAD,
        EFI_ACPI_6_3_PPTT_NODE_IS_LEAF,
        EFI_ACPI_6_3_PPTT_IMPLEMENTATION_NOT_IDENTICAL
      ),
      // CM_OBJECT_TOKEN  ParentToken
      REFERENCE_TOKEN (ProcHierarchyInfo[1]), // -> 'cluster in Cluster0
      // CM_OBJECT_TOKEN  GicCToken
      REFERENCE_TOKEN (GicCInfo[1]),
      // UINT32  NoOfPrivateResources
      CORE_RESOURCE_COUNT,
      // CM_OBJECT_TOKEN  PrivateResourcesArrayToken
      REFERENCE_TOKEN (CoreResources)
    },
    // Cluster1 - Cpu0
    {
      // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (ProcHierarchyInfo[3]),
      // UINT32  Flags
      PROC_NODE_FLAGS (
        EFI_ACPI_6_3_PPTT_PACKAGE_NOT_PHYSICAL,
        EFI_ACPI_6_3_PPTT_PROCESSOR_ID_VALID,
        EFI_ACPI_6_3_PPTT_PROCESSOR_IS_NOT_THREAD,
        EFI_ACPI_6_3_PPTT_NODE_IS_LEAF,
        EFI_ACPI_6_3_PPTT_IMPLEMENTATION_NOT_IDENTICAL
      ),
      // CM_OBJECT_TOKEN  ParentToken
      REFERENCE_TOKEN (ProcHierarchyInfo[2]), // -> 'cluster in Cluster1
      // CM_OBJECT_TOKEN  GicCToken
      REFERENCE_TOKEN (GicCInfo[2]),
      // UINT32  NoOfPrivateResources
      CORE_RESOURCE_COUNT,
      // CM_OBJECT_TOKEN  PrivateResourcesArrayToken
      REFERENCE_TOKEN (CoreResources)
    },

    // Cluster1 - Cpu1
    {
      // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (ProcHierarchyInfo[4]),
      // UINT32  Flags
      PROC_NODE_FLAGS (
        EFI_ACPI_6_3_PPTT_PACKAGE_NOT_PHYSICAL,
        EFI_ACPI_6_3_PPTT_PROCESSOR_ID_VALID,
        EFI_ACPI_6_3_PPTT_PROCESSOR_IS_NOT_THREAD,
        EFI_ACPI_6_3_PPTT_NODE_IS_LEAF,
        EFI_ACPI_6_3_PPTT_IMPLEMENTATION_NOT_IDENTICAL
      ),
      // CM_OBJECT_TOKEN  ParentToken
      REFERENCE_TOKEN (ProcHierarchyInfo[2]), // -> 'cluster in Cluster1
      // CM_OBJECT_TOKEN  GicCToken
      REFERENCE_TOKEN (GicCInfo[3]),
      // UINT32  NoOfPrivateResources
      CORE_RESOURCE_COUNT,
      // CM_OBJECT_TOKEN  PrivateResourcesArrayToken
      REFERENCE_TOKEN (CoreResources)
    },
  },

  // Processor Node Id Info
  {
    // A unique token used to identify this object
    REFERENCE_TOKEN (ProcNodeIdInfo),
    // Vendor ID (as described in ACPI ID registry)
    SIGNATURE_32('A', 'R', 'M', 'H'),
    // First level unique node ID
    0,
    // Second level unique node ID
    0,
    // Major revision of the node
    0,
    // Minor revision of the node
    0,
    // Spin revision of the node
    0
  },

  // Cache information
  {
    // 'cluster's L3 cache
    {
      REFERENCE_TOKEN (CacheInfo[0]),  // CM_OBJECT_TOKEN  Token
      CM_NULL_TOKEN,                   // CM_OBJECT_TOKEN  NextLevelOfCacheToken
      SIZE_1MB,                        // UINT32  Size
      2048,                            // UINT32  NumberOfSets
      8,                               // UINT32  Associativity
      CACHE_ATTRIBUTES (               // UINT8   Attributes
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_ALLOCATION_READ,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_CACHE_TYPE_UNIFIED,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
      ),
      64                               // UINT16  LineSize
    },
    // 'core's L1 instruction cache
    {
      REFERENCE_TOKEN (CacheInfo[1]),  // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (CacheInfo[3]),  // CM_OBJECT_TOKEN  NextLevelOfCacheToken
      SIZE_64KB,                       // UINT32  Size
      256,                             // UINT32  NumberOfSets
      4,                               // UINT32  Associativity
      CACHE_ATTRIBUTES (               // UINT8   Attributes
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_ALLOCATION_READ,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_CACHE_TYPE_INSTRUCTION,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
      ),
      64                               // UINT16  LineSize
    },
    // 'core's L1 data cache
    {
      REFERENCE_TOKEN (CacheInfo[2]),  // CM_OBJECT_TOKEN  Token
      REFERENCE_TOKEN (CacheInfo[3]),  // CM_OBJECT_TOKEN  NextLevelOfCacheToken
      SIZE_64KB,                       // UINT32  Size
      256,                             // UINT32  NumberOfSets
      4,                               // UINT32  Associativity
      CACHE_ATTRIBUTES (               // UINT8   Attributes
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_ALLOCATION_READ_WRITE,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_CACHE_TYPE_DATA,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
      ),
      64                               // UINT16  LineSize
    },
    // cores's L2 cache
    {
      REFERENCE_TOKEN (CacheInfo[3]),  // CM_OBJECT_TOKEN  Token
      CM_NULL_TOKEN,                   // CM_OBJECT_TOKEN  NextLevelOfCacheToken
      SIZE_1MB,                        // UINT32  Size
      2048,                            // UINT32  NumberOfSets
      8,                               // UINT32  Associativity
      CACHE_ATTRIBUTES (               // UINT8   Attributes
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_ALLOCATION_READ,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_CACHE_TYPE_UNIFIED,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
      ),
      64                               // UINT16  LineSize
    },
    // slc cache
    {
      REFERENCE_TOKEN (CacheInfo[4]),  // CM_OBJECT_TOKEN  Token
      CM_NULL_TOKEN,                   // CM_OBJECT_TOKEN  NextLevelOfCacheToken
      SIZE_8MB,                        // UINT32  Size
      4096,                            // UINT32  NumberOfSets
      16,                              // UINT32  Associativity
      CACHE_ATTRIBUTES (               // UINT8   Attributes
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_ALLOCATION_READ,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_CACHE_TYPE_UNIFIED,
        EFI_ACPI_6_3_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
      ),
      64                               // UINT16  LineSize
    },
  },
  // Resources private to the 'cluster (shared among cores) in Cluster
  {
    { REFERENCE_TOKEN (CacheInfo[0]) }  // -> 'cluster's L3 cache in Cluster
  },
  // Resources private to each individual 'core instance in Cluster
  {
    { REFERENCE_TOKEN (CacheInfo[1]) }, // -> 'core's L1 I-cache in Cluster
    { REFERENCE_TOKEN (CacheInfo[2]) }, // -> 'core's L1 D-cache in Cluster
    { REFERENCE_TOKEN (CacheInfo[3]) }  // -> 'core's L2 cache in Cluster
  },

  // Resources private to the SoC
  {
    { REFERENCE_TOKEN (CacheInfo[4])   },  // -> slc for SoC
    { REFERENCE_TOKEN (ProcNodeIdInfo) },  // -> ProcNodeIdInfo for SoC
  },
};

/** Initialize the Platform Configuration Repository.

  @param [in]  This        Pointer to the Platform Configuration Repository.

  @retval
    EFI_SUCCESS   Success
**/
STATIC
EFI_STATUS
EFIAPI
InitializePlatformRepository (
  IN  EDKII_PLATFORM_REPOSITORY_INFO  * CONST PlatformRepo
  )
{
  return EFI_SUCCESS;
}

/** Return a GT Block timer frame info list.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       A token for identifying the object
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetGTBlockTimerFrameInfo (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EDKII_COMMON_PLATFORM_REPOSITORY_INFO   * PlatformRepo;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  PlatformRepo = This->PlatRepoInfo->CommonPlatRepoInfo;

  if (Token != (CM_OBJECT_TOKEN)&PlatformRepo->GTBlock0TimerInfo) {
    return EFI_NOT_FOUND;
  }

  CmObject->ObjectId = CmObjectId;
  CmObject->Size = sizeof (PlatformRepo->GTBlock0TimerInfo);
  CmObject->Data = (VOID*)&PlatformRepo->GTBlock0TimerInfo;
  CmObject->Count = sizeof (PlatformRepo->GTBlock0TimerInfo) /
                      sizeof (PlatformRepo->GTBlock0TimerInfo[0]);
  return EFI_SUCCESS;
}

/** Return GIC CPU Interface Info.

  @param [in]      This           Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId     The Object ID of the CM object requested
  @param [in]      SearchToken    A unique token for identifying the requested
                                  CM_ARM_GICC_INFO object.
  @param [in, out] CmObject       Pointer to the Configuration Manager Object
                                  descriptor describing the requested Object.

  @retval EFI_SUCCESS             Success.
  @retval EFI_INVALID_PARAMETER   A parameter is invalid.
  @retval EFI_NOT_FOUND           The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetGicCInfo (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               SearchToken,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EDKII_COMMON_PLATFORM_REPOSITORY_INFO  * PlatformRepo;
  UINT32                                   TotalObjCount;
  UINT32                                   ObjIndex;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  PlatformRepo = This->PlatRepoInfo->CommonPlatRepoInfo;
  TotalObjCount = PLAT_CPU_COUNT;

  for (ObjIndex = 0; ObjIndex < TotalObjCount; ObjIndex++) {
    if (SearchToken == (CM_OBJECT_TOKEN)&PlatformRepo->GicCInfo[ObjIndex]) {
      CmObject->ObjectId = CmObjectId;
      CmObject->Size = sizeof (PlatformRepo->GicCInfo[ObjIndex]);
      CmObject->Data = (VOID*)&PlatformRepo->GicCInfo[ObjIndex];
      CmObject->Count = 1;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/** Return a list of Configuration Manager object references pointed to by the
    given input token.

  @param [in]      This           Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId     The Object ID of the CM object requested
  @param [in]      SearchToken    A unique token for identifying the requested
                                  CM_ARM_OBJ_REF list.
  @param [in, out] CmObject       Pointer to the Configuration Manager Object
                                  descriptor describing the requested Object.

  @retval EFI_SUCCESS             Success.
  @retval EFI_INVALID_PARAMETER   A parameter is invalid.
  @retval EFI_NOT_FOUND           The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetCmObjRefs (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               SearchToken,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EDKII_PLATFORM_REPOSITORY_INFO  * PlatformRepo;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  PlatformRepo = This->PlatRepoInfo;

  if (SearchToken == (CM_OBJECT_TOKEN)&PlatformRepo->CommonPlatRepoInfo->ClusterResources) {
    CmObject->Size = sizeof (PlatformRepo->CommonPlatRepoInfo->ClusterResources);
    CmObject->Data = (VOID*)&PlatformRepo->CommonPlatRepoInfo->ClusterResources;
    CmObject->Count = ARRAY_SIZE (PlatformRepo->CommonPlatRepoInfo->ClusterResources);
    return EFI_SUCCESS;
  }
  if (SearchToken == (CM_OBJECT_TOKEN)&PlatformRepo->CommonPlatRepoInfo->CoreResources) {
    CmObject->Size = sizeof (PlatformRepo->CommonPlatRepoInfo->CoreResources);
    CmObject->Data = (VOID*)&PlatformRepo->CommonPlatRepoInfo->CoreResources;
    CmObject->Count = ARRAY_SIZE (PlatformRepo->CommonPlatRepoInfo->CoreResources);
    return EFI_SUCCESS;
  }
  if (SearchToken == (CM_OBJECT_TOKEN)&PlatformRepo->CommonPlatRepoInfo->SocResources) {
    CmObject->Size = sizeof (PlatformRepo->CommonPlatRepoInfo->SocResources);
    CmObject->Data = (VOID*)&PlatformRepo->CommonPlatRepoInfo->SocResources;
    CmObject->Count = ARRAY_SIZE (PlatformRepo->CommonPlatRepoInfo->SocResources);
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

/** Return a standard namespace object.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetStandardNameSpaceObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EFI_STATUS                        Status;
  EDKII_PLATFORM_REPOSITORY_INFO  * PlatformRepo;

  Status = EFI_SUCCESS;
  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }
  PlatformRepo = This->PlatRepoInfo;

  switch (GET_CM_OBJECT_ID (CmObjectId)) {
    HANDLE_CM_OBJECT (
      EStdObjCfgMgrInfo,
      CmObjectId,
      PlatformRepo->CommonPlatRepoInfo->CmInfo,
      1
      );
    HANDLE_CM_OBJECT (
      EStdObjAcpiTableList,
      CmObjectId,
      PlatformRepo->FvpPlatRepoInfo->CmAcpiTableList,
      (sizeof (PlatformRepo->FvpPlatRepoInfo->CmAcpiTableList) /
        sizeof (PlatformRepo->FvpPlatRepoInfo->CmAcpiTableList[0]))
      );
    default: {
      Status = EFI_NOT_FOUND;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Object 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** Return an ARM namespace object.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetArmNameSpaceObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EFI_STATUS                        Status;
  EDKII_PLATFORM_REPOSITORY_INFO  * PlatformRepo;
  UINT32                            GicRedistCount;
  UINT32                            GicCpuCount;
  UINT32                            ProcHierarchyInfoCount;

  Status = EFI_SUCCESS;
  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }
  PlatformRepo = This->PlatRepoInfo;

  GicRedistCount = 1;
  GicCpuCount = PLAT_CPU_COUNT;
  ProcHierarchyInfoCount = PLAT_PROC_HIERARCHY_NODE_COUNT;

  Status = GetArmNameSpaceObjectPlat (This, CmObjectId, Token, CmObject);

  if (Status == EFI_NOT_FOUND) {

    Status = EFI_SUCCESS;

    switch (GET_CM_OBJECT_ID (CmObjectId)) {
      HANDLE_CM_OBJECT (
        EArmObjBootArchInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->BootArchInfo,
        1
        );

#ifdef HEADLESS_PLATFORM
      HANDLE_CM_OBJECT (
        EArmObjFixedFeatureFlags,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->FixedFeatureFlags,
        1
        );
#endif
      HANDLE_CM_OBJECT (
        EArmObjPowerManagementProfileInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->PmProfileInfo,
        1
        );
      HANDLE_CM_OBJECT (
        EArmObjGenericTimerInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->GenericTimerInfo,
        1
        );
      HANDLE_CM_OBJECT (
        EArmObjPlatformGenericWatchdogInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->Watchdog,
        1
        );
      HANDLE_CM_OBJECT (
        EArmObjPlatformGTBlockInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->GTBlockInfo,
        (sizeof (PlatformRepo->CommonPlatRepoInfo->GTBlockInfo) /
           sizeof (PlatformRepo->CommonPlatRepoInfo->GTBlockInfo[0]))
        );
      HANDLE_CM_OBJECT_REF_BY_TOKEN (
        EArmObjGTBlockTimerFrameInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->GTBlock0TimerInfo,
        (sizeof (PlatformRepo->CommonPlatRepoInfo->GTBlock0TimerInfo) /
           sizeof (PlatformRepo->CommonPlatRepoInfo->GTBlock0TimerInfo[0])),
        Token,
        GetGTBlockTimerFrameInfo
        );
      HANDLE_CM_OBJECT_REF_BY_TOKEN (
        EArmObjGicCInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->GicCInfo,
        GicCpuCount,
        Token,
        GetGicCInfo
        );
      HANDLE_CM_OBJECT (
        EArmObjGicDInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->GicDInfo,
        1
        );
      HANDLE_CM_OBJECT (
        EArmObjGicRedistributorInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->GicRedistInfo,
        GicRedistCount
        );
      HANDLE_CM_OBJECT (
        EArmObjSerialConsolePortInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->SpcrSerialPort,
        1
        );
      HANDLE_CM_OBJECT (
        EArmObjSerialDebugPortInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->DbgSerialPort,
        1
        );
      HANDLE_CM_OBJECT (
        EArmObjProcHierarchyInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->ProcHierarchyInfo,
        ProcHierarchyInfoCount
        );
      HANDLE_CM_OBJECT (
        EArmObjProcNodeIdInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->ProcNodeIdInfo,
        1
        );
      HANDLE_CM_OBJECT (
        EArmObjCacheInfo,
        CmObjectId,
        PlatformRepo->CommonPlatRepoInfo->CacheInfo,
        ARRAY_SIZE (PlatformRepo->CommonPlatRepoInfo->CacheInfo)
        );
      HANDLE_CM_OBJECT_SEARCH_PLAT_REPO (
        EArmObjCmRef,
        CmObjectId,
        Token,
        GetCmObjRefs
        );
      default: {
        Status = EFI_NOT_FOUND;
        DEBUG ((
          DEBUG_INFO,
          "INFO: Object 0x%x. Status = %r\n",
          CmObjectId,
          Status
          ));
        break;
      }
    }//switch
  }
  return Status;
}

/** Return an OEM namespace object.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
GetOemNameSpaceObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  switch (GET_CM_OBJECT_ID (CmObjectId)) {
    default: {
      Status = EFI_NOT_FOUND;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Object 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** The GetObject function defines the interface implemented by the
    Configuration Manager Protocol for returning the Configuration
    Manager Objects.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in, out] CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the requested Object.

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         The required object information is not found.
**/
EFI_STATUS
EFIAPI
MorelloPlatformGetObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN  OUT   CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  EFI_STATUS  Status;

  if ((This == NULL) || (CmObject == NULL)) {
    ASSERT (This != NULL);
    ASSERT (CmObject != NULL);
    return EFI_INVALID_PARAMETER;
  }

  switch (GET_CM_NAMESPACE_ID (CmObjectId)) {
    case EObjNameSpaceStandard:
      Status = GetStandardNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    case EObjNameSpaceArm:
      Status = GetArmNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    case EObjNameSpaceOem:
      Status = GetOemNameSpaceObject (This, CmObjectId, Token, CmObject);
      break;
    default: {
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: Unknown Namespace Object = 0x%x. Status = %r\n",
        CmObjectId,
        Status
        ));
      break;
    }
  }

  return Status;
}

/** The SetObject function defines the interface implemented by the
    Configuration Manager Protocol for updating the Configuration
    Manager Objects.

  @param [in]      This        Pointer to the Configuration Manager Protocol.
  @param [in]      CmObjectId  The Configuration Manager Object ID.
  @param [in]      Token       An optional token identifying the object. If
                               unused this must be CM_NULL_TOKEN.
  @param [in]      CmObject    Pointer to the Configuration Manager Object
                               descriptor describing the Object.

  @retval EFI_UNSUPPORTED  This operation is not supported.
**/
EFI_STATUS
EFIAPI
MorelloPlatformSetObject (
  IN  CONST EDKII_CONFIGURATION_MANAGER_PROTOCOL  * CONST This,
  IN  CONST CM_OBJECT_ID                                  CmObjectId,
  IN  CONST CM_OBJECT_TOKEN                               Token OPTIONAL,
  IN        CM_OBJ_DESCRIPTOR                     * CONST CmObject
  )
{
  return EFI_UNSUPPORTED;
}

/** A structure describing the configuration manager protocol interface.
*/
STATIC
CONST
EDKII_CONFIGURATION_MANAGER_PROTOCOL MorelloPlatformConfigManagerProtocol = {
  CREATE_REVISION(1,0),
  MorelloPlatformGetObject,
  MorelloPlatformSetObject,
  &MorelloRepositoryInfo
};

/**
  Entrypoint of Configuration Manager Dxe.

  @param  ImageHandle
  @param  SystemTable

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
ConfigurationManagerDxeInitialize (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_SYSTEM_TABLE  * SystemTable
  )
{
  EFI_STATUS         Status;

  // Initialize the Platform Configuration Repository before installing the
  // Configuration Manager Protocol
  Status = InitializePlatformRepository (
    MorelloPlatformConfigManagerProtocol.PlatRepoInfo
    );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: Failed to initialize the Platform Configuration Repository." \
      " Status = %r\n",
      Status
      ));
  }

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEdkiiConfigurationManagerProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID*)&MorelloPlatformConfigManagerProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: Failed to get Install Configuration Manager Protocol." \
      " Status = %r\n",
      Status
      ));
    goto error_handler;
  }

error_handler:
  return Status;
}
