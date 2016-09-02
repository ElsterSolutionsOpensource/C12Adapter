// File MCORE/private/MTimeZoneTz.cxx

#ifndef M__TIMEZONE_USE_TZ_IMPLEMENTATION
   #error "Do not compile .cxx files directly, they are included!"
#endif

#include "MAlgorithm.h"

// BEGIN tzfile.h chunk, which is in a public domain
//

#define TZ_MAGIC  "TZif"

struct tzhead {
   char tzh_magic[4];      // TZ_MAGIC
   char tzh_version[1];    // '\0' or '2' or '3' as of 2013
   char tzh_reserved[15];  // reserved; must be zero
   char tzh_ttisgmtcnt[4]; // coded number of trans. time flags
   char tzh_ttisstdcnt[4]; // coded number of trans. time flags
   char tzh_leapcnt[4];    // coded number of leap seconds
   char tzh_timecnt[4];    // coded number of transition times
   char tzh_typecnt[4];    // coded number of local time types
   char tzh_charcnt[4];    // coded number of abbr. chars
};

#define TZ_MAX_TIMES 2000
#define TZ_MAX_TYPES 256
#define TZ_MAX_CHARS 50
#define TZ_MAX_LEAPS 50

//
// END tzfile.h chunk

   class MSingletonFileName
   {
      MCriticalSection m_criticalSection; // has to be a member of the class

      MStdString m_fileName;

      const char** m_candidates;

   public:

      MSingletonFileName(const char** candidates);

      ~MSingletonFileName();

      const MStdString& GetFileName();
   };

   MSingletonFileName::MSingletonFileName(const char** candidates)
   :
      m_criticalSection(),
      m_fileName(),
      m_candidates(candidates)
   {
      M_ASSERT(candidates != NULL && candidates[0] != NULL); // there has to be at least one initializer
   }

   MSingletonFileName::~MSingletonFileName()
   {
   }

   const MStdString& MSingletonFileName::GetFileName()
   {
      MCriticalSection::Locker locker(m_criticalSection);
      if ( m_fileName.empty() )
      {
         for ( const char** it = m_candidates; *it != NULL; ++it )
         {
            const MStdString& str = *it;
            if ( MUtilities::IsPathExisting(str) )
            {
               m_fileName = str;
               return m_fileName; // success
            }
         }
         // report the very first name as it was the most probable
         MESystemError::ThrowFileNotOpen(m_candidates[0]);
         M_ENSURED_ASSERT(0);
      }
      return m_fileName;
   }

const MStdString& MTimeZone::GetCurrentTimezoneFilePath()
{
   static const char* s_paths[] = {"/etc/localtime",
                                   "/usr/local/etc/localtime",
                                   NULL};
   static MSingletonFileName s_cache(s_paths);
   return s_cache.GetFileName();
}

const MStdString& MTimeZone::GetZoneInfoDirectoryPath()
{
   static const char* s_paths[] = {"/usr/share/zoneinfo",
                                   "/usr/lib/zoneinfo",
                                   "/usr/local/etc/zoneinfo",
                                   NULL};
   static MSingletonFileName s_cache(s_paths);
   return s_cache.GetFileName();
}

