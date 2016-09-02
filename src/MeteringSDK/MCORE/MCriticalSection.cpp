// File MCORE/MCriticalSection.cpp

#include "MCOREExtern.h"
#include "MCriticalSection.h"

#if !M_NO_MULTITHREADING

    #if M_OS & M_OS_WIN32
        inline void DoCheckCriticalSection(CRITICAL_SECTION& cs)
        {
            M_ASSERT(((const unsigned*)&cs)[sizeof(cs) / sizeof(int) - 1] != 0xFFFFFFFFu);
        }
    #endif

MCriticalSection::MCriticalSection()
{
   #if M_OS & M_OS_WIN32
      ::InitializeCriticalSection(&m_criticalSection);
   #elif M_OS & M_OS_POSIX
      pthread_mutexattr_t attr;
      int error = pthread_mutexattr_init(&attr);
      if ( error != 0 )
         MESystemError::Throw(error);
      error = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      if ( error != 0 )
         MESystemError::Throw(error);
      error = pthread_mutex_init(&m_mutex, &attr);
      pthread_mutexattr_destroy(&attr); // ignore destroy error if any
      if ( error != 0 )
         MESystemError::Throw(error);
   #else
      #error "MCriticalSection is not implemented for this operating system"
   #endif
}

MCriticalSection::~MCriticalSection() M_NO_THROW
{
   #if M_OS & M_OS_WIN32
      ::DeleteCriticalSection(&m_criticalSection);
      #if M_DEBUG
            memset((void*)&m_criticalSection, 0xFF, sizeof(m_criticalSection));
      #endif
   #elif M_OS & M_OS_POSIX
      int error = pthread_mutex_destroy(&m_mutex);
      M_USED_VARIABLE(error); // Pacify release build
      M_ASSERT(error == 0);   // Avoid throwing errors from destructor
   #else
      #error "MCriticalSection is not implemented for this operating system"
   #endif
}

void MCriticalSection::Lock() const
{
   #if M_OS & M_OS_WIN32
      DoCheckCriticalSection(m_criticalSection);
      ::EnterCriticalSection(&m_criticalSection);
   #elif M_OS & M_OS_POSIX
      int error = pthread_mutex_lock(&m_mutex);
      if ( error != 0 )
            MESystemError::Throw(error);
   #else
      #error "MCriticalSection is not implemented for this operating system"
   #endif
}

bool MCriticalSection::TryLock() const
{
   #if M_OS & M_OS_WIN32
      DoCheckCriticalSection(m_criticalSection);
      return ::TryEnterCriticalSection(&m_criticalSection) != FALSE;
   #elif M_OS & M_OS_POSIX
      return pthread_mutex_trylock(&m_mutex) == 0;
   #else
      #error "MCriticalSection is not implemented for this operating system"
   #endif
}

void MCriticalSection::Unlock() const
{
   #if M_OS & M_OS_WIN32
      DoCheckCriticalSection(m_criticalSection);
      ::LeaveCriticalSection(&m_criticalSection);
   #elif M_OS & M_OS_POSIX
      int error = pthread_mutex_unlock(&m_mutex);
      if ( error != 0 )
            MESystemError::Throw(error);
   #else
      #error "MCriticalSection is not implemented for this operating system"
   #endif
}

#endif
