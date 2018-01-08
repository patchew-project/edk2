#ifndef __RDK_FILE_H__
#define __RDK_FILE_H__

#include "List.h"

#define ALLOCATE_STRING_MEM(X)  AllocateZeroPool((X + 1) * sizeof(CHAR16))
#define MAX_VAR                 6

typedef struct {
    CHAR16  *Name;
    LIST_ENTRY List;
} DIR_NODE;

extern EFI_STATUS
GetRdkVariable (
  IN  CONST CHAR16  *Name,
  OUT CONST CHAR16  **Value
  );

#endif /* __RDK_FILE_H__ */
