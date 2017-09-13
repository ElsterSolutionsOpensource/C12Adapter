#ifndef MCORE_MCOREDEFS_H
#define MCORE_MCOREDEFS_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MCOREDefs.h
///
/// Basic definitions of library MCORE.
/// This file is included by every public include file of MCORE library.
///

#ifndef MCORE_MCOREEXTERN_H // do not include precompiled header twice
   #error "Use precompiled header file and put MCORE/MCOREExtern.h in it! For simple projects just start your CPP file with MCORE/MCOREExtern.h"
#endif

//
// User macros follow
//

///@{
/// Shall be 1 if a MeteringSDK dynamic library is to be used or built, while value zero is for static library.
///
/// The macro applies to all MeteringSDK libraries, while the client applications or libraries are free
/// to decide whether they shall be dynamic or static, as well as how this shall be defined or handled in code.
/// A dynamic client library can contain either static or dynamic version of MeteringSDK.
/// If MeteringSDK is built dynamically, C/C++ dynamic runtime shall also be linked,
/// while for static version either dynamic or static runtime linkage is possible.
///
/// If the value of this macro is not specified, dynamic MeteringSDK is assumed for Windows,
/// while static is for any other operating system.
///
#ifndef M_DYNAMIC
   #if (M_OS & M_OS_WINDOWS) != 0
      #define M_DYNAMIC 1
   #else
      #define M_DYNAMIC 0
   #endif
#endif
///@}

/// Copyright message of the whole product.
///
/// This define is typically made in configuration file MeteringSDKSettings.h
/// the code below creates the default value if it was not.
///
#ifndef M_PRODUCT_LEGAL_COPYRIGHT
   #define M_PRODUCT_LEGAL_COPYRIGHT "Copyright (c) 1997-2017 Elster Solutions"
#endif

/// Product version string.
///
/// This define is typically made in configuration file MeteringSDKSettings.h
/// the code below creates the default value if it was not.
///
#ifndef M_PRODUCT_VERSION
   #define M_PRODUCT_VERSION "1.0.0.0"
#endif

/// Flag that disables Reflection API.
///
/// When Reflection is enabled, default, every reflected class will be added into the executable
/// whether or not it is actually used. This creates a considerable overhead, however adds
/// the ability of manipulating classes dynamically at runtime.
///
#ifndef M_NO_REFLECTION
   #define M_NO_REFLECTION 0
#endif

/// Flag that disables extended Reflection API.
///
/// Extended reflection API provides extra information about parameters and service prototypes,
/// and it is auxiliary for most of the use cases. Also, some compilers have difficulties
/// while compiling the macros invented for handling this extra information.
///
/// The default value is the same as \ref M_NO_REFLECTION for all compilers other than
/// those of Borland/Inprise/Embarcadero, for which this macro is always set to zero.
///
#ifndef M_NO_FULL_REFLECTION
   #if defined(__BORLANDC__) || defined(M_DOXYGEN)
      #define M_NO_FULL_REFLECTION 1
   #else
      #define M_NO_FULL_REFLECTION M_NO_REFLECTION
   #endif
#endif

/// Enable or disable support for files and file system operations.
///
/// The default is have support for files and file system.
///
#ifndef M_NO_FILESYSTEM
   #define M_NO_FILESYSTEM 0
#endif


/// Flag that disables Multithreading.
///
/// When this flag is nonzero, support of multithreading will be disabled in MeteringSDK,
/// which removes a big set of classes and features, such as background communication.
///
#ifndef M_NO_MULTITHREADING
   #define M_NO_MULTITHREADING 0
#endif

/// Flag that disables Sockets.
///
/// This is useful when it is necessary for the library to be linked without socket support.
/// This disables many useful features such as monitor or socket channel.
/// The value of this flag is zero by default, enable socket support.
///
#ifndef M_NO_SOCKETS
   #define M_NO_SOCKETS 0
#endif

/// Flag that disables socket UDP interface.
///
/// When sockets are enabled, this define allows disabling SOCKS proxy support.
/// When sockets are disabled, the value of this define does not have any effect.
///
#ifndef M_NO_SOCKETS_UDP
   #define M_NO_SOCKETS_UDP M_NO_SOCKETS
#endif

/// Flag that disables SOCKS proxy of Sockets.
///
/// When sockets are enabled, this define allows disabling SOCKS proxy support.
/// When sockets are disabled, the value of this define does not have any effect.
///
#ifndef M_NO_SOCKETS_SOCKS
   #define M_NO_SOCKETS_SOCKS M_NO_SOCKETS
#endif

/// Flag that disables time functions.
///
/// This disables a set of classes and features related to time.
/// The flag is zero by default, time is enabled, and it is useful in rare cases when
/// time is not used, but the memory is such a precious resource that squeezing
/// a few extra bytes makes a lot of sense.
///
#ifndef M_NO_TIME
   #define M_NO_TIME 0
#endif

/// Flag that disables MVariant class.
///
/// This define is very restrictive, incompatible with many features of MeteringSDK,
/// but it saves lots of code space.
///
#ifndef M_NO_VARIANT
   #define M_NO_VARIANT 0
#elif M_NO_VARIANT && !M_NO_REFLECTION
   #error "Support of reflectin (M_NO_REFLECTION=0) requires MVariant (M_NO_VARIANT=0)"
#endif

/// Flag that disables MVariant class.
///
/// This define is very restrictive, incompatible with many features of MeteringSDK,
/// but it saves lots of code space.
///
#ifndef M_NO_PROGRESS_MONITOR
   #define M_NO_PROGRESS_MONITOR 0
#endif

/// Windows specific flag that disables COM/OLE and Automation.
///
/// It is zero, enabled by default for Windows, disabled for any other operating system.
/// Can be disabled with giving a nonzero value for things such as COM security issues.
///
#ifndef M_NO_AUTOMATION
   #if (M_OS & M_OS_WINDOWS) != 0 || defined(M_DOXYGEN)
      #define M_NO_AUTOMATION 0
   #else
      #define M_NO_AUTOMATION 1
   #endif
#endif

/// Flag that disables serial port support.
///
/// Enabled by default, zero value. Having it nonzero is handy for
/// cases when MeteringSDK has to be used for rare applications that do not handle serial port.
///
#ifndef M_NO_SERIAL_PORT
   #define M_NO_SERIAL_PORT 0
#endif

/// Flag that disables standard console streams.
///
/// Those are accessible through \ref MStreamFile::GetStdOut(), \ref MStreamFile::GetStdIn(), and \ref MStreamFile::GetStdErr().
/// Console is not supported in Windows CE operating system, and for this OS the default value of this macro is zero.
///
#ifndef M_NO_CONSOLE
   #define M_NO_CONSOLE 0
#endif

/// Flag that disables registry manipulation class used to access Windows registry.
///
/// By default, this is zero for Windows, and nonzero for every other operating system.
///
/// \see \ref MRegistry
///
#ifndef M_NO_REGISTRY
   #if (M_OS & M_OS_WINDOWS) != 0 || defined(M_DOXYGEN)
      #define M_NO_REGISTRY 0
   #else
      #define M_NO_REGISTRY 1
   #endif
#endif

/// Whether the UNICODE version of the library has to be built or used.
///
/// When UNICODE is enabled, UTF-8 representation of human readable strings is used on all platforms.
///
/// \see \ref M_NO_WCHAR_T - whether to include wchar_t handling (required for M_UNICODE builds).
///
#ifndef M_UNICODE
   #if defined(_UNICODE) || defined(UNICODE)
      #define M_UNICODE 1
   #else
      #define M_UNICODE 0
   #endif
#endif

/// Flag that disables all uses of type wchar_t, including unicode conversion functions and unicode strings.
///
/// Enabled by default with zero value, can be disabled when a plain char client application does not need any wchar_t support.
///
/// \see M_UNICODE - whether to use unicode strings primarily.
///
#ifndef M_NO_WCHAR_T
   #define M_NO_WCHAR_T 0
#elif M_NO_WCHAR_T && M_UNICODE
   #error "Unicode build requires M_NO_WCHAR_T enabled"
#elif M_NO_WCHAR_T && !M_NO_AUTOMATION
   #error "Automation requires M_NO_WCHAR_T enabled"
#endif

/// Flag that disables character encoding and decoding.
///
/// When this flag is nonzero, support of local character conversions
/// related to methods of \ref MStr class will be disabled.
///
/// \see \ref M_NO_WCHAR_T
///
#ifndef M_NO_ENCODING
   #define M_NO_ENCODING 0
#endif

/// Flag that disables MessageCatalog and related internationalization facilities.
///
/// Enabled by default with zero value, can be disabled when a client application does not handle international text.
///
/// \see \ref M_GLOBAL_MESSAGE_CATALOG_DOMAIN - string that identifies the domain of the message catalog.
///
#ifndef M_NO_MESSAGE_CATALOG
   #define M_NO_MESSAGE_CATALOG 0
#endif

/// Domain string for global message catalog.
///
/// \ifnot M_NO_MESSAGE_CATALOG
/// This macro has an effect only when \ref M_NO_MESSAGE_CATALOG is zero, message catalog is enabled.
/// This is a string to denote the message catalog of the application, "MeteringSDK" by default.
/// \endif
///
#ifndef M_GLOBAL_MESSAGE_CATALOG_DOMAIN
   #define M_GLOBAL_MESSAGE_CATALOG_DOMAIN "MeteringSDK"
#endif


/// Flag to enable or disable MDynamicLibrary class - wrapper for shared object (dll) API.
///
/// By default, dynamic library is enabled only for \ref M_DYNAMIC builds.
///
#ifndef M_NO_DYNAMIC_LIBRARY
   #if M_DYNAMIC || defined(M_DOXYGEN)
      #define M_NO_DYNAMIC_LIBRARY 0
   #else
      #define M_NO_DYNAMIC_LIBRARY 1
   #endif
#endif


/// No verbose error information significantly decreases the size of the executable image.
///
/// Zero by default, error information is verbose.
/// Defining it to one helps fit the executable image into a small memory device.
///
#ifndef M_NO_VERBOSE_ERROR_INFORMATION
   #define M_NO_VERBOSE_ERROR_INFORMATION 0
#endif


/// Whether or not to support XML classes.
///
/// By default, XML classes are present in MCORE, the macro value is zero.
///
#ifndef M_NO_XML
   #define M_NO_XML 0
