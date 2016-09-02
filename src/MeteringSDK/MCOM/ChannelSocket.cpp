// File MCOM/ChannelSocket.cpp

#include "MCOMExtern.h"
#include "ChannelSocket.h"
#include "MCOMExceptions.h"
#include <MCORE/MEvent.h>
#include <MCORE/MTimer.h>

#if !M_NO_MCOM_CHANNEL_SOCKET

#if defined(WSAECONNRESET) // Windows, presumably. Has to be checked first.
   #define M__ECONNRESET    WSAECONNRESET
   #define M__ECONNABORTED  WSAECONNABORTED
#elif defined(ECONNRESET)
   #define M__ECONNRESET   ECONNRESET
   #define M__ECONNABORTED ECONNABORTED
#else
   #error "The define for ECONNRESET is not found"
#endif

#if (M_OS & M_OS_WINDOWS) != 0
   #ifndef MSG_PEEK
      #define MSG_PEEK 0x2 // Borland does not have it
   #elif MSG_PEEK != 0x2
      #error "Bad value guess for MSG_PEEK!"
   #endif
   #if !defined(EPIPE) // some Windows environments with old SDK
      #define EPIPE 32 // taken from Microsoft headers
   #elif EPIPE != 32
      #error "The define for EPIPE was not correct"
   #endif
#endif

using namespace std;

