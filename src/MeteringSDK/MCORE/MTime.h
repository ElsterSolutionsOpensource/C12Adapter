#ifndef MCORE_MTIME_H
#define MCORE_MTIME_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MTime.h

#include <MCORE/MObject.h>

#if !M_NO_TIME

/// Value to represent a moment in time, consists of both date and time information.
///
/// The time range currently supported is from year 1970 to year 2099 inclusively,
/// and the class is tested against Year 2038 problem, http://en.wikipedia.org/wiki/Year_2038_problem.
/// Attempts to manipulate out of range dates will lead to a bad date exception.
/// During assignment of properties the following constraints should be met:
/// <ul>
///   <li> Year is bigger than or equal to 1970 and smaller than or equal to 2099.</li>
///   <li> Month is between 1 and 12.</li>
///   <li> DayOfMonth is between 1 and the number of days in the month, up to 31.</li>
///   <li> Hour is between 0 and 23.</li>
///   <li> Minute is between 0 and 59.</li>
///   <li> Second is between 0 and 59.</li>
/// </ul>
///
/// No DST or timezone transformation is done automatically within the class during
/// manipulations with time. This is because the current computer timezone is not necessarily
/// the one the applications manipulate. Because the object does not have a flag indicating
/// whether the time is UTC or local, and the DST status is not available for the specific time,
/// it is up to the user to manipulate the information consistently.
///
/// Among all available time functions, only AsFormattedString is
/// based on the current computer's locale. Also, AsFormattedString exposes
/// the current computer timezone with its %Z format.
///
/// If MTime is created with no parameters, it will be initialized with null time,
/// which is a special value, much like a NULL pointer. Null time value is also
/// available as MTime::s_null static constant.
///
/// Attempts to change null time by ways other than full assignment of all time components
/// will fail with No value exception.
///
/// One can nullify time value by either of the following sequences:
/// \code
///    time.SetToNull();
///    time = MTime();
/// \endcode
/// One cannot initialize the object by calling property setters on null time separately like this:
/// \code
///    MTime value;
///    value.SetYear(2002); // Exception here, attempt to modify a null time
///    value.SetMonth(4);   // Unreachable code
/// \endcode
/// To work around such cases, one can start with initializing the object with the current time:
/// \code
///    MTime value = MTime::GetCurrentUtcTime(); // We are creating UTC
///    value.SetYear(2002);                      // Now it will work
///    value.SetMonth(4);
/// \endcode
///
class M_CLASS MTime : public MObject
{
public: // Types:

   ///@{
   /// Internal time type used by the class.
   ///
   /// It is always 64-bit, independent on time_t size.
   ///
   #if M_TIME_T_BIT_SIZE == 64
      typedef time_t InternalTimeType;
   #elif M_TIME_T_BIT_SIZE == 32
      typedef Mint64 InternalTimeType;
   #else
      #error "Something is wrong with time_t type!"
   #endif
   ///@}

   /// Month enumeration type. Months correspond to their ordinal numbers.
   /// The interface of MTime prefers to use int data type for month.
   ///
   enum MonthType
   {
      MonthJanuary   =  1,  ///< January
      MonthFebruary  =  2,  ///< February
      MonthMarch     =  3,  ///< March
      MonthApril     =  4,  ///< April
      MonthMay       =  5,  ///< May
      MonthJune      =  6,  ///< June
      MonthJuly      =  7,  ///< July
      MonthAugust    =  8,  ///< August
      MonthSeptember =  9,  ///< September
      MonthOctober   = 10,  ///< October
      MonthNovember  = 11,  ///< November
      MonthDecember  = 12   ///< December
   };

   /// Week day constants.
   /// The convention is that the week starts on Sunday (which is in fact culture-related).
   ///
   enum DayOfWeekType
   {
      WeekdaySunday    = 0,    ///< Sunday, zero based, starts the week
      WeekdayMonday    = 1,    ///< Monday, 1
      WeekdayTuesday   = 2,    ///< Tuesday, 2
      WeekdayWednesday = 3,    ///< Wednesday, 3
      WeekdayThursday  = 4,    ///< Thursday, 4
      WeekdayFriday    = 5,    ///< Friday, 5
      WeekdaySaturday  = 6     ///< Saturday, 6
   };