#endif


/// Whether to disable LUA cooperative multitasking at Input-Output.
///
/// LUA Cooperative multitasking allows context switching at certain Input-Output operations.
///
#ifndef M_NO_LUA_COOPERATIVE_IO
    #define M_NO_LUA_COOPERATIVE_IO 1
#endif

/// Whether to use /dev/crypto device as available in Linux.
///
/// When M_USE_CRYPTODEV is enabled, \ref M_USE_OPENSSL and \ref M_USE_CRYPTOAPI should be `0`.
///
#ifndef M_USE_CRYPTODEV
   #define M_USE_CRYPTODEV 0
#elif M_USE_CRYPTODEV && (M_OS & M_OS_POSIX) == 0
   #error "M_USE_CRYPTODEV cannot be enabled as this is not a POSIX operating system"
#elif M_USE_CRYPTODEV && ((defined(M_USE_OPENSSL) && M_USE_OPENSSL) || (defined(M_USE_CRYPTOAPI) && M_USE_CRYPTOAPI))
   #error "M_USE_OPENSSL, M_USE_CRYPTODEV, and M_USE_CRYPTOAPI have incompatible values - only one of them can be nonzero"
#endif

/// Whether to use Microsoft Crypto API as available in Windows.
///
/// When M_USE_CRYPTOAPI is enabled, \ref M_USE_OPENSSL and \ref M_USE_CRYPTODEV should be `0`.
///
#ifndef M_USE_CRYPTOAPI
   #define M_USE_CRYPTOAPI 0
#elif M_USE_CRYPTOAPI && (M_OS & M_OS_WINDOWS) == 0
   #error "M_USE_CRYPTOAPI cannot be enabled as this is not Windows operating system"
#elif M_USE_CRYPTOAPI && ((defined(M_USE_CRYPTODEV) && M_USE_CRYPTODEV) || (defined(M_USE_OPENSSL) && M_USE_OPENSSL))
   #error "M_USE_OPENSSL, M_USE_CRYPTODEV, and M_USE_CRYPTOAPI have incompatible values - only one of them can be nonzero"
#endif

/// Whether to use Open SSL library.
///
/// When M_USE_OPENSSL is enabled, \ref M_USE_CRYPTOAPI and \ref M_USE_CRYPTODEV should be `0`.
///
#ifndef M_USE_OPENSSL
   #define M_USE_OPENSSL 0
#elif M_USE_OPENSSL && ((defined(M_USE_CRYPTODEV) && M_USE_CRYPTODEV) || (defined(M_USE_CRYPTOAPI) && M_USE_CRYPTOAPI))
   #error "M_USE_OPENSSL, M_USE_CRYPTODEV, and M_USE_CRYPTOAPI have incompatible values - only one of them can be nonzero"
#endif

/// Whether to use Java Native Interface.
/// Set it to zero only for Java related code such as JNI facades.
///
#ifndef M_NO_JNI
   #if (M_OS & M_OS_ANDROID) != 0 || defined(M_DOXYGEN)
      #define M_NO_JNI 0
   #else
      #define M_NO_JNI 1
   #endif
#endif


// End of user macros
//

/// This macro has to be set to zero when MCORE is built.
/// It is done in the make files. Default is use library, not build it.
///
#ifndef MCORE_PROJECT_COMPILING
   #define MCORE_PROJECT_COMPILING 0
#endif

// Compiler dependent way to define a class or function, which is to be exported
// or imported from/to a dynamic library.
//

/// \def M_EXPORTED_CLASS
/// Class declared with this macro before the class name will be exported from compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// Typically, this macro is used in a conditional define with M_IMPORTED_CLASS to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_IMPORTED_CLASS
/// \see M_EXPORTED_ABSTRACT_CLASS - can save some extra space in case of abstract objects by not generating virtual table
/// \see M_EXPORTED_TEMPLATE_CLASS - exporting templates

/// \def M_IMPORTED_CLASS
/// Class declared with this macro before the class name will be imported into compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// Typically, this macro is used in a conditional define with M_EXPORTED_CLASS to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_EXPORTED_CLASS
/// \see M_IMPORTED_ABSTRACT_CLASS - can save some extra space in case of abstract objects by not generating virtual table
/// \see M_IMPORTED_TEMPLATE_CLASS - importing templates

/// \def M_EXPORTED_ABSTRACT_CLASS
/// Abstract class declared with this macro before the class name will be exported from compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// If the macro is applied to non-abstract class, the result is unpredictable.
/// Typically, this macro is used in a conditional define with M_IMPORTED_ABSTRACT_CLASS to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_IMPORTED_ABSTRACT_CLASS
/// \see M_EXPORTED_CLASS - exporting non-abstract non-template classes
/// \see M_EXPORTED_TEMPLATE_CLASS - exporting templates

/// \def M_IMPORTED_ABSTRACT_CLASS
/// Abstract class declared with this macro before the class name will be imported into compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// If the macro is applied to non-abstract class, the result is unpredictable.
/// Typically, this macro is used in a conditional define with M_EXPORTED_ABSTRACT_CLASS to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_IMPORTED_ABSTRACT_CLASS
/// \see M_EXPORTED_CLASS - exporting non-abstract non-template classes
/// \see M_IMPORTED_TEMPLATE_CLASS - importing templates

/// \def M_EXPORTED_TEMPLATE_CLASS
/// Template class declared with this macro before the class name will be exported from compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// If the macro is applied to non-template class, the result is unpredictable.
/// Typically, this macro is used in a conditional define with M_IMPORTED_TEMPLATE_CLASS to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_IMPORTED_TEMPLATE_CLASS
/// \see M_EXPORTED_CLASS - exporting non-abstract non-template classes
/// \see M_EXPORTED_ABSTRACT_CLASS - exporting abstract classes

/// \def M_IMPORTED_TEMPLATE_CLASS
/// Template class declared with this macro before the class name will be imported into compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// If the macro is applied to non-template class, the result is unpredictable.
/// Typically, this macro is used in a conditional define with M_EXPORTED_TEMPLATE_CLASS to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_EXPORTED_TEMPLATE_CLASS
/// \see M_EXPORTED_CLASS - exporting non-abstract non-template classes
/// \see M_IMPORTED_ABSTRACT_CLASS - importing abstract classes

/// \def M_EXPORTED_FUNC
/// Function declared to be exported from the compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// Typically, this macro is used in a conditional define with M_IMPORTED_FUNC to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_IMPORTED_FUNC

/// \def M_IMPORTED_FUNC
/// Function declared to be exported from the compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// Typically, this macro is used in a conditional define with M_EXPORTED_FUNC to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_EXPORTED_FUNC

/// \def M_NORETURN_FUNC
/// Function declared to never return.
///
/// Typically, this is either a function that always throws an exception, or a function that calls exit().

/// \def M_EXPORTED_C_FUNC
/// Function with plain C calling convention declared to be exported from the compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// Typically, this macro is used in a conditional define with M_IMPORTED_C_FUNC to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_IMPORTED_C_FUNC

/// \def M_IMPORTED_C_FUNC
/// Function with plain C calling convention declared to be exported from the compilation unit.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.
/// Typically, this macro is used in a conditional define with M_EXPORTED_C_FUNC to declare
/// exported/imported classes in the header of compilation unit.
///
/// \see M_EXPORTED_C_FUNC

/// \def M_CDECL
/// Specifies that the non-exported function shall have plain C calling convention.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.

/// \def M_UNUSED_FIELD
/// Specifies that the field within class or record is not used.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.

/// \def M_UNUSED_TYPEDEF
/// Specifies that the type definition that follows this macro is not used.
///
/// The effect of the macro depends on the compiler, operating system, and compilation mode.

#if M_OS & M_OS_WINDOWS
   #if defined(_MSC_VER) || defined(__MINGW32__)
      #define M_EXPORTED_CLASS __declspec(dllexport)
      #define M_IMPORTED_CLASS __declspec(dllimport)
      #if defined(__MINGW32__)
         #define M_EXPORTED_ABSTRACT_CLASS    __declspec(dllexport)
         #define M_IMPORTED_ABSTRACT_CLASS    __declspec(dllimport)
      #else
         #define M_EXPORTED_ABSTRACT_CLASS    __declspec(dllexport) __declspec(novtable)
         #define M_IMPORTED_ABSTRACT_CLASS    __declspec(dllimport) __declspec(novtable)
      #endif
      #define M_EXPORTED_TEMPLATE_CLASS    class __declspec(dllexport)
      #define M_IMPORTED_TEMPLATE_CLASS    class __declspec(dllimport)
      #define M_EXPORTED_FUNC              __declspec(dllexport)
      #define M_IMPORTED_FUNC              __declspec(dllimport)
      #define M_NORETURN_FUNC              __declspec(noreturn)
      #define M_EXPORTED_C_FUNC            extern "C" __declspec(dllexport)
      #define M_IMPORTED_C_FUNC            extern "C" __declspec(dllimport)
      #define M_CDECL                      __cdecl
   #elif defined(__BORLANDC__)
      #define M_EXPORTED_CLASS             __declspec(dllexport)
      #define M_IMPORTED_CLASS             __declspec(dllimport)
      #define M_EXPORTED_ABSTRACT_CLASS    __declspec(dllexport) __declspec(novtable)
      #define M_IMPORTED_ABSTRACT_CLASS    __declspec(dllimport) __declspec(novtable)
      #define M_EXPORTED_TEMPLATE_CLASS
      #define M_IMPORTED_TEMPLATE_CLASS
      #define M_EXPORTED_FUNC              __declspec(dllexport)
      #define M_IMPORTED_FUNC              __declspec(dllimport)
      #define M_NORETURN_FUNC              __declspec(noreturn)
      #define M_EXPORTED_C_FUNC            extern "C" __declspec(dllexport)
      #define M_IMPORTED_C_FUNC            extern "C" __declspec(dllimport)
      #define M_CDECL
   #else               // others
      #define M_EXPORTED_CLASS             __export
      #define M_IMPORTED_CLASS             __import
      #define M_EXPORTED_ABSTRACT_CLASS    __export
      #define M_IMPORTED_ABSTRACT_CLASS    __import
      #define M_EXPORTED_TEMPLATE_CLASS
      #define M_IMPORTED_TEMPLATE_CLASS
      #define M_EXPORTED_FUNC              __export
      #define M_IMPORTED_FUNC              __import
      #define M_NORETURN_FUNC
      #define M_EXPORTED_C_FUNC            extern "C" __export
      #define M_IMPORTED_C_FUNC            extern "C" __import
      #define M_CDECL
   #endif
   #define M_UNUSED_FIELD
   #define M_UNUSED_TYPEDEF
