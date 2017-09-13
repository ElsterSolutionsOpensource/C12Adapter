// File MCORE/MStreamSocket.cpp

#include "MCOREExtern.h"
#include "MException.h"
#include "MStreamSocket.h"
#include "MUtilities.h"
#include "MCriticalSection.h"
#include "MAlgorithm.h"

#if !M_NO_SOCKETS

#if (M_OS & M_OS_ANDROID) != 0
//   #include <ifaddrs.h>
   #include <sys/ioctl.h>
   #include <net/if.h>
#endif

namespace
{

#if (M_OS & M_OS_WINDOWS) != 0

   // Servicename must be a numeric port number (value taken from Platform SDK)
   #ifndef AI_NUMERICSERV
      #define AI_NUMERICSERV 0x00000008
   #endif
   #ifndef EWOULDBLOCK
      #define EWOULDBLOCK     140
   #elif EWOULDBLOCK != 140
      #error "Bad define guess for EWOULDBLOCK!"
   #endif
   #if !defined(__BORLANDC__) || !defined(_SSIZE_T_DEFINED) 
      // Socket data length type for Windows definition
      //
      typedef socklen_t ssize_t;
   #endif

#endif // (M_OS & M_OS_WINDOWS) != 0

#if !M_NO_SOCKETS_SOCKS

class SocksProtocolHandler
{
public:

   SocksProtocolHandler(MStreamSocket* socket)
   :
      m_req(),
      m_res(),
      m_socket(socket)
   {
   }