   /// Kind of time
   ///
   enum KindEnum
   {
      KindNull = 0,         ///< Null time, no value
      KindUnspecified = 1,  ///< Unspecified or unknown, but not null. Offset is zero
      KindUtc = 2,          ///< UTC time, offset is zero
      KindLocal = 3         ///< LOcal time, offset is present (can be zero)
   };

public: // Services:

   /// Default constructor, set the time to a null value
   ///
   /// Null value is indistinguishable from "1970-01-01 00:00:00"
   ///
   MTime()
   :
      m_time((InternalTimeType)0)  // Initialize to null time
   {
   }

   /// Constructor, which initializes the object with the standard C structure tm pointer.
   /// The pointer given is not constant, and the constructor can modify the value it
   /// points to by setting the day of week and the other data within the structure.
   ///
   /// The following integer fields within tm structure need to be initialized for the call to succeed:
   /// <ul>
   /// <li> tm_hour -- Hours since midnight, 0 .. 23 </li>
   /// <li> tm_isdst -- Positive value will mean daylight saving time is active;
   ///                  zero is that it is not active; negative value is for unknown status of DST. </li>
   /// <li> tm_mday -- Day of month, 1 .. 31 </li>
   /// <li> tm_min -- Minutes after hour, 0 .. 59 </li>
   /// <li> tm_mon -- Month, 0 .. 11, January is 0 </li>
   /// <li> tm_sec -- Seconds, 0 .. 59 </li>
   /// <li> tm_year -- Year (current year minus 1900) </li>
   /// </ul>
   /// Field tm_wday, day of week, 0 .. 6 starts from Sunday, and field
   /// tm_yday, day in the year, 0 .. 365, Starts from January 1 as zero day,
   /// get initialized inside the call.
   ///
   /// \pre A given pointer has to be properly initialized with a valid
   /// time value supported by this class (see above), or an exception is thrown.
   ///
   explicit MTime(struct tm* value) // SWIG_HIDE
   :
      m_time((InternalTimeType)0) // Initialize to null time for the case of failure
   {
      SetTM(value);
   }

   /// Constructor, which initializes the object with the standard C time_t value.
   ///
   /// \param value Number of seconds since January 1, 1970.
   ///     In some 32-bit systems where time_t is defined as a 32-bit integral type,
   ///     after 2038 the value of time_t will become negative.
   ///     When properly handled, this is not an issue.
   ///
   explicit MTime(time_t value)
   :
      m_time(DoTimeToInternal(value))
   {
   }

   /// Constructor, does conversion of time from string.
   ///
   /// \param str Time given as string, shall correctly represent time in one of the following formats,
   ///            shown as example: "2014-04-13 10:10:59", "2014-04-13 10:10" or "2014-04-13"
   ///
   explicit MTime(const MStdString& str)
   :
      m_time((InternalTimeType)0) // initialize to null time for the case of failure
   {
      SetAsString(str);
   }

   /// Constructor, which directly initializes the value with the given parameters.
   ///
   /// \pre Parameters must satisfy the Set precondition, be a valid time.
   /// Please see preconditions for Set.
   ///
   MTime(int year, int month, int days, int hours = 0, int minutes = 0, int seconds = 0)
   :
      m_time((InternalTimeType)0) // initialize to null time for the case of failure
   {
      Set(year, month, days, hours, minutes, seconds);
   }

   /// Copy constructor, one which copies the time from the time value given.
   ///
   MTime(const MTime& other)
   :
      m_time(other.m_time)
   {
   }

   /// Class destructor
   ///
   virtual ~MTime()
   {
   }

public: // comparisons and test services:

   /// Assignment operator.
   ///
   MTime& operator=(const MTime& other)
   {
      if ( this != &other )
         m_time = other.m_time; // will work fine for null time
      return *this;
   }