#else
   #define M_EXPORTED_CLASS
   #define M_IMPORTED_CLASS
   #define M_EXPORTED_ABSTRACT_CLASS
   #define M_IMPORTED_ABSTRACT_CLASS
   #define M_EXPORTED_TEMPLATE_CLASS
   #define M_IMPORTED_TEMPLATE_CLASS
   #define M_EXPORTED_FUNC
   #define M_IMPORTED_FUNC
   #if defined(__clang__)
      #define M_NORETURN_FUNC  __attribute__((noreturn))
      #define M_UNUSED_FIELD   __attribute__((unused))
      #define M_UNUSED_TYPEDEF __attribute__((unused))
   #elif defined(__GNUC__)
      #define M_NORETURN_FUNC  __attribute__((noreturn))
      #define M_UNUSED_FIELD
      #define M_UNUSED_TYPEDEF __attribute__((unused))
   #else
      #define M_NORETURN_FUNC
      #define M_UNUSED_FIELD
      #define M_UNUSED_TYPEDEF
   #endif
   #define M_EXPORTED_C_FUNC            extern "C"
   #define M_IMPORTED_C_FUNC            extern "C"
   #define M_CDECL
#endif

/// Macro that tells whether the current architecture is little endian or big endian.
/// Intel processors are little endian, Sun Sparcs and Motorolas are big endian.
///
#ifndef M_LITTLE_ENDIAN
   #define M_LITTLE_ENDIAN 1
#endif

/// Portable debug macro.
///
/// Usage of this macro from the client programs of MCORE is based on the assumption
/// that if the client is compiled with the debugging information on, it has to be
/// linked with the MCORE library, which also has debugging enabled.
///
#ifndef M_DEBUG
   #ifndef NDEBUG
      #define M_DEBUG 1
   #else
      #define M_DEBUG 0
   #endif
#endif

#if (M_OS & M_OS_WINDOWS) != 0
   /// Windows Active Codepage for unicode to multibyte translation.
   ///
   /// Redefines the standard CP_ACP, which serves the same purpose, but cannot be set to UTF8.
   /// When \ref M_UNICODE is nonzero, sets to use CP_UTF8.
   ///
   #if M_UNICODE
       #define M_WINDOWS_CP_ACP CP_UTF8
   #else
       #define M_WINDOWS_CP_ACP CP_ACP
   #endif
#endif

// Define compiler and OS-independent POSIX-like functions
//
#include <MCORE/MCOREFuncs.h>

/// \cond SHOW_INTERNAL

// Macro which allows making some MCORE classes and functions
// exportable and importable from/to DLL in Windows.
// Only non-template classes are defined M_CLASS since
// only those can be meaningfully shared.
// Redefine this stuff for various modes of MS Windows
//
#if M_DYNAMIC != 0
   #if MCORE_PROJECT_COMPILING
      #define M_CLASS           M_EXPORTED_CLASS
      #define M_ABSTRACT_CLASS  M_EXPORTED_ABSTRACT_CLASS
      #define M_TEMPLATE_CLASS  M_EXPORTED_TEMPLATE_CLASS
      #define M_FUNC            M_EXPORTED_FUNC
      #define M_C_FUNC          M_EXPORTED_C_FUNC
   #else
      #define M_CLASS           M_IMPORTED_CLASS
      #define M_ABSTRACT_CLASS  M_IMPORTED_ABSTRACT_CLASS
      #define M_TEMPLATE_CLASS  M_IMPORTED_TEMPLATE_CLASS
      #define M_FUNC            M_IMPORTED_FUNC
      #define M_C_FUNC          M_IMPORTED_C_FUNC
   #endif
#else
   #define M_CLASS
   #define M_ABSTRACT_CLASS
   #define M_TEMPLATE_CLASS
   #define M_FUNC
   #define M_C_FUNC
#endif

/// \endcond

/// Macro M_NEW can be used in client applications in places
/// where new is called without parameters.
/// The debugging version allows looking at the lines
/// in the source code where the memory leak took place.
///
/// Internally MCORE uses this macro in places of all memory
/// allocations where operator new would be used.
///
/// In its file MCOREDefs.cpp, MCORE also has code that
/// invokes memory leak dumps for the debugging version
/// at application exit. Any other DLL or the executable
/// file might overwrite this flag. This is the case
/// when the expected memory leaks are not dumped...
///
#if defined(M_DEBUG) && M_DEBUG && defined(_MSC_VER) && !defined(__BOUNDSCHECKER__) && ((M_OS & M_OS_WIN32_CE) == 0)
   #if M_USE_MFC
      #define M_NEW DEBUG_NEW
   #else
      #define M_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
   #endif
#else
   #define M_NEW new
#endif

