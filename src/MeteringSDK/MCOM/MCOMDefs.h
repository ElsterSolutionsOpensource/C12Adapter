#ifndef MCOM_MCOMDEFS_H
#define MCOM_MCOMDEFS_H
/// \addtogroup MCOM
///@{
/// \file MCOM/MCOMDefs.h
///
/// Basic definitions of library MCOM.
/// This file is included by every public include file of MCOM library.
///

#include <MCORE/MCORE.h>

/// Default is use library, not build it.
///
#ifndef MCOM_PROJECT_COMPILING
    #define MCOM_PROJECT_COMPILING 0
#endif

/// Whether or not to add RAS Dial feature to socket channel, so the device is dialed prior to connecting.
/// Support for RAS Dial is only available on Windows for interactive GUI based applications.
///
#ifndef M_NO_MCOM_RAS_DIAL
   #define M_NO_MCOM_RAS_DIAL ((M_OS & M_OS_WINDOWS) == 0 || (M_OS & M_OS_WIN32_CE) == M_OS_WIN32_CE) // Windows, but not Windows CE
#elif !M_NO_MCOM_RAS_DIAL && (M_OS & M_OS_WINDOWS) == 0
   #error "MCOM: Support for RAS Dial is only available on Windows for interactive GUI based applications"
#endif

/// Whether or not to include MCOM Monitor feature
///
#ifndef M_NO_MCOM_MONITOR
   #define M_NO_MCOM_MONITOR (M_NO_SOCKETS || M_NO_TIME)
#elif !M_NO_MCOM_MONITOR && (M_NO_SOCKETS || M_NO_TIME)
   #error "MCOM: Support for the communications monitor requires support for the sockets and time"
#endif

/// Whether or not clients should use shared pointer to access to MCOM Monitor
///
#ifndef M_NO_MCOM_MONITOR_SHARED_POINTER
   #define M_NO_MCOM_MONITOR_SHARED_POINTER M_NO_MCOM_MONITOR
#endif

/// Whether or not to include syslog based monitor
///
/// By default syslog monitor is enabled for UNIX-like operating systems
/// when the generic monitor is enabled (\ref M_NO_MCOM_MONITOR = 0)
///
#ifndef M_NO_MCOM_MONITOR_SYSLOG
   #ifdef M_DOXYGEN
      #define M_NO_MCOM_MONITOR_SYSLOG 0
   #elif (M_OS & M_OS_LINUX) == 0 || (M_OS & M_OS_ANDROID) != 0
      #define M_NO_MCOM_MONITOR_SYSLOG 1
   #else
      #define M_NO_MCOM_MONITOR_SYSLOG M_NO_MCOM_MONITOR
   #endif
#endif

/// Whether or not to include MCOM ChannelSocket feature, include by default
///
#ifndef M_NO_MCOM_CHANNEL_SOCKET
   #define M_NO_MCOM_CHANNEL_SOCKET M_NO_SOCKETS
#elif !M_NO_MCOM_CHANNEL_SOCKET && M_NO_SOCKETS
   #error "MCOM: Socket channel needs sockets support to be enabled"
#endif

/// Whether or not to include MCOM ChannelSocket feature, include by default
///
#ifndef M_NO_MCOM_CHANNEL_SOCKET_UDP
   #define M_NO_MCOM_CHANNEL_SOCKET_UDP M_NO_SOCKETS_UDP
#elif !M_NO_MCOM_CHANNEL_SOCKET_UDP && M_NO_SOCKETS_UDP
   #error "MCOM: UDP Socket channel needs UDP sockets support to be enabled"
#endif

/// Whether or not to include special handling of FIN that is arriving to a socket channel from a peer socket
///
#ifndef M_NO_MCOM_HANDLE_PEER_DISCONNECT
   #define M_NO_MCOM_HANDLE_PEER_DISCONNECT (M_NO_MCOM_CHANNEL_SOCKET || M_NO_MULTITHREADING)