   /// Compare this time with another time, ternary outcome comparison operator.
   ///
   /// \pre Objects are valid, not null, or an exception is thrown.
   /// It is not an error to compare an object with itself.
   ///
   int Compare(const MTime&) const;

   /// Equality test binary operator.
   ///
   /// Null time is equal to the other null time, and different with any other time.
   ///
   bool operator==(const MTime& other) const
   {
      return m_time == other.m_time; // do not use Compare, as it checks for null
   }

   /// Inequality test binary operator.
   ///
   /// Null time is equal to the other null time, and different with any other time.
   ///
   bool operator!=(const MTime& other) const
   {
      return m_time != other.m_time; // do not use Compare, as it checks for null
   }

   /// Less than test binary operator.
   ///
   /// \pre Objects are valid, not null, or an exception is thrown.
   /// It is not an error to compare an object with itself.
   ///
   bool operator<(const MTime& other) const
   {
       return Compare(other) < 0;
   }

   /// Greater than test binary operator.
   ///
   /// \pre Objects are valid, not null, or an exception is thrown.
   /// It is not an error to compare an object with itself.
   ///
   bool operator>(const MTime& other) const
   {
       return Compare(other) > 0;
   }

   /// Less than or equal test binary operator.
   ///
   /// \pre Objects are valid, not null, or an exception is thrown.
   /// It is not an error to compare an object with itself.
   ///
   bool operator<=(const MTime& other) const
   {
       return Compare(other) <= 0;
   }

   /// Greater than or equal test binary operator.
   ///
   /// \pre Objects are valid, not null, or an exception is thrown.
   /// It is not an error to compare an object with itself.
   ///
   bool operator>=(const MTime& other) const
   {
       return Compare(other) >= 0;
   }

#if !M_NO_REFLECTION

   /// Reflection enabled copy constructor, creating an object embedded directly into MTime.
   ///
   MVariant NewClone() const;

   /// Add an object to time object and return the result, reflection-enabling service.
   ///
   /// This service has little use in C++, but it is made for reflection.
   ///
   /// \pre The given object shall not be NULL, and shall be compatible for addition to time.
   ///
   MVariant Add(const MVariant&) const;

   /// Subtract an object to time object and return the result, reflection-enabling service.
   ///
   /// This service has little use in C++, but it is made for reflection.
   ///
   /// \pre The given object shall not be NULL, and shall be compatible for addition to time.
   ///
   MVariant Subtract(const MVariant&) const;

#endif

public: // Services:

   /// Initializes time value with all time parameters such as date, and possibly time.
   /// Daylight saving time or local timezone are not taken into consideration.
   ///
   /// \pre Parameters satisfy the following rules, which are the rules for a valid MTime object:
   /// <ul>
   ///   <li> Year is bigger than 1970 and smaller than or equal to 2099.</li>
   ///   <li> Month is between 1 and 12.</li>
   ///   <li> Day is between 1 and the number of days in the month, up to 31.</li>
   ///   <li> Hour is between 0 and 23.</li>
   ///   <li> Minute is between 0 and 59.</li>
   ///   <li> Second is between 0 and 59.</li>
   /// </ul>
   /// Bad time value exception is thrown if any of these conditions are not met.
   ///
   void Set(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);

   /// Initializes time value with date, reflective call.
   /// Daylight saving time or local timezone are not taken into consideration.
   ///
   /// \pre Parameters satisfy the following rules, which are the rules for a valid MTime object:
   /// <ul>
   ///   <li> Year is bigger than 1970 and smaller than 2099.</li>
   ///   <li> Month is between 1 and 12.</li>
   ///   <li> Day is between 1 and the number of days in the month, up to 31.</li>
   /// </ul>
   /// Bad time value exception is thrown if any of these conditions are not met.
   ///
   void SetDate(int year, int month, int day)
   {
      Set(year, month, day);
   }

   /// Get the value of this object as a standard C structure tm.
   /// The buffer shall be provided, which is different from the standard C function gmtime.
   ///
   /// \pre The current time value shall not be null, or a null value exception is thrown.
   ///
   struct tm* GetTM(struct tm* tmBuffer) const;

