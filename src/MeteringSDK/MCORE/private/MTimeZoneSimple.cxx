// File MCORE/private/MTimeZoneSimple.cxx

   static const int s_startTimeZoneHr = -12;
   static const int s_endTimeZoneHr = 13; // correct number, 13 inclusive

   #if (M_OS & M_OS_WINDOWS) != 0
      #define tzset _tzset
      #define tzname _tzname
      #define timezone _timezone
      #define daylight _daylight
   #endif

bool MTimeZone::DoSetByName(const MStdString& name)
{
   if ( !name.empty() )
   {
      // Do not use regular expressions to enable minimalist builds with simple time zones

      if ( name == tzname[0] || name == tzname[1] )
      {
         SetFromCurrentSystem();
         return true;
      }

      const char* pos = strstr(name.c_str(), "GMT");
      if ( pos == NULL )
         pos = strstr(name.c_str(), "UTC");
      if ( pos != NULL )
      {
         pos += 3; // skip
         if ( pos[0] == '\0' ) // accept GMT/UTC as a valid input, no offset
         {
            m_standardName = name;
            return true;
         }
         else if ( pos[0] == '-' || pos[0] == '+' || pos[0] == ' ' )
         {
            int hours;
            int minutes = 0;
            if ( sscanf(pos + 1, "%d:%d", &hours, &minutes) < 1 )
                return false;
            if ( minutes < 0 || minutes >= 60 || hours < 0 ) // expect positive values. Also, upper range for hours will be checked later
                return false;
            if ( pos[0] == '-' )
               hours = -hours;
            if ( hours < s_startTimeZoneHr || hours > s_endTimeZoneHr )
                return false;
            m_standardOffset = (hours * 60 + minutes) * 60;
            m_standardName = name;
            return true;
         }
      }
   }
   return false;
}

void MTimeZone::SetFromCurrentSystem()
{
   Clear();

   tzset();

   m_standardName = tzname[0];
   m_daylightName = tzname[1];

   m_standardOffset = -timezone;
   m_daylightOffset = daylight * 3600; // daylight is in hours
   m_switchToDaylightTime.SetToNull(); // not yet supported
   m_switchToStandardTime.SetToNull(); // not yet supported
   m_displayName.clear(); // not yet supported
}

bool MTimeZone::IsDST(const MTime& time, bool isTimeUtc) const
{
   return DoStaticTestIfDST(time, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, isTimeUtc);
}

int MTimeZone::GetUtcToLocalOffset(const MTime& ti) const
{
   int offset = m_standardOffset;
   if ( IsDST(ti, true) )
      offset += m_daylightOffset;
   return offset;
}

int MTimeZone::GetLocalToUtcOffset(const MTime& ti) const
{
   int offset = -m_standardOffset;
   if ( IsDST(ti) )
      offset -= m_daylightOffset;
   return offset;
}

MStdStringVector MTimeZone::GetAllTimeZoneNames()
{
   MStdStringVector result;
   for ( int i = s_startTimeZoneHr; i <= s_endTimeZoneHr; ++i ) // inclusive range
   {
      if ( i == 0 )
         result.push_back(MStdString("GMT", 3));
      else
         result.push_back(MGetStdString("GMT%+03d:00", i));
   }
   return result;
}

MStdStringVector MTimeZone::GetAllTimeZoneDisplayNames()
{
   return GetAllTimeZoneNames();
}

MStdStringVector MTimeZone::GetAllTimeZoneLocalNames()
{
   return GetAllTimeZoneNames();
}
