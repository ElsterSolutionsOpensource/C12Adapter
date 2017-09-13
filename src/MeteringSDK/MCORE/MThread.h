#ifndef MCORE_MTHREAD_H
#define MCORE_MTHREAD_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MThread.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_MULTITHREADING

#if !((M_OS & M_OS_WIN32) || (M_OS & M_OS_POSIX))
   #error "Not implemented!"
#endif

/// Operating system independent abstract thread.
///
/// No instances can be created, look at MThreadWorker and MThreadCurrent
/// for possible concrete classes.
/// 
/// Due to behavior, it is recommended that there is only one 
/// thread object created per actual thread.
///
/// Because it is common to derive concrete objects from thread,
/// many thread services and attributes have the word Thread in their names.
///
class M_ABSTRACT_CLASS MThread
{
public:

   /// Operating system dependent internal handle type.
   #if (M_OS & M_OS_WIN32)
      typedef void* InternalHandleType;
   #elif (M_OS & M_OS_POSIX)
      typedef pthread_t InternalHandleType;
   #endif // M_OS & M_OS_POSIX

protected:

   /// Thread protected constructor. Its behavior is specific to the
   /// concrete object which inherits from it.
   ///
   MThread(InternalHandleType thread = 0
#if M_OS & M_OS_WIN32
           , unsigned long unique = 0
#endif
      );

public:

   /// Destructor, destroys the thread object.
   ///
   virtual ~MThread();

   /// Get thread identifier, a number that is guaranteed to be unique per thread.
   ///
   unsigned long GetThreadId() const
   {
      #if M_OS & M_OS_WIN32
         return m_unique;
      #elif M_OS & M_OS_POSIX
         return m_thread;
      #endif
   }

   /// Get thread handle, operating system dependent thread object manipulator.
   ///
   InternalHandleType GetInternalHandle() const volatile
   {
      return m_thread;
   }

#if M_OS & M_OS_WIN32
   /// Resume the execution of the thread.
   ///
   /// This method is supported only on Windows.
   ///
   /// There is a counter, which gets incremented at each \ref Suspend call.
   /// If Suspend was not called, and/or the counter is zero,
   /// this method does nothing and returns true.
   ///
   /// Otherwise, this method decrements the suspend counter.
   /// If the counter is not zero, then false is returned and the thread remains suspended.
   /// When the counter is decremented to zero, meaning that the number of Resume calls
   /// matched the number of \ref Suspend calls, the method resumes the thread and returns true.
   ///
   /// \return bool True is returned if the thread is runnable after this call.
   ///
   /// \see \ref Suspend - increment the counter of Suspend calls, make sure the thread is suspended.
   ///
   bool Resume();

   /// Suspend the execution of the thread.
   ///
   /// This method is supported only on Windows.
   ///
   /// Increment the thread suspend counter, suspend the thread.
   /// To resume the thread, an equal number of \ref Resume should be called for the thread.
   ///
   /// \see \ref Resume - decrement the counter of Suspend calls, resume the thread when the counter becomes zero.
   ///
   void Suspend();
#endif

   /// Release the rest of our time slice letting the other threads run.
   ///
   static void Relinquish();  // I named it Yield first, but this conflicts with WIN32 Yield() macro

private: // Disabled operations:

   // Dummy copy constructor to prevent accidental copying
   //
   MThread(const MThread&);

   // Dummy assignment operator to prevent accidental assignment
   //
   void operator=(const MThread&);
   
protected:

   /// Thread handle
   ///
   volatile InternalHandleType m_thread;

#if M_OS & M_OS_WIN32
   /// Unique identifier
   ///
   unsigned long m_unique;
#endif
};

#endif // !M_NO_MULTITHREADING
#endif // MCORE_MTHREAD_H
///@}

