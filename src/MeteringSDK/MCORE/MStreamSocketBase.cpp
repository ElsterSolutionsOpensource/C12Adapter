// File MCORE/MStreamSocketBase.cpp

#include "MCOREExtern.h"
#include "MException.h"
#include "MStreamSocket.h"
#include "MUtilities.h"
#include "MCriticalSection.h"
#include "MAlgorithm.h"
#if !M_NO_LUA_COOPERATIVE_IO
#  include "LuaIO.h"
#endif

#if !M_NO_SOCKETS || !M_NO_SOCKETS_UDP

#if (M_OS & M_OS_WINDOWS) != 0
   #if _WIN32_WINNT >= 0x0600 // support of if_nametoindex() is added in Vista, 0x0600
      #include <netioapi.h>
      #include "private/ifaddrsWindows.h"
      #include "private/ifaddrsWindows.cxx"
   #endif
#elif (M_OS & M_OS_ANDROID) != 0
   #include <net/if.h>
   #include "private/ifaddrsAndroid.h"
   #include "private/ifaddrsAndroid.cxx"
#else
   #include <ifaddrs.h>
   #include <net/if.h>
   #include <sys/ioctl.h>
#endif

namespace
{

#if (M_OS & M_OS_WINDOWS) != 0
   M_COMPILED_ASSERT(MStreamSocketBase::InvalidSocket == INVALID_SOCKET);
   M_COMPILED_ASSERT(MStreamSocketBase::SocketErrorStatus == SOCKET_ERROR);

   #ifdef _MSC_VER // if we are under Microsoft, link with the sockets library
      #ifndef _WIN32_WCE
         #pragma comment(lib, "ws2_32.lib")
      #else
         #pragma comment(lib, "ws2.lib")
      #endif // !_WIN32_WCE
   #endif

   // Service name must be a numeric port number (value taken from Platform SDK)
   #ifndef AI_NUMERICSERV
      #define AI_NUMERICSERV 0x00000008
   #endif

class WSAInitializer
{
public:
   static MCriticalSection s_lock;

   WSAInitializer();
   ~WSAInitializer();
};

MCriticalSection WSAInitializer::s_lock;

WSAInitializer::WSAInitializer()
{
   WORD requiredVersion = MAKEWORD(2, 0);
   WSADATA wsaData;
   if ( ::WSAStartup(requiredVersion, &wsaData) != 0 )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
}

WSAInitializer::~WSAInitializer()
{
   ::WSACleanup();
}

void OSInitSocketsLibrary()
{
   MCriticalSection::Locker locker(WSAInitializer::s_lock);
   static WSAInitializer instance; // Meyers' singleton
}

#else  // ( M_OS & M_OS_WINDOWS )

   #if !M_NO_SOCKETS
      M_COMPILED_ASSERT(MStreamSocketBase::InvalidSocket == -1);
      M_COMPILED_ASSERT(MStreamSocketBase::SocketErrorStatus == -1);
   #endif

inline void OSInitSocketsLibrary()
{
}

#endif // ( M_OS & M_OS_WINDOWS )

} // namespace

#if !M_NO_SOCKETS

   M_LINK_THE_CLASS_IN(MSockOptEnum)

