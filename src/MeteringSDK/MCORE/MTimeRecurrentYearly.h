#ifndef MCORE_MTIMERECURRENTYEARLY_H
#define MCORE_MTIMERECURRENTYEARLY_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MTimeRecurrentYearly.h

#include <MCORE/MTimeRecurrent.h>

#if !M_NO_TIME

/// Yearly recurrent date.
///
/// The date is set by an anchor date in a year, denoted by Month and Day in that month,
/// time of the event, determined as an hour, a minute and a second.
///
/// The holiday schedule is not taken into consideration in date transformation.
/// Also, possibility of overlapping dates, their possible merges or rearrangements is not defined.
/// All calculations are performed in either standard time or in UTC, depending on the context.
/// The user shall account for possible DST shift by oneself.
///
/// There is also an offset type, which determines what to do with the anchor date,
/// whether it has to be modified at some condition.
///
/// There is a subtype of offset type, which also requires setting of a day in a week.
/// One type sets the date of occurrence by a day in a month, like first day in month.
/// Another type sets a certain weekday after the , like second Wednesday in January.
/// For both major subtypes one can set modifiers, all determined by OffsetType.
/// The type itself is wholly determined by a modifier.
///
/// When a type changes between day-of-the-month-based and day-of-the-week-based by modifying
/// the value of OffsetType, the Day or Weekday properties morph into each other.
/// Because of it, it is safe to set Day and Weekday properties explicitly after assignment to OffsetType.
/// It is not correct to access a weekday property of a day of the month recurring time,
/// or access a day property of a weekday-based recurring time.
///
class M_CLASS MTimeRecurrentYearly : public MTimeRecurrent
{
public: // Type:

   /// Type of the offset, which needs to be applied to modify anchor event.
   /// This is used only for Yearly and Monthly anchor types.
   ///
   enum OffsetType
   {
      OffsetNo                             =  0, ///< No offset from anchor, the date is set explicitly.

      // The date is set by a number of weekdays in a month, IsDayOfWeekIgnored false

      OffsetWeekdayBefore                  =  1, ///< The anchor date is the given weekday before the anchor, or the anchor itself if the weekday matched.
      OffsetWeekdayFirstAfter              =  2, ///< First weekday on, or after the anchor.
      OffsetWeekdaySecondAfter             =  3, ///< Second weekday on, or after the anchor.
      OffsetWeekdayThirdAfter              =  4, ///< Third weekday on, or after the anchor.
      OffsetWeekdayFourthAfter             =  5, ///< Fourth weekday on, or after the anchor.
      OffsetWeekdayLastAfter               =  6, ///< Last weekday on, or after the anchor.

      // The date is set explicitly with a date in a month, IsDayOfWeekIgnored true

      OffsetObserveOnThisAndFollowingDate  =  7, ///< Observe on date entered as well as on day following date entered.
      OffsetMondayIfSunday                 =  8, ///< Shift to Monday if the day is Sunday.
      OffsetFridayIfSunday                 =  9, ///< Shift to Friday if the day is Sunday.
      OffsetMondayIfSaturday               = 10, ///< Shift to Monday if the day is Saturday.
      OffsetFridayIfSaturday               = 11, ///< Shift to Friday if the day is Saturday.
      OffsetMondayIfSaturdayOrSunday       = 12, ///< Shift to Monday if Sunday or Saturday.
      OffsetFridayIfSaturdayOrSunday       = 13, ///< Shift to Friday if Sunday or Saturday.
      OffsetMondayIfSundayFridayIfSaturday = 14, ///< Postpone to Monday if Sunday, advance to Friday if Saturday.
      OffsetObserveOnFollowingDate         = 15  ///< Do not observe date entered. Observe on day following date entered.
   };

public: // Services:

   /// Default constructor, recurrent time, which is midnight of the New Year.
   ///
   /// All fields of such object are zero, which corresponds to yearly recurrent event at January 1, 00:00.
   ///
   MTimeRecurrentYearly()
   {
      m_value.m_all = 0;

      M_COMPILED_ASSERT(sizeof(m_value) == 8);              // a requirement for value-embedded object
      M_ASSERT(IsValid());
   }

   /// Constructor for day-based yearly time.
   ///
   /// A weekday parameter can be ignored for some recurring offset types.
   ///
   /// \param offsetType Correspondent enumeration value.
   /// \param month Month of the recurring date.
   /// \param dayOfMonth Day of the recurring date in the month.
   /// \param hour Hour of the recurring date.
   /// \param minute Minute of the recurring date.
   ///
   /// \pre The given value shall comprise of a valid yearly recurrent date-based time,
   /// or an exception is thrown.
   ///
   MTimeRecurrentYearly(OffsetType offsetType, MTime::MonthType month, int dayOfMonth = 1, int hour = 0, int minute = 0)
   {
      SetOnDay(offsetType, month, dayOfMonth, hour, minute);
   }

