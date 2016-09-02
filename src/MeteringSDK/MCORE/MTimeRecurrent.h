#ifndef MCORE_MTIMERECURRENT_H
#define MCORE_MTIMERECURRENT_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MTimeRecurrent.h

#include <MCORE/MObject.h>
#include <MCORE/MTime.h>
#include <MCORE/MTimeSpan.h>

#if !M_NO_TIME

/// Abstract value to represent a recurrent time, a certain time that happens periodically.
/// There are several types of recurrent periodic time events, denoted by children of this class.
///
class M_CLASS MTimeRecurrent : public MObject
{
protected: // Constructors:

   /// Default constructor. 
   /// Creates an empty recurrent time.
   ///
   MTimeRecurrent()
   {
   }

   /// Copy constructor, creates the current timezone from a copy given.
   /// If a copy of the current timezone is made, it will no longer be automatically
   /// updated from the computer. Instead, it will stay the same as it was during time the constructor was called.
   ///
   MTimeRecurrent(const MTimeRecurrent&)
   {
   }

public: // Destructor:

   /// Class destructor
   ///
   virtual ~MTimeRecurrent()
   {
   }

public: 

   /// Assignment operator.
   ///
   MTimeRecurrent& operator=(const MTimeRecurrent&)
   {
      return *this; // do nothing in this abstract class
   }

   /// Get the event pertinent to a given time period.
   /// The returned time will use a given time as a hint to return the moment,
   /// which represents this recurrent event.
   /// The time given is expected to be in UTC or Standard, and the
   /// the recurring moment will be in the correspondent UTC or standard time.
   ///
   /// For example, in case of a yearly recurring event, a given time is used to
   /// extract a year, for which the event shall be returned.
   ///
   virtual MTime GetPertinent(const MTime&) const = 0;

   /// Tells whether the recurrent date is valid, whether it has a proper range of all its values.
   ///
   bool IsValid() const M_NO_THROW;

   /// Checks whether the recurrent date is valid, whether it has a proper range of all its values.
   ///
   /// \pre An error is thrown if the given recurring date is valid.
   /// IsValid() call is used.
   ///
   virtual void CheckIsValid() const = 0;

   /// Set this recurring time to null value, signifying that there is no recurrence defined.
   /// Implementations in children will differ.
   ///
   virtual void SetToNull() M_NO_THROW = 0;

   /// Returns whether this recurring time is a null time, a special
   /// value, which tells that the recurring time is not initialized.
   ///
   /// \pre The object has to be valid, or an exception is thrown.
   ///
   virtual bool IsNull() const M_NO_THROW = 0;

   /// Throw an exception if the recurrent time is null.
   ///
   void CheckIfNotNull() const;

   /// Create a reflection-enabled clone of the recurring date.
   /// Variant returned has a recurring date embedded it it, 
   /// one which shall not be deleted because it does not allocate memory outside of variant.
   ///
   virtual MVariant NewClone() const = 0;

private: // Data:

   M_DECLARE_CLASS(TimeRecurrent)
};

#endif
///@}
#endif