bool MTimeZone::DoSetByName(const MStdString& originalName)
{
   Clear();

   MStreamFile file;
   MStdString fullName;
   MStdString standardName = originalName;
   if ( MUtilities::IsPathFull(originalName) )
      fullName = originalName;
   else if ( originalName.empty() ) // default zone
      fullName = GetCurrentTimezoneFilePath();
   else
   {
      fullName = GetZoneInfoDirectoryPath();
      MAddDirectorySeparatorIfNecessary(fullName);
      for ( int i = 0; ; )
      {
         const MPrivateWindowsToIanaType& mapping = s_windowsToIana[i];
         if ( originalName == mapping.m_windows )
         {
            fullName += mapping.m_iana;
            break;
         }
         else if ( originalName == mapping.m_iana )
         {
            standardName = mapping.m_windows; // redefine
            fullName += originalName;
            break;
         }
         ++i;
         if ( i == M_NUMBER_OF_ARRAY_ELEMENTS(s_windowsToIana) )
         {
            fullName += originalName;
            break;
         }
      }
   }

   try
   {
      file.Open(fullName, MStreamFile::FlagReadOnly | MStreamFile::FlagBuffered);

      tzhead head;
      file.ReadBytes(reinterpret_cast<char*>(&head), sizeof(head));

      unsigned leapSecondsCount     = MFromBigEndianUINT32(head.tzh_leapcnt);
      unsigned transitionTimesCount = MFromBigEndianUINT32(head.tzh_timecnt);
      unsigned typesCount           = MFromBigEndianUINT32(head.tzh_typecnt);
      unsigned abbreviationsCount   = MFromBigEndianUINT32(head.tzh_charcnt);

      if ( memcmp(head.tzh_magic, TZ_MAGIC, sizeof(head.tzh_magic)) != 0 ||
           (head.tzh_version[0] != '3' &&
            head.tzh_version[0] != '2' &&
            head.tzh_version[0] != '\0') ||
           leapSecondsCount     > TZ_MAX_LEAPS ||
           transitionTimesCount > TZ_MAX_TIMES ||
           typesCount           > TZ_MAX_TYPES ||
           abbreviationsCount   > TZ_MAX_CHARS )
      {
         throw 0; // sophisticated goto
      }

      m_dynamic.m_transitions.resize(transitionTimesCount);
      for ( unsigned i = 0; i < transitionTimesCount; ++i )
      {
         char buff [ sizeof(Muint32) ];
         file.ReadBytes(buff, sizeof(Muint32));
         DynamicTimeZone::TransitionType& transition = m_dynamic.m_transitions[i];
         transition.m_transitionTime = static_cast<time_t>(MFromBigEndianUINT32(buff));
      }
      for ( unsigned i = 0; i < transitionTimesCount; ++i )
      {
         DynamicTimeZone::TransitionType& transition = m_dynamic.m_transitions[i];
         char buff;
         file.ReadBytes(&buff, sizeof(buff));
         transition.m_offsetIndex = buff;
      }
      for ( unsigned j = 0; j < typesCount; ++j )
      {
         struct ttinfo {
             char   tt_gmtoff [ 4 ];
             Muint8 tt_isdst;
             Muint8 tt_abbrind;
         };
         ttinfo info;
         M_COMPILED_ASSERT(sizeof(info) == 6);
         file.ReadBytes(reinterpret_cast<char*>(&info), sizeof(info));

         int offset = static_cast<int>(MFromBigEndianUINT32(info.tt_gmtoff));
         for ( unsigned i = 0; i < transitionTimesCount; ++i )
         {
            DynamicTimeZone::TransitionType& transition = m_dynamic.m_transitions[i];
            if ( transition.m_offsetIndex == j )
            {
               transition.m_offset = offset;
               transition.m_isDst = info.tt_isdst;
               transition.m_abbreviationIndex = info.tt_abbrind;
            }
         }
      }

      const MTime& now = MTime::GetCurrentUtcTime();
      m_standardOffset = GetStandardOffsetForTime(now);
      m_daylightOffset = GetDaylightOffsetForYear(now.GetYear());
      m_standardName = standardName;
      m_displayName  = standardName;
      SetInitializedFromDatabase(true);
      DoComputeRecurringSwitchTimes();
   }
   catch ( ... )
   {
      if ( !file.IsOpen() ) // report as not existing timezone
         return false;

      // As the file was open, all the rest of errors are due to bad format.
      // Report them as such.
      MException::ThrowBadFileFormat(fullName);
      M_ENSURED_ASSERT(0);
   }
   return true;
}

void MTimeZone::SetFromCurrentSystem()
{
   Clear();
   DoSetByName(MStdString());
}

