// File MCORE/private/MTimeZoneWindows.cxx

   #if (M_OS & M_OS_WIN32_CE) != 0
      static const char s_timeZoneListRegistryPlacement[] = "SOFTWARE\\Time Zones";
   #else
      static const char s_timeZoneListRegistryPlacement[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones";
   #endif

void MTimeZone::DoSetByRegistry(MRegistry& reg)
{
   // MSDN KB115231 used below

   // Private definition of TZI contents, MSDN KB115231 used
   //
   struct TZInfo
   {
      LONG       Bias;
      LONG       StandardBias;
      LONG       DaylightBias;
      SYSTEMTIME StandardDate;
      SYSTEMTIME DaylightDate;

      bool operator==(const TZInfo& other) const
      {
         return memcmp(this, &other, sizeof(TZInfo)) == 0;
      }
      bool operator!=(const TZInfo& other) const
      {
         return !operator==(other);
      }
   };

   MByteString binaryData = reg.GetExistingBinary(MStdString("TZI", 3)); // will throw if not present
   if ( binaryData.size() < sizeof(TZInfo) ) // for some reason, XP has some extra data than described in the KB115231
   {
      MException::Throw(M_CODE_STR(M_ERR_TIME_FUNCTION_ERROR, "Time function error")); // assume this is still a rare misconfiguration possibility
      M_ENSURED_ASSERT(0);
   }

   YearlyTimeZoneInfo zone;

   m_standardName = reg.GetString("Std", MVariant::s_emptyString);
   m_daylightName = reg.GetString("Dlt", MVariant::s_emptyString);
   m_displayName = reg.GetString("Display", MVariant::s_emptyString);

   const MTime& now = MTime::GetCurrentUtcTime();

   TZInfo tzInfo;
   if ( reg.IsSubkeyPresent("Dynamic DST") )
   {
      MRegistry dynamicDst;
      dynamicDst.OpenSubkey(reg, "Dynamic DST");
      try
      {
         int lastEntry = dynamicDst.GetExistingInteger("LastEntry");
         m_dynamic.m_firstYear = dynamicDst.GetExistingInteger("FirstEntry");
         for ( int year = m_dynamic.m_firstYear; year <= lastEntry; ++year )
         {
           const MByteString& dynamicBinaryData = dynamicDst.GetExistingBinary(MToStdString(year)); // will throw if not present
            if ( dynamicBinaryData.size() < sizeof(TZInfo) ) // for some reason, XP has some extra data than described in the KB115231
            {
               MException::Throw(M_CODE_STR(M_ERR_TIME_FUNCTION_ERROR, "Time function error")); // assume this is still a rare misconfiguration possibility
               M_ENSURED_ASSERT(0);
            }
            memcpy(&tzInfo, dynamicBinaryData.data(), sizeof(tzInfo));

#if defined(M__LOG_TIMEZONES) && M__LOG_TIMEZONES != 0
            MStreamFile::GetStdOut()->WriteLine(MFormat("  %d  Bias = %2d StdBias = %2d, DltBias = %2d DltDate = %04d-%02d/%02d(%02d) %02d:%02d StdDate = %04d-%02d/%02d(%02d) %02d:%02d\n",
                   year, tzInfo.Bias / 60,  tzInfo.StandardBias / 60, tzInfo.DaylightBias / 60,
                   tzInfo.DaylightDate.wYear, tzInfo.DaylightDate.wMonth, tzInfo.DaylightDate.wDay, tzInfo.DaylightDate.wDayOfWeek, tzInfo.DaylightDate.wHour, tzInfo.DaylightDate.wMinute,
                   tzInfo.StandardDate.wYear, tzInfo.StandardDate.wMonth, tzInfo.StandardDate.wDay, tzInfo.StandardDate.wDayOfWeek, tzInfo.StandardDate.wHour, tzInfo.StandardDate.wMinute));
#endif

            // These had been true for every registry entry so far
            M_ASSERT(tzInfo.StandardBias == 0);
            M_ASSERT(tzInfo.StandardDate.wYear == 0);
            M_ASSERT(tzInfo.DaylightDate.wYear == 0);

            zone.m_standardOffset = -(tzInfo.Bias + tzInfo.StandardBias) * 60;
            zone.m_daylightOffset = -(tzInfo.DaylightBias - tzInfo.StandardBias) * 60;
            zone.m_switchToStandardTime.SetFromTimeZoneTime(tzInfo.StandardDate);
            zone.m_switchToDaylightTime.SetFromTimeZoneTime(tzInfo.DaylightDate);
            m_dynamic.m_tzi.push_back(zone);
         }
         if ( lastEntry < now.GetYear() ) // append only if it is smaller than last entry
         {
            memcpy(&tzInfo, binaryData.data(), sizeof(tzInfo));
            zone.m_standardOffset = -(tzInfo.Bias + tzInfo.StandardBias) * 60;
            zone.m_switchToStandardTime.SetFromTimeZoneTime(tzInfo.StandardDate);
            zone.m_switchToDaylightTime.SetFromTimeZoneTime(tzInfo.DaylightDate);
            zone.m_daylightOffset = -(tzInfo.DaylightBias - tzInfo.StandardBias) * 60;
            m_dynamic.m_tzi.push_back(zone);
         }
      }
      catch ( ... )
      {
         m_dynamic.Reset(); // incomplete dynamic DST gets cleared
         throw;
      }
      m_dynamic.SetInitialized(true); // do this prior to the following calls
      DoComputeRecurringSwitchTimes();
      m_standardOffset = GetStandardOffsetForTime(now);
      m_daylightOffset = GetDaylightOffsetForYear(now.GetYear());
   }
   else
   {
      memcpy(&tzInfo, binaryData.data(), sizeof(tzInfo));
      M_ASSERT(tzInfo.StandardDate.wYear == 0); // never seen a fixed year in the registry
      M_ASSERT(tzInfo.DaylightDate.wYear == 0); // never seen a fixed year in the registry

      m_switchToDaylightTime.SetFromTimeZoneTime(tzInfo.DaylightDate);
      m_switchToStandardTime.SetFromTimeZoneTime(tzInfo.StandardDate);
      M_ASSERT(m_switchToDaylightTime.IsNull() == m_switchToStandardTime.IsNull()); // has to be the case for all such timezones

      m_standardOffset = -(tzInfo.Bias + tzInfo.StandardBias) * 60; // this is where and how StandardBias participates in expression
      if ( m_switchToStandardTime.IsNull() && m_switchToDaylightTime.IsNull() )
         m_daylightOffset = 0;
      else
         m_daylightOffset = -(tzInfo.DaylightBias - tzInfo.StandardBias) * 60;
   }
}

   static bool DoMatchRegistry(MRegistry& tzRegistry, const MStdString& registryString, const MStdString& name, const MStdString& dispName)
   {
      const MStdString& candidate = tzRegistry.GetString(registryString, MVariant::s_emptyString);
      return !candidate.empty() && ((!name.empty() && candidate == name) || (!dispName.empty() && candidate == dispName));
   }

bool MTimeZone::DoSetByName(const MStdString& originalName)
{
   // MSDN KB115231 used below

   MStdString name;
   MStdString dispName;
   MStdString::size_type separator = originalName.find(s_timezoneNameSeparator, s_timezoneNameSeparatorSize);
   if ( separator == MStdString::npos )
      name = originalName; // dispName remains empty
   else
   {
      name.assign(originalName.c_str(), separator);
      dispName = originalName.substr(separator + s_timezoneNameSeparatorSize);
   }

   if ( name.empty() && dispName.empty() )
      return false;

   MRegistry timeZoneRegistry(MRegistry::KeyLocalMachine, s_timeZoneListRegistryPlacement, true);
   if ( timeZoneRegistry.IsOpen() )
   {
      if ( !name.empty() )
      {
         MRegistry tzReg(timeZoneRegistry, name);
         if ( tzReg.IsOpen() )
         {
            DoSetByRegistry(tzReg);
            return true;
         }
      }

      MStdString displayString("Display", 7);
      MStdString stdString("Std", 3);
      MStdString dltString("Dlt", 3);
      MStdStringVector timeZoneNames(GetAllTimeZoneNames());
      for ( MStdStringVector::const_iterator iter = timeZoneNames.begin(); iter != timeZoneNames.end(); ++iter )
      {
         MRegistry tzRegistry(timeZoneRegistry, *iter);
         if ( tzRegistry.IsOpen() &&
              (DoMatchRegistry(tzRegistry, displayString, name, dispName) ||
               DoMatchRegistry(tzRegistry, stdString,     name, dispName) ||
               DoMatchRegistry(tzRegistry, dltString,     name, dispName)) )
         {
            DoSetByRegistry(tzRegistry);
            return true;
         }
      }
   }
   return false;
}

   static void DoCopyStringToTimeZoneName(wchar_t* name, const MStdString& str)
   {
      M_COMPILED_ASSERT(sizeof(_TIME_ZONE_INFORMATION().StandardName) == sizeof(_TIME_ZONE_INFORMATION().DaylightName));

      MWideString dummy = MToWideString(str);
      const unsigned size = M_NUMBER_OF_ARRAY_ELEMENTS(_TIME_ZONE_INFORMATION().StandardName) - 1;
      wcsncpy(name, dummy.c_str(), size);
      M_ASSERT(name[size] == '\0'); // zero terminator is added by ZeroMemory
   }

void MTimeZone::SetFromCurrentSystem()
{
   Clear();

   TIME_ZONE_INFORMATION timeZone;
   DWORD zone = ::GetTimeZoneInformation(&timeZone);
   MESystemError::CheckLastSystemError(zone == TIME_ZONE_ID_INVALID); // if time zone is not supported at all

   SetByName(MToStdString(timeZone.StandardName));
}

bool MTimeZone::IsDST(const MTime& t, bool isTimeUtc) const
{
   if ( m_dynamic.GetInitialized() )
   {
      const YearlyTimeZoneInfo* zone = m_dynamic.GetYearlyTimeZoneInfo(t);
      M_ASSERT(zone != NULL);
      const MTimeRecurrentYearly& toDaylight = zone->m_switchToDaylightTime;
      const MTimeRecurrentYearly& toStandard = zone->m_switchToStandardTime;
      if ( toDaylight.IsNull() || toStandard.IsNull() )
         return false;
      int toDaylightMonth = toDaylight.GetMonth();
      int toStandardMonth = toStandard.GetMonth();
      if ( toDaylightMonth == 1 )
      {
         M_ASSERT(toDaylight.GetDayOfMonth() == 1); // for all timezones seen
         if ( toStandardMonth == 1 )
            return false;
         bool isSouthernHemisphere = toStandardMonth > 1 && toStandardMonth <= 6;
         if ( !isSouthernHemisphere )
            return false; // otherwise proceed with the check
      }
      else if ( toStandardMonth == 1 )
      {
         M_ASSERT(toStandard.GetDayOfMonth() == 1); // for all timezones seen
         bool isNorthernHemisphere = toDaylightMonth > 1 && toDaylightMonth <= 6;
         if ( isNorthernHemisphere )
            return false; // otherwise proceed with the check
      }
      return DoStaticTestIfDST(t, zone->m_switchToDaylightTime, zone->m_switchToStandardTime, zone->m_standardOffset, zone->m_daylightOffset, isTimeUtc);
   }
   return DoStaticTestIfDST(t, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, isTimeUtc);
}

      static void DoToSystemTime(const MTime& t, SYSTEMTIME* sysTime)
      {
         sysTime->wYear         = t.GetYear();
         sysTime->wMonth        = t.GetMonth();
         sysTime->wDayOfWeek    = t.GetMonth();
         sysTime->wDay          = t.GetDayOfMonth();
         sysTime->wHour         = t.GetHours();
         sysTime->wMinute       = t.GetMinutes();
         sysTime->wSecond       = t.GetSeconds();
         sysTime->wMilliseconds = 0;
      }

      static MTime DoFromSystemTime(SYSTEMTIME* sysTime)
      {
         return MTime(sysTime->wYear, sysTime->wMonth, sysTime->wDay, sysTime->wHour, sysTime->wMinute, sysTime->wSecond);
      }

int MTimeZone::GetUtcToLocalOffset(const MTime& t) const
{
   if ( m_dynamic.GetInitialized() )
   {
      const YearlyTimeZoneInfo* zone = m_dynamic.GetYearlyTimeZoneInfo(t);
      M_ASSERT(zone != NULL);
      int offset = zone->m_standardOffset;
      if ( DoStaticTestIfDST(t, zone->m_switchToDaylightTime, zone->m_switchToStandardTime, zone->m_standardOffset, zone->m_daylightOffset, true) )
         offset += zone->m_daylightOffset;
      return offset;
   }
   int offset = m_standardOffset;
   if ( DoStaticTestIfDST(t, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, true) )
      offset += m_daylightOffset;
   return offset;
}

int MTimeZone::GetLocalToUtcOffset(const MTime& t) const
{
   if ( m_dynamic.GetInitialized() )
   {
      const YearlyTimeZoneInfo* zone = m_dynamic.GetYearlyTimeZoneInfo(t);
      M_ASSERT(zone != NULL);
      int offset = -zone->m_standardOffset;
      if ( DoStaticTestIfDST(t, zone->m_switchToDaylightTime, zone->m_switchToStandardTime, zone->m_standardOffset, zone->m_daylightOffset, false) )
         offset -= zone->m_daylightOffset;
      return offset;
   }
   int offset = -m_standardOffset;
   if ( DoStaticTestIfDST(t, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, false) )
      offset -= m_daylightOffset;
   return offset;
}

MStdStringVector MTimeZone::GetAllTimeZoneNames()
{
   MRegistry conf(MRegistry::KeyLocalMachine, s_timeZoneListRegistryPlacement, true);
   return conf.GetAllSubkeys();
}

   static void DoFillTimezoneNames(MStdStringVector& names, bool longNames)
   {
      MStdString displayString("Display", 7);
      MRegistry reg(MRegistry::KeyLocalMachine, s_timeZoneListRegistryPlacement, true);
      names = reg.GetAllSubkeys();

      MStdStringVector::iterator it = names.begin();
      MStdStringVector::iterator itEnd = names.end();
      for ( ; it != itEnd; ++it )
      {
         try
         {
            MStdString name = *it;
            MRegistry child(reg, name);
            MStdString display = child.GetExistingString(displayString); // will throw if not present
            if ( longNames )
            {
               name.append(MTimeZone::s_timezoneNameSeparator, MTimeZone::s_timezoneNameSeparatorSize);
               name += display;
               *it = name;
            }
            else
               *it = display;
         }
         catch ( ... )
         {
            M_ASSERT(0); // it is okay at runtime (the system is misconfigured), but complain about it on debug
         }
      }
   }

MStdStringVector MTimeZone::GetAllTimeZoneDisplayNames()
{
   MStdStringVector result;
   DoFillTimezoneNames(result, true);
   return result;
}

MStdStringVector MTimeZone::GetAllTimeZoneLocalNames()
{
   MStdStringVector result;
   DoFillTimezoneNames(result, false);
   return result;
}
