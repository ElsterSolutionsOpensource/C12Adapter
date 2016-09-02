#ifndef MCORE_MTIMESPAN_H
#define MCORE_MTIMESPAN_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MTimeSpan.h

#include <MCORE/MTime.h>

#if !M_NO_TIME

/// Time span represents the difference between two times, a duration.
///
/// Time span would be a result of subtraction of two MTime values,
/// or it can be initialized directly. Time span can be convenient
/// to keep the date and time separately from each other.
/// Time span can be negative.
///
/// While null value of MTime cannot be manipulated with,
/// null value of time span is a valid value, zero span,
/// which corresponds to no difference between times.
/// Default constructor initializes time span with such null value.
///
/// Time span is independent from time zone and daylight saving status.
/// In fact, it can be used to represent a difference in time zones
/// or a DST shift. For example, this is how the time is converted to
/// time in another time zone:
/// \code
///    MTimeSpan shiftESTtoUTC(0, 0, -6);
///    MTimeSpan shiftPSTtoUTC(0, 0, -8);
///    MTimeSpan shiftDST(0, 0, 1);   // EST vs. EDT, PST vs. PDT
///
///    ...
///    // Convert from EST to PST
///    myTime += shiftPSTtoUTC - shiftESTtoUTC;
/// \endcode
///
class M_CLASS MTimeSpan : public MObject
{
   friend class M_CLASS MTime;

public: // Services:

   /// Default constructor that initializes the object to no span, no duration.
   ///
   MTimeSpan()
   :
      m_span(0)
   {
   }

   /// Constructor, which explicitly sets the time span data, including days.
   ///
   /// Beware that the order of parameters is reverse to what is usually assumed.
   /// Negative values represent negative time span.
   /// Also, no check is done for overflow condition
   /// (when number of days is so big that it does not fit in 67 years range supported by this class).
   ///
   /// \param secs Seconds of time span.
   /// \param mins Minutes of time span.
   /// \param hours Hours of time span.
   /// \param days Days of time span.
   ///
   explicit MTimeSpan(int secs, int mins = 0, int hours = 0, int days = 0)
   {
      Set(secs, mins, hours, days);
   }

   /// Constructor, which initializes the object to a difference between the two time stamps.
   /// Equivalent to (t1 - t2).
   ///
   /// <ol>
   /// <li> Neither time shall be null, or an exception is thrown. </li>
   /// <li> Absolute difference between the two times shall not be greater
   ///      than internal representation allows (currently LONG_MAX), or an exception is thrown. </li>
   /// </ol>
   ///
   MTimeSpan(const MTime& t1, const MTime& t2);

   /// Copy constructor.
   ///
   MTimeSpan(const MTimeSpan& other)
   :
      m_span(other.m_span)
   {
   }

   /// Object destructor.
   ///
   virtual ~MTimeSpan()
   {
   }

public: // Services:

   /// Whether the object represents no duration.
   ///
   bool IsNull() const
   {
      return m_span == 0;
   }

   /// Set the span to null.
   ///
   void SetToNull()
   {
      m_span = 0;
   }

   ///@{
   /// Represent time span as string.
   ///
   /// Zero time span is represented as "00:00:00".
   /// If the string fits in one day, the property will have only hours, minutes and seconds, 
   /// such as "00:00:01" (one second into the future), "23:00:00" (23 hours into the future).
   /// Otherwise, if the time is bigger than one day it will have number of days at the beginning,
   /// separated by a blank, such as "1000 00:00:00" for 1000 days into the future.
   /// Negative time span (referring to the past) will have a minus sign at the beginning,
   /// such as "-00:01:00" for one minute into the past, and "-100 00:00:00" for 100 days into the past.
   ///
   /// The given time span string syntax is this:
   ///   - A minus sign can appear at the very first position of the string,
   ///     which represents the time span towards the past.
   ///   - There can only be a single blank in the string input,
   ///     and it should appear prior to the number of days of the span.
   ///     After the blank there should be the time portion.
   ///   - Time portion can be the number of seconds alone.
   ///   - If the time portion has a single colon, it is assumed to be hours:minutes.
   ///   - If the time portion has two colons, it is assumed to be hours:minutes:seconds.
   ///
   /// Examples of valid string inputs:
   /// \code
   ///     "0"          // Zero time span
   ///     "-1"         // One second towards the past
   ///     "1234567"    // 1234567 seconds into the future
   ///     "1 1234"     // 1 day and 1234 seconds into the future
   ///     "-123 01:20" // 123 days, 1 hour, and 20 minutes into the past
   ///     "1 2:5:9"    // 1 day, 2 hours, 5 minutes, and 9 seconds into the future
   /// \endcode
   ///
   MStdString AsString() const;
   void SetAsString(const MStdString&);
   ///@}

