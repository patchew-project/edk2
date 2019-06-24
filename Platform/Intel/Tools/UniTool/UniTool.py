## @file
# generate UQI (Universal Question Identifier) unicode string for HII question PROMPT string. UQI string can be used to
# identify each HII question.
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

import re
import sys
import os
import getopt
import codecs
import fnmatch
import logging
import argparse

# global variable declarations
QuestionError = False
UqiList = re.compile('^#string[ \t]+([A-Z_0-9]+)[ \t]+#language[ \t]+uqi[ \t\r\n]+"(?:[x\S]{1,2})([0-9a-fA-F]{4,5})"',
                     re.M).findall
AllUqis = {}
StringDict = {}
GlobalVarId = {}
Options = {}

# Version message
__prog__ = 'UniTool'
__description__ = 'generate UQI unicode string for HII question PROMPT string.'
__copyright__ = 'Copyright (c) 2019, Intel Corporation. All rights reserved.<BR> '
__version__ = '%s Version %s' % (__prog__, '0.1 ')
_Usage = "Syntax:  %s [-b] [-u] [-l] [-x] [-h] [-d 'rootDirectory1'] [-d 'rootDirectory2'] [-d 'rootDirectory3']... \n[-q e|w]" \
         "'rootDirectory0' 'uqiFile'|'uqiFileDirectory' ['excludedDirectory1'] ['excludedDirectory2'] ['excludedDirectory3']...\n" \
         """\nFunction will sync up UQI definitions with uni files based on vfi/vfr/hfr/sd/sdi in the tree.\n
         Required Arguments:
           'rootdirectory0'       path to root directory
           'uqiFileDirectory'     path to UQI file(UqiList.uni)
           'uqiFile'              UQI file

         Return error if any duplicated UQI string or value in UQI list or if no definition
         for any string referred by HII question when -b or -u is specified

         NOTE: Options must be specified before parameters
         """ % (os.path.basename(sys.argv[0]))


# **********************************************************************
# description: Get uni file encoding
#
# arguments:   Filename - name of uni file
#
# returns:     utf-8 or utf-16
#
def GetUniFileEncoding(Filename):
    #
    # Detect Byte Order Mark at beginning of file.  Default to UTF-8
    #
    Encoding = 'utf-8'

    #
    # Read file
    #
    try:
        with open(Filename, mode='rb') as UniFile:
            FileIn = UniFile.read()
    except:
        return Encoding

    if (FileIn.startswith(codecs.BOM_UTF16_BE) or FileIn.startswith(codecs.BOM_UTF16_LE)):
        Encoding = 'utf-16'

    return Encoding


# rewrite function os.path.walk
def Walk(Top, Func, Arg):
    try:
        Names = os.listdir(Top)
    except os.error:
        return
    Func(Arg, Top, Names)
    for Name in Names:
        Name = os.path.join(Top, Name)
        if os.path.isdir(Name):
            Walk(Name, Func, Arg)


