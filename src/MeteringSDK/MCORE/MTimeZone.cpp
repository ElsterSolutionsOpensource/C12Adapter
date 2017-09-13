// File MCORE/MTimeZone.cpp

#include "MCOREExtern.h"
#include "MTimeZone.h"
#include "MUtilities.h"
#include "MException.h"
#include "MTimeRecurrentYearly.h"
#include "MRegistry.h"
#include "MCriticalSection.h"
#include "MStreamFile.h"

// #define M__LOG_TIMEZONES 1

#if !M_NO_TIME

#include "private/MTimeZoneMapping.cxx"
#if defined(M__TIMEZONE_USE_WINDOWS_REGISTRY) && M__TIMEZONE_USE_WINDOWS_REGISTRY == 1
    #include "private/MTimeZoneWindows.cxx"
#elif defined(M__TIMEZONE_USE_ANDROID_IMPLEMENTATION) && M__TIMEZONE_USE_ANDROID_IMPLEMENTATION != 0
    #include "private/MTimeZoneAndroid.cxx"
#elif defined(M__TIMEZONE_USE_TZ_IMPLEMENTATION) && M__TIMEZONE_USE_TZ_IMPLEMENTATION != 0
    #include "private/MTimeZoneTz.cxx"
#elif defined(M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION) && M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION != 0
    #include "private/MTimeZoneSimple.cxx"
#else
   #error "Inconsistent definition of timezone implementation macros"
#endif

   #if !M_NO_REFLECTION

      static MObject* DoNew0()
      {
         return M_NEW MTimeZone();
      }

      static MObject* DoNew1(const MVariant& p1)
      {
         if ( p1.IsObject() )
            return M_NEW MTimeZone(*M_DYNAMIC_CONST_CAST_WITH_THROW(MTimeZone, p1.AsObject()));
         else if ( p1.IsNumeric() )
            return M_NEW MTimeZone(p1.AsInt());
         else
            return M_NEW MTimeZone(p1.AsString());
      }

      static MObject* DoNew2(int standardOffset, const MStdString& name)
      {
         return M_NEW MTimeZone(standardOffset, name);
      }

      static MObject* DoNew4(int standardOffset, int daylightOffset, const MTimeRecurrentYearly& switchToDaylightTime, const MTimeRecurrentYearly& switchToStandardTime)
      {
         return M_NEW MTimeZone(standardOffset, daylightOffset, switchToDaylightTime, switchToStandardTime);
      }

      static MObject* DoNew6(int standardOffset, int daylightOffset, const MTimeRecurrentYearly& switchToDaylightTime, const MTimeRecurrentYearly& switchToStandardTime, const MStdString& standardName, const MStdString& daylightName)
      {
         return M_NEW MTimeZone(standardOffset, daylightOffset, switchToDaylightTime, switchToStandardTime, standardName, daylightName);
      }

   #endif