M_START_PROPERTIES(ChannelSocket)
   M_OBJECT_PROPERTY_PERSISTENT_STRING(ChannelSocket, ProxyString,    "", ST_constMStdStringA_X,  ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_PERSISTENT_UINT  (ChannelSocket, ConnectTimeout, 0)
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   M_OBJECT_PROPERTY_PERSISTENT_BOOL  (ChannelSocket, HandlePeerDisconnect,   true)
#endif
M_START_METHODS(ChannelSocket)
M_END_CLASS_TYPED(ChannelSocket, ChannelSocketBase, "CHANNEL_SOCKET")

#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT

// #define M__MCOM_TRACE_BACKGROUND_HANDLER 1

   // The class that handles sockets on the background to process the incoming FIN.
   //
   class MChannelSocketBackgroundHandler : public MThreadWorker
   {
   public: // Types:

      // For how often to check the channels
      //
      #if M_DEBUG
         static const unsigned c_checkPeriodMilliseconds = 500; // every half-second in debug
      #else
         static const unsigned c_checkPeriodMilliseconds = 5000; // every five seconds in release
      #endif

   private: // Types:

      typedef vector<MChannelSocket*> SocketChannelVector;

   public: // Constructor, destructor:

      MChannelSocketBackgroundHandler()
      {
         M_ASSERT(s_self == NULL);
         Start();
      }

      ~MChannelSocketBackgroundHandler()
      {
         #if M_DEBUG
            M_ASSERT(m_channels.empty()); // If you have this, there is a memory leak and a channel is not deleted
         #endif
         m_eventExit.Set();                // wake up the thread and finish processing, if there was some
         WaitUntilFinished(false); // do not throw
      }

   public:

      static void Register(MChannelSocket* chan)
      {
         MCriticalSection::Locker locker(s_lock);
         if ( s_self == NULL )
            s_self = M_NEW MChannelSocketBackgroundHandler;

         if ( std::find(s_self->m_channels.begin(), s_self->m_channels.end(), chan) == s_self->m_channels.end() )
            s_self->m_channels.push_back(chan);
      }

      static void Unregister(MChannelSocket* chan)
      {
         MChannelSocketBackgroundHandler* thisToDelete = NULL; // if this is not NULL, delete it

         // Critical section-protected part
         {
            MCriticalSection::Locker locker(s_lock);
            if ( s_self != NULL )
            {
               SocketChannelVector* channels = &s_self->m_channels;
               SocketChannelVector::iterator it = find(channels->begin(), channels->end(), chan);
               if ( it != channels->end() )
               {
                  channels->erase(it);
                  if ( channels->empty() )
                  {
                     thisToDelete = s_self;
                     s_self = NULL; // nothing more to watch, schedule this item for deletion
                  }
               }
            }
         }

         // Delete this outside of the critical section, and after NULL is safely assigned to s_self...
         //
         delete thisToDelete; // this can be NULL, in which case nothing is done...
      }

   private:

      virtual void Run()
      {
         while ( !m_eventExit.LockWithTimeout(c_checkPeriodMilliseconds) ) // this thread wakes up quite rarely
         {
            MCriticalSection::Locker vectorLocker(s_lock);
            SocketChannelVector::iterator it = m_channels.begin();
            SocketChannelVector::iterator itEnd = m_channels.end();
            #ifdef M__MCOM_TRACE_BACKGROUND_HANDLER
               MTimer printTimer;
            #endif
            for ( ; it < itEnd; ++it )
            {
               MChannelSocket* c = *it;
               if ( c->IsConnected() && c->GetHandlePeerDisconnect() && c->m_channelOperationCriticalSection.TryLock() )
               {
                  try
                  {
                     MStreamSocket& socket = c->GetSocket();
                     if ( socket.WaitToReceive(0) )  // if any bytes are immediately available
                     {
                        char buf [ 1 ]; // peek at a single byte, this is enough to detect end of stream
                        int size = socket.Recv(buf, sizeof(buf), MSG_PEEK); // look at the data, without actually fetching
                        if ( size == 0 ) // peer closed the socket
                        {
                           c->m_closedByBackgroundHandler = true;
                           c->m_socket.Close(); // Cannot use Disconnect because it flushes buffers, potentially do RAS disconnect
                           #ifdef M__MCOM_TRACE_BACKGROUND_HANDLER
                              printf("!!!############### Peer %p closed its end of connection\n", c);
                           #endif
                        }
                     }
                  }
                  catch ( MException& ex )
                  {
                     // ignore all sorts of errors that can raise, typically, socket related system errors
                     M_USED_VARIABLE(ex); // facilitate debugging
                  }
                  c->m_channelOperationCriticalSection.Unlock();
               }
            }
            #ifdef M__MCOM_TRACE_BACKGROUND_HANDLER
               if ( printTimer > 20 )
               {
                  printf("!!!=================== Execution of background thread is %u\n", diff);
                  printTimer.ResetTimer();
               }
            #endif
         }
      }

   private: // Data:

      // Event which is signaled at exit condition
      //
      MEvent m_eventExit;

      // Mutual exclusion object to exclude the possibility for accessing
      // the facilities of this class from multiple threads at the same time.
      // This is made static, as s_self is also handled here.
      //
      static MCriticalSection s_lock;

      // List of channels managed by the thread
      //
      SocketChannelVector m_channels;

      // Reference to Self of the singleton object.
      //
      static MChannelSocketBackgroundHandler* s_self;
   };

   MCriticalSection MChannelSocketBackgroundHandler::s_lock;
   MChannelSocketBackgroundHandler* MChannelSocketBackgroundHandler::s_self = NULL;
#endif

MChannelSocket::MChannelSocket()
:
   MChannelSocketBase(),
   m_socket()
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   , m_handlePeerDisconnect(true)  // initialize it here so the persistent property assignment does not bother to loop
   , m_closedByBackgroundHandler(false)
#endif
{
   m_socketPtr = &m_socket;
   M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(ChannelSocket);
}

MChannelSocket::~MChannelSocket()
{
   Disconnect();
}

void MChannelSocket::SetSocket(MStreamSocketBase& other)
{
   MStreamSocket* sock = M_DYNAMIC_CAST_WITH_THROW(MStreamSocket, &other); // check type casting
   m_socket.Swap(*sock);
   if ( m_socket.IsOpen() )
      DoInitChannel();

#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   m_closedByBackgroundHandler = false;
   if ( m_handlePeerDisconnect && IsConnected() ) // if we set a connected handle
      MChannelSocketBackgroundHandler::Register(this);
#endif
}

#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
void MChannelSocket::SetHandlePeerDisconnect(bool yes)
{
   if ( m_handlePeerDisconnect != yes ) // if we change value
   {
      if ( !yes )
         MChannelSocketBackgroundHandler::Unregister(this);
      else if ( IsConnected() ) // if we start handling on a connected socket
         MChannelSocketBackgroundHandler::Register(this);
      m_handlePeerDisconnect = yes;
   }
}
#endif

   static const unsigned s_eternitySeconds = static_cast<unsigned>(INT_MAX / 1000); // how many seconds are the same as eternity
   static const int      s_longConnectMilliseconds = 8000; // consider retryable only the errors that are taking this long to raise

   class MChannelConnectionHandler : public MStreamSocket::OperationHandler
   {
   private:

      MChannelSocket* m_channel;
      unsigned        m_timeout;
      MTimer          m_timer;

   public:

      MChannelConnectionHandler(MChannelSocket* channel)
      :
         m_channel(channel),
         m_timeout(channel->GetConnectTimeout()),
         m_timer(m_timeout * 1000)
      {
         M_ASSERT(m_channel != NULL);
      }

      bool ShouldTryConnectingAgain() const
      {
         return (m_timeout >= s_eternitySeconds) ||
                (m_timeout != 0 && !m_timer.IsExpired());
      }

      virtual void CheckIfCancelled()
      {
         m_channel->CheckIfOperationIsCancelled();
         if ( m_timeout != 0 &&
              m_timeout < s_eternitySeconds &&  // when the given timeout is more than this consider it to be an eternity
              m_timer.IsExpired() )
         {
            MCOMException::Throw(MErrorEnum::ChannelConnectTimeout, M_I("Failed to connect within %u seconds"), m_timeout);
            M_ENSURED_ASSERT(0);
         }
      }
   };

void MChannelSocket::Connect()
{
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   m_closedByBackgroundHandler = false;
#endif
   MChannelSocketBase::Connect();

   if ( m_isAutoAnswer )
      WaitForNextIncomingConnection();
   else
   {
      MTimer onePassTimer;
      MESocketError exSaved;
      MChannelConnectionHandler connectionHandler(this);
      for ( ; ; )
      {
         try
         {
            #if !M_NO_SOCKETS_SOCKS
               m_socket.ConnectWithProxyInterruptible(m_peerPort, m_peerAddress, m_proxyString, &connectionHandler);
            #else
               m_socket.ConnectInterruptible(m_peerPort, m_peerAddress, &connectionHandler);
            #endif
            break; // success
         }
         catch ( MESocketError& ex ) // the only type of error to catch here
         {
            M_USED_VARIABLE(ex); // help the debugger
            if ( onePassTimer.GetTimer() < s_longConnectMilliseconds || // this has to be tried first
                 !connectionHandler.ShouldTryConnectingAgain() )        // this should be tried next
            {
               throw;
            }
            exSaved = ex;
            onePassTimer.ResetTimer();
         }
         catch ( MCOMException& ex )
         {
            if ( ex.GetCode() == MErrorEnum::ChannelConnectTimeout && exSaved.GetSocketErrorCode() != 0 )
               exSaved.Rethrow();
            throw; // otherwise throw the original exception
         }
      }
      m_socket.SetSendTimeout(GetWriteTimeout());

      DoNotifyConnect();
   }
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   if ( m_handlePeerDisconnect )
      MChannelSocketBackgroundHandler::Register(this);
#endif
}

void MChannelSocket::WaitForNextIncomingConnection(bool)
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

   MStreamSocket serverSocket;
   serverSocket.Bind(m_autoAnswerPort, m_autoAnswerAddress);
   serverSocket.Listen(1); // listen for only one incoming call

   MTimer endTime(MTimer::SecondsToTimerMilliseconds(m_autoAnswerTimeout)); // timeout is in seconds
   while ( !serverSocket.TimedAccept(m_socket, 250) )
   {
      CheckIfOperationIsCancelled();
      if ( endTime.IsExpired() )
      {
         M_ASSERT(!m_socket.IsOpen());
         MCOMException::Throw(M_CODE_STR(M_ERR_TIMED_OUT_WHILE_WAITING_FOR_CONNECTION, M_I("Timed out while waiting for incoming socket connection")));
         M_ENSURED_ASSERT(0);
      }
   }

   DoNotifyConnect();
}