M_START_PROPERTIES(StreamSocketBase)
   M_OBJECT_PROPERTY_READONLY_STRING    (StreamSocketBase, LocalSocketName,    ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_UINT      (StreamSocketBase, LocalSocketPort)
   M_CLASS_PROPERTY_READONLY_STRING     (StreamSocketBase, LocalName,          ST_MStdString_S)
   M_CLASS_PROPERTY_READONLY_STRING     (StreamSocketBase, LocalAddress,       ST_MStdString_S)
   M_OBJECT_PROPERTY_READONLY_STRING    (StreamSocketBase, PeerSocketName,     ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_UINT      (StreamSocketBase, PeerSocketPort)
   M_OBJECT_PROPERTY_UINT               (StreamSocketBase, ReceiveTimeout)
   M_OBJECT_PROPERTY_READONLY_UINT      (StreamSocketBase, BytesReadyToRead)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT(StreamSocketBase, IsInputBufferEmpty)
M_START_METHODS(StreamSocketBase)
   M_OBJECT_SERVICE_OVERLOADED          (StreamSocketBase, Bind,               Bind,                2, ST_X_unsigned_constMStdStringA)
   M_OBJECT_SERVICE_OVERLOADED          (StreamSocketBase, Bind,               DoBind1,             1, ST_X_unsigned)  // SWIG_HIDE
   M_OBJECT_SERVICE                     (StreamSocketBase, WaitToReceive,                              ST_bool_X_unsigned)
   M_OBJECT_SERVICE                     (StreamSocketBase, WaitToSend,                                 ST_bool_X_unsigned)
   M_CLASS_SERVICE                      (StreamSocketBase, AddressToBinary,                            ST_MByteString_S_constMStdStringA)
   M_CLASS_SERVICE                      (StreamSocketBase, BinaryToAddress,                            ST_MStdString_S_constMByteStringA)
   M_OBJECT_SERVICE                     (StreamSocketBase, ClearInputBuffer,                           ST_X)
   M_OBJECT_SERVICE                     (StreamSocketBase, GetSockOpt,                                 ST_int_X_int_int)
   M_OBJECT_SERVICE                     (StreamSocketBase, GetSockOptBytes,                          ST_MByteString_X_int_int_unsigned)
   M_OBJECT_SERVICE                     (StreamSocketBase, SetSockOpt,                                 ST_X_int_int_constMVariantA)
M_END_CLASS(StreamSocketBase, Stream)

MStreamSocketBase::OsAddrinfoHolder::~OsAddrinfoHolder()
{
   if ( m_pointer != NULL )
      freeaddrinfo(m_pointer);
}

MStreamSocketBase::OsSocketHandleHolder::~OsSocketHandleHolder()
{
   if ( m_socketHandle != MStreamSocketBase::InvalidSocket )
      MStreamSocketBase::DoOsClose(m_socketHandle);
}

MStreamSocketBase::MStreamSocketBase(SocketHandleType sockfd)
:
   m_socketHandle(InvalidSocket),
   m_receiveTimeout(TimeoutDefault)
{
   OSInitSocketsLibrary();
   SetSocketHandle(sockfd);
}

MStreamSocketBase::~MStreamSocketBase() M_NO_THROW
{
   M_ASSERT(m_socketHandle == MStreamSocketBase::InvalidSocket); // surely closed in child destructors already
}

void MStreamSocketBase::SetSocketHandle(SocketHandleType sockfd)
{
   Close();
   if ( sockfd != InvalidSocket )
   {
      DoStartOpen(FlagReadWrite);
      m_socketHandle = sockfd;
      DoFinishOpen();
   }
   else
      M_ASSERT(InvalidSocket == m_socketHandle);
}

bool MStreamSocketBase::DoIsOpenImpl() const M_NO_THROW
{
   return InvalidSocket != m_socketHandle;
}

MStdString MStreamSocketBase::GetLocalSocketName() const
{
   M_ASSERT(InvalidSocket != m_socketHandle);

   sockaddr_storage socketAddress;
   memset(&socketAddress, 0, sizeof(socketAddress));
   socketAddress.ss_family = AF_UNSPEC;
   socklen_t socketAddressLength = sizeof(socketAddress);
   DoOsGetsockname(m_socketHandle, (sockaddr*)&socketAddress, &socketAddressLength);

   char res[NI_MAXHOST];
   DoOsGetnameinfo((sockaddr*)&socketAddress, socketAddressLength, res, sizeof(res), NULL, 0, NI_NUMERICHOST);
   return MToStdString(res);
}

unsigned MStreamSocketBase::GetLocalSocketPort() const
{
   M_ASSERT(InvalidSocket != m_socketHandle);

   sockaddr_storage socketAddress;
   memset(&socketAddress, 0, sizeof(socketAddress));
   socketAddress.ss_family = AF_UNSPEC;
   socklen_t socketAddressLength = sizeof(socketAddress);
   DoOsGetsockname(m_socketHandle, (sockaddr*)&socketAddress, &socketAddressLength);

   char res[NI_MAXSERV];
   DoOsGetnameinfo((sockaddr*)&socketAddress, socketAddressLength, NULL, 0, res, sizeof(res), NI_NUMERICSERV);
   return MToUnsigned(res);
}

void MStreamSocketBase::Bind(unsigned port, const MStdString& address)
{
   Close();

   DoStartOpen(FlagReadWrite);

   struct addrinfo hints;
   memset(&hints, 0, sizeof(hints));
   if ( this->GetClass() == MStreamSocket::GetStaticClass() )
      hints.ai_socktype = SOCK_STREAM;
   else
      hints.ai_socktype = SOCK_DGRAM;

   hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE;

   try
   {
      hints.ai_family = (address.empty() || IsAddressLocalIPv4(address))
                      ? AF_INET
                      : AF_UNSPEC;

      const char* hostname = NULL;
      if ( !address.empty() )
         hostname = address.c_str();

      char servname[NI_MAXSERV];
      MFormat(servname, sizeof(servname), "%d", port);

      OsAddrinfoHolder aih;
      DoOsGetaddrinfo(hostname, servname, &hints, &aih.m_pointer);

      for ( struct addrinfo* ai = aih.m_pointer; ai != NULL; ai = ai->ai_next )
      {
         try
         {
            DoAdjustAddress(ai);

            OsSocketHandleHolder sh;
            sh.m_socketHandle = DoOsSocket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

            const int reuseaddr = 1;
            DoOsSetsockopt(sh.m_socketHandle, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

            const int res = ::bind(sh.m_socketHandle, ai->ai_addr, static_cast<socklen_t>(ai->ai_addrlen));
            if ( res < 0 )
            {
               MESocketError::ThrowLastSocketError();
               M_ENSURED_ASSERT(0);
            }

            m_socketHandle = sh.m_socketHandle;
            sh.m_socketHandle = InvalidSocket;
            break;
         }
         catch ( ... )
         {
            if ( ai->ai_next == NULL )
               throw;
         }
      }
   }
   catch ( ... )
   {
      Close();
      throw;
   }

   DoFinishOpen();

#ifdef SO_NOSIGPIPE // Non-linux POSIX systems, prevent signaling broken pipe when peer does not listen any more
   int setNoSigPipe = 1;
   DoOsSetsockopt(m_sockfd, SOL_SOCKET, SO_NOSIGPIPE, (void*)&setNoSigPipe, sizeof(setNoSigPipe));
#endif
}

void MStreamSocketBase::DoBind1(unsigned port)
{
   Bind(port);
}

bool MStreamSocketBase::WaitToReceive(unsigned timeout) const
{
   return DoNonblockingReceiveWait(m_socketHandle, timeout);
}

bool MStreamSocketBase::WaitToSend(unsigned timeout) const
{
   if ( InvalidSocket == m_socketHandle )
      return true;

#if !M_NO_LUA_COOPERATIVE_IO
   return MLuaYieldAndSelect(m_sockfd, timeout, 1 /*write*/);
#else
   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(m_socketHandle, &fds);
   bool yea = DoOsSelect(static_cast<int>(m_socketHandle) + 1, NULL, &fds, NULL, timeout) != 0;
   M_ASSERT(yea == (FD_ISSET(m_socketHandle, &fds) != 0));
   return yea;
#endif
}

MStdString MStreamSocketBase::GetLocalName()
{
   OSInitSocketsLibrary(); // static service can be called without constructing any socket

   char name[NI_MAXHOST];
   DoOsGethostname(name, static_cast<unsigned>(sizeof(name)));
   return MToStdString(name);
}

MStdString MStreamSocketBase::GetLocalAddress()
{
   OSInitSocketsLibrary(); // static service can be called without constructing any socket

   char name[NI_MAXHOST];
   DoOsGethostname(name, static_cast<unsigned>(sizeof(name)));
   OsAddrinfoHolder ai;
   DoOsGetaddrinfo(name, NULL, NULL, &ai.m_pointer);
   DoOsGetnameinfo(ai.m_pointer->ai_addr, static_cast<socklen_t>(ai.m_pointer->ai_addrlen), name, static_cast<socklen_t>(sizeof(name)), NULL, 0, NI_NUMERICHOST);
   return MToStdString(name);
}

bool MStreamSocketBase::IsInputBufferEmpty() const
{
   return GetBytesReadyToRead() == 0;
}

M_NORETURN_FUNC void MStreamSocketBase::DoThrowEndOfStream()
{
   MESocketError::ThrowSocketReadTimeout();
}

void MStreamSocketBase::DoCloseImpl()
{
   if ( InvalidSocket != m_socketHandle )
   {
      DoSetNonBlocking(m_socketHandle, false); // perform close in blocking mode
      int res = DoOsClose(m_socketHandle);
      m_socketHandle = InvalidSocket; // do it prior to
      if ( res < 0 )
      {
         MESocketError::ThrowLastSocketError();
         M_ENSURED_ASSERT(0);
      }
   }
}

bool MStreamSocketBase::IsAddressLocalIPv4(const MStdString& address)  	
{  	
   return ( address == "localhost"  	
       || address == "127.0.0.1"  	
       || address == GetLocalName()  	  	 
       || address == GetLocalAddress() );  	  	 
}  	
	  	
bool MStreamSocketBase::IsAddressLocalIPv6(const MStdString& address)  	
{  	
   return ( address == "localhost"  	
       || address == "::1"  	
       || address == "0:0:0:0:0:0:0:1"  	
       || address == "::ffff:127.0.0.1" );  	  	 
} 

int MStreamSocketBase::GetSockOpt(int level, int option)
{
   int result;
   socklen_t resultSize = sizeof(result);
   DoOsGetsockopt(m_socketHandle, level, option, &result, &resultSize);
   return result;
}

MByteString MStreamSocketBase::GetSockOptBytes(int level, int option, unsigned bufferSize)
{
   MByteString result(bufferSize, '\0');
   socklen_t resultSize = static_cast<socklen_t>(result.size());
   DoOsGetsockopt(m_socketHandle, level, option, &(result[0]), &resultSize);
   return result;
}

void MStreamSocketBase::SetSockOpt(int level, int option, const MVariant& value)
{
   if ( value.IsNumeric() )
   {
      Muint32 v = value.AsDWord();
      DoOsSetsockopt(m_socketHandle, level, option, &v, sizeof(v));
   }
   else
   {
      const MSharedString& v = value.AsSharedString();
      DoOsSetsockopt(m_socketHandle, level, option, v.data(), v.size());
   }
}

MStreamSocketBase::SocketHandleType MStreamSocketBase::DoOsSocket(int domain, int type, int protocol)
{
   const MStreamSocketBase::SocketHandleType res = ::socket(domain, type, protocol);
   if ( MStreamSocketBase::InvalidSocket == res )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return res;
}

int MStreamSocketBase::DoOsSelect(int nfds, fd_set* rfds, fd_set* wfds, fd_set* efds, unsigned ms)
{
   struct timeval tvl, *ptvl = NULL;
   if ( ms != (unsigned)-1 )
   {
      tvl.tv_sec  = (ms / 1000u);
      tvl.tv_usec = (ms % 1000u) * 1000u;
      ptvl = &tvl;
   }
#if M_OS & M_OS_POSIX
BEGIN:
#endif
   const int res = ::select(nfds, rfds, wfds, efds, ptvl);
   if ( MStreamSocketBase::SocketErrorStatus == res )
   {
#if M_OS & M_OS_POSIX
      if ( EINTR == errno )
         goto BEGIN;
#endif
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return res;
}

int MStreamSocketBase::DoOsClose(SocketHandleType sockfd)
{
   int res;
#if M_OS & M_OS_POSIX
   do
   {
      res = ::close(sockfd);
   } while ( MStreamSocketBase::SocketErrorStatus == res && EINTR == errno );
#else // Windows
   res = ::closesocket(sockfd);
#endif
   return res;
}

int MStreamSocketBase::DoOsIoctl(SocketHandleType sockfd, unsigned long cmd, unsigned long* argp)
{
#if M_OS & M_OS_POSIX
   const int res = ::ioctl(sockfd, cmd, argp);
#else
   const int res = ::ioctlsocket(sockfd, cmd, argp);
#endif
   if ( MStreamSocketBase::SocketErrorStatus == res )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return res;
}

void MStreamSocketBase::DoOsGetsockopt(SocketHandleType sockfd, int level, int optname, void* optval, socklen_t* optlen)
{
   const int res = ::getsockopt(sockfd, level, optname, static_cast<char*>(optval), optlen);
   if ( MStreamSocketBase::SocketErrorStatus == res )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   M_ASSERT(res == 0);
}

void MStreamSocketBase::DoOsSetsockopt(SocketHandleType sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
   const int res = ::setsockopt(sockfd, level, optname, static_cast<const char*>(optval), optlen);
   if ( MStreamSocketBase::SocketErrorStatus == res )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   M_ASSERT(res == 0);
}

int MStreamSocketBase::DoOsListen(SocketHandleType sockfd, int backlog, bool throwException)
{
   const int res = ::listen(sockfd, backlog);
   if ( MStreamSocketBase::SocketErrorStatus == res && throwException )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return res;
}

void MStreamSocketBase::DoOsGethostname(char* name, unsigned namelen)
{
   const int res = ::gethostname(name, namelen);
   if ( res < 0 )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
}

int MStreamSocketBase::DoOsGetpeername(SocketHandleType sockfd, struct sockaddr* addr, socklen_t* addrlen, bool throwException)
{
   const int res = ::getpeername(sockfd, addr, addrlen);
   if ( MStreamSocketBase::SocketErrorStatus == res && throwException )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return res;
}

int MStreamSocketBase::DoOsGetsockname(SocketHandleType sockfd, struct sockaddr* addr, socklen_t* addrlen, bool throwException)
{
   const int res = ::getsockname(sockfd, addr, addrlen);
   if ( MStreamSocketBase::SocketErrorStatus == res && throwException )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return res;
}

int MStreamSocketBase::DoOsGetnameinfo(const struct sockaddr* addr, socklen_t addrlen, char* host, socklen_t hostlen, char* serv, socklen_t servlen, int flags, bool throwException)
{
#if M_OS & M_OS_POSIX
BEGIN:
#endif
   const int res = ::getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
   if ( 0 != res )
   {
#if M_OS & M_OS_POSIX
      if ( EAI_SYSTEM == res && EINTR == errno )
         goto BEGIN;
#endif

      if ( throwException )
      {
         MESocketError::ThrowSocketErrorFromReturnValue(res);
         M_ENSURED_ASSERT(0);
      }
   }
   return res;
}

int MStreamSocketBase::DoOsGetaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** response)
{
#if M_OS & M_OS_POSIX
BEGIN:
#endif
   const int res = ::getaddrinfo(node, service, hints, response);
   if ( 0 != res )
   {
#if M_OS & M_OS_POSIX
      if ( EAI_SYSTEM == res && EINTR == errno )
         goto BEGIN;
#endif
      MESocketError::ThrowSocketErrorFromReturnValue(res);
      M_ENSURED_ASSERT(0);
   }
   return res;
}

#if (M_OS & M_OS_WINDOWS) != 0

void MStreamSocketBase::DoSetNonBlocking(SocketHandleType sockfd, bool nonblock)
{
   unsigned long flag = nonblock ? 1 : 0;
   DoOsIoctl(sockfd, FIONBIO, &flag);
}

bool MStreamSocketBase::DoNonBlockingConnectionWait(SocketHandleType sockfd, unsigned ms)
{
   fd_set rfds, wfds, efds;
   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   FD_ZERO(&efds);
   FD_SET(sockfd, &rfds);
   FD_SET(sockfd, &wfds);
   FD_SET(sockfd, &efds);

   if ( DoOsSelect(static_cast<int>(sockfd) + 1, &rfds, &wfds, &efds, ms) == 0 )
      return false;

   if ( FD_ISSET(sockfd, &efds) )
   {
      int err = 0;
      socklen_t errlen = sizeof(err);
      DoOsGetsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen);
      if ( err )
      {
         ::WSASetLastError(err);
         MESocketError::ThrowLastSocketError();
         M_ENSURED_ASSERT(0);
      }
      return false;
   }

   if ( !FD_ISSET(sockfd, &rfds) && !FD_ISSET(sockfd, &wfds) )
      return false;

   return true;
}

#else

void MStreamSocketBase::DoSetNonBlocking(SocketHandleType sockfd, bool nonblock)
{
   int flags = fcntl(sockfd, F_GETFL, NULL);
   if ( flags < 0 )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }

   if ( fcntl(sockfd, F_SETFL, nonblock ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK) ) == -1 )
   {
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
}

bool MStreamSocketBase::DoNonBlockingConnectionWait(MStreamSocketBase::SocketHandleType sockfd, unsigned ms)
{
   fd_set rfds, wfds;
   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   FD_SET(sockfd, &rfds);
   FD_SET(sockfd, &wfds);

   if ( DoOsSelect(static_cast<int>(sockfd) + 1, &rfds, &wfds, NULL, ms) == 0 )
      return false;

   if ( !FD_ISSET(sockfd, &rfds) && !FD_ISSET(sockfd, &wfds) )
      return false;

   int err = 0;
   socklen_t errlen = sizeof(err);
   DoOsGetsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen);
   if ( err )
   {
      errno = err;
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return true;
}

#endif

bool MStreamSocketBase::DoNonblockingReceiveWait(SocketHandleType sockfd, unsigned ms)
{
   if ( InvalidSocket == sockfd )
      return true;

#if !M_NO_LUA_COOPERATIVE_IO
   return MLuaYieldAndSelect(sockfd, ms, 0 /*read*/);
#else
   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(sockfd, &fds);
   bool yea = DoOsSelect(static_cast<int>(sockfd) + 1, &fds, NULL, NULL, ms) != 0;
   M_ASSERT(yea == (FD_ISSET(sockfd, &fds) != 0));
   return yea;
#endif
}

#ifndef IN6_IS_ADDR_LINKLOCAL
   // Similar to what is defined in /usr/include/isc/net.h
   #define IN6_IS_ADDR_LINKLOCAL(a) (((a)->s6_addr[0] == 0xFE) && (((a)->s6_addr[1] & 0xC0) == 0x80))
#endif

void MStreamSocketBase::DoAdjustAddress(addrinfo* ai)
{
#if (M_OS & M_OS_WINDOWS) == 0 || (_WIN32_WINNT >= 0x0600) // support of if_nametoindex() is added to Windows Vista, 0x0600
   if ( ai->ai_family == AF_INET6 )
   {
      struct sockaddr_in6* ai6 = reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr);
      if ( ai6 != NULL 
#if (M_OS & M_OS_ANDROID) == 0 // in case of Android alyways attempt to set the network card index as Android does not have DHCPv6
           && IN6_IS_ADDR_LINKLOCAL(&ai6->sin6_addr)
#endif
           && ai6->sin6_scope_id == 0 ) // Link local address, with the interface that is not set
      {
         struct ifaddrs* ifaddresses;
         struct ifaddrs* addr = NULL;
         struct ifaddrs* addrBestMatch = NULL;

         int result = getifaddrs(&ifaddresses);
         if ( result != 0 )
            return; // cannot do anything in such case
         for ( struct ifaddrs* ifa = ifaddresses; ifa != NULL; ifa = ifa->ifa_next )
         {
            if ( ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET6 && (ifa->ifa_flags & IFF_UP) != 0 && (ifa->ifa_flags & IFF_LOOPBACK) == 0 )
            {
               struct sockaddr_in6* ai6i = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
               if ( IN6_IS_ADDR_LINKLOCAL(&ai6i->sin6_addr) )
               {
                  addr = ifa; // if more than one interface has link local address the last one will be selected, cannot do any better
                  if ( memcmp(&ai6i->sin6_addr, &ai6->sin6_addr, 16) == 0 ) // this is the exact address we are searching for, link local (compare only the IPv6 address part of an address)
                  {
                     addrBestMatch = addr;
                     break; // immediately proceed with modifying the address with the interface index
                  }
                  if ( addrBestMatch == NULL && strncmp(addr->ifa_name, "wlan", 4) == 0 )
                     addrBestMatch = addr; // first wlan interface will be effective
               }
            }
         }
         if ( addr != NULL )
         {
            if ( addrBestMatch != NULL )
               addr = addrBestMatch;
            ai6->sin6_scope_id = if_nametoindex(addr->ifa_name); // here it is, setting the interface index
         }
         freeifaddrs(ifaddresses);
      }
   }
#else
   M_USED_VARIABLE(ai);
#endif
}

