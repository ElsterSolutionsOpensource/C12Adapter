// File MCORE/MStreamSocketUdp.cpp

#include "MCOREExtern.h"
#include "MException.h"
#include "MUtilities.h"
#include "MCriticalSection.h"
#include "MAlgorithm.h"

#if !M_NO_SOCKETS_UDP

#include "MStreamSocketUdp.h"

#if (M_OS & M_OS_WINDOWS) != 0

   // Socket data length type for Windows definition
   //
   typedef socklen_t ssize_t;

   static const int OS_ETIMEDOUT = WSAETIMEDOUT;

#else  // (M_OS & M_OS_WINDOWS) != 0

   static const int OS_ETIMEDOUT = ETIMEDOUT;

#endif // (M_OS & M_OS_WINDOWS) != 0

   #if !M_NO_REFLECTION

      /// Constructor that creates UDP socket
      ///
      static MObject* DoNew()
      {
         return M_NEW MStreamSocketUdp();
      }

   #endif

M_START_PROPERTIES(StreamSocketUdp)
M_START_METHODS(StreamSocketUdp)
   M_CLASS_FRIEND_SERVICE               (StreamSocketUdp, New,                DoNew,                  ST_MObjectP_S)
   M_OBJECT_SERVICE                     (StreamSocketUdp, Connect,                                    ST_X_unsigned_constMStdStringA)
   M_OBJECT_SERVICE                     (StreamSocketUdp, Swap,                                       ST_X_MObjectP)
M_END_CLASS(StreamSocketUdp, StreamSocketBase)

MStreamSocketUdp::MStreamSocketUdp(SocketHandleType sockfd)
:
   MStreamSocketBase(sockfd),
   m_peerAddrLength(0),
   m_inputBufferSize(0),
   m_inputBufferCurr(0)
{
   memset(&m_peerAddr, 0, sizeof(m_peerAddr));
}

MStreamSocketUdp::~MStreamSocketUdp() M_NO_THROW
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

MStdString MStreamSocketUdp::GetName() const
{
   MStdString result;
   char addr [ NI_MAXHOST ];
   char serv [ NI_MAXSERV ];
   if ( m_peerAddrLength > 0 &&
        DoOsGetnameinfo((sockaddr*)&m_peerAddr, m_peerAddrLength, addr, sizeof(addr),
                        serv, sizeof(serv), NI_NUMERICHOST | NI_NUMERICSERV, false) == 0 )
   {
         result.append(addr);
         result += ':';
         result.append(serv);
   }
   if ( result.empty() )
      result.assign("<SocketUdp>", 11);
   return result;;
}

MStdString MStreamSocketUdp::GetPeerSocketName() const
{
   MStdString result;
   char addr [ NI_MAXHOST ];
   DoOsGetnameinfo((sockaddr*)&m_peerAddr, m_peerAddrLength, addr, sizeof(addr), 0, 0, NI_NUMERICHOST);
   result = addr;
   return result;
}

unsigned MStreamSocketUdp::GetPeerSocketPort() const
{
   char serv [ NI_MAXSERV ];
   DoOsGetnameinfo((sockaddr*)&m_peerAddr, m_peerAddrLength, NULL, 0, serv, sizeof(serv), NI_NUMERICSERV);
   return MToUnsigned(serv);
}

