## @file
# Trim files preprocessed by compiler
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

##
# Import Modules
#
import Common.LongFilePathOs as os
import sys
import struct
import time
import datetime
try:
    from configparser import ConfigParser
except:
    from ConfigParser import ConfigParser
from Common.BuildToolError import *
from Common.Misc import *
from Common.DataType import *
from Common.BuildVersion import gBUILD_VERSION
import Common.EdkLogger as EdkLogger
from Common.LongFilePathSupport import OpenLongFilePath as open

_BIOS_Signature = "$IBIOSI$"
_SectionKeyName = '__name__'
_SectionName = 'config'

_ConfigItem = {
    "BOARD_ID"   : {'Value' : '', 'Length' : 7},
    "BOARD_REV"  : {'Value' : '', 'Length' : 1},
    "BOARD_EXT"  : { 'Value' : '', 'Length' : 3},
    "BUILD_TYPE" : {'Value' : '', 'Length' :1},
    "VERSION_MAJOR" : {'Value' : '0000', 'Length' : 4},
    "VERSION_MINOR" : {'Value' : '00', 'Length' : 2},

}


_Usage = "Usage: GenBiosId -i Configfile -o OutputFile [-ob OutputBatchFile]"
_ConfigSectionNotDefine = "Not support the config file format, need config section"
_ConfigLenInvalid = "Config item %s length is invalid"
_ConfigItemInvalid = "Item %s is invalid"

def Main():
    try:
        EdkLogger.Initialize()
        if len(sys.argv) !=5 and len(sys.argv) != 7:
            EdkLogger.error("GenBiosId", OPTION_MISSING, ExtraData=_Usage)
    except FatalError as X:
        return 1
    InputFile = ''
    OutputFile = ''
    OutputBatchFile = ''
    for Index, Item in enumerate(sys.argv):
        if '-i' == Item:
            InputFile = sys.argv[Index + 1]
        if '-o' == Item:
            OutputFile = sys.argv[Index + 1]
        if '-ob' == Item:
            OutputBatchFile = sys.argv[Index + 1]
    if not os.path.exists(InputFile):
        EdkLogger.error("GenBiosId", FILE_NOT_FOUND, ExtraData="Input file not found")
    cf = ConfigParser()
    cf.optionxform = str
    cf.read(InputFile)
    if _SectionName not in cf._sections:
        EdkLogger.error("GenBiosId", FORMAT_NOT_SUPPORTED, ExtraData=_ConfigSectionNotDefine)
    for Item in cf._sections[_SectionName]:
        if Item == _SectionKeyName:
            continue
        if Item not in _ConfigItem:
            EdkLogger.error("GenBiosId", FORMAT_INVALID, ExtraData=_ConfigItemInvalid % Item)
        _ConfigItem[Item]['Value'] = cf._sections[_SectionName][Item]
        if len(_ConfigItem[Item]['Value']) != _ConfigItem[Item]['Length']:
            EdkLogger.error("GenBiosId", FORMAT_INVALID, ExtraData=_ConfigLenInvalid % Item)
    for Item in _ConfigItem:
        if not _ConfigItem[Item]['Value']:
            EdkLogger.error("GenBiosId", FORMAT_UNKNOWN_ERROR, ExtraData="Item %s is missing" % Item)
    utcnow = datetime.datetime.utcnow()
    TimeStamp = time.strftime("%y%m%d%H%M", utcnow.timetuple())

    Id_Str = _ConfigItem['BOARD_ID']['Value'] + _ConfigItem['BOARD_REV']['Value'] + '.' + _ConfigItem['BOARD_EXT']['Value'] + '.' + _ConfigItem['VERSION_MAJOR']['Value'] + \
             '.' + _ConfigItem["BUILD_TYPE"]['Value'] + _ConfigItem['VERSION_MINOR']['Value'] + '.' + TimeStamp
    with open(OutputFile, 'wb') as FdOut:
        for i in _BIOS_Signature:
            FdOut.write(struct.pack('B', ord(i)))

        for i in Id_Str:
            FdOut.write(struct.pack('H', ord(i)))

        FdOut.write(struct.pack('H', 0x00))
    if OutputBatchFile:
        with open(OutputBatchFile, 'w') as FdOut:
            if sys.platform.startswith('win'):
                Id_Str = 'SET BIOS_ID=' + Id_Str
            else:
                Id_Str = 'export BIOS_ID=' + Id_Str
            FdOut.write(Id_Str)
    return 0

if __name__ == '__main__':
    r = Main()
    ## 0-127 is a safe return range, and 1 is a standard default error
    if r < 0 or r > 127: r = 1
    sys.exit(r)