M_START_PROPERTIES(TimeZone)
   M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED  (TimeZone, UtcTime,                 ST_MObjectByValue_S)
   M_OBJECT_PROPERTY_READONLY_OBJECT_EMBEDDED (TimeZone, LocalTime,               ST_MObjectByValue_X)
   M_OBJECT_PROPERTY_READONLY_OBJECT_EMBEDDED (TimeZone, StandardTime,            ST_MObjectByValue_X)
   M_CLASS_PROPERTY_READONLY_OBJECT           (TimeZone, Current)
   M_OBJECT_PROPERTY_STRING                   (TimeZone, StandardName,            ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_STRING                   (TimeZone, DaylightName,            ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT      (TimeZone, HasSwitchTimes)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT      (TimeZone, SupportsDST)
   M_OBJECT_PROPERTY_INT                      (TimeZone, StandardOffset)
   M_OBJECT_PROPERTY_INT                      (TimeZone, DaylightOffset)
   M_OBJECT_PROPERTY_READONLY_OBJECT          (TimeZone, SwitchToStandardTime)
   M_OBJECT_PROPERTY_READONLY_OBJECT          (TimeZone, SwitchToDaylightTime)
   M_OBJECT_PROPERTY_STRING                   (TimeZone, DisplayName,             ST_constMStdStringA_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_BOOL_EXACT               (TimeZone, InitializedFromDatabase)
   M_CLASS_PROPERTY_READONLY_STRING_COLLECTION(TimeZone, AllTimeZoneNames,        ST_MStdStringVector_S)
   M_CLASS_PROPERTY_READONLY_STRING_COLLECTION(TimeZone, AllTimeZoneDisplayNames, ST_MStdStringVector_S)
   M_CLASS_PROPERTY_READONLY_STRING_COLLECTION(TimeZone, AllTimeZoneLocalNames,   ST_MStdStringVector_S)
M_START_METHODS(TimeZone)
   M_OBJECT_SERVICE                           (TimeZone, GetLocalToUtcOffset,     ST_int_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, GetUtcToLocalOffset,     ST_int_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, GetStandardOffsetForTime,ST_int_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, GetDaylightOffsetForYear,ST_int_X_int)
   M_OBJECT_SERVICE_OVERLOADED                (TimeZone, GetNextSwitchTime, GetNextSwitchTime,    2, ST_MObjectByValue_X_MObjectP_bool)
   M_OBJECT_SERVICE_OVERLOADED                (TimeZone, GetNextSwitchTime, DoGetNextSwitchTime1, 1, ST_MObjectByValue_X_MObjectP)  // SWIG_HIDE
   M_OBJECT_SERVICE_OVERLOADED                (TimeZone, GetSwitchTimeOffsetChange, GetSwitchTimeOffsetChange,    2, ST_int_X_MObjectP_bool)
   M_OBJECT_SERVICE_OVERLOADED                (TimeZone, GetSwitchTimeOffsetChange, DoGetSwitchTimeOffsetChange1, 1, ST_int_X_MObjectP) // SWIG_HIDE
   M_OBJECT_SERVICE_OVERLOADED                (TimeZone, IsDST, IsDST,        2,  ST_bool_X_MObjectP_bool)
   M_OBJECT_SERVICE_OVERLOADED                (TimeZone, IsDST, DoIsDST,      1,  ST_bool_X_MObjectP)      // SWIG_HIDE
   M_OBJECT_SERVICE                           (TimeZone, LocalToUtc,              ST_MObjectByValue_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, UtcToLocal,              ST_MObjectByValue_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, StandardToUtc,           ST_MObjectByValue_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, UtcToStandard,           ST_MObjectByValue_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, StandardToLocal,         ST_MObjectByValue_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, LocalToStandard,         ST_MObjectByValue_X_MObjectP)
   M_OBJECT_SERVICE                           (TimeZone, SetFromCurrentSystem,    ST_X)
   M_OBJECT_SERVICE                           (TimeZone, Clear,                   ST_X)
   M_CLASS_SERVICE                            (TimeZone, StandardNameWindowsToIana, ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                            (TimeZone, StandardNameIanaToWindows, ST_MStdString_S_constMStdStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED          (TimeZone, New, DoNew0,         0,  ST_MObjectP_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED          (TimeZone, New, DoNew1,         1,  ST_MObjectP_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED          (TimeZone, New, DoNew2,         2,  ST_MObjectP_S_int_constMStdStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED          (TimeZone, New, DoNew4,         4,  ST_MObjectP_S_int_int_MObjectP_MObjectP)
   M_CLASS_FRIEND_SERVICE_OVERLOADED          (TimeZone, New, DoNew6,         6,  ST_MObjectP_S_int_int_MObjectP_MObjectP_constMStdStringA_constMStdStringA)
   M_OBJECT_SERVICE                           (TimeZone, NewClone,                ST_MObjectP_X)
M_END_CLASS(TimeZone, Object)

const char    MTimeZone::s_timezoneNameSeparator []  = " / ";
const unsigned MTimeZone::s_timezoneNameSeparatorSize = M_NUMBER_OF_ARRAY_ELEMENTS(s_timezoneNameSeparator) - 1;

MTimeZone::MTimeZone()
:
   m_dynamic(),
   m_standardName(),
   m_daylightName(),
   m_displayName(),
   m_standardOffset(0),
   m_daylightOffset(0),
   m_switchToDaylightTime(),
   m_switchToStandardTime()
{
   SetFromCurrentSystem();
}

MTimeZone::MTimeZone(const MStdString& name)
:
   m_dynamic(),
   m_standardName(),
   m_daylightName(),
   m_displayName(),
   m_standardOffset(0),
   m_daylightOffset(0),
   m_switchToDaylightTime(),
   m_switchToStandardTime()
{
   SetByName(name);
}

MTimeZone::MTimeZone(int standardOffset)
:
   m_dynamic(),
   m_standardName(),
   m_daylightName(),
   m_displayName(),
   m_daylightOffset(0),
   m_switchToDaylightTime(),
   m_switchToStandardTime()
{
   SetStandardOffset(standardOffset);
}

MTimeZone::MTimeZone(int standardOffset, const MStdString& standardName)
:
   m_dynamic(),
   m_standardName(standardName),
   m_daylightName(),
   m_displayName(),
   m_daylightOffset(0),
   m_switchToDaylightTime(),
   m_switchToStandardTime()
{
   SetStandardOffset(standardOffset);
}

MTimeZone::MTimeZone(int                         standardOffset,
                     int                         daylightOffset,
                     const MTimeRecurrentYearly& switchToDaylightTime,
                     const MTimeRecurrentYearly& switchToStandardTime)
:
   m_dynamic(),
   m_standardName(),
   m_daylightName(),
   m_displayName(),
   m_switchToDaylightTime(switchToDaylightTime),
   m_switchToStandardTime(switchToStandardTime)
{
   SetStandardOffset(standardOffset);
   SetDaylightOffset(daylightOffset);
}

MTimeZone::MTimeZone(int                         standardOffset,
                     int                         daylightOffset,
                     const MTimeRecurrentYearly& switchToDaylightTime,
                     const MTimeRecurrentYearly& switchToStandardTime,
                     const MStdString&           standardName,
                     const MStdString&           daylightName)
:
   m_dynamic(),
   m_standardName(standardName),
   m_daylightName(daylightName),
   m_displayName(),
   m_switchToDaylightTime(switchToDaylightTime),
   m_switchToStandardTime(switchToStandardTime)
{
   SetStandardOffset(standardOffset);
   SetDaylightOffset(daylightOffset);
}

MTimeZone::MTimeZone(const MTimeZone& other)
{
   operator=(other);
}

MTimeZone::~MTimeZone()
{
}

MTimeZone& MTimeZone::operator=(const MTimeZone& other)
{
   if ( this != &other )
   {
      m_dynamic               = other.m_dynamic;
      m_standardName          = other.m_standardName;
      m_daylightName          = other.m_daylightName;
      m_displayName           = other.m_displayName;
      m_standardOffset        = other.m_standardOffset;
      m_daylightOffset        = other.m_daylightOffset;
      m_switchToDaylightTime  = other.m_switchToDaylightTime;
      m_switchToStandardTime  = other.m_switchToStandardTime;
   }
   return *this;
}

bool MTimeZone::operator==(const MTimeZone& other) const
{
   // easy and fast fields first
   //
   if ( m_standardName   != other.m_standardName   ||
        m_daylightName   != other.m_daylightName   ||
        m_displayName    != other.m_displayName    ||
        m_standardOffset != other.m_standardOffset ||
        m_daylightOffset != other.m_daylightOffset )
   {
      return false;
   }

   if ( m_dynamic.IsPresent() )
      return m_dynamic == other.m_dynamic;
   else
      return GetSwitchToDaylightTimeConst() == other.GetSwitchToDaylightTimeConst() &&
             GetSwitchToStandardTimeConst() == other.GetSwitchToStandardTimeConst();
}

MTimeZone* MTimeZone::GetCurrent()
{
   // Use Meyer's singleton to handle the current timezone.
   // See method's description for details of this implementation.
   //
   const unsigned          s_timezoneUpdatePeriod = 10000; // update every 10 seconds
   static MCriticalSection s_currentTimeZoneCriticalSection;
   static MTimeZone        s_currentTimeZone; // Global current timezone
   static unsigned         s_currentTimeZoneUpdateTickCount = 0; // basically, any value will work, not just zero

   unsigned currentTickCount = MUtilities::GetTickCount();
   if ( s_currentTimeZoneUpdateTickCount < currentTickCount ) // case of overflow will work fine
   {
      s_currentTimeZoneUpdateTickCount = currentTickCount + s_timezoneUpdatePeriod;
      MCriticalSection::Locker locker(s_currentTimeZoneCriticalSection);
      MTimeZone localTimezone;
      localTimezone.SetFromCurrentSystem();
      if ( s_currentTimeZone != localTimezone )
         s_currentTimeZone = localTimezone;
   }
   return &s_currentTimeZone;
}

void MTimeZone::SetByName(const MStdString& name)
{
   Clear();
   if ( !DoSetByName(name) )
   {
      MException::Throw(M_ERR_TIME_ZONE_S1_NOT_FOUND, M_I("Time zone '%s' is not found"), name.c_str());
      M_ASSERT(0);
   }
}

void MTimeZone::SetStandardName(const MStdString& name)
{
   m_standardName = name;
   SetInitializedFromDatabase(false);
}

void MTimeZone::SetDaylightName(const MStdString& name)
{
   m_daylightName = name;
   SetInitializedFromDatabase(false);
}

void MTimeZone::SetDisplayName(const MStdString& name)
{
   m_displayName = name;
   SetInitializedFromDatabase(false);
}

void MTimeZone::SetStandardOffset(int offset)
{
   if ( offset < (-13 * 60 * 60) || offset > (13 * 60 * 60) || offset % (5 * 60) != 0 )  // +-13 hours, divisible by 5 minutes
   {
      MException::Throw(M_ERR_BAD_TIME_VALUE, M_I("Standard Offset is outside range of -13 .. 13 hours, or not divisible by 5 minutes"));
      M_ENSURED_ASSERT(0);
   }
   m_standardOffset = offset;
   SetInitializedFromDatabase(false);
}

void MTimeZone::SetDaylightOffset(int offset)
{
   if ( offset < (-3 * 60 * 60) || offset > (3 * 60 * 60) || offset % (5 * 60) != 0 )  // +-3 hours, divisible by 5 minutes
   {
      MException::Throw(M_ERR_BAD_TIME_VALUE, M_I("Daylight Offset is outside range of -3 .. 3 hours, or not divisible by 5 minutes"));
      M_ENSURED_ASSERT(0);
   }
   m_daylightOffset = offset;
   SetInitializedFromDatabase(false);
}

int MTimeZone::GetStandardOffsetForTime(const MTime& utcTime) const
{
   MTime t = utcTime;
   while ( IsDST(t, true) )
      t += MTimeSpan(60 * 60 * 24 * 90); // add 90 days
   return GetUtcToLocalOffset(t);
}

int MTimeZone::GetDaylightOffsetForYear(int year) const
{
   MTime time(year, MTime::MonthJanuary, 1);
   int offset1 = GetUtcToLocalOffset(time);
   bool isDst1 = IsDST(time, true);

   time.Set(year, MTime::MonthJuly, 1);
   int offset2 = GetUtcToLocalOffset(time);
   bool isDst2 = IsDST(time, true);

   if ( offset1 == offset2 ) // if going to DST in spring did not shift the clock
   {                         // there could be the case when the DST change starts to happen the second half of the year
      time.Set(year, MTime::MonthDecember, 25);
      offset1 = GetUtcToLocalOffset(time);
      if ( !isDst1 )         // last chance
         isDst1 = IsDST(time, true); // case when in the southern hemisphere the DST is freshly introduced in the given year
   }

   if ( !isDst1 && !isDst2 ) // if there was no DST change in the year
      return 0;              //   return zero whether or not there was a shift

   // DST offset is always positive
   if ( offset1 < offset2 )
      return offset2 - offset1;
   else
      return offset1 - offset2;
}

MTimeRecurrentYearly& MTimeZone::GetSwitchToDaylightTime()
{
   return m_switchToDaylightTime;
}

MTimeRecurrentYearly& MTimeZone::GetSwitchToStandardTime()
{
   return m_switchToStandardTime;
}

void MTimeZone::DoComputeRecurringSwitchTimes()
{
   M_ASSERT(m_switchToDaylightTime.IsNull());
   M_ASSERT(m_switchToStandardTime.IsNull());

   // Check if the switch times in the coming three years are representable by MTimeRecurrentYearly.
   // This is how we determine if there are any DST switch times that make sense.
   //
   // The calculation is done using local time, but we have to use
   // MTime::GetCurrentUtcTime() instead of MTime::GetCurrentLocalTime()
   // in order to prevent recursion. However, this is okay as the current time
   // does not have to be precise - we only need some approximate date to look up for three years in advance.

   MTime time = MTime::GetCurrentUtcTime();            // get UTC time to prevent recursion (see the comment above)
   MTime switchTime1 = GetNextSwitchTime(time, false); // but perform calculation in local times (see the comment above)
   if ( !switchTime1.IsNull() )
   {
      time = switchTime1;
      time += MTimeSpan(60 * 60 * 24 * 32); // assume that a sane DST schedule is not changing time in less than 32 days
      MTime switchTime2 = GetNextSwitchTime(time, false);
      if ( !switchTime2.IsNull() )
      {
         MTimeRecurrentYearly recurrentTime1;
         MTimeRecurrentYearly recurrentTime2;
         DoCalculateRecurrentFromTime(recurrentTime1, switchTime1);
         DoCalculateRecurrentFromTime(recurrentTime2, switchTime2);
         if ( !recurrentTime1.IsNull() && !recurrentTime2.IsNull() )
         {
            int change1 = GetSwitchTimeOffsetChange(switchTime1);
            int change2 = GetSwitchTimeOffsetChange(switchTime2);
            if ( change1 < 0 && change2 > 0 ) // Northern hemisphere
            {
               m_switchToDaylightTime = recurrentTime1;
               m_switchToStandardTime = recurrentTime2;
            }
            else if ( change2 < 0 && change1 > 0 ) // Southern hemisphere
            {
               m_switchToDaylightTime = recurrentTime2;
               m_switchToStandardTime = recurrentTime1;
            }
            else
            {
               M_ASSERT(0); // while this is theoretically possible this is bizarre, signal on debug
            }
         }
      }
   }
}

void MTimeZone::DoCalculateRecurrentFromTime(MTimeRecurrentYearly& result, const MTime& time)
{
   M_ASSERT(result.IsNull());
   MTime switchTime;
   MTimeSpan oneMon(60 * 60 * 24 * 30);
   MTimeSpan lessThanAYear(60 * 60 * 24 * 365);
   struct tm t;
   bool found = false;
#if defined(M__LOG_TIMEZONES) && M__LOG_TIMEZONES != 0
   MStreamFile::GetStdOut()->WriteLine("TIMEZONE: " + m_standardName);
#endif
   time.GetTM(&t);
   result.SetOnWeekday(MTimeRecurrentYearly::OffsetWeekdayFirstAfter,
                       static_cast<MTime::MonthType>(t.tm_mon + 1), 1,  // tm_mon is zero based
                       t.tm_hour, t.tm_min, static_cast<MTime::DayOfWeekType>(t.tm_wday));
   result.SetSeconds(t.tm_sec);

   // Perform brute force offset guessing
   for ( int offset = 6;        // 6 = MTimeRecurrentYearly::OffsetWeekdayLastAfter
         !found && offset >= 2; // 2 = MTimeRecurrentYearly::OffsetWeekdayFirstAfter
         --offset )
   {
#if defined(M__LOG_TIMEZONES) && M__LOG_TIMEZONES != 0
      MStreamFile::GetStdOut()->WriteLine("   trying: " + MToStdString(offset));
#endif
      result.SetOffsetType(static_cast<MTimeRecurrentYearly::OffsetType>(offset));
      switchTime = time;
      int year = t.tm_year + 1900;
      int yearEnd = year + 6; // weeks cycle
      for ( ; year <= yearEnd; ++year )
      {
         switchTime.SetYear(year);
         switchTime = result.GetPertinent(switchTime);
         const MTime& realSwitchTime = GetNextSwitchTime(switchTime - oneMon);
         if ( realSwitchTime != switchTime )
         {
#if defined(M__LOG_TIMEZONES) && M__LOG_TIMEZONES != 0
            MStreamFile::GetStdOut()->WriteLine("      bad match: our_" + switchTime.AsString() + " <> real_" + realSwitchTime.AsString());
#endif
            found = false;
            break;
         }
         found = true;
#if defined(M__LOG_TIMEZONES) && M__LOG_TIMEZONES != 0
         MStreamFile::GetStdOut()->WriteLine("          match: our_" + switchTime.AsString() + " = real_" + realSwitchTime.AsString());
#endif
         M_ASSERT(GetSwitchTimeOffsetChange(switchTime) != 0);
      }
   }

   if ( !found && !(t.tm_mon == 0 && t.tm_mday == 1 && t.tm_hour == 0 && t.tm_min == 0) ) // try fixed date if this is not Jan 1, 00:00
   {
      result.SetOnDay(MTimeRecurrentYearly::OffsetNo, static_cast<MTime::MonthType>(t.tm_mon + 1), t.tm_mday,  t.tm_hour, t.tm_min);
      result.SetSeconds(t.tm_sec);
      switchTime = time;
      int year = t.tm_year;
      int yearEnd = year + 6; // weeks cycle
      for ( ; year <= yearEnd; ++year, switchTime += lessThanAYear )
      {
         switchTime = result.GetPertinent(switchTime);
         const MTime& realSwitchTime = GetNextSwitchTime(switchTime - oneMon);
         if ( realSwitchTime != switchTime )
         {
#if defined(M__LOG_TIMEZONES) && M__LOG_TIMEZONES != 0
            MStreamFile::GetStdOut()->WriteLine("      bad match: our_" + switchTime.AsString() + " <> real_" + realSwitchTime.AsString());
#endif
            found = false;
            break;
         }
         found = true;
#if defined(M__LOG_TIMEZONES) && M__LOG_TIMEZONES != 0
         MStreamFile::GetStdOut()->WriteLine("          match: our_" + switchTime.AsString() + " = real_" + realSwitchTime.AsString());
#endif
         M_ASSERT(GetSwitchTimeOffsetChange(switchTime) != 0);
      }

   }

   if ( !found ) // everything failed
      result.SetToNull();
}

MTime MTimeZone::GetUtcTime()
{
   return MTime(MTime::GetUtcSecondsSince1970());
}

MTime MTimeZone::GetLocalTime() const
{
   MTime utc = GetUtcTime();
   return UtcToLocal(utc);
}

MTime MTimeZone::GetStandardTime() const
{
   MTime utc = GetUtcTime();
   return UtcToStandard(utc);
}

MTime MTimeZone::UtcToLocal(const MTime& t) const
{
   return t + MTimeSpan(GetUtcToLocalOffset(t));
}

MTime MTimeZone::LocalToUtc(const MTime& t) const
{
   return t + MTimeSpan(GetLocalToUtcOffset(t));
}

MTime MTimeZone::UtcToStandard(const MTime& t) const
{
   MTime result;
   result = t + MTimeSpan(GetStandardOffsetForTime(t));
   return result;
}

MTime MTimeZone::StandardToUtc(const MTime& t) const
{
   MTime result;
   result = t - MTimeSpan(GetStandardOffsetForTime(t));
   return result;
}

MTime MTimeZone::StandardToLocal(const MTime& t) const
{
   return UtcToLocal(StandardToUtc(t));
}

MTime MTimeZone::LocalToStandard(const MTime& t) const
{
   return UtcToStandard(LocalToUtc(t));
}

void MTimeZone::Clear()
{
   m_dynamic.Reset();
   m_standardName.clear(),
   m_daylightName.clear(),
   m_standardOffset = 0;
   m_daylightOffset = 0;
   m_switchToDaylightTime.SetToNull();
   m_switchToStandardTime.SetToNull();
   SetInitializedFromDatabase(false);
}

bool MTimeZone::DoStaticTestIfDST(const MTime& ti, const MTimeRecurrentYearly& switchToDaylight, const MTimeRecurrentYearly& switchToStandard, int standardOffset, int daylightOffset, bool isUtc)
{
   if ( daylightOffset == 0 || switchToDaylight.IsNull() || switchToStandard.IsNull() )
      return false;

   int switchToDaylightMonth = switchToDaylight.GetMonth();
   int switchToStandardMonth = switchToStandard.GetMonth();
   bool isNorthernHemisphere = (switchToDaylightMonth > 1  && switchToDaylightMonth <= 6) ||   // there are crazy timezones, like W.Australia, that switch times in December
                              (switchToStandardMonth >= 6 && switchToStandardMonth <= 12);    // but it seems, no one switches in January. This explains why there is only one '>'
                                                                                              // as 1 means do no switch on Windows
   const int year = ti.GetYear();
   const MTime yearTimeStart(year, 1, 1);
   const MTime yearTimeEnd(year + 1, 1, 1); // any date in the future after one year, actually
   MTime switchToDaylightTime;
   MTime switchToStandardTime;
   if ( switchToDaylightMonth == 1 && (switchToDaylight.GetOffsetType() == MTimeRecurrentYearly::OffsetNo || 
                                       switchToDaylight.GetOffsetType() == MTimeRecurrentYearly::OffsetWeekdayFirstAfter) ) // means no switch this year
   {
      switchToDaylightTime = isNorthernHemisphere ? yearTimeStart : yearTimeEnd;
   }
   else
   {
      switchToDaylightTime = switchToDaylight.GetPertinent(yearTimeStart);
      if ( isUtc ) // adjust switch times to UTF times
         switchToDaylightTime -= MTimeSpan(standardOffset);
   }
   if ( switchToStandardMonth == 1 && (switchToStandard.GetOffsetType() == MTimeRecurrentYearly::OffsetNo || 
                                       switchToStandard.GetOffsetType() == MTimeRecurrentYearly::OffsetWeekdayFirstAfter) ) // means no switch this year
   {
      switchToStandardTime = isNorthernHemisphere ? yearTimeEnd : yearTimeStart;
   }
   else
   {
      switchToStandardTime = switchToStandard.GetPertinent(yearTimeStart);
      if ( isUtc ) // adjust switch times to UTF times
         switchToStandardTime -= MTimeSpan(standardOffset + daylightOffset);
   }

   bool result;
   if ( isNorthernHemisphere )
      result = ti >= switchToDaylightTime && ti < switchToStandardTime;
   else
      result = ti < switchToStandardTime || ti >= switchToDaylightTime;

#if defined(M__LOG_TIMEZONES) && M__LOG_TIMEZONES != 0
   MStreamFile::GetStdOut()->WriteLine(MFormat("%s: dlt=%s std=%s stdO=%d dltO=%d u=%d r=%d\n",
          ti.AsString().c_str(), switchToDaylightTime.AsString().c_str(), switchToStandardTime.AsString().c_str(),
          standardOffset / 3600, daylightOffset / 3600, (int)isUtc, (int)result));
#endif

   return result;
}

   const int s_switchTimeSearchPeriodInDays = 32; // does not have to be the exact number of days/seconds
   const int s_periodsToLookAhead = 36; // with 32 days for a period this is approximately 3.14 years

   const int s_switchTimeSearchPeriodInSeconds = s_switchTimeSearchPeriodInDays * 24 * 60 * 60;
   const int s_switchTimeSearchEndInSeconds = s_switchTimeSearchPeriodInSeconds * s_periodsToLookAhead;

   typedef int (MTimeZone::*OffsetFunctionType)(const MTime& time) const;

   inline bool DoFindSwitchTime(const MTimeZone* zone, MTime& result, MTime& from, MTime& to, OffsetFunctionType offsetFunc)
   {
      int offsetFrom = (zone->*offsetFunc)(from);
      int offsetTo = (zone->*offsetFunc)(to);
      if ( offsetFrom != offsetTo )
      {
         for ( ;; )
         {
            int diffSeconds = (to - from).ToSeconds();
            M_ENSURED_ASSERT(diffSeconds > 0);
            if ( diffSeconds == 1 ) // found it, have to guess what is the exact switch time
            {
               int fromSeconds = from.GetSeconds();
               int toSeconds = to.GetSeconds();
               if ( fromSeconds == 0 )
                  result = from;
               else if ( toSeconds == 0 )
                  result = to;
               else if ( fromSeconds == 59 || fromSeconds == 1 )
                  result = from;
               else
               {
                  M_ASSERT(toSeconds == 59 || toSeconds == 1); // some weird timezones can assert here, comment out if so
                  result = to;
               }
               return true;
            }
            MTime pivot = from + MTimeSpan(diffSeconds / 2);
            int offsetPivot = (zone->*offsetFunc)(pivot);
            if ( offsetPivot != offsetTo )
            {
               M_ASSERT(offsetPivot == offsetFrom);
               from = pivot;
               offsetFrom = offsetPivot;
            }
            else
            {
               M_ASSERT(offsetPivot == offsetTo);
               M_ASSERT(offsetPivot != offsetFrom);
               to = pivot;
               offsetTo = offsetPivot;
            }
         }
      }
      return false;
   }

MTime MTimeZone::GetNextSwitchTime(const MTime& anchorTime, bool isTimeUtc) const
{
   MTime result;

   // itLast value is approximate,
   // about three years ahead of anchor or three years ahead of current date
   // whichever is later
   MTime itLast = MTime::GetCurrentUtcTime();
   if ( itLast < anchorTime )
      itLast = anchorTime;
   itLast += MTimeSpan(s_switchTimeSearchEndInSeconds);

   MTime itNext;
   MTimeSpan addendum(s_switchTimeSearchPeriodInSeconds);

   OffsetFunctionType offsetFunc = isTimeUtc
                                 ? &MTimeZone::GetUtcToLocalOffset
                                 : &MTimeZone::GetLocalToUtcOffset;
   for ( MTime it = anchorTime; it <= itLast; it = itNext )
   {
      itNext = it + addendum;
      if ( DoFindSwitchTime(this, result, it, itNext, offsetFunc) )
         break; // found
   }

#if M_DEBUG
   if ( !result.IsNull() )
   {
      int offsetPrev = (this->*offsetFunc)(result - MTimeSpan(1));
      int offsetNext = (this->*offsetFunc)(result + MTimeSpan(1));
      M_ASSERT(offsetPrev != offsetNext); // there was indeed a switch
      M_ASSERT(GetSwitchTimeOffsetChange(result, isTimeUtc) != 0); // this is the same but also checking a func
   }
#endif

   return result;
}

int MTimeZone::GetSwitchTimeOffsetChange(const MTime& time, bool isTimeUtc) const
{
   MTimeSpan oneSecond(1);
   OffsetFunctionType offsetFunc = isTimeUtc
                                 ? &MTimeZone::GetUtcToLocalOffset
                                 : &MTimeZone::GetLocalToUtcOffset;
   // declare a named variable for easy debugging
   int result = (this->*offsetFunc)(time + oneSecond) - (this->*offsetFunc)(time - oneSecond);
   return result;
}

bool MTimeZone::SupportsDST() const
{
   if ( m_dynamic.GetInitialized() )
   {  // Currently have a non-optimal, but uniform method that works the same on all platforms
      MTimeSpan delta(60 * 60 * 24 * 32); // minimalist duration of DST of 32 days
      MTime next = MTime::GetCurrentUtcTime();
      for ( int i = 24; i >= 0; --i, next += delta ) // somewhat more than two years lookup should be more than enough
      {
         if ( IsDST(next, true) )
            return true;
      }
      return false;
   }
   return GetDaylightOffset() != 0 && !m_switchToDaylightTime.IsNull() && !m_switchToStandardTime.IsNull();
}

bool MTimeZone::HasSwitchTimes() const
{
   if ( m_dynamic.GetInitialized() )
   {  // Currently have a non-optimal, but uniform method that works the same on all platforms
      const MTime& next = GetNextSwitchTime(MTime(2000, 1, 1), true);
      return !next.IsNull();
   }
   return GetDaylightOffset() != 0 && !m_switchToDaylightTime.IsNull() && !m_switchToStandardTime.IsNull();
}

MStdString MTimeZone::StandardNameWindowsToIana(const MStdString& windowsName)
{
   MStdString result;
   for ( size_t i = 0; i < M_NUMBER_OF_ARRAY_ELEMENTS(s_windowsToIana); ++i )
   {
      const MPrivateWindowsToIanaType& m = s_windowsToIana[i];
      if ( windowsName == m.m_windows )
      {
         result = m.m_iana;
         break;
      }
   }
   return result;
}

MStdString MTimeZone::StandardNameIanaToWindows(const MStdString& ianaName)
{
   MStdString result;
   for ( size_t i = 0; i < M_NUMBER_OF_ARRAY_ELEMENTS(s_windowsToIana); ++i )
   {
      const MPrivateWindowsToIanaType& m = s_windowsToIana[i];
      if ( ianaName == m.m_iana )
      {
         result = m.m_windows;
         break;
      }
   }
   return result;
}

#if !M_NO_REFLECTION

bool MTimeZone::DoIsDST(const MTime& time) const
{
   return IsDST(time); // use default parameter
}

MTime MTimeZone::DoGetNextSwitchTime1(const MTime& anchorTime) const
{
   return GetNextSwitchTime(anchorTime); // use default parameter
}

int MTimeZone::DoGetSwitchTimeOffsetChange1(const MTime& time) const
{
   return GetSwitchTimeOffsetChange(time);
}

#endif // !M_NO_REFLECTION
#endif
