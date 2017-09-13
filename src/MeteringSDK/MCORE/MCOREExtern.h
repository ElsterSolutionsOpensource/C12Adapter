#ifndef MCORE_MCOREEXTERN_H
#define MCORE_MCOREEXTERN_H

#ifdef M_USE_USTL
#include <ustl.h>
#define std ustl
#endif

/// \addtogroup MCORE
///@{
/// \file MCORE/MCOREExtern.h
///
/// This file shall be included into every cpp source file that uses MeteringSDK.
/// Typically, this is a precompiled file.
///

//@{
/// Operating system interface masks.
/// One can use M_OS macro in conjunction
/// with these flags to determine which OS interface is supported.
/// These are the defines to allow their usage at compile time.
/// At runtime more precise information is available through MGetRuntimeOSMask call.
///
#define M_OS_ANSI_C     0x00000      ///< ANSI C interface required
#define M_OS_DOS        0x00001
#define M_OS_DOS16      0x00003
#define M_OS_DOS32      0x00007
#define M_OS_WINDOWS    0x00010
#define M_OS_WIN32      0x00040
#define M_OS_WIN64      0x00080
#define M_OS_WIN32_CE   0x01000
#define M_OS_POSIX      0x10000
#define M_OS_UNIX       0x20000
#define M_OS_CYGWIN     0x40000
#define M_OS_LINUX      0x80000
#define M_OS_UCLINUX    0x100000
#define M_OS_NUTTX      0x200000
#define M_OS_CMX        0x400000
#define M_OS_QNXNTO     0x2000000
#define M_OS_BSD        0x4000000
#define M_OS_ANDROID    0x8000000
//@}

/// Operating system define, uses flag values for supported interfaces
///
#if defined(__uClinux__)
   #define M_OS (M_OS_LINUX | M_OS_UCLINUX | M_OS_POSIX)
#elif defined (__nuttx__)
   #define M_OS (M_OS_NUTTX | M_OS_POSIX)
#elif defined (__cmx__)
   #define M_OS M_OS_CMX
#elif defined(__ANDROID__) || defined(__ANDROID_API__)
   #define M_OS (M_OS_ANDROID | M_OS_LINUX | M_OS_POSIX)
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)
   #define M_OS (M_OS_LINUX | M_OS_POSIX)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
   #define M_OS (M_OS_BSD | M_OS_POSIX)
#elif defined(__QNXNTO__)
   #define M_OS (M_OS_QNXNTO | M_OS_POSIX)
#elif defined(__CYGWIN__)
   #define M_OS (M_OS_CYGWIN | M_OS_POSIX)
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(_Windows) || defined(_WINDOWS)
   #if defined(_WIN32_WCE)
      #define M_OS (M_OS_WINDOWS | M_OS_WIN32 | M_OS_WIN32_CE)
   #elif defined(_WIN64) || defined(__WIN64__)
      #define M_OS (M_OS_WINDOWS | M_OS_WIN32 | M_OS_WIN64)
   #elif defined(_WIN32) || defined(__WIN32__)
      #define M_OS (M_OS_WINDOWS | M_OS_WIN32)
   #else
      #define M_OS (M_OS_WINDOWS | M_OS_WIN16)
   #endif
#elif defined(dos) || defined(__DOS__) || defined(__MSDOS__)
   #define M_OS (M_OS_DOS)
#elif defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE)
   #if defined(_POSIX_VERSION) || defined(_POSIX_SOURCE)
      #define M_OS (M_OS_UNIX | M_OS_POSIX)
   #else
      #define M_OS (M_OS_UNIX)
   #endif
#elif defined(_POSIX_VERSION) || defined(_POSIX_SOURCE)
   #define M_OS (M_OS_POSIX)
#else
   #error "Unable to determine host operating system, it may be unsupported..."
#endif

#ifdef _UNICODE
   #ifndef UNICODE
      #define UNICODE
   #endif
#else
   #ifdef UNICODE
      #define _UNICODE
   #endif
#endif

#if defined(_MSC_VER) // Visual C++

   // Disable Microsoft security warnings, as they are not
   #ifndef _CRT_SECURE_NO_WARNINGS
      #define _CRT_SECURE_NO_WARNINGS
   #endif

   #ifndef _SCL_SECURE_NO_WARNINGS
      #define _SCL_SECURE_NO_WARNINGS
   #endif

   // Disable warnings that are not informative
   //
   #pragma warning(disable : 4786) // disable "identifier truncated"
   #pragma warning(disable : 4273) // disable "inconsistent DLL linkage"
   #pragma warning(disable : 4231) // disable "the extern before template is a nonstandard extension"
   #pragma warning(disable : 4660) // disable "template-class specialization is already instantiated"
   #pragma warning(disable : 4251) // disable "class has to be exportable to be used outside DLL"
   #pragma warning(disable : 4073) // disable "initializers put in library initialization area"
   #pragma warning(disable : 4065) // disable "switch statement contains default but no case"
   #pragma warning(disable : 4355) // disable "this : used in base member initializer list"
   #pragma warning(disable : 4275) // disable "non dll-interface class used as base for dll-interface class"
   // #pragma warning(disable : 4702) // disable "unreachable code"
   // #pragma warning(disable : 4127) // disable "conditional expression is constant"
   #pragma warning(disable : 4290) // disable "C++ exception specification ignored except to indicate a function is not __declspec(nothrow)"

   // Enable useful warnings turned off by default
   #if M_DEBUG // some parameters may only be used by debug builds
      #pragma warning(3 : 4100) // warning level 3: unreferenced formal parameter
   #endif

   #if defined(_WIN32_WCE) && _MSC_VER < 1400
      #pragma warning(disable : 4291) // disable "no matching operator delete found"
   #endif

   // Define IE version based on WINVER version, if not defined
   #if !defined(_WIN32_WCE) && !defined(_WIN32_IE)
      #define _WIN32_IE WINVER
   #endif

   #if defined(_DEBUG) && defined(_MT) && defined(_WIN32_WCE)
      // Supress warnings when compiling std::vector. From MSDN (C4275 warning):
      //    C4275 can be ignored in Microsoft Visual C++ 2005 if you are deriving
      //    from a type in the Standard C++ Library, compiling a debug release (/MTd)
      //    and where the compiler error message refers to _Container_base.
      #pragma warning(disable:4275)
   #endif

