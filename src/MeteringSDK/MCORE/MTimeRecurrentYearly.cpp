// File MCORE/MTimeRecurrentYearly.cpp

#include "MCOREExtern.h"
#include "MTimeRecurrentYearly.h"
#include "MException.h"

#if !M_NO_TIME

   #if !M_NO_REFLECTION

      static MVariant DoNew0()
      {
         MTimeRecurrentYearly t;
         return MVariant(&t, MVariant::ACCEPT_OBJECT_EMBEDDED);
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
      static MVariant DoNewOnDay(MTimeRecurrentYearly::OffsetType offsetType, MTime::MonthType month, int dayOfMonth, int hour, int minute)
      {
         MTimeRecurrentYearly t(offsetType, month, dayOfMonth, hour, minute);
         return MVariant(&t, MVariant::ACCEPT_OBJECT_EMBEDDED);
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
      static MVariant DoNewOnWeekday(MTimeRecurrentYearly::OffsetType offsetType, MTime::MonthType month, int dayOfMonth, int hour, int minute, MTime::DayOfWeekType weekday)
      {
         MTimeRecurrentYearly t(offsetType, month, dayOfMonth, hour, minute, weekday);
         return MVariant(&t, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

   #endif

M_START_PROPERTIES(TimeRecurrentYearly)
   M_OBJECT_PROPERTY_INT                  (TimeRecurrentYearly, OffsetType)
   M_OBJECT_PROPERTY_INT                  (TimeRecurrentYearly, Month)
   M_OBJECT_PROPERTY_INT                  (TimeRecurrentYearly, DayOfMonth)
   M_OBJECT_PROPERTY_INT                  (TimeRecurrentYearly, DayOfWeek)
   M_OBJECT_PROPERTY_INT                  (TimeRecurrentYearly, Hours)
   M_OBJECT_PROPERTY_INT                  (TimeRecurrentYearly, Minutes)
   M_OBJECT_PROPERTY_INT                  (TimeRecurrentYearly, Seconds)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT  (TimeRecurrentYearly, IsDayOfWeekIgnored)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT  (TimeRecurrentYearly, IsNull)

   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetNo)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetWeekdayBefore)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetWeekdayFirstAfter)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetWeekdaySecondAfter)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetWeekdayThirdAfter)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetWeekdayFourthAfter)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetWeekdayLastAfter)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetObserveOnThisAndFollowingDate)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetMondayIfSunday)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetFridayIfSunday)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetMondayIfSaturday)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetFridayIfSaturday)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetMondayIfSaturdayOrSunday)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetFridayIfSaturdayOrSunday)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetMondayIfSundayFridayIfSaturday)
   M_CLASS_ENUMERATION                    (TimeRecurrentYearly, OffsetObserveOnFollowingDate)
M_START_METHODS(TimeRecurrentYearly)
   M_OBJECT_SERVICE                       (TimeRecurrentYearly, CheckIsValid,                  ST_X)
   M_OBJECT_SERVICE                       (TimeRecurrentYearly, SetToNull,                     ST_X)
   M_OBJECT_SERVICE                       (TimeRecurrentYearly, GetPertinent,                  ST_MObjectByValue_X_MObjectP)
   M_OBJECT_SERVICE                       (TimeRecurrentYearly, GetPertinentForYear,           ST_MObjectByValue_X_int)
   M_OBJECT_SERVICE                       (TimeRecurrentYearly, SetOnDay,                      ST_X_int_int_int_int_int)
   M_OBJECT_SERVICE                       (TimeRecurrentYearly, SetOnWeekday,                  ST_X_int_int_int_int_int_int)
   M_OBJECT_SERVICE                       (TimeRecurrentYearly, NewClone,                      ST_MVariant_X)
   M_CLASS_FRIEND_SERVICE                 (TimeRecurrentYearly, New,          DoNew0,          ST_MVariant_S)
   M_CLASS_FRIEND_SERVICE                 (TimeRecurrentYearly, NewOnDay,     DoNewOnDay,      ST_MVariant_S_int_int_int_int_int)
   M_CLASS_FRIEND_SERVICE                 (TimeRecurrentYearly, NewOnWeekday, DoNewOnWeekday,  ST_MVariant_S_int_int_int_int_int_int)
M_END_CLASS(TimeRecurrentYearly, TimeRecurrent)

MTimeRecurrentYearly::~MTimeRecurrentYearly()
{
}

