#ifndef MCORE_MSYNCHRONIZER_H
#define MCORE_MSYNCHRONIZER_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MSynchronizer.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_MULTITHREADING

/// Abstract synchronizer object.
///
/// Synchronizers are event and semaphore.
/// Critical section would also be a synchronizer, 
/// but due to internal details it is not derived from this class.
///
/// The implementation of the class is system dependent.
///
/// Use Locker subclass to handle the synchronizer in a way
/// that guarantees the boundaries of the lock within the C++ scope
/// whether or not an exception thrown.
///
class M_CLASS MSynchronizer
{
public: // Types:

   /// Scope based locker that acquires the lock at construction, and releases it at destruction.
   ///
   /// Typical usage of this convenience class is
   /// \code
   ///    {
   ///        MSynchronizer::Locker lock(criticalSection); // acquire the lock on the critical section
   ///
   ///        // Any code here that can possibly throw an exception
   ///
   ///    }   // Lock is released whether or not the above code threw an exception
   /// \endcode
   ///
   class M_CLASS Locker
   {
   public: // Constructor and destructor:

      /// Lock the given object at construction.
      ///
      /// If the object is already locked within a different thread,
      /// the constructor call will wait until the lock is released.
      ///
      /// Lock timeout is set to infinity so that if the constructor succeeds,
      /// \ref IsLocked() will always return true.
      ///
      /// The constructor can fail with a system error exception.
      ///
      Locker(const MSynchronizer& s);

      /// Lock the given object with timeout at construction.
      ///
      /// If the object is already locked within a different thread,
      /// the constructor will wait for the given number of milliseconds
      /// until the lock is released.
      ///
      /// If the given timeout expires the constructor exits successfully,
      /// but \ref IsLocked() will return false.
      ///
      /// The constructor can fail with a system error exception.
      ///
      Locker(const MSynchronizer& s, long timeout);

      /// If the lock was acquired at construction, unlock the client synchronizer at destruction.
      ///
      /// Typically, the destructor is called implicitly when the locker object goes out of scope.
      ///
      ~Locker() M_NO_THROW;

   public: // Services:

      /// Whether the object has been locked in the constructor
      ///
      /// This is useful to determine if the timeout has occurred,
      /// in which case the return value will be false.
      ///
      bool IsLocked() const
      {
         return m_locked;
      }

   private: // prohibit copy and compare:

      Locker(const Locker&);
      const Locker& operator=(const Locker&);
      bool operator==(const Locker&) const;
      bool operator!=(const Locker&) const;

   private: // Attributes:

      // Synchronizer object reference, which is handled by this locker
      //
      MSynchronizer& m_synchronizer;

      // Flag whether the resource has been locked successfully
      //
      bool m_locked;
   };

protected: // Constructor:

   /// Constructor that creates synchronizer.
   ///
   /// As the class is abstract, the constructor is protected.
   /// Child classes have to initialize this class field m_handle
   /// in their constructors.
   ///
   MSynchronizer()
#if M_OS & M_OS_WIN32
   :
      m_handle(0) // initialize to invalid handle
#endif
   {
   }

   /// Destructor.
   ///
   virtual ~MSynchronizer() M_NO_THROW;

public: // Services:

   /// Lock without timeout.
   ///
   /// Interpretation of this operation may vary depending on the child class.
   ///
   void Lock()
   {
      LockWithTimeout(-1);
   }

   ///@{
   /// Lock the synchronizer or timeout if the object is being locked for a specified number of milliseconds.
   ///
   /// Interpretation of this operation may vary depending on the child class.
   ///
   /// \param timeout Timeout in milliseconds.
   ///     Negative value means infinite timeout.
   ///     Zero timeout can be used to know if the object is currently locked.
   ///     Care should be taken when specifying very long timeouts,
   ///     since the parameter is of type long, which is 32-bits long, about 25 days of milliseconds.
   /// \return True if the lock is acquired, false if timeout took place.
   ///
#if (M_OS & M_OS_WIN32) != 0 || defined(M_DOXYGEN)
   bool LockWithTimeout(long timeout);
#elif (M_OS & M_OS_POSIX) != 0
   virtual bool LockWithTimeout(long timeout) = 0;
#else
   #error "No implementation of synchronizer exists for this OS"
#endif
   ///@}

   /// Unlock the synchronizer by a count of one.
   ///
   /// Interpretation of this operation may vary depending on the child class.
   ///
   virtual void Unlock() = 0;

public: // Static services of the class:

#if (M_OS & M_OS_WIN32) != 0 || defined(M_DOXYGEN)
   /// Wait until all the objects in the list are set to nonsignaled state.
   ///
   /// Windows-only method.
   ///
   /// Currently only up to five objects are supported. Only two synchronizers are mandatory.
   /// Once NULL is specified as object, no subsequent objects shall be supplied.
   /// There are assertions in the debug version.
   /// No timeout is provided, waiting is done infinitely.
   ///
   /// \param p0 First object, required.
   /// \param p1 Second object, required.
   /// \param p2 Third object, optional.
   /// \param p3 Fourth object, optional.
   /// \param p4 Fifth object, optional.
   ///
   static void WaitForAll(MSynchronizer* p0, MSynchronizer* p1, MSynchronizer* p2 = 0, MSynchronizer* p3 = NULL, MSynchronizer* p4 = NULL)
   {
      DoWaitForMany(-1, NULL, p0, p1, p2, p3, p4);
   }

