#ifndef MCORE_MSEMAPHORE_H
#define MCORE_MSEMAPHORE_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MSemaphore.h

#include <MCORE/MSynchronizer.h>

#if !M_NO_MULTITHREADING
#if (M_OS & M_OS_POSIX)
   #include <semaphore.h>
#endif

/// Semaphore to be used to synchronize resource access.
///
/// The implementation of the class is system dependent.
///
/// The semaphore is very often used with MSynchronizer::Lock helper class as 
/// the following:
/// \code
///    MSemaphore semaphore;
///    ...
///    void MyFunction()
///    {
///       MSemaphore::Locker locker(semaphore);
///       ... // do any protected operation
///       // semaphore will be unlocked here automatically
///    }
/// \endcode
class M_CLASS MSemaphore : public MSynchronizer
{
public: // Constructor, destructor:

   /// Constructor that creates semaphore.
   ///
#if (M_OS & M_OS_WIN32)
   MSemaphore(long initialCount, long maxCount, MConstChars name = NULL);
#elif (M_OS & M_OS_POSIX)
   MSemaphore(long initialCount, long maxCount);
#else
   #error "No implementation of semaphore exists for this OS"
#endif

   /// Destructor.
   ///
   virtual ~MSemaphore() M_NO_THROW;

public: // Services:

   /// Unlock semaphore to allow other threads/processes to access the resource.
   /// This service unlocks the semaphore with the count of one.
   /// One needs to take into consideration the monitor-specific UnlockWithCount
   /// service too, which is also capable of returning a previous count.
   ///
   virtual void Unlock();

   /// Unlock semaphore with specific number of counts
   /// to allow other threads/processes to access the resource.
   /// This is a semaphore-specific variation of Unlock service.
   /// The function can be used when the number of the resources is increased to 
   /// add the resource to control.
   ///
   /// The function allows adding more than one resource and knowing the 
   /// previous resource count. The return value is the previous semaphore count.
   ///
   long UnlockWithCount(long count);

#if (M_OS & M_OS_POSIX)
   /// \see \ref MSynchronizer::LockWithTimeout(long)
   ///
   virtual bool LockWithTimeout(long timeout);
#endif

private:

   #if M_OS & M_OS_POSIX
   sem_t m_semaphore;
   #endif // M_OS_POSIX
};

#endif  // !M_NO_MULTITHREADING
///@}
#endif
