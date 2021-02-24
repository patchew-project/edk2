/** @file

  Copyright (c) 2021, ARM Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Cm or CM   - Configuration Manager
    - Obj or OBJ - Object
**/

#ifndef CONFIGURATION_MANAGER_H__
#define CONFIGURATION_MANAGER_H__

/** The configuration manager version.
*/
#define CONFIGURATION_MANAGER_REVISION CREATE_REVISION (1, 0)

/** The OEM ID
*/
#define CFG_MGR_OEM_ID        { 'A', 'R', 'M', 'L', 'T', 'D' }
#define CFG_MGR_OEM_REVISION  0x20181101

/** A helper macro for mapping a reference token
*/
#define REFERENCE_TOKEN(Field)                                    \
  (CM_OBJECT_TOKEN)((UINT8*)&CommonPlatformInfo +                 \
    OFFSET_OF (EDKII_COMMON_PLATFORM_REPOSITORY_INFO, Field))

/** A helper macro that constructs the MPID based on the
    Aff0, Aff1, Aff2, Aff3 values
*/
#define GET_MPID3(Aff3, Aff2, Aff1, Aff0)                         \
  (((Aff3##ULL) << 32) | ((Aff2) << 16) | ((Aff1) << 8) | (Aff0))

/** A helper macro for populating the GIC CPU information
*/
#define GICC_ENTRY(                                                      \
          CPUInterfaceNumber,                                            \
          Mpidr,                                                         \
          PmuIrq,                                                        \
          VGicIrq,                                                       \
          GicRedistBase,                                                 \
          EnergyEfficiency,                                              \
          SpeIrq,                                                        \
          ProximityDomain,                                               \
          ClockDomain                                                    \
          ) {                                                            \
    CPUInterfaceNumber,       /* UINT32  CPUInterfaceNumber           */ \
    CPUInterfaceNumber,       /* UINT32  AcpiProcessorUid             */ \
    EFI_ACPI_6_2_GIC_ENABLED, /* UINT32  Flags                        */ \
    0,                        /* UINT32  ParkingProtocolVersion       */ \
    PmuIrq,                   /* UINT32  PerformanceInterruptGsiv     */ \
    0,                        /* UINT64  ParkedAddress                */ \
    FixedPcdGet64 (                                                      \
      PcdGicInterruptInterfaceBase                                       \
      ),                      /* UINT64  PhysicalBaseAddress          */ \
    0,                        /* UINT64  GICV                         */ \
    0,                        /* UINT64  GICH                         */ \
    VGicIrq,                  /* UINT32  VGICMaintenanceInterrupt     */ \
    GicRedistBase,            /* UINT64  GICRBaseAddress              */ \
    Mpidr,                    /* UINT64  MPIDR                        */ \
    EnergyEfficiency,         /* UINT8   ProcessorPowerEfficiencyClass*/ \
    SpeIrq,                   /* UINT16  SpeOverflowInterrupt         */ \
    ProximityDomain,          /* UINT32  ProximityDomain              */ \
    ClockDomain,              /* UINT32  ClockDomain                  */ \
    EFI_ACPI_6_3_GICC_ENABLED,/* UINT32  Flags                        */ \
    }

/** A helper macro for populating the Processor Hierarchy Node flags
*/
#define PROC_NODE_FLAGS(                                                \
          PhysicalPackage,                                              \
          AcpiProcessorIdValid,                                         \
          ProcessorIsThread,                                            \
          NodeIsLeaf,                                                   \
          IdenticalImplementation                                       \
          )                                                             \
  (                                                                     \
    PhysicalPackage |                                                   \
    (AcpiProcessorIdValid << 1) |                                       \
    (ProcessorIsThread << 2) |                                          \
    (NodeIsLeaf << 3) |                                                 \
    (IdenticalImplementation << 4)                                      \
  )

/** A helper macro for populating the Cache Type Structure's attributes
*/
#define CACHE_ATTRIBUTES(                                               \
          AllocationType,                                               \
          CacheType,                                                    \
          WritePolicy                                                   \
          )                                                             \
  (                                                                     \
    AllocationType |                                                    \
    (CacheType << 2) |                                                  \
    (WritePolicy << 4)                                                  \
  )