MTimeRecurrentYearly::OffsetType MTimeRecurrentYearly::GetOffsetType() const
{
   CheckIfNotNull();
   return OffsetType(m_value.m_field.m_offsetType);
}

void MTimeRecurrentYearly::SetOffsetType(OffsetType type)
{
   MENumberOutOfRange::CheckInteger(int(OffsetNo), int(OffsetObserveOnFollowingDate), int(type), M_OPT_STR("OffsetType"));
   m_value.m_field.m_offsetType = type;
}

bool MTimeRecurrentYearly::IsDayOfWeekIgnored() const
{
   CheckIfNotNull();
   return (OffsetType)m_value.m_field.m_offsetType < OffsetWeekdayBefore || (OffsetType)m_value.m_field.m_offsetType > OffsetWeekdayLastAfter;
}

MTime::MonthType MTimeRecurrentYearly::GetMonth() const
{
   CheckIfNotNull();
   return MTime::MonthType(m_value.m_field.m_month);
}

void MTimeRecurrentYearly::SetMonth(MTime::MonthType month)
{
   MENumberOutOfRange::CheckInteger(int(MTime::MonthJanuary), int(MTime::MonthDecember), int(month), M_OPT_STR("Month"));
   m_value.m_field.m_month = month;
}

int MTimeRecurrentYearly::GetDayOfMonth() const
{
   CheckIfNotNull();
   return int(m_value.m_field.m_day);
}

void MTimeRecurrentYearly::SetDayOfMonth(int day)
{
   MENumberOutOfRange::CheckInteger(1, 31, int(day), M_OPT_STR("DayOfMonth"));
   m_value.m_field.m_day = day;
}

MTime::DayOfWeekType MTimeRecurrentYearly::GetDayOfWeek() const
{
   CheckIfNotNull();
   return MTime::DayOfWeekType(m_value.m_field.m_weekday);
}

void MTimeRecurrentYearly::SetDayOfWeek(MTime::DayOfWeekType weekday)
{
   MENumberOutOfRange::CheckInteger(int(MTime::WeekdaySunday), int(MTime::WeekdaySaturday), int(weekday), M_OPT_STR("Weekday"));
   m_value.m_field.m_weekday = weekday;
}

int MTimeRecurrentYearly::GetHours() const
{
   CheckIfNotNull();
   return m_value.m_field.m_hours;
}

void MTimeRecurrentYearly::SetHours(int hours)
{
   MENumberOutOfRange::CheckInteger(0, 23, int(hours), M_OPT_STR("Hours"));
   m_value.m_field.m_hours = hours;
}

int MTimeRecurrentYearly::GetMinutes() const
{
   CheckIfNotNull();
   return m_value.m_field.m_minutes;
}

void MTimeRecurrentYearly::SetMinutes(int minutes)
{
   MENumberOutOfRange::CheckInteger(0, 59, int(minutes), M_OPT_STR("Minutes"));
   m_value.m_field.m_minutes = minutes;
}

int MTimeRecurrentYearly::GetSeconds() const
{
   CheckIfNotNull();
   return m_value.m_field.m_seconds;
}

void MTimeRecurrentYearly::SetSeconds(int seconds)
{
   MENumberOutOfRange::CheckInteger(0, 59, int(seconds), M_OPT_STR("Seconds"));
   m_value.m_field.m_seconds = seconds;
}

void MTimeRecurrentYearly::SetOnDay(OffsetType offsetType, MTime::MonthType month, int day, int hour, int minute)
{
   MTimeRecurrentYearly candidate;
   candidate.SetUnchecked(offsetType, month, day, hour, minute);
   if ( !candidate.IsDayOfWeekIgnored() )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_BAD_TIME_VALUE, "Yearly recurrent day-based date has incompatible offset type"));
      M_ENSURED_ASSERT(0);
   }
   candidate.CheckIsValid(); // this can throw an exception
   operator=(candidate); // assign a valid recurrent date
}

void MTimeRecurrentYearly::SetOnWeekday(OffsetType offsetType, MTime::MonthType month, int day, int hour, int minute, MTime::DayOfWeekType weekday)
{
   MTimeRecurrentYearly candidate;
   candidate.SetUnchecked(offsetType, month, day, hour, minute, 0, weekday);
   if ( candidate.IsDayOfWeekIgnored() )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_BAD_TIME_VALUE, "Yearly recurrent weekday-based date has incompatible offset type"));
      M_ENSURED_ASSERT(0);
   }
   candidate.CheckIsValid(); // this can throw an exception
   operator=(candidate); // assign a valid recurrent date
}

