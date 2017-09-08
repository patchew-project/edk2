/** @file

  Copyright (c) 2016 Socionext Inc. All rights reserved.<BR>
  Copyright (c) 2017, Linaro, Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DebugLib.h>
#include <Library/DmaLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/NetLib.h>

#include "NetsecDxe.h"
#include "netsec_for_uefi/pfdep.h"

EFI_CPU_ARCH_PROTOCOL      *mCpu;

STATIC NETSEC_DEVICE_PATH NetsecPathTemplate =  {
  {
    {
      MESSAGING_DEVICE_PATH,
      MSG_MAC_ADDR_DP,
      {
        (UINT8) (sizeof(MAC_ADDR_DEVICE_PATH)),
        (UINT8) (sizeof(MAC_ADDR_DEVICE_PATH) >> 8)
      }
    },
    {
      {
        0
      }
    },
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      sizeof(EFI_DEVICE_PATH_PROTOCOL), 0
    }
  }
};

STATIC
VOID
GetCurrentMacAddress(
  OUT UINT8   *Mac)
{
  Mac[0] = MmioRead8(MAC_ADDRESS + 3);
  Mac[1] = MmioRead8(MAC_ADDRESS + 2);
  Mac[2] = MmioRead8(MAC_ADDRESS + 1);
  Mac[3] = MmioRead8(MAC_ADDRESS + 0);
  Mac[4] = MmioRead8(MAC_ADDRESS + 7);
  Mac[5] = MmioRead8(MAC_ADDRESS + 6);
}

/*
 *  Probe()
 */
STATIC
EFI_STATUS
Probe (
  IN  EFI_HANDLE          Handle,
  IN  NETSEC_DRIVER       *LanDriver
  )
{
  ogma_param_t  Param;
  ogma_err_t    ogma_err;
  UINT64        dmac_hm_cmd_base, dmac_mh_cmd_base, core_cmd_base;
  UINT32        dmac_hm_cmd_size, dmac_mh_cmd_size, core_cmd_size;

  SetMem (&Param, sizeof(Param), 0);

  Param.use_gmac_flag = OGMA_TRUE;

  Param.use_jumbo_pkt_flag = PcdGet8 (PcdJumboPacket);

  Param.desc_ring_param[OGMA_DESC_RING_ID_NRM_TX].valid_flag = OGMA_TRUE;
  Param.desc_ring_param[OGMA_DESC_RING_ID_NRM_TX].little_endian_flag = OGMA_TRUE;
  Param.desc_ring_param[OGMA_DESC_RING_ID_NRM_TX].tmr_mode_flag = OGMA_FALSE;
  Param.desc_ring_param[OGMA_DESC_RING_ID_NRM_TX].entry_num = PcdGet16 (PcdEncTxDescNum);
  Param.desc_ring_param[OGMA_DESC_RING_ID_NRM_RX].valid_flag = OGMA_TRUE;
  Param.desc_ring_param[OGMA_DESC_RING_ID_NRM_RX].little_endian_flag = OGMA_TRUE;
  Param.desc_ring_param[OGMA_DESC_RING_ID_NRM_RX].tmr_mode_flag = OGMA_FALSE;
  Param.desc_ring_param[OGMA_DESC_RING_ID_NRM_RX].entry_num = PcdGet16 (PcdDecRxDescNum);

  // phy-interface
  Param.gmac_config.phy_interface = OGMA_PHY_INTERFACE_RGMII;

  // Read and save the Permanent MAC Address
  GetCurrentMacAddress (LanDriver->SnpMode.PermanentAddress.Addr);

  LanDriver->SnpMode.CurrentAddress = LanDriver->SnpMode.PermanentAddress;
  DEBUG ((DEBUG_NET | DEBUG_INFO,
    "Netsec: HW MAC Address: %02x-%02x-%02x-%02x-%02x-%02x\n",
    LanDriver->SnpMode.PermanentAddress.Addr[0],
    LanDriver->SnpMode.PermanentAddress.Addr[1],
    LanDriver->SnpMode.PermanentAddress.Addr[2],
    LanDriver->SnpMode.PermanentAddress.Addr[3],
    LanDriver->SnpMode.PermanentAddress.Addr[4],
    LanDriver->SnpMode.PermanentAddress.Addr[5]));

  // Get hm microcode's physical addresses
  dmac_hm_cmd_base = MmioRead32 (HM_ME_ADDRESS_H);
  dmac_hm_cmd_base <<= 32;
  dmac_hm_cmd_base |= MmioRead32 (HM_ME_ADDRESS_L);
  dmac_hm_cmd_size = MmioRead32 (HM_ME_SIZE);

  // Get mh microcode's physical addresses
  dmac_mh_cmd_base = MmioRead32 (MH_ME_ADDRESS_H);
  dmac_mh_cmd_base <<= 32;
  dmac_mh_cmd_base |= MmioRead32 (MH_ME_ADDRESS_L);
  dmac_mh_cmd_size = MmioRead32 (MH_ME_SIZE);

  // Get core microcode's physical addresses
  core_cmd_base = MmioRead32 (PACKET_ME_ADDRESS);
  core_cmd_size = MmioRead32 (PACKET_ME_SIZE);

  ogma_err = ogma_init ((VOID *)((UINTN)PcdGet32(PcdNetsecDxeBaseAddress)),
                        Handle, &Param,
                        (VOID *)dmac_hm_cmd_base, dmac_hm_cmd_size,
                        (VOID *)dmac_mh_cmd_base, dmac_mh_cmd_size,
                        (VOID *)core_cmd_base, core_cmd_size,
                        &LanDriver->Handle);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR, "NETSEC: ogma_init() failed with error code %d\n",
      ogma_err));
    return EFI_DEVICE_ERROR;
  }

  ogma_enable_top_irq (LanDriver->Handle,
                       OGMA_TOP_IRQ_REG_NRM_RX | OGMA_TOP_IRQ_REG_NRM_TX);

  return EFI_SUCCESS;
}

