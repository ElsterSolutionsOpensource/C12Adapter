# /usr/bin/python
#
# Generates
#     src/MeteringSDK/MCORE/private/encodings.cxx
# Review and run this script regularly

import sys
import os
import re

mappingsRootUrl = "ftp://ftp.unicode.org/Public/MAPPINGS/"
outFileNameCxx  = "src/MeteringSDK/MCORE/private/encodings.cxx"

encodings = (
    ("8859-1",  "ISO8859/8859-1.TXT",  "Western European: German, Icelandic, Irish, Italian, Norwegian, Portuguese, Spanish"),
    ("8859-2",  "ISO8859/8859-2.TXT",  "Central European: Bosnian, Polish, Croatian, Czech, Slovak, Slovene, Serbian, Hungarian"),
    ("8859-3",  "ISO8859/8859-3.TXT",  "South European: Maltese, Esperanto"),
    ("8859-4",  "ISO8859/8859-4.TXT",  "North European: Estonian, Latvian, Lithuanian, Greenlandic, and Sami"),
    ("8859-5",  "ISO8859/8859-5.TXT",  "Latin/Cyrillic: Belarusian, Bulgarian, Macedonian, Russian, Serbian, Ukrainian"),
    ("8859-6",  "ISO8859/8859-6.TXT",  "Latin/Arabic"),
    ("8859-7",  "ISO8859/8859-7.TXT",  "Latin/Greek"),
    ("8859-8",  "ISO8859/8859-8.TXT",  "Latin/Hebrew"),
    ("8859-9",  "ISO8859/8859-9.TXT",  "Latin-5 Turkish"),
    ("8859-10", "ISO8859/8859-10.TXT", "Latin-6 Nordic: better fit for some North European languages than 8859-4"),
    ("8859-11", "ISO8859/8859-11.TXT", "Latin/Thai"),
    # there is no "8859-12" or "ISO8859/8859-12.TXT"
    ("8859-13", "ISO8859/8859-13.TXT", "Latin-7 Baltic Rim: other variation of 8859-4 and 8859-10"),
    ("8859-14", "ISO8859/8859-14.TXT", "Latin-8 Celtic"),
    ("8859-15", "ISO8859/8859-15.TXT", "Latin-9 Better 8859-1 with the Euro sign"),
    ("8859-16", "ISO8859/8859-16.TXT", "Latin-10 South-Eastern European"),
    ("CP1250",  "VENDORS/MICSFT/WindowsBestFit/bestfit1250.txt", "Microsoft ANSI/OEM Eastern Europe"),
    ("CP1251",  "VENDORS/MICSFT/WindowsBestFit/bestfit1251.txt", "Microsoft ANSI/OEM Cyrillic"),
    ("CP1252",  "VENDORS/MICSFT/WindowsBestFit/bestfit1252.txt", "Microsoft ANSI/OEM Latin I"),
    ("CP1253",  "VENDORS/MICSFT/WindowsBestFit/bestfit1253.txt", "Microsoft ANSI/OEM Greek"),
    ("CP1254",  "VENDORS/MICSFT/WindowsBestFit/bestfit1254.txt", "Microsoft ANSI/OEM Turkish"),
    ("CP1255",  "VENDORS/MICSFT/WindowsBestFit/bestfit1255.txt", "Microsoft ANSI/OEM Hebrew"),
    ("CP1256",  "VENDORS/MICSFT/WindowsBestFit/bestfit1256.txt", "Microsoft ANSI/OEM Arabic"),
    ("CP1257",  "VENDORS/MICSFT/WindowsBestFit/bestfit1257.txt", "Microsoft ANSI/OEM Baltic"),
    ("CP437",   "VENDORS/MICSFT/PC/CP437.TXT",                   "Microsoft DOS Latin US"),
    ("CP737",   "VENDORS/MICSFT/PC/CP737.TXT",                   "Microsoft DOS Greek"),
    ("CP775",   "VENDORS/MICSFT/PC/CP775.TXT",                   "Microsoft DOS Baltic Rim"),
    ("CP850",   "VENDORS/MICSFT/PC/CP850.TXT",                   "Microsoft DOS Latin1"),
    ("CP852",   "VENDORS/MICSFT/PC/CP852.TXT",                   "Microsoft DOS Latin2"),
    ("CP855",   "VENDORS/MICSFT/PC/CP855.TXT",                   "Microsoft DOS Cyrillic"),
    ("CP857",   "VENDORS/MICSFT/PC/CP857.TXT",                   "Microsoft DOS Turkish"),
    ("CP860",   "VENDORS/MICSFT/PC/CP860.TXT",                   "Microsoft DOS Portuguese"),
    ("CP861",   "VENDORS/MICSFT/PC/CP861.TXT",                   "Microsoft DOS Icelandic"),
    ("CP862",   "VENDORS/MICSFT/PC/CP862.TXT",                   "Microsoft DOS Hebrew"),
    ("CP863",   "VENDORS/MICSFT/PC/CP863.TXT",                   "Microsoft DOS French Canada"),
    ("CP864",   "VENDORS/MICSFT/PC/CP864.TXT",                   "Microsoft DOS Arabic"),
    ("CP865",   "VENDORS/MICSFT/PC/CP865.TXT",                   "Microsoft DOS Nordic"),
    ("CP866",   "VENDORS/MICSFT/PC/CP866.TXT",                   "Microsoft DOS Cyrillic Russian"),
    ("CP869",   "VENDORS/MICSFT/PC/CP869.TXT",                   "Microsoft DOS Greek2"),
    ("CP874",   "VENDORS/MICSFT/PC/CP874.TXT",                   "Microsoft DOS Thai")
)