   /// Constructor for weekday-based yearly time.
   ///
   /// \param offsetType Correspondent enumeration value.
   /// \param month Month of the recurring date.
   /// \param dayOfMonth Day of the recurring date in the month.
   /// \param hour Hour of the recurring date.
   /// \param minute Minute of the recurring date.
   /// \param weekday Week day enumeration.
   ///
   /// \pre The given value shall comprise of a valid yearly recurrent date-based time,
   /// or an exception is thrown.
   ///
   MTimeRecurrentYearly(OffsetType offsetType, MTime::MonthType month, int dayOfMonth, int hour, int minute, MTime::DayOfWeekType weekday)
   {
      SetOnWeekday(offsetType, month, dayOfMonth, hour, minute, weekday);
   }

   /// Copy constructor, creates the current timezone from a copy given.
   /// If a copy of the current timezone is made, it will no longer be automatically
   /// updated from the computer. Instead, it will stay the same as it was during time the constructor was called.
   ///
   MTimeRecurrentYearly(const MTimeRecurrentYearly& other)
   :
      MTimeRecurrent(other)
   {
      m_value.m_all = other.m_value.m_all;
   }

   /// Object destructor.
   ///
   virtual ~MTimeRecurrentYearly();

   /// Assignment operator, assigns recurrent time to another.
   ///
   MTimeRecurrentYearly& operator=(const MTimeRecurrentYearly& other)
   {
      m_value.m_all = other.m_value.m_all;
      return *this;
   }

   /// Equality test binary operator.
   /// Two recurrent times are equal if all their rules are equal.
   ///
   bool operator==(const MTimeRecurrentYearly& other) const
   {
      return m_value.m_all == other.m_value.m_all;
   }

   /// Inequality test binary operator.
   /// Two recurrent times are not equal if any of their rules are not equal.
   ///
   bool operator!=(const MTimeRecurrentYearly& other) const
   {
      return !operator==(other);
   }

public: // Properties:

   ///@{
   /// The offset type for this yearly recurring event.
   /// The offset type tells what to do with the event if it falls to a holiday, etc.
   /// Look at OffsetType enumeration for features.
   ///
   OffsetType GetOffsetType() const;
   void SetOffsetType(OffsetType type);
   ///@}

   /// Returns whether the recurrent time will ignore the day of week property due to offset type.
   ///
   bool IsDayOfWeekIgnored() const;

   ///@{
   /// Month parameter for the recurring time.
   ///
   /// \pre The given value shall be in range 1 to 12, defined by MDate::Month,
   /// or an exception is thrown.
   ///
   MTime::MonthType GetMonth() const;
   void SetMonth(MTime::MonthType month);
   ///@}

   ///@{
   /// Set the anchor day of the month parameter for the recurring time.
   /// For many recurring date types this is 1, first day in the month of interest.
   ///
   /// \pre The given value shall be in range 1 to 31.
   /// The month is not checked whether the date exists for such month.
   /// This is done to allow setting the date prior to month.
   ///
   int GetDayOfMonth() const;
   void SetDayOfMonth(int day);
   ///@}

   ///@{
   /// The anchor day of the month parameter for the recurring time.
   /// For many recurring date types this is 1, first day in the month of interest.
   ///
   /// \pre The property value shall be in range 1 to 31.
   /// The month is not checked whether the date exists for such month.
   /// This is done to allow setting the date prior to month.
   /// The current event shall be the day of the month time.
   /// Otherwise an exception is thrown.
   ///
   MTime::DayOfWeekType GetDayOfWeek() const;
   void SetDayOfWeek(MTime::DayOfWeekType weekday);
   ///@}

   ///@{
   /// Hours within the day when the recurring event shall happen.
   /// Zero will mean the beginning of the day, midnight.
   ///
   /// \pre The given value shall be in range 0 to 23.
   ///
   int GetHours() const;
   void SetHours(int hours);
   ///@}

   ///@{
   /// Minute within the day when the recurring event shall happen.
   /// Zero will mean the beginning of the hour.
   ///
   /// \pre The given value shall be in range 0 to 59.
   ///
   int GetMinutes() const;
   void SetMinutes(int minutes);
   ///@}

   ///@{
   /// Seconds within the day when the recurring event shall happen.
   /// Zero will mean the beginning of the minute.
   ///
   /// \pre The given value shall be in range 0 to 59.
   ///
   int GetSeconds() const;
   void SetSeconds(int seconds);
   ///@}

public: // Services:

   /// Set all the parameters of the recurrent day in a single call.
   ///
   /// \param offsetType Correspondent enumeration value.
   /// \param month Month of the recurring date.
   /// \param dayOfMonth Day of the recurring date in the month.
   /// \param hour Hour of the recurring date.
   /// \param minute Minute of the recurring date.
   ///
   /// \pre The given value shall comprise of a valid yearly recurrent time,
   /// or an exception is thrown. The algorithm is written in such a way that no modification
   /// to any field of the class is made if any of the given parameters are invalid.
   ///
   void SetOnDay(OffsetType offsetType, MTime::MonthType month, int dayOfMonth, int hour = 0, int minute = 0);