# **********************************************************************
# description: Parses commandline arguments and options
#              Calls function processUni to build dictionary of strings
#              Calls other functions according to user specified options
#
# arguments:   argv - contains all input from command line
#                   - must contain path to root directory
#                   - may contain options -h, -u, -l, -b or -x before path
#
# returns:     none
#
def main():
    ##### Read input arguments and options
    global AllUqis, UqiList, QuestionError
    parser = argparse.ArgumentParser(prog=__prog__,
                                     description=__description__ + __copyright__,
                                     conflict_handler='resolve')
    parser.add_argument('-v', '--version', action='version', version=__version__,
                        help="show program's version number and exit")
    parser.add_argument('-b', '--build', action='store_true', dest='BuildOption',
                        help="Build option returns error if any new UQI needs assigning " \
                             "based on vfi/vfr/hfr/sd/sdi when no -u option is specified")
    parser.add_argument('-u', '--updata', action='store_true', dest='UpdateUQIs',
                        help="Create new UQIs that does not already exist in uqiFile for" \
                             "any string requiring a UQI based on vfi/vfr/hfr/sd/sdi" \
                             "NOTE: 'uqiFile' cannot be readonly!")
    parser.add_argument('-l', '--lang', action='store_true', dest='LangOption',
                        help="Language deletion option (keeps only English and uqi)" \
                             "moves all UQIs to 'uqiFile', NOTE: Uni files cannot be readonly!")
    parser.add_argument('-x', '--exclude', metavar='FILEDIR', action='append', dest='ExcludeOption',
                        help="Exclude 'rootDirectory'/'excludedDirectory1' &" \
                             "'rootDirectory'/'excludedDirectory2'... from UQI list build")
    parser.add_argument('-d', '--dir', action='append', metavar='FILEDIR', dest='DirName',
                        help="Add multiple root directories to process")
    parser.add_argument('-q', '--question', dest='Question', choices=['w', 'e'],
                        help="Print warning(w) or return error(e) if different HII questions" \
                             "are referring same string token")
    parser.add_argument('-o', '--output', metavar='FILENAME', dest='Output',
                        help="Uni outPut file")
    Opts = parser.parse_args()

    ExcludeOption = False
    DirNameList = Opts.DirName
    QuestionOption = Opts.Question
    Destname = Opts.Output
    ExDirList = Opts.ExcludeOption
    if ExDirList:
        ExcludeOption = True
    BuildOption = Opts.BuildOption
    UpdateUQIs = Opts.UpdateUQIs
    LangOption = Opts.LangOption
    ExPathList = []

    if ExDirList:
        try:
            for EachExDir in ExDirList:
                for EachRootDir in DirNameList:
                    if EachExDir == EachRootDir:
                        print("\nERROR: excludedDirectory is same as rootDirectory\n")
                        return
                    ExPathList.append(EachRootDir + os.sep + EachExDir)
        except:
            print(_Usage)
            return

    global Options
    Options = {'Destname': Destname, 'DirNameList': DirNameList, 'ExPathList': ExPathList, 'BuildOption': BuildOption,
               'UpdateUQIs': UpdateUQIs, 'LangOption': LangOption, 'ExcludeOption': ExcludeOption,
               'QuestionOption': QuestionOption}
    print("UQI file: %s" % Destname)
    for EachDirName in DirNameList:
        Walk(EachDirName, processUni, None)
    if QuestionError:
        return
    if os.path.isdir(Options['Destname']):
        DestFileName = Options['Destname'] + os.sep + 'UqiList.uni'
    else:
        DestFileName = Options['Destname']
    if os.path.exists(DestFileName) and (DestFileName not in list(AllUqis.keys())):
        try:
            Encoding = GetUniFileEncoding(DestFileName)
            with codecs.open(DestFileName, 'r+', Encoding) as destFile:
                DestFileBuffer = destFile.read()
        except IOError as e:
            print("ERROR: " + e.args[1])
            return
        AllUqis[DestFileName] = UqiList(DestFileBuffer)
    if BuildOption:
        ReturnVal = newUqi()
        if (ReturnVal == 1):
            print('Please fix UQI ERROR(s) above before proceeding.')
        else:
            print("No UQI issues detected\n")
    return


# **********************************************************************
# description: newUqi collects a list of all currently used uqi values in the tree
#              Halt build if any duplicated string or value in UQI list.
#              If -u option was specified, creates new UQIs that does not
#              already exist in uqiFile for any string requiring a UQI.
#
# arguments:   none
#
# returns:     0 on success
#              1 on error - this should cause the build to halt
#

Syntax = "S"
SyntaxRE = re.compile('#string[ \t]+[A-Z_0-9]+[ \t]+#language[ \t]+uqi[ \t\r\n]+"([x\S]{1,2}).*', re.DOTALL).findall