encodingNameRE = re.compile("^[A-Z0-9-]{1,7}$")
emptyLineRE = re.compile("^\\s*([#;].*)?$")  # with optional comment
simpleMapRE = re.compile("(0x[0-9a-fA-F]{2})\\s+(0x[0-9a-fA-F]{4})\\s*(?:[#;]\\s*(.+))?")
undefinedMapRE = re.compile("(0x[0-9a-fA-F]{2})\\s+(?:[#;]\\s*(.+))?")
windowsIgnoreRE = re.compile("^(?:CODEPAGE|CPINFO|\x1A).*")
windowsStartRE = re.compile("^MBTABLE[ \t]+([0-9]+).*")
windowsStopRE = re.compile("^WCTABLE[ \t]+.*")

def writeText(fileName, text):
    rootDir = os.path.dirname(os.path.dirname(os.path.realpath(__file__))) # two levels up this file to reach development root
    outFileNameList = [rootDir]
    outFileNameList.extend(fileName.split('/'))
    fullOutFileName = os.path.join(*outFileNameList)
    open(fullOutFileName, "wb").write(text.encode('utf8'))       # always write UNIX text format
    print("-- Written: " + fullOutFileName)

def collectEncodingData(encoding, mapping, info):
    if not encodingNameRE.match(encoding):
        raise Exception("Encoding should consist of up to seven uppercase letters, digits and dash, but given " + encoding)
    mappingUrl = "ftp://ftp.unicode.org/Public/MAPPINGS/" + mapping
    print("-- Encoding '" + encoding + '" in: ' + mappingUrl)
    if sys.version[0] == '2': # Python 2.x
        import urllib2
        text = urllib2.urlopen(mappingUrl).read()
    else:
        import urllib.request
        text = urllib.request.urlopen(mappingUrl).read().decode('utf8')

    positionalMap = []
    lineNumber = 0
    first = 0xFF
    last = 0
    for line in text.split('\n'):
        lineNumber += 1
        m = emptyLineRE.match(line)
        if m:
            continue
        m = simpleMapRE.match(line)
        if m:
            charCode = int(m.groups()[0], 0)
            wcharCode = int(m.groups()[1], 0)
            description = m.groups()[2]
            if not description:
                description = ""
            if first == 0xFF:
                if charCode != wcharCode:
                    first = charCode
                    positionalMap.append((charCode, wcharCode, description))
            else:
                positionalMap.append((charCode, wcharCode, description))
            last = charCode
            continue
        m = undefinedMapRE.match(line)
        if m:
            charCode = int(m.groups()[0], 0)
            description = m.groups()[1]
            if not description:
                description = ""
            if first == 0xFF:
                if charCode != wcharCode:
                    first = charCode
                    positionalMap.append((charCode, 0, description))
            else:
                positionalMap.append((charCode, 0, description))
            last = charCode
            continue
        m = windowsStartRE.match(line)
        if m:
            numChars = m.groups()[0]
            if numChars != "256":
                raise Exception("We do not support Windows best fit format if the number of characters is not 256")
            continue
        m = windowsStopRE.match(line)
        if m:
            break
        m = windowsIgnoreRE.match(line)
        if not m:
            raise Exception("%s(%d): bad line '%s'" % (mappingUrl, lineNumber, line))
    if first > last:
        raise Exception("%s(%d): Suspicious, first is bigger than last: '%d > %d'" % (first, last))
    return {"encoding"        : encoding,
            "info"            : info,
            "mapping"         : mapping,
            "mappingUrl"      : mappingUrl,
            "first"           : first,
            "last"            : last,
            "positionalMap"   : positionalMap}