   /// Set the value for this object from a value given as standard C structure tm.
   ///
   /// The following integer fields within tm structure need to be initialized for the call to succeed:
   /// <ul>
   /// <li> tm_year -- Year (current year minus 1900). </li>
   /// <li> tm_mon -- Months since January, 0 .. 11, January is 0.</li>
   /// <li> tm_mday -- Day of the month, 1 .. 31.</li>
   /// <li> tm_hour -- Hours since midnight, 0 .. 23.</li>
   /// <li> tm_min -- Minutes after the hour, 0 .. 59.</li>
   /// <li> tm_sec -- Seconds after the minute, 0 .. 59. </li>
   /// <li> tm_isdst -- Positive value indicates that daylight saving time is active;
   ///                  zero indicates that daylight saving time is not active; 
   ///                  negative value indicates that the daylight saving time status is unknown.</li>
   /// </ul>
   ///
   /// The following integer fields are initialized inside the call:
   /// <ul>
   /// <li> tm_wday -- Days since Sunday, 0 .. 6, Sunday is 0. </li>
   /// <li> tm_yday -- Days since January 1, January 1 is 0. </li>
   /// </ul>
   ///
   /// \pre A given time value satisfies the range of the time object, see above.
   /// Otherwise an exception is thrown.
   ///
   void SetTM(struct tm*);

   ///@{
   /// Value of this object from a value given as time_t.
   ///
   /// There is no implicit conversion between object and time_t to prevent misuse.
   /// For the same reason the constructor that takes time_t is declared explicit.
   ///
   time_t GetTimeT() const
   {
      return DoInternalToTime(m_time);
   }
   void SetTimeT(time_t value)
   {
      m_time = DoTimeToInternal(value);
   }
   ///@}

   ///@{
   /// Read-write property that allows handling of time object as the number of seconds since 1970.
   ///
   /// Number of seconds since UNIX epoch, 1970, is a widely used way of representing time,
   /// and for compatibility reason, this property handles it as double precision floating point value.
   ///
   double GetSecondsSince1970() const
   {
      return (double)m_time;
   }
   void SetSecondsSince1970(double seconds)
   {
      m_time = static_cast<InternalTimeType>(seconds);
   }
   ///@}

public: // Property accessors:

   ///@{
   /// The year part of time value.
   ///
   /// Year value supported is 1970 to 2099.
   /// If the time is null, accessing this property results in No Value exception.
   ///
   int GetYear() const;
   void SetYear(int);
   ///@}

   ///@{
   /// Gets the day of month.
   ///
   /// Month value in range 1 to 12.
   /// If the time is null, accessing this property results in No Value exception.
   ///
   int GetDayOfMonth() const;
   void SetDayOfMonth(int);
   ///@}

#if (M_OS & M_OS_NUTTX) == 0

   ///@{
   /// Day of the year starting from January 1 as day one.
   ///
   /// Value is in range 1 to 366.
   /// If the time is null, accessing this property results in No Value exception.
   ///
   int GetDayOfYear() const;
   void SetDayOfYear(int);
   ///@}

#endif

   ///@{
   /// Month number within the year of the time event.
   ///
   /// Value is in range 1 to 12.
   /// If the time is null, accessing this property results in No Value exception.
   ///
   int GetMonth() const;
   void SetMonth(int);
   ///@}

   ///@{
   /// Hours part of time of the day.
   ///
   /// Value is in range 0 to 23.
   /// If the time is null, accessing this property results in No Value exception.
   ///
   int GetHours() const;
   void SetHours(int);
   ///@}

   ///@{
   /// Minutes part of time of the day.
   ///
   /// Value is in range 0 to 59.
   /// If the time is null, accessing this property results in No Value exception.
   ///
   int GetMinutes() const;
   void SetMinutes(int);
   ///@}

   ///@{
   /// Seconds part of time of the day.
   ///
   /// Value is in range 0 to 59.
   /// If the time is null, accessing this property results in No Value exception.
   ///
   int GetSeconds() const;
   void SetSeconds(int);
   ///@}

