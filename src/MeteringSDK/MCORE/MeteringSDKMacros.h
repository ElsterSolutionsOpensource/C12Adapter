#ifndef MCORE_METERINGSDKMACROS_H
#define MCORE_METERINGSDKMACROS_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MeteringSDKMacros.h
///
/// MeteringSDK generic macro definitions, including version and settings file inclusion.
///
/// This file gets included into soure code of many languages, such as C, C++, IDL and RC,
/// therefore, only generic preprocessor definitions should be here.
///


/// Helper macro that expands into its parameter
///
/// This macro is handy for making single literals out of non-literals or multiple literals, part of macro trickery.
///
#define M_MACRO_DUMMY(x) x

/// Take a parameter of the macro and make a string out of it.
///
/// Differing from the standard # prefix, this macro addresses the case if the given argument is a macro to be expanded.
///
#define M_MACRO_STRING(x)    M__DO_MACRO_STRING(x)

///@{
/// Helper macro that concatenates the pre-expanded parameters
///
/// This define makes possible to have __FILE__ or M_LL(__FILE__) in the source code, other defines of this kind.
/// Variable number of parameters are supported, from two to eight.
///
#define M_MACRO_CONCATENATE(a, b)                    M__DO_MACRO_CONCATENATE(a, b)
#define M_MACRO_CONCATENATE3(a, b, c)                M_MACRO_CONCATENATE(M_MACRO_CONCATENATE(a, b), c)
#define M_MACRO_CONCATENATE4(a, b, c, d)             M_MACRO_CONCATENATE(M_MACRO_CONCATENATE3(a, b, c), d)
#define M_MACRO_CONCATENATE5(a, b, c, d, e)          M_MACRO_CONCATENATE(M_MACRO_CONCATENATE4(a, b, c, d), e)
#define M_MACRO_CONCATENATE6(a, b, c, d, e, f)       M_MACRO_CONCATENATE(M_MACRO_CONCATENATE5(a, b, c, d, e), f)
#define M_MACRO_CONCATENATE7(a, b, c, d, e, f, g)    M_MACRO_CONCATENATE(M_MACRO_CONCATENATE6(a, b, c, d, e, f), g)
#define M_MACRO_CONCATENATE8(a, b, c, d, e, f, g, h) M_MACRO_CONCATENATE(M_MACRO_CONCATENATE7(a, b, c, d, e, f, g), h)
///@}


// MeteringSDK version information include
//
#include "MeteringSDKVersion.h"

/// MeteringSDK version string separated by dots, loadable into MVersion class
///
#define M_SDK_VERSION_STRING M_MACRO_STRING(M_MACRO_CONCATENATE7(M_SDK_VERSION_MAJOR, ., M_SDK_VERSION_MIDDLE, ., M_SDK_VERSION_MINOR, ., M_SDK_VERSION_TAG))


// User customizations include
//
#include <MeteringSDKSettings.h>

// Metercat macro support, ETM_BuildVerStr
#ifndef ETM_BuildVerStr
// Checks of MeteringSDKSettings.h consistency
#if !defined(M_PRODUCT_VERSION_MAJOR) || !defined(M_PRODUCT_VERSION_MIDDLE) || !defined(M_PRODUCT_VERSION_MINOR) || !defined(M_PRODUCT_VERSION_TAG)
   #error "Define M_PRODUCT_VERSION_MAJOR, M_PRODUCT_VERSION_MIDDLE, M_PRODUCT_VERSION_MINOR, and M_PRODUCT_VERSION_TAG in your MeteringSDKSettings.h so they compose your product version like 1.2.3.9876"
#endif
#endif

/// MeteringSDK file version literal, separated by commas, as used by Windows resource FILEVERSION
///
/// If file version is not found in MeteringSDKSettings.h configuration file, it is composed from 
/// mandatory macros M_PRODUCT_VERSION_MAJOR, M_PRODUCT_VERSION_MIDDLE, M_PRODUCT_VERSION_MINOR, and M_PRODUCT_VERSION_TAG.
///
#ifndef M_FILE_VERSION
   #define M_FILE_VERSION M_PRODUCT_VERSION_MAJOR,M_PRODUCT_VERSION_MIDDLE,M_PRODUCT_VERSION_MINOR,M_PRODUCT_VERSION_TAG
#endif

/// Product version string, can be defined in the configuration file or composed from entities there.
///
/// If product version is not found in MeteringSDKSettings.h configuration file, it is composed from 
/// mandatory macros M_PRODUCT_VERSION_MAJOR, M_PRODUCT_VERSION_MIDDLE, M_PRODUCT_VERSION_MINOR, and M_PRODUCT_VERSION_TAG.
///
#ifndef M_PRODUCT_VERSION_STRING
   #define M_PRODUCT_VERSION_STRING  M_MACRO_STRING(M_MACRO_CONCATENATE7(M_PRODUCT_VERSION_MAJOR, ., M_PRODUCT_VERSION_MIDDLE, ., M_PRODUCT_VERSION_MINOR, ., M_PRODUCT_VERSION_TAG))
#endif

/// \cond SHOW_INTERNAL

// Private helper macros for preprocessor trickery
//
#define M__DO_MACRO_STRING(x) #x
#define M__DO_MACRO_CONCATENATE(a, b) M__DO_MACRO_CONCATENATE_2(a, b)
#define M__DO_MACRO_CONCATENATE_2(a, b) a##b

/// \endcond

///@}
#endif
