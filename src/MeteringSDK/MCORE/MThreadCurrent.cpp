// File MCORE/MThreadCurrent.cpp

#include "MCOREExtern.h"
#include "MThreadCurrent.h"

#if !M_NO_MULTITHREADING

MThreadCurrent MThreadCurrent::s_mainThread;

MThreadCurrent::MThreadCurrent()
:
   MThread(GetStaticCurrentThreadInternalHandle()
              #if M_OS & M_OS_WIN32
                 , GetStaticCurrentThreadId()
              #endif
           )
{
}

MThreadCurrent::~MThreadCurrent()
{
   // handle should not be deleted
}

#endif