#elif !M_NO_MCOM_HANDLE_PEER_DISCONNECT && (M_NO_MCOM_CHANNEL_SOCKET || M_NO_MULTITHREADING)
   #error "MCOM: Handling of peer disconnect is a socket channel feature that requires multithreading"
#endif


/// Whether or not to include MCOM ChannelModem feature, include by default.
///
#ifndef M_NO_MCOM_CHANNEL_MODEM
   #define M_NO_MCOM_CHANNEL_MODEM (M_NO_TIME || M_NO_SERIAL_PORT)
#elif !M_NO_MCOM_CHANNEL_MODEM && (M_NO_TIME || M_NO_SERIAL_PORT)
   #error "MCOM: Modem channel needs time support to be enabled"
#endif

/// Whether or not to include ANSI C12.18 protocol support, class MProtocolC1218
///
#ifndef M_NO_MCOM_PROTOCOL_C1218
   #define M_NO_MCOM_PROTOCOL_C1218 0
#endif

/// Whether or not to include ANSI C12.21 protocol support, class MProtocolC1221
///
#ifndef M_NO_MCOM_PROTOCOL_C1221
   #define M_NO_MCOM_PROTOCOL_C1221 0
#elif !M_NO_MCOM_PROTOCOL_C1221 && M_NO_MCOM_PROTOCOL_C1218
   #error "MCOM: Either disable C12.21, or enable C12.18 protocol"
#endif

/// Whether or not to include ANSI C12.22 protocol support, class MProtocolC1222
///
#ifndef M_NO_MCOM_PROTOCOL_C1222
   #define M_NO_MCOM_PROTOCOL_C1222 0
#endif

/// Whether or not to include MCOM Factory feature.
///
#ifndef M_NO_MCOM_FACTORY
   #define M_NO_MCOM_FACTORY M_NO_REFLECTION
#elif !M_NO_MCOM_FACTORY && M_NO_REFLECTION
   #error "MCOM: MCOMFactory needs reflection support to be enabled"
#endif

/// Whether or not to include Command queue feature (Q commands). Include by default.
///
#ifndef M_NO_MCOM_COMMAND_QUEUE
   #define M_NO_MCOM_COMMAND_QUEUE 0
#elif M_NO_MCOM_COMMAND_QUEUE && !M_NO_MCOM_PROTOCOL_C1222
   #error "MCOM: ANSI C12.22 implementation requires command queue support"
#endif

/// Whether or not to have protocol threading capability. Include by default.
/// One can have it only if command queue and multithreading are both enabled.
///
#ifndef M_NO_MCOM_PROTOCOL_THREAD
   #define M_NO_MCOM_PROTOCOL_THREAD (M_NO_MCOM_COMMAND_QUEUE || M_NO_MULTITHREADING)
#elif !M_NO_MCOM_PROTOCOL_THREAD && (M_NO_MCOM_COMMAND_QUEUE || M_NO_MULTITHREADING)
   #error "MCOM: Protocol thread need multithreading and queueing enabled"
#endif

/// Whether or not to have support for KeepSessionAlive protocol property.
/// By default, the feature is included if multithreading is on.
///
#ifndef M_NO_MCOM_KEEP_SESSION_ALIVE
   #define M_NO_MCOM_KEEP_SESSION_ALIVE M_NO_MULTITHREADING
#elif M_NO_MCOM_KEEP_SESSION_ALIVE == 0 && M_NO_MULTITHREADING
   #error "MCOM: Keeping session alive requires support for multithreading"
#endif


/// Whether or not to have protocol method Protocol.IdentifyMeter and related.
///
/// By default, include Protocol.IdentifyMeter
///
#ifndef M_NO_MCOM_IDENTIFY_METER
   #define M_NO_MCOM_IDENTIFY_METER 0
#endif


/// \cond SHOW_INTERNAL