/** A helper macro for returning configuration manager objects
*/
#define HANDLE_CM_OBJECT(ObjId, CmObjectId, Object, ObjectCount)      \
  case ObjId: {                                                       \
    CmObject->ObjectId = CmObjectId;                                  \
    CmObject->Size = sizeof (Object);                                 \
    CmObject->Data = (VOID*)&Object;                                  \
    CmObject->Count = ObjectCount;                                    \
    DEBUG ((                                                          \
      DEBUG_INFO,                                                     \
      #CmObjectId ": Ptr = 0x%p, Size = %d, Count = %d\n",            \
      CmObject->Data,                                                 \
      CmObject->Size,                                                 \
      CmObject->Count                                                 \
      ));                                                             \
    break;                                                            \
  }

/** A helper macro for returning configuration manager objects
    referenced by token
*/
#define HANDLE_CM_OBJECT_REF_BY_TOKEN(                                      \
          ObjId,                                                            \
          CmObjectId,                                                       \
          Object,                                                           \
          ObjectCount,                                                      \
          Token,                                                            \
          HandlerProc                                                       \
          )                                                                 \
  case ObjId: {                                                             \
    CmObject->ObjectId = CmObjectId;                                        \
    if (Token == CM_NULL_TOKEN) {                                           \
      CmObject->Size = sizeof (Object);                                     \
      CmObject->Data = (VOID*)&Object;                                      \
      CmObject->Count = ObjectCount;                                        \
      DEBUG ((                                                              \
        DEBUG_INFO,                                                         \
        #CmObjectId ": Ptr = 0x%p, Size = %d, Count = %d\n",                \
        CmObject->Data,                                                     \
        CmObject->Size,                                                     \
        CmObject->Count                                                     \
        ));                                                                 \
    } else {                                                                \
      Status = HandlerProc (This, CmObjectId, Token, CmObject);             \
      DEBUG ((                                                              \
        DEBUG_INFO,                                                         \
        #CmObjectId ": Token = 0x%p, Ptr = 0x%p, Size = %d, Count = %d\n",  \
        (VOID*)Token,                                                       \
        CmObject->Data,                                                     \
        CmObject->Size,                                                     \
        CmObject->Count                                                     \
        ));                                                                 \
    }                                                                       \
    break;                                                                  \
  }

/** A helper macro for returning configuration manager objects referenced
    by token when the entire platform repository is in scope and the
    CM_NULL_TOKEN value is not allowed.
*/
#define HANDLE_CM_OBJECT_SEARCH_PLAT_REPO(                                  \
          ObjId,                                                            \
          CmObjectId,                                                       \
          Token,                                                            \
          HandlerProc                                                       \
          )                                                                 \
  case ObjId: {                                                             \
    CmObject->ObjectId = CmObjectId;                                        \
    if (Token == CM_NULL_TOKEN) {                                           \
      Status = EFI_INVALID_PARAMETER;                                       \
      DEBUG ((                                                              \
        DEBUG_ERROR,                                                        \
        #ObjId ": CM_NULL_TOKEN value is not allowed when searching"        \
        " the entire platform repository.\n"                                \
        ));                                                                 \
    } else {                                                                \
      Status = HandlerProc (This, CmObjectId, Token, CmObject);             \
      DEBUG ((                                                              \
        DEBUG_INFO,                                                         \
        #ObjId ": Token = 0x%p, Ptr = 0x%p, Size = %d, Count = %d\n",       \
        (VOID*)Token,                                                       \
        CmObject->Data,                                                     \
        CmObject->Size,                                                     \
        CmObject->Count                                                     \
        ));                                                                 \
    }                                                                       \
    break;                                                                  \
  }