   void Run(Muint16 service, const MByteString& address);
   void Run(Muint16 service, const MByteString& address, const MByteString& username, const MByteString& password);

private:
   void Send() const;
   void Recv(size_t size);
   void ContinueSocks5(Muint16 service, const MByteString& address);
   void ContinueSocks4(Muint16 service, const MByteString& address, const MByteString& username);
   M_NORETURN_FUNC void ThrowException(MConstLocalChars message) const;
   void DoCheckParameterSize(const MByteString& param) const;

private:
   MByteString m_req;
   MByteString m_res;
   MStreamSocket* m_socket;
};

void SocksProtocolHandler::DoCheckParameterSize(const MByteString& param) const
{
   if ( param.size() == 0 || param.size() > 255 )
   {
      ThrowException(M_I("SOCKS5 configuration parameter has incorrect size"));
      M_ENSURED_ASSERT(0);
   }
}

void SocksProtocolHandler::Send() const
{
   m_socket->WriteBytes(&m_req[0], M_64_CAST(unsigned, m_req.size()));
}

void SocksProtocolHandler::Recv(size_t size)
{
   m_res.resize(size);
   m_socket->ReadBytes(&m_res[0], M_64_CAST(unsigned, m_res.size()));
}

M_NORETURN_FUNC void SocksProtocolHandler::ThrowException(MConstLocalChars message) const
{
   MESocketError::ThrowSocketError(0, MErrorEnum::ProxySocketError, message);
   M_ENSURED_ASSERT(0);
}

void SocksProtocolHandler::Run(Muint16 service, const MByteString& address)
{
   DoCheckParameterSize(address);

   m_req.reserve(512);
   m_res.reserve(512);

   // 5 - version
   // 1 - count of authentication methods
   // 0 - with no authentication
   m_req.assign("\x05\x01\x00", 3);
   Send();

   Recv(2);
   if ( m_res[0] == 0x05 )
   {
      // use socks5 protocol
      if ( m_res[1] != 0x00 )
      {
         ThrowException(M_I("The requested type of authentication is not supported by SOCKS5 server"));
      }

      ContinueSocks5(service, address);
   }
   else
   {
      // try to use socks4a protocol
      ContinueSocks4(service, address, "");
   }
}

void SocksProtocolHandler::Run(Muint16 service, const MByteString& address, const MByteString& username, const MByteString& password)
{
   DoCheckParameterSize(address);

   m_req.reserve(512);
   m_res.reserve(512);

   // 5 - version
   // 1 - count of authentication methods
   // 2 - username:password authentication
   m_req.assign("\x05\x01\x02", 3);
   Send();

   Recv(2);
   if ( m_res[0] == 0x05 )
   {
      // use socks5 protocol
      if ( m_res[1] == 0x00 )
      {
         // pass - with no authentication
      }
      else if ( m_res[1] == 0x02 )
      {
         DoCheckParameterSize(password);

         // username:password authentication
         m_req.assign("\x01", 1);
         m_req.push_back(static_cast<MByteString::value_type>(username.size()));
         m_req += username;
         m_req.push_back(static_cast<MByteString::value_type>(password.size()));
         m_req += password;
         Send();

         Recv(2);
         if ( m_res[0] != 0x01 || m_res[1] != 0x00 )
         {
            ThrowException(M_I("SOCKS5 authentication failed"));
         }
      }
      else
      {
         ThrowException(M_I("The requested type of authentication is not supported by SOCKS5 server"));
      }

      ContinueSocks5(service, address);
   }
   else
   {
      ContinueSocks4(service, address, username);
   }
}

void SocksProtocolHandler::ContinueSocks5(Muint16 service, const MByteString& address)
{
   // 5 version
   // 1 connect
   // 0 reserved
   // 3 ATYP - full domain name
   m_req.assign("\05\x01\x00\x03", 4);
   m_req.push_back(static_cast<MByteString::value_type>(address.size()));
   m_req += address;

   const Muint16 ns = htons(service);
   m_req.push_back(reinterpret_cast<const char*>(&ns)[0]);
   m_req.push_back(reinterpret_cast<const char*>(&ns)[1]);
   Send();

   Recv(4);
   if ( m_res[0] != 0x05 )
   {
      ThrowException(M_I("Unexpected response from SOCKS5 server"));
   }

   switch ( m_res[1] )
   {
   case 0x00:
      // success
      break;
   case 0x01:
      ThrowException(M_I("SOCKS5 server error"));
      break;
   case 0x02:
      ThrowException(M_I("Connection not allowed by SOCKS5 ruleset"));
      break;
   case 0x03:
      ThrowException(M_I("Network unreachable from SOCKS5 server"));
      break;
   case 0x04:
      ThrowException(M_I("Host unreachable from SOCKS5 server"));
      break;
   case 0x05:
      ThrowException(M_I("Connection from SOCKS5 server refused"));
      break;
   case 0x06:
      ThrowException(M_I("TTL expired at SOCKS5 server end"));
      break;
   case 0x07:
      ThrowException(M_I("Command is not supported by SOCKS5 server"));
      break;
   case 0x08:
      ThrowException(M_I("Type of network address is not supported by SOCKS5 server"));
      break;
   default:
      ThrowException(M_I("Undefined SOCKS5 error"));
   }

   const int atyp = static_cast<int>(m_res[3]);
   switch ( atyp )
   {
   case 0x01:
      // IPv4
      Recv(4 + 2); // IPv4 address (4 bytes) + port number (2 bytes)
      break;
   case 0x03:
      {
         // full domain name
         const size_t respSize = static_cast<size_t>(static_cast<unsigned char>(m_res[4]));
         Recv(respSize + 2); // domain name ('respSize' bytes) + port number (2 bytes)
      }
      break;
   case 0x04:
      // IPv6 address
      Recv(16 + 2); // IPv6 address (16 bytes) + port number (2 bytes)
      break;
   default:
      ThrowException(M_I("Unexpected SOCKS5 ATYP field"));
   }
}

void SocksProtocolHandler::ContinueSocks4(Muint16 service, const MByteString& address, const MByteString& username)
{
   DoCheckParameterSize(username);

   m_req.resize(8);
   m_req[0] = 0x04; // version
   m_req[1] = 0x01; // CONNECT

   const Muint16 ns = htons(service);
   m_req[2] = reinterpret_cast<const char*>(&ns)[0];
   m_req[3] = reinterpret_cast<const char*>(&ns)[1];

   m_req[4] = 0;
   m_req[5] = 0;
   m_req[6] = 0;
   m_req[7] = static_cast<unsigned char>(255);

   m_req += username;
   m_req.push_back('\0');
   m_req += address;
   m_req.push_back('\0');
   Send();

   Recv(8);
   switch ( m_res[1] )
   {
   case 0x5a:
      // request granted
      break;
   case 0x5b:
      ThrowException(M_I("Request by SOCKS5 server rejected or failed"));
      break;
   case 0x5c:
      ThrowException(M_I("SOCKS5 identification service is absent or not reachable"));
      break;
   case 0x5d:
      ThrowException(M_I("SOCKS5 identification service could not confirm user credentials"));
      break;
   default:
      ThrowException(M_I("Undefined SOCKS5 error"));
   }
}

#endif // !M_NO_SOCKETS_SOCKS

} // namespace

   #if !M_NO_REFLECTION

      /// Constructor that creates socket
      ///
      static MObject* DoNew()
      {
         return M_NEW MStreamSocket();
      }

   #endif

