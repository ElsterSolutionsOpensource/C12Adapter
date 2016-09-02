// File MCOM/MonitorFilePrivateThread.cpp

#include "MCOMExtern.h"
#include "MonitorFilePrivateThread.h"

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING

MCriticalSection           MMonitorFilePrivateThread::m_lock;
MMonitorFilePrivateThread* MMonitorFilePrivateThread::m_self = NULL;

MMonitorFilePrivateThread::~MMonitorFilePrivateThread()
{
   // This shall not be done within m_lock, as there will be a deadlock if event is not set just yet,
   // but the critical section is about to be entered in the Run() call

   // NOTE: the following assert can still happen in a normal path if a new monitor
   // is created immediately after an old is deleted, but it is okay to happen.
   // It is still here to cure the case when a destructor is called at an unexpected time.
   //
   M_ASSERT(m_monitors.empty());

   try
   {
      m_eventExit.Set();                // wake up the thread and finish processing, if there was some
      #if M_OS & M_OS_UCLINUX
         WaitUntilFinished(false);
      #else
         WaitUntilFinished(false, 100000); // timeout of 100 seconds in case the thread takes long to proceed for some crazy reason
      #endif
   }
   catch ( ... )
   {
   }
}

void MMonitorFilePrivateThread::Run()
{
   while ( !m_eventExit.LockWithTimeout(MONITOR_FLUSH_INTERVAL) )
   {
      MCriticalSection::Locker locker(m_lock);

      MonitorFileVector::iterator it = m_monitors.begin();
      MonitorFileVector::iterator itEnd = m_monitors.end();
      for ( ; it != itEnd; ++it )
         (*it)->OnIdle();
   }
}

void MMonitorFilePrivateThread::AttachMonitor(MMonitorFile* monitor)
{
   MCriticalSection::Locker locker(m_lock);

   MMonitorFilePrivateThread* self;
   if ( m_self == NULL )
      self = M_NEW MMonitorFilePrivateThread();
   else
      self = m_self;

   MonitorFileVector* monitors = &self->m_monitors;
   MonitorFileVector::iterator it = find(monitors->begin(), monitors->end(), monitor);
   if ( it == monitors->end() ) // no such monitor
   {
      monitors->push_back(monitor);
      if ( m_self == NULL )
      {
         m_self = self;
         m_self->Start(); // start the thread after the assignment to m_self!
      }
   }
}

void MMonitorFilePrivateThread::DetachMonitor(MMonitorFile* monitor)
{
   volatile MMonitorFilePrivateThread* thisToDelete = NULL; // if this is not NULL, delete it

   // Critical section-protected part
   {
      MCriticalSection::Locker locker(m_lock);

      M_ASSERT(m_self != NULL);

      MonitorFileVector* monitors = &m_self->m_monitors;
      MonitorFileVector::iterator it = find(monitors->begin(), monitors->end(), monitor);
      if ( it != monitors->end() )
      {
         monitors->erase(it);
         if ( monitors->size() == 0 )
         {
            thisToDelete = m_self;
            m_self = NULL; // nothing more to watch, schedule this item for deletion
         }
      }
   }

   // Delete this outside of the critical section, and after NULL is safely assigned to m_self...
   //
   delete thisToDelete; // this can be NULL, in which case nothing is done...
}

#endif // !M_NO_MCOM_MONITOR