/** The number of CPUs
*/
#define PLAT_CPU_COUNT              4

/** The number of platform generic timer blocks
*/
#define PLAT_GTBLOCK_COUNT          1

/** The number of timer frames per generic timer block
*/
#define PLAT_GTFRAME_COUNT          2

/** The number of Processor Hierarchy Nodes
    - one package node
    - two cluster nodes
    - two cores in cluster 0
    - two cores in cluster 1
*/
#define PLAT_PROC_HIERARCHY_NODE_COUNT  7

/** The number of unique cache structures:
    - cluster L3 unified cache
    - core L1 instruction cache
    - core L1 data cache
    - core L2 cache
    - slc unified cache
*/
#define PLAT_CACHE_COUNT                5

/** The number of resources private to the cluster
    - L3 cache
*/
#define CLUSTER_RESOURCE_COUNT  1

/** The number of resources private to 'core instance
    - L1 data cache
    - L1 instruction cache
    - L2 cache
*/
#define CORE_RESOURCE_COUNT  3

/** The number of resources private to SoC
    - slc cache
    - Proc Node Id Info
*/
#define SOC_RESOURCE_COUNT  2

/** A structure describing the platform configuration
    manager repository information
*/
typedef struct CommonPlatformRepositoryInfo {
  /// Configuration Manager Information
  CM_STD_OBJ_CONFIGURATION_MANAGER_INFO CmInfo;

  /// Boot architecture information
  CM_ARM_BOOT_ARCH_INFO                 BootArchInfo;

#ifdef HEADLESS_PLATFORM
  /// Fixed feature flag information
  CM_ARM_FIXED_FEATURE_FLAGS            FixedFeatureFlags;
#endif

  /// Power management profile information
  CM_ARM_POWER_MANAGEMENT_PROFILE_INFO  PmProfileInfo;

  /// GIC CPU interface information
  CM_ARM_GICC_INFO                      GicCInfo[PLAT_CPU_COUNT];

  /// GIC distributor information
  CM_ARM_GICD_INFO                      GicDInfo;

  /// GIC Redistributor information
  CM_ARM_GIC_REDIST_INFO                GicRedistInfo;

  /// Generic timer information
  CM_ARM_GENERIC_TIMER_INFO             GenericTimerInfo;

  /// Generic timer block information
  CM_ARM_GTBLOCK_INFO                   GTBlockInfo[PLAT_GTBLOCK_COUNT];

  /// Generic timer frame information
  CM_ARM_GTBLOCK_TIMER_FRAME_INFO       GTBlock0TimerInfo[PLAT_GTFRAME_COUNT];

  /// Watchdog information
  CM_ARM_GENERIC_WATCHDOG_INFO          Watchdog;

  /** Serial port information for the
      serial port console redirection port
  */
  CM_ARM_SERIAL_PORT_INFO               SpcrSerialPort;

  /// Serial port information for the DBG2 UART port
  CM_ARM_SERIAL_PORT_INFO               DbgSerialPort;

  // Processor topology information
  CM_ARM_PROC_HIERARCHY_INFO            ProcHierarchyInfo[PLAT_PROC_HIERARCHY_NODE_COUNT];

  // Processor Node Id Info
  CM_ARM_PROC_NODE_ID_INFO              ProcNodeIdInfo;


  // Cache information
  CM_ARM_CACHE_INFO                     CacheInfo[PLAT_CACHE_COUNT];

  // Cluster private resources
  CM_ARM_OBJ_REF                        ClusterResources[CLUSTER_RESOURCE_COUNT];

  // Core private resources
  CM_ARM_OBJ_REF                        CoreResources[CORE_RESOURCE_COUNT];

  // SoC Resources
  CM_ARM_OBJ_REF                        SocResources[SOC_RESOURCE_COUNT];

} EDKII_COMMON_PLATFORM_REPOSITORY_INFO;

#endif // CONFIGURATION_MANAGER_H__
