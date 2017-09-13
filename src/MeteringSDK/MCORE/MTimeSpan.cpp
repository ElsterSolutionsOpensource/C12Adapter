// File MCORE/MTimeSpan.cpp

#include "MCOREExtern.h"
#include "MTimeSpan.h"
#include "MAlgorithm.h"
#include "MException.h"

#if !M_NO_TIME

   #if !M_NO_REFLECTION

      static MVariant DoNew0()
      {
         MTimeSpan span;
         return MVariant(&span, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      static MVariant DoNew1(const MVariant& v)
      {
         MTimeSpan span;
         if ( v.IsObject() )
            span = *M_DYNAMIC_CONST_CAST_WITH_THROW(MTimeSpan, v.AsObject());
         else if ( v.IsNumeric() )
            span.Set(v.AsInt());
         else
            span.SetAsString(v.AsString());
         return MVariant(&span, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      static MVariant DoNewSMHD(int sec, int min, int hrs, int days)
      {
         MTimeSpan span(sec, min, hrs, days);
         return MVariant(&span, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      /// Constructor, which explicitly sets the time span data from seconds, minutes and hours value.
      ///
      /// Beware that the order of parameters is reverse to what is usually assumed.
      /// Negative values represent negative time span.
      /// Also, no check is done for overflow condition
      /// (when number of hours is so big that it does not fit in 67 years range supported by this class).
      ///
      /// \param secs Seconds of time span.
      /// \param mins Minutes of time span.
      /// \param hours Hours of time span.
      ///
      static MVariant DoNewSMH(int secs, int mins, int hours)
      {
         return DoNewSMHD(secs, mins, hours, 0);
      }

      /// Create time span from the given number of seconds.
      ///
      /// \param seconds Number of seconds to span.
      /// \return Result time span.
      ///
      static MVariant DoNewSeconds(int seconds)
      {
         return DoNewSMHD(seconds, 0, 0, 0);
      }

      /// Create time span from the given number of minutes.
      ///
      /// \param minutes Number of minutes to span.
      /// \return Result time span.
      ///
      static MVariant DoNewMinutes(int minutes)
      {
         return DoNewSMHD(0, minutes, 0, 0);
      }

      /// Create time span from the given number of hours.
      ///
      /// \param hours Number of hours to span.
      /// \return Result time span.
      ///
      static MVariant DoNewHours(int hours)
      {
         return DoNewSMHD(0, 0, hours, 0);
      }

      /// Create time span from the given number of days.
      ///
      /// \param days Number of days to span.
      /// \return Result time span.
      ///
      static MVariant DoNewDays(int days)
      {
         return DoNewSMHD(0, 0, 0, days);
      }

   #endif

M_START_PROPERTIES(TimeSpan)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT   (TimeSpan, IsNull)
   M_OBJECT_PROPERTY_READONLY_INT          (TimeSpan, Days)
   M_OBJECT_PROPERTY_READONLY_INT          (TimeSpan, Hours)
   M_OBJECT_PROPERTY_READONLY_INT          (TimeSpan, Minutes)
   M_OBJECT_PROPERTY_READONLY_INT          (TimeSpan, Seconds)
   M_OBJECT_PROPERTY_READONLY_INT_EXACT    (TimeSpan, ToHours)     // DOXYGEN_HIDE SWIG_HIDE due to existence below of services with the same name
   M_OBJECT_PROPERTY_READONLY_INT_EXACT    (TimeSpan, ToMinutes)   // DOXYGEN_HIDE SWIG_HIDE due to existence below of services with the same name
   M_OBJECT_PROPERTY_READONLY_INT_EXACT    (TimeSpan, ToSeconds)   // DOXYGEN_HIDE SWIG_HIDE due to existence below of services with the same name
   M_OBJECT_PROPERTY_READONLY_STRING_EXACT (TimeSpan, AsString, ST_MStdString_X)
M_START_METHODS(TimeSpan)
   M_OBJECT_SERVICE                 (TimeSpan, ToHours,                     ST_int_X)
   M_OBJECT_SERVICE                 (TimeSpan, ToMinutes,                   ST_int_X)
   M_OBJECT_SERVICE                 (TimeSpan, ToSeconds,                   ST_int_X)
   M_OBJECT_SERVICE_NAMED           (TimeSpan, Compare, CompareWithVariant, ST_int_X_constMVariantA)
   M_OBJECT_SERVICE_OVERLOADED      (TimeSpan, Set, Set,                 4, ST_X_int_int_int_int)
   M_OBJECT_SERVICE_OVERLOADED      (TimeSpan, Set, DoSet3,              3, ST_X_int_int_int)   // SWIG_HIDE
   M_OBJECT_SERVICE_OVERLOADED      (TimeSpan, Set, DoSet1,              1, ST_X_int)           // SWIG_HIDE
   M_OBJECT_SERVICE                 (TimeSpan, Add,                         ST_MVariant_X_constMVariantA)
   M_OBJECT_SERVICE                 (TimeSpan, Subtract,                    ST_MVariant_X_constMVariantA)
   M_OBJECT_SERVICE                 (TimeSpan, Multiply,                    ST_MVariant_X_int)
   M_OBJECT_SERVICE                 (TimeSpan, Divide,                      ST_MVariant_X_int)
   M_OBJECT_SERVICE                 (TimeSpan, SetToNull,                   ST_X)
   M_OBJECT_SERVICE                 (TimeSpan, AsFormattedString,           ST_MStdString_X_MConstChars)
   M_CLASS_FRIEND_SERVICE           (TimeSpan, NewSeconds, DoNewSeconds,    ST_MVariant_S_int)
   M_CLASS_FRIEND_SERVICE           (TimeSpan, NewHours,   DoNewHours,      ST_MVariant_S_int)
   M_CLASS_FRIEND_SERVICE           (TimeSpan, NewMinutes, DoNewMinutes,    ST_MVariant_S_int)
   M_CLASS_FRIEND_SERVICE           (TimeSpan, NewDays,    DoNewDays,       ST_MVariant_S_int)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(TimeSpan, New,        DoNewSMHD,    4, ST_MVariant_S_int_int_int_int)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(TimeSpan, New,        DoNewSMH,     3, ST_MVariant_S_int_int_int)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(TimeSpan, New,        DoNew1,       1, ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(TimeSpan, New,        DoNew0,       0, ST_MVariant_S)
   M_OBJECT_SERVICE                 (TimeSpan, NewClone,                    ST_MVariant_X)
M_END_CLASS(TimeSpan, Object)

#if !M_NO_REFLECTION

MVariant MTimeSpan::Add(const MVariant& other) const
{
   if ( MVariant::StaticIsObject(other) ) // this check is necessary to catch problems with reflective calls
   {
      const MObject* obj = other.AsExistingObject();
      if ( obj->GetClass() == MTimeSpan::GetStaticClass() )
      {
         MTimeSpan timeSpan = *this;
         timeSpan += *M_CHECKED_CAST(const MTimeSpan*, obj);
         return MVariant(&timeSpan, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }
   }
   MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_BINARY_OPERATION_BETWEEN_INCOMPATIBLE_ARGUMENTS, "Binary operation between incompatible arguments"));
   M_ENSURED_ASSERT(0);
   return MVariant();
}

MVariant MTimeSpan::Multiply(int numberOfTimes) const
{
   MTimeSpan timeSpan = *this;
   timeSpan *= numberOfTimes;
   return MVariant(&timeSpan, MVariant::ACCEPT_OBJECT_EMBEDDED);
}

MVariant MTimeSpan::Divide(int numberOfTimes) const
{
   MTimeSpan timeSpan = *this;
   timeSpan /= numberOfTimes;
   return MVariant(&timeSpan, MVariant::ACCEPT_OBJECT_EMBEDDED);
}

MVariant MTimeSpan::Subtract(const MVariant& other) const
{
   if ( MVariant::StaticIsObject(other) ) // this check is necessary to catch problems with reflective calls
   {
      const MObject* obj = other.AsExistingObject();
      if ( obj->GetClass() == MTimeSpan::GetStaticClass() )
      {
         MTimeSpan timeSpan = *this;
         timeSpan -= *M_CHECKED_CAST(const MTimeSpan*, obj);
         return MVariant(&timeSpan, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }
   }
   MException::Throw(MException::ErrorSoftware, M_ERR_BINARY_OPERATION_BETWEEN_INCOMPATIBLE_ARGUMENTS, M_I("Binary operation between incompatible arguments"));
   M_ENSURED_ASSERT(0);
   return MVariant();
}

MVariant MTimeSpan::NewClone() const
{
   MTimeSpan span(*this);
   return MVariant(&span, MVariant::ACCEPT_OBJECT_EMBEDDED);
}

void MTimeSpan::DoSet1(int secs)
{
   Set(secs);
}

void MTimeSpan::DoSet3(int secs, int mins, int hours)
{
   Set(secs, mins, hours);
}

#endif

#if M_OS & M_OS_NUTTX
   #define difftime(t1,t0) (double)(t1 - t0)
#endif

MTimeSpan::MTimeSpan(const MTime& t1, const MTime& t2)
{
   t1.CheckIfNotNull();
   t2.CheckIfNotNull();
   double diff = difftime(t1.GetTimeT(), t2.GetTimeT());
   if ( diff < LONG_MIN || diff > LONG_MAX )
   {
      MException::Throw(M_ERR_TIME_SPAN_TOO_LARGE_CANNOT_BE_REPRESENTED, M_I("Time span too large - cannot be represented"));
      M_ENSURED_ASSERT(0);
   }
   m_span = (long) diff;
}

MStdString MTimeSpan::AsString() const
{
   MStdString result;
   if ( IsNull() )
#ifdef M_USE_USTL
      result.append((size_t)1, '0');
#else
      result.assign(1, '0');
#endif
   else
   {
      char buff [ 64 ];
      int days = GetDays();
      int hours = GetHours();
      int minutes = GetMinutes();
      int seconds = GetSeconds();

      char* b = buff;
      size_t len = 0;
      if ( m_span < 0 )
      {
         M_ASSERT(days <= 0 && hours <= 0 && minutes <= 0 && seconds <= 0);
         *b++ = '-';
         ++len;
         days = -days;
         hours = -hours;
         minutes = -minutes;
         seconds = -seconds;
      }

      size_t size;
      if ( days == 0 )
         size = MFormat(b, sizeof(buff) - len, "%02d:%02d:%02d", hours, minutes, seconds);
      else
         size = MFormat(b, sizeof(buff) - len, "%d %02d:%02d:%02d", days, hours, minutes, seconds);
      M_ASSERT(size < sizeof(buff) - len); // Check if the supplied buffer fits, as ensured by the format
      len += size;
      result.assign(buff, len);
   }
   return result;
}

void MTimeSpan::SetAsString(const MStdString& str)
{
   int seconds = 0;
   int minutes = 0;
   int hours = 0;
   int days = 0;
   bool error = false;
   bool negative = false;
   MStdString input = MAlgorithm::TrimString(str);
   if ( !input.empty() && input[0] == '-' )
   {
      negative = true;
      input.erase(input.begin());
   }
   try
   {
      MStdString::size_type posBlank  = input.find(' ');
      if ( posBlank != MStdString::npos )
      {
         input[posBlank] = '\0';
         days = (int)MToUnsignedLong(input.data());
         input.erase((size_t)0, posBlank + 1);
      }

      MStdStringVector vec = MAlgorithm::SplitWithDelimiter(input, ':', false, true);
      if ( vec.size() == 1 && posBlank == MStdString::npos ) // only seconds given
      {
         long span = (long)MToUnsignedLong(input); // then this is seconds, given explicitly
         m_span = negative ? -span : span;
         return; // done
      }
      else if ( vec.size() == 2 )
      {
         hours = (int)MToUnsignedLong(vec[0]);
         minutes = (int)MToUnsignedLong(vec[1]);
      }
      else if ( vec.size() == 3 )
      {
         hours = (int)MToUnsignedLong(vec[0]);
         minutes = (int)MToUnsignedLong(vec[1]);
         seconds = (int)MToUnsignedLong(vec[2]);
      }
      else
         error = true;

      if ( posBlank != MStdString::npos ) // only if days were given
         MENumberOutOfRange::CheckIntegerRange(0, 23, hours);
      MENumberOutOfRange::CheckIntegerRange(0, 59, minutes);
      MENumberOutOfRange::CheckIntegerRange(0, 59, seconds);
   }
   catch ( ... )
   {
      error = true;
   }
   if ( error )
   {
      MException::Throw(M_ERR_BAD_TIME_VALUE, M_I("Cannot convert given string to time span"));
      M_ENSURED_ASSERT(0);
   }
   if ( negative )
   {
      seconds = -seconds;
      minutes = -minutes;
      hours   = -hours;
      days    = -days;
   }
   Set(seconds, minutes, hours, days); // this can throw an error, too
}

   static void DoAddInteger(MStdString& buffer, int value, bool absolute)
   {
      char buff [ 16 ];
      if ( absolute && value < 0 )
         value = -value;
      size_t size = MFormat(buff, sizeof(buff), "%d", value);
      M_ASSERT(size < sizeof(buff)); // Check if the supplied buffer fits, as ensured by the format
      buffer.append(buff, size);
   }

   static void DoAddInteger02d(MStdString& buffer, int value, bool absolute)
   {
      char buff [ 16 ];
      char* p = buff;
      size_t len = 0;
      if ( value < 0 )
      {
         if ( !absolute )
         {
            *p++ = '-';
            ++len;
         }
         value = -value;
      }
      size_t size = MFormat(p, sizeof(buff) - len, "%02d", value);
      M_ASSERT(size < sizeof(buff)); // Check if the supplied buffer fits, as ensured by the format
      len += size;
      buffer.append(buff, len);
   }

MStdString MTimeSpan::AsFormattedString(MConstChars format) const
{
   M_ASSERT(format != NULL);

   MStdString buff;
   for ( ;; )
   {
      char ch = *format++;
      if ( ch == '\0' )
         break;
      if ( ch == '%' )
      {
         bool absolute;
         ch = *format++;
         if ( ch == '-' )
         {
            ch = *format++;
            if ( ch == '\0' )
            {
               buff.append("%-", 2);
               goto OUT_OF_LOOP;
            }
            absolute = false;
         }
         else
            absolute = true;
         switch ( ch  )
         {
         case '%':
            buff += '%';
            break;
         case 'N':
            if ( m_span < 0 )
               buff += '-';
            else if ( m_span > 0 )
               buff += '+';
            else
               buff += ' ';
            break;
         case 'n':
            if ( m_span < 0 )
               buff += '-';
            break;
         case 'D':
         case 'd':
            DoAddInteger(buff, GetDays(), absolute);
            break;
         case 'h':
            DoAddInteger(buff, ToHours(), absolute);
            break;
         case 'H':
            DoAddInteger02d(buff, GetHours(), absolute);
            break;
         case 'm':
            DoAddInteger(buff, ToMinutes(), absolute);
            break;
         case 'M':
            DoAddInteger02d(buff, GetMinutes(), absolute);
            break;
         case 's':
            DoAddInteger(buff, ToSeconds(), absolute);
            break;
         case 'S':
            DoAddInteger02d(buff, GetSeconds(), absolute);
            break;
         case 'c': // Best possible time span representation for current locale
            {
               int days = GetDays();
               if ( days != 0 )
               {
                  int hours = GetHours();
                  int minutes = GetMinutes();
                  int seconds = GetSeconds();
                  if ( m_span < 0 )
                  {
                     days = -days;
                     hours = -hours;
                     minutes = -minutes;
                     seconds = -seconds;
                     buff += '-';
                  }
                  buff += MGetStdString(M_I("%d days %02d:%02d:%02d"), days, hours, minutes, seconds); // locale dependent
               }
               else
                  buff += AsString();
            }
            break;
         case 'X': // Also see %c
            buff += AsString();
            break;
         case '\0':
            buff += '%';
            goto OUT_OF_LOOP;
         default:
            buff += '%';
            buff += ch;
            break;
         }
      }
      else
         buff += ch;
   }

OUT_OF_LOOP:
   return buff;
}

int MTimeSpan::GetHours() const
{
   int result = ToHours() - GetDays() * 24;
   M_ASSERT(result >= -23 && result <= 23);
   return result;
}

int MTimeSpan::GetMinutes() const
{
   int result = ToMinutes() - ToHours() * 60;
   M_ASSERT(result >= -59 && result <= 59);
   return result;
}

int MTimeSpan::GetSeconds() const
{
   int result = ToSeconds() - ToMinutes() * 60;
   M_ASSERT(result >= -59 && result <= 59);
   return result;
}

int MTimeSpan::CompareWithVariant(const MVariant& other) const
{
   if ( other.IsNumeric() )
      return Compare(MTimeSpan(other.AsInt()));
   return Compare(*M_DYNAMIC_CAST_WITH_THROW(MTimeSpan, other.AsExistingObject()));
}

MTimeSpan MTimeSpan::operator/(int numberOfTimes) const
{
   if ( numberOfTimes == 0 )
   {
      MException::ThrowDivisionByZero();
      M_ENSURED_ASSERT(0);
   }
   return MTimeSpan(m_span / numberOfTimes);
}

M_FUNC MTime& operator+=(MTime& p1, const MTimeSpan& p2)
{
   p1.CheckIfNotNull();
   p1.m_time += p2.m_span;
   return p1;
}

M_FUNC MTime& operator-=(MTime& p1, const MTimeSpan& p2)
{
   p1.CheckIfNotNull();
   p1.m_time -= p2.m_span;
   return p1;
}

M_FUNC MTime operator+(const MTime& p1, const MTimeSpan& p2)
{
   p1.CheckIfNotNull();
   MTime result(p1);
   result += p2;
   return result;
}

M_FUNC MTime operator+(const MTimeSpan& p1, const MTime& p2)
{
   return p2 + p1;
}

M_FUNC MTime operator-(const MTime& p1, const MTimeSpan& p2)
{
   p1.CheckIfNotNull();
   MTime result(p1);
   result -= p2;
   return result;
}

M_FUNC MTimeSpan operator-(const MTime& t1, const MTime& t2)
{
   return MTimeSpan(t1, t2);
}

#if !M_NO_VARIANT
unsigned MTimeSpan::GetEmbeddedSizeof() const
{
   M_COMPILED_ASSERT(sizeof(MTimeSpan) <= sizeof(MVariant::ObjectByValue));
   return sizeof(MTimeSpan);
}
#endif

#endif
