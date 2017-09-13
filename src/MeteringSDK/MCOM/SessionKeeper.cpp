// File MCOM/SessionKeeper.h

#include "MCOMExtern.h"
#include "Protocol.h"
#include "SessionKeeper.h"
#include "MCOMExceptions.h"

MProtocolLayerWrapper::MProtocolLayerWrapper(MProtocol* proto)
:
   m_protocol(proto),
#if !M_NO_MCOM_MONITOR
   m_monitor(NULL), // nullify so there is no problem if an error is thrown
#endif
   m_failed(true) // at start assume the service failed to start (this will be overwritten later)
{
   m_protocol->DoCheckChannel(true); // check the channel is present, but do not verify there is no background communication
   MChannel* chan = m_protocol->GetChannel();
   chan->CheckIfConnected();
   #if !M_NO_MCOM_MONITOR
      MMonitor* mon = chan->GetMonitor();
      if ( mon != NULL )
         m_monitor = mon;
   #endif
}

MProtocolLayerWrapper::~MProtocolLayerWrapper() M_NO_THROW
{
}

M_NORETURN_FUNC void MProtocolLayerWrapper::HandleFailureAndRethrow(MException& ex)
{
   HandleFailureNoThrow(ex);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

void MProtocolLayerWrapper::DoThrowIfNotRetryable(MException& ex, bool communicationErrorIsRetryable)
{
   bool doThrow = (communicationErrorIsRetryable != (ex.GetKind() == MException::ErrorCommunication));
   if ( !doThrow )
   {
      const MClass* cls = ex.GetClass();
      doThrow = cls->IsKindOf(MEOperationCancelled::GetStaticClass()) || 
                cls->IsKindOf(MEChannelDisconnectedUnexpectedly::GetStaticClass()) || 
                cls->IsKindOf(MECollisionDetected::GetStaticClass());
   }
   if ( doThrow )
   {
      ex.Rethrow(); // do not signal link layer failure because there was not one
      M_ENSURED_ASSERT(0);
   }
}

void MProtocolLayerWrapper::HandleFailureSilently()
{
   m_failed = true;
}

#if !M_NO_VERBOSE_ERROR_INFORMATION
void MProtocolLayerWrapper::PrependRetriesExpired(MException& ex)
{
   // We have to make sure we do not prepend the string twice
   //
   MStdString prepend = M_I_STR(M_I("Retries expired with error: "));
   if ( m_strncmp(ex.AsString().c_str(), prepend.c_str(), prepend.size()) != 0 )
      ex.Prepend(prepend);
}
#endif

MProtocolLinkLayerWrapper::MProtocolLinkLayerWrapper(MProtocol* proto)
:
   MProtocolLayerWrapper(proto)
{
//?? Currently, there is no OnDataLinkLayerStart
//   #if !M_NO_MCOM_MONITOR
//      if ( m_monitor != NULL )
//         m_monitor->OnDataLinkLayerStart();
//   #endif
   m_failed = false; // created wrapper successfully
}

MProtocolLinkLayerWrapper::~MProtocolLinkLayerWrapper() M_NO_THROW
{
   if ( !m_failed && m_protocol != NULL )
   {
      try
      {
         m_protocol->IncrementCountLinkLayerPacketsSuccessful();
         #if !M_NO_MCOM_MONITOR
            if ( m_monitor != NULL )
               m_monitor->OnDataLinkLayerSuccess();
         #endif
      }
      catch ( ... )
      {
         M_ASSERT(0);
      }
   }
}

void MProtocolLinkLayerWrapper::HandleFailureNoThrow(MException& ex) M_NO_THROW
{
   M_ASSERT(m_protocol != NULL);
   if ( !m_failed ) // if we have not reported the failure yet
   {
      m_failed = true;
      try
      {
         m_protocol->IncrementCountLinkLayerPacketsFailed();
         #if !M_NO_MCOM_MONITOR
            if ( m_monitor != NULL )
               m_monitor->OnDataLinkLayerFail(ex.AsString());
         #endif
      }
      catch ( ... )
      {
         M_ASSERT(0);
      }
   }
}

void MProtocolLinkLayerWrapper::NotifyOrThrowRetry(MException& reasonException, unsigned retries)
{
   ThrowIfNotRetryable(reasonException);
   if ( retries == 0 )
   {
      PrependRetriesExpired(reasonException);
      HandleFailureAndRethrow(reasonException);
      M_ENSURED_ASSERT(0);
   }
   else
      NotifyRetry(reasonException);
}

#if !M_NO_VERBOSE_ERROR_INFORMATION
void MProtocolLinkLayerWrapper::NotifyRetry(const MException& reasonException) M_NO_THROW
{
   NotifyRetry(reasonException.AsString());
}

void MProtocolLinkLayerWrapper::NotifyRetry(const MStdString& str) M_NO_THROW
{
   try
   {
      m_protocol->IncrementCountLinkLayerPacketsRetried();
      #if !M_NO_MCOM_MONITOR
         if ( m_monitor != NULL  )
            m_monitor->OnDataLinkLayerRetry(str);
      #endif
   }
   catch ( ... )
   {
      M_ASSERT(0);
   }
}
#endif // !M_NO_VERBOSE_ERROR_INFORMATION

MProtocolServiceWrapper::MProtocolServiceWrapper(MProtocol* proto, MConstChars name, unsigned flags)
:
   MProtocolLayerWrapper(proto),
   m_flags(flags),
   m_dropSessionAfterFailure(false)
{
#if !M_NO_VERBOSE_ERROR_INFORMATION
   if ( name != NULL )
   {
      M_ASSERT(name[0] != '\0');
      m_name = name;
   }
#else
   M_ASSERT(name == NULL);
#endif
#if !M_NO_MCOM_MONITOR
   else
      m_monitor = NULL; // do not show anything on monitor if service has no name
#endif
   DoInit();
}

MProtocolServiceWrapper::MProtocolServiceWrapper(MProtocol* proto, MConstChars serviceName, MCOMNumberConstRef number, int i1, int i2)
:
   MProtocolLayerWrapper(proto),
   m_flags(ServiceOrdinary),
   m_dropSessionAfterFailure(false)
{
#if !M_NO_VERBOSE_ERROR_INFORMATION
   M_ASSERT(serviceName != NULL && serviceName[0] != '\0');

   char name [ MProtocol::MAXIMUM_SERVICE_NAME_STRING_SIZE ];
   m_protocol->DoBuildComplexServiceName(name, serviceName, number, i1, i2);
   m_name = name;
   M_ASSERT(m_name.size() < MProtocol::MAXIMUM_SERVICE_NAME_STRING_SIZE); // Check if MAXIMUM_SERVICE_NAME_STRING_SIZE is big enough
#else
   M_ASSERT(serviceName == NULL);
#endif
   DoInit();
}

void MProtocolServiceWrapper::DoInit()
{
   M_ASSERT(m_protocol != NULL);
   M_ASSERT(m_failed); // service is assumed failed at this point
   M_ASSERT((m_flags & (ServiceEndsSessionKeeping | ServiceStartsSessionKeeping)) != (ServiceEndsSessionKeeping | ServiceStartsSessionKeeping)); // The two flags are incompatible

   MChannel* chan = m_protocol->GetChannel();
   M_ASSERT(chan != NULL); // ensured by the previous call in constructor
   try
   {
      chan->CheckIfConnected();
      chan->CheckIfOperationIsCancelled();

      if ( (m_flags & ServiceNotQueueable) == 0 ) // for not queuable services do not perform this check
         m_protocol->DoCheckChannel();

      #if !M_NO_MCOM_KEEP_SESSION_ALIVE
         // Next, notify possible session keeper thread that the service is entered
         m_protocol->m_sessionKeeper.EnterService();
      #endif
      #if !M_NO_MCOM_MONITOR
         // Only then notify the start of the service
         if ( m_monitor != NULL )
         {
            M_ASSERT(!m_name.empty());
            m_monitor->OnApplicationLayerStart(m_name);
         }
      #endif
   }
   catch ( ... )
   {
      m_protocol->m_isInSession = false; // in case of any exception at the init stage ensure we are not in session
      throw;
   }
   m_failed = false; // service is successfully started

   // Last, register this service with protocol
   // No exceptions are throwable by the code above!
   //
   m_protocol->m_serviceWrappers.push_back(this);

}

MProtocolServiceWrapper::~MProtocolServiceWrapper() M_NO_THROW
{
   M_ASSERT(m_protocol != NULL);

   if ( m_protocol->m_serviceWrappers.back() == this ) // most usual case - pure stack
      m_protocol->m_serviceWrappers.pop_back();
   else // otherwise we have sequence
   {
      MProtocolServiceWrapper::Stack::iterator it = std::find(m_protocol->m_serviceWrappers.begin(), m_protocol->m_serviceWrappers.end(), this);
      M_ASSERT(it != m_protocol->m_serviceWrappers.end());
      m_protocol->m_serviceWrappers.erase(it);
   }

   if ( m_failed )
   {
      if ( m_dropSessionAfterFailure )
         m_protocol->m_isInSession = false;
   }
   else
   {
      m_protocol->m_isInSession = ((m_flags & ServiceEndsSessionKeeping) == 0);
      m_protocol->IncrementCountApplicationLayerServicesSuccessful();
      #if !M_NO_MCOM_MONITOR
         if ( m_monitor != NULL )
         {
            M_ASSERT(!m_name.empty());
            try
            {
               m_monitor->OnApplicationLayerSuccess(m_name);
            }
            catch ( ... )
            {
               M_ASSERT(0);
            }
         }
      #endif
   }

   #if !M_NO_MCOM_KEEP_SESSION_ALIVE
      // At last (and always) notify the session keeper we are about to end the service
      m_protocol->m_sessionKeeper.LeaveService();
   #endif
}

void MProtocolServiceWrapper::HandleFailureNoThrow(MException& ex) M_NO_THROW
{
   if ( !m_failed ) // if we have not reported the failure already
   {
      m_failed = true;

      // Determine whether to drop session after application layer failure, if there is an application layer failure.
      // According to present knowledge, only C12 protocols maintain session after application layer error.
      //
      MEC12NokResponse* c12e = M_DYNAMIC_CAST(MEC12NokResponse, &ex);
      if ( c12e != NULL )
      {
         MEC12NokResponse::ResponseCode code = c12e->GetResponseCode();
         m_dropSessionAfterFailure = (m_flags & ServiceStartsSessionKeeping) != 0 ||
                                     (code == MEC12NokResponse::RESPONSE_RNO || code == MEC12NokResponse::RESPONSE_ISSS || code == MEC12NokResponse::RESPONSE_SME);
      }
      else if ( M_DYNAMIC_CAST(MEC12BadProcedureResult, &ex) != NULL )
         m_dropSessionAfterFailure = false; // always maintain session after
      else
         m_dropSessionAfterFailure = true;

#if !M_NO_VERBOSE_ERROR_INFORMATION
      if ( !m_name.empty() )
      {
         // We have to make sure we do not append the string twice
         //
         MStdString str = ex.AsString();
         MStdString app = MGetStdString(M_I(" in %s"), m_name.c_str());
         int startPos = int(str.size()) - int(app.size());
         if ( startPos < 0 || m_strncmp(str.c_str() + startPos, app.c_str(), app.size()) != 0 )
            ex.Append(app);
      }
#endif

      m_protocol->IncrementCountApplicationLayerServicesFailed();

      #if !M_NO_MCOM_MONITOR
         if ( m_monitor != NULL )
         {
            try
            {
               m_monitor->OnApplicationLayerFail(ex.AsString());
            }
            catch ( ... )
            {
               M_ASSERT(0);
            }
         }
      #endif
   }
}

void MProtocolServiceWrapper::HandleFailureSilently()
{
   m_dropSessionAfterFailure = false;
   MProtocolLayerWrapper::HandleFailureSilently();
}

MProtocolServiceWrapper* MProtocolServiceWrapper::DoGetTopLevelWrapper(MProtocol* proto)
{
   if ( proto->m_serviceWrappers.empty() )      // but still provide handling in release
      return NULL;
   return proto->m_serviceWrappers.back();
}

void MProtocolServiceWrapper::StaticNotifyOrThrowRetry(MProtocol* proto, MException& ex, unsigned retries)
{
   MProtocolServiceWrapper* wrapper = DoGetTopLevelWrapper(proto);
   if ( wrapper != NULL )
      wrapper->NotifyOrThrowRetry(ex, retries);
   else if ( retries == 0 )
   {
      PrependRetriesExpired(ex);
      proto->IncrementCountApplicationLayerServicesFailed();

      #if !M_NO_MCOM_MONITOR
         if ( proto->GetChannel()->GetMonitor() != NULL )
         {
            try
            {
               proto->GetChannel()->GetMonitor()->OnApplicationLayerFail(ex.AsString());
            }
            catch ( ... )
            {
               M_ASSERT(0);
            }
         }
      #endif
      ex.Rethrow();
      M_ENSURED_ASSERT(0);
   }
   else
   {
      proto->IncrementCountApplicationLayerServicesRetried();
      #if !M_NO_MCOM_MONITOR
         if ( proto->GetChannel()->GetMonitor() != NULL )
         {
            try
            {
               proto->GetChannel()->GetMonitor()->OnApplicationLayerRetry(ex.AsString());
            }
            catch ( ... )
            {
               M_ASSERT(0);
            }
         }
      #endif
   }
}

void MProtocolServiceWrapper::NotifyOrThrowRetry(MException& ex, unsigned retries)
{
   ThrowIfNotRetryable(ex);
   if ( retries == 0 )
   {
      PrependRetriesExpired(ex);
      HandleFailureAndRethrow(ex);
      M_ENSURED_ASSERT(0);
   }
   else
   {
      m_protocol->IncrementCountApplicationLayerServicesRetried();
      #if !M_NO_MCOM_MONITOR
         if ( m_monitor != NULL )
            m_monitor->OnApplicationLayerRetry(ex.AsString());
      #endif
   }
}

M_NORETURN_FUNC void MProtocolServiceWrapper::StaticHandleFailureAndRethrow(MProtocol* proto, MException& ex)
{
   MProtocolServiceWrapper* wrapper = DoGetTopLevelWrapper(proto);
   if ( wrapper != NULL )
      wrapper->HandleFailureAndRethrow(ex);
   else
      ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

MSessionKeeper::~MSessionKeeper()
{
   InitStopKeeping();
   FinalizeStopKeeping();
}

void MSessionKeeper::InitStopKeeping()
{
   // Do not place this into criticalSection
   m_state = StateExiting;
   m_wakeup.Set();
}

void MSessionKeeper::FinalizeStopKeeping() M_NO_THROW
{
   try
   {
      M_ASSERT(m_state == StateExiting || m_state == StateNotRunning);
      #if M_OS & M_OS_UCLINUX
         WaitUntilFinished(false);
      #else
         WaitUntilFinished(false, 10000); // timeout of 10 seconds in case the thread takes long to proceed for some crazy reason
      #endif
   }
   catch ( ... )
   {
      M_ASSERT(0);
   }
   ClearErrorIfAny();
   m_state = StateNotRunning;
}

   // Start or stop session keeping, depending on the change of the state.
   // Also, this can throw an exception related to session keeping
   //
   static void DoRefreshSessionKeepingState(MSessionKeeper* keeper)
   {
      // Helper class that emulates a service.
      // Used to change keep session alive state depending on the current protocol state
      //
      struct LocalMiniService
      {
         MSessionKeeper* m_keeper;

         LocalMiniService(MSessionKeeper* sessionKeeper)
         :
            m_keeper(sessionKeeper)
         {
            m_keeper->EnterService(); // this can throw an exception
         }

         ~LocalMiniService() M_NO_THROW
         {
            m_keeper->LeaveService();
         }
      };

      LocalMiniService dummyService(keeper); // Start or stop session keeping, depending on the change of the state. This can throw an exception
   }

void MSessionKeeper::SetKeepSessionAlive(bool yes)
{
   MCriticalSection::Locker locker(m_lock);
   m_keepSessionAlive = yes;
   DoRefreshSessionKeepingState(this);
}

void MSessionKeeper::CheckAndThrowErrors()
{
   MCriticalSection::Locker locker(m_lock);
   if (  m_exception != NULL )
      DoRefreshSessionKeepingState(this);
}

void MSessionKeeper::EnterService()
{
   // Notice this algorithm does not depend on the current value of m_keepSessionAlive
   MCriticalSection::Locker locker(m_lock);

   M_ASSERT(m_keeperCounter == 0 || m_state != StateKeeping); // only if we are on the top there could be keeping active
   if ( m_state == StateKeeping ) // Silence the keeper
      m_state = StateNotKeeping;

   if ( m_exception != NULL )
   {
      try
      {
         M_ASSERT(m_state != StateKeeping); // if there is an error registered, we are not keeping session
         m_exception->Rethrow();
      }
      catch ( ... )
      {
         ClearErrorIfAny(); // clear m_exception variable
         throw;
      }
   }
   ++m_keeperCounter; // raise the level AFTER exception
}

void MSessionKeeper::LeaveService() M_NO_THROW
{
   try
   {
      MCriticalSection::Locker locker(m_lock);

      --m_keeperCounter;
      if ( m_keeperCounter == 0 && m_keepSessionAlive && m_protocol->m_isInSession && m_protocol->IsConnected() ) // if it is time to start keeping
      {
         unsigned firstDelay = m_protocol->DoGetKeepSessionAliveFirstDelay();
         if ( firstDelay == 0 ) // the protocol suggested we do not need to keep the session
            return;             //   do nothing, do not wakeup thread
         if ( m_state == StateNotRunning )
            Start();
         m_nextEventTick = MUtilities::GetTickCount() + firstDelay;
         m_state = StateKeeping;
         m_wakeup.Set();
      }
   }
   catch ( ... )
   {
      M_ASSERT(0); // ignore everything in release
   }
}

void MSessionKeeper::CancelService() M_NO_THROW
{
   MCriticalSection::Locker locker(m_lock);

   M_ASSERT(m_keeperCounter == 0 || m_state != StateKeeping); // only if we are on the top there could be keeping active
   if ( m_state == StateKeeping ) // Silence the keeper
      m_state = StateNotKeeping;
   ClearErrorIfAny();
}

void MSessionKeeper::ClearErrorIfAny() M_NO_THROW
{
   delete m_exception;
   m_exception = NULL;
}

void MSessionKeeper::Run()
{
   long diff;
   for ( ;; )
   {
      m_wakeup.Lock(); // Here we reside when there is no ongoing session keeping

   STATE_HANDLING_CRITICAL_SECTION: // do not replace with loop as there are continue operators
      {                             // that relate to the outer loop
         MCriticalSection::Locker locker(m_lock);
         switch ( m_state )
         {
         default:   // this is (hopefully) a totally impossible case (this includes StateNotRunning)
            M_ASSERT(0);
         case StateExiting:    // Silently exit thread in this case
            return;
         case StateNotKeeping: // The possibility here is while the thread was awakening, a protocol service is entered
            continue;
         case StateKeeping:
            {
               long currentTick = static_cast<long>(MUtilities::GetTickCount());
               diff = m_nextEventTick - currentTick;
               if ( diff <= 20 ) // okay if it is hit 20 milliseconds earlier than necessary
               {
                  try
                  {
                     diff = (long)m_protocol->DoSendKeepSessionAliveMessage();
                     if ( diff == 0 )
                     {
                        m_state = StateNotKeeping;     // Protocol requested to cancel keeping
                        continue;
                     }
                  }
                  catch ( MException& ex )
                  {
                     m_state = StateNotKeeping;     // stop keeping after the very first exception
                     if ( m_exception == NULL && m_protocol->IsConnected() )
                        m_exception = ex.NewClone();
                     continue;
                  }
                  currentTick = MUtilities::GetTickCount();
                  m_nextEventTick = diff + currentTick; // call GetTickCount again, because it can be a lengthy operation
               }
            }
            break; // do a delay outside of critical section
         }
      }

      M_ASSERT(diff > 0); // this assert can actually fail in a rare condition if it took very long to call GetTickCount

      // Here we delay in such a way that if there is a change in the state, we handle the change timely.
      if ( diff > 1000 ) // if difference is bigger than one second
         diff = 1000;    // make a nap for one second, therefore being responsive to changes of state in the foreground object
      if ( diff > 0 ) // see the above assert
         MUtilities::Sleep((unsigned)diff);
      goto STATE_HANDLING_CRITICAL_SECTION;
   }
}

#endif
