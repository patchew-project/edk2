/** @file
  BaseRng Library that uses the TimerLib to provide reasonably random numbers.
  Do not use this on a production system.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>

/**
 * Using the TimerLib GetPerformanceCounterProperties() we delay
 * for enough time for the PerformanceCounter to increment.
 * Depending on your system
 *
 * If the return value from GetPerformanceCounterProperties (TimerLib)
 * is zero, this function will not delay and attempt to assert.
 */
STATIC
VOID
DecentDelay (
  VOID
  )
{
  UINT64 StartValue;
  UINT64 EndValue;
  UINT64 CounterHz;
  UINT64 MinumumDelayInMicroSeconds;

  // Get the counter properties
  CounterHz = GetPerformanceCounterProperties (&StartValue, &EndValue);
  // Make sure we won't divide by zero
  if (CounterHz == 0) {
    ASSERT(CounterHz != 0); // Assert so the developer knows something is wrong
    return;
  }
  // Calculate the minimum delay based on 1.5 microseconds divided by the hertz.
  // We calculate the length of a cycle (1/CounterHz) and multiply it by 1.5 microseconds
  // This ensures that the performance counter has increased by at least one
  MinumumDelayInMicroSeconds = 1500000 / CounterHz;

  MicroSecondDelay (MinumumDelayInMicroSeconds);
}


/**
  Generates a 16-bit random number.

  if Rand is NULL, then ASSERT().

  @param[out] Rand     Buffer pointer to store the 16-bit random value.

  @retval TRUE         Random number generated successfully.
  @retval FALSE        Failed to generate the random number.

**/
BOOLEAN
EFIAPI
GetRandomNumber16 (
  OUT     UINT16                    *Rand
  )
{
  UINT32  Index;
  UINT8  *RandPtr;

  ASSERT (Rand != NULL);

  if (Rand == NULL) {
    return FALSE;
  }

  RandPtr = (UINT8*)Rand;
  // Get 2 bytes of random ish data
  // This should take around 10us
  for (Index = 0; Index < 2; Index ++) {
    *RandPtr = (UINT8)(GetPerformanceCounter () & 0xFF);
    DecentDelay (); // delay to give chance for performance counter to catch up
    RandPtr++;
  }
  return TRUE;
}

/**
  Generates a 32-bit random number.

  if Rand is NULL, then ASSERT().

  @param[out] Rand     Buffer pointer to store the 32-bit random value.

  @retval TRUE         Random number generated successfully.
  @retval FALSE        Failed to generate the random number.

**/
BOOLEAN
EFIAPI
GetRandomNumber32 (
  OUT     UINT32                    *Rand
  )
{
  UINT32  Index;
  UINT8* RandPtr;

  ASSERT (Rand != NULL);

  if (NULL == Rand) {
    return FALSE;
  }

  RandPtr = (UINT8 *) Rand;
  // Get 4 bytes of random ish data
  // This should take around 20ms
  for (Index = 0; Index < 4; Index ++) {
    *RandPtr = (UINT8) (GetPerformanceCounter () & 0xFF);
    DecentDelay (); // delay to give chance for performance counter to catch up
    RandPtr++;
  }
  return TRUE;
}

/**
  Generates a 64-bit random number.

  if Rand is NULL, then ASSERT().

  @param[out] Rand     Buffer pointer to store the 64-bit random value.

  @retval TRUE         Random number generated successfully.
  @retval FALSE        Failed to generate the random number.

**/
BOOLEAN
EFIAPI
GetRandomNumber64 (
  OUT     UINT64                    *Rand
  )
{
  UINT32  Index;
  UINT8* RandPtr;

  ASSERT (Rand != NULL);

  if (NULL == Rand) {
    return FALSE;
  }

  RandPtr = (UINT8 *) Rand;
  // Get 8 bytes of random ish data
  // This should take around 40ms
  for (Index = 0; Index < 8; Index ++) {
    *RandPtr = (UINT8) (GetPerformanceCounter () & 0xFF);
    DecentDelay (); // delay to give chance for performance counter to catch up
    RandPtr++;
  }

  return TRUE;
}

/**
  Generates a 128-bit random number.

  if Rand is NULL, then ASSERT().

  @param[out] Rand     Buffer pointer to store the 128-bit random value.

  @retval TRUE         Random number generated successfully.
  @retval FALSE        Failed to generate the random number.

**/
BOOLEAN
EFIAPI
GetRandomNumber128 (
  OUT     UINT64                    *Rand
  )
{
  ASSERT (Rand != NULL);
  // This should take around 80ms

  // Read first 64 bits
  if (!GetRandomNumber64 (Rand)) {
    return FALSE;
  }

  // Read second 64 bits
  return GetRandomNumber64 (++Rand);
}
