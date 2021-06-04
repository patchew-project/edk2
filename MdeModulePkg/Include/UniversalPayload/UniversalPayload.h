/** @file
  Universal Payload general definations.

Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __UNIVERSAL_PAYLOAD_H__
#define __UNIVERSAL_PAYLOAD_H__

#include <Library/HobLib.h>

#pragma pack(1)

typedef struct {
  UINT8                Revision;
  UINT8                Reserved;
  UINT16               Length;
} PLD_GENERIC_HEADER;

#pragma pack()

/**
  Returns the size of a structure of known type, up through and including a specified field.

  @param   TYPE     The name of the data structure that contains the field specified by Field.
  @param   Field    The name of the field in the data structure.

  @return  size, in bytes.

**/
#define PLD_SIZEOF_THROUGH_FIELD(TYPE, Field) (OFFSET_OF(TYPE, Field) + sizeof (((TYPE *) 0)->Field))

#define IS_PLD_HEADER_HAS_REVISION(GuidHob, ExpectedRevision) \
  ( \
    (GuidHob != NULL) && \
    (sizeof (PLD_GENERIC_HEADER) <= GET_GUID_HOB_DATA_SIZE (GuidHob)) && \
    (((PLD_GENERIC_HEADER *) GET_GUID_HOB_DATA (GuidHob))->Length <= GET_GUID_HOB_DATA_SIZE (GuidHob)) && \
    (((PLD_GENERIC_HEADER *) GET_GUID_HOB_DATA (GuidHob))->Revision == ExpectedRevision) \
  )

#endif // __UNIVERSAL_PAYLOAD_H__
