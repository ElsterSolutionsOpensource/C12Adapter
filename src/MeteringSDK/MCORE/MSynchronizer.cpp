// File MCORE/MSynchronizer.cpp

#include "MCOREExtern.h"
#include "MSynchronizer.h"
#include "MException.h"

#if !M_NO_MULTITHREADING

MSynchronizer::Locker::Locker(const MSynchronizer& s)
:
   m_synchronizer(const_cast<MSynchronizer&>(s)),
   m_locked(false)
{ 
   m_synchronizer.Lock();
   m_locked = true;
}

MSynchronizer::Locker::Locker(const MSynchronizer& s, long timeout)
:
   m_synchronizer(const_cast<MSynchronizer&>(s)),
   m_locked(false)
{ 
   m_locked = m_synchronizer.LockWithTimeout(timeout);
}

MSynchronizer::Locker::~Locker() M_NO_THROW
{
   if ( m_locked )
      m_synchronizer.Unlock();
}

MSynchronizer::~MSynchronizer() M_NO_THROW
{
   #if (M_OS & M_OS_WIN32)

      if ( m_handle != 0 )
         CloseHandle(m_handle);

   #elif (M_OS & M_OS_POSIX)
   
      // In Pthreads there are pthread_mutex_t and pthread_cond_t types for synchronization objects, they are destroyed in the derived classes
      
   #else
      #error "No implementation of semaphore exists for this OS"
   #endif
}

#if (M_OS & M_OS_WIN32)
   bool MSynchronizer::LockWithTimeout(long timeout)
   {
         M_ASSERT(m_handle != 0);
         switch( ::WaitForSingleObject(m_handle, timeout < 0 ? INFINITE : timeout) )
         {
         case WAIT_OBJECT_0:
            return true;
         case WAIT_TIMEOUT:
            break;
         default:
            M_ASSERT(0); // An unknown code was returned once. Throw error in this case.
         case WAIT_FAILED:
            MESystemError::ThrowLastSystemError();
            M_ENSURED_ASSERT(0);
         }
         return false;
   }
#endif

#if (M_OS & M_OS_WIN32)
   bool MSynchronizer::DoWaitForMany(long timeout, unsigned* which, MSynchronizer* p1, MSynchronizer* p2, MSynchronizer* p3, MSynchronizer* p4, MSynchronizer* p5)
   {
         M_ASSERT(p1 != NULL && p2 != NULL);

         HANDLE handles [ 5 ];
         handles[0] = p1->m_handle;
         handles[1] = p2->m_handle;

         DWORD handlesCount;
         if ( p3 != NULL )
         {
            handles[2] = p3->m_handle;
            if ( p4 != NULL )
            {
               handles[3] = p4->m_handle;
               if ( p5 != NULL )
               {
                  handles[4] = p5->m_handle;
                  handlesCount = 5;
               }
               else
                  handlesCount = 4;
            }
            else
            {
               M_ASSERT(p5 == NULL);
               handlesCount = 3;
            }
         }
         else
         {
            M_ASSERT(p4 == NULL && p5 == NULL);
            handlesCount = 2;
         }

         DWORD ret = ::WaitForMultipleObjects(handlesCount, handles, ((which == NULL) ? TRUE : FALSE), timeout < 0 ? INFINITE : static_cast<DWORD>(timeout));
         M_COMPILED_ASSERT(WAIT_OBJECT_0 == 0); // below code depends on it
         if ( ret <= (WAIT_OBJECT_0 + 5) )
         {
            if ( which != NULL )
               *which = ret;
            return true;
         }
         if ( ret != WAIT_TIMEOUT )
         {
            M_ASSERT(ret == WAIT_FAILED); // WAIT_ABANDONED_x is not supported
            MESystemError::ThrowLastSystemError();
            M_ENSURED_ASSERT(0);
         }
         return false;
   }
#endif

#endif
