#ifndef MCOM_MONITORFILEPRIVATETHREAD_H
#define MCOM_MONITORFILEPRIVATETHREAD_H
// File MCOM/MonitorFilePrivateThread.h

#include <MCOM/MonitorFile.h>

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING
/// \cond SHOW_INTERNAL

// Activity singleton object, a single thread that flushes the collected messages in all monitors.
//
// Walks the existing monitor objects and executes their runner functions,
// ensuring that the monitored events are stored into files and/or sent through the network.
//
// The thread is created at first use, awakes periodically each MONITOR_FLUSH_INTERVAL number of milliseconds.
//
// This is a private class which is not exposed to the end users.
//
class MMonitorFilePrivateThread : public MThreadWorker
{
   friend class MCOM_CLASS MMonitorFile;

private: // Constants:

   enum
   {
      MONITOR_FLUSH_INTERVAL = 500 // Time in milliseconds in between flushing messages.
   };

private: // Types:

   // Type that stands for the list of file monitor clients.
   //
   typedef std::vector<MMonitorFile*>
      MonitorFileVector;

private: // Constructor and destructor:
   
   // Constructor that creates the file monitor thread object.
   //
   // \pre The object should be created first by a file monitor object,
   // otherwise the behavior is undefined.
   //
   // \post The object is created, ready to poll the file monitors and run their
   // idle functions.
   //
   MMonitorFilePrivateThread()
   :
      MThreadWorker(),
      m_monitors(),
      m_eventExit(false, false) // automatically reset the exit event, cleared at the beginning
   {
      M_ASSERT(m_self == NULL); // it will be set later in the new operator
      // Do not start the thread here, Self is not yet assigned, list of monitors is not set up
   }

   // Object destructor. 
   // The children of this class shall call DoFinish in their destructors.
   //
   virtual ~MMonitorFilePrivateThread();

private: // Services:

   // Worker thread running function which implements monitor polling 
   // time after time and calling its idle function.
   //
   virtual void Run();

public: // Services:

   // Attach the new MMonitorFile object to this monitor.
   //
   // \pre The given instance shall not be already present in the private thread,
   // there is a debug check for that.
   //
   // \post The given monitor object gets attached to the MMonitorFilePrivateThread,
   // and its OnIdle will start to be called periodically from the background thread.
   //
   static void AttachMonitor(MMonitorFile*);

   // Detach the MMonitorFile object to this monitor.
   //
   // \pre The given instance shall not be already present in the private thread,
   // there is a debug check for that.
   //
   // \post The given monitor object gets detached from the MMonitorFilePrivateThread.
   //
   static void DetachMonitor(MMonitorFile*);

private: // Properties:

   // List of file monitor clients.
   //
   MonitorFileVector m_monitors;

   // Event which is signaled at exit condition
   //
   MEvent m_eventExit;

   // Mutual exclusion object to exclude the possibility for accessing
   // the facilities of this class from multiple threads at the same time.
   // This is made static, as m_self is also handled here.
   //
   static MCriticalSection m_lock;

   // Reference to Self of the singleton object.
   //
   static MMonitorFilePrivateThread* m_self;
};

/// \endcond SHOW_INTERNAL
#endif // !M_NO_MCOM_MONITOR
#endif
