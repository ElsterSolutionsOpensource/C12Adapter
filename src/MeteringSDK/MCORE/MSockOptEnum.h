#ifndef MCORE_MSOCKOPTENUM_H
#define MCORE_MSOCKOPTENUM_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MSockOptEnum.h

#include <MCORE/MObject.h>

#if !M_NO_SOCKETS || !M_NO_SOCKETS_UDP

/// \cond SHOW_INTERNAL

#ifndef IPV6_TCLASS        // relatively new definition
   #define IPV6_TCLASS ((M_OS & M_OS_WINDOWS) != 0 ? 39 : 67)
#elif ((M_OS & M_OS_WINDOWS) != 0 && IPV6_TCLASS != 39) || ((M_OS & M_OS_WINDOWS) == 0 && IPV6_TCLASS != 67)
   #error "Unexpected value of IPV6_TCLASS, please check"
#endif

/// \endcond SHOW_INTERNAL

/// Enumerations and constants for socket options.
///
/// These are used as parameters of \ref MStreamSocketBase::SetSockOpt,
/// \ref MStreamSocketBase::GetSockOpt, and \ref MStreamSocketBase::GetSockOptBytes.
///
/// C++ interface can use the system defined enumerations directly.
///
class M_CLASS MSockOptEnum : public MObject
{
public:

   /// Actual enumeration type
   ///
   enum Type
   {
      // Socket level options
      
      SolSocket           = SOL_SOCKET,            ///< Socket level value, first parameter of \ref MStreamSocketBase::GetSockOpt and \ref MStreamSocketBase::SetSockOpt
      SoAcceptconn        = SO_ACCEPTCONN,         ///< Get listening status of the socket, get only, integer 0 or -1
      SoBroadcast         = SO_BROADCAST,          ///< Configure for sending a broadcast, integer 0 or -1
      SoDebug             = SO_DEBUG,              ///< Debug mode, integer 0 or -1
      SoDontroute         = SO_DONTROUTE,          ///< Do not route, integer 0 or -1
      SoError             = SO_ERROR,              ///< Return socket error and clear error, get only, integer
      SoKeepalive         = SO_KEEPALIVE,          ///< Configure for keep alive packets, integer 0 or -1
      SoLinger            = SO_LINGER,             ///< Linger on closing the socket until all the data is sent, integer 0 or -1
      SoRcvbuf            = SO_RCVBUF,             ///< Integer size of receive buffer
      SoReuseaddr         = SO_REUSEADDR,          ///< If the socket is bound to an already bound address, reuse the address, 0 or -1
      SoRcvtimeo          = SO_RCVTIMEO,           ///< recv timeout in milliseconds for blocking mode
      SoSndbuf            = SO_SNDBUF,             ///< Integer size of send buffer
      SoSndtimeo          = SO_SNDTIMEO,           ///< send timeout in milliseconds for blocking mode

      // IPv6 options

      IpprotoIpv6          = IPPROTO_IPV6,          ///< IPv6 level value, first parameter of GetSockOpt/SetSockOpt
      Ipv6Tclass           = IPV6_TCLASS,           ///< IPv6 traffic class value, integer property
      Ipv6V6only           = IPV6_V6ONLY,           ///< Whether the socket is restricted to IPv6 communications only, integer 0 or -1
   };

public: // Constructors, destructor:

   /// Constructor that creates an unassigned enumeration type.
   ///
   MSockOptEnum()
   {
   }

   /// Constructor that initializes the object with the given value of type.
   ///
   MSockOptEnum(Type& type)
   :
      m_type(type)
   {
   }

   /// Constructor that copies an enumeration type from another MSockOptEnum value.
   ///
   MSockOptEnum(const MSockOptEnum& type)
   :
      m_type(type.m_type)
   {
   }

   /// Assignment operator that initializes an enumeration type with the value of another given type.
   ///
   MSockOptEnum& operator=(const MSockOptEnum& other)
   {
      m_type = other.m_type;
      return *this;
   }

   /// Checks for equal values of the type and another type.
   ///
   bool operator==(const MSockOptEnum& other) const
   {
      return m_type == other.m_type;
   }

   /// Checks for inequality of the type and another type.
   ///
   bool operator!=(const MSockOptEnum& other) const
   {
      return !operator==(other);
   }

private: // Data:

   // Type that holds the value of the enumeration
   //
   Type m_type;

   M_DECLARE_CLASS(SockOptEnum)
};

#endif
///@}
#endif
