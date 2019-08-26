/** @file
  Header file for SMM Access Driver.

  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _SMM_ACCESS_H_
#define _SMM_ACCESS_H_

/**
  This function is to install an SMM Access PPI
  - <b>Introduction</b> \n
    A module to install a PPI for controlling SMM mode memory access basically for S3 resume usage.

  - @pre
    - _PEI_MASTER_BOOT_MODE_PEIM_PPI: A PPI published by foundation to get bootmode executed earlier.
    - _PEI_PERMANENT_MEMORY_INSTALLED_PPI: a PPI that will be installed after memory controller initialization completed to indicate that physical memory is usable after this point.

  - @result
    Publish _PEI_SMM_ACCESS_PPI.

    @retval EFI_SUCCESS           - Ppi successfully started and installed.
    @retval EFI_NOT_FOUND         - Ppi can't be found.
    @retval EFI_OUT_OF_RESOURCES  - Ppi does not have enough resources to initialize the driver.
**/
EFI_STATUS
EFIAPI
PeiInstallSmmAccessPpi (
  VOID
  );
#endif
