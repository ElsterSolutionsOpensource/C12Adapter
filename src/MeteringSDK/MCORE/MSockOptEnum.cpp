// File MCORE/MSockOptEnum.cpp

#include "MCOREExtern.h"
#include "MSockOptEnum.h"

#if !M_NO_SOCKETS || !M_NO_SOCKETS_UDP

M_START_PROPERTIES(SockOptEnum)
   M_CLASS_ENUMERATION(SockOptEnum, SolSocket)
   M_CLASS_ENUMERATION(SockOptEnum, SoAcceptconn)
   M_CLASS_ENUMERATION(SockOptEnum, SoBroadcast)
   M_CLASS_ENUMERATION(SockOptEnum, SoDebug)
   M_CLASS_ENUMERATION(SockOptEnum, SoDontroute)
   M_CLASS_ENUMERATION(SockOptEnum, SoError)
   M_CLASS_ENUMERATION(SockOptEnum, SoKeepalive)
   M_CLASS_ENUMERATION(SockOptEnum, SoLinger)
   M_CLASS_ENUMERATION(SockOptEnum, SoRcvbuf)
   M_CLASS_ENUMERATION(SockOptEnum, SoReuseaddr)
   M_CLASS_ENUMERATION(SockOptEnum, SoRcvtimeo)
   M_CLASS_ENUMERATION(SockOptEnum, SoSndbuf)
   M_CLASS_ENUMERATION(SockOptEnum, SoSndtimeo)
   M_CLASS_ENUMERATION(SockOptEnum, IpprotoIpv6)
   M_CLASS_ENUMERATION(SockOptEnum, Ipv6Tclass)
   M_CLASS_ENUMERATION(SockOptEnum, Ipv6V6only)
M_START_METHODS(SockOptEnum)
M_END_CLASS(SockOptEnum, Object)

#endif
