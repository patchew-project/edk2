/** @file
Provides a unit test framework logging.  This allows tests to focus on testing logic
and the library to handle unit test specific logging.

Copyright (c) 2016, Microsoft Corporation

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

#ifndef __UNIT_TEST_LOG_LIB_H__
#define __UNIT_TEST_LOG_LIB_H__



///================================================================================================
///================================================================================================
///
/// UNIT TEST LOGGING DEFINITIONS AND FUNCTIONS
///
///================================================================================================
///================================================================================================

VOID
EFIAPI
UnitTestLog (
  IN  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  IN  UINTN                       ErrorLevel,
  IN  CONST CHAR8                 *Format,
  ...
  );

VOID
EFIAPI
UnitTestLogInit (
IN OUT UNIT_TEST  *Test,
IN UINT8      *Buffer OPTIONAL,
IN UINTN      BufferSize
);

VOID
EFIAPI
UnitTestLogFailure(
  IN  UNIT_TEST_FRAMEWORK_HANDLE  Framework,
  FAILURE_TYPE FailureType,
  IN  CONST CHAR8                 *Format,
  ...
);

#endif
