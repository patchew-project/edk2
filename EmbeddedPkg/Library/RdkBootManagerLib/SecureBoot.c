#include <RdkBootManagerLib.h>

STATIC
EFI_STATUS
OpenFileByDevicePath(
    IN OUT  EFI_DEVICE_PATH_PROTOCOL  **FilePath,
    OUT     EFI_FILE_HANDLE           *FileHandle,
    IN      UINT64                    OpenMode,
    IN      UINT64                    Attributes
)
{
    EFI_STATUS                        Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *EfiSimpleFileSystemProtocol;
    EFI_FILE_PROTOCOL                 *Handle1;
    EFI_FILE_PROTOCOL                 *Handle2;
    EFI_HANDLE                        DeviceHandle;

    //if ((FilePath == NULL || FileHandle == NULL)) {
    if ((FilePath == NULL )) {
        return EFI_INVALID_PARAMETER;
    }

    Status = gBS->LocateDevicePath (
                 &gEfiSimpleFileSystemProtocolGuid,
                 FilePath,
                 &DeviceHandle
             );
    if (EFI_ERROR (Status)) {
        return Status;
    }

    Status = gBS->OpenProtocol(
                 DeviceHandle,
                 &gEfiSimpleFileSystemProtocolGuid,
                 (VOID**)&EfiSimpleFileSystemProtocol,
                 gImageHandle,
                 NULL,
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
             );
    if (EFI_ERROR (Status)) {
        return Status;
    }

    Status = EfiSimpleFileSystemProtocol->OpenVolume(EfiSimpleFileSystemProtocol, &Handle1);
    if (EFI_ERROR (Status)) {
        FileHandle = NULL;
        return Status;
    }

    //
    // go down directories one node at a time.
    //
    while (!IsDevicePathEnd (*FilePath)) {
        //
        // For file system access each node should be a file path component
        //
        if (DevicePathType    (*FilePath) != MEDIA_DEVICE_PATH ||
                DevicePathSubType (*FilePath) != MEDIA_FILEPATH_DP
           ) {
            FileHandle = NULL;
            return (EFI_INVALID_PARAMETER);
        }
        //
        // Open this file path node
        //
        Handle2  = Handle1;
        Handle1 = NULL;

        //
        // Try to test opening an existing file
        //
        Status = Handle2->Open (
                     Handle2,
                     &Handle1,
                     ((FILEPATH_DEVICE_PATH*)*FilePath)->PathName,
                     OpenMode &~EFI_FILE_MODE_CREATE,
                     0
                 );

        //
        // see if the error was that it needs to be created
        //
        if ((EFI_ERROR (Status)) && (OpenMode != (OpenMode &~EFI_FILE_MODE_CREATE))) {
            Status = Handle2->Open (
                         Handle2,
                         &Handle1,
                         ((FILEPATH_DEVICE_PATH*)*FilePath)->PathName,
                         OpenMode,
                         Attributes
                     );
        }
        //
        // Close the last node
        //
        Handle2->Close (Handle2);

        if (EFI_ERROR(Status)) {
            return (Status);
        }

        //
        // Get the next node
        //
        *FilePath = NextDevicePathNode (*FilePath);
    }

    //
    // This is a weak spot since if the undefined SHELL_FILE_HANDLE format changes this must change also!
    //
    *FileHandle = (VOID*)Handle1;

    return EFI_SUCCESS;
}

EFI_STATUS
GetFileHandler (
    OUT EFI_FILE_HANDLE *FileHandle,
    IN  CONST CHAR16    *Path,
    IN  UINT64          OpenMode
)
{
    EFI_STATUS                          Status;
    EFI_DEVICE_PATH_PROTOCOL            *KeyFileDevicePath;
    EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL  *DevicePathFromTextProtocol;

    Status        = EFI_SUCCESS;
    KeyFileDevicePath   = NULL;

    Status = gBS->LocateProtocol (
                 &gEfiDevicePathFromTextProtocolGuid,
                 NULL,
                 (VOID**)&DevicePathFromTextProtocol
             );
    ASSERT_EFI_ERROR(Status);

    //  KeyFileDevicePath = gEfiShellProtocol->GetDevicePathFromFilePath(Path);
    KeyFileDevicePath =  DevicePathFromTextProtocol->ConvertTextToDevicePath(Path);
    if(KeyFileDevicePath != NULL)
    {
        Status = OpenFileByDevicePath(&KeyFileDevicePath,FileHandle,OpenMode,0);
        if(Status != EFI_SUCCESS)
        {
            DEBUG ((DEBUG_INFO, "Getting FileHandle of %s Failed\n",Path));
        }
    }
    return Status;
}