#elif defined(__BORLANDC__) // Borland specific

   #include <dir.h>
   #include <conio.h>

   #if (__BORLANDC__ < 0x580) // pre-BDS versions
      #include <values.h>
   #endif

#elif defined (__GNUC__)
   #define M_GCC_VERSION (__GNUC__ * 10000 \
                        + __GNUC_MINOR__ * 100 \
                        + __GNUC_PATCHLEVEL__)
#endif

#include "MeteringSDKMacros.h"


#if M_USE_MFC
   #if defined _WIN32_WCE
      // Supress this message:
      //    _CE_ACTIVEX was not defined because this Windows CE SDK does not have DCOM.
      //    _CE_ACTIVEX could be caused to be defined by defining _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA, but it is recommended that this be done only for single-threaded apps.
      //    _ATL_NO_HOSTING was defined because _CE_ACTIVEX was not defined.
      // Discussion of this message: http://blogs.msdn.com/jeffabraham/archive/2005/10/10/479390.aspx
      // MeteringSDK does not depend on COM threading support, so it should be safe to ignore this message.
      #define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA // suppress warnings about lack of DCOM support on CE
   #endif
   // avoid 'operator delete(void *,int,char const *,int) already defined' errors
   // when linking with nafxcwd.lib (MFC static libs)
   #ifdef _DLL
      #ifndef _AFXDLL
         #define _AFXDLL
      #endif
   #endif
   #include <afx.h>
#endif

// Standard include files

#if !(M_OS & M_OS_WIN32_CE) && !(M_OS & M_OS_CMX)
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>

   // There is no share.h on UNIX, nor on CE
   #if !(M_OS & M_OS_POSIX)
      #include <share.h>
   #endif
#endif

#if !(M_OS & M_OS_NUTTX) && !(M_OS & M_OS_CMX)
   #include <memory.h>
   #include <malloc.h>
   #include <locale.h>
   #include <wchar.h>
#endif
#include <time.h>
#if !(M_OS & M_OS_CMX)
   #include <assert.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#if (M_OS & M_OS_WINDOWS)
   // Windows specific files
   #define WIN32_LEAN_AND_MEAN
   #define _WIN32_DCOM 1
   #include <windows.h>
   #include <wincrypt.h>

   #include <winsock2.h>
   #include <ws2tcpip.h>
   #if !defined(__BORLANDC__) && !defined(_WIN32_WCE)
      #include <wspiapi.h>
   #endif

   #if !defined(_WIN32_WCE)
      #include <io.h>
      #include <process.h>
      #include <conio.h>

      #if defined(_MSC_VER)
          #include <eh.h>   // structured exception handling
      #endif
   #endif

   #if defined(__MINGW32__)
      #include <new> // attach bad_alloc exception ... For some reason VC++
                     // doesn't throw exception in case when new cannot allocate
                     // enough memory. Just returns NULL.
   #else
      #include <new.h>
   #endif
#elif (M_OS & M_OS_POSIX)
   // Unixes
   #include <unistd.h>
   #include <sys/time.h>
   #include <sys/socket.h>
   #include <arpa/inet.h>
   #include <netinet/in.h>
   #include <errno.h>
   #include <dirent.h>     // directory access, for MFildFile

   #if !(M_OS & M_OS_NUTTX)
      #include <netdb.h>
      #include <fnmatch.h> // file name matching
   #else
      struct lconv;
   #endif

   // for serial port access
   #include <termios.h>
   #include <sys/ioctl.h>

   #if (M_OS & M_OS_QNXNTO)
      #include <atomic.h>
   #endif

//   #if (M_OS & M_OS_CYGWIN)
//      #include <io.h>
//   #else
//      #include <sys/io.h>
//   #endif
#elif (M_OS & M_OS_CMX)
   struct lconv;
   #define assert(test)  ((void)0)
   #include <stdint.h>
   #include <stddef.h>
   typedef int ssize_t;
#endif

#ifndef M_USE_USTL
// STL files
#include <memory>
#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <stdexcept> // New standard requires <exception>, but all compilers support stdexcept
#include <queue>
#include <deque>
#include <typeinfo>
#include <limits>

#include <iostream>
#if !(M_OS & M_OS_WIN32_CE) && !(M_OS & M_OS_CMX)
   #include <iomanip>
   #include <fstream>
   #include <sstream>
#endif
#endif

#if defined(M_NO_JNI) && !M_NO_JNI
   #include <jni.h>    // Java native interface
#endif

// Support for debugging on Visual C++ when Bounds Checker is not present
//
#if defined(_MSC_VER) && (_MSC_VER < 1400) && !defined(__BOUNDSCHECKER__) && !(M_OS & M_OS_WIN32_CE)
   /// Enable debug version of new operator
   #define _CRTDBG_MAP_ALLOC
   #include <crtdbg.h>
#endif

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_MT) && (M_OS & M_OS_WIN32_CE)
   #pragma warning(default:4275)
#endif

#if !MCORE_PROJECT_COMPILING
// speed up compilation of other projects, which use this header as precompiled
    #include <MCORE/MCORE.h>
#endif

///@}
#endif