/// Macro which allows making some MCOM classes and functions
/// exportable and importable from/to DLL in Windows.
/// Only non-template classes are defined MCOM_CLASS since
/// only those can be meaningfully shared.
/// Redefine this stuff for various modes of MS Windows
///
#if M_DYNAMIC != 0
   #if MCOM_PROJECT_COMPILING
      #define MCOM_CLASS          M_EXPORTED_CLASS
      #define MCOM_ABSTRACT_CLASS M_EXPORTED_ABSTRACT_CLASS
      #define MCOM_TEMPLATE_CLASS M_EXPORTED_TEMPLATE_CLASS
      #define MCOM_FUNC           M_EXPORTED_FUNC
      #define MCOM_C_FUNC         M_EXPORTED_C_FUNC
   #else
      #define MCOM_CLASS          M_IMPORTED_CLASS
      #define MCOM_ABSTRACT_CLASS M_IMPORTED_ABSTRACT_CLASS
      #define MCOM_TEMPLATE_CLASS M_IMPORTED_TEMPLATE_CLASS
      #define MCOM_FUNC           M_IMPORTED_FUNC
      #define MCOM_C_FUNC         M_IMPORTED_C_FUNC
   #endif
#else
   #define MCOM_CLASS
   #define MCOM_ABSTRACT_CLASS
   #define MCOM_TEMPLATE_CLASS
   #define MCOM_FUNC
   #define MCOM_C_FUNC
#endif

/// \endcond SHOW_INTERNAL

///@{
/// Type for Table or Function number.
///
/// Depending on compilation mode, this is either MVariant or an unsigned.
///
#if !M_NO_VARIANT
   typedef MVariant MCOMNumber;
#else
   typedef unsigned MCOMNumber;
#endif
///@}

///@{
/// Type for constant reference Table or Function number, right-value.
///
/// Depending on compilation mode, this is either const MVariant& or an unsigned.
///
#if !M_NO_VARIANT
   typedef const MVariant& MCOMNumberConstRef;
#else
   typedef unsigned MCOMNumberConstRef;
#endif
///@}

#if !M_NO_MCOM_IDENTIFY_METER
   class MIdentifyString;
#endif

class MCOM_CLASS MChannel;

#if !M_NO_SERIAL_PORT
   class MCOM_CLASS MChannelSerialPort;
   class MCOM_CLASS MChannelCurrentLoop;
   class MCOM_CLASS MChannelOpticalProbe;
#endif

#if !M_NO_MCOM_CHANNEL_MODEM
   class MCOM_CLASS MChannelModem;
   class MCOM_CLASS MChannelModemCallback;
#endif

#if !M_NO_MCOM_CHANNEL_SOCKET
   class MCOM_CLASS MChannelSocketBase;
   class MCOM_CLASS MChannelSocket;
   class MCOM_CLASS MChannelSocketCallback;
#endif
#if !M_NO_MCOM_CHANNEL_SOCKET_UDP
   class MCOM_CLASS MChannelSocketUdp;
   class MCOM_CLASS MChannelSocketUdpCallback;
#endif

class MCOM_CLASS MProtocol;

#if !M_NO_MCOM_PROTOCOL_THREAD
   class MCOM_CLASS MProtocolThread;
#endif

#if !M_NO_MCOM_PROTOCOL_C1218
   class MCOM_CLASS MProtocolC12;
   class MCOM_CLASS MProtocolC1218;
#endif

#if !M_NO_MCOM_PROTOCOL_C1221
   class MCOM_CLASS MProtocolC1221;
#endif


#if !M_NO_MCOM_PROTOCOL_C1222
   class MCOM_CLASS MProtocolC12;
   class MCOM_CLASS MProtocolC1222;
#endif

#if !M_NO_MCOM_MONITOR
   class MCOM_CLASS MMonitor;
   class MCOM_CLASS MMonitorFile;
   class MCOM_CLASS MMonitorSocket;
   class MCOM_CLASS MLogFile;
   class MCOM_CLASS MLogFileWriter;
   class MCOM_CLASS MLogFileReader;
#endif

///@}
#endif
