// File MCOM/ChannelSocketBase.cpp

#include "MCOMExtern.h"
#include "ChannelSocketBase.h"
#include "ChannelSocketUdp.h"
#include "MCOMExceptions.h"

#if !M_NO_MCOM_CHANNEL_SOCKET

#if !M_NO_MCOM_RAS_DIAL
   M_COMPILED_ASSERT((M_OS & M_OS_WINDOWS) == M_OS_WINDOWS); // RAS functionality is present in Windows only

   #include <wininet.h>
   #include <ras.h>
   #include <RasError.h>
#endif

using namespace std;

M_START_PROPERTIES(ChannelSocketBase)
   M_OBJECT_PROPERTY_PERSISTENT_STRING(ChannelSocketBase, PeerAddress,            "", ST_constMStdStringA_X,  ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_PERSISTENT_UINT  (ChannelSocketBase, PeerPort,               1153)
   M_OBJECT_PROPERTY_PERSISTENT_UINT  (ChannelSocketBase, AutoAnswerPort,         1153)
   M_OBJECT_PROPERTY_PERSISTENT_STRING(ChannelSocketBase, AutoAnswerAddress,      "", ST_constMStdStringA_X,  ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_READONLY_UINT    (ChannelSocketBase, ActualLocalPort)
   M_OBJECT_PROPERTY_READONLY_UINT    (ChannelSocketBase, ActualPeerPort)
   M_OBJECT_PROPERTY_READONLY_STRING  (ChannelSocketBase, ActualPeerAddress,  ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_STRING  (ChannelSocketBase, ActualLocalAddress, ST_MStdString_X)
   M_OBJECT_PROPERTY_OBJECT           (ChannelSocketBase, Socket)
#if !M_NO_MCOM_RAS_DIAL
   M_OBJECT_PROPERTY_PERSISTENT_STRING(ChannelSocketBase, RasDialName,            "", ST_constMStdStringA_X,  ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_PERSISTENT_UINT  (ChannelSocketBase, RasDialConnectDelay,    50)
   M_OBJECT_PROPERTY_PERSISTENT_UINT  (ChannelSocketBase, RasDialDisconnectDelay, 100)
#endif
M_START_METHODS(ChannelSocketBase)
#if !M_NO_MCOM_RAS_DIAL
   M_OBJECT_SERVICE                   (ChannelSocketBase, RasConnect,            ST_X)
   M_OBJECT_SERVICE                   (ChannelSocketBase, RasDisconnect,         ST_X)
#endif
M_END_CLASS_TYPED(ChannelSocketBase, Channel, "CHANNEL_SOCKET_BASE")

MChannelSocketBase::MChannelSocketBase()
:
   MChannel(),
   m_socketPtr(NULL)
#if !M_NO_MCOM_RAS_DIAL
   , m_rasConnection(0)
   , m_rasConnectionMadeInConnect(false)
#endif
{
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelSocketBase);
}

MChannelSocketBase::~MChannelSocketBase()
{
}

void MChannelSocketBase::SetAutoAnswerPort(unsigned port)
{
   MENumberOutOfRange::CheckInteger(0, 0xFFFF, (int)port, M_OPT_STR("AUTO_ANSWER_PORT"));
   m_autoAnswerPort = port;
}

void MChannelSocketBase::SetPeerPort(unsigned port)
{
   MENumberOutOfRange::CheckInteger(0, 0xFFFF, (int)port, M_OPT_STR("PEER_PORT"));
   m_peerPort = port;
}

MStdString MChannelSocketBase::GetActualLocalAddress() const
{
   CheckIfConnectedConst();
   M_ASSERT(m_socketPtr != NULL);
   return m_socketPtr->GetLocalSocketName();
}

unsigned MChannelSocketBase::GetActualLocalPort() const
{
   CheckIfConnectedConst();
   M_ASSERT(m_socketPtr != NULL);
   return m_socketPtr->GetLocalSocketPort();
}

MStdString MChannelSocketBase::GetActualPeerAddress() const
{
   CheckIfConnectedConst();
   M_ASSERT(m_socketPtr != NULL);
   return m_socketPtr->GetPeerSocketName();
}

unsigned MChannelSocketBase::GetActualPeerPort() const
{
   CheckIfConnectedConst();
   M_ASSERT(m_socketPtr != NULL);
   return m_socketPtr->GetPeerSocketPort();
}

void MChannelSocketBase::FlushOutputBuffer(unsigned)
{
}

void MChannelSocketBase::Connect()
{
   MChannel::Connect();

   #if !M_NO_MCOM_RAS_DIAL
      if ( m_rasConnection == 0 && !m_rasDialName.empty() )
      {
         m_rasConnectionMadeInConnect = true; // do it before the sleep so we hang up if the user canceled the wait
         try
         {
            RasConnect();
         }
         catch ( ... )
         {
            if ( m_rasConnection == 0 )
               m_rasConnectionMadeInConnect = false;
            throw;
         }
      }
   #endif
}

bool MChannelSocketBase::IsConnected() const
{
   M_ASSERT(m_socketPtr != NULL);
   return m_socketPtr->IsOpen();
}

void MChannelSocketBase::Disconnect()
{
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   MCriticalSection::Locker channelLocker(m_channelOperationCriticalSection);
#endif
   try
   {
      m_unreadBuffer.clear();
      if ( m_socketPtr->IsOpen() )
      {
         m_socketPtr->Close();
         DoNotifyDisconnect();
      }
   }
   catch ( ... ) // Disconnect never throws anything
   {
      m_socketPtr->Close(); // attempt to close again, safe operation to do multiple times
   }

   #if !M_NO_MCOM_RAS_DIAL
      // Even when Ras was connected by our RasDial() service a client could clear
      // m_rasDialName property to prevent Ras disconnection so check m_rasDialName.
      if ( m_rasConnectionMadeInConnect && !m_rasDialName.empty() )
      {
         M_ASSERT(m_rasConnection != 0);
         RasDisconnect();
         M_ASSERT(m_rasConnection == 0);
         m_rasConnectionMadeInConnect = false;
      }
   #endif
}

void MChannelSocketBase::DoClearInputBuffer()
{
   try
   {
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
      MCriticalSection::Locker channelLocker(m_channelOperationCriticalSection);
#endif
      m_socketPtr->ClearInputBuffer();
   }
   catch ( MException& ex )
   {
      DoHandleExceptionAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
}

unsigned MChannelSocketBase::DoWrite(const char* buf, unsigned len)
{
   try
   {
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
      MCriticalSection::Locker channelLocker(m_channelOperationCriticalSection);
#endif
      m_socketPtr->WriteBytes(buf, len);
   }
   catch ( MException& ex )
   {
      DoHandleExceptionAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
   return len;
}

unsigned MChannelSocketBase::DoRead(char* buff, unsigned size, unsigned timeout)
{
   unsigned result = 0u;
   try
   {
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
      MCriticalSection::Locker channelLocker(m_channelOperationCriticalSection);
#endif
      m_socketPtr->SetReceiveTimeout(timeout);
      result = m_socketPtr->ReadAvailableBytes(buff, size);
   }
   catch ( MException& ex )
   {
      DoHandleExceptionAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
   return result;
}

#if !M_NO_MCOM_RAS_DIAL

void MChannelSocketBase::RasConnect()
{
   if ( m_rasConnection != 0 || IsConnected() )
   {
      MCOMException::Throw(M_ERR_RAS_DIAL_ALREADY_CONNECTED, M_I("Channel is already connected or it has already dialed a RAS connection"));
      M_ENSURED_ASSERT(0);
   }
   if ( m_rasDialName.empty() )
   {
      MCOMException::Throw(M_ERR_RAS_DIAL_NAME_EMPTY, M_I("RAS dial name is empty"));
      M_ENSURED_ASSERT(0);
   }

   // RAS dial will work only in GUI applications, those that have main window
   //
   HWND activeWindow = ::GetActiveWindow();
   if ( activeWindow == NULL )
      activeWindow = ::GetForegroundWindow();
   #if M_UNICODE
      DWORD result = ::InternetDial(activeWindow, (LPWSTR)MToWideString(m_rasDialName).c_str(), INTERNET_DIAL_UNATTENDED, &m_rasConnection, 0);
   #else
      DWORD result = ::InternetDialA(activeWindow, (LPSTR)m_rasDialName.c_str(), INTERNET_DIAL_UNATTENDED, &m_rasConnection, 0);
   #endif
   if ( result != ERROR_SUCCESS )
   {
      M_ASSERT(m_rasConnection == 0);

      #if M_UNICODE
         wchar_t localErrStr[512];
         DWORD dwRetVal = ::RasGetErrorString(result, localErrStr, 512);
         if ( dwRetVal == ERROR_SUCCESS )
         {
            MCOMException::Throw(M_ERR_RAS_DIAL_NOT_CONNECTED, M_I("RAS error: %s"), MToStdString(localErrStr).c_str());
            M_ENSURED_ASSERT(0);
         }
      #else
         char localErrStr[512];
         DWORD dwRetVal = ::RasGetErrorString(result, localErrStr, 512);
         if ( dwRetVal == ERROR_SUCCESS )
         {
            MCOMException::Throw(M_ERR_RAS_DIAL_NOT_CONNECTED, M_I("RAS error: %s"), localErrStr);
            M_ENSURED_ASSERT(0);
         }
      #endif
      MConstLocalChars errStr = M_I("Unknown error while establishing RAS connection '%s'");
      if ( result == ERROR_INVALID_PARAMETER )
         errStr = M_I("Invalid parameter for RAS connection '%s'");

      MCOMException::Throw(M_ERR_RAS_DIAL_NOT_CONNECTED, errStr, MStr::ToEscapedString(m_rasDialName).c_str());
      M_ENSURED_ASSERT(0);
   }
   if ( m_rasConnection != 0 )
      this->Sleep(m_rasDialConnectDelay);
   else
      m_rasConnectionMadeInConnect = false; // it was already connected
}

void MChannelSocketBase::RasDisconnect() M_NO_THROW
{
   if ( m_rasConnection != 0 )
   {
      try
      {
         this->Sleep(m_rasDialDisconnectDelay);
      }
      catch ( ... )
      {
         // Only "MEOperationCancelled" may be caught here,
         // so ignore this exception and disconnect immediately after catch
      }
      ::InternetHangUp(m_rasConnection, 0); // ignore errors if any
      m_rasConnection = 0;
      m_rasConnectionMadeInConnect = false;
   }
}

#endif

   static void DoAddAddressAndPort(MStdString& result, const MStdString& addr, unsigned port)
   {
      if ( !addr.empty() )
      {
         bool hasColon = addr.find(':') != MByteString::npos;
         if ( hasColon ) // if the peer address has ":", case of IPv6, wrap it
            result += '[';
         result += addr;
         if ( hasColon )
            result += ']';
         result += ':';
      }
      result += MToStdString(port);
   }

MStdString MChannelSocketBase::GetMediaIdentification() const
{
   MStdString result;
   char str [ 32 ];
   result.reserve(128); // helps 90% of cases
   result = MUtilities::GetLocalHostName();
   bool isUdp = MChannelSocketUdp::GetStaticClass() == GetClass();
   if ( m_isAutoAnswer )
   {
      if ( isUdp )
         result.append(":SERVER_UDP:", 12);
      else
         result.append(":SERVER:", 8);
      DoAddAddressAndPort(result, m_autoAnswerAddress, m_autoAnswerPort);
      result += ':';
   }
   else
   {
      if ( isUdp )
         result.append(":SOCKET_UDP:", 12);
      else
         result.append(":SOCKET:", 8);
   }
   bool actualsAdded = false;
   if ( IsConnected() )
   {
      try
      {
         DoAddAddressAndPort(result, GetActualPeerAddress(), GetActualPeerPort());
         actualsAdded = true;
      }
      catch ( ... )
      {
         // swallow any possible socket error raised at getting actual addresses
      }
   }
   if ( !actualsAdded )  // Either not connected, or there was a failure at a system call
   {                     // Recover without doing any system calls
      if ( m_isAutoAnswer )
      {
         size_t uniq = reinterpret_cast<size_t>((void*)this); // Create memory-unique number from object's address.
         uniq /= sizeof(MChannelSocketBase);                      // Make it smaller for presentation convenience, still unique
         size_t size = MFormat(str, sizeof(str), "#%X", (unsigned)uniq); // convert to unsigned, in hopes this one is still unique
         M_ASSERT(size < sizeof(str)); // check if the supplied buffer fits (due to used format this is the case)
         result.append(str, size);
      }
      else
         DoAddAddressAndPort(result, m_peerAddress, m_peerPort);
   }
   return result;
}

M_NORETURN_FUNC void MChannelSocketBase::DoHandleExceptionAndRethrow(MException& ex)
{
   CheckIfOperationIsCancelled();

   if ( ex.GetCode() == MErrorEnum::SocketReadTimeout )
      MEChannelReadTimeout::Throw(0);
   else
      ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

#endif // !M_NO_MCOM_CHANNEL_SOCKET