#if (M_OS & M_OS_WINDOWS) != 0
   inline bool DoCheckIfIPv6Installed()
   {
      bool isIPv6present = false;
      INT iProtocols[2];
      iProtocols[0] = IPPROTO_TCP; // there search for IPv6 support
      iProtocols[1] = 0;
      DWORD dwBufferLen = 16384; // Allocate a 16K buffer to retrieve all the protocols providers for IPv6, this is more than enough
      LPWSAPROTOCOL_INFOW lpProtocolInfo = (LPWSAPROTOCOL_INFOW) M_NEW char[dwBufferLen];
      INT iNuminfo = WSAEnumProtocolsW(iProtocols, lpProtocolInfo, &dwBufferLen);
      if ( iNuminfo != SOCKET_ERROR )
      {
         for ( int i = 0; i < iNuminfo; ++i )
         {
            if ( lpProtocolInfo[i].iAddressFamily == AF_INET6 )
            {
               isIPv6present = true;
               break;
            }
         }
      }
      delete [] lpProtocolInfo;
      return isIPv6present;
   }
#endif

#endif

MByteString MStreamSocketBase::AddressToBinary(const MStdString& addr)
{
   MByteString result;
   #if !(M_OS & M_OS_NUTTX)
      OSInitSocketsLibrary(); // static service can be called without constructing any socket
   #endif
   #if (M_OS & M_OS_WINDOWS) != 0
      int ret;
      if ( addr.find(':') != MStdString::npos ) 
      {
         sockaddr_in6 ip6;
         memset(&ip6, 0, sizeof(ip6));
         INT ip6size = static_cast<INT>(sizeof(ip6));
         ret = ::WSAStringToAddressW(const_cast<LPWSTR>(MToWideString(addr).c_str()), AF_INET6,  NULL, (LPSOCKADDR)&ip6, &ip6size);
         if ( SocketErrorStatus == ret )
         {
            DoThrowBadIpAddress();
            M_ENSURED_ASSERT(0);
         }
         result.assign((const char*)&ip6.sin6_addr, sizeof(ip6.sin6_addr));
      }
      else
      {
         sockaddr_in ip4;
         memset(&ip4, 0, sizeof(ip4));
         INT ip4size = static_cast<INT>(sizeof(ip4));
         ret = ::WSAStringToAddressW(const_cast<LPWSTR>(MToWideString(addr).c_str()), AF_INET,  NULL, (LPSOCKADDR)&ip4, &ip4size);
         if ( SocketErrorStatus == ret )
         {
            DoThrowBadIpAddress();
            M_ENSURED_ASSERT(0);
         }
         result.assign((const char*)&ip4.sin_addr, sizeof(ip4.sin_addr));
      }
   #else
      int af;
      size_t len;
      char buf [ sizeof(in6_addr) ]; // use the biggest sizeof
      if ( addr.find(':') != MStdString::npos )
      {
         af = AF_INET6;
         len = sizeof(in6_addr);
      }
      else
      {
         af = AF_INET;
         len = sizeof(in_addr);
      }
      int ret = inet_pton(af, addr.c_str(), buf);
      if ( ret <= 0 )
      {
         DoThrowBadIpAddress();
         M_ENSURED_ASSERT(0);
      }
      result.assign(buf, len);
   #endif
   return result;
}

