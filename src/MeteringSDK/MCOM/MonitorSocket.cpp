// File MCOM/MMonitorSocket.cpp

#include "MCOMExtern.h"
#include "MCOMExceptions.h"
#include "MonitorSocket.h"

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING

   const unsigned s_PORT = 34783;
   const unsigned s_SEND_TIMEOUT = 5000; // 5 seconds
   static const char str_CLIENT_ADDRESS [] = "CLIENT_ADDRESS";
   static const char str_LOCALHOST      [] = "localhost";
   static const char str_127_0_0_1      [] = "127.0.0.1";

   #if !M_NO_REFLECTION
      /// Constructor that creates a socket monitor.
      ///
      static MMonitorSocket* DoNew0()
      {
         return M_NEW MMonitorSocket;
      }

      static MMonitorSocket* DoNew1(const MStdString& clientAddress)
      {
         return M_NEW MMonitorSocket(clientAddress);
      }
   #endif

M_START_PROPERTIES(MonitorSocket)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT (MonitorSocket, IsSocketOpen)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT (MonitorSocket, IsAddressLocal)
   M_OBJECT_PROPERTY_STRING              (MonitorSocket, ClientAddress, ST_constMStdStringA_X, ST_X_constMStdStringA)
M_START_METHODS(MonitorSocket)
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (MonitorSocket, New, DoNew1, 1,  ST_MObjectP_S_constMStdStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (MonitorSocket, New, DoNew0, 0,  ST_MObjectP_S)
M_END_CLASS(MonitorSocket, MonitorFile)

MMonitorSocket::MMonitorSocket(const MStdString& clientAddress)
:
   m_host(),
   m_clientAddress(),
   m_noSocketOpenFailed(1),
   m_nextTimeToConnect(0)
{
   SetClientAddress(clientAddress);
}

MMonitorSocket::~MMonitorSocket()
{
   DoFinish(); // every destructor has to call this service
   m_socket.Close();
}

bool MMonitorSocket::IsSocketOpen() const
{
   return m_socket.IsOpen();
}

bool MMonitorSocket::IsAddressLocal() const
{
   try
   {
      MConstChars hostChars = m_host.c_str();
      return m_stricmp(hostChars, str_LOCALHOST) == 0 ||
             m_stricmp(hostChars, str_127_0_0_1) == 0 ||
             m_stricmp(hostChars, MStreamSocketBase::GetLocalName().c_str()) == 0 ||
             m_stricmp(hostChars, MStreamSocketBase::GetLocalAddress().c_str()) == 0;
   }
   catch ( ... ) // ignore possible socket errors, like if the socket library is not installed
   {
   }
   return false; // assume the address is not local in case there was a socket error
}


void MMonitorSocket::SetClientAddress(const MStdString& address)
{
   if ( address.empty() )
   {
         m_clientAddress = str_LOCALHOST;
   }
   else
      m_clientAddress = address;

   MStdString oldHost = m_host;
   if ( m_stricmp(m_clientAddress.c_str(), str_LOCALHOST) == 0 || m_clientAddress == str_127_0_0_1 )
      m_host = MUtilities::GetLocalHostName();
   else
      m_host = m_clientAddress;

   if ( oldHost != m_host )
      m_socket.Close(); // Close will succeed if not open. Cause retargeting of the monitor.
}

void MMonitorSocket::Attach(const MStdString& mediaIdentification)
{
   m_mediaIdentification = mediaIdentification;
   MMonitorFile::Attach(m_mediaIdentification);
}

void MMonitorSocket::Detach()
{
   MMonitorFile::Detach();
   m_socket.Close(); // Close is always successful
   m_mediaIdentification.clear();
}

   class MMonitorSocketConnectionHandler : public MStreamSocket::OperationHandler
   {
      MMonitorSocket* m_monitor;

   public:

      MMonitorSocketConnectionHandler(MMonitorSocket* monitor)
      :
         m_monitor(monitor)
      {
         M_ASSERT(m_monitor != NULL);
      }

      virtual void CheckIfCancelled()
      {
         if ( m_monitor->m_listening == 0 || m_monitor->m_isFinished )
         {
            MEOperationCancelled::Throw();
            M_ENSURED_ASSERT(0);
         }
      }
   };

unsigned MMonitorSocket::DoSendBackgroundBuffer(const MByteString& backgroundThreadBuffer)
{
   unsigned ret = MMonitorFile::DoSendBackgroundBuffer(backgroundThreadBuffer);

   M_ASSERT(m_listening != 0);
   try
   {
      m_socket.Write(backgroundThreadBuffer);
   }
   catch ( MException& ex )
   {
      M_USED_VARIABLE(ex); // this way allow debugging
      try // Intentionally ignore errors
      {
         m_socket.Close();
      }
      catch ( ... )
      {
         M_ASSERT(0);  // Warn on debug only
      }

      try
      {
         MMonitorSocketConnectionHandler handler(this);
         m_socket.ConnectInterruptible(s_PORT, m_host, &handler);
         Attach(m_mediaIdentification); // resend the media information
         if ( !m_syncMessagePosted )
            PostSyncMessage();
         m_socket.Write(backgroundThreadBuffer);
         return (unsigned)-1; // Success, we are interested in data...
      }
      catch ( MException& ex )
      {
         M_USED_VARIABLE(ex);
         m_socket.Close();
      }
      return ret;
   }
   return (unsigned)-1; // Success, we are interested in data...
}

#endif // !M_NO_MCOM_MONITOR
