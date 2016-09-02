#ifndef MCORE_MTHREADCURRENT_H
#define MCORE_MTHREADCURRENT_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MThreadCurrent.h

#include <MCORE/MThread.h>

#if !M_NO_MULTITHREADING

/// Thread instance that attaches to the current thread to allow
/// manipulations with the thread parameters.
///
/// The majority of the functionality is achieved with MThread
/// parent services.
/// 
/// It is recommended that there is only one
/// thread object created per actual thread.
///
class M_CLASS MThreadCurrent : public MThread
{
public: // Constructor, destructor:

   /// Creates object and immediately attaches it to the current thread.
   ///
   /// \pre System resources and security settings should 
   /// allow for this operation to succeed, otherwise a number of
   /// system errors will be thrown.
   ///
   MThreadCurrent();

   /// Destroys the current thread object, but does not
   /// influence the actual thread itself.
   ///
   virtual ~MThreadCurrent();

   /// Access the main thread, static and global method.
   ///
   static MThreadCurrent* GetMain()
   {
      return &s_mainThread;
   }

   /// Static version of current thread identifier, unique per machine.
   ///
   /// This is a shortcut for MThreadCurrent::GetThreadId.
   ///
   static unsigned long GetStaticCurrentThreadId()
   {
      #if M_OS & M_OS_WIN32
         return ::GetCurrentThreadId();
      #elif M_OS & M_OS_POSIX
         return pthread_self();
      #endif
   }

   /// Static version of current thread handle property.
   ///
   /// This is a shortcut for MThreadCurrent::GetInternalHandle.
   ///
   static InternalHandleType GetStaticCurrentThreadInternalHandle()
   {
      #if M_OS & M_OS_WIN32
         return ::GetCurrentThread();
      #elif M_OS & M_OS_POSIX
         return pthread_self();
      #endif
   }

private: // Attributes:

   // Main thread current reference
   //
   static MThreadCurrent s_mainThread;
};

#endif // !M_NO_MULTITHREADING
///@}
#endif
