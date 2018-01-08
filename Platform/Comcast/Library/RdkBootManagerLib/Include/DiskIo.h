#ifndef _RDK_DISK_IO_H_
#define _RDK_DISK_IO_H_

extern
EFI_STATUS
PartitionRead (
	IN CHAR8  *PartitionName,
	IN VOID   *Image,
	IN UINTN  Size
	);

extern
EFI_STATUS
PartitionWrite (
	IN CHAR8  *PartitionName,
	IN VOID   *Image,
	IN UINTN  Size
	);

#endif /* _RDK_DISK_IO_H_ */
