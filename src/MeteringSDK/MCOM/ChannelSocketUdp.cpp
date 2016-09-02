// File MCOM/ChannelSocketUdp.cpp

#include "MCOMExtern.h"
#include "ChannelSocketUdp.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_CHANNEL_SOCKET_UDP

#if defined(WSAECONNRESET) // Windows, presumably. Has to be checked first.
   #define M__ECONNRESET WSAECONNRESET
#elif defined(ECONNRESET)
   #define M__ECONNRESET ECONNRESET
#else
   #error "The define for ECONNRESET is not found"
#endif

// Do not mention MStreamSocketUdp::MAXIMUM_DATAGRAM_SIZE in the header as it
//    spoils MToolsJava generation. Have the equivalence check in the header instead.
M_COMPILED_ASSERT(static_cast<int>(MStreamSocketUdp::MAXIMUM_DATAGRAM_SIZE) == static_cast<int>(MChannelSocketUdp::MaximumUdpDatagramSize));

M_START_PROPERTIES(ChannelSocketUdp)
M_START_METHODS(ChannelSocketUdp)
   M_OBJECT_SERVICE(ChannelSocketUdp, WriteDatagram, ST_X_constMByteStringA)
   M_OBJECT_SERVICE(ChannelSocketUdp, ReadDatagram,  ST_MByteString_X)
M_END_CLASS_TYPED(ChannelSocketUdp, ChannelSocketBase, "CHANNEL_SOCKET_UDP")

MChannelSocketUdp::MChannelSocketUdp()
:
   MChannelSocketBase(),
   m_socket()
{
   m_socketPtr = &m_socket;
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelSocketUdp);
}

MChannelSocketUdp::~MChannelSocketUdp()
{
   Disconnect();
}

void MChannelSocketUdp::SetSocket(MStreamSocketBase& other)
{
   MStreamSocketUdp* sock = M_DYNAMIC_CAST_WITH_THROW(MStreamSocketUdp, &other); // check type casting
   m_socket.Swap(*sock);
   if ( m_socket.IsOpen() )
      DoInitChannel();
}

void MChannelSocketUdp::Connect()
{
   MChannelSocketBase::Connect();
   if ( m_isAutoAnswer )
      WaitForNextIncomingConnection();
   else
   {
      m_socket.Connect(m_peerPort, m_peerAddress);
      DoNotifyConnect();
   }
}

void MChannelSocketUdp::WaitForNextIncomingConnection(bool)
{
   if ( !m_isAutoAnswer )
   {
      MChannel::WaitForNextIncomingConnection(false); // this throws exception "channel is not in answer mode"
      M_ENSURED_ASSERT(0);
   }

   if ( IsConnected() )
      Disconnect();

   m_cancelCommunication = 0;
   m_cancelCommunicationGuard = 0;

   m_socket.Bind(m_autoAnswerPort, m_autoAnswerAddress);

   MTimer endTime(MTimer::SecondsToTimerMilliseconds(m_autoAnswerTimeout)); // timeout is in seconds, but timer is in milliseconds
   char buff [ 1 ];
   for ( ;; )
   {
      unsigned readSize = ReadWithTimeout(buff, sizeof(buff), 250);
      if ( readSize > 0 )
      {
         M_ASSERT(readSize == 1);
         UnreadBuffer(buff, readSize);
         break;
      }
      CheckIfOperationIsCancelled();
      if ( endTime.IsExpired() )
      {
         m_socket.Close(); // so we show as !IsConnected
         MCOMException::Throw(M_CODE_STR(M_ERR_TIMED_OUT_WHILE_WAITING_FOR_CONNECTION, M_I("Timed out while waiting for incoming socket connection")));
         M_ENSURED_ASSERT(0);
      }
   }
   m_socket.SetReceiveTimeout(m_readTimeout);
   DoNotifyConnect();
}

void MChannelSocketUdp::WriteDatagramBuffer(const char* buff, unsigned size)
{
   WriteBuffer(buff, size);
}

void MChannelSocketUdp::WriteDatagram(const MByteString& bytes)
{
   WriteDatagramBuffer(bytes.data(), static_cast<unsigned>(bytes.size()));
}

unsigned MChannelSocketUdp::ReadDatagramBuffer(char* buff, unsigned size)
{
   return DoReadCancellable(buff, size, m_readTimeout, true);
}

MByteString MChannelSocketUdp::ReadDatagram()
{
   MByteString result;
   char buff [ MaximumUdpDatagramSize ];
   unsigned size = ReadDatagramBuffer(buff, sizeof(buff));
   M_ASSERT(size <= sizeof(buff));
   result.assign(buff, size);
   return result;
}

#endif // !M_NO_MCOM_CHANNEL_SOCKET_UDP