   /// Gets the day of the week.
   ///
   /// Value is in range 0 to 6, Sunday to Saturday.
   /// If the time is null, accessing this property results in No Value exception.
   ///
   DayOfWeekType GetDayOfWeek() const;

   /// Get the week of the month for the date.
   ///
   /// When the returned value is zero it means the date is part of the week that belongs to the previous month.
   /// This would be the case of April 1 if it is Friday, for example.
   ///
   /// \param startsOnSunday If the week starts on Sunday, otherwise the week starts on Monday.
   /// \return The week number in range that starts from 1 for the first week of the month.
   ///
   int GetWeekOfMonth(bool startsOnSunday) const;

   /// Get the week of the year for the date.
   ///
   /// \param startsOnSunday If the week starts on Sunday, otherwise the week starts on Monday.
   /// \return The week number that starts from 1 for the first week of the year.
   ///
   int GetWeekOfYear(bool startsOnSunday) const;

   /// Set this time to null value.
   ///
   void SetToNull()
   {
      m_time = (InternalTimeType)0;
   }

public: // Conversions:

   /// Return the date fraction of this MTime object, does not change this object.
   ///
   /// \pre Object is initialized.
   ///
   MTime AsDate() const;

   ///@{
   /// Represent this time as string in a most general way. Dependency on the current locale is watched.
   ///
   /// \pre Object is created. If object is null, then string is "0".
   ///
   MStdString AsString() const;
   void SetAsString(const MStdString&);
   ///@}

   /// AsFormattedString time using standard C function strftime as formatter.
   ///
   /// \pre Time is not null, or an exception is thrown. If the format is bad, an error is thrown.
   ///
   /// \param formatString
   ///        Format string, similar to formats supported by ANSI C function strflime().
   ///
   /// The following is a full list of supported formats:
   /// <ul>
   /// <li> "%%"  -- Replaced by a single % </li>
   /// <li> "%c"  -- Date and time in a locale sensitive representation, current thread locale. </li>
   /// <li> "%@c" -- Date and time in a locale sensitive representation, current user locale. </li>
   /// <li> "%d"  -- Two-digit day of month with possible leading zero, 01 .. 31. </li>
   /// <li> "%#d" -- Two-digit day of month without the leading zero, 1 .. 31. </li>
   /// <li> "%H"  -- Hour, 24 hour format 01 .. 23. </li>
   /// <li> "%#H" -- Hour, 24 hour format without a leading zero 1 .. 23. </li>
   /// <li> "%I"  -- Hour, 12 hour format 01 .. 12, note that this is upper case letter i.</li>
   /// <li> "%#I" -- Hour, 12 hour format without a leading zero 0 .. 12, note that this is upper case letter i. </li>
   /// <li> "%j"  -- Day of year, 001 .. 366. </li>
   /// <li> "%#j" -- Day of year without leading zeros, 1 .. 366. </li>
   /// <li> "%m"  -- Digits for month, 01 .. 12. </li>
   /// <li> "%#m" -- Digits for month without leading zero, 1 .. 12. </li>
   /// <li> "%M"  -- Minute, 00 .. 59. </li>
   /// <li> "%#M" -- Minute without leading zero, 0 .. 59. </li>
   /// <li> "%p"  -- Expands to AM or PM. </li>
   /// <li> "%q"  -- Week of the current month, 00 .. 06, where week 01 is the first full week that starts on Sunday. </li>
   /// <li> "%#q" -- Week of the current month without leading zero, 0 .. 6, where week 1 is the first full week that starts on Sunday. </li>
   /// <li> "%Q"  -- Week of the current month, 00 .. 06, where week 01 is the first full week that starts on Monday. </li>
   /// <li> "%#Q" -- Week of the current month without leading zero, 0 .. 6, where week 1 is the first full week that starts on Monday. </li>
   /// <li> "%S"  -- Second, 00 .. 59. </li>
   /// <li> "%#S" -- Second without leading zero, 0 .. 59. </li>
   /// <li> "%u"  -- Weekday, 1 .. 7, starting from Monday. </li>
   /// <li> "%U"  -- Week of the current year, 00 .. 53, where 01 is the first Sunday. </li>
   /// <li> "%#U" -- Week of the current year, 0 .. 53, where 1 is the first Sunday. </li>
   /// <li> "%w"  -- Weekday, 0 .. 6, starting from Sunday. </li>
   /// <li> "%W"  -- Week of the current year, 00 .. 53, where 01 is the first Monday. </li>
   /// <li> "%#W" -- Week of the current year, 0 .. 53, where 1 is the first Monday. </li>
   /// <li> "%x"  -- Date fraction in a locale sensitive representation, current thread locale. </li>
   /// <li> "%@x" -- Date fraction in a locale sensitive representation, current user locale. </li>
   /// <li> "%X"  -- Time fraction in a locale sensitive representation, current thread locale. </li>
   /// <li> "%@X" -- Time fraction in a locale sensitive representation, current user locale. </li>
   /// <li> "%y"  -- Two-digit year format, 00 .. 99. </li>
   /// <li> "%#y" -- Two-digit year without leading zero, 0 .. 99. </li>
   /// <li> "%Y"  -- Full four-digit year. </li>
   /// </ul>
   ///
   /// Infixes '#' and '@' in the above formats are:
   /// <ul>
   /// <li> "#" Modifies the numeric formats so that the leading zeros are not written.
   ///      Different from Microsoft Visual C++, this modifier has no effect on locale sensitive formats.
   /// <li> "@" Applies to locale dependent formats, and makes usage of user locale instead of thread locale.
   /// </ul>
   /// Infixes are ignored in formats for which they have no effect.
   ///
   MStdString AsFormattedString(MConstChars formatString) const;

public: // Checks:

