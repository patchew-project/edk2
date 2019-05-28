##
# UefiBuild Plugin that supports Window Capsule files based on the
# Windows Firmware Update Platform spec.
# Creates INF, Cat, and then signs it
#
#
# Copyright (c) 2018, Microsoft Corporation
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
##

import sys
import re
import datetime
import os
import logging
from CatGenerator import CatGenerator
from InfGenerator import InfGenerator
from CatGenerator import RunCmd

def CatalogSignWithSignTool(SignToolPath, ToSignFilePath, PfxFilePath, PfxPass=None):
    # check signtool path
    if not os.path.exists(SignToolPath):
        logging.error("Path to signtool invalid.  %s" % SignToolPath)
        return -1

    # Adjust for spaces in the path (when calling the command).
    if " " in SignToolPath:
        SignToolPath = '"' + SignToolPath + '"'

    OutputDir = os.path.dirname(ToSignFilePath)
    # Signtool docs https://docs.microsoft.com/en-us/dotnet/framework/tools/signtool-exe
    # todo: link to catalog signing documentation
    params = "sign /a /fd SHA256 /f " + PfxFilePath
    if PfxPass is not None:
        # add password if set
        params = params + ' /p ' + PfxPass
    params = params + ' /debug /v "' + ToSignFilePath + '" '
    ret = RunCmd(SignToolPath, params, workingdir=OutputDir)
    if(ret != 0):
        logging.error("Signtool failed %d" % ret)
    return ret

class WindowsCapsuleSupportHelper(object):
    @staticmethod
    def _LocateLatestWindowsKits():
        result = None
        
        # Start with a base path and use it to start locating the ideal directory.
        base_path = os.path.join(os.getenv("ProgramFiles(x86)"), "Windows Kits")
        
        # Check for Win 10 kits first.
        base_10_path = os.path.join(base_path, "10", "bin")
        if os.path.isdir(base_10_path):
            # If you can find one of the new kit paths, use it.
            # Walk backwards to test the most recent kit first.
            for sub_path in reversed(os.listdir(base_10_path)):
                if sub_path.startswith("10.") and os.path.isdir(os.path.join(base_10_path, sub_path, "x64")):
                    result = os.path.join(base_10_path, sub_path, "x64")
                    break

            # Otherwise, fall back to the legacy path.
            if not result and os.path.isdir(os.path.join(base_10_path, "x64")):
                result = os.path.join(base_10_path, "x64")
        # If not, fall back to Win 8.1.
        elif os.path.isdir(os.path.join(base_path, "8.1", "bin", "x64")):
            result = os.path.join(base_path, "8.1", "bin", "x64")
        return result

    def RegisterHelpers(self, obj):
        fp = os.path.abspath(__file__)
        obj.Register("PackageWindowsCapsuleFiles", WindowsCapsuleSupportHelper.PackageWindowsCapsuleFiles, fp)


    @staticmethod
    def PackageWindowsCapsuleFiles(OutputFolder, ProductName, ProductFmpGuid, CapsuleVersion_DotString,CapsuleVersion_HexString, ProductFwProvider, ProductFwMfgName, ProductFwDesc, CapsuleFileName, PfxFile=None, PfxPass=None, Rollback=False, Arch='amd64', OperatingSystem_String='Win10'):
        logging.debug("CapsulePackage: Create Windows Capsule Files")
        #Make INF
        InfFilePath = os.path.join(OutputFolder, ProductName + ".inf")
        InfTool = InfGenerator(ProductName, ProductFwProvider, ProductFmpGuid, Arch, ProductFwDesc, CapsuleVersion_DotString, CapsuleVersion_HexString)
        InfTool.Manufacturer = ProductFwMfgName #optional
        ret = InfTool.MakeInf(InfFilePath, CapsuleFileName, Rollback)
        if(ret != 0):
            raise Exception("CreateWindowsInf Failed with errorcode %d" % ret)
        #Make CAT
        CatFilePath = os.path.realpath(os.path.join(OutputFolder, ProductName + ".cat"))
        CatTool = CatGenerator(Arch, OperatingSystem_String)
        ret = CatTool.MakeCat(CatFilePath)
        
        if(ret != 0):
            raise Exception("Creating Cat file Failed with errorcode %d" % ret)
        if(PfxFile is not None):
            #Find Signtool
            WinKitsPath = WindowsCapsuleSupportHelper._LocateLatestWindowsKits()
            SignToolPath = os.path.join(WinKitsPath, "signtool.exe")
            if not os.path.exists(SignToolPath):
                raise Exception("Can't find signtool on this machine.")
            #dev sign the cat file
            ret = CatalogSignWithSignTool(SignToolPath, CatFilePath, PfxFile, PfxPass)
            if(ret != 0):
                raise Exception("Signing Cat file Failed with errorcode %d" % ret)
        return ret
