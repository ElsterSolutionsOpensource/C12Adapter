// File MCORE/MThread.cpp

#include "MCOREExtern.h"
#include "MThread.h"
#include "MException.h"

#if !M_NO_MULTITHREADING

MThread::MThread(InternalHandleType thread
#if M_OS & M_OS_WIN32
                 , unsigned long unique
#endif
   )
:
   m_thread(thread)
#if M_OS & M_OS_WIN32
   , m_unique(unique)
#endif
{
}

MThread::~MThread()
{
}

void MThread::Relinquish()
{
#if M_OS & M_OS_POSIX
# if defined(_POSIX_PRIORITY_SCHEDULING) && (_POSIX_PRIORITY_SCHEDULING+0 > 0) \
   || (defined(_POSIX_THREAD_PRIORITY_SCHEDULING) && (_POSIX_THREAD_PRIORITY_SCHEDULING+0 > 0)) \
   || (defined(_XOPEN_REALTIME) && (_XOPEN_REALTIME+0 >= 0))
   sched_yield();
# else
   timespec tsc = { 0, 0 };
#  if defined(_POSIX_TIMERS) && (_POSIX_TIMERS+0 >= 0))     \
   || (defined(_XOPEN_REALTIME) && (_XOPEN_REALTIME+0 >= 0)
   const int status = nanosleep(&tsc, 0);
#  else
   const int status = pselect(0, 0, 0, 0, &tsc, 0);
#  endif
   M_ASSERT(!(status && status != EINTR));
# endif
#elif M_OS & M_OS_WIN32
   ::Sleep(0);
#endif  
}

#if M_OS & M_OS_WIN32
bool MThread::Resume()
{
   ULONG res = ::ResumeThread(GetInternalHandle());
   MESystemError::CheckLastSystemError(res == (ULONG)-1 || res > MAXIMUM_SUSPEND_COUNT);
   return res <= 1; // 0 means was not suspended, 1 means was suspended and resumed
}

void MThread::Suspend()
{
   ULONG res = ::SuspendThread(GetInternalHandle());
   MESystemError::CheckLastSystemError(res == ULONG(-1) || res > MAXIMUM_SUSPEND_COUNT);
}
#endif

#endif // M_NO_MULTITHREADING

