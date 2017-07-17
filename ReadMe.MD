
This code base is designed for the MinnowBoard Max/Turbot platforms using the Intel(R) Atom(tm) Processor E3800 Series.
This code base is based on UDK2015 core packages.

# INDEX

1.  HOW TO CREATE A FULL SOURCE TREE
2.  HOW TO BUILD (WINDOWS ENVIRONMENT)
3.  MAINTAINERS

## 1. HOW TO CREATE A FULL SOURCE TREE

1. Create a new folder (directory) on the root of your local development machine
   for use as your work space (this example uses "C:\MyWorkspace").

   NOTE: Some code paths are very long, and placing the working directory too
         deep below the ROOT directory may cause the path to be longer than the
         operating system or file system's maximum path length.

2. Get MinnowBoard Max branch from GitHub with the following command.
   1. Run `git clone -b minnowboard-max-udk2015 https://github.com/tianocore/edk2-platforms.git`
   2. Enter folder edk2-platforms
   3. Run `git checkout fd6510974d21282b9f8c1ff003c05b92a464bca6`

   Once the download completes, all EDK II packages and files will be found
   under the "C:\MyWorkspace\edk2-platforms" directory.

3. Get the BaseTools binary and copy them to BaseTools binary folder.
   1. Enter folder BaseTools\Bin
   2. `git clone https://github.com/tianocore/edk2-BaseTools-win32.git Win32`
   3. Enter folder Win32
   4. `git checkout ea691aec89b06aa83474100df1de000a875b4ea0`

4. Download MinnowBoard MAX 0.96 Binary Object Modules from http://firmware.intel.com/projects/minnowboard-max.
   The "MinnowBoard_MAX-0.96-Binary.Objects.zip" file contains three additional
   folders required for the full source tree.
   
   IA32FamilyCpuPkg
   
   Vlv2BinaryPkg
   
   Vlv2MiscBinariesPkg
   
   Unzip and copy the three folders to the workspace directory (e.g. "C:\MyWorkspace\edk2-platforms").


## 2. HOW TO BUILD (WINDOWS ENVIRONMENT)

Windows System Configuration:
  Microsoft Windows 7 Ultimate 64-bit*

1. Setup Build Environment

   1. Install Visual Studio 2008/2010/2012/2013 on the build machine.

   2. Install the latest Python 2 Release from: https://www.python.org/downloads/windows/
      * Make sure that a file with the extension of ".py" will be opened by Python.exe.

2. Extract Source Code
   1. Follow the instructions of "HOW TO CREATE A FULL SOURCE TREE" (step 1 above)
      to create a full source tree.

   2. Follow the instructions found in the file "Patch-HOWTO.txt" located in your
      workspace (e.g. "C:\MyWorkspace\CryptoPkg\Library\OpensslLib\Patch-HOWTO.txt") 
      to install the Openssl source code.

3. Install the iasl compiler by downloading iasl-win-20141107.zip from the following
   location: "https://acpica.org/downloads/version-20141107" and place the unzipped
   content ("iasl.exe") into the directory "C:\ASL" on your local hard drive
   (create the folder "C:\ASL" if it does not exist).
   
4a. Build Steps (32-bit)

   1. To build a 32-bit release version
      * Open a command prompt of Microsoft Visual Studio, such as "Developer Command Prompt for VS2013";
      * Type the command: `cd C:\MyWorkspace\edk2-platforms\Vlv2TbltDevicePkg`
      * Type the command: `Build_IFWI.bat /IA32 MNW2 Release` to build a release version.

   2. To build a 32-bit debug version
      * Open a command prompt of Microsoft Visual Studio, such as "Developer Command Prompt for VS2013";
      * Type the command: `cd C:\MyWorkspace\edk2-platforms\Vlv2TbltDevicePkg`
      * Type the command: `Build_IFWI.bat /IA32 MNW2 Debug` to build a debug version.

   3. After the build successfully completes, the 8MB firmware binary image will be located in the
       following location on your local hard drive:

      "C:\MyWorkspace\edk2-platforms\Vlv2TbltDevicePkg\Stitch\"

4b. Build Steps (64-bit)

   1. To build a 64-bit release version
      * Open a command prompt of Microsoft Visual Studio, such as "Developer Command Prompt for VS2013";
      * Type the command: `cd C:\MyWorkspace\edk2-platforms\Vlv2TbltDevicePkg`
      * Type the command: `Build_IFWI.bat MNW2 Release` to build a release version.

   2. To build a 64-bit debug version
      * Open a command prompt of Microsoft Visual Studio, such as "Developer Command Prompt for VS2013";
      * Type the command: `cd C:\MyWorkspace\edk2-platforms\Vlv2TbltDevicePkg`
      * Type the command: `Build_IFWI.bat MNW2 Debug` to build a debug version.

   3. After the build successfully completes, the 8MB firmware binary image will be located in the
      following location on your local hard drive:

      "C:\MyWorkspace\edk2-platforms\Vlv2TbltDevicePkg\Stitch\"

## MAINTAINERS

* david.wei@intel.com
* mang.guo@intel.com
* vincent.zimmer@intel.com
* mike.wu@intel.com
* shifeix.a.lu@intel.com