/*
 *  UEFI Stop() function
 */
STATIC
EFI_STATUS
EFIAPI
SnpStop (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL    *Snp
  )
{
  EFI_TPL       SavedTpl;
  EFI_STATUS    Status;

  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Serialize access to data and registers
  SavedTpl = gBS->RaiseTPL (NETSEC_TPL);

  // Check state of the driver
  switch (Snp->Mode->State) {
  case EfiSimpleNetworkStarted:
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStopped:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not started\n"));
    ReturnUnlock (EFI_NOT_STARTED);
  default:
    DEBUG ((DEBUG_ERROR, "NETSEC: Driver in an invalid state: %u\n",
      (UINTN)Snp->Mode->State));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  // Change the state
  Snp->Mode->State = EfiSimpleNetworkStopped;
  Status = EFI_SUCCESS;

  // Restore TPL and return
ExitUnlock:
  gBS->RestoreTPL (SavedTpl);
  return Status;
}

/*
 *  UEFI Initialize() function
 */
STATIC
EFI_STATUS
EFIAPI
SnpInitialize (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *Snp,
  IN  UINTN                         RxBufferSize  OPTIONAL,
  IN  UINTN                         TxBufferSize  OPTIONAL
  )
{
  NETSEC_DRIVER           *LanDriver;
  EFI_TPL                 SavedTpl;
  EFI_STATUS              Status;

  ogma_phy_link_status_t  phy_link_status;
  ogma_err_t              ogma_err;
  ogma_gmac_mode_t        ogma_gmac_mode;

  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Serialize access to data and registers
  SavedTpl = gBS->RaiseTPL (NETSEC_TPL);

  // Check that driver was started but not initialised
  switch (Snp->Mode->State) {
  case EfiSimpleNetworkStarted:
    break;
  case EfiSimpleNetworkInitialized:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver already initialized\n"));
    ReturnUnlock (EFI_SUCCESS);
  case EfiSimpleNetworkStopped:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not started\n"));
    ReturnUnlock (EFI_NOT_STARTED);
  default:
    DEBUG ((DEBUG_ERROR, "NETSEC: Driver in an invalid state: %u\n",
      (UINTN)Snp->Mode->State));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  // Find the LanDriver structure
  LanDriver = INSTANCE_FROM_SNP_THIS(Snp);

  // ##### open
  ogma_err = ogma_clean_rx_desc_ring (LanDriver->Handle,
                                      OGMA_DESC_RING_ID_NRM_RX);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_clean_rx_desc_ring() failed with error code %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  ogma_err = ogma_clean_tx_desc_ring (LanDriver->Handle,
                                      OGMA_DESC_RING_ID_NRM_TX);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_clean_tx_desc_ring() failed with error code %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  ogma_clear_desc_ring_irq_status (LanDriver->Handle, OGMA_DESC_RING_ID_NRM_TX,
                                   OGMA_CH_IRQ_REG_EMPTY);

  // ##### open_sub
  ogma_err = ogma_start_desc_ring (LanDriver->Handle, OGMA_DESC_RING_ID_NRM_RX);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_start_desc_ring(ring_id=%d) failed with error code %d\n",
      OGMA_DESC_RING_ID_NRM_RX,
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  ogma_err = ogma_set_irq_coalesce_param(LanDriver->Handle,
                                         OGMA_DESC_RING_ID_NRM_RX,
                                         RXINT_PKTCNT,
                                         OGMA_FALSE,
                                         RXINT_TMR_CNT_US);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_set_irq_coalesce_param() failed with error code %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  ogma_err = ogma_start_desc_ring (LanDriver->Handle, OGMA_DESC_RING_ID_NRM_TX);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_start_desc_ring(ring_id=%d) failed with error code %d\n",
      OGMA_DESC_RING_ID_NRM_TX,
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  ogma_disable_desc_ring_irq (LanDriver->Handle, OGMA_DESC_RING_ID_NRM_TX,
                              OGMA_CH_IRQ_REG_EMPTY);

  // ##### configure_mac
  ogma_err = ogma_stop_gmac(LanDriver->Handle, OGMA_TRUE, OGMA_TRUE);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_stop_gmac() failed with error status %d\n",
      ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  ogma_err = ogma_get_phy_link_status(LanDriver->Handle, PcdGet8(PcdPhyDevAddr), &phy_link_status);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_get_phy_link_status() failed error code %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  SetMem (&ogma_gmac_mode, sizeof(ogma_gmac_mode_t), 0);
  ogma_gmac_mode.link_speed = phy_link_status.link_speed;
  ogma_gmac_mode.half_duplex_flag = (ogma_bool)phy_link_status.half_duplex_flag;
  if ((!phy_link_status.half_duplex_flag) && PcdGet8(PcdFlowCtrl)) {
    ogma_gmac_mode.flow_ctrl_enable_flag      = (ogma_bool)PcdGet8(PcdFlowCtrl);
    ogma_gmac_mode.flow_ctrl_start_threshold  = (ogma_uint16)PcdGet16(PcdFlowCtrlStartThreshold);
    ogma_gmac_mode.flow_ctrl_stop_threshold   = (ogma_uint16)PcdGet16(PcdFlowCtrlStopThreshold);
    ogma_gmac_mode.pause_time                 = (ogma_uint16)PcdGet16(PcdPauseTime);
  }

  ogma_err = ogma_set_gmac_mode(LanDriver->Handle, &ogma_gmac_mode);
  if(ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_set_gmac() failed with error status %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  ogma_err = ogma_start_gmac(LanDriver->Handle, OGMA_TRUE, OGMA_TRUE);
  if(ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_start_gmac() failed with error status %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  // Declare the driver as initialized
  Snp->Mode->State = EfiSimpleNetworkInitialized;
  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO | DEBUG_LOAD, "NETSEC: Driver started\n"));

  // Restore TPL and return
ExitUnlock:
  gBS->RestoreTPL (SavedTpl);
  return Status;
}

/*
 *  UEFI Shutdown () function
 */
STATIC
EFI_STATUS
EFIAPI
SnpShutdown (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL    *Snp
  )
{
  NETSEC_DRIVER     *LanDriver;
  EFI_TPL           SavedTpl;
  EFI_STATUS        Status;

  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Serialize access to data and registers
  SavedTpl = gBS->RaiseTPL (NETSEC_TPL);

  // First check that driver has already been initialized
  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not yet initialized\n"));
    ReturnUnlock (EFI_DEVICE_ERROR);
  case EfiSimpleNetworkStopped:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver in stopped state\n"));
    ReturnUnlock (EFI_NOT_STARTED);
  default:
    DEBUG ((DEBUG_ERROR, "NETSEC: Driver in an invalid state: %u\n",
      (UINTN)Snp->Mode->State));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  // Find the LanDriver structure
  LanDriver = INSTANCE_FROM_SNP_THIS(Snp);

  ogma_stop_gmac (LanDriver->Handle, OGMA_TRUE, OGMA_TRUE);

  ogma_stop_desc_ring (LanDriver->Handle, OGMA_DESC_RING_ID_NRM_RX);
  ogma_stop_desc_ring (LanDriver->Handle, OGMA_DESC_RING_ID_NRM_TX);

  Snp->Mode->State = EfiSimpleNetworkStarted;
  Status = EFI_SUCCESS;

  // Restore TPL and return
ExitUnlock:
  gBS->RestoreTPL (SavedTpl);
  return Status;
}

STATIC
VOID
EFIAPI
NotifyExitBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_SIMPLE_NETWORK_PROTOCOL     *Snp;
  EFI_STATUS                      Status;

  Snp = Context;

  if (Snp->Mode != EfiSimpleNetworkStopped) {
    Status = SnpShutdown (Snp);
    if (!EFI_ERROR (Status)) {
      SnpStop (Snp);
    }
  }
  gBS->CloseEvent (Event);
}

/*
 *  UEFI Start() function
 */
STATIC
EFI_STATUS
EFIAPI
SnpStart (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL    *Snp
  )
{
  EFI_SIMPLE_NETWORK_MODE   *Mode;
  EFI_TPL                   SavedTpl;
  EFI_STATUS                Status;
  NETSEC_DRIVER             *LanDriver;

  LanDriver = INSTANCE_FROM_SNP_THIS(Snp);

  // Check Snp instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Serialize access to data and registers
  SavedTpl = gBS->RaiseTPL (NETSEC_TPL);
  Mode = Snp->Mode;

  // Check state of the driver
  switch (Mode->State) {
  case EfiSimpleNetworkStopped:
    break;
  case EfiSimpleNetworkStarted:
  case EfiSimpleNetworkInitialized:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver already started\n"));
    ReturnUnlock (EFI_ALREADY_STARTED);
  default:
    DEBUG ((DEBUG_ERROR, "NETSEC: Driver in an invalid state: %u\n",
      (UINTN)Snp->Mode->State));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  Status = gBS->CreateEvent (EVT_SIGNAL_EXIT_BOOT_SERVICES, NETSEC_TPL,
                  NotifyExitBoot, Snp, &LanDriver->ExitBootEvent);
  ASSERT_EFI_ERROR (Status);

  // Change state
  Mode->State = EfiSimpleNetworkStarted;
  Status = EFI_SUCCESS;

  // Restore TPL and return
ExitUnlock:
  gBS->RestoreTPL (SavedTpl);
  return Status;

}

/*
 *  UEFI ReceiveFilters() function
 */
STATIC
EFI_STATUS
EFIAPI
SnpReceiveFilters (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL     *Snp,
  IN  UINT32                          Enable,
  IN  UINT32                          Disable,
  IN  BOOLEAN                         Reset,
  IN  UINTN                           NumMfilter  OPTIONAL,
  IN  EFI_MAC_ADDRESS                 *Mfilter    OPTIONAL
  )
{
  EFI_TPL             SavedTpl;
  EFI_STATUS          Status;

  // Check Snp Instance
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Serialize access to data and registers
  SavedTpl = gBS->RaiseTPL (NETSEC_TPL);

  // First check that driver has already been initialized
  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not yet initialized\n"));
    ReturnUnlock (EFI_DEVICE_ERROR);
  case EfiSimpleNetworkStopped:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not started\n"));
    ReturnUnlock (EFI_NOT_STARTED);
  default:
    DEBUG ((DEBUG_ERROR, "NETSEC: Driver in an invalid state: %u\n", (UINTN)Snp->Mode->State));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  Status = EFI_SUCCESS;

  // Restore TPL and return
ExitUnlock:
  gBS->RestoreTPL (SavedTpl);
  return Status;
}

/*
 *  UEFI GetStatus () function
 */
STATIC
EFI_STATUS
EFIAPI
SnpGetStatus (
  IN      EFI_SIMPLE_NETWORK_PROTOCOL   *Snp,
      OUT UINT32                        *IrqStat  OPTIONAL,
      OUT VOID                          **TxBuff  OPTIONAL
  )
{
  NETSEC_DRIVER             *LanDriver;
  EFI_TPL                   SavedTpl;
  EFI_STATUS                Status;
  pfdep_pkt_handle_t        pkt_handle;
  LIST_ENTRY                *Link;

  ogma_phy_link_status_t  phy_link_status;
  ogma_err_t              ogma_err;

  // Check preliminaries
  if (Snp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Serialize access to data and registers
  SavedTpl = gBS->RaiseTPL (NETSEC_TPL);

  // Check that driver was started and initialised
  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not yet initialized\n"));
    ReturnUnlock (EFI_DEVICE_ERROR);
  case EfiSimpleNetworkStopped:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not started\n"));
    ReturnUnlock (EFI_NOT_STARTED);
  default:
    DEBUG ((DEBUG_ERROR, "NETSEC: Driver in an invalid state: %u\n",
      (UINTN)Snp->Mode->State));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  // Find the LanDriver structure
  LanDriver = INSTANCE_FROM_SNP_THIS(Snp);

  // Update the media status
  ogma_err = ogma_get_phy_link_status (LanDriver->Handle,
                                       PcdGet8(PcdPhyDevAddr),
                                       &phy_link_status);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_get_phy_link_status failed with error code: %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  Snp->Mode->MediaPresent = phy_link_status.up_flag;

  ogma_err = ogma_clean_tx_desc_ring (LanDriver->Handle,
                                      OGMA_DESC_RING_ID_NRM_TX);

  if (TxBuff != NULL) {
    *TxBuff = NULL;
    //
    // Find a buffer in the list that has been released
    //
    for (Link = GetFirstNode (&LanDriver->TxBufferList);
         !IsNull (&LanDriver->TxBufferList, Link);
         Link = GetNextNode (&LanDriver->TxBufferList, Link)) {

      pkt_handle = BASE_CR (Link, PACKET_HANDLE, Link);
      if (pkt_handle->Released) {
        *TxBuff = pkt_handle->Buffer;
        RemoveEntryList (Link);
        FreePool (pkt_handle);
        break;
      }
    }
  }

  if (IrqStat != 0) {
    *IrqStat = 0;
  }

  Status = EFI_SUCCESS;

  // Restore TPL and return
ExitUnlock:
  gBS->RestoreTPL (SavedTpl);
  return Status;
}

/*
 *  UEFI Transmit() function
 */
STATIC
EFI_STATUS
EFIAPI
SnpTransmit (
  IN  EFI_SIMPLE_NETWORK_PROTOCOL   *Snp,
  IN  UINTN                         HdrSize,
  IN  UINTN                         BufSize,
  IN  VOID                          *BufAddr,
  IN  EFI_MAC_ADDRESS               *SrcAddr    OPTIONAL,
  IN  EFI_MAC_ADDRESS               *DstAddr    OPTIONAL,
  IN  UINT16                        *Protocol   OPTIONAL
  )
{
  NETSEC_DRIVER             *LanDriver;
  EFI_TPL                   SavedTpl;
  EFI_STATUS                Status;

  ogma_tx_pkt_ctrl_t  tx_pkt_ctrl;
  ogma_frag_info_t    scat_info;
  ogma_uint16         tx_avail_num;
  ogma_err_t          ogma_err;
  UINT16              Proto;
  pfdep_pkt_handle_t  pkt_handle;

  // Check preliminaries
  if ((Snp == NULL) || (BufAddr == NULL)) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: SnpTransmit(): NULL Snp (%p) or BufAddr (%p)\n", Snp, BufAddr));
    return EFI_DEVICE_ERROR;
  }

  pkt_handle = AllocateZeroPool (sizeof *pkt_handle);
  if (pkt_handle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  pkt_handle->Buffer = BufAddr;
  pkt_handle->RecycleForTx = TRUE;

  // Serialize access to data and registers
  SavedTpl = gBS->RaiseTPL (NETSEC_TPL);

  // Check that driver was started and initialised
  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not yet initialized\n"));
    ReturnUnlock (EFI_DEVICE_ERROR);
  case EfiSimpleNetworkStopped:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not started\n"));
    ReturnUnlock (EFI_NOT_STARTED);
  default:
    DEBUG ((DEBUG_ERROR, "NETSEC: Driver in an invalid state: %u\n",
      (UINTN)Snp->Mode->State));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  // Find the LanDriver structure
  LanDriver = INSTANCE_FROM_SNP_THIS(Snp);

  ogma_err = ogma_clear_desc_ring_irq_status (LanDriver->Handle,
                                              OGMA_DESC_RING_ID_NRM_TX,
                                              OGMA_CH_IRQ_REG_EMPTY);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_clear_desc_ring_irq_status failed with error code: %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  ogma_err = ogma_clean_tx_desc_ring (LanDriver->Handle,
                                      OGMA_DESC_RING_ID_NRM_TX);
  if (ogma_err != OGMA_ERR_OK) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_clean_tx_desc_ring failed with error code: %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  // Ensure header is correct size if non-zero
  if (HdrSize) {
    if (HdrSize != Snp->Mode->MediaHeaderSize) {
      DEBUG ((DEBUG_ERROR, "NETSEC: SnpTransmit(): Invalid HdrSize %d\n",
        HdrSize));
      ReturnUnlock (EFI_INVALID_PARAMETER);
    }

    if ((DstAddr == NULL) || (Protocol == NULL)) {
      DEBUG ((DEBUG_ERROR,
        "NETSEC: SnpTransmit(): NULL DstAddr %p or Protocol %p\n",
        DstAddr, Protocol));
      ReturnUnlock (EFI_INVALID_PARAMETER);
    }

    // Copy destination address
    CopyMem (BufAddr, (VOID *)DstAddr, NET_ETHER_ADDR_LEN);
    // Copy source address
    CopyMem (BufAddr + NET_ETHER_ADDR_LEN, (VOID *)SrcAddr, NET_ETHER_ADDR_LEN);
    // Copy protocol
    Proto = HTONS(*Protocol);
    CopyMem (BufAddr + (NET_ETHER_ADDR_LEN * 2), (VOID *)&Proto, sizeof(UINT16));
  }

  Status = DmaMap (MapOperationBusMasterRead, BufAddr, &BufSize,
             &scat_info.phys_addr, &pkt_handle->Mapping);
  if (EFI_ERROR (Status)) {
    goto ExitUnlock;
  }

  scat_info.addr        = BufAddr;
  scat_info.len         = BufSize;

  SetMem (&tx_pkt_ctrl, sizeof (ogma_tx_pkt_ctrl_t), 0);

  tx_pkt_ctrl.pass_through_flag     = OGMA_TRUE;
  tx_pkt_ctrl.target_desc_ring_id   = OGMA_DESC_RING_ID_GMAC;

  // check empty slot
  do {
    tx_avail_num = ogma_get_tx_avail_num (LanDriver->Handle,
                                          OGMA_DESC_RING_ID_NRM_TX);
  } while (tx_avail_num < SCAT_NUM);

  // send
  ogma_err = ogma_set_tx_pkt_data (LanDriver->Handle,
                                   OGMA_DESC_RING_ID_NRM_TX,
                                   &tx_pkt_ctrl,
                                   SCAT_NUM,
                                   &scat_info,
                                   pkt_handle);

  if (ogma_err != OGMA_ERR_OK) {
    DmaUnmap (pkt_handle->Mapping);
    FreePool (pkt_handle);
    DEBUG ((DEBUG_ERROR,
      "NETSEC: ogma_set_tx_pkt_data failed with error code: %d\n",
      (INT32)ogma_err));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  //
  // Queue the descriptor so we can release the buffer once it has been
  // consumed by the hardware.
  //
  InsertTailList (&LanDriver->TxBufferList, &pkt_handle->Link);

  gBS->RestoreTPL (SavedTpl);
  return EFI_SUCCESS;

  // Restore TPL and return
ExitUnlock:
  FreePool (pkt_handle);
  gBS->RestoreTPL (SavedTpl);
  return Status;
}

/*
 *  UEFI Receive() function
 */
EFI_STATUS
EFIAPI
SnpReceive (
  IN      EFI_SIMPLE_NETWORK_PROTOCOL   *Snp,
      OUT UINTN                         *HdrSize    OPTIONAL,
  IN  OUT UINTN                         *BuffSize,
      OUT VOID                          *Data,
      OUT EFI_MAC_ADDRESS               *SrcAddr    OPTIONAL,
      OUT EFI_MAC_ADDRESS               *DstAddr    OPTIONAL,
      OUT UINT16                        *Protocol   OPTIONAL
  )
{
  EFI_TPL             SavedTpl;
  EFI_STATUS          Status;
  NETSEC_DRIVER       *LanDriver;

  ogma_err_t          ogma_err;
  ogma_rx_pkt_info_t  rx_pkt_info;
  ogma_frag_info_t    rx_data;
  ogma_uint16         len;
  pfdep_pkt_handle_t  pkt_handle;

  // Check preliminaries
  if ((Snp == NULL) || (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  // Serialize access to data and registers
  SavedTpl = gBS->RaiseTPL (NETSEC_TPL);

  // Check that driver was started and initialised
  switch (Snp->Mode->State) {
  case EfiSimpleNetworkInitialized:
    break;
  case EfiSimpleNetworkStarted:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not yet initialized\n"));
    ReturnUnlock (EFI_DEVICE_ERROR);
  case EfiSimpleNetworkStopped:
    DEBUG ((DEBUG_WARN, "NETSEC: Driver not started\n"));
    ReturnUnlock (EFI_NOT_STARTED);
  default:
    DEBUG ((DEBUG_ERROR, "NETSEC: Driver in an invalid state: %u\n",
      (UINTN)Snp->Mode->State));
    ReturnUnlock (EFI_DEVICE_ERROR);
  }

  // Find the LanDriver structure
  LanDriver = INSTANCE_FROM_SNP_THIS(Snp);

  if (ogma_get_rx_num (LanDriver->Handle, OGMA_DESC_RING_ID_NRM_RX) > 0) {

    ogma_err = ogma_get_rx_pkt_data(LanDriver->Handle,
                                    OGMA_DESC_RING_ID_NRM_RX,
                                    &rx_pkt_info, &rx_data, &len, &pkt_handle);
    if (ogma_err != OGMA_ERR_OK) {
      DEBUG ((DEBUG_ERROR,
        "NETSEC: ogma_get_rx_pkt_data failed with error code: %d\n",
        (INT32)ogma_err));
      ReturnUnlock (EFI_DEVICE_ERROR);
    }

    DmaUnmap (pkt_handle->Mapping);
    pkt_handle->Mapping = NULL;

    CopyMem (Data, (VOID*)rx_data.addr, len);
    *BuffSize = len;

    pfdep_free_pkt_buf (LanDriver->Handle, rx_data.len, rx_data.addr,
      rx_data.phys_addr, PFDEP_TRUE, pkt_handle);
  } else {
    // not received any packets
    ReturnUnlock (EFI_NOT_READY);
  }

  if (HdrSize != NULL) {
    *HdrSize = LanDriver->SnpMode.MediaHeaderSize;
  }

  ogma_clear_desc_ring_irq_status (LanDriver->Handle,
                                   OGMA_DESC_RING_ID_NRM_TX,
                                   OGMA_CH_IRQ_REG_EMPTY);

  ogma_clean_tx_desc_ring(LanDriver->Handle, OGMA_DESC_RING_ID_NRM_TX);

  ogma_enable_top_irq (LanDriver->Handle,
                       OGMA_TOP_IRQ_REG_NRM_TX | OGMA_TOP_IRQ_REG_NRM_RX);

  Status = EFI_SUCCESS;

  // Restore TPL and return
ExitUnlock:
  gBS->RestoreTPL (SavedTpl);
  return Status;
}

/*
 *  Entry point for the Netxec driver
 */
EFI_STATUS
NetsecDxeEntry (
  IN  EFI_HANDLE        Handle,
  IN  EFI_SYSTEM_TABLE  *SystemTable)
{
  EFI_STATUS                  Status;
  NETSEC_DRIVER               *LanDriver;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
  EFI_SIMPLE_NETWORK_MODE     *SnpMode;
  NETSEC_DEVICE_PATH          *NetsecPath;

  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&mCpu);
  ASSERT_EFI_ERROR(Status);

  // Allocate Resources
  LanDriver = AllocateZeroPool (sizeof(NETSEC_DRIVER));
  NetsecPath = AllocateCopyPool (sizeof(NETSEC_DEVICE_PATH),
                                 &NetsecPathTemplate);

  // Initialize pointers
  Snp = &(LanDriver->Snp);
  SnpMode = &(LanDriver->SnpMode);
  Snp->Mode = SnpMode;

  // Set the signature of the LAN Driver structure
  LanDriver->Signature = NETSEC_SIGNATURE;

  // Probe the device
  Status = Probe (Handle, LanDriver);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR,
      "NETSEC:NetsecDxeEntry(): Probe failed with status %d\n", Status));
    return Status;
  }

  // Assign fields and func pointers
  Snp->Revision = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
  Snp->WaitForPacket = NULL;
  Snp->Initialize = SnpInitialize;
  Snp->Start = SnpStart;
  Snp->Stop = SnpStop;
  Snp->Reset = NULL;
  Snp->Shutdown = SnpShutdown;
  Snp->ReceiveFilters = SnpReceiveFilters;
  Snp->StationAddress = NULL;
  Snp->Statistics = NULL;
  Snp->MCastIpToMac = NULL;
  Snp->NvData = NULL;
  Snp->GetStatus = SnpGetStatus;
  Snp->Transmit = SnpTransmit;
  Snp->Receive = SnpReceive;

  // Fill in simple network mode structure
  SnpMode->State = EfiSimpleNetworkStopped;
  SnpMode->HwAddressSize = NET_ETHER_ADDR_LEN;    // HW address is 6 bytes
  SnpMode->MediaHeaderSize = sizeof(ETHER_HEAD);  // Size of an Ethernet header
  SnpMode->MaxPacketSize = EFI_PAGE_SIZE;         // Ethernet Frame (with VLAN tag +4 bytes)

  // Supported receive filters
  SnpMode->ReceiveFilterMask = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST |
                               EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST |
                               EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST |
                               EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS |
                               EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST;

  // Initially-enabled receive filters
  SnpMode->ReceiveFilterSetting = EFI_SIMPLE_NETWORK_RECEIVE_UNICAST |
                                  EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST |
                                  EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST;

  // Netsec has 64bit hash table. We can filter an infinite MACs, but
  // higher-level software must filter out any hash collisions.
  SnpMode->MaxMCastFilterCount = MAX_MCAST_FILTER_CNT;
  SnpMode->MCastFilterCount = 0;
  ZeroMem (&SnpMode->MCastFilter,
           MAX_MCAST_FILTER_CNT * sizeof(EFI_MAC_ADDRESS));

  // Set the interface type (1: Ethernet or 6: IEEE 802 Networks)
  SnpMode->IfType = NET_IFTYPE_ETHERNET;

  // Mac address is changeable
  SnpMode->MacAddressChangeable = TRUE;

  // We can only transmit one packet at a time
  SnpMode->MultipleTxSupported = FALSE;

  // MediaPresent checks for cable connection and partner link
  SnpMode->MediaPresentSupported = TRUE;
  SnpMode->MediaPresent = FALSE;

  //  Set broadcast address
  SetMem (&SnpMode->BroadcastAddress, sizeof (EFI_MAC_ADDRESS), 0xFF);

  // Assign fields for device path
  NetsecPath->Netsec.MacAddress = SnpMode->PermanentAddress;
  NetsecPath->Netsec.IfType = SnpMode->IfType;

  InitializeListHead (&LanDriver->TxBufferList);

  // Initialise the protocol
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &LanDriver->ControllerHandle,
                  &gEfiSimpleNetworkProtocolGuid, Snp,
                  &gEfiDevicePathProtocolGuid, NetsecPath,
                  NULL);

  // Say what the status of loading the protocol structure is
  if (EFI_ERROR(Status)) {
    ogma_terminate (LanDriver->Handle);
    FreePool (LanDriver);
  }

  return Status;
}
