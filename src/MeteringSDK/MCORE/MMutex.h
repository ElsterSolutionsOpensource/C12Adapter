#ifndef MCORE_MMUTEX_H
#define MCORE_MMUTEX_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MMutex.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MSynchronizer.h>

#if !M_NO_MULTITHREADING && (M_OS & M_OS_WIN32)

/// Basic synchronization object.
/// Functionality of the MMutex is same as MCriticalSection.
///
class M_CLASS MMutex : public MSynchronizer
{
public:

   /// Guide class for MMutex.
   /// Usage example:
   /// \code
   ///   void foo()
   ///   {
   ///      MMutex::Locker lock(mutex);
   ///      ++m_resource;
   ///   }  // mutex will be released here automatically
   /// \endcode
   ///
   class M_CLASS Locker
   {
   public:

      /// Locks given mutex
      ///
      /// \pre Valid MMutex object in any state.
      ///
      Locker(MMutex& mutex);

      /// Releases stored mutex
      ///
      /// \pre Stored mutex must be in a signaled state (locked)
      ///
      ~Locker() M_NO_THROW;

   private:
      MMutex* m_mutex;
   };

   /// Creates mutex object with specified name.
   /// NOTE: Do not use named object due to portability issues.
   ///
   MMutex(MConstChars name = NULL);

   /// Destroys mutex object.
   ///
   virtual ~MMutex() M_NO_THROW;

   /// Releases the mutex object.
   ///
   /// \pre Mutex must be in signaled state (locked).
   ///
   virtual void Unlock();
};

#endif
///@}
#endif
