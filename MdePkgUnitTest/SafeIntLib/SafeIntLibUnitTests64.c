/** @file
  64-bit specific functions for unit-testing INTN and UINTN functions in
  SafeIntLib.

  Copyright (c) 2017, Microsoft Corporation

  All rights reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  1. Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

#include "SafeIntLibUnitTests.h"

UNIT_TEST_STATUS
EFIAPI
TestSafeInt32ToUintn_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  INT32       Operand;
  UINTN       Result;

  //
  // If Operand is non-negative, then it's a cast
  //
  Result = 0;
  Operand = 0x5bababab;
  Status = SafeInt32ToUintn (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x5bababab, Result);

  //
  // Otherwise should result in an error status
  //
  Operand = (-1537977259);
  Status = SafeInt32ToUintn (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (UINTN_ERROR, Result);

  Status = SafeInt32ToUintn (Operand, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUint32ToIntn_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINT32      Operand;
  INTN        Result;

  //
  // For x64, INTN is same as INT64 which is a superset of INT32
  // This is just a cast then, and it'll never fail
  //

  //
  // If Operand is non-negative, then it's a cast
  //
  Result = 0;
  Operand = 0xabababab;
  Status = SafeUint32ToIntn (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0xabababab, Result);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeIntnToInt32_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  INTN        Operand;
  INT32       Result;

  //
  // If Operand is between MIN_INT32 and  MAX_INT32 inclusive, then it's a cast
  //
  Result = 0;
  Operand = 0x5bababab;
  Status = SafeIntnToInt32 (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x5bababab, Result);

  Operand = (-1537977259);
  Status = SafeIntnToInt32 (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL ((-1537977259), Result);

  //
  // Otherwise should result in an error status
  //
  Operand = (0x5babababefefefef);
  Status = SafeIntnToInt32 (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INT32_ERROR, Result);

  Operand =  (-6605562033422200815);
  Status = SafeIntnToInt32 (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INT32_ERROR, Result);

  Status = SafeIntnToInt32 (Operand, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeIntnToUint32_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  INTN        Operand;
  UINT32      Result;

  //
  // If Operand is between 0 and  MAX_UINT32 inclusive, then it's a cast
  //
  Result = 0;
  Operand = 0xabababab;
  Status = SafeIntnToUint32 (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0xabababab, Result);

  //
  // Otherwise should result in an error status
  //
  Operand = (0x5babababefefefef);
  Status = SafeIntnToUint32 (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (UINT32_ERROR, Result);

  Operand =  (-6605562033422200815);
  Status = SafeIntnToUint32 (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (UINT32_ERROR, Result);

  Status = SafeIntnToUint32 (Operand, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUintnToUint32_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINTN       Operand;
  UINT32      Result;

  //
  // If Operand is <= MAX_UINT32, then it's a cast
  //
  Result = 0;
  Operand = 0xabababab;
  Status = SafeUintnToUint32 (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0xabababab, Result);

  //
  // Otherwise should result in an error status
  //
  Operand = (0xababababefefefef);
  Status = SafeUintnToUint32 (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (UINT32_ERROR, Result);

  Status = SafeUintnToUint32 (Operand, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUintnToIntn_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINTN       Operand;
  INTN        Result;

  //
  // If Operand is <= MAX_INTN (0x7fff_ffff_ffff_ffff), then it's a cast
  //
  Result = 0;
  Operand = 0x5babababefefefef;
  Status = SafeUintnToIntn (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x5babababefefefef, Result);

  //
  // Otherwise should result in an error status
  //
  Operand = (0xababababefefefef);
  Status = SafeUintnToIntn (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INTN_ERROR, Result);

  Status = SafeUintnToIntn (Operand, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUintnToInt64_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINTN       Operand;
  INT64       Result;

  //
  // If Operand is <= MAX_INT64, then it's a cast
  //
  Result = 0;
  Operand = 0x5babababefefefef;
  Status = SafeUintnToInt64 (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x5babababefefefef, Result);

  //
  // Otherwise should result in an error status
  //
  Operand = (0xababababefefefef);
  Status = SafeUintnToInt64 (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INT64_ERROR, Result);

  Status = SafeUintnToInt64 (Operand, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeInt64ToIntn_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  INT64       Operand;
  INTN        Result;

  //
  // INTN is same as INT64 in x64, so this is just a cast
  //
  Result = 0;
  Operand = 0x5babababefefefef;
  Status = SafeInt64ToIntn (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x5babababefefefef, Result);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeInt64ToUintn_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  INT64       Operand;
  UINTN       Result;

  //
  // If Operand is non-negative, then it's a cast
  //
  Result = 0;
  Operand = 0x5babababefefefef;
  Status = SafeInt64ToUintn (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x5babababefefefef, Result);

  //
  // Otherwise should result in an error status
  //
  Operand =  (-6605562033422200815);
  Status = SafeInt64ToUintn (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (UINTN_ERROR, Result);

  Status = SafeInt64ToUintn (Operand, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUint64ToIntn_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINT64      Operand;
  INTN        Result;

  //
  // If Operand is <= MAX_INTN (0x7fff_ffff_ffff_ffff), then it's a cast
  //
  Result = 0;
  Operand = 0x5babababefefefef;
  Status = SafeUint64ToIntn (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x5babababefefefef, Result);

  //
  // Otherwise should result in an error status
  //
  Operand = (0xababababefefefef);
  Status = SafeUint64ToIntn (Operand, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INTN_ERROR, Result);

  Status = SafeUint64ToIntn (Operand, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUint64ToUintn_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINT64      Operand;
  UINTN       Result;

  //
  // UINTN is same as UINT64 in x64, so this is just a cast
  //
  Result = 0;
  Operand = 0xababababefefefef;
  Status = SafeUint64ToUintn (Operand, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0xababababefefefef, Result);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUintnAdd_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINTN       Augend;
  UINTN       Addend;
  UINTN       Result;

  //
  // If the result of addition doesn't overflow MAX_UINTN, then it's addition
  //
  Result = 0;
  Augend = 0x3a3a3a3a12121212;
  Addend = 0x3a3a3a3a12121212;
  Status = SafeUintnAdd (Augend, Addend, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x7474747424242424, Result);

  //
  // Otherwise should result in an error status
  //
  Augend = 0xababababefefefef;
  Addend = 0xbcbcbcbcdededede;
  Status = SafeUintnAdd (Augend, Addend, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (UINTN_ERROR, Result);

  Status = SafeUintnAdd (Augend, Addend, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeIntnAdd_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  INTN        Augend;
  INTN        Addend;
  INTN        Result;

  //
  // If the result of addition doesn't overflow MAX_INTN
  // and doesn't underflow MIN_INTN, then it's addition
  //
  Result = 0;
  Augend = 0x3a3a3a3a3a3a3a3a;
  Addend = 0x3a3a3a3a3a3a3a3a;
  Status = SafeIntnAdd (Augend, Addend, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x7474747474747474, Result);

  Augend = (-4195730024608447034);
  Addend = (-4195730024608447034);
  Status = SafeIntnAdd (Augend, Addend, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL ((-8391460049216894068), Result);

  //
  // Otherwise should result in an error status
  //
  Augend = 0x5a5a5a5a5a5a5a5a;
  Addend = 0x5a5a5a5a5a5a5a5a;
  Status = SafeIntnAdd (Augend, Addend, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INTN_ERROR, Result);

  Augend = (-6510615555426900570);
  Addend = (-6510615555426900570);
  Status = SafeIntnAdd (Augend, Addend, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INTN_ERROR, Result);

  Status = SafeIntnAdd (Augend, Addend, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUintnSub_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINTN       Minuend;
  UINTN       Subtrahend;
  UINTN       Result;

  //
  // If Minuend >= Subtrahend, then it's subtraction
  //
  Result = 0;
  Minuend = 0x5a5a5a5a5a5a5a5a;
  Subtrahend = 0x3b3b3b3b3b3b3b3b;
  Status = SafeUintnSub (Minuend, Subtrahend, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x1f1f1f1f1f1f1f1f, Result);

  //
  // Otherwise should result in an error status
  //
  Minuend = 0x5a5a5a5a5a5a5a5a;
  Subtrahend = 0x6d6d6d6d6d6d6d6d;
  Status = SafeUintnSub (Minuend, Subtrahend, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (UINTN_ERROR, Result);

  Status = SafeUintnSub (Minuend, Subtrahend, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeIntnSub_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  INTN        Minuend;
  INTN        Subtrahend;
  INTN        Result;

  //
  // If the result of subtractions doesn't overflow MAX_INTN or
  // underflow MIN_INTN, then it's subtraction
  //
  Minuend = 0x5a5a5a5a5a5a5a5a;
  Subtrahend = 0x3a3a3a3a3a3a3a3a;
  Result = 0;
  Status = SafeIntnSub (Minuend, Subtrahend, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x2020202020202020, Result);

  Minuend = 0x3a3a3a3a3a3a3a3a;
  Subtrahend = 0x5a5a5a5a5a5a5a5a;
  Status = SafeIntnSub (Minuend, Subtrahend, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL ((-2314885530818453536), Result);

  //
  // Otherwise should result in an error status
  //
  Minuend = (-8825501086245354106);
  Subtrahend = 8825501086245354106;
  Status = SafeIntnSub (Minuend, Subtrahend, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INTN_ERROR, Result);

  Minuend = (8825501086245354106);
  Subtrahend = (-8825501086245354106);
  Status = SafeIntnSub (Minuend, Subtrahend, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INTN_ERROR, Result);

  Status = SafeIntnSub (Minuend, Subtrahend, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeUintnMult_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  UINTN       Multiplicand;
  UINTN       Multiplier;
  UINTN       Result;

  //
  // If the result of multiplication doesn't overflow MAX_UINTN, it will succeed
  //
  Result = 0;
  Multiplicand = 0x123456789a;
  Multiplier = 0x1234567;
  Status = SafeUintnMult (Multiplicand, Multiplier, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x14b66db9745a07f6, Result);

  //
  // Otherwise should result in an error status
  //
  Multiplicand = 0x123456789a;
  Multiplier = 0x12345678;
  Status = SafeUintnMult (Multiplicand, Multiplier, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (UINTN_ERROR, Result);

  Status = SafeUintnMult (Multiplicand, Multiplier, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}

UNIT_TEST_STATUS
EFIAPI
TestSafeIntnMult_64 (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN UNIT_TEST_CONTEXT           Context
  )
{
  EFI_STATUS  Status;
  INTN        Multiplicand;
  INTN        Multiplier;
  INTN        Result;

  //
  // If the result of multiplication doesn't overflow MAX_INTN and doesn't
  // underflow MIN_UINTN, it will succeed
  //
  Result = 0;
  Multiplicand = 0x123456789;
  Multiplier = 0x6789abcd;
  Status = SafeIntnMult (Multiplicand, Multiplier, &Result);
  UT_ASSERT_NOT_EFI_ERROR (Status);
  UT_ASSERT_EQUAL (0x75cd9045220d6bb5, Result);

  //
  // Otherwise should result in an error status
  //
  Multiplicand = 0x123456789;
  Multiplier = 0xa789abcd;
  Status = SafeIntnMult (Multiplicand, Multiplier, &Result);
  UT_ASSERT_EQUAL (RETURN_BUFFER_TOO_SMALL, Status);
  UT_ASSERT_EQUAL (INTN_ERROR, Result);

  Status = SafeIntnMult (Multiplicand, Multiplier, NULL);
  UT_ASSERT_EQUAL (RETURN_INVALID_PARAMETER, Status);

  return UNIT_TEST_PASSED;
}
