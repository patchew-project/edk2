/** @file

  EBC-specific functions for unit-testing INTN and UINTN functions in
  SafeIntLib.

  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "SafeIntLibUnitTests.h"

#define _EBC_UNIT_TEST(CaseName)              \
UNIT_TEST_STATUS                              \
EFIAPI                                        \
CaseName##_32 (                               \
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,   \
  IN UNIT_TEST_CONTEXT           Context      \
  );                                          \
                                              \
UNIT_TEST_STATUS                              \
EFIAPI                                        \
CaseName##_64 (                               \
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,   \
  IN UNIT_TEST_CONTEXT           Context      \
  );                                          \
                                              \
UNIT_TEST_STATUS                              \
EFIAPI                                        \
CaseName (                                    \
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,   \
  IN UNIT_TEST_CONTEXT           Context      \
  )                                           \
{                                             \
  if (sizeof (UINTN) == sizeof (UINT32)) {    \
    return CaseName##_32(Framework, Context); \
  } else {                                    \
    return CaseName##_64(Framework, Context); \
  }                                           \
}

_EBC_UNIT_TEST (TestSafeInt32ToUintn)
_EBC_UNIT_TEST (TestSafeUint32ToIntn)
_EBC_UNIT_TEST (TestSafeIntnToInt32)
_EBC_UNIT_TEST (TestSafeIntnToUint32)
_EBC_UNIT_TEST (TestSafeUintnToUint32)
_EBC_UNIT_TEST (TestSafeUintnToIntn)
_EBC_UNIT_TEST (TestSafeUintnToInt64)
_EBC_UNIT_TEST (TestSafeInt64ToIntn)
_EBC_UNIT_TEST (TestSafeInt64ToUintn)
_EBC_UNIT_TEST (TestSafeUint64ToIntn)
_EBC_UNIT_TEST (TestSafeUint64ToUintn)
_EBC_UNIT_TEST (TestSafeUintnAdd)
_EBC_UNIT_TEST (TestSafeIntnAdd)
_EBC_UNIT_TEST (TestSafeUintnSub)
_EBC_UNIT_TEST (TestSafeIntnSub)
_EBC_UNIT_TEST (TestSafeUintnMult)
_EBC_UNIT_TEST (TestSafeIntnMult)