/// Overloadable ASSERT macro, compatible with C language.
/// The library uses this version of assertion in places where the condition can still happen
/// at runtime at the extreme low probability.
/// There are also \ref M_ENSURED_ASSERT and \ref M_COMPILED_ASSERT.
///
#ifndef M_ASSERT
   #if M_DEBUG
      #if (M_OS & M_OS_WIN32_CE) != 0
         #define M_ASSERT(e) _ASSERT(e)   // assert() does not work on CE platform

      #elif defined(__BCPLUSPLUS__) // enhance debugging under C++Builder

         M_FUNC bool AssertMaybeRetry(bool bExpr, const char* sExpr, const char* sFile, int nLine, const char* sFunc );
         #define M_ASSERT(e) \
            do {  if ( ! AssertMaybeRetry( (e), #e, __FILE__, __LINE__, __FUNC__ ) ) break; \
                  ::DebugBreak(); \
               }  while (1)
            //Note: do..while structure allows using the macro with trailing ';'
            // and correct CPU stepping with breakpoints
      #else
         #define M_ASSERT(e)  assert(e)
      #endif
   #else
      #define M_ASSERT(e)  ((void)0)
   #endif
#endif

///@{
/// Version of ASSERT macro, which tells that the given condition is best ensured to be true at runtime.
///
/// This converts into instructions to optimizer which can lead to considerable speed and size improvement.
/// There are also \ref M_ASSERT and \ref M_COMPILED_ASSERT.
///
#ifndef M_ENSURED_ASSERT
   #if (!defined(M_DEBUG) || !M_DEBUG) && defined(_MSC_VER)
      #if (M_OS & M_OS_WIN32_CE) == 0
         #define M_ENSURED_ASSERT(e)  __assume(e)
      #else
         #define M_ENSURED_ASSERT(e)  // CE compiler seems to have problems with __assume(0) - see Galatea CR#7933
      #endif
   #elif defined(__clang__) || (defined(__GNUC__) && M_GCC_VERSION >= 40500) // GCC since 4.5
      #define M_ENSURED_ASSERT(e)  if ( e ) (void)0; else __builtin_unreachable()
   #else
      #define M_ENSURED_ASSERT(e)  M_ASSERT(e)
   #endif
#endif
///@}

///@{
/// Version of ASSERT macro, which works at compile time.
/// This way, the given expression shall be constant, known at compile time.
/// M_COMPILED_ASSERT can be used not only in the execution code, but also in every place where a declaration is possible.
/// There are also M_ASSERT and M_ENSURED_ASSERT.
///
/// The other feature of compiled assert is that it exists in both debug and release version,
/// since there is no extra overhead caused by its presence.
///
/// When the compiled assertion is failing, there will be an error message of some sort at compile time.
/// Many implementations will show an error that the array bound in type definition is negative.
/// There will also be an error if an attempt to have a variable expression but not constant one is made.
///
#ifndef M_COMPILED_ASSERT
   #if  (__cplusplus > 199711) || (defined(_MSC_VER) && _MSC_VER >= 1600)   // if this is C++11 or later or this is Visual C++ 2010 or later
      #define M_COMPILED_ASSERT(e) static_assert((e), "Compile time assertion failed: " M_MACRO_STRING(e))
   #else
      #define M_COMPILED_ASSERT(e) typedef int M_MACRO_CONCATENATE(__MStaticAssertionFault, __LINE__) [ (e) ? 1 : -1 ] M_UNUSED_TYPEDEF
   #endif
#endif
///@}

///@{
/// Deprecated declaration marker.
///
/// Use it to define methods that need to be reviewed or replaced.
///
#if defined(_MSC_VER)
   #define M_DEPRECATED(text) __declspec(deprecated(text))
#elif defined(__GNUC__)
   #define M_DEPRECATED(text) __attribute__((deprecated))
#else
   #define M_DEPRECATED(text)
#endif
///@}

///@{
/// Tells whether the processor can handle unaligned data.
///
/// If this is nonzero all data manipulation will be made only on properly aligned data.
/// For example, integers will have to be located on an address divisible by four for a value to be taken,
/// a double precision number will locate on an address divisible by eight and so on.
/// Defining this to zero can give some minor boost in performance.
///
/// If this value is not defined a guess is attempted based on processor architecture.
/// Currently only Intel allows unaligned data. Possible C preprocessor defines are taken from
/// http://sourceforge.net/p/predef/wiki/Architectures/
///
#ifndef M_NO_UNALIGNED_DATA
   #if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__i386__) || defined(__IA32__)
      #define M_NO_UNALIGNED_DATA 0
   #else
      #define M_NO_UNALIGNED_DATA 1
   #endif
#endif
///@}

/// Macro that tells the bit size of a pointer.
///
/// Note that it is assumed that unsigned integer and plain integer have the same size.
///
#ifndef M_POINTER_BIT_SIZE
   #if defined(__WORDSIZE)
      #define M_POINTER_BIT_SIZE (__WORDSIZE)
   #elif UINT_MAX == SIZE_MAX
      #define M_POINTER_BIT_SIZE 32
   #else
      #define M_POINTER_BIT_SIZE 64
   #endif
#endif

/// Macro that tells the bit size of an unsigned long value.
///
/// On the known 32-bit operating systems unsigned long and long are always 32 bits.
/// On the known 64-bit operating systems Windows has a 32-bit unsigned long
/// while all the other OSes such as Linux have it 64-bits.
///
#ifndef M_UNSIGNED_LONG_BIT_SIZE
   #if ULONG_MAX == 0xFFFFFFFF
      #define M_UNSIGNED_LONG_BIT_SIZE 32
   #else
      #define M_UNSIGNED_LONG_BIT_SIZE 64
   #endif
#endif

/// Macro that tells the bit size of wchar_t.
///
#if !defined(M_WCHAR_T_BIT_SIZE) && !M_NO_WCHAR_T
   #if (M_OS & M_OS_WINDOWS) != 0
      #define M_WCHAR_T_BIT_SIZE 16
   #else // any *NIX such as Linux or iOS
      #define M_WCHAR_T_BIT_SIZE 32
   #endif
#endif

/// Macro that tells the bit size of time_t.
///
#if !defined(M_TIME_T_BIT_SIZE)
   #if defined(_MSC_VER)
      #if defined(_USE_32BIT_TIME_T) || (M_OS & M_OS_WIN32_CE) != 0
         #define M_TIME_T_BIT_SIZE 32
      #else
         #define M_TIME_T_BIT_SIZE 64
      #endif
   #else
      #define M_TIME_T_BIT_SIZE  M_POINTER_BIT_SIZE
   #endif
#endif

/// \cond SHOW_INTERNAL
M_COMPILED_ASSERT(sizeof(size_t) * 8 == M_POINTER_BIT_SIZE && sizeof(void*) * 8 == M_POINTER_BIT_SIZE);
M_COMPILED_ASSERT(sizeof(time_t) * 8 == M_TIME_T_BIT_SIZE);
#if !M_NO_WCHAR_T
   M_COMPILED_ASSERT(sizeof(wchar_t) * 8 == M_WCHAR_T_BIT_SIZE);
#endif
/// \endcond SHOW_INTERNAL

//@{
/// Compiler and operating system independent unsigned integer types.
///
#if defined(_MSC_VER) || defined(__BORLANDC__)
   #define MUINT64C(cnst)          cnst ## UI64
   #define MINT64C(cnst)           cnst ## I64
   typedef unsigned __int64        Muint64;
   typedef __int64                 Mint64;
   typedef unsigned                Muint32;
   typedef int                     Mint32;
   typedef unsigned short          Muint16;
   typedef short                   Mint16;
   typedef unsigned char           Muint8;
   typedef signed char             Mint8;
   typedef UINT_PTR                Muintptr;
#elif defined(__GNUC__) || defined(__GNUG__)
   #define MUINT64C(cnst)          cnst ## ULL
   #define MINT64C(cnst)           cnst ## LL
   typedef unsigned long long      Muint64;
   typedef long long               Mint64;
   typedef unsigned                Muint32;
   typedef int                     Mint32;
   typedef unsigned short          Muint16;
   typedef short                   Mint16;
   typedef unsigned char           Muint8;
   typedef signed char             Mint8;
   typedef uintptr_t               Muintptr;

   #if !M_NO_WCHAR_T
      /// wstring is undefined under GCC
      ///
      typedef std::basic_string <wchar_t> wstring;
   #endif
#elif defined(ewarm)
   #define MUINT64C(cnst)          cnst ## ULL
   #define MINT64C(cnst)           cnst ## LL
   typedef unsigned long long      Muint64;
   typedef long long               Mint64;
   typedef unsigned                Muint32;
   typedef int                     Mint32;
   typedef unsigned short          Muint16;
   typedef short                   Mint16;
   typedef unsigned char           Muint8;
   typedef signed char             Mint8;
   typedef uintptr_t               Muintptr;
#else
   #error "Please code the OS dependent types..."
#endif
//@}

/// Macro for calculation of size of statically allocated arrays.
///
/// The array given as parameter should be defined with explicit subscripts, and the declaration has to be visible.
/// The parameter should not be just a pointer to the array.
/// Otherwise the macro expands into a compile error.
///
/// The implementation is tricky, and it was borrowed from Chromium project.
///
#define M_NUMBER_OF_ARRAY_ELEMENTS(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/// Compiler dependent way of instantiating templates.
/// Note that templates with multiple parameters are not supported.
///
#if (defined(__BORLANDC__) || defined(_MSC_VER)) && M_DYNAMIC
   #define M_DEFINE_TEMPLATE_CLASS(t) template class t;
#else
   #define M_DEFINE_TEMPLATE_CLASS(t)  // no need to do anything otherwise
#endif

/// Declare that the given variable is used, possibly compiler-dependent.
/// The parameter shall be a single variable name.
///
#define M_USED_VARIABLE(v)  (void)(v)

/// Define no-throw clause for a function, compiler-dependent.
///
#if defined(_MSC_VER) || defined(__GNUC__)
   #define M_NO_THROW   throw()
#else // Borland, others
   #define M_NO_THROW
#endif

/// Declare that the function parameter is a printf format.
///
/// This macro helps find bad format uses using gcc compiler
///
#ifdef __GNUC__
   #define M_PRINTF_FORMAT(f, a) __attribute__((format(printf, (f), (a))))
#else
   #define M_PRINTF_FORMAT(f, a)
#endif

/// Get the OS platform code, determined at runtime.
/// (opposite to compile-defined M_OS_ masks determined at compile time).
///
/// At present, the call will always return the value M_OS as it was known at compile time.
///
M_FUNC unsigned MGetRuntimeOSMask();

/// A define that always expands to a long char string prefix L for a previously expanded parameter.
/// If the parameter is a macro that resolves into a string.
/// Use it as follows: M_LL(__FILE__).
/// Without the above helper defines it would not be possible to pre-expand a __FILE__ macro.
///
/// \see M_L for conditional define of UNICODE prefix for any macro or string based on UNICODE compatibility mode.
///
#define M_LL(str) M_MACRO_CONCATENATE(L, str)

/// Legacy define that is used for strings, in this version it expands into string itself.
///
/// Also, see M_LL for unconditional define of UNICODE prefix for any macro or string.
///
#define M_L(str) str

/// Pointer to zero terminated character array that represents a readable text.
///
typedef char* MChars;

/// Pointer to zero terminated constant character array that represents a readable text.
///
/// This type definition is to distinguish from plain const char*
/// that can stand for an array of raw bytes.
///
typedef const char* MConstChars;

/// Type for a single human readable character, legacy define.
///
/// This type is discouraged from usage as a single char will not be able to represent
/// a locale-sensitive non-English readable character. Always use a string
/// for locale dependent human readable text.
///
typedef char MChar;

/// Type for single unsigned character.
///
typedef unsigned char MUnsignedChar;

/// Pointer to zero-terminated constant ASCII characters that are to be localized.
/// This is typically an English text.
/// A unique signed character pointer is used so the compile errors can be seen at the case of misuse.
///
typedef const signed char* MConstLocalChars;

/// Byte string, used for byte buffer handling purpose.
/// Note that this is not the same as the readable character string.
/// It has usage of STL basic_string, but the underlying implementation
/// can be different in case the compiler does not support STL.
///
typedef std::string MByteString;

/// Plain standard character string in utf-8 encoding.
///
/// Note that this is not necessarily the same type as MByteString.
/// It has usage of STL basic_string, but the underlying implementation
/// can be different in case the compiler does not support STL.
/// Note that this is not necessarily the same type as MByteString,
/// and the direct assignments between them are not allowed.
///
typedef std::string  MStdString;

/// Legacy define, equivalent of MStdString.
///
/// \see MStdString
///
typedef std::string MAnsiString;

#if !M_NO_WCHAR_T
/// UNICODE-based wide character string.
/// It is possible that this type gets defined only in the UNICODE version.
/// It has usage of STL basic_string, but the underlying implementation
/// can be different in case the compiler does not support STL.
///
typedef std::wstring MWideString;
#endif

/// Vector of standard strings.
///
typedef std::vector<MStdString> MStdStringVector;

/// Vector of byte strings.
///
typedef std::vector<MByteString> MByteStringVector;

/// This define sets the OS-independent separator for directories.
///
#if M_OS & (M_OS_POSIX | M_OS_UNIX | M_OS_BSD)
   #define M_DIRECTORY_SEPARATOR   '/'
#else // DOS, Windows (VMS is not covered)
   #define M_DIRECTORY_SEPARATOR   '\\'
#endif

///@{
/// Legacy conversion macros, please expand in your source code.
///
#define MAnsiToStdString(s) (s)
#define MStdToAnsiString(s) (s)
#define MByteToStdString(s) (s)
#define MStdToByteString(s) (s)
#define MToAnsiString       MToStdString

#if !M_NO_WCHAR_T
   #define MWideToStdString(s) MToStdString(s)
   #define MStdToWideString(s) MToWideString(s)
#endif

///@}

#if defined(__BORLANDC__)
   // Treat enumerations as integers.
   // Reason : BCB compiles DLL libraries with enums as chars where it is possible,
   // but in the EXE file the enumerations are always compiled as integers.
   // If you do not have this directive, you will get inconsistent code.
   //
   #pragma option -b
#endif

///@{
/// Set the DLL instance of the current library.
///
/// This service is used for initialization purpose only in case the current MeteringSDK
/// is a DLL library. In this case call it from DllMain of that library with the library
/// instance given as parameter.
///
#if (M_OS & M_OS_WINDOWS)
   M_FUNC void MSetDLLInstance(HINSTANCE inst) M_NO_THROW;
   M_FUNC HINSTANCE MGetDLLInstance() M_NO_THROW;
#endif
///@}

///@{
/// String formatter global function.
///
/// Format specification corresponds to standard library \c printf function, described as follows:
///
/// The length modifier:
///  - \c hh - the following integer conversion corresponds to a signed char
///            or unsigned  char argument, or a following n conversion
///            corresponds to a pointer to a signed char argument.
///  - \c h  - the following integer conversion corresponds to a short int
///            or unsigned  short int argument, or a following n conversion
///            corresponds to a pointer to a short int argument.
///  - \c l  - the following integer conversion corresponds to a long int
///            or unsigned long int argument, or a following n conversion
///            corresponds to a pointer to a long int argument, or a
///            following  c conversion  corresponds  to  a wchar_t argument,
///            or a following s conversion corresponds to a pointer to
///            \c wchar_t argument.
///  - \c ll - a following integer conversion corresponds to a long long int
///            or unsigned long long int argument, or a following n
///            conversion corresponds to a pointer to a long long int argument.
///  - \c q  - this is a synonym for \c ll.
///  - \c z  - the following integer conversion corresponds to a \c size_t or \c ssize_t argument.
///  - \c t  - the following integer conversion corresponds to a \c ptrdiff_t argument.
///
/// The conversion specifier:
///
///  - \c d, \c i - the int argument is converted to signed decimal
///        notation. The precision, if any, gives the minimum number of
///        digits that must appear; if the converted value
///        requires fewer digits, it is padded on the left with zeros.
///        The default precision is \c 1. When \c 0 is printed with an
///        explicit precision 0, the output is empty.
///  - \c o, \c u, \c x, \c X - the unsigned int argument is converted to unsigned
///              octal (o), unsigned decimal (u), or unsigned
///              hexadecimal (x and X) notation. The letters abcdef
///              are used for x conversions; the letters ABCDEF are
///              used for X conversions. The precision, if any, gives
///              the minimum number of digits that must appear; if the
///              converted value requires fewer digits, it is
///              padded on the left with zeros. The default precision
///              is 1. When 0 is printed with an explicit precision 0,
///              the output is empty.
///  - \c e, \c E - the double argument is rounded and converted in the
///        style [-]d.ddde-/+dd where there is one digit before the
///        decimal-point character and the number of digits after it is
///        equal to the precision; if the precision is missing, it is
///        taken as 6; if the precision is zero, no decimal-point
///        character appears. An E conversion uses the letter E (rather
///        than e) to introduce the exponent. The exponent always
///        contains at least two digits; if the value is zero, the
///        exponent is 00.
///  - \c f, \c F - the double argument is rounded and converted to decimal
///        notation in the style [-]ddd.ddd, where the number of
///        digits after the decimal-point character is equal to the
///        precision specification. If the precision is missing, it is
///        taken as 6; if the precision is explicitly zero, no
///        decimal-point character appears. If a decimal point
///        appears, at least one digit appears before it.
///  - \c g, \c G - the double argument is converted in style f or e (or F or E
///        for G conversions). The precision specifies the number of
///        significant digits. If the precision is missing, 6 digits
///        are given; if the precision is zero, it is treated as 1.
///        Style e is used if the exponent from its conversion is less
///        than -4 or greater than or equal to the precision.
///        Trailing zeros are removed from the fractional part of the
///        result; a decimal point appears only if it is followed by
///        at least one digit.
///  - \c c - if no \c l modifier is present, the int argument is converted to
///     an unsigned char, and the resulting character is written. If
///     an l modifier is present, the \c wint_t (wide character)
///     argument is converted to a multibyte sequence by a call to
///     the \c wcrtomb function, with a conversion state starting in the
///     initial state, and the resulting multibyte string is written.
///  - \c s - if an h modifier is present: The const char * argument is
///     expected to be a pointer to an array of character type (pointer
///     to a string). Characters from the array are written up to (but
///     not including) a terminating null byte ('\0'); if a precision
///     is specified, no more than the number specified are written. If
///     a precision is given, no null byte need be present; if the
///     precision is not specified, or is greater than the size of the
///     array, the array must contain a terminating null byte.
///
///     If an \c l modifier is present: The const wchar_t * argument is
///     expected to be a pointer to an array of wide characters. Wide
///     characters from the array are converted to multibyte characters
///     (each by a call to the \c wcrtomb function, with a conversion
///     state starting in the initial state before the first wide
///     character), up to and including a terminating null wide
///     character. The resulting multibyte characters are written up to
///     (but not including) the terminating null byte. If a precision
///     is specified, no more bytes than the number specified are
///     written, but no partial multibyte characters are written. Note
///     that the precision determines the number of bytes written, not
///     the number of wide characters or screen positions. The array
///     must contain a terminating null wide character, unless a
///     precision is given and it is so small that the number of bytes
///     written exceeds it before the end of the array is reached.
///
///  - \c p - the void* pointer argument is printed in hexadecimal (as if by "%#x" or "%#lx").
///
///  - \c n - the number of characters written so far is stored into the
///     integer indicated by the int * (or variant) pointer
///     argument. No argument is converted.
///
///  - \c C - When used with character buffer and format, specifies a
///     wide-character (wchar_t); when used with wide-character
///     buffer and format, specifies a single-byte character (char).
///
///  - \c S - When used with character buffer and format, specifies a
///     wide-character string (wchar_t*); when used with wide-character
///     buffer and format, specifies a single-byte character string (char*).
///
///  - \c % - a '%' is written.  No argument is converted. The complete
///     conversion specification is '%%'.
///
M_FUNC size_t MFormatVALc(char* buf, size_t size, const char* format, const lconv* lc, va_list args) M_NO_THROW;
M_FUNC size_t MFormatVA(char* buf, size_t size, const char* format, va_list args) M_NO_THROW;
M_FUNC size_t MFormatLc(char* buf, size_t size, const char* format, const lconv* lc, ...) M_NO_THROW;
M_FUNC size_t MFormat(char* buf, size_t size, const char* format, ...) M_NO_THROW;
#if !M_NO_WCHAR_T
M_FUNC size_t MFormatVALc(wchar_t* buf, size_t size, const wchar_t* format, const lconv* lc, va_list args) M_NO_THROW;
M_FUNC size_t MFormatVA(wchar_t* buf, size_t size, const wchar_t* format, va_list args) M_NO_THROW;
M_FUNC size_t MFormatLc(wchar_t* buf, size_t size, const wchar_t* format, const lconv* lc, ...) M_NO_THROW;
M_FUNC size_t MFormat(wchar_t* buf, size_t size, const wchar_t* format, ...) M_NO_THROW;
#endif
///@}

/// \cond SHOW_INTERNAL

/// Private functions
M_FUNC int MStringToSigned(const char* string, bool& sign, bool& overflow, const char** end);
M_FUNC unsigned MStringToUnsigned(const char* string, bool& sign, bool& overflow, const char** end);
M_FUNC Mint64 MStringToInt64(const char* string, bool& sign, bool& overflow, const char** end);
M_FUNC Muint64 MStringToUInt64(const char* string, bool& sign, bool& overflow, const char** end);
#if !M_NO_WCHAR_T
M_FUNC int MStringToSigned(const wchar_t* string, bool& sign, bool& overflow, const wchar_t** end);
M_FUNC unsigned MStringToUnsigned(const wchar_t* string, bool& sign, bool& overflow, const wchar_t** end);
M_FUNC Mint64 MStringToInt64(const wchar_t* string, bool& sign, bool& overflow, const wchar_t** end);
M_FUNC Muint64 MStringToUInt64(const wchar_t* string, bool& sign, bool& overflow, const wchar_t** end);
#endif

/// \endcond

///@{
/// Get the formatted string representation for given format with parameter expansion.
///
M_FUNC MStdString MGetStdString(MConstLocalChars str, ...) M_NO_THROW;
M_FUNC MStdString MGetStdString(MConstChars str, ...) M_NO_THROW;
///@}

///@{
/// Get the formatted string representation for given format with parameter expansion through va_list.
///
M_FUNC MStdString MGetStdStringVA(MConstLocalChars str, va_list va) M_NO_THROW;
M_FUNC MStdString MGetStdStringVA(MConstChars str, va_list va) M_NO_THROW;
///@}

///@{
/// Convert a character string to an integer number.
///
/// If the string starts with \c 0x, the number is considered to be hexadecimal.
/// If the string does not represent a correct integer number a conversion error is thrown.
///
/// \param str
///     String to convert to a number.
///
/// \return int - result of the conversion
///
M_FUNC int MToInt(const char* str);
inline int MToInt(const MByteString& str)
{
   return MToInt(str.c_str());
}
#if !M_NO_WCHAR_T
M_FUNC int MToInt(const wchar_t* str);
inline int MToInt(const MWideString& str)
{
   return MToInt(str.c_str());
}
#endif
///@}

///@{
/// Convert a character string to unsigned number.
///
/// If the string starts with \c 0x, the number is considered to be hexadecimal.
/// If the string does not represent a correct unsigned number a conversion error is thrown.
///
/// \param str
///     String to convert to a number.
///
/// \return unsigned - result of the conversion
///
M_FUNC unsigned MToUnsigned(const char* str);
inline unsigned MToUnsigned(const MByteString& str)
{
   return MToUnsigned(str.c_str());
}
#if !M_NO_WCHAR_T
M_FUNC unsigned MToUnsigned(const wchar_t* str);
inline unsigned MToUnsigned(const MWideString& str)
{
   return MToUnsigned(str.c_str());
}
#endif
///@}

///@{
/// Convert a character string to a 64-bit number.
///
/// If the string starts with \c 0x, the number is considered to be hexadecimal.
/// If the string does not represent a correct 64-bit number a conversion error is thrown.
///
/// \param str
///     String to convert to a number.
///
/// \return Mint64 - result of the conversion
///
M_FUNC Mint64 MToInt64(const char* str);
inline Mint64 MToInt64(const MByteString& str)
{
   return MToInt64(str.c_str());
}
#if !M_NO_WCHAR_T
M_FUNC Mint64 MToInt64(const wchar_t* str);
inline Mint64 MToInt64(const MWideString& str)
{
   return MToInt64(str.c_str());
}
#endif
///@}

///@{
/// Convert a character string to an unsigned 64-bit number.
///
/// If the string starts with \c 0x, the number is considered to be hexadecimal.
/// If the string does not represent a correct 64-bit number, a conversion error is thrown.
///
/// \param str
///     String to convert to an unsigned 64-bit number.
///
/// \return Muint64 - result of the conversion
///
M_FUNC Muint64 MToUInt64(const char* str);
inline Muint64 MToUInt64(const MByteString& str)
{
   return MToUInt64(str.c_str());
}
#if !M_NO_WCHAR_T
M_FUNC Muint64 MToUInt64(const wchar_t* str);
inline Muint64 MToUInt64(const MWideString& str)
{
   return MToUInt64(str.c_str());
}
#endif
///@}

///@{
/// Convert a character string to unsigned long number.
///
/// If the string starts with \c 0x, the number is considered to be hexadecimal.
/// If the string does not represent a correct unsigned long number a conversion error is thrown.
///
/// \param str
///     String to convert to a number.
///
/// \return unsigned long - result of the conversion
///
inline unsigned long MToUnsignedLong(const char* str)
{
#if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
   M_COMPILED_ASSERT(sizeof(int) == sizeof(unsigned long) && sizeof(unsigned long) == 4);
   return static_cast<unsigned long>(MToUnsigned(str));
#else // true 64-bit platform such as Linux
   M_COMPILED_ASSERT(sizeof(Muint64) == sizeof(unsigned long) && sizeof(unsigned long) == 8);
   return static_cast<unsigned long>(MToUInt64(str));
#endif
}
inline unsigned long MToUnsignedLong(const MByteString& str)
{
   return MToUnsignedLong(str.c_str());
}
#if !M_NO_WCHAR_T
inline unsigned long MToUnsignedLong(const wchar_t* str)
{
   #if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
      M_COMPILED_ASSERT(sizeof(int) == sizeof(unsigned long) && sizeof(unsigned long) == 4);
      return static_cast<unsigned long>(MToUnsigned(str));
   #else // true 64-bit platform such as Linux
      M_COMPILED_ASSERT(sizeof(Muint64) == sizeof(unsigned long) && sizeof(unsigned long) == 8);
      return static_cast<unsigned long>(MToUInt64(str));
   #endif
}
inline unsigned long MToUnsignedLong(const MWideString& str)
{
   return MToUnsignedLong(str.c_str());
}
#endif
///@}

///@{
/// Convert a character string to a long number.
///
/// If the string starts with \c 0x, the number is considered to be hexadecimal.
/// If the string does not represent a correct long number, a conversion error is thrown.
///
/// \param str
///     String to convert to a number.
///
/// \return long - result of the conversion
///
inline long MToLong(const char* str)
{
   #if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
      M_COMPILED_ASSERT(sizeof(int) == sizeof(long) && sizeof(long) == 4);
      return static_cast<long>(MToInt(str));
   #else // true 64-bit platform such as Linux
      M_COMPILED_ASSERT(sizeof(Mint64) == sizeof(long) && sizeof(long) == 8);
      return static_cast<long>(MToInt64(str));
   #endif
}
inline long MToLong(const MByteString& str)
{
   return MToLong(str.c_str());
}
#if !M_NO_WCHAR_T
inline long MToLong(const wchar_t* str)
{
#if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
   M_COMPILED_ASSERT(sizeof(int) == sizeof(long) && sizeof(long) == 4);
   return static_cast<long>(MToInt(str));
#else // true 64-bit platform such as Linux
   M_COMPILED_ASSERT(sizeof(Mint64) == sizeof(long) && sizeof(long) == 8);
   return static_cast<long>(MToInt64(str));
#endif
}
inline long MToLong(const MWideString& str)
{
   return MToLong(str.c_str());
}
#endif
///@}

///@{
/// Convert a character string to a double precision floating point number.
///
/// If the string starts with \c 0x, the number is considered to be hexadecimal.
/// Such hexadecimal number cannot have fraction or exponent.
/// If the string does not represent a correct double precision floating point number, a conversion error is thrown.
///
/// \param str
///     String to convert to a double precision floating point number.
///
/// \return double - result of the conversion
///
M_FUNC double MToDouble(const char* str);
inline double MToDouble(const MByteString& str)
{
   return MToDouble(str.c_str());
}
#if !M_NO_WCHAR_T
M_FUNC double MToDouble(const wchar_t* str);
inline double MToDouble(const MWideString& str)
{
   return MToDouble(str.c_str());
}
#endif
///@}

///@{
/// Convert the long value to a characters string, using the given buffer.
///
/// No exceptions can be thrown, but the supplied buffer has to have at least 16 characters.
///
M_FUNC char* MToChars(Mint64 value, char* buff) M_NO_THROW;
#if !M_NO_WCHAR_T
M_FUNC wchar_t* MToChars(Mint64 value, wchar_t* buff) M_NO_THROW;
#endif
///@}

///@{
/// Convert the unsigned long value to a characters string, using the given buffer.
///
/// No exceptions can be thrown, but the supplied buffer has to have at least 16 characters.
///
M_FUNC char* MToChars(Muint64 value, char* buff) M_NO_THROW;
#if !M_NO_WCHAR_T
M_FUNC wchar_t* MToChars(Muint64 value, wchar_t* buff) M_NO_THROW;
#endif
///@}

///@{
/// Convert the int value to a characters string, using the given buffer.
///
/// No exceptions can be thrown, but the supplied buffer has to have at least 16 characters.
///
M_FUNC char* MToChars(int value, char* buff) M_NO_THROW;
M_FUNC char* MToChars(unsigned value, char* buff) M_NO_THROW;
#if !M_NO_WCHAR_T
M_FUNC wchar_t* MToChars(int value, wchar_t* buff) M_NO_THROW;
M_FUNC wchar_t* MToChars(unsigned value, wchar_t* buff) M_NO_THROW;
#endif
///@}

///@{
/// Convert the long value to a characters string, using the given buffer.
///
/// No exceptions can be thrown, but the supplied buffer has to have at least 16 characters.
///
inline char* MToChars(long value, char* buff) M_NO_THROW
{
#if M_POINTER_BIT_SIZE == 32
   return MToChars(static_cast<int>(value), buff);
#else
   return MToChars(static_cast<Mint64>(value), buff);
#endif
}
#if !M_NO_WCHAR_T
inline wchar_t* MToChars(long value, wchar_t* buff) M_NO_THROW
{
#if M_POINTER_BIT_SIZE == 32
   return MToChars(static_cast<int>(value), buff);
#else
   return MToChars(static_cast<Mint64>(value), buff);
#endif
}
#endif
///@}

///@{
/// Convert the unsigned long value to a characters string, using the given buffer.
///
/// No exceptions can be thrown, but the supplied buffer has to have at least 16 characters.
///
inline char* MToChars(unsigned long value, char* buff) M_NO_THROW
{
#if M_POINTER_BIT_SIZE == 32
   return MToChars(static_cast<unsigned>(value), buff);
#else
   return MToChars(static_cast<Muint64>(value), buff);
#endif
}
#if !M_NO_WCHAR_T
inline wchar_t* MToChars(unsigned long value, wchar_t* buff) M_NO_THROW
{
#if M_POINTER_BIT_SIZE == 32
   return MToChars(static_cast<unsigned>(value), buff);
#else
   return MToChars(static_cast<Muint64>(value), buff);
#endif
}
#endif
///@}

///@{
/// Convert the double precision value to a characters string, using the given buffer.
///
/// \param value The number to convert.
/// \param buff Destination buffer of at least 24 bytes long.
/// \param shortestFormat If \c false, ".0" will always be appended to whole numbers, this is the default.
///                       If \c true, if the number is whole, it will not have a decimal separator followed by zero.
/// \param precision Precision of the number representation.
///                  The value 14, default, guarantees absence of rounding artefacts.
///                  Precision 17 is the maximum, and the result number can look like 1.00000000000000001.
///                  Precisions above 17 are equivalent to 17.
///
M_FUNC char* MToChars(double value, char* buff, bool shortestFormat = false, unsigned precision = 14) M_NO_THROW;
#if !M_NO_WCHAR_T
M_FUNC wchar_t* MToChars(double value, wchar_t* buff, bool shortestFormat = false, unsigned precision = 14) M_NO_THROW;
#endif
///@}

///@{
/// Convenience special version of \c strnlen that works in all supported platforms.
///
/// This is a multiplatform version of POSIX call \c strnlen.
///
M_FUNC size_t Mstrnlen(const char* string, size_t maxlen) M_NO_THROW;
#if !M_NO_WCHAR_T
M_FUNC size_t Mstrnlen(const wchar_t* string, size_t maxlen) M_NO_THROW;
#endif
///@}

/// Convert the long value to a standard string.
///
/// Memory exceptions can be thrown in case no memory is available,
/// otherwise any long value can be represented as a string.
///
M_FUNC MStdString MToStdString(Mint64 value);

/// Convert the unsigned long value to a standard string.
///
/// Memory exceptions can be thrown in case no memory is available,
/// otherwise any unsigned long value can be represented as a string.
///
M_FUNC MStdString MToStdString(Muint64 value);

/// Convert the long value to a string standard string.
///
/// Memory exceptions can be thrown in case no memory is available,
/// otherwise any integer value can be represented as a string.
///
M_FUNC MStdString MToStdString(int value);

/// Convert the unsigned long value to a standard string.
///
/// Memory exceptions can be thrown in case no memory is available,
/// otherwise any unsigned value can be represented as a string.
///
M_FUNC MStdString MToStdString(unsigned value);

/// Convert the long value to a standard string.
///
/// Memory exceptions can be thrown in case no memory is available,
/// otherwise any long value can be represented as a string.
///
inline MStdString MToStdString(long value)
{
#if M_POINTER_BIT_SIZE == 32
   return MToStdString(static_cast<int>(value));
#else
   return MToStdString(static_cast<Mint64>(value));
#endif
}

/// Convert the unsigned long value to a standard string.
///
/// Memory exceptions can be thrown in case no memory is available,
/// otherwise any unsigned long value can be represented as a string.
///
inline MStdString MToStdString(unsigned long value)
{
#if M_POINTER_BIT_SIZE == 32
   return MToStdString(static_cast<unsigned>(value));
#else
   return MToStdString(static_cast<Muint64>(value));
#endif
}

/// Convert the double precision floating point value to string.
///
/// Memory exceptions can be thrown in case no memory is available,
/// otherwise any double precision floating point value can be represented as a string.
///
/// \param value The number to convert.
/// \param shortestFormat If \c false, ".0" will always be appended to whole numbers, this is the default.
///                       If \c true, if the number is whole, it will not have a decimal separator followed by zero.
/// \param precision Precision of the number representation.
///                  The value 14, default, guarantees absence of rounding artefacts.
///                  Precision 17 is the maximum, and the result number can look like 1.00000000000000001.
///                  Precisions above 17 are equivalent to 17.
///
M_FUNC MStdString MToStdString(double value, bool shortestFormat = false, unsigned precision = 14);

/// Convert the byte string value to a string.
///
/// Memory exceptions can be thrown in case no memory is available.
/// In case of UNICODE, the conversion can yield many errors due to multibyte to UNICODE transformation.
///
inline const MStdString& MToStdString(const MByteString& s)
{
   return s;
}

#if !M_NO_WCHAR_T
/// Convert the zero terminated constant character pointer to the standard wide string according to the current locale.
///
/// Memory exceptions can be thrown in case no memory is available.
/// The conversion can yield many errors due to multibyte to UNICODE transformation.
///
M_FUNC MWideString MToWideString(const char* str);

/// Convert the zero terminated constant wide character pointer to the standard ANSI string according to the current locale.
///
/// Memory exceptions can be thrown in case no memory is available.
/// The conversion can yield many errors due to multibyte to UNICODE transformation.
///
M_FUNC MWideString MToWideString(const wchar_t* str);

/// Convert the constant character pointer to the standard wide string according to the current locale.
///
/// Memory exceptions can be thrown in case no memory is available.
/// The conversion can yield many errors due to multibyte to UNICODE transformation.
///
M_FUNC MWideString MToWideString(const char* buff, size_t size);

/// Convert the constant character pointer to the standard ANSI string according to the current locale.
///
/// Memory exceptions can be thrown in case no memory is available.
/// The conversion can yield many errors due to multibyte to UNICODE transformation.
///
M_FUNC MStdString MToStdString(const wchar_t* buff, size_t size);

/// Convert the standard ASCII string to the standard wide string according to the current locale.
///
/// Memory exceptions can be thrown in case no memory is available.
/// The conversion can yield many errors due to multibyte to UNICODE transformation.
///
M_FUNC MWideString MToWideString(const MStdString&);

/// Convert the standard wide string to the standard ASCII string according to the current locale.
///
/// Memory exceptions can be thrown in case no memory is available.
/// In case certain characters are invalid in the current locale, the result
/// string could be unreadable.
///
M_FUNC MStdString MToStdString(const MWideString&);
#endif

/// Convert the constant character pointer to the standard string, whether that
/// is a UNICODE or plain character string.
/// If the given character pointer is NULL, an empty string is returned.
///
M_FUNC MStdString MToStdString(const char*);

/// Convert the constant character pointer and size to the standard string, whether that
/// is a UNICODE or plain character string.
///
M_FUNC MStdString MToStdString(const char*, size_t size);

#if !M_NO_WCHAR_T
/// Convert the constant wide character pointer to the standard string, whether that
/// is a UNICODE or plain character string.
/// If the given character pointer is NULL, an empty string is returned.
///
M_FUNC MStdString MToStdString(const wchar_t*);

/// Convert the constant wide character pointer and size to the standard string,
/// whether that is a UNICODE or plain character string.
///
M_FUNC MStdString MToStdString(const wchar_t*, size_t size);
#endif

/// Lite pseudo-encryption-decryption algorithm.
/// Applied twice on the buffer, it yields the same value.
/// The size shall be divisible by eight, and the source shall be aligned to double word boundary.
///
/// \pre The source and destination are valid pointers.
/// The size is divisible by eight.
/// Debug version has certain limited checking for
/// validity of this condition.
///
inline void MEncryptDecryptLite(char* dest, const char* src, unsigned size)
{
#ifdef __GNUC__
   M_ENSURED_ASSERT((reinterpret_cast<ssize_t>(src) & 0x3) == 0); // source must be aligned by DWORD boundary
#else
   M_ENSURED_ASSERT((reinterpret_cast<INT_PTR>(src) & 0x3) == 0); // source must be aligned by DWORD boundary
#endif

   M_ENSURED_ASSERT(size % 8 == 0); // size must be divisible by 8

   const Muint32* it    = reinterpret_cast<const Muint32*>(src);
   const Muint32* itEnd = reinterpret_cast<const Muint32*>(src + size);
   while ( it != itEnd )
   {
      #if M_LITTLE_ENDIAN
         Muint32 dword = *it++ ^ 0xE1DA7B35u; // encryption key is here, spread through the code
         *dest++ = (char)(dword);
         dword >>= 8;
         *dest++ = (char)(dword);
         dword >>= 8;
         *dest++ = (char)(dword);
         dword >>= 8;
         *dest++ = (char)(dword);
         dword = *it++ ^ 0x5E6F720Bu; // encryption key is here, spread through the code
         *dest++ = (char)(dword);
         dword >>= 8;
         *dest++ = (char)(dword);
         dword >>= 8;
         *dest++ = (char)(dword);
         dword >>= 8;
         *dest++ = (char)(dword);
      #else
         #error "The algorithm is not yet developed for big endian architectures"
      #endif
   }
}


/// Swap all bytes of 2-byte value.
///
inline Muint16 MSwapUINT16(unsigned val)
{
   M_ASSERT(val <= 0xFFFFu);
   return Muint16((val << 8) | (Muint16(val) >> 8));   // type conversion is needed for non-16 bit architectures!
}

/// Swap all bytes of 3-byte value, return it as 32-bit value due to computer architecture restriction.
///
inline Muint32 MSwapUINT24(unsigned val)
{
   M_ASSERT(val <= 0xFFFFFFu);
   return ((val & 0xFF) << 16) | (val & 0xFF00) |  ((val & 0xFF0000) >> 16);
}

/// Swap all bytes of 4-byte value.
///
inline Muint32 MSwapUINT32(Muint32 val)
{
   return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) | ((val & 0xFF0000) >> 8) |  ((val & 0xFF000000) >> 24);
}

/// Convert a two-byte value to a big endian value.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline Muint16 MToBigEndianUINT16(unsigned val)
{
   M_ASSERT(val <= 0xFFFFu);
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return Muint16((val << 8) | (Muint16(val) >> 8));   // type conversion is needed for non-16 bit architectures!
   #else
      return Muint16(val);
   #endif
}

/// Convert a two-byte value to a big endian value denoted by a given pointer.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline void MToBigEndianUINT16(unsigned val, void* twoBytes)
{
   M_ASSERT(val <= 0xFFFFu);
   unsigned char* v = (unsigned char*)twoBytes;
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      v[1] = Muint8(val);
      v[0] = Muint8(val >> 8);
   #else
      v[0] = Muint8(val);
      v[1] = Muint8(val >> 8);
   #endif
}

/// Convert a big endian two-byte value to the value which is handled by the
/// current computer architecture.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline unsigned MFromBigEndianUINT16(Muint16 val)
{
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return unsigned(Muint16(val << 8) | Muint8(val >> 8)); // type conversion is needed for non-16 bit architectures!
   #else
      return unsigned(val);
   #endif
}

/// Convert a big endian two-byte value given as its address
/// to the value which is handled by the current computer architecture.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline unsigned MFromBigEndianUINT16(const void* twoBytes)
{
   const unsigned char* v = (const unsigned char*)twoBytes;
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return (unsigned(v[0]) << 8) | unsigned(v[1]);
   #else
      return (unsigned(v[1]) << 8) | unsigned(v[0]);
   #endif
}

/// Convert a 32 byte unsigned integer value to a big endian value which is 24 bits long.
/// For the alignment reasons, the value through which this operation is handled is Muint32.
///
inline Muint32 MToBigEndianUINT24(Muint32 val)
{
   M_ASSERT(val <= 0xFFFFFFu);
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return ((val & 0xFF) << 16) | (val & 0xFF00) |  ((val & 0xFF0000) >> 16);
   #else
      return val;
   #endif
}

/// Convert a three-byte value to a big endian value denoted by a given pointer.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline void MToBigEndianUINT24(unsigned val, void* threeBytes)
{
   M_ASSERT(val <= 0xFFFFFFu);
   unsigned char* v = (unsigned char*)threeBytes;
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      v[2] = Muint8(val);
      v[1] = Muint8(val >> 8);
      v[0] = Muint8(val >> 16);
   #else
      v[0] = Muint8(val);
      v[1] = Muint8(val >> 8);
      v[2] = Muint8(val >> 16);
   #endif
}

/// Convert a big endian three-byte value to the value which is handled by the
/// current computer architecture.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline Muint32 MFromBigEndianUINT24(Muint32 val)
{
   M_ASSERT(val <= 0xFFFFFFu);
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return ((val & 0xFF) << 16) | (val & 0xFF00) |  ((val & 0xFF0000) >> 16);
   #else
      return val;
   #endif
}

/// Convert a big endian three-byte value given as byte string to the value which is handled by the
/// current computer architecture.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline Muint32 MFromBigEndianUINT24(const void* threeBytes)
{
   const unsigned char* v = (const unsigned char*)threeBytes;
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return (unsigned(v[0]) << 16) | (unsigned(v[1]) << 8) | unsigned(v[2]);
   #else
      return (unsigned(v[2]) << 16) | (unsigned(v[1]) << 8) | unsigned(v[0]);
   #endif
}

/// Convert a 32 byte unsigned integer value to a big endian 32-bit value.
///
inline Muint32 MToBigEndianUINT32(Muint32 val)
{
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) | ((val & 0xFF0000) >> 8) |  ((val & 0xFF000000) >> 24);
   #else
      return val;
   #endif
}

