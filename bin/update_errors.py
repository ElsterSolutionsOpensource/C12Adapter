#!/usr/bin/python
#
# Update error definitions:
#    - remove unused error codes
#    - create MErrorEnum.h and MErrorEnum.cpp contents

import os, sys, subprocess, shutil, re, glob, string, tempfile, fnmatch
from private import utilities
from optparse import OptionParser

usedErrorSymbols = [
        # nothing here currently
        ]

mErrorEnumH = r"""#ifndef MCORE_MERRORENUM_H
#define MCORE_MERRORENUM_H

//
// THIS FILE IS GENERATED, DO NOT EDIT!
// Instead edit library's *Errors.inc files and generate this file with bin/update_errors.py.
//

/// \addtogroup MCORE
///@{
/// \file MCORE/MErrorEnum.h
///
/// List of MeteringSDK error constants.
///
/// THIS FILE IS GENERATED, DO NOT EDIT!
/// Instead edit library's *Errors.inc files and generate this file with bin/update_errors.py.
///

#include <MCORE/MObject.h>

/// Error enumeration
///
class M_CLASS MErrorEnum : public MObject
{
public: // Types:

   /// Actual enumeration type.
   ///
   enum Type
   {
%s
   };

private: // Prevent abusing this class as value:

   MErrorEnum();
   MErrorEnum(const MErrorEnum&);
   MErrorEnum& operator=(const MErrorEnum&);
   bool operator==(const MErrorEnum& other) const;
   bool operator!=(const MErrorEnum& other) const;

   M_DECLARE_CLASS(ErrorEnum)
};

// Private library errors

%s///@}
#endif
"""

mErrorEnumCpp = r"""// File MCORE/MErrorEnum.cpp

#include "MCOREExtern.h"
#include "MErrorEnum.h"

//
// THIS FILE IS GENERATED, DO NOT EDIT!
// Instead edit library's *Errors.inc files and generate this file with bin/update_errors.py
//

M_START_PROPERTIES(ErrorEnum)
%sM_START_METHODS(ErrorEnum)
M_END_CLASS(ErrorEnum, Object)
"""

def findStringInSources(path, excludeFileMask, s):
    for file in glob.glob(os.path.join(path, "*")):
        if os.path.isdir(file):
            if findStringInSources(file, excludeFileMask, s):
                return True
        elif re.match(excludeFileMask, os.path.basename(file)):
            #if open(file, "r").read().find(s) > 0:
            #    utilities.log("findErrorInSources skips " + file + " where we found " + s)
            pass
        elif open(file, "r").read().find(s) > 0:
            #utilities.log("  Found %s in %s" % (s, file))
            return True
    return False

def cleanupErrorsInDirectory(path, fullPathToErrorsFile, allErrors):
    error = False
    newTxt = ""
    collectTxt = ""
    searchExcludeMask = "^(.*Errors.inc)|(MErrorEnum.*)$"
    utilities.log("Processing " + fullPathToErrorsFile)
    txt = open(fullPathToErrorsFile, "r").read()
    for line in utilities.split_without_last_empty_line(txt):
        if len(line.strip()) == 0:
            if len(collectTxt) > 0:
                newTxt += collectTxt + "\n"
                collectTxt = ""
            elif len(newTxt) < 3 or newTxt[-3:] != '\n\n\n': # do not have too many blank lines
                newTxt += "\n"
        else:
            line = line.strip("\r")
            if re.match("^[ \t]*[/#].*$", line):
                collectTxt += line + '\n'
            else:
                r = re.match("^(M__ERROR)\\(([A-Z_0-9]+),[ \t]*([x0-9A-F]+)\\)", line)
                if not r:
                    r = re.match("^(M__ERROR_ENUM)\\(([A-Za-z0-9]+),[ \t]*(0x[0-9A-F]+)\\)", line)
                    if not r:
                        print("!!! Line not matched: " + line)
                        error = True
                        continue
                macro = r.groups(0)[0]
                symbol = r.groups(0)[1]
                code = int(r.groups(0)[2], 0)
                if code in allErrors:
                    print("!!! Duplicate 0x%08X: %s:%s vs. %s:%s" \
                        % (code, os.path.basename(fullPathToErrorsFile), symbol, allErrors[code]["file_name"], allErrors[code]["symbol"]))
                    error = True
                    continue

                if findStringInSources(path, searchExcludeMask, symbol):
                    #utilities.log("  %s, 0x%08X found" % (symbol, code))
                    add = True
                elif findStringInSources(os.path.dirname(path), searchExcludeMask, symbol):
                    utilities.log("  %s, 0x%08X found outside its directory" % (symbol, code))
                    add = True
                elif symbol in usedErrorSymbols:
                    utilities.log("    %s, 0x%08X not used in code, but included explicitly" % (symbol, code))
                    add = True
                else:
                    utilities.log("  %s, 0x%08X excluded" % (symbol, code))
                    add = False

                if add:
                    allErrors[code] = dict(macro     = macro,
                                           symbol    = symbol,
                                           file_name = os.path.basename(fullPathToErrorsFile),
                                           comment   = collectTxt.strip())
                    newTxt += collectTxt
                    intCode = code if code < 0x7FFFFFFF else code - 0x100000000
                    newTxt += "%s(%s, 0x%08X) // (unsigned)%u, (int)%d\n" % (macro, symbol, code, code, intCode)
                collectTxt = ""
    if error:
        utilities.print_and_exit("There were errors while processing %s" % fullPathToErrorsFile)

    if txt.strip() == newTxt.strip():
        utilities.log("File '%s' did not change" % fullPathToErrorsFile)
    else:
        open(fullPathToErrorsFile, "w").write(newTxt)
        utilities.log("File '%s' written" % fullPathToErrorsFile)