   /// Wait until all the objects in the list are set to nonsignaled state, or timeout expires.
   ///
   /// Windows-only method.
   ///
   /// Currently only up to five objects are supported. Only two synchronizers are mandatory.
   /// Once NULL is specified as object, no subsequent objects shall be supplied.
   /// There are assertions in the debug version.
   ///
   /// \param timeout Timeout in milliseconds.
   ///     Negative value means infinite timeout.
   ///     Zero timeout can be used to know if all objects are currently locked.
   ///     Care should be taken when specifying very long timeouts,
   ///     since the parameter is of type long, which is 32-bits, about 25 days of milliseconds.
   /// \param p0 First object, required.
   /// \param p1 Second object, required.
   /// \param p2 Third object, optional.
   /// \param p3 Fourth object, optional.
   /// \param p4 Fifth object, optional.
   /// \return True if the lock on all objects is acquired, false if there is a timeout.
   ///
   static bool WaitWithTimeoutForAll(long timeout, MSynchronizer* p0, MSynchronizer* p1, MSynchronizer* p2 = 0, MSynchronizer* p3 = NULL, MSynchronizer* p4 = NULL)
   {
      return DoWaitForMany(timeout, NULL, p0, p1, p2, p3, p4);
   }

   /// Wait until any of the objects in the list are set to nonsignaled state.
   ///
   /// Windows-only method.
   ///
   /// Currently only up to five objects are supported. Only two synchronizers are mandatory.
   /// Once NULL is specified as object, no subsequent objects shall be supplied.
   /// There are assertions in the debug version.
   /// No timeout is provided, waiting is done infinitely.
   ///
   /// \param p0 First object, required.
   /// \param p1 Second object, required.
   /// \param p2 Third object, optional.
   /// \param p3 Fourth object, optional.
   /// \param p4 Fifth object, optional.
   /// \return Zero based index of the object that is in nonsignaled state.
   ///
   static unsigned WaitForAny(MSynchronizer* p0, MSynchronizer* p1, MSynchronizer* p2 = NULL, MSynchronizer* p3 = NULL, MSynchronizer* p4 = NULL)
   {
      unsigned which;
      DoWaitForMany(-1, &which, p0, p1, p2, p3, p4);
      return which;
   }

   /// Wait until any of the objects in the list are set to nonsignaled state, or timeout expires.
   ///
   /// Windows-only method.
   ///
   /// Currently only up to five objects are supported. Only two synchronizers are mandatory.
   /// Once NULL is specified as object, no subsequent objects shall be supplied.
   /// There are assertions in the debug version.
   ///
   /// \param timeout Timeout in milliseconds.
   ///     Negative value means infinite timeout.
   ///     Zero timeout can be used to know if all objects are currently locked.
   ///     Care should be taken when specifying very long timeouts,
   ///     since the parameter is of type long, which is 32-bits, about 25 days of milliseconds.
   /// \param which Return value through pointer, zero based index of the object that is in nonsignaled state.
   /// \param p0 First object, required.
   /// \param p1 Second object, required.
   /// \param p2 Third object, optional.
   /// \param p3 Fourth object, optional.
   /// \param p4 Fifth object, optional.
   /// \return True if the lock on an object is acquired, false if there is a timeout.
   ///
   static bool WaitWithTimeoutForAny(long timeout, unsigned* which, MSynchronizer* p0, MSynchronizer* p1, MSynchronizer* p2 = 0, MSynchronizer* p3 = NULL, MSynchronizer* p4 = NULL)
   {
      return DoWaitForMany(timeout, which, p0, p1, p2, p3, p4);
   }

private: // Services:

   // Wait until any or all the objects in the list are set to nonsignaled state, or timeout expires.
   //
   // Internal implementation.
   // Currently only up to five objects are supported. Only two synchronizers are mandatory,
   // the others can be omitted. The timeout given is in milliseconds, negative value means infinity.
   //
   static bool DoWaitForMany(long timeout, unsigned* which, MSynchronizer* p0, MSynchronizer* p1, MSynchronizer* p2, MSynchronizer* p3, MSynchronizer* p4);
#endif

protected: // Attributes:

   #if (M_OS & M_OS_WIN32)

      // Handle of the synchronizer object
      //
      HANDLE m_handle;

   #elif (M_OS & M_OS_POSIX)
   
      // In Pthreads there are pthread_mutex_t and pthread_cond_t types for synchronization objects, we will add them in the derived classes

   #else
      #error "No implementation of synchronizer exists for this OS"
   #endif
};

#endif  // !M_NO_MULTITHREADING
///@}
#endif
