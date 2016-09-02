#ifndef MCORE_MNONCOPYABLE_H
#define MCORE_MNONCOPYABLE_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MNonCopyable.h

#include <MCORE/MCOREDefs.h>

/// Provides a simple and expressive way to declare non-copyable classes.
/// Example:
/// \code
///    class MScheduler : MNonCopyable        // private inheritance
///    {
///       // class declaration
///       // NOTE: you do not need declare private copying constructor and
///       //       assignment operator.
///    };
/// \endcode
/// This class has no virtual destructor so you should never cast your
/// classes to MNonCopyable. Use private inheritance to guarantee this.
///
class M_CLASS MNonCopyable
{
protected:

   /// Default constructor of an abstract noncopyable class, protected
   ///
   MNonCopyable()
   {
   }

private: // restrict copying

   MNonCopyable(const MNonCopyable&);

   const MNonCopyable& operator=(const MNonCopyable&);
};

///@}
#endif