#if (M_OS & M_OS_WIN32) != 0

void MTimeRecurrentYearly::SetFromTimeZoneTime(const SYSTEMTIME& ti) M_NO_THROW
{
   // Reference: https://msdn.microsoft.com/en-us/library/ms725481.aspx

   SetToNull();
   if ( ti.wYear != 0 ) // exact yearly date translates into a fixed recurring date
   {
      M_ASSERT(0); // !!?? the below code is most likely a bad idea, signal if used
      SetUnchecked(OffsetNo, MTime::MonthType(ti.wMonth), ti.wDay, ti.wHour, ti.wMinute, ti.wSecond, MTime::DayOfWeekType(ti.wDayOfWeek));
   }
   else if ( ti.wMonth != 0 && ti.wDay >= 1 && ti.wDay <= 5 ) // Windows-defined week number in the recurring date translates to a proper offset type
   {
      OffsetType offsetType = OffsetType(OffsetWeekdayFirstAfter - 1 + ti.wDay);
      SetUnchecked(offsetType, MTime::MonthType(ti.wMonth), 1, ti.wHour, ti.wMinute, ti.wSecond, MTime::DayOfWeekType(ti.wDayOfWeek));
   }
}

void MTimeRecurrentYearly::ChangeTimeZoneTime(SYSTEMTIME& ti) const
{
   if ( IsNull() )
   {
      ZeroMemory(&ti, sizeof(SYSTEMTIME));
   }
   else
   {
      OffsetType offsetType = GetOffsetType();
      if ( offsetType < OffsetWeekdayFirstAfter || offsetType > OffsetWeekdayLastAfter )
      {
         MException::Throw(M_ERR_RECURRENT_TIME_OFFSET_D1_IS_NOT_SUPPORTED_BY_OS, M_I("Recurrent time offset %d is not supported by OS"), offsetType);
         M_ENSURED_ASSERT(0);
      }
      M_ASSERT(!IsDayOfWeekIgnored());

      ZeroMemory(&ti, sizeof(SYSTEMTIME));

      ti.wMonth     = (WORD)GetMonth();
      ti.wDay       = (WORD)(int(offsetType) - (int(OffsetWeekdayFirstAfter) - 1));
      ti.wDayOfWeek = (WORD)GetDayOfWeek();
      ti.wHour      = (WORD)GetHours();
      ti.wMinute    = (WORD)GetMinutes();
      ti.wSecond    = (WORD)GetSeconds();

      M_ASSERT(ti.wYear         == 0); // these fields are initialized with the above ZeroMemory call
      M_ASSERT(ti.wMilliseconds == 0); // these fields are initialized with the above ZeroMemory call
   }
}

#endif

   const int s_secondsInDay = 60 * 60 * 24;
   const int s_secondsInWeek = s_secondsInDay * 7;

MTime MTimeRecurrentYearly::GetPertinent(const MTime& tagTime) const
{
   return GetPertinentForYear(tagTime.GetYear());
}

