// File MCORE/MMutex.cpp

#include "MCOREExtern.h"
#include "MMutex.h"
#include "MException.h"

#if !M_NO_MULTITHREADING && (M_OS & M_OS_WIN32)

MMutex::Locker::Locker(MMutex& mutex)
:
   m_mutex(&mutex)
{
   m_mutex->Lock();
}

MMutex::Locker::~Locker() M_NO_THROW
{
   try
   {
      m_mutex->Unlock();
   }
   catch ( ... )
   {
      M_ASSERT(0);
   }
}

MMutex::MMutex(MConstChars name)
{
   #if (M_OS & M_OS_WIN32)
      #if M_UNICODE
         m_handle = ::CreateMutex(0, FALSE, MToWideString(name).c_str());
      #else
         m_handle = ::CreateMutex(0, FALSE, name);
      #endif
      MESystemError::CheckLastSystemError(m_handle == 0);
   #else
      #error "No implementation of mutex exists for this OS"
   #endif
}

MMutex::~MMutex() M_NO_THROW
{
}

void MMutex::Unlock()
{
   #if (M_OS & M_OS_WIN32) != 0
      BOOL result = ::ReleaseMutex(m_handle);
      MESystemError::CheckLastSystemError(!result);
   #else
      #error "No implementation of mutex exists for this OS"
   #endif
}

#endif