def newUqi():
    global Options, GlobalVarId, AllUqis, Syntax, SyntaxRE
    UqiRange = []
    UqiStringList = []
    CreateUQI = []
    ReturnVal = 0
    BaseNumSpaces = 47  # Used to line up the UQI values in the resulting uqiFile

    # Look for duplication in the current UQIs and collect current range of UQIs
    for path in AllUqis.keys():
        for UqiString in AllUqis[path]:  # path contains the path and Filename of each uni file
            # Checks for duplicated strings in UQI list
            for TempString in UqiStringList:
                if TempString == UqiString[0]:
                    print("ERROR: UQI string %s was assigned more than once and will cause corruption!" % UqiString[0])
                    print("Delete one occurrence of the string and rerun tool.")
                    ReturnVal = 1  # halt build

            UqiStringList.append(UqiString[0])

            # Checks for duplicated UQI values in UQI list
            if int(UqiString[1], 16) in UqiRange:
                print("ERROR: UQI value %04x was assigned more than once and will cause corruption!" % int(UqiString[1],
                                                                                                           16))
                print("Delete one occurrance of the UQI and rerun tool to create alternate value.")
                ReturnVal = 1  # halt build
            UqiRange.append(int(UqiString[1], 16))

    for StringValue in GlobalVarId.keys():
        StringFound = False
        for path in StringDict.keys():
            for UniString in StringDict[path]:  # path contains the path and Filename of each uni file
                if (StringValue == UniString):
                    StringFound = True
                    break
        if not StringFound:
            print("ERROR: No definition for %s referred by HII question" % (StringValue))
            ReturnVal = 1  # halt build

    # Require a UQI for any string in vfr/vfi files
    for StringValue in GlobalVarId.keys():
        # Ignore strings defined as STRING_TOKEN(0)
        if (StringValue != "0"):
            # Check if this string already exists in the UQI list
            if (StringValue not in UqiStringList) and (StringValue not in CreateUQI):
                CreateUQI.append(StringValue)
                if not Options['UpdateUQIs']:
                    print("ERROR: No UQI for %s referred by HII question" % (StringValue))
                    ReturnVal = 1  # halt build after printing all error messages

    if (ReturnVal == 1):
        return ReturnVal

    # Update uqiFile with necessary UQIs
    if Options['UpdateUQIs'] and CreateUQI:
        if os.path.isdir(Options['Destname']):
            DestFileName = Options['Destname'] + os.sep + 'UqiList.uni'
        else:
            DestFileName = Options['Destname']
        try:
            Encoding = GetUniFileEncoding(DestFileName)
            with codecs.open(DestFileName, 'r+', Encoding) as OutputFile:
                PlatformUQI = OutputFile.read()
        except IOError as e:
            print("ERROR: " + e.args[1])
            if (e.args[0] == 2):
                try:
                    with codecs.open(DestFileName, 'w', Encoding) as OutputFile:
                        print(DestFileName + " did not exist.  Creating new file.")
                        PlatformUQI = ''
                except:
                    print("Error creating " + DestFileName + ".")
                    return 1
            if (e.args[1] == "Permission denied"):
                print(
                    "\n%s is Readonly.  You must uncheck the ReadOnly attibute to run the -u option.\n" % DestFileName)
                return 1

        # Determines and sets the UQI number format
        # TODO: there is probably a more elegant way to do this...
        SyntaxL = SyntaxRE(PlatformUQI)
        if len(SyntaxL) != 0:
            Syntax = SyntaxL[0]

        # script is reading the file in and writing it back instead of appending because the codecs module
        # automatically adds a BOM wherever you start writing. This caused build failure.
        UqiRange.sort()
        if (UqiRange == []):
            NextUqi = 0
        else:
            NextUqi = UqiRange[len(UqiRange) - 1] + 1

        for StringValue in CreateUQI:
            print("%s will be assigned a new UQI value" % StringValue)
            UqiRange.append(NextUqi)
            #
            # Lines up the UQI values in the resulting uqiFile
            #
            Spaces = " " * (BaseNumSpaces - len(StringValue))
            PlatformUQI += '#string %s%s #language uqi \"%s%04x\"\r\n' % (StringValue, Spaces, Syntax, NextUqi)
            print("#string %s%s #language uqi  \"%s%04X\"" % (StringValue, Spaces, Syntax, NextUqi))
            NextUqi += 1

        with codecs.open(DestFileName, 'r+', Encoding) as OutputFile:
            OutputFile.seek(0)
            OutputFile.write(PlatformUQI)

    return 0