bool MTimeZone::IsDST(const MTime& t, bool isTimeUtc) const
{
   if ( m_dynamic.GetInitialized() )
   {
      time_t time = t.GetTimeT();
      DynamicTimeZone::TransitionsVector::const_reverse_iterator it = m_dynamic.m_transitions.rbegin();
      DynamicTimeZone::TransitionsVector::const_reverse_iterator itEnd = m_dynamic.m_transitions.rend() - 1;
      if ( isTimeUtc )
      {
         for ( ; it < itEnd; ++it )
         {
            const DynamicTimeZone::TransitionType& tt = *it;
            if ( tt.m_transitionTime <= time )
            {
               bool result = tt.m_isDst != 0;
               return result;
            }
         }
      }
      else
      {
         for ( ; it < itEnd; ++it )
         {
            const DynamicTimeZone::TransitionType& tt = *it;
            const int previousOffset = (*(it + 1 == itEnd ? it : it + 1)).m_offset;
            long diff = tt.m_transitionTime - (time - previousOffset);
            if ( diff <= 0 )
            {
               /*
               const DynamicTimeZone::TransitionType& ttn = *(it - 1);
               long diff2 = tt.m_offset - ttn.m_offset;
               if ( diff > diff2 )
                  return ttn.m_isDst != 0;
                  */
               bool result = tt.m_isDst != 0;
               return result;
            }
         }
      }
      return false; // no better guess
   }
   return DoStaticTestIfDST(t, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, isTimeUtc);
}

int MTimeZone::GetUtcToLocalOffset(const MTime& t) const
{
   if ( m_dynamic.GetInitialized() )
   {
      M_ASSERT(!m_dynamic.m_transitions.empty());

      time_t time = t.GetTimeT();
      DynamicTimeZone::TransitionsVector::const_reverse_iterator it = m_dynamic.m_transitions.rbegin();
      DynamicTimeZone::TransitionsVector::const_reverse_iterator itEnd = m_dynamic.m_transitions.rend() - 1;
      for ( ; it < itEnd; ++it )
      {
         const DynamicTimeZone::TransitionType& tt = *it;
         if ( tt.m_transitionTime <= time )
            return tt.m_offset;
      }

      // offset is smaller than the smallest, return the smallest offset
      return (*m_dynamic.m_transitions.begin()).m_offset;
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
      time_t time = t.GetTimeT();
      DynamicTimeZone::TransitionsVector::const_reverse_iterator it = m_dynamic.m_transitions.rbegin();
      DynamicTimeZone::TransitionsVector::const_reverse_iterator itEnd = m_dynamic.m_transitions.rend() - 2;
      for ( ; it < itEnd; ++it )
      {
         const DynamicTimeZone::TransitionType& tt = *it;
         const int previousNegativeOffset = (*(it + 1)).m_offset;
         if ( tt.m_transitionTime <= time - previousNegativeOffset )
            return -tt.m_offset;
      }

      // offset is smaller than the smallest, return the smallest offset
      return -(*m_dynamic.m_transitions.begin()).m_offset;
   }
   int offset = -m_standardOffset;
   if ( DoStaticTestIfDST(t, m_switchToDaylightTime, m_switchToStandardTime, m_standardOffset, m_daylightOffset, false) )
      offset -= m_daylightOffset;
   return offset;
}

MStdStringVector MTimeZone::GetAllTimeZoneNames()
{
   MStdStringVector result;
   MStdString fullPath;
   MStdString zoneInfoDirectoryPath = GetZoneInfoDirectoryPath();
   fullPath.reserve(64); // typical for "/usr/share/zoneinfo/" plus no more than 16 bytes of the name
   for ( unsigned i = 0; i < M_NUMBER_OF_ARRAY_ELEMENTS(s_windowsToIana); ++i )
   {
      fullPath = zoneInfoDirectoryPath;
      MAddDirectorySeparatorIfNecessary(fullPath);
      const MPrivateWindowsToIanaType& mapping = s_windowsToIana[i];
      fullPath += mapping.m_iana;
      if ( MUtilities::IsPathExisting(fullPath) )
         result.push_back(mapping.m_windows);
   }
   MAlgorithm::InplaceSort(result, false, true);
   return result;
}

MStdStringVector MTimeZone::GetAllTimeZoneDisplayNames()
{
   return GetAllTimeZoneNames(); // ??!! will implement later
}

MStdStringVector MTimeZone::GetAllTimeZoneLocalNames()
{
   return GetAllTimeZoneNames(); // ??!! will implement later
}