def positionalMapToString(encodingData):
    first = encodingData["first"]
    last = encodingData["last"]
    if first == last:
        s = ""
        encodingData["positionalMapId"] = "NULL"
    else:
        encoding = encodingData["encoding"]
        mappingUrl = encodingData["mappingUrl"]
        encodingData["positionalMapId"] = ("s_positionalMap__%s" % encoding).replace("-", "_")
        s  = "static const Muint16 %s[] =\n" % encodingData["positionalMapId"]
        s += "   { // Codepage %s source %s\n" % (encoding, mappingUrl)
        positionalMap = encodingData["positionalMap"]
        for i in range(len(positionalMap)):
            charCode    = positionalMap[i][0]
            wcharCode   = positionalMap[i][1]
            description = positionalMap[i][2]
            if len(description) > 0:
                description = ": " + description
            s += "      0x%04X%s  // '\\x%02X'%s\n" \
                    % (wcharCode, (' ' if (i == len(positionalMap) - 1) else ','), charCode, description)
        s += "   };\n\n"
    return s

def codepageToString(encodingData):
    first = encodingData["first"]
    last = encodingData["last"]
    encoding = encodingData["encoding"]
    encodingRemainder = 7 - len(encoding)
    if encodingRemainder < 0:
        raise Exception("Encoding name should fit within 7 characters, but given: " + encoding)
    elif encodingRemainder > 0:
        encoding += "\\0" * encodingRemainder
    info = encodingData["info"]
    mappingUrl = encodingData["mappingUrl"]
    positionalMap = encodingData["positionalMap"]
    s =  '      {  // %s\n'  % info
    s += '         {"%s"}, // Source %s\n' % (encoding, mappingUrl)
    s += '         0x%02X, // First narrow character that does not match wide character\n' % first
    s += '         0x%02X, // Last narrow character in this codepage\n' % last
    if encodingData["positionalMapId"] == "NULL":
        s += "         NULL\n"
    else:
        s += "         const_cast<Muint16*>(%s) // C++Builder workaround, cannot declare fields as const\n" % encodingData["positionalMapId"]
    s += '      },\n'
    return s

def generateAllEncodings(encodings):
    global outFileNameCxx

    encodingDataList = []
    for encoding, mapping, info in encodings:
        encodingDataList.append(collectEncodingData(encoding, mapping, info))

    s  = "// File " + outFileNameCxx + "\n"
    s += "//\n"
    s += "// DO NOT EDIT! THIS FILE IS GENERATED by script\n"
    s += "//    bin/update_encodings.py\n"
    s += "// The source is the online data for the mapping of a one-byte codepage to Unicode\n\n"
    for encodingData in encodingDataList:
        s += positionalMapToString(encodingData)
        
    s += "static const MOneByteCodepage s_oneByteCodepages[] =\n   {\n"
    for encodingData in encodingDataList:
        s += codepageToString(encodingData)
    s += '      {{"ASCII\\0\\0"}, 0x80, 0x7F, NULL}\n   };\n\n'
    s += "// DO NOT EDIT! THIS FILE IS GENERATED!\n//\n// End of file\n"
    writeText(outFileNameCxx, s)

if __name__ == '__main__':
    generateAllEncodings(encodings)
