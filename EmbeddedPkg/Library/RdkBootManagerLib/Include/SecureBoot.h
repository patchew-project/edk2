#ifndef _RDK_SECURE_BOOT_H_
#define _RDK_SECURE_BOOT_H_

#define FILE_HDR_SIZE 16

extern UINTN Str2Int (
	VOID * Str
);

extern EFI_STATUS RdkSecureBoot (
		EFI_HANDLE		ImageHandle,
		EFI_BOOT_SERVICES      *BootServices);

extern EFI_STATUS RdkReadFile (
		IN  	CONST CHAR16 			*Path,
		IN OUT  VOID                    **BufferPtr,
		OUT  	UINTN                   *FileSize
		);

extern EFI_STATUS RdkWriteFile (
		IN  	CONST CHAR16 			*Path,
		IN OUT  VOID                    **BufferPtr,
		OUT  	UINTN                   *FileSize
		);

extern EFI_STATUS GetFileHandler (
    		OUT 	EFI_FILE_HANDLE *FileHandle,
    		IN 	CONST CHAR16    *Path,
    		IN  	UINT64          OpenMode
);

typedef enum KEY
{
	PK_KEY=1,
	KEK_KEY,
	DB_KEY,
	DBX_KEY
} eKey;

#endif /* _RDK_SECURE_BOOT_H_ */
