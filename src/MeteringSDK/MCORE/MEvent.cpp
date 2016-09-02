// File MCORE/MEvent.cpp

#include "MCOREExtern.h"
#include "MEvent.h"
#include "MException.h"

#if !M_NO_MULTITHREADING

#if M_OS & M_OS_WIN32
   MEvent::MEvent(bool setInitially, bool manualClear, MConstChars name)
#else
   MEvent::MEvent(bool setInitially, bool manualClear)
#endif
:
   MSynchronizer()
{
   #if (M_OS & M_OS_WIN32)
      #if M_UNICODE
         m_handle = ::CreateEvent(NULL, (manualClear ? TRUE : FALSE), (setInitially ? TRUE : FALSE), name == NULL ? NULL : MToWideString(name).c_str());
      #else
         m_handle = ::CreateEvent(NULL, (manualClear ? TRUE : FALSE), (setInitially ? TRUE : FALSE), name);
      #endif
      MESystemError::CheckLastSystemError(m_handle == NULL);
   #elif (M_OS & M_OS_POSIX)
      m_event = setInitially;
      m_manualClear = manualClear;
      int error = pthread_mutex_init(&m_mutex, NULL);
      if ( error != 0 )
         MESystemError::Throw(error);
      error = pthread_cond_init(&m_cond, NULL);
      if ( error != 0 )
         MESystemError::Throw(error);
   #else
      #error "No implementation of event exists for this OS"
   #endif
}

MEvent::~MEvent() M_NO_THROW
{
   #if (M_OS & M_OS_POSIX)
      pthread_cond_destroy(&m_cond);
      pthread_mutex_destroy(&m_mutex);
   #endif
}

void MEvent::Set()
{
   #if (M_OS & M_OS_WIN32)
      M_ASSERT(m_handle != 0);
      BOOL bo = ::SetEvent(m_handle);
      MESystemError::CheckLastSystemError(!bo);
   #elif (M_OS & M_OS_POSIX)
      int error = pthread_mutex_lock(&m_mutex);
      if ( error != 0 )
         MESystemError::Throw(error);
      m_event = true;
      error = pthread_cond_signal(&m_cond);
      pthread_mutex_unlock(&m_mutex);
      if ( error != 0 )
         MESystemError::Throw(error);
   #else
      #error "No implementation of event exists for this OS"
   #endif
}

void MEvent::Clear()
{
   #if (M_OS & M_OS_WIN32)
      M_ASSERT(m_handle != 0);
      MESystemError::CheckLastSystemError(!::ResetEvent(m_handle));
   #elif (M_OS & M_OS_POSIX)
      int error = pthread_mutex_lock(&m_mutex);
      if ( error != 0 )
         MESystemError::Throw(error);
      m_event = false;
      error = pthread_mutex_unlock(&m_mutex);
      if ( error != 0 )
         MESystemError::Throw(error);
   #else
      #error "No implementation of event exists for this OS"
   #endif
}

void MEvent::Unlock()
{
   Clear();
}

#if (M_OS & M_OS_WIN32)
   // Windows implementation is in MSynchronizer class
#elif M_OS & M_OS_POSIX
   bool MEvent::LockWithTimeout(long timeout)
   {
      int error = pthread_mutex_lock(&m_mutex);
      if ( error != 0 )
         MESystemError::Throw(error);
      while ( !m_event )
      {
         int error = 0;
         if ( timeout < 0 )
            error = pthread_cond_wait(&m_cond, &m_mutex); // this unlocks mutex at the beginning of wait and locks it back when the wait is finished
         else
         {
            timespec tsc;

#if (M_OS & M_OS_QNXNTO)
            if ( clock_gettime(CLOCK_REALTIME, &tsc) < 0 )
               MESystemError::ThrowLastSystemError();
#else

            #ifndef TIMEVAL_TO_TIMESPEC // Case of Android, for example
               #define TIMEVAL_TO_TIMESPEC(tv, ts) {       \
                           (ts)->tv_sec = (tv)->tv_sec;              \
                           (ts)->tv_nsec = (tv)->tv_usec * 1000;     \
                     }
            #endif

            timeval  tvl;
            gettimeofday(&tvl, NULL);
            TIMEVAL_TO_TIMESPEC(&tvl, &tsc);
#endif
            tsc.tv_sec  += timeout / 1000;
            tsc.tv_nsec += timeout % 1000 * 1000000;
            if ( tsc.tv_nsec >= 1000000000 )
            {
                tsc.tv_sec  += 1;
                tsc.tv_nsec -= 1000000000;
            }
            error = pthread_cond_timedwait(&m_cond, &m_mutex, &tsc);
            if ( error == ETIMEDOUT )
            {
               pthread_mutex_unlock(&m_mutex);
               return false;
            }
         }
         if ( error != 0 )
         {
            pthread_mutex_unlock(&m_mutex);
            MESystemError::Throw(error);
            M_ENSURED_ASSERT(0);
         }
      }
      if ( !m_manualClear ) // clear event if it is not manually cleared
         m_event = false;
      error = pthread_mutex_unlock(&m_mutex);
      if ( error != 0 )
         MESystemError::Throw(error);
      return true;
   }
#endif

#endif