   /// AsFormattedString the time span object as string.
   ///
   /// The supported formats are:
   /// <ul>
   /// <li> %%  -- Replaced by a single % </li>
   /// <li> %N  -- For negative time span yield '-', for positive '+', ' ' for zero. </li>
   /// <li> %n  -- For negative time span yield '-', otherwise nothing. </li>
   /// <li> %d  -- Number of full days in the duration, same as %D. </li>
   /// <li> %D  -- Number of full days in the duration, same as %d. </li>
   /// <li> %h  -- Number of total hours in duration. </li>
   /// <li> %H  -- Number of hours in 24 hour format in excess of days, 00 .. 23. </li>
   /// <li> %m  -- Number of total minutes in duration. </li>
   /// <li> %M  -- Number of minutes in excess of hours, 00 .. 59. </li>
   /// <li> %s  -- Number of total seconds in duration. </li>
   /// <li> %S  -- Number of seconds in excess of minutes, 00 .. 59. </li>
   /// <li> %X  -- Full time span in the format days hours:minutes:seconds, same as AsString.</li>
   /// <li> %c  -- Full time span in a locale sensitive representation.</li>
   /// </ul>
   ///
   /// By default, formats %d, %D, %h, %H, %m, %M, %s and %S
   /// always give unsigned, absolute values, even if the span is negative.
   /// One can use %N and %n format to add sign in a desired place.
   /// Alternatively, when format has a minus sign character like
   /// %-d, %-D, %-h, %-H, %-m, %-M, %-s or %-S,
   /// the values will have sign if they are negative.
   /// For other formats, the minus sign character is ignored.
   ///
   /// \pre The format string has to be valid.
   ///
   MStdString AsFormattedString(MConstChars formatString) const;

   /// Set all parameters for the time span.
   /// Beware that the order of parameters is reverse to one usually assumed.
   /// Negative values indicate negative time span (into the past).
   ///
   /// Also, no check is done for overflow condition
   /// (when number of days is so big that it does not fit in 67 years range supported by this class).
   ///
   void Set(int secs, int mins = 0, int hours = 0, int days = 0)
   {
      m_span = secs + 60 * (mins + 60 * (hours + days * 24));
   }

   /// Get days fraction of the time span, total 24 hour portions in the duration.
   /// Time span could be negative, in which case the value will be negative too.
   /// There is no correspondent ToDays function, as days is the largest quantity of MTimeSpan.
   ///
   int GetDays() const
   {
      return m_span / (24 * 60 * 60);
   }

   /// Get hours part of the time span, -23 to 23.
   /// \see \ref ToHours which returns total hours in the time span.
   ///
   int GetHours() const;

   /// Get minutes fraction of the time span.
   /// \see \ref ToMinutes which returns total minutes in the time span.
   ///
   int GetMinutes() const;

   /// Get seconds fraction of the time span.
   /// \see \ref ToSeconds which returns total seconds in the time span.
   ///
   int GetSeconds() const;

   /// Convert time span to hours.
   /// This function differs from GetHours in that it returns the total value.
   ///
   /// \pre Object is initialized.
   ///
   int ToHours() const
   {
      return m_span / 3600;
   }

   /// Convert time span to minutes.
   /// This function differs from GetMinutes in that it returns the total value.
   ///
   /// \pre Object is initialized.
   ///
   int ToMinutes() const
   {
      return m_span / 60;
   }

   /// Convert time span to seconds.
   /// This function differs from GetSeconds in that it returns the total value.
   ///
   /// \pre Object is initialized.
   ///
   int ToSeconds() const
   {
      return m_span;
   }

   /// Compare two time spans, ternary comparison service.
   /// The exact value returned is not specified, only the signed value or zero.
   ///
   /// \param other Other variant to compare with.
   /// \return 0 = spans are equal, negative = this one is shorter, positive = this one is longer
   ///
   int Compare(const MTimeSpan& other) const
   {
      return m_span - other.m_span;
   }

   /// Same as Compare, but uses a variant as the other type.
   ///
   /// \param other If this is a numeric value, then it is the number of seconds.
   ///     If this is an object of type MTimeSpan, then a regular Compare is called.
   /// \return 0 = spans are equal, negative = this one is shorter, positive = this one is longer
   ///
   int CompareWithVariant(const MVariant& other) const;

#if !M_NO_REFLECTION
public: // Reflection enabling services:

   /// Add an object to time span object and return the result, reflection-enabling service.
   ///
   /// \pre The given object shall not be NULL, and shall be compatible for addition to time.
   ///
   MVariant Add(const MVariant&) const;

   /// Subtract an object to time span object and return the result, reflection-enabling service.
   ///
   /// \pre The given object shall not be NULL, and shall be compatible for addition to time.
   ///
   MVariant Subtract(const MVariant&) const;

   /// Multiply a time span object with the given number of times, return the result.
   ///
   /// \pre The given object shall not be NULL, and shall be compatible for addition to time.
   ///
   MVariant Multiply(int numberOfTimes) const;

   /// Divide a time span object with the given number of times, return the result.
   ///
   /// \pre The given object shall not be NULL, and shall be compatible for addition to time.
   ///
   MVariant Divide(int numberOfTimes) const;

   /// Reflection-enabling copy constructor.
   ///
   /// \pre The given object shall be of type TimeSpan, or an exception is thrown.
   ///
   MVariant NewClone() const;

