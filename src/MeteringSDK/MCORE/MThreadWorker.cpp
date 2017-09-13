// File MCORE/MThread.cpp

#include "MCOREExtern.h"
#include "MThreadWorker.h"
#include "MException.h"
#include "MUtilities.h"
#include "MAutomation.h"
#include "MJavaEnv.h"
#include "MScopeSaviors.h"

#if !M_NO_MULTITHREADING

  MThreadWorker::StaticRunFunctionType MThreadWorker::s_staticRunFunction = MThreadWorker::StaticRun;

MThreadWorker::MThreadWorker()
:
   MThread(),
   m_threadLock(),
   m_isRunning(false),
   m_exitException()
{
}

MThreadWorker::~MThreadWorker()
{
   M_ASSERT(!IsRunning());   // DEBUG mode signal, make sure there is a WaitUntilFinished before this destructor
   M_ASSERT(m_thread == 0);  // DEBUG mode signal, make sure there is a WaitUntilFinished before this destructor
   if ( m_thread != 0 )      // Release-mode cleanup of resources
   {
      #if (M_OS & M_OS_WIN32) != 0
         BOOL ok = ::CloseHandle(m_thread);
         M_USED_VARIABLE(ok);
         M_ASSERT(ok);
      #elif (M_OS & M_OS_POSIX) != 0
         int status = pthread_detach(m_thread);
         M_USED_VARIABLE(status);
         M_ASSERT(status == 0);
      #else
         #error "Not implemented!"
      #endif
   }
}

void MThreadWorker::StaticRun(MThreadWorker* thread)
{
   M_ASSERT(thread->m_isRunning); // was set in the Start()

   // Seed random number with available entropy, not for encryption, of course
   srand(MUtilities::GetTickCount() + static_cast<unsigned>(reinterpret_cast<Muintptr>(thread)));

   #if !M_NO_AUTOMATION
      MAutomation::COMInitializer initializer(false);
   #endif
   #if !M_NO_JNI
      MJavaEnv javaEnv; // this attaches the currently running thread to the Java machine
   #endif
   try
   {
      thread->Run();

      MCriticalSection::Locker locker(thread->m_threadLock);
      thread->m_isRunning = false;
   }
   catch ( MException& e )
   {
      MCriticalSection::Locker locker(thread->m_threadLock);
      M_ASSERT(thread->m_exitException.get() == 0);
      thread->m_exitException.reset(e.NewClone());
      thread->m_isRunning = false;
   }
}

#if M_OS & M_OS_WINDOWS
unsigned __stdcall MThreadWorker::DoThreadRoutineCallback(MThreadWorker* thread)
{
   s_staticRunFunction(thread);
   return EXIT_SUCCESS;
}
#elif M_OS & M_OS_POSIX
void* MThreadWorker::DoThreadRoutineCallback(MThreadWorker* thread)
{
   s_staticRunFunction(thread);
   return NULL;
}
#else
   #error "This operating system is not supported yet"
#endif

#if (M_OS & M_OS_WIN32_CE)

inline uintptr_t const _beginthreadex(
   void* security, unsigned stack_size, unsigned (__stdcall* start_address)(void*),
   void* arglist, unsigned initflag, unsigned* thrdaddr)
{
   DWORD unique = 0;
   HANDLE hthread = CreateThread(static_cast<LPSECURITY_ATTRIBUTES>(security), stack_size,
                                 reinterpret_cast<LPTHREAD_START_ROUTINE>(start_address), arglist, initflag, &unique);
   if ( hthread != 0 )
   {
      *thrdaddr = unique;
   }
   return reinterpret_cast<uintptr_t const>(hthread);
}

#endif   // (M_OS & M_OS_WIN32_CE)

void MThreadWorker::Start()
{
   // lock the new thread until the initialization is complete
   MCriticalSection::Locker locker(m_threadLock);

   M_ASSERT(!m_isRunning);                // When this assert is hit this is a new start of the same running worker
   M_ASSERT(GetInternalHandle() == 0);    //   if so, make sure there is a WaitUntilFinished before this call

   m_isRunning = true; // force it here first hand
   m_exitException.reset(NULL); // delete the previous exception, if there was

   InternalHandleType thread = 0;
#if (M_OS & M_OS_WIN32) != 0
   unsigned unique = 0;
   thread = reinterpret_cast<InternalHandleType>(_beginthreadex(0, 0,
                                                                reinterpret_cast<unsigned (__stdcall *)(void *)>(&MThreadWorker::DoThreadRoutineCallback),
                                                                this, 0, &unique));
   if ( thread == 0 )
   {
      m_isRunning = false;
      MESystemError::ThrowLastSystemError();
   }
   m_unique = unique;
#elif (M_OS & M_OS_POSIX) != 0
   const int status = pthread_create(&thread, 0, reinterpret_cast<void * (*)(void *)>(
                                        &MThreadWorker::DoThreadRoutineCallback), this);
   if ( status != 0 )
   {
      m_isRunning = false;
      MESystemError::Throw(status);
      M_ENSURED_ASSERT(0);
   }
#else
   #error "This operating system is not supported yet"
#endif
   m_thread = thread;
}

MException* MThreadWorker::GetExitException()
{
   MCriticalSection::Locker locker(m_threadLock); // avoid race condition
   if ( IsRunning() )
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_THREAD_SHOULD_FINISH_EXECUTION_TO_GET_RESULT,
                        "Thread should finish execution to get its result");
      M_ENSURED_ASSERT(0);
   }
   return m_exitException.get();
}

bool MThreadWorker::IsRunning() const
{
   MCriticalSection::Locker locker(m_threadLock); // use the locker here so that the method will wait until Start finishes
   return m_isRunning;
}

bool MThreadWorker::WaitUntilFinished(bool throwIfError, long timeout)
{
   if ( m_thread != 0 )
   {
#if M_OS & M_OS_WIN32
      DWORD status = ::WaitForSingleObject(m_thread, timeout < 0 ? INFINITE : timeout);
      if ( status == WAIT_OBJECT_0 )
      {
         ::CloseHandle(m_thread);
         m_thread = 0;
      }
      else
      {
         MESystemError::CheckLastSystemError(status != WAIT_TIMEOUT);
      }
#elif M_OS & M_OS_POSIX
      int status;
      #if (M_OS & (M_OS_NUTTX | M_OS_ANDROID)) != 0 // Android and NuttX do not support timeout
         status = pthread_join(m_thread, 0);
      #else
         if ( timeout < 0 )
            status = pthread_join(m_thread, 0);
         else
         {
            timespec tsc;
            #if (M_OS & M_OS_QNXNTO)
               if ( clock_gettime(CLOCK_REALTIME, &tsc) < 0 )
               {
                  MESystemError::ThrowLastSystemError();
                  M_ENSURED_ASSERT(0);
               }
            #else
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
            #if (M_OS & M_OS_QNXNTO) != 0
               status = pthread_timedjoin(m_thread, 0, &tsc);
            #else
               status = pthread_timedjoin_np(m_thread, 0, &tsc);
            #endif
         }
      #endif

      if ( status == 0 ) // finished, handle shall not be used nowhere further
         m_thread = 0;
      else if ( status != ETIMEDOUT && status != EINVAL )
      {
         MESystemError::Throw(status);
         M_ENSURED_ASSERT(0);
      }
#endif
   }

   if ( m_thread == 0 )
   {
      MCriticalSection::Locker locker(m_threadLock);
      if ( throwIfError && m_exitException.get() )
      {
         m_exitException->Rethrow();
         M_ENSURED_ASSERT(0);
      }
      return true;
   }
   return false;
}

#endif