def updateAllErrorsIntoDict(sourceDir, allErrors):
    for file in glob.glob(os.path.join(sourceDir, "*")):
        if os.path.isdir(file):
            fullPathToErrorsFile = os.path.join(file, os.path.basename(file) + "Errors.inc")
            if not os.path.exists(fullPathToErrorsFile):
                updateAllErrorsIntoDict(file, allErrors)
            else:
                cleanupErrorsInDirectory(file, fullPathToErrorsFile, allErrors)

def generateErrorEnum(mcoreDir, allErrors):
    hPublicErrorsStr = ""
    hPrivateErrorsStr = ""
    cppPublicErrorsStr = ""
    currFilePublic = ""
    currFilePrivate = ""
    for code, v in sorted(allErrors.items()):
        if v["macro"] == "M__ERROR_ENUM":
            comment = v["comment"]
            if comment.find("///") >= 0:
                comment = comment.replace("///", "      ///")
            else:
                comment = comment.replace("//", "      ///")
            symbol = v["symbol"]
            fileName = v["file_name"]
            if currFilePublic != fileName:
                hPublicErrorsStr += "      // %s\n\n" % fileName
                currFilePublic = fileName
            hPublicErrorsStr += "%s\n      %s = 0x%08X,\n\n" % (comment, symbol, code)
            cppPublicErrorsStr += "   M_CLASS_ENUMERATION_UINT(ErrorEnum, %s)\n" % symbol
        elif v["macro"] == "M__ERROR":
            comment = v["comment"]
            if comment.find("///") < 0:
                comment = comment.replace("//", "///")
            symbol = v["symbol"]
            fileName = v["file_name"]
            if currFilePrivate != fileName:
                hPrivateErrorsStr += "// %s\n\n" % fileName
                currFilePrivate = fileName
            hPrivateErrorsStr += "%s\nconst MErrorEnum::Type %s = static_cast<MErrorEnum::Type>(0x%08X);\n\n" % (comment, symbol, code)
        else:
            utilities.print_and_exit("Program error, unknown macro %s" % v["macro"])
    
    hPublicErrorsStr = hPublicErrorsStr[:-3] # remove the last comma
    
    hErrorEnumFileNameH = os.path.join(mcoreDir, "MErrorEnum.h")
    hErrorEnumFileNameCpp = os.path.join(mcoreDir, "MErrorEnum.cpp")
    open(hErrorEnumFileNameH, "w").write(mErrorEnumH % (hPublicErrorsStr, hPrivateErrorsStr))
    open(hErrorEnumFileNameCpp, "w").write(mErrorEnumCpp % cppPublicErrorsStr)

def updateAllErrors(sourceDir):
    meteringSdkDir = os.path.join(sourceDir, "src", "MeteringSDK")
    mcoreDir = os.path.join(meteringSdkDir, "MCORE")
    if not os.path.isdir(mcoreDir):
        utilities.print_and_exit("Could not locate directory %s" % mcoreDir)
    allErrors = dict()
    updateAllErrorsIntoDict(meteringSdkDir, allErrors)
    generateErrorEnum(mcoreDir, allErrors)

if __name__ == '__main__':
    sourceDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) # two levels up this file to reach development root
    parser = OptionParser(usage="usage: %prog [options]")
    parser.description = "Update error definitions"
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="Verbose output")
    parser.add_option("--source-dir",    action="store", dest="srcdir", default=sourceDir, help="MeterTools directory, default %default")
    (opts, args) = parser.parse_args()
    if len(args) != 0:
        utilities.print_and_exit("No parameters are accepted, try --help")
    
    utilities.verbose = opts.verbose
    updateAllErrors(opts.srcdir)
