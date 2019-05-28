## @file
 # Script to generate Cat files for capsule update based on supplied inf file
 #
 # Copyright (c) 2019, Microsoft Corporation
 # Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
 # SPDX-License-Identifier: BSD-2-Clause-Patent
 #
 ##

import os
import logging
import datetime
import subprocess
import threading

class PropagatingThread(threading.Thread):
    def run(self):
        self.exc = None
        try:
            if hasattr(self, '_Thread__target'):
                # Thread uses name mangling prior to Python 3.
                self.ret = self._Thread__target(*self._Thread__args, **self._Thread__kwargs)
            else:
                self.ret = self._target(*self._args, **self._kwargs)
        except BaseException as e:
            self.exc = e
    def join(self, timeout=None):
        super(PropagatingThread, self).join()
        if self.exc:
             raise self.exc
        return self.ret
def reader(filepath, outstream, stream):
    f = None
    # open file if caller provided path
    if(filepath):
        f = open(filepath, "w")
    while True:
        s = stream.readline().decode()
        if not s:
            stream.close()
            break
        if(f is not None):
            # write to file if caller provided file
            f.write(s)
        if(outstream is not None):
            # write to stream object if caller provided object
            outstream.write(s)
        logging.info(s.rstrip())
    if(f is not None):
        f.close()
def RunCmd(cmd, parameters, capture=True, workingdir=None, outfile=None, outstream=None, environ=None):
    cmd = cmd.strip('"\'')
    if " " in cmd:
        cmd = '"' + cmd + '"'
    if parameters is not None:
        parameters = parameters.strip()
        cmd += " " + parameters
    starttime = datetime.datetime.now()
    logging.info("Cmd to run is: " + cmd) 
    logging.info("------------------------------------------------")
    logging.info("--------------Cmd Output Starting---------------")
    logging.info("------------------------------------------------")
    c = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=workingdir, shell=True, env=environ)
    if(capture):
        outr = PropagatingThread(target=reader, args=(outfile, outstream, c.stdout,))
        outr.start()
        outr.join()
        c.wait()
    else:
        c.wait()
  
    endtime = datetime.datetime.now()
    delta = endtime - starttime
    logging.info("------------------------------------------------")
    logging.info("--------------Cmd Output Finished---------------")
    logging.info("--------- Running Time (mm:ss): {0[0]:02}:{0[1]:02} ----------".format(divmod(delta.seconds, 60)))
    logging.info("------------------------------------------------")
    return c.returncode

class CatGenerator(object):
    SUPPORTED_OS = {'win10': '10',
                    '10': '10',
                    '10_au': '10_AU',
                    '10_rs2': '10_RS2',
                    '10_rs3': '10_RS3',
                    '10_rs4': '10_RS4',
                    'server10': 'Server10',
                    'server2016': 'Server2016',
                    'serverrs2': 'ServerRS2',
                    'serverrs3': 'ServerRS3',
                    'serverrs4': 'ServerRS4'
                    }

    def __init__(self, arch, os):
        self.Arch = arch
        self.OperatingSystem = os

    @property
    def Arch(self):
        return self._arch

    @Arch.setter
    def Arch(self, value):
        value = value.lower()
        if(value == "x64") or (value == "amd64"):  # support amd64 value so INF and CAT tools can use same arch value
            self._arch = "X64"
        elif(value == "arm"):
            self._arch = "ARM"
        elif(value == "arm64") or (value == "aarch64"):  # support UEFI defined aarch64 value as well
            self._arch = "ARM64"
        else:
            logging.critical("Unsupported Architecture: %s", value)
            raise ValueError("Unsupported Architecture")

    @property
    def OperatingSystem(self):
        return self._operatingsystem

    @OperatingSystem.setter
    def OperatingSystem(self, value):
        key = value.lower()
        if(key not in CatGenerator.SUPPORTED_OS.keys()):
            logging.critical("Unsupported Operating System: %s", key)
            raise ValueError("Unsupported Operating System")
        self._operatingsystem = CatGenerator.SUPPORTED_OS[key]

    def MakeCat(self, OutputCatFile, PathToInf2CatTool=None):
        # Find Inf2Cat tool
        if(PathToInf2CatTool is None):
            PathToInf2CatTool = os.path.join(os.getenv("ProgramFiles(x86)"), "Windows Kits", "10",
                                             "bin", "x86", "Inf2Cat.exe")
            if not os.path.exists(PathToInf2CatTool):
                logging.debug("Windows Kit 10 not Found....trying 8.1")
                # Try 8.1 kit
                PathToInf2CatTool.replace("10", "8.1")

        # check if exists
        if not os.path.exists(PathToInf2CatTool):
            raise Exception("Can't find Inf2Cat on this machine.  Please install the Windows 10 WDK - "
                            "https://developer.microsoft.com/en-us/windows/hardware/windows-driver-kit")

        # Adjust for spaces in the path (when calling the command).
        if " " in PathToInf2CatTool:
            PathToInf2CatTool = '"' + PathToInf2CatTool + '"'

        OutputFolder = os.path.dirname(OutputCatFile)
        # Make Cat file
        cmd = "/driver:. /os:" + self.OperatingSystem + "_" + self.Arch + " /verbose"
        ret = RunCmd(PathToInf2CatTool, cmd, workingdir=OutputFolder)
        if(ret != 0):
            raise Exception("Creating Cat file Failed with errorcode %d" % ret)
        if(not os.path.isfile(OutputCatFile)):
            raise Exception("CAT file (%s) not created" % OutputCatFile)

        return 0
