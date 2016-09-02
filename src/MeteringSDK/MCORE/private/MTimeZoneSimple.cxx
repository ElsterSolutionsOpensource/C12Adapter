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
         if ( pos[0] == '\0' ) // accept GMT as a valid input
         {
            m_standardName = name;
            return true;
         }
         else if ( (pos[0] == '-' || pos[0] == '+' || pos[0] == ' ') && isdigit(pos[1]) && isdigit(pos[2]) )
         {
            int offset = (pos[1] - '0') + (pos[2] - '0');
            if ( pos[0] == '-' )
               offset = -offset;
            if ( offset >= s_startTimeZoneHr && offset <= s_endTimeZoneHr )
            {
               m_standardName = name;
               m_standardOffset = offset;
               return true;
            }
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
      result.push_back(MGetStdString("GMT%+d:00", i));
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