/// Convert a four-byte value to a big endian value denoted by a given pointer.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline void MToBigEndianUINT32(unsigned val, void* fourBytes)
{
   unsigned char* v = (unsigned char*)fourBytes;
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      v[3] = Muint8(val);
      v[2] = Muint8(val >> 8);
      v[1] = Muint8(val >> 16);
      v[0] = Muint8(val >> 24);
   #else
      v[0] = Muint8(val);
      v[1] = Muint8(val >> 8);
      v[2] = Muint8(val >> 16);
      v[3] = Muint8(val >> 24);
   #endif
}

/// Convert a big endian 4-byte value to the value which is handled by the
/// current computer architecture.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline Muint32 MFromBigEndianUINT32(Muint32 val)
{
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return ((val & 0xFF) << 24) | ((val & 0xFF00) << 8) | ((val & 0xFF0000) >> 8) |  ((val & 0xFF000000) >> 24);
   #else
      return val;
   #endif
}

/// Convert a big endian 4-byte value given as const void pointer according to the
/// current computer architecture.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline Muint32 MFromBigEndianUINT32(const void* fourBytes)
{
   const unsigned char* v = (const unsigned char*)fourBytes;
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return (unsigned(v[0]) << 24) | (unsigned(v[1]) << 16) | (unsigned(v[2]) << 8) | unsigned(v[3]);
   #else
      return (unsigned(v[3]) << 24) | (unsigned(v[2]) << 16) | (unsigned(v[1]) << 8) | unsigned(v[0]);
   #endif
}

