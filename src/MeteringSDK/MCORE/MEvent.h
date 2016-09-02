#ifndef MCORE_MEVENT_H
#define MCORE_MEVENT_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MEvent.h

#include <MCORE/MSynchronizer.h>

#if !M_NO_MULTITHREADING

/// Class that supports event paradigm, synchronizer kind.
///
/// The implementation of the class is system dependent.
///
class M_CLASS MEvent : public MSynchronizer
{
public: // Constructor, destructor:

   /// Constructor that creates the event.
   /// The event can be set to signaled state initially, starting from its creation.
   /// Also, if the event is with manual clearing, the only way to clear the event
   /// is with Clear service, otherwise the event will be cleared as soon as someone
   /// has waited till the event.
   ///
   /// Note that the name for the event is a system dependent parameter.
   ///
   /// \pre Resources should allow creating the event.  
   ///
#if M_OS & M_OS_WIN32
   MEvent(bool setInitially = false, bool manualClear = false, MConstChars name = NULL);
#else
   MEvent(bool setInitially = false, bool manualClear = false);
#endif

   /// Destructor.
   ///
   virtual ~MEvent() M_NO_THROW;

public: // Services:

   /// Set the event into signaled state.
   ///
   /// \pre The event shall be successfully created,
   /// or a system exception takes place.
   ///
   void Set();

   /// Clear the event from signaled state.
   ///
   /// \pre The event shall be successfully created,
   /// or a system exception takes place.
   ///
   void Clear();

   /// The implementation of this virtual clears the event.
   ///
   /// \pre The event shall be successfully created,
   /// or a system exception takes place. The count has to be
   /// one, the other value will raise an exception.
   ///
   virtual void Unlock();

#if M_OS & M_OS_POSIX
   /// \see \ref MSynchronizer::LockWithTimeout(long)
   ///
   virtual bool LockWithTimeout(long timeout);
#endif

protected: // Attributes:
/// \cond SHOW_INTERNAL

   #if M_OS & M_OS_POSIX
      bool m_event;
      bool m_manualClear;
      pthread_mutex_t m_mutex;
      pthread_cond_t m_cond;
   #endif

/// \endcond SHOW_INTERNAL
};

#endif  // !M_NO_MULTITHREADING
///@}
#endif
