// File MCORE/MSemaphore.cpp

#include "MCOREExtern.h"
#include "MSemaphore.h"
#include "MException.h"

#if !M_NO_MULTITHREADING

#if (M_OS & M_OS_WIN32)
   MSemaphore::MSemaphore(long initialCount, long maxCount, MConstChars name)
   {
      #if M_UNICODE
         m_handle = ::CreateSemaphore(NULL, initialCount, maxCount, MToWideString(name).c_str()); // creates or opens, if exists already
      #else
         m_handle = ::CreateSemaphore(NULL, initialCount, maxCount, name); // creates or opens, if exists already
      #endif
      MESystemError::CheckLastSystemError(m_handle == 0);
   }

   MSemaphore::~MSemaphore() M_NO_THROW
   {
   }

   void MSemaphore::Unlock()
   {
      UnlockWithCount(1);
   }

   long MSemaphore::UnlockWithCount(long count)
   {
      M_ASSERT(m_handle);
      long prevCount;
      MESystemError::CheckLastSystemError(!::ReleaseSemaphore(m_handle, count, &prevCount));
      return prevCount;
   }

#elif (M_OS & M_OS_POSIX)
   #include <time.h>


   MSemaphore::MSemaphore(long initialCount, long maxCount)
   {
      MESystemError::CheckLastSystemError((sem_init(&m_semaphore, 0, initialCount) == -1)); //initialize an unnamed semaphore
   }

   MSemaphore::~MSemaphore() M_NO_THROW
   {
      sem_destroy(&m_semaphore); //free resources
   }

   void MSemaphore::Unlock()
   {
      MESystemError::CheckLastSystemError(!(sem_post( &m_semaphore) == 0));
   }

   long MSemaphore::UnlockWithCount(long count)
   {
      int prevCount ;
      sem_getvalue(&m_semaphore, &prevCount);
      for ( int i=0; i < count; ++i )
         MESystemError::CheckLastSystemError((sem_post(&m_semaphore) == -1));
      return prevCount;
   }

   bool MSemaphore::LockWithTimeout(long timeout)
   {
      if ( timeout < 0 )
      {
         for ( ;; )
         {
            int res = sem_wait(&m_semaphore);

            if ( res == 0 )
            {
               // OK
               break;
            }
            else if ( res > 0 )
            {
               // glibc bug, copy the returned error code to errno
               errno = res;
            }

            if ( EINTR == errno )
            {
               // the call was interrupted by a signal handler
               continue;
            }

            MESystemError::CheckLastSystemError(res != 0);
            M_ENSURED_ASSERT(0);
         }
      }
      else
      {
         struct timespec tsc;
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

         for (;;)
         {
            int res = sem_timedwait(&m_semaphore, &tsc);

            if ( res == 0 )
            {
               // OK
               break;
            }
            else if ( res >  0 )
            {
               // glibc bug, copy the returned error code to errno
               errno = res;
            }

            if ( EINTR == errno )
            {
               // the call was interrupted by a signal handler
               continue;
            }
            else if ( ETIMEDOUT == errno )
            {
               // the call timed out before the semaphore could be locked
               return false;
            }

            MESystemError::CheckLastSystemError(res != 0);
            M_ENSURED_ASSERT(0);
         }
      }

      return true;
   }

#else
   #error "No implementation of semaphore exists for this OS"
#endif
#endif