/// Convert a two-byte value to a little endian value.
///
/// On SUN SPARK and the like, this service swaps the bytes, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it does nothing.
///
inline Muint16 MToLittleEndianUINT16(unsigned val)
{
   M_ASSERT(val <= 0xFFFFu);
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return Muint16(val);
   #else
      return Muint16((val << 8) | (Muint16(val) >> 8)); // type conversion is needed for non-16 bit architectures!
   #endif
}

/// Convert a two-byte value to a little endian value denoted by a given pointer.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline void MToLittleEndianUINT16(unsigned val, void* twoBytes)
{
   M_ASSERT(val <= 0xFFFFu);
   unsigned char* v = (unsigned char*)twoBytes;
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      v[0] = Muint8(val);
      v[1] = Muint8(val >> 8);
   #else
      v[1] = Muint8(val);
      v[0] = Muint8(val >> 8);
   #endif
}

/// Convert a two-byte little endian value to the value which is handled by the
/// current computer architecture.
///
/// On SUN SPARK and the like, this service swaps the bytes, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it does nothing.
///
inline unsigned MFromLittleEndianUINT16(Muint16 val)
{
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return unsigned(val);
   #else
      return unsigned(Muint16(val << 8) | (val >> 8)); // type conversion is needed for non-16 bit architectures!
   #endif
}