# **********************************************************************
# description: Parses each uni file to collect dictionary of strings
#              Removes additional languages and overwrites current uni files
#              if -l option was specified
#
# arguments:   path - directory location of file including file name
#              Filename - name of file to be modified
#
# returns:     error string if failure occurred;
#              none if completed sucessfully
#
# the following are global so that parsefile is quicker

FindUniString = re.compile(
    '^#string[ \t]+([A-Z_0-9]+)(?:[ \t\r\n]+#language[ \t]+[a-zA-Z-]{2,5}[ \t\r\n]+".*"[ \t]*[\r]?[\n]?)*',
    re.M).findall

OtherLang = re.compile(
    '^#string[ \t]+[A-Z_0-9]+(?:[ \t\r\n]+#language[ \t]+[a-zA-Z-]{2,5}[ \t\r\n]+".*"[ \t]*[\r]?[\n]?)*', re.M).findall
EachLang = re.compile('[ \t\r\n]+#language[ \t]+([a-zA-Z-]{2,5})[ \t\r\n]+".*"[ \t]*[\r]?[\n]?').findall

UqiStrings = re.compile('^#string[ \t]+[A-Z_0-9]+[ \t]+#language[ \t]+uqi[ \t\r\n]+".*"[ \t]*[\r]?[\n]?', re.M)


def parsefile(path, Filename):
    global Options, StringDict, AllUqis, UqiList, FindUniString, OtherLang, EachLang, UqiStrings

    FullPath = path + os.sep + Filename

    try:
        UniEncoding = GetUniFileEncoding(FullPath)
        with codecs.open(FullPath, 'r', UniEncoding) as UniFile:
            Databuffer = UniFile.read()
    except:
        print("Error opening " + FullPath + " for reading.")
        return
    WriteFile = False

    if os.path.isdir(Options['Destname']):
        DestFileName = Options['Destname'] + os.sep + 'UqiList.uni'
    else:
        DestFileName = Options['Destname']

    if Options['LangOption']:
        try:
            UqiEncoding = GetUniFileEncoding(DestFileName)
            with codecs.open(DestFileName, 'r+', UqiEncoding) as OutputFile:
                PlatformUQI = OutputFile.read()
        except IOError as e:
            print("ERROR: " + e.args[1])
            if (e.args[0] == 2):
                try:
                    with codecs.open(DestFileName, 'w', UqiEncoding) as OutputFile:
                        print(DestFileName + " did not exist.  Creating new file.")
                        PlatformUQI = ''
                except:
                    print("Error creating " + DestFileName + ".")
                    return
            else:
                print("Error opening " + DestFileName + " for appending.")
                return

        if (Filename != DestFileName.split(os.sep)[-1]):
            Uqis = re.findall(UqiStrings, Databuffer)
            if Uqis:
                for Uqi in Uqis:
                    PlatformUQI += Uqi
                with codecs.open(DestFileName, 'r+', UqiEncoding) as OutputFile:
                    OutputFile.seek(0)
                    OutputFile.write(PlatformUQI)
            Databuffer = re.sub(UqiStrings, '', Databuffer)
            if Uqis:
                WriteFile = True
                print("Deleted uqis from %s" % FullPath)
            stringlist = OtherLang(Databuffer)
            for stringfound in stringlist:
                ThisString = EachLang(stringfound)
                for LanguageFound in ThisString:
                    if ((LanguageFound != 'en') and (LanguageFound != 'en-US') and (LanguageFound != 'eng') and (
                            LanguageFound != 'uqi')):
                        Databuffer = re.sub(re.escape(stringfound), '', Databuffer)
                        WriteFile = True
                        print("Deleted %s from %s" % (LanguageFound, FullPath))
    if (Filename != DestFileName.split(os.sep)[-1]):
        # adding strings to dictionary
        StringDict[r'%s' % FullPath] = FindUniString(Databuffer)
    # adding UQIs to dictionary
    AllUqis[r'%s' % FullPath] = UqiList(Databuffer)

    if WriteFile:
        try:
            with codecs.open(FullPath, 'w', UniEncoding) as UniFile:
                UniFile.write(Databuffer)
        except:
            print("Error opening " + FullPath + " for writing.")
    return