MTime MTimeRecurrentYearly::GetPertinentForYear(int year) const
{
   if ( IsNull() )
      return MTime();

   OffsetType type = GetOffsetType();
   MTime ti(year, GetMonth(), ((type == OffsetWeekdayLastAfter) ? 1 : GetDayOfMonth()), GetHours(), GetMinutes(), GetSeconds());
   if ( type != OffsetNo )
   {
      MTime::DayOfWeekType weekday = ti.GetDayOfWeek();
      if ( !IsDayOfWeekIgnored() )
      {
         int daysDiff = GetDayOfWeek() - weekday;
         int secondsDiff = daysDiff * s_secondsInDay;
         if ( secondsDiff != 0 )
         {
            if ( secondsDiff < 0 )
               secondsDiff += s_secondsInWeek;
            ti += MTimeSpan(secondsDiff);
         }

         switch ( type )
         {
         default:
            M_ENSURED_ASSERT(0);
         case OffsetWeekdayBefore:
            ti -= MTimeSpan(s_secondsInWeek);
            break;
         case OffsetWeekdayFirstAfter:
            // done already
            break;
         case OffsetWeekdaySecondAfter:
            ti += MTimeSpan(s_secondsInWeek);
            break;
         case OffsetWeekdayThirdAfter:
            ti += MTimeSpan(s_secondsInWeek * 2);
            break;
         case OffsetWeekdayFourthAfter:
            ti += MTimeSpan(s_secondsInWeek * 3);
            break;
         case OffsetWeekdayLastAfter:
            ti += MTimeSpan(s_secondsInWeek * 4);
            if ( ti.GetMonth() != GetMonth() ) // if the month changed, go back one week
               ti -= MTimeSpan(s_secondsInWeek);
            break;
         }
         M_ASSERT(ti.GetDayOfWeek() == GetDayOfWeek());
         // M_ASSERT(ti.GetMonth() == GetMonth()); <- this check works for most but not for all dates as anchor date is just any date within month
      }
      else
      {
         switch ( type )
         {
         default:
            M_ENSURED_ASSERT(0);
         case OffsetObserveOnThisAndFollowingDate:
            // Equivalent to type == OffsetNo
            break;
         case OffsetMondayIfSunday:
            if ( ti.GetDayOfWeek() == MTime::WeekdaySunday )
               ti += MTimeSpan(s_secondsInDay);
            break;
         case OffsetFridayIfSunday:
            if ( ti.GetDayOfWeek() == MTime::WeekdaySunday )
               ti -= MTimeSpan(s_secondsInDay * 2);
            break;
         case OffsetMondayIfSaturday:
            if ( ti.GetDayOfWeek() == MTime::WeekdaySaturday )
               ti += MTimeSpan(s_secondsInDay * 2);
            break;
         case OffsetFridayIfSaturday:
            if ( ti.GetDayOfWeek() == MTime::WeekdaySaturday )
               ti -= MTimeSpan(s_secondsInDay);
            break;
         case OffsetMondayIfSaturdayOrSunday:
            if ( ti.GetDayOfWeek() == MTime::WeekdaySaturday )
               ti += MTimeSpan(s_secondsInDay * 2);
            else if ( ti.GetDayOfWeek() == MTime::WeekdaySunday )
               ti += MTimeSpan(s_secondsInDay);
            break;
         case OffsetFridayIfSaturdayOrSunday:
            if ( ti.GetDayOfWeek() == MTime::WeekdaySaturday )
               ti -= MTimeSpan(s_secondsInDay);
            else if ( ti.GetDayOfWeek() == MTime::WeekdaySunday )
               ti -= MTimeSpan(s_secondsInDay * 2);
            break;
         case OffsetMondayIfSundayFridayIfSaturday:
            if ( ti.GetDayOfWeek() == MTime::WeekdaySaturday )
               ti -= MTimeSpan(s_secondsInDay);
            else if ( ti.GetDayOfWeek() == MTime::WeekdaySunday )
               ti += MTimeSpan(s_secondsInDay);
            break;
         case OffsetObserveOnFollowingDate:
            ti += MTimeSpan(s_secondsInDay);
            break;
         }
      }
   }
   return ti;
}

void MTimeRecurrentYearly::CheckIsValid() const
{
   if ( m_value.m_all != 0 ) // not is null, faster
   {
      MTimeRecurrentYearly t; // cannot use constructor or Set function due to recursion
      t.SetOffsetType(GetOffsetType());
      t.SetMonth(GetMonth());
      t.SetDayOfMonth(GetDayOfMonth());
      t.SetHours(GetHours());
      t.SetMinutes(GetMinutes());
      t.SetSeconds(GetSeconds());
      t.SetDayOfWeek(GetDayOfWeek());
      // day is checked separately

      int daysInANonLeapYear = MTime::GetNumberOfDaysInMonth(1999, GetMonth()); // 1999 was not a leap year
      MENumberOutOfRange::CheckInteger(1, daysInANonLeapYear, GetDayOfMonth(), M_OPT_STR("Day"));
   }
}

void MTimeRecurrentYearly::SetToNull() M_NO_THROW
{
   m_value.m_all = 0;
}

bool MTimeRecurrentYearly::IsNull() const M_NO_THROW
{
   return m_value.m_all == 0;
}

MVariant MTimeRecurrentYearly::NewClone() const
{
   MTimeRecurrentYearly t(*this);
   return MVariant(&t, MVariant::ACCEPT_OBJECT_EMBEDDED);
}

#if !M_NO_VARIANT
unsigned MTimeRecurrentYearly::GetEmbeddedSizeof() const
{
   M_COMPILED_ASSERT(sizeof(MTimeRecurrentYearly) <= sizeof(MVariant::ObjectByValue));
   return sizeof(MTimeRecurrentYearly);
}
#endif

#endif