   /// Set all the parameters of the recurrent day into weekday-based yearly time.
   ///
   /// \param offsetType Correspondent enumeration value.
   /// \param month Month of the recurring date.
   /// \param dayOfMonth Day of the recurring date in the month.
   /// \param hour Hour of the recurring date.
   /// \param minute Minute of the recurring date.
   /// \param weekday Week day enumeration.
   ///
   /// \pre The given value shall comprise of a valid yearly recurrent date-based time,
   /// or an exception is thrown.
   ///
   void SetOnWeekday(OffsetType offsetType, MTime::MonthType month, int dayOfMonth, int hour, int minute, MTime::DayOfWeekType weekday);

   /// Set all the parameters of the recurrent yearly time in a single call.
   /// \pre The given value shall comprise of a valid yearly recurrent date-based time, however no checking is done.
   ///
   /// \see \ref SetOnDay and SetOnWeekday for a safe error-checked versions of SetUnchecked.
   ///
   void SetUnchecked(OffsetType offsetType, MTime::MonthType month, int day, int hours = 0, int minutes = 0, int seconds = 0, MTime::DayOfWeekType weekday = MTime::WeekdaySunday)
   {
      m_value.m_field.m_offsetType = offsetType;
      m_value.m_field.m_month      = month;
      m_value.m_field.m_day        = day;
      m_value.m_field.m_weekday    = weekday;
      m_value.m_field.m_hours      = hours;
      m_value.m_field.m_minutes    = minutes;
      m_value.m_field.m_seconds    = seconds;
      m_value.m_field.m_filler     = 0; // has to be assigned for the equality comparison to work correctly
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
   virtual MTime GetPertinent(const MTime&) const;

   /// Checks whether the recurrent date is valid, and whether all its values are within their proper range.
   ///
   /// \pre An error is thrown if the given recurring date is invalid.
   ///
   virtual void CheckIsValid() const;

   /// Set this recurring time to null value, signifying that there is no recurrence defined.
   /// This implementation overrides the pure virtual defined in the base class.
   ///
   virtual void SetToNull() M_NO_THROW;

   /// Returns whether this recurring time is a null time, a special
   /// value, which tells that the recurring time is not initialized.
   /// This implementation overrides the pure virtual defined in the base class.
   ///
   /// \pre The object has to be valid, or an exception is thrown.
   ///
   virtual bool IsNull() const M_NO_THROW;

   /// Create a reflection-enabled clone of the recurring date.
   /// Variant returned has a recurring date embedded it it,
   /// one which shall not be deleted because it does not allocate memory outside of variant.
   ///
   virtual MVariant NewClone() const;

#if (M_OS & M_OS_WIN32) != 0

   /// Windows specific call that sets the recurring date using the system time as defined in the timezone
   /// as used by SetCurrentTimezone or GetCurrentTimezone.
   ///
   void SetFromTimeZoneTime(const SYSTEMTIME& ti) M_NO_THROW;

   /// Windows specific call that sets the system time from the recurring date using the format
   /// used by SetCurrentTimezone or GetCurrentTimezone.
   ///
   /// \pre The current yearly recurrent time shall have a format supported by
   /// the operating system, otherwise an error is thrown.
   ///
   void ChangeTimeZoneTime(SYSTEMTIME& ti) const;

#endif

#if !M_NO_VARIANT
   /// Recurrent time is an embedded object type, therefore return its size in bytes.
   ///
   /// \return size of MTimeRecurrentYearly in bytes.
   ///
   virtual unsigned GetEmbeddedSizeof() const;
#endif

private: // Types:

   // Type that holds internal representation of yearly recurrent time.
   // This has to be no more than 4 bytes long.
   //
   union InternalRepresentationType
   {
      Muint64 m_all; // All fields in one
      struct
      {
         Muint8 m_offsetType; // Anchor offset type
         Muint8 m_month;      // Anchor month, 1 .. 12
         Muint8 m_day;        // Anchor day of the month, 1 .. 31
         Muint8 m_weekday;    // The weekday of interest, pertinent only for certain offset types, 0 .. 6
         Muint8 m_hours;      // Hour within a day when an event takes place, 0 .. 23
         Muint8 m_minutes;    // Minutes within a day when an event takes place, 0 .. 59
         Muint8 m_seconds;    // Seconds within a day when an event takes place, 0 .. 59
         Muint8 m_filler;     // not used, should always be zero
      } m_field;
   };

private: // Data:

   // Value for recurrent time
   //
   InternalRepresentationType m_value;

   M_DECLARE_CLASS(TimeRecurrentYearly)
};

#endif
///@}
#endif