M_START_PROPERTIES(StreamSocket)
   M_OBJECT_PROPERTY_UINT               (StreamSocket, SendTimeout)
   M_OBJECT_PROPERTY_BOOL               (StreamSocket, NoDelay)
   M_OBJECT_PROPERTY_INT                (StreamSocket, SendBufferSize)
   M_OBJECT_PROPERTY_INT                (StreamSocket, ReceiveBufferSize)
M_START_METHODS(StreamSocket)
   M_CLASS_FRIEND_SERVICE               (StreamSocket, New,                DoNew,                  ST_MObjectP_S)
   M_OBJECT_SERVICE                     (StreamSocket, Connect,                                    ST_X_unsigned_constMStdStringA)
#if !M_NO_SOCKETS_SOCKS
   M_OBJECT_SERVICE                     (StreamSocket, ConnectWithProxy,                           ST_X_unsigned_constMStdStringA_constMStdStringA)
#endif
   M_OBJECT_SERVICE_OVERLOADED          (StreamSocket, Listen,             Listen,              1, ST_X_unsigned)
   M_OBJECT_SERVICE_OVERLOADED          (StreamSocket, Listen,             DoListen,            0, ST_X) // SWIG_HIDE
   M_OBJECT_SERVICE                     (StreamSocket, Accept,                                     ST_X_MObjectP)
   M_OBJECT_SERVICE                     (StreamSocket, TimedAccept,                                ST_bool_X_MObjectP_unsigned)
   M_OBJECT_SERVICE                     (StreamSocket, Swap,                                       ST_X_MObjectP)
   M_OBJECT_SERVICE                     (StreamSocket, SetLinger,                                  ST_X_bool_int)
M_END_CLASS(StreamSocket, StreamSocketBase)

MStreamSocket::MStreamSocket(SocketHandleType sockfd)
:
   MStreamSocketBase(sockfd),
   m_sendTimeout(TimeoutDefault)
{
}

MStreamSocket::~MStreamSocket() M_NO_THROW
{
   try
   {
      Close();
   }
   catch ( ... )
   {
      M_ASSERT(0); // it is normal in release mode to swallow an exception thrown by Close(), warn in debug
   }
}

MStdString MStreamSocket::GetPeerSocketName() const
{
   M_ASSERT(InvalidSocket != m_socketHandle);

   sockaddr_storage socketAddress;
   socklen_t socketAddressLength = sizeof(socketAddress);
   DoOsGetpeername(m_socketHandle, (sockaddr*)&socketAddress, &socketAddressLength);

   char res[NI_MAXHOST];
   DoOsGetnameinfo((sockaddr*)&socketAddress, socketAddressLength, res, sizeof(res), NULL, 0, NI_NUMERICHOST);
   return MToStdString(res);
}

unsigned MStreamSocket::GetPeerSocketPort() const
{
   M_ASSERT(InvalidSocket != m_socketHandle);

   sockaddr_storage socketAddress;
   socklen_t socketAddressLength = sizeof(socketAddress);
   DoOsGetpeername(m_socketHandle, (sockaddr*)&socketAddress, &socketAddressLength);

   char res[NI_MAXSERV];
   DoOsGetnameinfo((sockaddr*)&socketAddress, socketAddressLength, NULL, 0, res, sizeof(res), NI_NUMERICSERV);
   return MToUnsigned(res);
}

void MStreamSocket::DoListen()
{
   Listen();
}

void MStreamSocket::Listen(unsigned backlog)
{
   DoOsListen(m_socketHandle, backlog);
}

void MStreamSocket::Accept(MStreamSocket& socket)
{
   M_ASSERT(InvalidSocket == socket.m_socketHandle);

   socket.DoStartOpen(FlagReadWrite);

#if M_OS & M_OS_POSIX
BEGIN:
#endif
   const SocketHandleType res = ::accept(m_socketHandle, NULL, NULL);
   if ( InvalidSocket == res )
   {
#if M_OS & M_OS_POSIX
      if ( EINTR == errno )
         goto BEGIN;
#endif
      unsigned err = MESocketError::GetLastGlobalSocketError();
      socket.Close();
      MESocketError::ThrowSocketError(err);
      M_ENSURED_ASSERT(0);
   }

   socket.m_socketHandle = res;
   try
   {
      DoSetNonBlocking(res, true);
      socket.SetLinger(true, 60);
   }
   catch ( MException& )
   {
      socket.Close(); // make an error an error
      throw;
   }
   socket.DoFinishOpen();
}

