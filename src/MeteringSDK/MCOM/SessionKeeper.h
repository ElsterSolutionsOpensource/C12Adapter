#ifndef MCOM_SESSIONKEEPER_H
#define MCOM_SESSIONKEEPER_H
/// \addtogroup MCOM
///@{
/// \file MCOM/SessionKeeper.h

#include <MCOM/MCOMDefs.h>

/// \cond SHOW_INTERNAL

// Class that facilitates handling of protocol layers.
//
class MCOM_CLASS MProtocolLayerWrapper
{
protected: // Constructor:

   // Constructor. Takes the protocol that is to be layered, and whether this is the app-layer wrapper or link layer wrapper.
   //
   MProtocolLayerWrapper(MProtocol* proto);

   // Destructor. 
   // Notice the destructor is protected. No MProtocolLayerWrapper shall be destroyed by root class.
   //
   virtual ~MProtocolLayerWrapper() M_NO_THROW;

public: // Services:

   // Handle failure of this application level service, do not throw an exception, only modify it.
   //
   virtual void HandleFailureNoThrow(MException& ex) M_NO_THROW = 0;

   // Only set status to failed, and do nothing else
   //
   virtual void HandleFailureSilently();

   // Handle failure of this application level service, and throw it as a modified exception.
   //
   M_NORETURN_FUNC void HandleFailureAndRethrow(MException& ex);

   // Prepend Retries expired message before the exception text
   //
#if !M_NO_VERBOSE_ERROR_INFORMATION
   static void PrependRetriesExpired(MException& ex);
#else
   static void PrependRetriesExpired(MException&)
   {
   }
#endif // !M_NO_VERBOSE_ERROR_INFORMATION

protected: 

   // Implementation method that throws a given exception if it is not of the kind that can be retried.
   // Whether or not a communication-related error is retryable or not is given as parameter.
   //
   static void DoThrowIfNotRetryable(MException& ex, bool communicationErrorIsRetryable);

private: // Copying preventors:

   MProtocolLayerWrapper();
   MProtocolLayerWrapper(const MProtocolLayerWrapper&);
   void operator=(const MProtocolLayerWrapper&);

protected: // Data:

   // Protocol for which the handling is done.
   //
   MProtocol* m_protocol;

#if !M_NO_MCOM_MONITOR

   // Monitor that is used to show notifications
   //
   MMonitor* m_monitor;

#endif

   // Service failed, NotifyFailure was called
   //
   bool m_failed;
};

// Class that facilitates handling of protocol link layers.
//
class MCOM_CLASS MProtocolLinkLayerWrapper : public MProtocolLayerWrapper
{
public: // Constructor, destructor:

   // Constructor. Takes the protocol that is to be layered, and whether this is the app-layer wrapper or link layer wrapper.
   //
   MProtocolLinkLayerWrapper(MProtocol* proto);

   // Destructor.
   //
   virtual ~MProtocolLinkLayerWrapper() M_NO_THROW;

   // Handle failure of this application level service, do not throw an exception, only modify it.
   //
   virtual void HandleFailureNoThrow(MException& ex) M_NO_THROW;

   // Notify a notify a link layer retry, using exception as a reason.
   //
   void NotifyOrThrowRetry(MException& reasonException, unsigned retries);

#if !M_NO_VERBOSE_ERROR_INFORMATION
   // Notify a link layer retry, using exception as a reason.
   //
   void NotifyRetry(const MException& reasonException) M_NO_THROW;

   // Notify about link layer event, not a retry.
   //
   void NotifyRetry(const MStdString& str) M_NO_THROW;
#else
   void NotifyRetry(const MException&)
   {
   }
   void NotifyRetry(const MStdString&)
   {
   }
#endif // !M_NO_VERBOSE_ERROR_INFORMATION

   // Throw a given exception if it is not of the kind that can be retried.
   // For link layer wrapper, not retryable are non-communication errors.
   //
   static void ThrowIfNotRetryable(MException& ex)
   {
      DoThrowIfNotRetryable(ex, true); // communication error is retryable
   }
};

// Keeper helper class that shall wrap every protocol service.
//
class MCOM_CLASS MProtocolServiceWrapper : public MProtocolLayerWrapper
{
public: // Types:

   // Possible flags of the service
   //
   enum FlagsMask
   {
      ServiceOrdinary             = 0, // Ordinary service
      ServiceStartsSessionKeeping = 1, // Service starts session keeping (cannot be combined with the next mask)
      ServiceEndsSessionKeeping   = 2, // Service ends session keeping (cannot be combined with the previous mask)
      ServiceNotQueueable         = 4  // Service is not queueable, no check for background communication is to be done
   };

   // Type for stack of service wrappers, handled by protocol.
   //
   typedef std::vector<MProtocolServiceWrapper*>
      Stack;

public: // Constructor and destructor:

   // Constructor for application layer wrapper. Takes the protocol that is to be kept protected.
   // Service name, if no string, means the service will not be reported on the monitor.
   // If endsSession is true, this service does not continue session.
   //
   MProtocolServiceWrapper(MProtocol* proto, MConstChars serviceName = NULL, unsigned flags = ServiceOrdinary);

   // Constructor for application layer wrapper. Takes the protocol that is to be kept protected.
   // Service name, number and two extra parameters are used to build service signature.
   // FlagsMask is always ServiceOrdinary for this service.
   //
   MProtocolServiceWrapper(MProtocol* proto, MConstChars serviceName, MCOMNumberConstRef number, int i1, int i2);