/// Convert a two-byte little endian value given as address
/// to the value which is handled by the current computer architecture.
///
/// On SUN SPARK and the like, this service swaps the bytes, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it does nothing.
///
inline unsigned MFromLittleEndianUINT16(const void* twoBytes)
{
   const unsigned char* v = static_cast<const unsigned char*>(twoBytes);
   #if M_LITTLE_ENDIAN // little to little
      return (unsigned(v[1]) << 8) | unsigned(v[0]);
   #else
      return (unsigned(v[0]) << 8) | unsigned(v[1]);
   #endif
}

/// Convert a 32 byte unsigned integer value to a little endian value which is 24 bits long.
/// For the alignment reasons, the value through which this operation is handled is Muint32.
///
inline Muint32 MToLittleEndianUINT24(Muint32 val)
{
   M_ASSERT(val <= 0xFFFFFFu);
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      return val;
   #else
      return ((val & 0xFF) << 16) | (val & 0xFF00) |  ((val & 0xFF0000) >> 16);
   #endif
}

/// Convert a four-byte value to a little endian value denoted by a given pointer.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline void MToLittleEndianUINT32(unsigned val, void* fourBytes)
{
   unsigned char* v = static_cast<unsigned char*>(fourBytes);
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      v[0] = Muint8(val);
      v[1] = Muint8(val >> 8);
      v[2] = Muint8(val >> 16);
      v[3] = Muint8(val >> 24);
   #else
      v[3] = Muint8(val);
      v[2] = Muint8(val >> 8);
      v[1] = Muint8(val >> 16);
      v[0] = Muint8(val >> 24);
   #endif
}