bool MStreamSocket::TimedAccept(MStreamSocket& socket, unsigned timeout)
{
   M_ASSERT(InvalidSocket == socket.m_socketHandle); // Socket has to be unopened

   if ( !WaitToReceive(timeout) )
      return false;
   Accept(socket);
   return true;
}

void MStreamSocket::Swap(MStreamSocket& other)
{
   M_DYNAMIC_CAST_WITH_THROW(MStreamSocket, &other); // check for reflection call
   DoSwap(other);
   std::swap(m_socketHandle,   other.m_socketHandle);
   std::swap(m_sendTimeout, other.m_sendTimeout);
   std::swap(m_receiveTimeout, other.m_receiveTimeout);
}

void MStreamSocket::ConnectInterruptible(unsigned port, const MStdString& address, OperationHandler* oph)
{
   Close();
   M_ASSERT(InvalidSocket == m_socketHandle);

   DoStartOpen(FlagReadWrite);

   struct addrinfo hints;
   memset(&hints, 0, sizeof(hints));
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_NUMERICSERV;

   try
   {
      hints.ai_family = (address.empty() || IsAddressLocalIPv4(address))
                      ? AF_INET
                      : AF_UNSPEC;

      const char* const hostname = address.c_str();

      char servname[NI_MAXSERV];
      MFormat(servname, sizeof(servname), "%d", port);

      OsAddrinfoHolder aih;
      DoOsGetaddrinfo(hostname, servname, &hints, &aih.m_pointer);

      bool rethrow = false;
      for ( struct addrinfo* ai = aih.m_pointer; ai != NULL; ai = ai->ai_next )
      {
         try
         {
            OsSocketHandleHolder sh;

            DoAdjustAddress(ai);

            sh.m_socketHandle = DoOsSocket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            DoSetNonBlocking(sh.m_socketHandle, true);

         #if M_OS & M_OS_POSIX
         BEGIN:
         #endif
            const int res = ::connect(sh.m_socketHandle, ai->ai_addr, static_cast<socklen_t>(ai->ai_addrlen));
            if ( SocketErrorStatus == res )
            {
         #if M_OS & M_OS_POSIX
               if ( EINTR == errno )
                  goto BEGIN;
               if ( errno != EINPROGRESS )
               {
                  MESocketError::ThrowLastSocketError();
                  M_ENSURED_ASSERT(0);
               }
         #else // windows
               const int err = ::WSAGetLastError();
               if ( WSAEWOULDBLOCK != err && WSAEINPROGRESS != err )
               {
                  MESocketError::ThrowLastSocketError();
                  M_ENSURED_ASSERT(0);
               }
         #endif
               while ( !DoNonBlockingConnectionWait(sh.m_socketHandle, 1000) )
               {
                  if ( oph )
                  {
                     try
                     {
                        oph->CheckIfCancelled();
                     }
                     catch ( ... )
                     {
                        rethrow = true;
                        throw;
                     }
                  }
               }
            }

            const struct linger option = {1, 60}; // linger for 60 seconds on close
            DoOsSetsockopt(sh.m_socketHandle, SOL_SOCKET, SO_LINGER, &option, sizeof(option));
            m_socketHandle = sh.m_socketHandle;
            sh.m_socketHandle = InvalidSocket;
            break;
         }
         catch ( ... )
         {
            if ( ai->ai_next == NULL || rethrow )
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
}

#if !M_NO_SOCKETS_SOCKS
void MStreamSocket::ConnectWithProxyInterruptible(unsigned port, const MStdString& address, const MStdString& proxyAddress, OperationHandler* oph)
{
   std::vector<MStdString> splittedProxyAddress = MAlgorithm::SplitWithDelimiter(proxyAddress, ':', false, true);

   bool useAuthorization = false;
   MStdString pUsername;
   MStdString pPassword;
   MStdString pAddress;
   MStdString pService;

   switch ( splittedProxyAddress.size() )
   {
   case 0:
      // without proxy
      ConnectInterruptible(port, address, oph);
      return;
   case 1:
      // it's address without authorization and port number
      pAddress = splittedProxyAddress[0];
      if ( pAddress.size() == 0 )
      {
         ConnectInterruptible(port, address, oph);
         return;
      }
      pService = "1080";
      break;
   case 2:
      // it's address with port number
      pAddress = splittedProxyAddress[0];
      pService = splittedProxyAddress[1];
      break;
   case 3:
      // it's address with authorization and port number
      useAuthorization = true;
      {
         std::vector<MStdString> usernameAndPassword = MAlgorithm::SplitWithDelimiter(splittedProxyAddress[0], '@', false, true);
         switch (usernameAndPassword.size())
         {
         case 0:
            break;
         case 1:
            pUsername = usernameAndPassword[0];
            break;
         case 2:
            pUsername = usernameAndPassword[0];
            pPassword = usernameAndPassword[1];
            break;
         default:
            // throw exception
            break;
         };
      }
      pAddress = splittedProxyAddress[1];
      pService = splittedProxyAddress[2];
      break;
   default:
      // throw exception
      break;
   };

   ConnectInterruptible(MToUnsigned(pService.c_str()), pAddress, oph);

   SocksProtocolHandler handler(this);
   try
   {
      if ( useAuthorization )
         handler.Run(port, address, pUsername, pPassword);
      else
         handler.Run(port, address);
   }
   catch ( ... )
   {
      Close();
      throw;
   }
}
#endif // !M_NO_SOCKETS_SOCKS

void MStreamSocket::ClearInputBuffer()
{
   char buff [ 256 ];
   while ( !IsInputBufferEmpty() )
      if ( Recv(buff, sizeof(buff), 0) == 0 )
         break;
}

unsigned MStreamSocket::Recv(char* buffer, unsigned length, int flags)
{
   M_ASSERT(InvalidSocket != m_socketHandle);

#if M_OS & M_OS_POSIX
BEGIN:
#endif
   const ::ssize_t res = ::recv(m_socketHandle, buffer, length, flags);
   if ( res < 0 )
   {
#if M_OS & M_OS_POSIX
      if ( EINTR == errno )
         goto BEGIN;
#endif
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return static_cast<unsigned>(res);
}

bool MStreamSocket::GetNoDelay() const
{
   int flag = 0;
   socklen_t size = sizeof(flag);
   DoOsGetsockopt(m_socketHandle, IPPROTO_TCP, TCP_NODELAY, &flag, &size);
   return 0 != flag;
}

void MStreamSocket::SetNoDelay(bool noDelay)
{
   const int flag = (noDelay ? 1 : 0);
   DoOsSetsockopt(m_socketHandle, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

void MStreamSocket::SetLinger(bool doLinger, int time)
{
   MENumberOutOfRange::CheckNamedIntegerRange(0, SHRT_MAX, time, "Linger");
   const struct linger option = 
       { 
         #if (M_OS & M_OS_WINDOWS) != 0
            static_cast<unsigned short>(doLinger ? 1u : 0u),
            static_cast<unsigned short>(time)
         #else
            (doLinger ? 1 : 0),
            static_cast<int>(time)
         #endif
      };
   DoOsSetsockopt(m_socketHandle, SOL_SOCKET, SO_LINGER, &option, sizeof(option));
}

int MStreamSocket::GetSendBufferSize() const
{
   int res;
   socklen_t len = sizeof(len);
   DoOsGetsockopt(m_socketHandle, SOL_SOCKET, SO_SNDBUF, &res, &len);
   return res;
}

void MStreamSocket::SetSendBufferSize(int size)
{
   DoOsSetsockopt(m_socketHandle, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

int MStreamSocket::GetReceiveBufferSize() const
{
   int res;
   socklen_t len = sizeof(res);
   DoOsGetsockopt(m_socketHandle, SOL_SOCKET, SO_RCVBUF, &res, &len);
   return res;
}

void MStreamSocket::SetReceiveBufferSize(int size)
{
   DoOsSetsockopt(m_socketHandle, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

unsigned MStreamSocket::Send(const char* buf, unsigned len, int flags)
{
#if M_OS & M_OS_POSIX
BEGIN:
#endif
   const int res = ::send(m_socketHandle, buf, len, flags);
   if ( res < 0 )
   {
#if M_OS & M_OS_POSIX
      if ( EINTR == errno )
         goto BEGIN;
#endif
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
   return static_cast<unsigned>(res);
}

unsigned MStreamSocket::GetBytesReadyToRead() const
{
   unsigned long res;
   DoOsIoctl(m_socketHandle, FIONREAD, &res);
   return static_cast<unsigned>(res);
}

unsigned MStreamSocket::DoReadAllAvailableBytesImpl(char* buf, unsigned len)
{
   return DoReadAvailableBytesPrivate(buf, len, false);
}

unsigned MStreamSocket::DoReadAvailableBytesImpl(char* buf, unsigned len)
{
   return DoReadAvailableBytesPrivate(buf, len, true);
}

unsigned MStreamSocket::DoReadAvailableBytesPrivate(char* buf, unsigned len, bool throwOnEndOfStream)
{
   for ( ;; )
   {
#ifdef MSG_NOSIGNAL // Linux
      const ::ssize_t res = ::recv(m_socketHandle, buf, len, MSG_NOSIGNAL | MSG_DONTWAIT);
#elif (M_OS & M_OS_LINUX) != 0 // something is wrong, Linux does not have MSG_NOSIGNAL
      #error "Linux is expected to have MSG_NOSIGNAL"
#else
      const ssize_t res = ::recv(m_socketHandle, buf, len, 0);
#endif
      if ( res > 0 )
      {
         M_ASSERT(static_cast<unsigned>(res) <= len);
         return static_cast<unsigned>(res);
      }

      if ( res == 0 )
      {
         if ( throwOnEndOfStream )
         {
            #if defined(WSAECONNRESET) // Windows, presumably. Has to be checked first.
               #define M__ECONNRESET   WSAECONNRESET
            #elif defined(ECONNRESET)
               #define M__ECONNRESET   ECONNRESET
            #else
               #error "Could not find ECONNRESET!"
            #endif
            MESocketError::ThrowSocketError(M__ECONNRESET, M_CODE_STR(MErrorEnum::SocketClosedByPeer, M_I("Socket connection closed by peer")));
            M_ENSURED_ASSERT(0);
         }
         return 0; // end of stream
      }
      int err = MESocketError::GetLastGlobalSocketError();
      if ( EWOULDBLOCK == err 
#if (M_OS & M_OS_WINDOWS) != 0
           || WSAEWOULDBLOCK == err
#endif
         )
      {
         if ( WaitToReceive(m_receiveTimeout) )
            continue;
         return 0; // timeout
      }
#if M_OS & M_OS_POSIX
      else if ( EINTR == err || EAGAIN == err )
         continue;
#endif
      MESocketError::ThrowLastSocketError();
      M_ENSURED_ASSERT(0);
   }
}

M_NORETURN_FUNC void MStreamSocket::DoThrowEndOfStream()
{
   MESocketError::ThrowSocketReadTimeout();
}

void MStreamSocket::DoWriteBytesImpl(const char* buf, unsigned len)
{
   for ( ;; )
   {
#ifdef MSG_NOSIGNAL // Linux
      const ssize_t res = ::send(m_socketHandle, buf, len, MSG_NOSIGNAL | MSG_DONTWAIT);
#elif (M_OS & M_OS_LINUX) != 0 // something is wrong, Linux does not have MSG_NOSIGNAL
      #error "Linux is expected to have MSG_NOSIGNAL"
#else
      ssize_t res = ::send(m_socketHandle, buf, len, 0);
#endif
      if ( res < 0 )
      {
         int err = MESocketError::GetLastGlobalSocketError();
         if ( EWOULDBLOCK == err 
#if (M_OS & M_OS_WINDOWS) != 0
              || WSAEWOULDBLOCK == err
#endif
            )
         {
            if ( WaitToSend(m_receiveTimeout) )
               continue;
            MESocketError::ThrowSocketReadTimeout();
            M_ENSURED_ASSERT(0);
         }
   #if M_OS & M_OS_POSIX
         else if ( EINTR == err || EAGAIN == err )
            continue;
   #endif
         MESocketError::ThrowLastSocketError();
         M_ENSURED_ASSERT(0);
      }

      M_ASSERT(len >= static_cast<unsigned>(res));
      len -= res;
      if ( len == 0 )
         break;
      buf += res;
   }
}

MStdString MStreamSocket::GetName() const
{
   MStdString result;
   sockaddr_storage socketAddress;
   socklen_t socketAddressLength = sizeof(socketAddress);
   if ( DoOsGetpeername(m_socketHandle, (sockaddr*)&socketAddress, &socketAddressLength, false) == 0 )
   {
      char addr [ NI_MAXHOST ];
      char serv [ NI_MAXSERV ];
      if ( DoOsGetnameinfo((sockaddr*)&socketAddress, socketAddressLength, addr, sizeof(addr), serv, sizeof(serv), NI_NUMERICHOST | NI_NUMERICSERV, false) == 0 )
      {
         result.reserve(NI_MAXSERV * 2);
         result.append(addr);
         result += ':';
         result.append(serv);
      }
   }
   if ( result.empty() )
      result.assign("<Socket>", 8);
   return result;;
}

#endif
