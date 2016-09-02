None of these files should be included neither into make/project files nor into user source code.

MCORE/private/*.cxx files are included privately into MCORE/*.cpp
MCORE/private/*.h files are included into MCORE/*.cpp and/or MCORE/*.h

Some of these files are based on external sources, as mentioned in the copyright notices
of these individual files.
Maintainer of these should be checking for the updates and make sure
all relevant bugs and security issues are merged from the following projects:

  - http://w1.fi/wpa_supplicant
    Used for AES key wrap/unwrap implementation
  - http://pugixml.org
    pugixml lightweight DOM parser
  - http://utfcpp.sourceforge.net
    UTF8 conversion library
  - https://github.com/morristech/android-ifaddrs 
    Android implementation of getifaddrs and freeifaddrs
  - https://opensource.apple.com/source/Heimdal/Heimdal-453.40.10/lib/roken/ifaddrs.hin?txt,
    https://opensource.apple.com/source/Heimdal/Heimdal-453.40.10/lib/roken/getifaddrs_w32.c?txt
    Windows implementation of getifaddrs and freeifaddrs.

The following files in this directory are automatically generated,
see the insides of these files for more information:
  - MTimeZoneMapping.cxx
  - encodings.cxx