   /// Set the new time span value from the given number if seconds.
   ///
   /// \param seconds Seconds value to initialize time span.
   ///
   void DoSet1(int seconds);

   /// Set the new time span value from the given time.
   ///
   /// \param seconds Seconds value of the result time span.
   /// \param minutes Minutes value of the result time span.
   /// \param hours Hours value of the result time span.
   ///
   void DoSet3(int seconds, int minutes, int hours);

#endif

#if !M_NO_VARIANT
   /// Time span is an embedded object type, therefore return its size in bytes.
   ///
   /// \return size of MTimeSpan in bytes.
   ///
   virtual unsigned GetEmbeddedSizeof() const;
#endif

public: // Redefined operators:

   /// Assignment operator.
   ///
   /// \pre Object is created.
   ///
   MTimeSpan& operator=(const MTimeSpan& other)
   {
      if ( this != &other )
         m_span = other.m_span;
      return *this;
   }

   /// Assignment with addition operator.
   ///
   /// \pre Object is created and initialized.
   ///
   MTimeSpan& operator+=(const MTimeSpan& other)
   {
      m_span += other.m_span;
      return *this;
   }

   /// Assignment with subtraction operator.
   ///
   /// \pre Object is created and initialized.
   ///
   MTimeSpan& operator-=(const MTimeSpan& other)
   {
      m_span -= other.m_span;
      return *this;
   }

   /// Assignment with multiplication operator.
   ///
   /// \pre Object is created and initialized.
   ///
   MTimeSpan& operator*=(int numberOfTimes)
   {
      m_span *= numberOfTimes;
      return *this;
   }

   /// Assignment with division operator.
   ///
   /// \pre Object is created and initialized.
   ///
   MTimeSpan& operator/=(int numberOfTimes)
   {
      m_span /= numberOfTimes;
      return *this;
   }

   /// Equality comparison operator.
   ///
   /// \pre Objects are initialized.
   ///
   bool operator==(const MTimeSpan& other) const
   {
      return m_span == other.m_span;
   }

   /// Inequality comparison operator.
   ///
   /// \pre Objects are initialized.
   ///
   bool operator!=(const MTimeSpan& other) const
   {
      return m_span != other.m_span;
   }

   /// Less-than operator.
   ///
   /// \pre Objects are initialized.
   ///
   bool operator<(const MTimeSpan& other) const
   {
      return Compare(other) < 0;
   }

   /// Greater-than operator.
   ///
   /// \pre Objects are initialized.
   ///
   bool operator>(const MTimeSpan& other) const
   {
      return Compare(other) > 0;
   }

   /// Less than or equal to operator.
   ///
   /// \pre Objects are initialized.
   ///
   bool operator<=(const MTimeSpan& other) const
   {
      return Compare(other) <= 0;
   }

   /// Greater than or equal to operator.
   ///
   /// \pre Objects are initialized.
   ///
   bool operator>=(const MTimeSpan& other) const
   {
      return Compare(other) >= 0;
   }

   /// Negation operator, does not modify self.
   ///
   MTimeSpan operator-() const
   {
      return MTimeSpan(-m_span); // initialize span from number of total seconds
   }

   /// Division operator.
   ///
   /// \pre Object is initialized and numberOfTimes shall not be zero.
   ///
   MTimeSpan operator/(int numberOfTimes) const;

   /// Subtraction operator.
   ///
   /// \pre Objects are initialized, no checks are done.
   ///
   MTimeSpan operator-(const MTimeSpan& other) const
   {
      return MTimeSpan(m_span - other.m_span);
   }

   /// Add operator.
   ///
   /// \pre Objects are initialized, no checks are done.
   ///
   MTimeSpan operator+(const MTimeSpan& other) const
   {
      return MTimeSpan(m_span + other.m_span);
   }

public: // Friend functions

   /// Assign-multiplicative operator.
   ///
   friend MTimeSpan operator*(const MTimeSpan& p1, int p2)
   {
      return MTimeSpan(p1.m_span * p2);
   }

   /// Assign-multiplicative operator.
   ///
   friend MTimeSpan operator*(int p1, const MTimeSpan& p2)
   {
      return MTimeSpan(p1 * p2.m_span);
   }

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

   /// Add a time span to time value, and without changing the two return the result.
   ///
   /// \pre Time shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTime operator+(const MTime&, const MTimeSpan&);

   /// Add a time span to time value, and without changing the two return the result.
   ///
   /// \pre Time shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTime operator+(const MTimeSpan&, const MTime&);

   /// Subtract a time span from time value, and without changing the two return the result.
   ///
   /// \pre Time shall not be null, or an exception is thrown.
   ///
   friend M_FUNC MTime operator-(const MTime&, const MTimeSpan&);

   /// Subtract one time value from another and return the resultant time span.
   ///
   /// \pre Neither time shall be null, or an exception is thrown.
   ///
   friend M_FUNC MTimeSpan operator-(const MTime& t1, const MTime& t2);

private: // Private data:

   // Span in seconds. Signed long type is used internally for storage (can be negative).
   //
   long m_span;

   M_DECLARE_CLASS(TimeSpan)
};

#endif
///@}
#endif