   // Destructor. Unlocks the resource and sets the timer.
   //
   virtual ~MProtocolServiceWrapper() M_NO_THROW;

   // Handle failure of this application level service, do not throw an exception, only modify it.
   //
   virtual void HandleFailureNoThrow(MException& ex) M_NO_THROW;

   // If the retry count is zero, throw an error as retries expired,
   // otherwise notify the error with monitor and return.
   //
   static void StaticNotifyOrThrowRetry(MProtocol*, MException& reasonException, unsigned retryCount);

   // Notify a notify a link layer retry, using exception as a reason.
   //
   void NotifyOrThrowRetry(MException& reasonException, unsigned retryCount);

   // Handle failure of this application level service, and throw it as a modified exception.
   //
   M_NORETURN_FUNC static void StaticHandleFailureAndRethrow(MProtocol*, MException& ex);

   // Throw a given exception if it is not of the kind that can be retried.
   // For application layer, communication errors are not retryable.
   //
   void ThrowIfNotRetryable(MException& ex)
   {
      DoThrowIfNotRetryable(ex, false); // app-layer error is not retryable
   }

   virtual void HandleFailureSilently();

private: // Methods:

   // Access the top level wrapper of protocol.
   //
   static MProtocolServiceWrapper* DoGetTopLevelWrapper(MProtocol* proto);

   // Initializer helper, callable from constructors.
   //
   void DoInit();

private: // Data:

#if !M_NO_VERBOSE_ERROR_INFORMATION
   // Service name
   //
   MStdString m_name;
#endif

   // Flags for this service
   //
   unsigned m_flags;

   // Whether after the failure the protocol should be set to be outside session
   //
   bool m_dropSessionAfterFailure;
};

#if !M_NO_MCOM_KEEP_SESSION_ALIVE

// Class that provides KeepSessionAlive functionality.
// Keeper starts a new thread which waits for the event that monitors any channel activity.
// Every instance of SessionKeeper must be linked to suitable MProtocol object. 
// Such protocol shall implement two methods: DoGetKeepSessionAliveFirstDelay and DoSendKeepSessionAliveMessage.
//
class MCOM_CLASS MSessionKeeper : public MThreadWorker
{
public: // Classes and types:

private: // Type:

   // Current state of the session keeper background thread
   //
   enum BackgroundThreadState
   {
      StateExiting    = -1, // Keeper thread is exiting
      StateNotRunning = 0,  // Keeper thread is not running
      StateNotKeeping = 1,  // Keeper thread is running, but not keeping session
      StateKeeping    = 2   // Keeper thread is running and keeping session
   };

public: // Constructor and destructor:

   // Session keeper constructor that takes the client protocol object.
   //
   MSessionKeeper(MProtocol* proto)
   :
      m_wakeup(),
      m_protocol(proto),
      m_keepSessionAlive(false),
      m_state(StateNotRunning),
      m_keeperCounter(0),
      m_nextEventTick(0),
      m_exception(NULL)
   {
      M_ASSERT(proto != NULL);
   }

   // Session keeper destructor.
   //
   virtual ~MSessionKeeper();

public: // Properties:

   //@{
   // Keep session alive property.
   //
   bool GetKeepSessionAlive() const
   {
      return m_keepSessionAlive;
   }
   void SetKeepSessionAlive(bool yes);
   //@}

   // Check if any errors have appeared during keeping, and throw them
   //
   void CheckAndThrowErrors();

   // Call that declares the service is entered.
   //
   // \pre Each entering of the session shall be accompanied with a
   // matching call of LeaveService. Exceptions can be thrown if there were errors during session keeping.
   //
   // \post The service is reported as started.
   // Any session keeping interrupts until the session is left.
   // 
   void EnterService();

   // Call that declares the service is left.
   //
   // \pre Each leaving of the session shall Finish a
   // matching call of EnterService. No exceptions can be thrown at this step.
   //
   // \post The service is reported as left.
   // Any active session keeping restarts, if appropriate.
   // 
   void LeaveService() M_NO_THROW;

   // Cancel the possibly going keeping
   //
   void CancelService() M_NO_THROW;

   void InitStopKeeping();
   void FinalizeStopKeeping() M_NO_THROW;

protected:

   // Clear session keeping thread error.
   //
   void ClearErrorIfAny() M_NO_THROW;

   // Thread procedure that fulfills the purpose of session keeping.
   //
   virtual void Run();

private:

   // Protect accessing to the below fields in a multithreaded environment.
   //
   MCriticalSection m_lock;

   // Wakeup event
   //
   MEvent m_wakeup;

   // Client protocol
   //
   MProtocol* m_protocol;

   // Whether the session shall be kept alive
   //
   bool m_keepSessionAlive;

   // Current state of the background thread
   //
   BackgroundThreadState m_state;

   // Counter that is incremented as service is being called.
   // This is to handle the case when one service calls another.
   //
   int m_keeperCounter;

   // Next milliseconds tick at which to start the keeping event.
   //
   unsigned m_nextEventTick;

   // Exception caught by the background thread and rethrown by the foreground thread.
   //
   mutable MException* m_exception;
};

#endif

/// \endcond SHOW_INTERNAL

///@}
#endif