void MStreamSocketUdp::Connect(unsigned port, const MStdString& address)
{
   Close();
   M_ASSERT(InvalidSocket == m_socketHandle);

   DoStartOpen(FlagReadWrite);
   m_inputBufferSize = 0;
   m_inputBufferCurr = 0;

   struct addrinfo hints;
   memset(&hints, 0, sizeof(hints));
   hints.ai_socktype = SOCK_DGRAM;
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
      for ( struct addrinfo* ai = aih.m_pointer; ai != NULL; ai = ai->ai_next )
      {
         try
         {
            OsSocketHandleHolder sh;

            DoAdjustAddress(ai);

            sh.m_socketHandle = DoOsSocket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            m_socketHandle = sh.m_socketHandle;
            sh.m_socketHandle = InvalidSocket;

            m_peerAddrLength = static_cast<socklen_t>(ai->ai_addrlen);
            M_ASSERT(m_peerAddrLength <= static_cast<socklen_t>(sizeof(sockaddr_storage))); // ensured by the OS
            memcpy(&m_peerAddr, ai->ai_addr, m_peerAddrLength);
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
}

void MStreamSocketUdp::ClearInputBuffer()
{
   m_inputBufferCurr = 0;
   m_inputBufferSize = 0;
}

unsigned MStreamSocketUdp::GetBytesReadyToRead() const
{
   return m_inputBufferSize - m_inputBufferCurr;
}

unsigned MStreamSocketUdp::RecvFrom(char* buffer, unsigned length, int flags, sockaddr* addr, socklen_t* addrLen)
{
   M_ASSERT(InvalidSocket != m_socketHandle);

#if M_OS & M_OS_POSIX
BEGIN:
#endif
   const ::ssize_t res = ::recvfrom(m_socketHandle, buffer, length, flags, addr, addrLen);
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

unsigned MStreamSocketUdp::SendTo(const char* buf, unsigned len, int flags, const sockaddr* addr, socklen_t addrLength)
{
#if M_OS & M_OS_POSIX
BEGIN:
#endif
   const int res = ::sendto(m_socketHandle, buf, len, flags, addr, addrLength);
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

unsigned MStreamSocketUdp::Recv(char* buf, unsigned len, int flags)
{
   m_peerAddrLength = sizeof(m_peerAddr); // reset peer address
   return RecvFrom(buf, len, flags, (sockaddr*)&m_peerAddr, &m_peerAddrLength);
}

unsigned MStreamSocketUdp::Send(const char* buf, unsigned len, int flags)
{
   return SendTo(buf, len, flags, (const sockaddr*)&m_peerAddr, m_peerAddrLength);
}

void MStreamSocketUdp::Swap(MStreamSocketUdp& other)
{
   M_DYNAMIC_CAST_WITH_THROW(MStreamSocketUdp, &other); // check for reflection call
   M_DYNAMIC_CAST_WITH_THROW(MStreamSocketUdp, this); // check for reflection call
   DoSwap(other);
   std::swap(m_socketHandle,    other.m_socketHandle);
   std::swap(m_receiveTimeout,  other.m_receiveTimeout);
   std::swap(m_peerAddr,        other.m_peerAddr);
   std::swap(m_peerAddrLength,  other.m_peerAddrLength);
   std::swap(m_inputBufferSize, other.m_inputBufferSize);
   std::swap(m_inputBufferCurr, other.m_inputBufferCurr);

   char tmpBuffer[MStreamSocketUdp::MAXIMUM_DATAGRAM_SIZE];
   M_COMPILED_ASSERT(sizeof(tmpBuffer) == MStreamSocketUdp::MAXIMUM_DATAGRAM_SIZE);
   M_COMPILED_ASSERT(sizeof(m_inputBuffer) == MStreamSocketUdp::MAXIMUM_DATAGRAM_SIZE);
   memcpy(tmpBuffer, m_inputBuffer, MStreamSocketUdp::MAXIMUM_DATAGRAM_SIZE);
   memcpy(m_inputBuffer, other.m_inputBuffer, MStreamSocketUdp::MAXIMUM_DATAGRAM_SIZE);
   memcpy(other.m_inputBuffer, tmpBuffer, MStreamSocketUdp::MAXIMUM_DATAGRAM_SIZE);
}

unsigned MStreamSocketUdp::DoReadAllAvailableBytesImpl(char* buf, unsigned len)
{
   return DoReadAvailableBytesImpl(buf, len);
}

unsigned MStreamSocketUdp::DoReadAvailableBytesImpl(char* buf, unsigned len)
{
   M_ASSERT(InvalidSocket != m_socketHandle);
   M_ASSERT(m_inputBufferSize >= m_inputBufferCurr);

   unsigned diff = m_inputBufferSize - m_inputBufferCurr;
   if ( diff > 0 )
   {
      if ( diff >= len )
      {
         memcpy(buf, m_inputBuffer + m_inputBufferCurr, len);
         m_inputBufferCurr += len;
         return len; // done
      }
      memcpy(buf, m_inputBuffer + m_inputBufferCurr, diff);
      m_inputBufferCurr = 0;
      m_inputBufferSize = 0;
      return diff;
   }

   if ( !WaitToReceive(m_receiveTimeout) )
      return 0;

   if ( len >= MAXIMUM_DATAGRAM_SIZE ) // buffer fits into datagram
      return Recv(buf, len, 0);
   else // have to reallocate
   {
      unsigned size = Recv(m_inputBuffer, sizeof(m_inputBuffer), 0);
      if ( size <= len )
      {
         memcpy(buf, m_inputBuffer, size);
         return size;
      }
      else
      {
         memcpy(buf, m_inputBuffer, len);
         m_inputBufferCurr = len;
         m_inputBufferSize = size;
         return len;
      }
   }
}

void MStreamSocketUdp::DoWriteBytesImpl(const char* buf, unsigned len)
{
#ifdef MSG_NOSIGNAL // Linux
   unsigned ret = Send(buf, len, MSG_NOSIGNAL);
#elif (M_OS & M_OS_LINUX) != 0 // something is wrong, Linux does not have MSG_NOSIGNAL
   #error "Linux is expected to have MSG_NOSIGNAL"
#else
   unsigned ret = Send(buf, len, 0);
#endif
   if ( ret != len )
   {  // This is an error in program
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_PACKET_IS_TOO_BIG, "The outgoing packet does not fit into datagram"));
      M_ENSURED_ASSERT(0);
   }
}

#endif
