#ifndef MCORE_MCRITICALSECTION_H
#define MCORE_MCRITICALSECTION_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MCriticalSection.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MException.h>

#if !M_NO_MULTITHREADING

/// System independent lightweight synchronization object.
///
/// Critical section resembles the interface of MSynchronizer
/// object, however it is not derived from it due to an implementation
/// detail. Critical section works on the thread level only,
/// one cannot use critical sections to synchronize processes.
/// Different from synchronizer objects, critical sections
/// are implemented in a way that if multithreading is not
/// defined, they can still be used in the source code,
/// however they will perform no action.
/// Critical section is reentrant on a per-thread basis,
/// therefore it can be entered multiple times from the same thread,
/// in which case it shall be left from this thread the same number of times
/// in order for the critical section to be released.
///
/// \anchor MCriticalSection_usage
/// Critical section is very often used with the Lock helper class as the following:
/// \code
///    MCriticalSection criticalSection;
///    ...
///    void MyFunction()
///    {
///       MCriticalSection::Locker lock(criticalSection);
///       ... // do any protected operation
///       // critical section will be unlocked here automatically
///    }
/// \endcode
/// A typical error is to omit variable declaration in the locker like this:
/// \code
///       MCriticalSection::Locker(criticalSection); // ERROR! critical section does not extend scope
///       ... // THIS WILL NOT BE PROTECTED BY CRITICAL SECTION!
/// \endcode
/// in which case the locker will not do what it is designed for.
///
class M_CLASS MCriticalSection
{
public: // Type and class definitions:

   /// Class that helps dealing with critical sections
   /// within a single execution scope.
   ///
   /// \see MCriticalSection - critical section class to be used with this locker
   /// \see \ref MCriticalSection_usage "example in class description" for information on how to use it.
   ///
   class M_CLASS Locker
   {
   public: // Constructor and destructor:

      /// Create a critical section lock with the object given as parameter,
      /// and lock the given critical section for the exclusive usage.
      ///
      /// \param criticalSection
      ///    Critical section object to lock from the given thread.
      ///
      /// \see \ref MCriticalSection_usage "example in class description" for information on how to use it.
      ///
      Locker(const MCriticalSection& criticalSection)
      :
         m_criticalSection(&criticalSection)
      {
         m_criticalSection->Lock();
      }

      /// Unlock the critical section and destroy the lock object.
      ///
      /// Typically done automatically when the lock object goes out of scope.
      ///
      /// \see \ref MCriticalSection_usage "example in class description" for information on how to use it.
      ///
      ~Locker()
      {
         m_criticalSection->Unlock();
      }

   private: // Attributes:

      // Critical section reference, which is handled by this lock
      //
      const MCriticalSection* m_criticalSection;
   };

public: // Constructor, destructor, services:

   /// Constructor of the critical section
   ///
   /// \pre There should be enough of system resources,
   /// otherwise the behavior is undefined.
   ///
   MCriticalSection();

   /// Destructor
   ///
   /// \pre No object should wait or lock this particular critical section,
   /// otherwise the behavior is undefined.
   ///
   ~MCriticalSection() M_NO_THROW;

   /// Lock the critical section for exclusive usage of resources.
   ///
   /// If the critical section is locked by another thread,
   /// the call will wait until the resource is freed, and then lock the section.
   ///
   /// Critical section can be locked multiple times by the same thread,
   /// in which case it shall be unlocked by the same number of times
   /// for the critical section to become released.
   ///
   /// \pre Critical section should be valid, and there
   /// should be enough system resources, otherwise the behavior is undefined.
   ///
   /// \see MCriticalSection::Locker - more convenient way of dealing with the critical section
   ///
   void Lock() const;

   /// Attempt to acquire a lock on the critical section for exclusive usage of resources.
   ///
   /// This method never waits. If the critical section is locked by another thread,
   /// the call will return false immediately, otherwise it will lock the section.
   /// If the call returns false, Unlock shall not be called to release the section.
   ///
   /// Critical section can be locked multiple times by the same thread,
   /// in which case it shall be unlocked by the same number of times
   /// for the critical section to become released.
   ///
   /// \pre Critical section should be valid, and there
   /// should be enough system resources, otherwise the behavior is undefined.
   ///
   /// \see MCriticalSection::Lock - wait until the section is acquired
   ///
   bool TryLock() const;

   /// Unlock the critical section.
   ///
   /// If the critical section is entered multiple times by the same thread,
   /// it will not be released until unlocked the same number of times.
   ///
   /// \pre Critical section should be previously locked by this object,
   /// otherwise the behavior is undefined.
   ///
   /// \see MCriticalSection::Locker - more convenient way of dealing with the critical section
   ///
   void Unlock() const;

private:

   MCriticalSection(const MCriticalSection&);            // disallow copying of critical section
   MCriticalSection& operator=(const MCriticalSection&); // disallow assigning critical section
   bool operator==(const MCriticalSection&) const;       // disallow comparing critical section
   bool operator!=(const MCriticalSection&) const;       // disallow comparing critical section

private: // Attributes:

   #if M_OS & M_OS_WIN32

      // System critical section object
      //
      mutable CRITICAL_SECTION m_criticalSection;

   #elif M_OS & M_OS_POSIX

      friend class MThreadWorker;

      mutable pthread_mutex_t m_mutex;

   #else
      #error "MCriticalSection is not implemented for this operating system"
   #endif
};

#else // !M_NO_MULTITHREADING
/// \cond SHOW_INTERNAL

// Dummy implementation that will do nothing - no multithreading

class MCriticalSection
{
public:

   class Locker
   {
   public: // Constructor and destructor:

      Locker(const MCriticalSection&)
      {
      }

   };

public:

   MCriticalSection()
   {
   }

   void Lock() const
   {
   }

   void Unlock() const
   {
   }

private:

   MCriticalSection(const MCriticalSection&);            // disallow copying of critical section
   MCriticalSection& operator=(const MCriticalSection&); // disallow assigning critical section
   bool operator==(const MCriticalSection&) const;       // disallow comparing critical section
   bool operator!=(const MCriticalSection&) const;       // disallow comparing critical section
};

/// \endcond
#endif // !M_NO_MULTITHREADING
///@}
#endif
