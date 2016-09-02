// File MCORE/MThread.cpp

#include "MCOREExtern.h"
#include "MThreadWorker.h"
#include "MException.h"
#include "MUtilities.h"
#include "MAutomation.h"
#include "MJavaEnv.h"

#if !M_NO_MULTITHREADING

  MThreadWorker::StaticRunFunctionType MThreadWorker::s_staticRunFunction = MThreadWorker::StaticRun;

MThreadWorker::MThreadWorker()
:
   MThread(),
   m_threadLock(),
   m_exitException()
{
}

MThreadWorker::~MThreadWorker()
{
#if M_DEBUG
   MCriticalSection::Locker locker(m_threadLock);
   M_ASSERT(GetInternalHandle() == 0); // when this assert is hit, it means the thread object was deleted before the thread ended
#endif
}

void MThreadWorker::StaticRun(MThreadWorker* thread)
{
   // Seed random number with available entropy
   srand(MUtilities::GetTickCount() + static_cast<unsigned>(reinterpret_cast<Muintptr>(thread)));

   #if !M_NO_AUTOMATION
      MAutomation::COMInitializer initializer(false);
   #endif
   #if !M_NO_JNI
      MJavaEnv javaEnv; // this attaches the currently running thread to the Java machine
   #endif

   MUniquePtr<MException> exception;
   try
   {
      thread->Run();
   }
   catch ( MException& e )
   {
      exception.reset(e.NewClone());
   }
   
   MCriticalSection::Locker locker(thread->m_threadLock);
   if ( thread->GetInternalHandle() )
   {
      M_ASSERT(thread->m_exitException.get() == 0);
      thread->m_exitException.reset(exception.release());
      thread->Destroy();
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
   M_ASSERT(GetInternalHandle() == 0);

   // lock the new thread until the initialization is complete
   MCriticalSection::Locker locker(m_threadLock);
   m_exitException.reset(NULL);

   InternalHandleType thread = 0;
#if M_OS & M_OS_WIN32
   unsigned unique = 0;
   thread = reinterpret_cast<InternalHandleType>(
      _beginthreadex(0, 0, reinterpret_cast<unsigned (__stdcall *)(void *)>(
                        &MThreadWorker::DoThreadRoutineCallback), this, 0, &unique));
   MESystemError::CheckLastSystemError(thread == 0);
#elif M_OS & M_OS_POSIX
   const int status = pthread_create(&thread, 0, reinterpret_cast<void * (*)(void *)>(
                                        &MThreadWorker::DoThreadRoutineCallback), this);
   if ( status != 0 )
   {
      MESystemError::Throw(status);
      M_ENSURED_ASSERT(0);
   }

#endif
   m_thread = thread;
#if M_OS & M_OS_WIN32
   m_unique = unique;
#endif
}

bool MThreadWorker::IsFinished() const
{
   MCriticalSection::Locker locker(m_threadLock);
   return GetInternalHandle() == 0;
}

MException* MThreadWorker::GetExitException()
{
   MCriticalSection::Locker locker(m_threadLock);
   if ( GetInternalHandle() != 0 )
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_THREAD_SHOULD_FINISH_EXECUTION_TO_GET_RESULT,
                        "Thread should finish execution to get its result");
      M_ENSURED_ASSERT(0);
   }

   return m_exitException.get();
}

#if M_OS & M_OS_UCLINUX
   bool MThreadWorker::WaitUntilFinished(bool throwIfError)
#else
   bool MThreadWorker::WaitUntilFinished(bool throwIfError, long timeout)
#endif
{

   InternalHandleType thread = 0;
   {
      MCriticalSection::Locker locker(m_threadLock);
      thread = GetInternalHandle();
   }

   bool finished = (thread == 0);
   if ( !finished )
   {
#if M_OS & M_OS_WIN32
      DWORD status = ::WaitForSingleObject(thread, timeout < 0 ? INFINITE : timeout);
      if ( status == WAIT_OBJECT_0 || GetInternalHandle() == 0 )
      {
         finished = true;
      }
      else
      {
         MESystemError::CheckLastSystemError(status != WAIT_TIMEOUT);
      }
#elif M_OS & M_OS_POSIX
      int status = 0;

   #if (M_OS & (M_OS_NUTTX | M_OS_ANDROID)) != 0 // POSIX Android or NuttX
      status = pthread_join(thread, 0);
   #else
      if ( timeout < 0 )
      {
         status = pthread_join(thread, 0);
      }
      else
      {
         timespec tsc;
#if (M_OS & M_OS_QNXNTO)
         if ( clock_gettime(CLOCK_REALTIME, &tsc) < 0 )
            MESystemError::ThrowLastSystemError();
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
         status = pthread_timedjoin(thread, 0, &tsc);
#else
         status = pthread_timedjoin_np(thread, 0, &tsc);
#endif
      }
   #endif

      if ( status == 0 || GetInternalHandle() == 0 )
      {
         finished = true;
      }
      else if (status && status != ETIMEDOUT && status != EINVAL )
      {
         MESystemError::Throw(status);
      }
#endif
   }

   if ( finished )
   {
      MESystemError::ClearGlobalSystemError();
      MCriticalSection::Locker locker(m_threadLock);
      M_ASSERT(m_thread == 0);
      if ( throwIfError && m_exitException.get() )
      {
         m_exitException->Rethrow();
         M_ENSURED_ASSERT(0);
      }
   }

   return finished;
}

#endif
