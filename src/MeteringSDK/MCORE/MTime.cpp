// File MCORE/MTime.cpp

#include "MCOREExtern.h"
#include "MTime.h"
#include "MTimeSpan.h"
#include "MUtilities.h"
#include "MTimeZone.h"
#include "MException.h"
#include "MAlgorithm.h"

#if !M_NO_TIME

   #if !M_NO_REFLECTION

      /// Default constructor, set the time to a null value.
      ///
      /// Null value is indistinguishable from "1970-01-01 00:00:00".
      ///
      static MVariant DoNew()
      {
         MTime time;
         return MVariant(&time, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      /// Create a new time from the parameter.
      ///
      /// \param v Will be one of the following:
      ///    - When object, this should be of Time type, and a copy will be created.
      ///    - When this is a numeric value, this is the count of seconds since January 1, 1970 (UNIX time).
      ///    - Otherwise the parameter is converted into a string, 
      ///      in which case it should have format like "2015-01-27 14:50:59".
      ///
      static MVariant DoNew1(const MVariant& v)
      {
         MTime time;
         if ( v.IsObject() )
            time = *M_DYNAMIC_CONST_CAST_WITH_THROW(MTime, v.AsObject());
         else if ( v.IsNumeric() )
            time.SetSecondsSince1970(v.AsDouble()); // use AsDouble so it works after 2038...
         else
            time.SetAsString(v.AsString());
         return MVariant(&time, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      /// Create a new time from date parameters such as year, month and day.
      ///
      /// The time of the created date will be 00:00:00.
      ///
      /// \param year Year of the date.
      /// \param month Month of the date.
      /// \param day Day of the date.
      ///
      static MVariant DoNewDate(int year, int month, int day)
      {
         MTime t(year, month, day);
         return MVariant(&t, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      static MVariant DoNewTime(int year, int month, int days, int hours, int minutes, int seconds)
      {
         MTime t(year, month, days, hours, minutes, seconds);
         return MVariant(&t, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      /// Add a time span to time object and return the result.
      ///
      /// The time object to which the time span object is added does not change.
      ///
      /// \param timeSpan Time span object, which is to be added.
      ///                 If the given object is not a time span, an error is thrown.
      /// \return Result Time object.
      ///
      MVariant MTime::Add(const MVariant& timeSpan) const
      {
         CheckIfNotNull();
         if ( MVariant::StaticIsObject(timeSpan) ) // this check is necessary to catch problems with reflective calls
         {
            const MObject* obj = timeSpan.AsExistingObject();
            if ( obj->GetClass() == MTimeSpan::GetStaticClass() )
            {
               MTime time = *this + *M_CHECKED_CAST(const MTimeSpan*, obj);
               return MVariant(&time, MVariant::ACCEPT_OBJECT_EMBEDDED);
            }
         }
         MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_BINARY_OPERATION_BETWEEN_INCOMPATIBLE_ARGUMENTS, "Binary operation between incompatible arguments"));
         M_ENSURED_ASSERT(0);
         return MVariant();
      }

      /// Subtract two time related objects.
      ///
      /// The time object from which the other object is subtracted does not change.
      ///
      /// \param other If this is time span object, the result is time.
      ///              If this is time object, the result is time span.
      /// \return Result Time or time span object, depending on the argument.
      ///
      MVariant MTime::Subtract(const MVariant& other) const
      {
         CheckIfNotNull();
         if ( MVariant::StaticIsObject(other) ) // this check is necessary to catch problems with reflective calls
         {
            const MObject* obj = other.AsExistingObject();
            const MClass* cls = obj->GetClass();
            if ( cls == MTimeSpan::GetStaticClass() )
            {
               MTime time = *this;
               time -= *M_CHECKED_CAST(const MTimeSpan*, obj);
               return MVariant(&time, MVariant::ACCEPT_OBJECT_EMBEDDED);
            }
            else if ( cls == MTime::GetStaticClass() )
            {
               MTimeSpan timeSpan = *this - *M_CHECKED_CAST(const MTime*, obj);
               return MVariant(&timeSpan, MVariant::ACCEPT_OBJECT_EMBEDDED);
            }
         }
         MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_BINARY_OPERATION_BETWEEN_INCOMPATIBLE_ARGUMENTS, "Binary operation between incompatible arguments"));
         M_ENSURED_ASSERT(0);
         return MVariant();
      }

      /// Create a copy time.
      ///
      /// \return Time, copy of this time object, same moment.
      ///
      MVariant MTime::NewClone() const
      {
         MTime t(*this);
         return MVariant(&t, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

   #endif

M_START_PROPERTIES(Time)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT    (Time, IsNull)
   M_OBJECT_PROPERTY_INT                    (Time, Year)
   M_OBJECT_PROPERTY_INT                    (Time, DayOfMonth)
   M_OBJECT_PROPERTY_READONLY_INT           (Time, DayOfYear)
   M_OBJECT_PROPERTY_INT                    (Time, Month)
   M_OBJECT_PROPERTY_INT                    (Time, Hours)
   M_OBJECT_PROPERTY_INT                    (Time, Minutes)
   M_OBJECT_PROPERTY_INT                    (Time, Seconds)
   M_OBJECT_PROPERTY_DOUBLE                 (Time, SecondsSince1970)
   M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED(Time, CurrentUtcTime,          ST_MObjectByValue_S)
   M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED(Time, CurrentLocalTime,        ST_MObjectByValue_S)
   M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED(Time, CurrentStandardTime,     ST_MObjectByValue_S)
   M_OBJECT_PROPERTY_READONLY_INT           (Time, DayOfWeek)
   M_OBJECT_PROPERTY_READONLY_INT           (Time, NumberOfDaysInThisMonth)
   M_OBJECT_PROPERTY_STRING_EXACT           (Time, AsString,                ST_MStdString_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_READONLY_OBJECT_EMBEDDED_EXACT(Time, AsDate,           ST_MObjectByValue_X)
M_START_METHODS(Time)
   M_OBJECT_SERVICE                         (Time, Compare,                 ST_int_X_MObjectP)
   M_OBJECT_SERVICE                         (Time, Set,                     ST_X_int_int_int_int_int_int)
   M_OBJECT_SERVICE                         (Time, SetDate,                 ST_X_int_int_int)
   M_OBJECT_SERVICE                         (Time, Add,                     ST_MVariant_X_constMVariantA)
   M_OBJECT_SERVICE                         (Time, Subtract,                ST_MVariant_X_constMVariantA)
   M_OBJECT_SERVICE                         (Time, SetToNull,               ST_X)
   M_OBJECT_SERVICE                         (Time, CheckIfNotNull,          ST_X)
   M_OBJECT_SERVICE                         (Time, AsFormattedString,       ST_MStdString_X_MConstChars)
   M_OBJECT_SERVICE                         (Time, GetWeekOfMonth,          ST_int_X_bool)
   M_OBJECT_SERVICE                         (Time, GetWeekOfYear,           ST_int_X_bool)
   M_CLASS_SERVICE                          (Time, IsLeapYear,              ST_bool_S_int)
   M_CLASS_FRIEND_SERVICE_OVERLOADED        (Time, New, DoNew,          0,  ST_MVariant_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED        (Time, New, DoNew1,         1,  ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED        (Time, New, DoNewDate,      3,  ST_MVariant_S_int_int_int)
   M_CLASS_FRIEND_SERVICE_OVERLOADED        (Time, New, DoNewTime,      6,  ST_MVariant_S_int_int_int_int_int_int)
   M_OBJECT_SERVICE                         (Time, NewClone,                ST_MVariant_X)
   M_CLASS_SERVICE                          (Time, Year2to4,                ST_int_S_int)
   M_CLASS_SERVICE                          (Time, Year4to2,                ST_int_S_int)
   M_CLASS_SERVICE                          (Time, GetNumberOfDaysInMonth,  ST_int_S_int_int)
M_END_CLASS(Time, Object)

const MTime MTime::s_null;
const int   MTime::s_leapYearDays    [ 13 ] = { -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
const int   MTime::s_nonLeapYearDays [ 13 ] = { -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364 };

const int s_minimumYear   = 1970; // we do not support negative UNIX epoch times
const int s_maximumYear   = 2099; // this one is artificial, practical

int MTime::Compare(const MTime& other) const
{
   if ( !MClass::StaticIsKindOf(other, MTime::GetStaticClass()) ) // this check is necessary to catch problems with reflective calls
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_BINARY_OPERATION_BETWEEN_INCOMPATIBLE_ARGUMENTS, "Binary operation between incompatible arguments"));
      M_ENSURED_ASSERT(0);
   }

   CheckIfNotNull();
   other.CheckIfNotNull();
   if ( m_time < other.m_time )
      return -1;
   if ( m_time > other.m_time )
      return 1;
   return 0;
}

void MTime::Set(int year, int month, int days, int hours, int mins, int secs)
{
   struct tm value;
   value.tm_sec = secs;
   value.tm_min = mins;
   value.tm_hour = hours;
   value.tm_mday = days;
   value.tm_mon = month - 1;         // tm_mon is 0 based
   value.tm_year = year - 1900;      // tm_year is 1900 based
#if !(M_OS & M_OS_NUTTX)
   value.tm_isdst = 0;               // we will do DST by ourselves! Tell we are not on DST
#endif
   SetTM(&value); // checks for ranges are done internally here
}

void MTime::SetTM(struct tm* value)
{
   // Year and month ranges are checked within GetNumberOfDaysInMonth, no need to check them here explicitly
   //
   if ( value->tm_mday < 1  || value->tm_mday > GetNumberOfDaysInMonth(value->tm_year + 1900, value->tm_mon + 1) ||
        value->tm_hour < 0  || value->tm_hour > 23 ||
        value->tm_min  < 0  || value->tm_min  > 59 ||
        value->tm_sec  < 0  || value->tm_sec  > 59
#if !(M_OS & M_OS_NUTTX)
        || value->tm_isdst != 0 // DST is outside of this class!
#endif
      )
   {
      MException::ThrowBadTimeValue();
      M_ENSURED_ASSERT(0);
   }

   int year = value->tm_year; // Year is 1900-based

   // Days. We have a simpler function for the leap year here, as we do not need the check.
   //
   int res = (year - 70) * 365 + ((year - 1) >> 2) - 17; // 17 is the number of leap years from 1900 to 1970
   const int* numberOfDays = ((year & 3) == 0) ? MTime::s_leapYearDays : MTime::s_nonLeapYearDays;
   res += numberOfDays[value->tm_mon]; // month is zero based
   res += value->tm_mday;

   res *= 24;           // Hours
   res += value->tm_hour;

   res *= 60;           // Minutes
   res += value->tm_min;

   // Seconds resolution appears only here, time to convert to 64-bit time
   //
   m_time = static_cast<InternalTimeType>(res) * 60 + static_cast<InternalTimeType>(value->tm_sec);
}

   const unsigned c_baseDayOfWeek   = 4;                   // Base day of week, 01-01-70 is Thursday
   const unsigned c_daySeconds      = 24 * 60 * 60;        // Number of seconds in a day
   const unsigned c_yearSeconds     = 365 * c_daySeconds;  // Number of seconds in a year
   const unsigned c_fourYearSeconds = 1461 * c_daySeconds; // Number of seconds in a 4-year interval (with leap year)

struct tm* MTime::GetTM(struct tm* value) const
{
   CheckIfNotNull();

   const int* yearDays = MTime::s_nonLeapYearDays;
   unsigned caltim = static_cast<unsigned>(m_time); // do all calculations with unsigned type, 2038 does not affect it

   unsigned tmptim = caltim / c_fourYearSeconds; // number of four-years
   caltim -= tmptim * c_fourYearSeconds;

   unsigned yday;
   tmptim = tmptim * 4 + 70;
   if ( caltim >= c_yearSeconds )
   {
      tmptim++;
      caltim -= c_yearSeconds;
      if ( caltim >= c_yearSeconds )
      {
         tmptim++;
         caltim -= c_yearSeconds;

         // It takes 366 days-worth of seconds to get past a leap year.
         if ( caltim >= (c_yearSeconds + c_daySeconds) )
         {
            tmptim++;
            caltim -= (c_yearSeconds + c_daySeconds);
         }
         else
            yearDays = MTime::s_leapYearDays;
      }
   }

   // tmptim is the value for tm_year.
   // caltim is the number of elapsed seconds since the beginning of year

   value->tm_year = tmptim;
   yday = caltim / c_daySeconds;
#if !(M_OS & M_OS_NUTTX)
   value->tm_yday = yday;
#endif
   caltim -= yday * c_daySeconds;

   for ( tmptim = 1; static_cast<unsigned>(yearDays[tmptim]) < yday; ++tmptim )
      ;

   --tmptim;
   value->tm_mon = tmptim;
   value->tm_mday = yday - static_cast<unsigned>(yearDays[tmptim]);
#if !(M_OS & M_OS_NUTTX)
   value->tm_wday = static_cast<int>(((static_cast<unsigned>(m_time) / c_daySeconds) + c_baseDayOfWeek) % 7); // Sunday is 0
#endif
   value->tm_hour = static_cast<int>(caltim / 3600); // Midnight is 0
   caltim -= static_cast<unsigned>(value->tm_hour) * 3600;
   value->tm_min = static_cast<int>(caltim / 60);
   value->tm_sec = static_cast<int>(caltim - (value->tm_min * 60));
#if !(M_OS & M_OS_NUTTX)
   value->tm_isdst = 0;
#endif

   #if M_DEBUG

      #if M_TIME_T_BIT_SIZE == 64
         int checkYear = value->tm_year + 1900;
         if ( checkYear > 1970 && checkYear < 2100 ) // The only range guaranteed by MeteringSDK
         {
            struct tm tm;

            // Use thread safe versions
            // gmtime does not do any DST or timezone
            //
            #if (M_OS & M_OS_WINDOWS) != 0
               gmtime_s(&tm, &m_time); 
            #else
               gmtime_r(&m_time, &tm);
            #endif

            M_ASSERT(tm.tm_year  == value->tm_year);
            M_ASSERT(tm.tm_mon   == value->tm_mon);
            M_ASSERT(tm.tm_mday  == value->tm_mday);
            M_ASSERT(tm.tm_wday  == value->tm_wday);
            M_ASSERT(tm.tm_hour  == value->tm_hour);
            M_ASSERT(tm.tm_min   == value->tm_min);
            M_ASSERT(tm.tm_sec   == value->tm_sec);
            M_ASSERT(tm.tm_isdst == value->tm_isdst);
         }
      #endif

      {
         MTime tmp;
         tmp.SetTM(value); // this does a bunch of checks already
         M_ASSERT(tmp.m_time == m_time);
      }

   #endif

   return value;
}

int MTime::GetYear() const
{
   struct tm value;
   return GetTM(&value)->tm_year + 1900;
}

void MTime::SetYear(int year)
{
   struct tm value;
   GetTM(&value);
   value.tm_year = year - 1900;
   SetTM(&value); // checks are done internally in this service
}

int MTime::GetDayOfMonth() const
{
   struct tm value;
   return GetTM(&value)->tm_mday;
}

void MTime::SetDayOfMonth(int day)
{
   struct tm value;
   GetTM(&value);
   value.tm_mday = day;
   SetTM(&value); // checks are done internally in this service
}

#if (M_OS & M_OS_NUTTX) == 0
int MTime::GetDayOfYear() const
{
   struct tm value;
   return GetTM(&value)->tm_yday + 1; // tm_yday starts from zero
}
#endif

int MTime::GetMonth() const
{
   struct tm value;
   return GetTM(&value)->tm_mon + 1;
}

void MTime::SetMonth(int month)
{
   struct tm value;
   GetTM(&value);
   value.tm_mon = month - 1;
   SetTM(&value); // checks are done internally in this service
}

int MTime::GetHours() const
{
   struct tm value;
   return GetTM(&value)->tm_hour;
}

void MTime::SetHours(int hours)
{
   struct tm value;
   GetTM(&value);
   value.tm_hour = hours;
   SetTM(&value); // checks are done internally in this service
}

int MTime::GetMinutes() const
{
   struct tm value;
   return GetTM(&value)->tm_min;
}

void MTime::SetMinutes(int mins)
{
   struct tm value;
   GetTM(&value);
   value.tm_min = mins;
   SetTM(&value); // checks are done internally in this service
}

int MTime::GetSeconds() const
{
   struct tm value;
   return GetTM(&value)->tm_sec;
}

void MTime::SetSeconds(int secs)
{
   struct tm value;
   GetTM(&value);
   value.tm_sec = secs;
   SetTM(&value); // checks are done internally in this service
}

MTime::DayOfWeekType MTime::GetDayOfWeek() const
{
   CheckIfNotNull();
   int wday = static_cast<int>(((static_cast<unsigned>(m_time) / c_daySeconds) + c_baseDayOfWeek) % 7); // Sunday is 0
   M_ASSERT(wday >= MTime::WeekdaySunday && wday <= MTime::WeekdaySaturday);
#if M_DEBUG && (M_OS & M_OS_NUTTX) == 0
   struct tm value;
   M_ASSERT(GetTM(&value)->tm_wday == wday);
#endif
   return static_cast<MTime::DayOfWeekType>(wday);
}

int MTime::GetWeekOfMonth(bool startsOnSunday) const
{
   int week = GetDayOfWeek();
   if ( !startsOnSunday )
   {
      if ( week == 0 )
         week = 6;
      else
         --week;
   }
   int day = GetDayOfMonth();
   int result = (day + 6 - week) / 7;
   M_ASSERT(result >= 0 && result <= 5);
   return result;
}

int MTime::GetWeekOfYear(bool startsOnSunday) const
{
   int week = GetDayOfWeek();
   if ( !startsOnSunday )
   {
      if ( week == 0 )
         week = 6;
      else
         --week;
   }
   int day = GetDayOfYear();
   int result = (day + 6 - week) / 7;
   M_ASSERT(result >= 0 && result <= 53);
   return result;
}

MStdString MTime::AsString() const
{
   MStdString result;
   if ( IsNull() )
   {
      #ifdef M_USE_USTL
         result.append((size_t)1, '0');
      #else
         result.assign(1, '0');
      #endif
   }
   else
   {
      MChar buff [ 64 ];
      struct tm t;
      GetTM(&t);
      size_t len = MFormat(buff, 128, "%04d-%02d-%02d %02d:%02d:%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
      result.assign(buff, len);
   }
   return result;
}

void MTime::SetAsString(const MStdString& str)
{
   int second = 0;
   int minute = 0;
   int hour = 0;
   int day = 0;   // will be overwritten
   int month = 0; // will be overwritten
   int year = 0;  // will be overwritten
   bool error = false;
   bool timeGiven = false;
   try
   {
      MStdStringVector vecDateTime = MAlgorithm::SplitWithDelimiter(str, ' ', true);
      if ( vecDateTime.size() == 2 ) // both date and time are given
      {
         MStdStringVector vecTime = MAlgorithm::SplitWithDelimiter(vecDateTime[1], ':', false, true);
         if ( vecTime.size() == 2 || vecTime.size() == 3 )
         {
            hour = MToLong(vecTime[0]);
            minute = MToLong(vecTime[1]);
            if ( vecTime.size() == 3 )
               second = MToLong(vecTime[2]);
            timeGiven = true;
         }
         else
            error = true;
      }

      if ( !error && (vecDateTime.size() == 2 || vecDateTime.size() == 1) )
      {
         MStdStringVector vecDate = MAlgorithm::SplitWithDelimiter(vecDateTime[0], '-', false, true);
         if ( vecDate.size() == 3 )
         {
            year = MToLong(vecDate[0]);
            month = MToLong(vecDate[1]);
            day = MToLong(vecDate[2]);
         }
         else if ( vecDate.size() == 1 )
         {
            vecDate = MAlgorithm::SplitWithDelimiter(vecDateTime[0], '/', false, true);
            if ( vecDate.size() == 3 )
            {
               month = MToLong(vecDate[0]);
               day = MToLong(vecDate[1]);
               year = MToLong(vecDate[2]);
               if ( year < 70 ) // handle short year only for US dates
                  year += 2000;
               else if ( year < 100 )
                  year += 1900;
            }
            else if ( vecDate.size() == 2 )
            {
               month = MToLong(vecDate[0]);
               day = MToLong(vecDate[1]);
               year = MTime::GetCurrentLocalTime().GetYear();
            }
            else if ( vecDate.size() == 1 && !timeGiven && MToLong(vecDate[0]) == 0 )
            {
               SetToNull();
               return;
            }
            else
               error = true;
         }
         else
            error = true;
      }
      else
         error = true;

      if ( !error )
         Set(year, month, day, hour, minute, second); // this can throw an error
   }
   catch ( ... )
   {
      error = true;
   }

   if ( error )
   {
      MException::ThrowBadTimeValue(str.c_str());
      M_ENSURED_ASSERT(0);
   }
}

   const size_t s_localTimeBufferSize = 256;
   const size_t s_localTimeShortBufferSize = s_localTimeBufferSize / 2 - 1; // "date" or "time" in "date time"

   // Format type, should match android Java definition exactly
   //
   enum DateTimeFormatEnum
   {
      FORMAT_DATE_TIME = 0,
      FORMAT_DATE = 1,
      FORMAT_TIME = 2,
      FORMAT_AMPM = 3
   };

   #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
      #include <jni.h> // repeat the include purely for the IDE to recognize types
      #include "MJavaEnv.h"

      static const char s_androidDateTimeFormatterClassName[] = "com/elster/MTools/android/DateTimeFormatter";
      static jmethodID s_idFormat = NULL;
   #endif // (M_OS & M_OS_ANDROID) != 0

   static size_t DoFormatLocalDateTime(char* buffer, const MTime& time, DateTimeFormatEnum format, bool useUserLocale)
   {
      size_t size = 0;

      #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI

         MJavaEnv env;
         jclass clazz = env.FindClass(s_androidDateTimeFormatterClassName); // do not cache class object, it is thread dependent!
         if ( s_idFormat == NULL ) // but cache methods
            s_idFormat = env.GetStaticMethodID(clazz, "format", "(JI)Ljava/lang/String;");
         jstring jStr = (jstring)env->CallStaticObjectMethod(clazz, s_idFormat, (jlong)time.GetTimeT(), (jint)format);
         env.CheckForJavaException();
         const char* cStr = env->GetStringUTFChars(jStr, NULL);
         env.CheckForJavaException();
         size = strlen(cStr);
         if ( size >= s_localTimeShortBufferSize ) // this is impossible in the correct Android environment
         {
            M_ASSERT(0);                           // report on Debug
            size = s_localTimeShortBufferSize - 1; //    but recover in Release
         }
         memcpy(buffer, cStr, size);
         env->ReleaseStringUTFChars(jStr, cStr);
         env->DeleteLocalRef(jStr);

      #elif (M_OS & M_OS_WINDOWS) != 0

         struct tm tm;
         time.GetTM(&tm);
         SYSTEMTIME t;
         t.wYear   = (WORD)(tm.tm_year + 1900);
         t.wMonth  = (WORD)(tm.tm_mon + 1);
         t.wDay    = (WORD)(tm.tm_mday);
         t.wHour   = (WORD)(tm.tm_hour);
         t.wMinute = (WORD)(tm.tm_min);
         t.wSecond = (WORD)(tm.tm_sec);
         t.wMilliseconds = 0;

         size_t ret;
         LCID locale = useUserLocale ? ::GetUserDefaultLCID() : ::GetThreadLocale();
         switch ( format )
         {
         default:
            M_ENSURED_ASSERT(0);
         case FORMAT_DATE_TIME:
            size = ::GetDateFormatA(locale, DATE_SHORTDATE, &t, NULL, buffer, s_localTimeShortBufferSize);
            MESystemError::CheckLastSystemError(size <= 0);
            buffer[size - 1] = ' ';
            // fall through into time
         case FORMAT_TIME:
            ret = ::GetTimeFormatA(locale, 0, &t, NULL, buffer + size, s_localTimeShortBufferSize);
            MESystemError::CheckLastSystemError(ret <= 0);
            size += ret;
            break;
         case FORMAT_DATE:
            size = ::GetDateFormatA(locale, DATE_SHORTDATE, &t, NULL, buffer, s_localTimeShortBufferSize);
            MESystemError::CheckLastSystemError(size <= 0);
            break;
         case FORMAT_AMPM:
            size = ::GetTimeFormatA(locale, 0, &t, "tt", buffer, s_localTimeShortBufferSize);
            MESystemError::CheckLastSystemError(size <= 0);
            break;
         }
         --size; // remove '\0'

      #else

         #if !(M_OS & M_OS_NUTTX) && !(M_OS & M_OS_CMX)
            #ifdef __BORLANDC__ // this actually never happens currently as (M_OS & M_OS_WINDOWS) != 0 takes precedence
               #define M__LOCALE_KIND LC_ALL  // Way to avoid Borland crash at the following conversion routine such as strtod()
            #else
               #define M__LOCALE_KIND LC_TIME // Standard and optimum way of setting time locale in the current thread
            #endif
            MStdString currentLocale;
            if ( useUserLocale )
            {
               const char* localeChars = setlocale(M__LOCALE_KIND, NULL);
               if ( localeChars != NULL )
                  currentLocale = localeChars;
               setlocale(M__LOCALE_KIND, "");
            }
         #endif

         struct tm tm;
         time.GetTM(&tm);
         const char* fmt;
         switch ( format )
         {
         default:
            M_ENSURED_ASSERT(0);
         case FORMAT_DATE_TIME:
            fmt = "%x %X";
            break;
         case FORMAT_TIME:
            fmt = "%X";
            break;
         case FORMAT_DATE:
            fmt = "%x";
            break;
         case FORMAT_AMPM:
            fmt = "%p";
            break;
         }
         size = strftime(buffer, s_localTimeShortBufferSize, fmt, &tm);
         // size == 0 is not an error, as it is a valid result of %p

         #if !(M_OS & M_OS_NUTTX) && !(M_OS & M_OS_CMX)
            if ( useUserLocale && !currentLocale.empty() )
               setlocale(M__LOCALE_KIND, currentLocale.c_str());
            #undef M__LOCALE_KIND
         #endif
      #endif

      return size;
   }

   static M_NORETURN_FUNC void DoThrowBadPrintFormat(char c)
   {
      if ( c == '\0' )
         c = '%';
      MException::Throw(MException::ErrorSoftware, M_ERR_BAD_PRINT_FORMAT_S1, "Bad print format '%c'", c);
      M_ENSURED_ASSERT(0);
   }

#if M_DEBUG
    static void DoCheckWithStrftime(const MTime& time, const char* buffer, size_t size, const char* fmt, bool useShortFormat)
    {
       struct tm tm;
       time.GetTM(&tm);
       char checkBuffer [ s_localTimeBufferSize ];
       size_t checkSize = strftime(checkBuffer, s_localTimeBufferSize, fmt, &tm);
       char* p = checkBuffer;
       if ( useShortFormat )
       {
          while ( *p == '0' && *(p + 1) != '\0' )
          {
             ++p;
             --checkSize;
          }
       }
       M_ASSERT(checkSize == size);
       M_ASSERT(memcmp(buffer, p, size) == 0);
    }
#else
   inline void DoCheckWithStrftime(const MTime&, const char*, size_t, const char*, bool useShortFormat)
   {
   }
#endif

MStdString MTime::AsFormattedString(MConstChars format) const
{  // apparently, strftime is very different on all platforms
   MStdString result;

   CheckIfNotNull(); // as some of the below formats do not do the check

   size_t size;
   char buffer [ s_localTimeBufferSize ]; // big enough for every number and short string

   for ( const char* p = format; *p != '\0'; ++p )
   {
      if ( *p != '%' )
         result += *p;
      else
      {
         ++p;
         bool useShortFormat = false;
         bool useUserLocale = false;

      SWITCH_AGAIN:
         switch ( *p )
         {
         default:
            DoThrowBadPrintFormat(*p); // if it is \0, no problem, will be reported as '%'
            M_ENSURED_ASSERT(0);
         case '@':
            if ( useUserLocale || useShortFormat ) // cannot have both, cannot repeat same
            {
               DoThrowBadPrintFormat(*p);
               M_ENSURED_ASSERT(0);
            }
            useUserLocale = true;
            ++p;
            goto SWITCH_AGAIN;
         case '#':
            if ( useUserLocale || useShortFormat ) // cannot have both, cannot repeat same
            {
               DoThrowBadPrintFormat(*p);
               M_ENSURED_ASSERT(0);
            }
            useShortFormat = true;
            ++p;
            goto SWITCH_AGAIN;
         case '%': // Replaced by a single %
            buffer[0] = '%';
            size = 1;
            DoCheckWithStrftime(*this, buffer, size, "%%", false);
            break;
         case 'c': // Date and time in a locale sensitive representation
            size = DoFormatLocalDateTime(buffer, *this, FORMAT_DATE_TIME, useUserLocale);
            break;
         case 'd': // Two-digit day of month with possible leading zero, 01 .. 31
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", GetDayOfMonth());
            DoCheckWithStrftime(*this, buffer, size, "%d", useShortFormat);
            break;
         case 'H': // Hour, 24 hour format 01 .. 23
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", GetHours());
            DoCheckWithStrftime(*this, buffer, size, "%H", useShortFormat);
            break;
         case 'I': // Hour, 12 hour format 01 .. 12, note that this is upper case letter i
            {
               int hours = GetHours();
               hours = (hours % 12) != 0
                     ? (hours % 12)
                     : 12;
               size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", hours);
               DoCheckWithStrftime(*this, buffer, size, "%I", useShortFormat);
            }
            break;
         case 'j': // Day of year, 001 .. 366
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%03d", GetDayOfYear());
            DoCheckWithStrftime(*this, buffer, size, "%j", useShortFormat);
            break;
         case 'm': // Digits for month, 01 .. 12
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", GetMonth());
            DoCheckWithStrftime(*this, buffer, size, "%m", useShortFormat);
            break;
         case 'M': // Minute, 00 .. 59
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", GetMinutes());
            DoCheckWithStrftime(*this, buffer, size, "%M", useShortFormat);
            break;
         case 'p': // AM or PM
            size = DoFormatLocalDateTime(buffer, *this, FORMAT_AMPM, useUserLocale);
            break;
         case 'q': // Week of the current month, 00 .. 06, where 01 is the date of the first Sunday
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", static_cast<int>(GetWeekOfMonth(true)));
            break;
         case 'Q': // Week of the current month, 00 .. 06, where 01 is the date of the first Monday
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", static_cast<int>(GetWeekOfMonth(false)));
            break;
         case 'S': // Second, 00 .. 59
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", GetSeconds());
            DoCheckWithStrftime(*this, buffer, size, "%S", useShortFormat);
            break;
         case 'u': // Weekday, 1 .. 7, starting from Monday (also see 'w')
            {
               int weekday = static_cast<int>(GetDayOfWeek());
               if ( weekday == 0 )  // move Sunday to become 7
                  weekday = 7;
               size = MFormat(buffer, sizeof(buffer), "%d", weekday);
               #if !defined(__BORLANDC__) && !defined(_MSC_VER) // Borland and Visual C++ do not implement it
                  DoCheckWithStrftime(*this, buffer, size, "%u", false);
               #endif
            }
            break;
         case 'U': // Week of the current year, 00 .. 53, where 01 is the week of the first Sunday
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", static_cast<int>(GetWeekOfYear(true)));
            DoCheckWithStrftime(*this, buffer, size, "%U", useShortFormat);
            break;
         case 'w': // Weekday, 0 .. 6, starting from Sunday (also see 'u')
            size = MFormat(buffer, sizeof(buffer), "%d", static_cast<int>(GetDayOfWeek()));
            DoCheckWithStrftime(*this, buffer, size, "%w", false);
            break;
         case 'W': // Week of the current year, 00 .. 53, where 01 is the week of the first Monday
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", static_cast<int>(GetWeekOfYear(false)));
            DoCheckWithStrftime(*this, buffer, size, "%W", useShortFormat);
            break;
         case 'x': // Date fraction
            size = DoFormatLocalDateTime(buffer, *this, FORMAT_DATE, useUserLocale);
            break;
         case 'X': // Time fraction
            size = DoFormatLocalDateTime(buffer, *this, FORMAT_TIME, useUserLocale);
            break;
         case 'y': // Two-digit year format, 00 .. 99
            size = MFormat(buffer, sizeof(buffer), useShortFormat ? "%d" : "%02d", Year4to2(GetYear()));
            DoCheckWithStrftime(*this, buffer, size, "%y", useShortFormat);
            break;
         case 'Y': // Full four-digit year
            size = MFormat(buffer, sizeof(buffer), "%04d", GetYear());
            DoCheckWithStrftime(*this, buffer, size, "%Y", false);
            break;
         }
         result.append(buffer, size);
      }
   }
   return result;
}

MTime MTime::AsDate() const
{
   struct tm value;
   GetTM(&value);
   value.tm_hour = 0;
   value.tm_min = 0;
   value.tm_sec = 0;
   return MTime(&value);
}

void MTime::CheckIfNotNull() const
{
   if ( IsNull() )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
}


   #if ((M_OS & M_OS_WIN32_CE) != 0) // Windows CE does not have GetSystemTimeAsFileTime. Implement using the above inline

      inline void DoGetSystemTimeAsFileTime(FILETIME* fileTime)
      {
         // The following two system calls are equivalent to GetSystemTimeAsFileTime, but it is not supported on CE
         //
         SYSTEMTIME systemTime;
         GetSystemTime(&systemTime);
         SystemTimeToFileTime(&systemTime, fileTime);
      }

      inline void GetSystemTimeAsFileTime(FILETIME* fileTime)
      {
         DoGetSystemTimeAsFileTime(fileTime);
      }

   #endif

MTime::InternalTimeType MTime::GetUtcSecondsSince1970()
{
#if M_OS & M_OS_POSIX
   time_t t = time(NULL);
   return DoTimeToInternal(t);
#elif M_OS & M_OS_CMX
   ???
   return 0;
#else
   #ifdef __BORLANDC__
      Muint64 biasBetweenEpocs   = MUINT64C(116444736000000000);
      Muint64 nanosecondsDivisor = MUINT64C(10000000);
   #else
      const Muint64 biasBetweenEpocs   = MUINT64C(116444736000000000);
      const Muint64 nanosecondsDivisor = MUINT64C(10000000);
   #endif

   Muint64 fileTimeUint;
   GetSystemTimeAsFileTime((FILETIME*)&fileTimeUint);

   Muint64 time64 = (fileTimeUint - biasBetweenEpocs) / nanosecondsDivisor;
   M_ASSERT(time64 < MUINT64C(0xFFFFFFFF)); // check if we are beyond the year of 2048...
   return static_cast<InternalTimeType>(time64);
#endif
}

MTime MTime::GetCurrentLocalTime()
{
   return MTimeZone::GetCurrent()->GetLocalTime();
}

MTime MTime::GetCurrentStandardTime()
{
   return MTimeZone::GetCurrent()->GetStandardTime();
}

int MTime::GetNumberOfDaysInMonth(int year, int month)
{
   if ( month < MonthJanuary || month > MonthDecember ||
        year < s_minimumYear || year > s_maximumYear )
   {
      MException::ThrowBadTimeValue();
      M_ENSURED_ASSERT(0);
   }
   const int* days = IsLeapYear(year) ? s_leapYearDays : s_nonLeapYearDays;
   return days[month] -  days[month - 1];
}

bool MTime::IsLeapYear(int year)
{
   if ( year < s_minimumYear || year > s_maximumYear )
   {
      MException::ThrowBadTimeValue();
      M_ENSURED_ASSERT(0);
   }

   bool isLeap = (year % 400 == 0) || ((year % 100 != 0) && (year % 4 == 0));
   return isLeap;
}

int MTime::Year2to4(int year)
{
   if ( year < 0 || year > 99 )
   {
      MException::ThrowBadTimeValue();
      M_ENSURED_ASSERT(0);
   }
   return year + ((year < 90) ? 2000 : 1900);
}

int MTime::Year4to2(int year)
{
   if ( year < 1990 || year > 2089 )
   {
      MException::ThrowBadTimeValue();
      M_ENSURED_ASSERT(0);
   }
   return year - ((year >= 2000) ? 2000 : 1900);
}

#if !M_NO_VARIANT
unsigned MTime::GetEmbeddedSizeof() const
{
   M_COMPILED_ASSERT(sizeof(MTime) <= sizeof(MVariant::ObjectByValue));
   return sizeof(MTime);
}
#endif

#endif