MStdString MStreamSocketBase::BinaryToAddress(const MByteString& addr)
{
   MStdString result;

   #if !(M_OS & M_OS_NUTTX)
      OSInitSocketsLibrary(); // static service can be called without constructing any socket
   #endif
   #if (M_OS & M_OS_WIN32_CE) != 0 // Windows CE

      wchar_t buf [ 128 ]; // size much bigger than any possible address
      int ret;
      DWORD bufLength = M_NUMBER_OF_ARRAY_ELEMENTS(buf) - 1;
      switch ( addr.size() )
      {
      case 4:
         {
            sockaddr_in ip4;
            memset(&ip4, 0, sizeof(ip4));
            ip4.sin_family = AF_INET;
            memcpy(&ip4.sin_addr, addr.data(), 4);
            ret = ::WSAAddressToStringW((LPSOCKADDR)&ip4, sizeof(ip4), 0, buf, &bufLength);
         }
         break;
      case 16:
         {
            sockaddr_in6 ip6;
            memset(&ip6, 0, sizeof(ip6));
            ip6.sin6_family = AF_INET6;
            memcpy(&ip6.sin6_addr, addr.data(), 16);
            ret = ::WSAAddressToStringW((LPSOCKADDR)&ip6, sizeof(ip6), 0, buf, &bufLength);
         }
         break;
      default:
         ret = OS_SockErrorStatus; // throw
         break;
      }
      if ( OS_SockErrorStatus == ret )
      {
         DoThrowBadIpAddress();
         M_ENSURED_ASSERT(0);
      }
      result = MToStdString(buf, bufLength);

   #elif (M_OS & M_OS_WINDOWS) != 0 // other Windows

      wchar_t buf [ 128 ]; // size much bigger than any possible address
      int ret;
      DWORD bufLength = M_NUMBER_OF_ARRAY_ELEMENTS(buf) - 1;
      switch ( addr.size() )
      {
      case 4:
         {
            sockaddr_in ip4;
            memset(&ip4, 0, sizeof(ip4));
            ip4.sin_family = AF_INET;
            memcpy(&ip4.sin_addr, addr.data(), 4);
            ret = ::WSAAddressToStringW((LPSOCKADDR)&ip4, sizeof(ip4), 0, buf, &bufLength);
         }
         break;
      case 16:
         {
            sockaddr_in6 ip6;
            memset(&ip6, 0, sizeof(ip6));
            ip6.sin6_family = AF_INET6;
            memcpy(&ip6.sin6_addr, addr.data(), 16);
            ret = ::WSAAddressToStringW((LPSOCKADDR)&ip6, sizeof(ip6), 0, buf, &bufLength);
         }
         break;
      default:
         ret = SocketErrorStatus; // throw
         break;
      }
      if ( SocketErrorStatus == ret )
      {
         DoThrowBadIpAddress();
         M_ENSURED_ASSERT(0);
      }
      M_ASSERT(buf[bufLength - 1] == '\0'); // ensure the system calls include trailing zero into the address
      result = MToStdString(buf, bufLength - 1); // skip trailing zero

   #else

      char buf [ 128 ]; // size much bigger than any possible address
      int af;
      switch ( addr.size() )
      {
      default:
         DoThrowBadIpAddress();
         M_ENSURED_ASSERT(0);
      case 4:
         af = AF_INET;
         break;
      case 16:
         af = AF_INET6;
         break;
      }
      const char* dst = inet_ntop(af, addr.data(), buf, sizeof(buf) - 1);
      if ( dst == NULL )
      {
         DoThrowBadIpAddress();
         M_ENSURED_ASSERT(0);
      }
      result = buf;

   #endif
   return result;
}

M_NORETURN_FUNC void MStreamSocketBase::DoThrowBadIpAddress()
{
#if (M_OS & M_OS_WINDOWS) != 0
   if ( !DoCheckIfIPv6Installed() )
   {
      MException::Throw(M_CODE_STR(MErrorEnum::BadIpAddress, M_I("Given address is not recognized as IPv4, and there is no IPv6 support installed")));
      M_ENSURED_ASSERT(0);
   }
   else
#endif
   {
      MException::Throw(M_CODE_STR(MErrorEnum::BadIpAddress, M_I("Given address is not recognized as IPv4 or IPv6")));
      M_ENSURED_ASSERT(0);
   }
}

#endif // !M_NO_SOCKETS || !M_NO_SOCKETS_UDP