   /// Returns whether this time is a null time, a special
   /// value that tells that the time is not initialized.
   /// One can nullify time value by the following sequence:
   /// \code
   ///     time = MTime::s_null;
   /// \endcode
   ///
   /// \pre The object has to be valid, or an exception is thrown.
   ///
   bool IsNull() const
   {
      return m_time == (InternalTimeType)0;
   }

   /// Check if this time is not null.
   ///
   /// \pre If this time IsNull, the exception No Value is thrown.
   ///
   void CheckIfNotNull() const;

public: // Static services:

   /// Get Greenwich Mean Time (or UTC).
   /// This function is the analog of standard C runtime function time(),
   /// but it fixes a known problem with Borland C runtime, where it accounts for US timesavings only.
   ///
   static InternalTimeType GetUtcSecondsSince1970();

   /// Get Greenwich Mean Time (or UTC).
   ///
   static MTime GetCurrentUtcTime()
   {
      return MTime(GetUtcSecondsSince1970());
   }

   /// Get local time according to the current computer settings.
   /// Note that the local time respects the DST status, while GetCurrentStandardTime does not.
   ///
   static MTime GetCurrentLocalTime();

   /// Get current standard time according to the current computer settings.
   /// Note that the standard time does not respect the DST status, while GetCurrentLocalTime does.
   ///
   static MTime GetCurrentStandardTime();

   /// Get number of days in specified year and month.
   /// Year needs to be specified to determine the number of days in February.
   ///
   /// \pre Year shall be in range 1970 to 2099,
   /// and month shall be within 1 and 12, or an exception is thrown.
   ///
   static int GetNumberOfDaysInMonth(int year, int month);

   /// Get number of days in specified year and month.
   /// Year needs to be specified to determine the number of days in February.
   ///
   /// \pre Year shall be in range 1970 to 2099,
   /// and month shall be within 1 and 12, or an exception is thrown.
   ///
   int GetNumberOfDaysInThisMonth() const
   {
      return GetNumberOfDaysInMonth(GetYear(), GetMonth());
   }