void MChannelSocket::Disconnect()
{
   #if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
      // Do it first hand to prevent iterating through a closing channel
      MChannelSocketBackgroundHandler::Unregister(this);
      if ( m_closedByBackgroundHandler )
      {
         M_ASSERT(!m_socket.IsOpen());
         m_closedByBackgroundHandler = false;
         DoNotifyDisconnect();
      }
   #endif

   MChannelSocketBase::Disconnect();
}

M_NORETURN_FUNC void MChannelSocket::DoHandleExceptionAndRethrow(MException& ex)
{
   CheckIfOperationIsCancelled();

#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   if ( m_closedByBackgroundHandler )
   {
      m_closedByBackgroundHandler = false;
      MEChannelDisconnectedUnexpectedly::Throw(); // do not merge with the below code
      M_ENSURED_ASSERT(0);
   }
#endif

   bool unexpectedDisconnect = false;
   if ( ex.GetCode() == MErrorEnum::SocketClosedByPeer )
      unexpectedDisconnect = true;
   else
   {
      const MESocketError* socketError = M_DYNAMIC_CAST(const MESocketError, &ex);
      if ( socketError != NULL )
      {
         unsigned code = socketError->GetSocketErrorCode();
         if ( code == M__ECONNRESET || code == M__ECONNABORTED || code  == EPIPE )
         {
            unexpectedDisconnect = true;
         }
      }
   }
   if ( unexpectedDisconnect )
   {
      #if !M_NO_VERBOSE_ERROR_INFORMATION
         MEChannelDisconnectedUnexpectedly e;
         e.AppendToString(". ");
         e.Append(ex.AsString());
         e.Rethrow();
      #else
         MEChannelDisconnectedUnexpectedly::Throw();
      #endif
   }
   else 
      MChannelSocketBase::DoHandleExceptionAndRethrow(ex);
   M_ENSURED_ASSERT(0);
}

void MChannelSocket::CheckIfConnected()
{
#if !M_NO_MCOM_HANDLE_PEER_DISCONNECT
   if ( m_closedByBackgroundHandler )
   {
      m_closedByBackgroundHandler = false;
      MEChannelDisconnectedUnexpectedly::Throw(); // do not merge with the below code
      M_ENSURED_ASSERT(0);
   }
#endif
   MChannel::CheckIfConnected();
}

#endif // !M_NO_MCOM_CHANNEL_SOCKET
