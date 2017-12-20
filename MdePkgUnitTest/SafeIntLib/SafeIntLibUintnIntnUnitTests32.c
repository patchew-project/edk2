/** @file

  32-bit specific functions for unit-testing INTN and UINTN functions in
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

#define _UNIT_TEST_32_BIT(CaseName) \
UNIT_TEST_STATUS \
EFIAPI \
CaseName##_32 ( \
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework, \
  IN UNIT_TEST_CONTEXT           Context \
  ); \
\
UNIT_TEST_STATUS \
EFIAPI \
CaseName ( \
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework, \
  IN UNIT_TEST_CONTEXT           Context \
  ) \
{ \
  return CaseName##_32(Framework, Context); \
}

_UNIT_TEST_32_BIT(TestSafeInt32ToUintn)
_UNIT_TEST_32_BIT(TestSafeUint32ToIntn)
_UNIT_TEST_32_BIT(TestSafeIntnToInt32)
_UNIT_TEST_32_BIT(TestSafeIntnToUint32)
_UNIT_TEST_32_BIT(TestSafeUintnToUint32)
_UNIT_TEST_32_BIT(TestSafeUintnToIntn)
_UNIT_TEST_32_BIT(TestSafeUintnToInt64)
_UNIT_TEST_32_BIT(TestSafeInt64ToIntn)
_UNIT_TEST_32_BIT(TestSafeInt64ToUintn)
_UNIT_TEST_32_BIT(TestSafeUint64ToIntn)
_UNIT_TEST_32_BIT(TestSafeUint64ToUintn)
_UNIT_TEST_32_BIT(TestSafeUintnAdd)
_UNIT_TEST_32_BIT(TestSafeIntnAdd)
_UNIT_TEST_32_BIT(TestSafeUintnSub)
_UNIT_TEST_32_BIT(TestSafeIntnSub)
_UNIT_TEST_32_BIT(TestSafeUintnMult)
_UNIT_TEST_32_BIT(TestSafeIntnMult)