# **********************************************************************
# description: Searches tree for uni files
#              Calls parsefile to collect dictionary of strings in each uni file
#              Calls searchVfiFile for each vfi or vfr file found
#
# arguments:   argument list is built by os.path.walk function call
#              arg     - None
#              dirname - directory location of files
#              names   - specific files to search in directory
#
# returns:     none
#
def processUni(args, dirname, names):
    global Options
    # Remove excludedDirectory
    if Options['ExcludeOption']:
        for EachExDir in Options['ExPathList']:
            for dir in names:
                if os.path.join(dirname, dir) == EachExDir:
                    names.remove(dir)

    for entry in names:
        FullPath = dirname + os.sep + entry
        if fnmatch.fnmatch(FullPath, '*.uni'):
            parsefile(dirname, entry)
        if fnmatch.fnmatch(FullPath, '*.vf*'):
            searchVfiFile(FullPath)
        if fnmatch.fnmatch(FullPath, '*.sd'):
            searchVfiFile(FullPath)
        if fnmatch.fnmatch(FullPath, '*.sdi'):
            searchVfiFile(FullPath)
        if fnmatch.fnmatch(FullPath, '*.hfr'):
            searchVfiFile(FullPath)
    return


# **********************************************************************
# description: Compose a dictionary of all strings that may need UQIs assigned
#              to them and key is the string
#
# arguments:   Filename - name of file to search for strings
#
# returns:     none
#

# separate regexes for readability
StringGroups = re.compile(
    '^[ \t]*(?:oneof|numeric|checkbox|orderedlist)[ \t]+varid.+?(?:endoneof|endnumeric|endcheckbox|endorderedlist);',
    re.DOTALL | re.M).findall
StringVarIds = re.compile(
    '[ \t]*(?:oneof|numeric|checkbox|orderedlist)[ \t]+varid[ \t]*=[ \t]*([a-zA-Z_0-9]+\.[a-zA-Z_0-9]+)').findall
StringTokens = re.compile('prompt[ \t]*=[ \t]*STRING_TOKEN[ \t]*\(([a-zA-Z_0-9]+)\)').findall


def searchVfiFile(Filename):
    global Options, GlobalVarId, StringGroups, StringVarIds, StringTokens, QuestionError
    try:
        with open(Filename, 'r') as VfiFile:
            Databuffer = VfiFile.read()

        # Finds specified lines in file
        VfiStringGroup = StringGroups(Databuffer)

        # Searches for prompts within specified lines
        for EachGroup in VfiStringGroup:
            for EachString in StringTokens(EachGroup):
                # Ignore strings defined as STRING_TOKEN(0), STRING_TOKEN(STR_EMPTY) or STRING_TOKEN(STR_NULL)
                if (EachString != "0") and (EachString != "STR_EMPTY") and (EachString != "STR_NULL"):
                    if EachString not in GlobalVarId:
                        GlobalVarId[EachString] = StringVarIds(EachGroup)
                    else:
                        if (GlobalVarId[EachString][0] != StringVarIds(EachGroup)[0]):
                            if Options['QuestionOption']:
                                if Options['QuestionOption'] == "e":
                                    QuestionError = True
                                    print("ERROR:"),
                                if Options['QuestionOption'] == "w":
                                    print("WARNING:"),
                                print("%s referred by different HII questions(%s and %s)" % (
                                    EachString, GlobalVarId[EachString][0], StringVarIds(EachGroup)[0]))
    except:
        print("Error opening file at %s for reading." % Filename)


if __name__ == '__main__':
    sys.exit(main())
