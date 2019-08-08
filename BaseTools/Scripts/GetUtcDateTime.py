## @file
#  Get current UTC date and time information and output as ascii code.
#
#  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#

VersionNumber = '0.1'
import sys
import datetime

def Usage():
    print ("GetUtcDateTime - Version " + VersionNumber)
    print ("Usage:")
    print ("GetUtcDateTime [type]")
    print ("  --year:            Return UTC year of now")
    print ("                     Example output (2019): 39313032")
    print ("  --date:            Return UTC date MMDD of now")
    print ("                     Example output (7th August): 37303830")
    print ("  --time:            Return 24-hour-format UTC time HHMM of now")
    print ("                     Example output (4:25): 35323430")

def Main():
  if len(sys.argv) == 1:
    Usage()
    return 0

  today = datetime.datetime.utcnow()
  if sys.argv[1].strip().lower() == "--year":
    ReversedNumber = str(today.year)[::-1]
    print (''.join(hex(ord(HexString))[2:] for HexString in ReversedNumber))
    return 0
  if sys.argv[1].strip().lower() == "--date":
    ReversedNumber = str(today.strftime("%m%d"))[::-1]
    print (''.join(hex(ord(HexString))[2:] for HexString in ReversedNumber))
    return 0
  if sys.argv[1].strip().lower() == "--time":
    ReversedNumber = str(today.strftime("%H%M"))[::-1]
    print (''.join(hex(ord(HexString))[2:] for HexString in ReversedNumber))
    return 0
  else:
    Usage()
    return 0

if __name__ == '__main__':
    sys.exit(Main())
