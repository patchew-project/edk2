#ifndef __RDK_BOOT_MANAGER_LIB_H__
#define __RDK_BOOT_MANAGER_LIB_H__

#include <Library/BdsLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/ShellLib.h>
#include <Library/DevicePathLib.h>
#include <Library/FileHandleLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/DiskIo.h>
#include <Protocol/BlockIo.h>
#include <Protocol/LoadFile.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/AndroidFastbootPlatform.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/AuthenticatedVariableFormat.h>
#include <HttpBootDxe/HttpBootDxe.h>
#include <Include/Guid/AuthenticatedVariableFormat.h>
#include "SecureBoot.h"
#include "HttpBoot.h"
#include "RdkFile.h"
#include "DiskIo.h"

#endif /* __RDK_BOOT_MANAGER_LIB_H__ */