STATIC
EFI_STATUS
CreateTimeBasedPayload (
    IN OUT UINTN  *DataSize,
    IN OUT UINT8  **Data
)
{
    EFI_STATUS                       Status;
    UINT8                            *NewData;
    UINT8                            *Payload;
    UINTN                            PayloadSize;
    EFI_VARIABLE_AUTHENTICATION_2    *DescriptorData;
    UINTN                            DescriptorSize;
    EFI_TIME                         Time;

    if (Data == NULL || DataSize == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // In Setup mode or Custom mode, the variable does not need to be signed but the
    // parameters to the SetVariable() call still need to be prepared as authenticated
    // variable. So we create EFI_VARIABLE_AUTHENTICATED_2 descriptor without certificate
    // data in it.
    //

    Payload     = *Data;
    PayloadSize = *DataSize;

    DescriptorSize    = OFFSET_OF (EFI_VARIABLE_AUTHENTICATION_2, AuthInfo) + OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData);
    NewData = (UINT8*) AllocateZeroPool (DescriptorSize + PayloadSize);
    if (NewData == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    if ((Payload != NULL) && (PayloadSize != 0)) {
        CopyMem (NewData + DescriptorSize, Payload, PayloadSize);
    }

    DescriptorData = (EFI_VARIABLE_AUTHENTICATION_2 *) (NewData);

    ZeroMem (&Time, sizeof (EFI_TIME));
    Status = gRT->GetTime (&Time, NULL);
    if (EFI_ERROR (Status)) {
        FreePool(NewData);
        return Status;
    }
    Time.Pad1       = 0;
    Time.Nanosecond = 0;
    Time.TimeZone   = 0;
    Time.Daylight   = 0;
    Time.Pad2       = 0;
    CopyMem (&DescriptorData->TimeStamp, &Time, sizeof (EFI_TIME));

    DescriptorData->AuthInfo.Hdr.dwLength         = OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData);
    DescriptorData->AuthInfo.Hdr.wRevision        = 0x0200;
    DescriptorData->AuthInfo.Hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;
    CopyGuid (&DescriptorData->AuthInfo.CertType, &gEfiCertPkcs7Guid);

    if (Payload != NULL) {
        FreePool(Payload);
    }

    *DataSize = DescriptorSize + PayloadSize;
    *Data     = NewData;
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SetBootMode (
    IN UINT8  SecureBootMode
)
{
    return gRT->SetVariable (
               EFI_CUSTOM_MODE_NAME,
               &gEfiCustomModeEnableGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
               sizeof (UINT8),
               &SecureBootMode
           );
}

STATIC
EFI_STATUS
SetVariable (
    IN EFI_SIGNATURE_LIST *PkCert,
    IN UINTN              DataSize,
    IN eKey               KeyType
)
{
    UINT32  Attr;
    EFI_STATUS   Status=EFI_SUCCESS ;
    Attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS
           | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
    if(KeyType == PK_KEY)
    {
        DEBUG ((DEBUG_INFO, "Setting PK Key\n"));
        Status = gRT->SetVariable (
                     EFI_PLATFORM_KEY_NAME,
                     &gEfiGlobalVariableGuid,
                     Attr,
                     DataSize,
                     PkCert
                 );
    }
    else if( KeyType == KEK_KEY)
    {
        DEBUG ((DEBUG_INFO, "Setting KEK Key\n"));
        Status = gRT->SetVariable (
                     EFI_KEY_EXCHANGE_KEY_NAME,
                     &gEfiGlobalVariableGuid,
                     Attr,
                     DataSize,
                     PkCert
                 );

        Status = gRT->SetVariable (
                     EFI_IMAGE_SECURITY_DATABASE,
                     &gEfiImageSecurityDatabaseGuid,
                     Attr,
                     DataSize,
                     PkCert
                 );
    }
    else
    {
        ASSERT(FALSE);
    }
    return Status;

}

STATIC
VOID
PopulateCert (
    OUT EFI_SIGNATURE_LIST  **Cert,
    IN  UINTN               DataSize,
    IN  UINT8               *Data
)
{
    EFI_SIGNATURE_DATA  *CertData = NULL;

    if( (*Cert) == NULL)
    {
        (*Cert) = (EFI_SIGNATURE_LIST*) AllocateZeroPool ( sizeof(EFI_SIGNATURE_LIST)
                  + sizeof(EFI_SIGNATURE_DATA) - 1
                  + DataSize );

        ASSERT ((*Cert) != NULL);
    }
    (*Cert)->SignatureListSize   = (UINT32) (sizeof(EFI_SIGNATURE_LIST)
                                   + sizeof(EFI_SIGNATURE_DATA) - 1
                                   + DataSize);
    (*Cert)->SignatureSize       = (UINT32) (sizeof(EFI_SIGNATURE_DATA) - 1 + DataSize);
    (*Cert)->SignatureHeaderSize = 0;
    CopyGuid (&(*Cert)->SignatureType, &gEfiCertX509Guid);


    CertData = (EFI_SIGNATURE_DATA*) ((UINTN)(*Cert) + sizeof(EFI_SIGNATURE_LIST) + (*Cert)->SignatureHeaderSize);
    ASSERT (CertData != NULL);

    CopyGuid (&CertData->SignatureOwner, &gEfiGlobalVariableGuid);
    CopyMem (&CertData->SignatureData, Data, DataSize);
}

STATIC
EFI_STATUS
RegisterCert (
    IN  UINT8   *KeyData,
    IN  UINTN   KeySize,
    IN  eKey    KeyType
)
{
    EFI_STATUS          Status;
    EFI_SIGNATURE_LIST  *Cert = NULL;

    Status = SetBootMode(CUSTOM_SECURE_BOOT_MODE);
    ASSERT_EFI_ERROR (Status);

    PopulateCert(&Cert, KeySize, KeyData);

    KeySize = Cert->SignatureListSize;

    Status = CreateTimeBasedPayload (&KeySize, (UINT8**) &Cert);
    ASSERT_EFI_ERROR (Status);

    Status = SetVariable(Cert,KeySize,KeyType);
    return Status;
}

UINTN
Str2Int (
    VOID * Str
)
{
    UINTN i, Size;
    UINT8 *Ptr = Str;

    for(i=0, Size=0; i<FILE_HDR_SIZE; i++)
    {
        Size = (Ptr[i] - '0') + (Size * 10);
    }

    return Size;
}

STATIC
VOID
RdkSetVariable (
    VOID
)
{
    UINTN      		KekCrtSize;
    UINT8       	*KekCrtData;
    CONST CHAR16	*KekCrtPath;
    EFI_STATUS  	Status;

    Status = GetRdkVariable(L"KEK", &KekCrtPath);
    ASSERT_EFI_ERROR (Status);

    Status = RdkReadFile (
                 KekCrtPath,
                 (VOID **)&KekCrtData,
                 &KekCrtSize
             );
    ASSERT_EFI_ERROR (Status);

    Status = gRT->SetVariable (
                 L"RdkRootCertificate",
                 &gRdkGlobalVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 KekCrtSize,
                 KekCrtData
             );
    ASSERT_EFI_ERROR(Status);

    /* Read PK and KEK keys from reserved partition */
    STATIC UINT8 *Buffer = NULL;
    UINT32 RsvdReadSize = PcdGet32 (PcdRdkRsvdReadSize);
    Buffer = AllocateZeroPool(RsvdReadSize);
    PartitionRead((CHAR8 *)FixedPcdGetPtr (PcdRdkRsvdPartitionName), Buffer, RsvdReadSize);

    INT32 PkKeySize, KekKeySize;

    UINT8 * PkKey = Buffer;
    PkKeySize = Str2Int (PkKey);
    PkKey += FILE_HDR_SIZE;

    UINT8 * KekKey = PkKey + PkKeySize;
    KekKeySize = Str2Int (KekKey);
    KekKey += FILE_HDR_SIZE;

    INT8* SetupMode = NULL;
    eKey KeyType;

    KeyType = PK_KEY;
    Status = RegisterCert(PkKey,PkKeySize,KeyType);
    GetEfiGlobalVariable2 (L"SetupMode", (VOID**)&SetupMode, NULL);

    if (*SetupMode == 0)
    {
        DEBUG ((DEBUG_INFO, "PK Key Got Registered. Now System in User Mode\n"));
        KeyType = KEK_KEY;
        Status = RegisterCert(KekKey,KekKeySize,KeyType);
    }
    else if(*SetupMode == 1)
    {
        DEBUG ((DEBUG_INFO, "System in Standard System Mode ::: Secure Boot Not enabled\n"));
        ASSERT_EFI_ERROR(Status);
    }

    FreePool(Buffer);
}

EFI_STATUS
RdkReadFile (
    IN      CONST CHAR16  *Path,
    IN OUT  VOID          **BufferPtr,
    OUT     UINTN         *FileSize
)
{
    UINTN             BufferSize;
    UINT64            SourceFileSize;
    VOID              *Buffer;
    EFI_STATUS        Status;
    EFI_FILE_HANDLE   FileHandle;

    Status = GetFileHandler(&FileHandle, Path, EFI_FILE_MODE_READ);
    ASSERT_EFI_ERROR(Status);

    if (FileSize == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    Buffer = NULL;

    // Get the file size
    Status = FileHandle->SetPosition (FileHandle, (UINT64) -1);
    if (EFI_ERROR (Status)) {
        goto ON_EXIT;
    }

    Status = FileHandle->GetPosition (FileHandle, &SourceFileSize);
    if (EFI_ERROR (Status)) {
        goto ON_EXIT;
    }

    Status = FileHandle->SetPosition (FileHandle, 0);
    if (EFI_ERROR (Status)) {
        goto ON_EXIT;
    }

    BufferSize = (UINTN) SourceFileSize;
    Buffer =  AllocateZeroPool(BufferSize);
    if (Buffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    BufferSize = (UINTN) SourceFileSize;
    *FileSize  = BufferSize;

    Status = FileHandle->Read (FileHandle, &BufferSize, Buffer);
    if (EFI_ERROR (Status) || BufferSize != *FileSize) {
        FreePool (Buffer);
        Buffer = NULL;
        Status  = EFI_BAD_BUFFER_SIZE;
        goto ON_EXIT;
    }

ON_EXIT:

    *BufferPtr = Buffer;
    return Status;
}

EFI_STATUS
RdkWriteFile (
    IN      CONST CHAR16    *Path,
    IN OUT  VOID            **BufferPtr,
    OUT     UINTN           *FileSize
)
{
    EFI_STATUS        Status;
    EFI_FILE_HANDLE   FileHandle;

    if (FileSize == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    Status = GetFileHandler(&FileHandle, Path, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE);
    ASSERT_EFI_ERROR(Status);

    Status = FileHandle->Write (FileHandle, FileSize, *BufferPtr);
    ASSERT_EFI_ERROR (Status);

    return Status;
}

EFI_STATUS
RdkSecureBoot (
    EFI_HANDLE        ImageHandle,
    EFI_BOOT_SERVICES *BootServices
)
{
    UINT8                               *FdtData;
    UINTN                               FdtDataSize;
    UINTN                               *ExitDataSize;
    CHAR16                              **ExitData;
    CHAR16                        	    LoadOption[128];
    CONST CHAR8				                  *CmdLine;
    CONST CHAR16			                  *ImagePath;
    CONST CHAR16			                  *DtbPath;
    EFI_STATUS                          Status;
    EFI_HANDLE                          Handle;
    EFI_DEVICE_PATH_PROTOCOL            *FilePath;
    EFI_LOADED_IMAGE_PROTOCOL           *ImageInfo;
    EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL  *DevicePathFromTextProtocol;

    FilePath      = NULL;
    ExitData      = NULL;
    ExitDataSize  = NULL;
    CmdLine	      = (CONST CHAR8 *)FixedPcdGetPtr (PcdRdkCmdLineArgs);

    Status = GetRdkVariable(L"DTB", &DtbPath); 
    ASSERT_EFI_ERROR (Status);

    Status = RdkReadFile (DtbPath, (VOID**) &FdtData, &FdtDataSize);
    ASSERT_EFI_ERROR (Status);

    Status = gBS->InstallConfigurationTable (&gFdtTableGuid,(VOID*)FdtData);
    ASSERT_EFI_ERROR (Status);

    RdkSetVariable();

    Status = GetRdkVariable(L"IMAGE", &ImagePath); 
    ASSERT_EFI_ERROR (Status);

    Status = gBS->LocateProtocol (
                 &gEfiDevicePathFromTextProtocolGuid,
                 NULL,
                 (VOID**)&DevicePathFromTextProtocol
             );
    ASSERT_EFI_ERROR(Status);

    FilePath  = DevicePathFromTextProtocol->ConvertTextToDevicePath(ImagePath);

    Status    = BootServices->LoadImage (
                    TRUE,
                    ImageHandle,
                    FilePath,
                    NULL,
                    0,
                    &Handle
                );
    ASSERT_EFI_ERROR (Status);

    UnicodeSPrintAsciiFormat (LoadOption, sizeof(LoadOption), CmdLine);

    Status = BootServices->HandleProtocol (Handle, &gEfiLoadedImageProtocolGuid, (VOID **) &ImageInfo);
    ASSERT_EFI_ERROR (Status);
    ImageInfo->LoadOptionsSize  = sizeof(LoadOption);
    ImageInfo->LoadOptions      = LoadOption;

    Status = BootServices->StartImage (Handle, ExitDataSize, ExitData);
    ASSERT_EFI_ERROR (Status);

    return Status;
}