   /// Tells if a given year is a leap year.
   ///
   /// The algorithm is that the leap year must be divisible by 4.
   /// If the year is on a century boundary (divisible by 100),
   /// then it is only a leap year if the year is divisible by 400.
   ///
   /// \pre Year shall be within 1970 to 2099,
   /// or an exception is thrown.
   ///
   static bool IsLeapYear(int year);

   /// Tells if This year is a leap year.
   /// Returns information about This time object, not about the current year.
   ///
   /// The algorithm is that the leap year must be divisible by 4.
   /// If the year is on a century boundary (divisible by 100),
   /// then it is only a leap year if the year is divisible by 400.
   ///
   /// \pre Time shall not be Null, and year of the time shall be within 1970 to 2099,
   /// or an exception is thrown.
   ///
   bool IsThisYearLeap()
   {
      return IsLeapYear(GetYear());
   }

   /// Convert two-digit year number to four-digit according to the currently active calendar conventions.
   ///
   /// The current year number switch point is 90.
   /// This service is helpful for abbreviated time handling.
   ///
   /// \param year The two-digit year in range 0 to 99. Exception is thrown otherwise.
   /// \return Year in range 1990 to 2089.
   ///
   static int Year2to4(int year);

   /// Convert four-digit year number to two-digit according to the currently active calendar conventions.
   ///
   /// The current year number switch point is 90.
   /// This service is helpful for meter time handling.
   ///
   /// \param year The full four digit year in range 1990 to 2089, otherwise an exception is thrown.
   /// \return Two-digit year 0 to 99.
   ///
   static int Year4to2(int year);

#if !M_NO_VARIANT
   /// Time is an embedded object type, therefore return its size in bytes
   ///
   /// \return Size of MTime in bytes.
   ///
   virtual unsigned GetEmbeddedSizeof() const;
#endif

public: // Friend operators:

   /// Add time span to this time and assign the result back to time.
   ///
   /// \pre Time shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTime& operator+=(MTime&, const MTimeSpan&);

   /// Subtract time span to this time and assign the result back to time.
   ///
   /// \pre Time shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTime& operator-=(MTime&, const MTimeSpan&);

   /// Add a time span to time value, and without changing the two, return the result.
   ///
   /// \pre Time shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTime operator+(const MTime&, const MTimeSpan&);

   /// Add a time span to time value, and without changing the two, return the result.
   ///
   /// \pre Time shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTime operator+(const MTimeSpan&, const MTime&);

   /// Subtract a time span from time value, and without changing the two, return the result.
   ///
   /// \pre Time shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTime operator-(const MTime&, const MTimeSpan&);

   /// Subtract one time value from another and return the resultant time span.
   ///
   /// \pre Both times shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTimeSpan operator-(const MTime&, const MTime&);

public: // Semi-public constants:

   /// Null time constant value, similar to NULL for pointers.
   /// It can be used to refer to uninitialized time value.
   ///
   static const MTime s_null;

   /// Array that consists of the cumulative number of days in leap year.
   ///
   static const int s_leapYearDays [ 13 ];

   /// Array that consists of the cumulative number of days in non-leap year.
   ///
   static const int s_nonLeapYearDays [ 13 ];

private: // Methods:

   static time_t DoInternalToTime(InternalTimeType t)
   {
      #if M_TIME_T_BIT_SIZE == 64
         return t;
      #elif M_TIME_T_BIT_SIZE == 32
         time_t result = static_cast<time_t>(t); // this can overflow resulting into a negative number
         return result;
      #else
         #error "Something is wrong with the time_t data type"
      #endif
   }

   static InternalTimeType DoTimeToInternal(time_t value)
   {
      #if M_TIME_T_BIT_SIZE == 64
         return value;
      #elif M_TIME_T_BIT_SIZE == 32
         return static_cast<InternalTimeType>(static_cast<unsigned>(value)); // this will always work as desired
      #else
         #error "Something is wrong with the time_t data type"
      #endif
   }

private: // Data:

   // Internal holder of time value
   //
   InternalTimeType m_time;

   M_DECLARE_CLASS(Time)
};

#endif
///@}
#endif