/// Convert a four-byte little endian value given as address
/// to the value which is handled by the current computer architecture.
///
/// On SUN SPARK and the like, this service swaps the bytes, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it does nothing.
///
inline unsigned MFromLittleEndianUINT32(const void* fourBytes)
{
   const unsigned char* v = static_cast<const unsigned char*>(fourBytes);
   #if M_LITTLE_ENDIAN // little to little...
      return (unsigned(v[3]) << 24) | (unsigned(v[2]) << 16) | (unsigned(v[1]) << 8) | unsigned(v[0]);
   #else
      return (unsigned(v[0]) << 24) | (unsigned(v[1]) << 16) | (unsigned(v[2]) << 8) | unsigned(v[3]);
   #endif
}

/// Convert an eight-byte value to a little endian value denoted by a given pointer.
///
/// On SUN SPARK and the like, this service does nothing, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it swaps the bytes.
///
inline void MToLittleEndianUINT64(Muint64 val, void* eightBytes)
{
   unsigned char* v = static_cast<unsigned char*>(eightBytes);
   #if M_LITTLE_ENDIAN // if the computer architecture is different...
      v[0] = Muint8(val);
      v[1] = Muint8(val >> 8);
      v[2] = Muint8(val >> 16);
      v[3] = Muint8(val >> 24);
      v[4] = Muint8(val >> 32);
      v[5] = Muint8(val >> 40);
      v[6] = Muint8(val >> 48);
      v[7] = Muint8(val >> 56);
   #else
      v[7] = Muint8(val);
      v[6] = Muint8(val >> 8);
      v[5] = Muint8(val >> 16);
      v[4] = Muint8(val >> 24);
      v[3] = Muint8(val >> 32);
      v[2] = Muint8(val >> 40);
      v[1] = Muint8(val >> 48);
      v[0] = Muint8(val >> 56);
   #endif
}

/// Convert an eight-byte little endian value given as address
/// to Muint64 value which is handled by the current computer architecture.
///
/// On SUN SPARK and the like, this service swaps the bytes, while on Intel x86, i64, ARM, 
/// and the other little endian architectures it does nothing.
///
inline Muint64 MFromLittleEndianUINT64(const void* eightBytes)
{
   const unsigned char* v = static_cast<const unsigned char*>(eightBytes);
   #if M_LITTLE_ENDIAN // little to little...
      return (Muint64(v[7]) << 56) | (Muint64(v[6]) << 48) | (Muint64(v[5]) << 40) | (Muint64(v[4]) << 32) ||
             (Muint64(v[3]) << 24) | (Muint64(v[2]) << 16) | (Muint64(v[1]) << 8) | Muint64(v[0]);
   #else
      return (Muint64(v[0]) << 56) | (Muint64(v[1]) << 48) | (Muint64(v[2]) << 40) | (Muint64(v[3]) << 32) ||
             (Muint64(v[4]) << 24) | (Muint64(v[5]) << 16) | (Muint64(v[6]) << 8) | Muint64(v[7]);
   #endif
}

/// Convert two bytes in a pointer, given as pointer, into a 16 byte unsigned integer value.
/// The pointer could be unaligned.
///
inline Muint16 MToAlignedUINT16(const void* twoBytes)
{
   const unsigned char* v = static_cast<const unsigned char*>(twoBytes);
   return Muint16(v[0] | (v[1] << 8));
}

/// Convert two bytes in a pointer, given as pointer. into a 16 byte unsigned integer value.
/// The pointer could be unaligned.
///
inline Muint32 MToAlignedUINT32(const void* fourBytes)
{
   const unsigned char* v = static_cast<const unsigned char*>(fourBytes);
   return Muint32(v[0]) | Muint32(v[1] << 8) | Muint32(v[2] << 16) | Muint32(v[3] << 24);
}

/// Add a directory separator if it is not at the end of the path (directory).
///
/// Like in UNIX, if the directory name is /tmp/tools, the result path will be /tmp/tools/.
/// If it was /tmp/tools/ already, this function will do nothing.
/// If the directory is an empty string (assuming the current one), nothing is done too.
///
M_FUNC void MAddDirectorySeparatorIfNecessary(MStdString& directory);

/// Silent way of checking if a given pointer is not null.
///
/// This is to avoid newer C++ compiler behavior of assuming pointer to 'this'
/// and object references are never null.
///
/// The implementation might have to be reviewed for cases of aggressive whole-code
/// optimization which can look at the implementation of MPointerIsNull.
#if __cplusplus > 201100L
   M_FUNC bool MPointerIsNull(const void* ptr);
#else
   inline bool MPointerIsNull(const void* ptr)
   {
      return ptr == NULL;
   }
#endif

/// Silent way of checking if a given reference is actually a null pointer.
///
/// This is to avoid newer C++ compiler behavior of assuming pointer to 'this'
/// and object references are never null.
///
template
   <class Type>
bool MReferenceIsNull(const Type& ref)
{
   return MPointerIsNull((const void*)&ref);
}


/// \cond SHOW_INTERNAL

// Forward declarations:
//

class M_CLASS MObject;

#if !M_NO_REFLECTION
   class M_CLASS MPropertyDefinition;
   class M_CLASS MServiceDefinition;
#endif

#if !M_NO_VARIANT
   class M_CLASS MVariant;
#endif

class M_CLASS MDictionary;

#if !M_NO_FILESYSTEM
   class M_CLASS MCurrentPathSubstitutor;
   class M_CLASS MFindFile;
   class M_CLASS MIniFile;
#endif

class M_CLASS MStr;
class M_CLASS MTimer; // Timer is not part of time related set of classes
class M_CLASS MUtilities;
class M_CLASS MVersion;

class M_CLASS MErrorEnum;
class M_CLASS MException;
class M_CLASS MEIndexOutOfRange;
class M_CLASS MENumberOutOfRange;

class M_CLASS MRegexp;

#if !M_NO_PROGRESS_MONITOR
   class M_CLASS MProgressAction;
   class M_CLASS MProgressMonitor;
#endif

#if !M_NO_REFLECTION
   class M_CLASS MClass;
#endif

#if !M_NO_AUTOMATION
   class M_CLASS MAutomation;
#endif

#if !M_NO_REGISTRY
   class M_CLASS MRegistry;
#endif


#if !M_NO_TIME
   class M_CLASS MTimeSpan;
   class M_CLASS MTime;
   class M_CLASS MTimeZone;
#endif

#if !M_NO_MULTITHREADING
   class M_CLASS MThread;
   class M_CLASS MThreadCurrent;
   class M_CLASS MThreadWorker;
   class M_CLASS MSemaphore;
   class M_CLASS MCriticalSection;
   class M_CLASS MInterlocked;
   class M_CLASS MEvent;
#endif

#if !M_NO_MESSAGE_CATALOG
   class M_CLASS MMessageFile;
   class M_CLASS MMessageCatalog;
#endif

class M_CLASS MStream;
class M_CLASS MStreamMemory;
#if !M_NO_FILESYSTEM
   class M_CLASS MStreamFile;
#endif

#if !M_NO_SOCKETS
   class M_CLASS MStreamSocketBase;
   class M_CLASS MStreamSocketUdp;
   class M_CLASS MStreamSocket;
#endif

#if !M_NO_XML
   class M_CLASS MXmlNode;
   class M_CLASS MXmlDocument;
#endif


#if !M_NO_CONSOLE
   class M_CLASS MCommandLineParser;
#endif

#if !M_NO_JNI
   class M_CLASS MJavaEnv;
#endif

/// \endcond SHOW_INTERNAL

///@}
#endif
