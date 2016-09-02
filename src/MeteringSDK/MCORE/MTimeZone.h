// File MCORE/MTimeZone.h
#ifndef MCORE_MTIMEZONE_H
#define MCORE_MTIMEZONE_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MTimeZone.h

#include <MCORE/MObject.h>
#include <MCORE/MTime.h>
#include <MCORE/MTimeSpan.h>
#include <MCORE/MTimeRecurrentYearly.h>

#if !M_NO_TIME

/// \cond SHOW_INTERNAL
#if !defined(M__TIMEZONE_USE_WINDOWS_REGISTRY)  && !defined(M__TIMEZONE_USE_ANDROID_IMPLEMENTATION) && \
    !defined(M__TIMEZONE_USE_TZ_IMPLEMENTATION) && !defined(M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION)

   // Guess suitable implementation
   #if (M_OS & M_OS_WINDOWS) != 0 
      #if M_NO_REGISTRY
         #error "TimeZone on Windows requires M_NO_REGISTRY=0. Or define M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION for simple timezone behavior"
      #endif
      #define M__TIMEZONE_USE_WINDOWS_REGISTRY 1
   #elif (M_OS & M_OS_ANDROID) != 0
      #if !M_NO_JNI
         #define M__TIMEZONE_USE_ANDROID_IMPLEMENTATION 1
      #else
         #define M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION 1
      #endif
   #elif (M_OS & (M_OS_POSIX | M_OS_LINUX)) != 0
      #define M__TIMEZONE_USE_TZ_IMPLEMENTATION 1
   #else
      #define M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION 1
   #endif
#elif (M__TIMEZONE_USE_WINDOWS_REGISTRY+0)  + (M__TIMEZONE_USE_ANDROID_IMPLEMENTATION+0) + \
      (M__TIMEZONE_USE_TZ_IMPLEMENTATION+0) + (M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION+0) != 1 // cheaters can cheat with negative numbers, oh well...
   #error "Inconsistent definition of timezone implementation macros - only one should be set to 1"
#endif
#if defined(M__TIMEZONE_USE_ANDROID_IMPLEMENTATION) && M__TIMEZONE_USE_ANDROID_IMPLEMENTATION
   #if !defined(M_NO_JNI) || M_NO_JNI != 0
      #error "jni.h is not included, define configuration option M_NO_JNI=0"
   #endif
#endif
/// \endcond SHOW_INTERNAL

/// Time zone.
///
/// Timezone holds information about offset from Coordinated Universal Time,
/// Daylight Saving Time (DST) settings, switch dates, and names for all of it.
/// Timezones can be created by supplying values directly,
/// or by name using services supplied by the operating system.
///
/// There is a static property, Current timezone, from which the computer current timezone
/// can be accessed. However one cannot change computer's timezone using MeteringSDK interfaces.
///
class M_CLASS MTimeZone : public MObject
{
private: // Types:
#if defined(M__TIMEZONE_USE_WINDOWS_REGISTRY) && M__TIMEZONE_USE_WINDOWS_REGISTRY == 1

   struct YearlyTimeZoneInfo
   {
      int m_standardOffset;
      int m_daylightOffset;
      MTimeRecurrentYearly m_switchToStandardTime;
      MTimeRecurrentYearly m_switchToDaylightTime;

      bool operator==(const YearlyTimeZoneInfo& other) const
      {
         return m_standardOffset       == other.m_standardOffset &&
                m_daylightOffset       == other.m_daylightOffset &&
                m_switchToStandardTime == other.m_switchToStandardTime &&
                m_switchToDaylightTime == other.m_switchToDaylightTime;
      }
      bool operator!=(const YearlyTimeZoneInfo& other) const
      {
         return !operator==(other);
      }
   };

   // Vector of TZI data for different years
   //
   typedef std::vector<YearlyTimeZoneInfo>
      YearlyTimeZoneInfoVector;

   struct DynamicTimeZone
   {
      YearlyTimeZoneInfoVector m_tzi;
      int m_firstYear;
      bool m_isInitialized;

      DynamicTimeZone()
      :
         m_tzi(),
         m_firstYear(0),
         m_isInitialized(false)
      {
      }

      DynamicTimeZone(const DynamicTimeZone& other)
      :
         m_firstYear(other.m_firstYear),
         m_tzi(other.m_tzi),
         m_isInitialized(other.m_isInitialized)
      {
      }

      ~DynamicTimeZone()
      {
      }

      DynamicTimeZone& operator=(const DynamicTimeZone& other)
      {
         m_tzi           = other.m_tzi;
         m_firstYear     = other.m_firstYear;
         m_isInitialized = other.m_isInitialized;
         return *this;
      }

      bool operator==(const DynamicTimeZone& other) const
      {
         return m_isInitialized == other.m_isInitialized &&
                m_firstYear     == other.m_firstYear     &&
                m_tzi           == other.m_tzi;
      }
      bool operator!=(const DynamicTimeZone& other) const
      {
         return !operator==(other);
      }

      bool GetInitialized() const
      {
         return m_isInitialized;
      }
      void SetInitialized(bool yes)
      {
         m_isInitialized = yes;
         // even in case of false do not clear anything for the case the parameter will be set back to true shortly
      }

      bool IsPresent() const
      {
         M_ASSERT((m_firstYear == 0) == m_tzi.empty());
         // m_isInitialized is not affected and it is a different kind of information
         return m_firstYear != 0;
      }

      void Reset()
      {
         m_tzi.clear();
         m_firstYear = 0;
         m_isInitialized = false;
      }

      const YearlyTimeZoneInfo* GetYearlyTimeZoneInfo(const MTime& relevantTime) const
      {
         M_ASSERT(IsPresent());
         const int year = relevantTime.GetYear();
         YearlyTimeZoneInfoVector::size_type index = static_cast<YearlyTimeZoneInfoVector::size_type>((year < m_firstYear)
                                                                                                      ? 0 
                                                                                                      : year - m_firstYear);
         if ( index >= m_tzi.size() )
            index = m_tzi.size() - 1;
         return &m_tzi[index];
      }
   };

#elif defined(M__TIMEZONE_USE_ANDROID_IMPLEMENTATION) && M__TIMEZONE_USE_ANDROID_IMPLEMENTATION != 0

   struct DynamicTimeZone
   {
      jobject m_timeZone;
      bool m_isInitialized; // separate flag, do not rely on m_genericTimeZone pointer

      DynamicTimeZone()
      :
         m_timeZone(NULL),
         m_isInitialized(false)
      {
      }

      DynamicTimeZone(const DynamicTimeZone& other)
      :
         m_timeZone(NULL),
         m_isInitialized(false)
      {
         operator=(other);
      }

      ~DynamicTimeZone()
      {
         Reset();
      }

      DynamicTimeZone& operator=(const DynamicTimeZone& other);

      bool operator==(const DynamicTimeZone& other) const;
      bool operator!=(const DynamicTimeZone& other) const
      {
         return !operator==(other);
      }

      void Reset();

      bool GetInitialized() const
      {
         return m_isInitialized;
      }
      void SetInitialized(bool yes)
      {
         m_isInitialized = yes;
      }

      bool IsPresent() const
      {
         return m_timeZone != NULL;
      }
   };

#elif defined(M__TIMEZONE_USE_TZ_IMPLEMENTATION) && M__TIMEZONE_USE_TZ_IMPLEMENTATION != 0

   struct DynamicTimeZone
   {
   public: // Types:

      struct TransitionType
      {
         Mint64 m_transitionTime;
         union
         {
            Muint64 m_attributes;
            struct
            {
               int     m_offset;
               Muint8  m_isDst;       // do not use bool, as it is 4 bytes in gcc
               Muint8  m_offsetIndex;
               Muint8  m_abbreviationIndex;
            };
         };

         TransitionType()
         {
            m_transitionTime = 0;
            m_attributes = 0;
         }

         TransitionType(const TransitionType& other)
         {
            operator=(other);
         }

         TransitionType& operator=(const TransitionType& other)
         {
            m_transitionTime = other.m_transitionTime;
            m_attributes     = other.m_attributes;
            return *this;
         }

         bool operator==(const TransitionType& other) const
         {
            return m_transitionTime == other.m_transitionTime &&
                   m_attributes     == other.m_attributes;
         }
         bool operator!=(const TransitionType& other) const
         {
            return !operator==(other);
         }
      };

      typedef std::vector<TransitionType>
         TransitionsVector;

   public: // Fields:

      TransitionsVector m_transitions;

      DynamicTimeZone()
      :
         m_transitions()
      {
      }

      DynamicTimeZone(const DynamicTimeZone& other)
      :
         m_transitions(other.m_transitions)
      {
      }

      ~DynamicTimeZone()
      {
      }

      DynamicTimeZone& operator=(const DynamicTimeZone& other)
      {
         m_transitions = other.m_transitions;
         return *this;
      }

      bool operator==(const DynamicTimeZone& other) const
      {
         return m_transitions == other.m_transitions;
      }
      bool operator!=(const DynamicTimeZone& other) const
      {
         return !operator==(other);
      }

      void Reset()
      {
         m_transitions.clear();
      }

      bool GetInitialized() const
      {
         return !m_transitions.empty();
      }
      void SetInitialized(bool yes)
      {
         if ( !yes )
            Reset();
      }

      bool IsPresent() const
      {
         return GetInitialized();
      }
   };

#elif defined(M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION) && M__TIMEZONE_USE_SIMPLE_IMPLEMENTATION != 0

   // Dummy, implementation convenience
   //
   struct DynamicTimeZone
   {
      bool GetInitialized() const
      {
         return false;
      }
      void SetInitialized(bool yes)
      {
         M_ASSERT(!yes); // should not be true for simple implementations
         M_USED_VARIABLE(yes);
      }

      bool IsPresent() const
      {
         return false;
      }

      void Reset()
      {
      }

      bool operator==(const DynamicTimeZone&) const
      {
         return true;
      }
      bool operator!=(const DynamicTimeZone& other) const
      {
         return !operator==(other);
      }
   };

#else
   #error "Inconsistent definition of timezone implementation macros"
#endif

public: // Services:

   /// Default constructor to create the current timezone, the one that matches the computer timezone.
   ///
   /// If the user creates such timezone, it will not be updated automatically if the computer timezone changes
   /// after the class is created. Instead, it will stay the same as the time when it was created.
   /// Similarly, one can change any properties of this object without affecting the computer timezone.
   ///
   /// To create a timezone with all fields set to zeros or empty strings, use constructor that accepts
   /// a single standard offset, and have it at zero.
   ///
   /// \post \ref InitializedFromDatabase property will be true.
   ///
   MTimeZone();

   /// Constructor that creates a new timezone using operating system defined timezone name.
   ///
   /// All timezone properties get filled from the operating system definition for
   /// such timezone.
   ///
   /// \param name
   ///    Name of the timezone.
   ///    The given timezone name shall be valid, and it shall be defined in the operating system,
   ///    otherwise an exception is thrown.
   ///
   /// \post \ref InitializedFromDatabase property will be false.
   ///
   MTimeZone(const MStdString& name);

   /// Constructor that creates a new timezone with only a standard offset defined.
   ///
   /// No name is given to daylight or to standard time.
   /// The created timezone will have no daylight saving time settings.
   ///
   /// \param standardOffset
   ///     Standard offset of this timezone in seconds.
   ///     There is a check that the given offset is within range of -13 to 13 hours,
   ///     which translates into standardOffset value range of -46800 .. 46800,
   ///     and it should be divisible by 5 minutes (300 seconds), or an exception will be raised.
   ///
   /// \post \ref InitializedFromDatabase property will be false.
   ///
   MTimeZone(int standardOffset);

   /// Constructor to create a timezone with a standard offset and name.
   //
   /// No name is given to to daylight time, and the newly created timezone will have no daylight saving time settings.
   /// This call does not use the operating system facilities to verify whether such
   /// timezone name exists, or whether it has the same standard offset as one supplied.
   ///
   /// \param standardOffset
   ///     Standard offset of this timezone from UTC in seconds.
   ///     There is a check that the given offset is within range of -13 to 13 hours,
   ///     which translates into range -46800 .. 46800,
   ///     and it should be divisible by 5 minutes (300 seconds), or an exception will be raised.
   /// \param standardName
   ///     The name to give to the newly created timezone object, does not have to be the one defined by the operating system.
   ///
   /// \post \ref InitializedFromDatabase property will be false.
   ///
   MTimeZone(int standardOffset, const MStdString& standardName);

   /// Constructor that creates a new unnamed timezone with an offset, and daylight saving time parameters defined.
   ///
   /// No names are given to daylight time or to standard time. The given offsets are in seconds.
   /// This call does not use the operating system facilities to verify whether such
   /// timezone name exists, or whether it has the same standard offset as one supplied.
   ///
   /// \param standardOffset
   ///     Standard offset of this timezone from UTC in seconds.
   ///     There is a check that the given offset is within range of -13 to 13 hours,
   ///     which translates into range -46800 .. 46800,
   ///     and it should be divisible by 5 minutes (300 seconds), or an exception will be raised.
   /// \param daylightOffset
   ///     Daylight offset of this timezone from standard in seconds, typically 3600 seconds or one hour.
   ///     There is a check that the given offset is within range of -3 to 3 hours (range -10800 .. 10800),
   ///     and it should be divisible by 5 minutes (300 seconds), or an exception will be raised.
   /// \param switchToDaylightTime
   ///     Yearly recurring switch time, at which the daylight time starts.
   ///     On the northern hemisphere this would typically be in March or April,
   ///     while on the southern this is September or October.
   /// \param switchToStandardTime
   ///     Yearly recurring switch time, at which the daylight time ends.
   ///     On the northern hemisphere this would typically be in September or October,
   ///     while on the southern this is March or April.
   ///
   /// \post \ref InitializedFromDatabase property will be false.
   ///
   MTimeZone(int standardOffset, int daylightOffset, const MTimeRecurrentYearly& switchToDaylightTime, const MTimeRecurrentYearly& switchToStandardTime);

   /// Constructor that creates a new named timezone with daylight saving time defined.
   ///
   /// All timezone properties are filled with parameters given in this constructor.
   /// This call does not use the operating system facilities to verify whether such
   /// timezone name exists, or whether it has the same standard offset as one supplied.
   ///
   /// \param standardOffset
   ///     Standard offset of this timezone from UTC in seconds.
   ///     There is a check that the given offset is within range of -13 to 13 hours,
   ///     which translates into range -46800 .. 46800,
   ///     and it should be divisible by 5 minutes (300 seconds), or an exception will be raised.
   /// \param daylightOffset
   ///     Daylight offset of this timezone from standard in seconds, typically 3600 seconds or one hour.
   ///     There is a check that the given offset is within range of -3 to 3 hours (range -10800 .. 10800),
   ///     and it should be divisible by 5 minutes (300 seconds), or an exception will be raised.
   /// \param switchToDaylightTime
   ///     Yearly recurring switch time, at which the daylight time starts.
   ///     On the northern hemisphere this would typically be in March or April,
   ///     while on the southern this is September or October.
   /// \param switchToStandardTime
   ///     Yearly recurring switch time, at which the daylight time ends.
   ///     On the northern hemisphere this would typically be in September or October,
   ///     while on the southern this is March or April.
   /// \param standardName
   ///     The name to give to the newly created timezone object, does not have to be the one defined by the operating system.
   /// \param daylightName
   ///     The daylight name to give to the newly created timezone object, does not have to be the one defined by the operating system.
   ///
   /// \post \ref InitializedFromDatabase property will be false.
   ///
   MTimeZone(int standardOffset, int daylightOffset, const MTimeRecurrentYearly& switchToDaylightTime, const MTimeRecurrentYearly& switchToStandardTime, const MStdString& standardName, const MStdString& daylightName);

   /// Copy constructor, creates a timezone from a copy given.
   ///
   /// After construction, a copy can be modified without influencing the computer timezone,
   /// or the timezone from which this copy is made.
   ///
   /// \param other
   ///    Timezone object from which a copy shall be made.
   ///
   MTimeZone(const MTimeZone& other);

   /// Object destructor
   ///
   virtual ~MTimeZone();

   /// Assignment operator, assigns one timezone into another.
   ///
   /// Assignment to a current timezone only changes the object instance,
   /// it does not change the timezone in the computer.
   ///
   /// \param other
   ///    Timezone object from which all properties of this timezone shall be initialized.
   ///
   MTimeZone& operator=(const MTimeZone& other);

   /// Equality operator, returns true if all properties of one timezone match another timezone's properties.
   ///
   bool operator==(const MTimeZone& other) const;

   /// Inequality operator, returns true if any property of one timezone does not match one in another timezone.
   ///
   bool operator!=(const MTimeZone& other) const
   {
      return !operator==(other);
   }

public: // Important static properties and services:

   /// Access the globally present current computer timezone.
   ///
   /// Important behavior of this property is that the computer timezone information is not fetched every time
   /// this property is accessed. Instead, the timezone value is cached to speed up the call.
   /// To support cases when the computer timezone changes from the administrative facility of the operating system,
   /// this property, when accessed, re-reads the timezone information, but no more often than each ten seconds.
   /// The value returned by this call will only reinitialize if the operating system's timezone changes.
   ///
   /// This property can be used from multiple threads, however one should never attempt to change the
   /// timezone returned by this method.
   ///
   /// While the computer timezone can be accessed, the MeteringSDK interface
   /// does not have means to change the current timezone as it is assumed
   /// to be an administrative task separate from applications built with MeteringSDK.
   ///
   /// \see SetFromCurrentSystem - forcefully reinitialize the timezone object from the current system timezone.
   ///
   static MTimeZone* GetCurrent();

   /// Access the names of all globally present timezones, as defined by the operating system.
   ///
   /// A typical timezone name will be "Eastern Standard Time".
   /// Note that the timezone name is not locale dependent.
   ///
   /// \seeprop{GetAllTimeZoneDisplayNames, AllTimeZoneDisplayNames} - return descriptive names suitable for displaying to the user.
   ///
   static MStdStringVector GetAllTimeZoneNames();

   /// Access the names of all globally present timezones, verbose representation.
   ///
   /// A typical timezone display name will be "Eastern Standard Time | (GMT-05:00) Eastern Time (US & Canada)".
   /// Note that the display timezone name is locale dependent.
   ///
   /// \seeprop{GetAllTimeZoneNames, AllTimeZoneNames} - return short timezone names in English locale.
   /// \seeprop{GetAllTimeZoneLocalNames, AllTimeZoneLocalNames} - different from this, return shorter but localized timezone names.
   ///
   static MStdStringVector GetAllTimeZoneDisplayNames();

   /// Access the unprocessed local names of the timezone, a rather concise representation.
   ///
   /// A typical timezone local name will be "(GMT-05:00) Eastern Time (US & Canada)".
   /// Note that the display timezone name is locale dependent.
   ///
   /// \seeprop{GetAllTimeZoneNames, AllTimeZoneNames} - return short timezone names in English locale.
   /// \seeprop{GetAllTimeZoneDisplayNames, AllTimeZoneDisplayNames} - different from this, return longer and more verbose localized timezone names.
   ///
   static MStdStringVector GetAllTimeZoneLocalNames();

   /// Timezone name separator, as used to separate name from display name.
   ///
   static const MChar s_timezoneNameSeparator[];

   /// Number of characters of timezone name separator, s_timezoneNameSeparator.
   ///
   static const unsigned s_timezoneNameSeparatorSize;

public: // Time extracting properties:

   /// Get current UTC time, independent from any timezone, static property.
   ///
   static MTime GetUtcTime();

   /// Get local time according to this timezone.
   ///
   /// Local time respects DST and the timezone shift.
   ///
   MTime GetLocalTime() const;

   /// Get standard time according to this timezone.
   ///
   /// Standard time does not respect DST, if DST exists for this timezone.
   ///
   MTime GetStandardTime() const;

public: // Time extracting properties:

   ///@{
   /// Whether the timezone was initialized from the operating system standard timezone database, or directly specified by the user.
   ///
   /// This flag will be true only if the timezone is loaded from the internal timezone database, as present in the OS.
   /// Once the timezone is loaded from the database, any attempt to change any of its properties
   /// leads to setting this flag back to false.
   ///
   bool InitializedFromDatabase() const
   {
      return m_dynamic.GetInitialized();
   }
   void SetInitializedFromDatabase(bool yes)
   {
      m_dynamic.SetInitialized(yes);
   }
   ///@}

   ///@{
   /// Standard timezone name, as used by this object.
   ///
   const MStdString& GetStandardName() const
   {
      return m_standardName;
   }
   void SetStandardName(const MStdString&);
   ///@}

   ///@{
   /// Get daylight saving time name of this timezone object.
   ///
   const MStdString& GetDaylightName() const
   {
      return m_daylightName;
   }
   void SetDaylightName(const MStdString& name);
   ///@}

   ///@{
   /// Timezone shift in seconds from UTC at the present year.
   ///
   /// Timezone DST status is not respected by this value, shall be taken into consideration separately.
   /// This property modifies only this timezone setting, without affecting the computer timezone.
   ///
   /// \pre When set, there is a check that the given offset is within range.
   ///      The correct standard offset should be within range of -13 .. 13 hours,
   ///      which translates into range -46800 .. 46800,
   ///      and it should be divisible by 5 minutes (300 seconds).
   ///      Otherwise an exception will be thrown.
   ///
   int GetStandardOffset() const
   {
      return m_standardOffset;
   }
   void SetStandardOffset(int offset);
   ///@}

   ///@{
   /// DST shift from Standard in seconds at the present year, typically 3600 seconds or one hour.
   ///
   /// DST shift stays the same for any date of the year.
   /// One can use the method IsDST to determine if the given local time is in the DST.
   /// Local time DST status is not respected by this value, 
   /// shall be taken into consideration separately.
   /// Normally DST makes a shift by one hour, but there are time zones when the
   /// shift is made by 30 minutes, or 2 hours.
   ///
   /// \pre When set, there is a check that the given offset is within range.
   ///      The correct daylight saving offset shall be in range -10800 .. 10800, which is -3 .. 3 hours,
   ///      and it should be divisible by 300 seconds (5 minutes).
   ///      Otherwise an exception will be thrown.
   ///
   int GetDaylightOffset() const
   {
      return m_daylightOffset;
   }
   void SetDaylightOffset(int offset);
   ///@}

   /// Whether the local timezone has a notion of DST currently, and in the future.
   ///
   /// When this property is true, the timezone has times currently or in the future where \ref IsDST is true.
   /// When this property is false the timezone has no daylight saving related switches currently, or in the future.
   /// The timezone can still have DST in the past, or it can have transitions in the future unrelated to DST.
   ///
   /// \see \ref HasSwitchTimes tells if the timezone has any transitions, regular or not, in the past or in the future.
   ///
   bool SupportsDST() const;

   /// Whether the local timezone has any switch times in the past or in the future, related to DST or not.
   ///
   /// When this property is false the timezone has no DST switches, or any other changes to the offset.
   ///
   /// \see \ref SupportsDST tells if the timezone has DST related switches currently or in the future.
   ///
   bool HasSwitchTimes() const;

   ///@{
   /// Locale dependent timezone name suitable for displaying to humans.
   ///
   const MStdString& GetDisplayName() const
   {
      return m_displayName;
   }
   void SetDisplayName(const MStdString& name);
   ///@}

   /// Get the shift of the given local time from UTC, given the properties of the current timezone.
   ///
   /// Different from \refprop{GetDaylightOffset, DaylightOffset}
   /// and \refprop{GetStandardOffset, StandardOffset}, this service takes into consideration
   /// whether the given time is within DST period.
   ///
   /// A closely related call \ref GetUtcToLocalOffset will yield a reverse value unless the
   /// given timezone supports DST, and the localTime parameter is within a DST hour.
   ///
   /// Local time ambiguity that takes place when DST is being switched back to Standard
   /// is resolved by this method in a way as if the given local time is on DST.
   ///
   /// Invalid local time that takes place when DST is switched on from Standard
   /// is not reported as error.
   ///
   /// \param localTime
   ///      The time for which to determine the current offset from UTC, has to be local time.
   ///      If the current time is null, a No Value exception is thrown.
   /// \return Offset in seconds
   ///
   int GetLocalToUtcOffset(const MTime& localTime) const;

   /// Get the shift of the given UTC time from Local, given the properties of the current timezone.
   ///
   /// Different from \refprop{GetDaylightOffset, DaylightOffset}
   /// and \refprop{GetStandardOffset, StandardOffset}, this service takes into consideration
   /// whether the given time is within DST period.
   ///
   /// A closely related call \ref GetLocalToUtcOffset will yield a reverse value unless the
   /// given timezone supports DST, and the utcTime parameter is within a DST hour. 
   /// 
   /// \param utcTime
   ///      The time for which to determine the current offset from Local, has to be UTC time.
   ///      If the current time is null, a No Value exception is thrown.
   /// \return Offset in seconds
   ///
   int GetUtcToLocalOffset(const MTime& utcTime) const;

   /// Get the standard offset for a given time.
   ///
   /// If the time given is not on DST, this is equal to local offset.
   /// Otherwise, if the given time is on DST, this is the local offset
   /// at the time this DST period ends.
   ///
   /// \param utcTime
   ///      The UTC time for which to determine the current standard offset.
   ///      If the current time is null, a No Value exception is thrown.
   /// \return Standard offset in seconds.
   ///
   int GetStandardOffsetForTime(const MTime& utcTime) const;

   /// Get the daylight offset in a given year.
   ///
   /// If DST is not observed in the given year, zero is returned.
   /// Otherwise, if the given year has DST, this is the number of seconds
   /// the local time clock has to move at DST.
   ///
   /// If the timezone has an offset change that is not DST related,
   /// zero is returned for such year.
   ///
   /// \param year
   ///      The year in four-digit format for which to determine the DST offset.
   /// \return Offset in seconds.
   ///
   int GetDaylightOffsetForYear(int year) const;

   /// True will mean that according to this timezone, the given time is in DST.
   ///
   /// When the service takes the local time, as determined by the second parameter being false,
   /// and when the switch from Standard to DST occurs,
   /// there is a range of impossible local time values (typically an hour).
   /// By convention, since it is already after the DST switch,
   /// this time range resolves into a time as if DST switch has occurred, and for such time IsDST will return True.
   /// When DST switches into Local, and the clock is moved backwards, there is an ambiguous local time,
   /// and for such time IsDST will again return True.
   ///
   /// When the service takes a UTC time, as determined by the second parameter being true,
   /// there is no ambiguity.
   ///
   /// \param time
   ///      The time for which to determine if it fits within DST period.
   ///      If the current time is null, an exception is thrown.
   ///      Whether this is a UTC or local time is determined
   ///      by optional parameter isTimeUtc, which is false by default (local time if not present).
   /// \param isTimeUtc whether the given time is in UTC.
   ///      When absent, this is false, and the time is given in local format.
   ///
   /// \return bool Whether the given local or UTC time is within the Daylight Saving Time range.
   ///
   bool IsDST(const MTime& time, bool isTimeUtc = false) const;

   /// The recurring date where time goes from Standard to DST.
   ///
   /// This property does not mean that the timezone does not have any switches,
   /// rather it means there is information on recurring dates in the time zone definition.
   /// Some operating systems, such as Linux or Android, do not offer such information,
   /// and it is recommended to use \ref GetNextSwitchTime.
   ///
   /// Null time will be returned if DST transfer time does not exist in the timezone, this is a convention.
   ///
   /// \seeprop{GetSwitchToStandardTime, SwitchToStandardTime} for the switch to standard from DST time in recurring format.
   /// \see \ref GetNextSwitchTime returns \ref MTime object with the DST switch time that will be occurring in the future.
   ///
   MTimeRecurrentYearly& GetSwitchToDaylightTime();

   /// Constant C++ version of \ref GetSwitchToDaylightTime.
   ///
   const MTimeRecurrentYearly& GetSwitchToDaylightTimeConst() const
   {
      return const_cast<MTimeZone*>(this)->GetSwitchToDaylightTime();
   }

   /// The recurring date where time goes from DST back to normal in a year of this time.
   ///
   /// This property does not mean that the timezone does not have any switches,
   /// rather it means there is information on recurring dates in the time zone definition.
   /// Some operating systems, such as Linux or Android, do not offer such information,
   /// and it is recommended to use \ref GetNextSwitchTime.
   ///
   /// Null time will mean DST transfer time does not exist in the timezone, this is a convention.
   ///
   /// \seeprop{GetSwitchToDaylightTime, SwitchToDaylightTime} for the switch from standard to DST time in recurring format.
   /// \see \ref GetNextSwitchTime returns \ref MTime object with the DST switch time that will be occurring in the future.
   ///
   MTimeRecurrentYearly& GetSwitchToStandardTime();

   /// Constant C++ version of \ref GetSwitchToStandardTime.
   ///
   const MTimeRecurrentYearly& GetSwitchToStandardTimeConst() const
   {
      return const_cast<MTimeZone*>(this)->GetSwitchToStandardTime();
   }

   /// Calculate the time at which the timezone offset will be switched.
   ///
   /// The offset switch can be due to a regular and recurring DST switch,
   /// or it can be changed once at some particular date even without going
   /// on or off DST. It is very possible that the next switch time will appear
   /// in more than a year or two from the anchor date.
   /// For example, for "Russian Standard Time" and an anchor date "2011-04-01",
   /// the next switch time returned is "2014-10-25".
   ///
   /// If there is no offset switch starting from the specified anchor time then 
   /// this method returns null time (IsNull).
   /// Otherwise return the moment at which the offset switch is performed.
   ///
   /// Not all operating systems offer an API to access switch times,
   /// and an algorithm will try to determine the switch time to the precision of one second
   /// by first walking forward by 64-day steps from the anchor and looking at returned local offsets.
   /// When the difference in offsets is found within the 64-day interval,
   /// a binary search for a switch time is performed.
   /// Therefore, the call can be slow, but no slower than one second on Android.
   ///
   /// Different from \refprop{GetSwitchToStandardTime, SwitchToStandardTime} and
   /// \refprop{GetSwitchToDaylightTime, SwitchToDaylightTime},
   /// this API is supported on all operating systems.
   ///
   /// \param anchorTime
   ///     Time, local or UTC, that is the starting time from which to search for the switch time.
   ///     Whether this is a UTC time is determined by isTimeUtc parameter.
   /// \param isTimeUtc
   ///     Whether the anchor time and a return value are in UTC or Local, false by default.
   /// \return
   ///     Time, local or UTC, when the next switch will be performed by the timezone.
   ///     The returned time can be null if there is no next switch time.  
   ///     Whether this is a UTC time is determined by isTimeUtc parameter.
   ///
   /// \seeprop{GetSwitchToStandardTime, SwitchToStandardTime} for the switch from DST to standard in recurring format
   /// \seeprop{GetSwitchToDaylightTime, SwitchToDaylightTime} for the switch from standard to DST time in recurring format
   ///
   MTime GetNextSwitchTime(const MTime& anchorTime, bool isTimeUtc = false) const;

   /// Return the number of seconds added or subtracted around the given switch time.
   ///
   /// \param time
   ///      Switch time coming from \ref GetNextSwitchTime or by any other means.
   ///      Whether this is a UTC or local is determined by isTimeUtc parameter.
   ///      It is not an error to give a time at which no switch is performed,
   ///      in this case the returned value will be zero.
   /// \param isTimeUtc
   ///      Whether the anchor time and a return value are in UTC or Local, false by default.
   /// \return
   ///      Seconds added or subtracted by the current timezone at a given time.
   ///      If the given time is not a switch time then the returned value is zero.
   ///
   int GetSwitchTimeOffsetChange(const MTime& time, bool isTimeUtc = false) const;

   /// Clear all fields of timezone.
   ///
   /// Effectively creates a UTC timezone, however its name will be empty.
   /// The method is a convenience call before creating a custom timezone.
   ///
   void Clear();

   /// Convert the given time from UTC to Local, using this timezone information.
   ///
   /// The standard offset and DST offset are applied if the given time fits in the DST range.
   ///
   /// \param time
   ///      UTC time to convert to Local according to current timezone rules.
   ///      It is an expectation that the given time is a UTC one, while of course this cannot be checked.
   ///      The object shall not be null, or an exception is thrown.
   ///
   /// \see LocalToUtc - Convert local time to UTC.
   /// \see UtcToStandard - Convert UTC time to Standard, without respecting the DST offset and switch time.
   /// \see StandardToUtc - Convert Standard time to UTC, without respecting the DST offset and switch time.
   /// \see LocalToStandard - Convert Local time to Standard by possibly applying a DST shift.
   /// \see StandardToLocal - Convert Standard time to Local by possibly applying a DST shift.
   ///
   MTime UtcToLocal(const MTime& time) const;

   /// Convert the given time from Local to UTC, using this locale.
   ///
   /// The standard offset and DST offset are applied if the given time fits in the DST range.
   ///
   /// Local time ambiguity that takes place when DST is being switched back to Standard
   /// is resolved by this method in a way as if the given local time is on DST.
   ///
   /// Invalid local time that takes place when DST is switched on from Standard
   /// is not reported as error.
   ///
   /// \param time
   ///      Local time to convert to UTC according to current timezone rules.
   ///      It is an expectation that the given time is a local one, while of course this cannot be checked.
   ///      The object shall not be null, or an exception is thrown.
   ///
   /// \see UtcToLocal - Convert UTC time to local.
   /// \see UtcToStandard - Convert UTC time to Standard, without respecting the DST offset and switch time.
   /// \see StandardToUtc - Convert Standard time to UTC, without respecting the DST offset and switch time.
   /// \see LocalToStandard - Convert Local time to Standard by possibly applying a DST shift.
   /// \see StandardToLocal - Convert Standard time to Local by possibly applying a DST shift.
   ///
   MTime LocalToUtc(const MTime& time) const;

   /// Convert the given time from UTC to Standard, without respecting the DST offset and switch time.
   ///
   /// The standard offset is applied. DST has no effect irregardless of whether or not it is defined.
   ///
   /// \param time
   ///      UTC time to convert to Standard according to current timezone rules.
   ///      It is an expectation that the given time is a local one, while of course this cannot be checked.
   ///      The object shall not be null, or an exception is thrown.
   ///
   /// \see UtcToLocal - Convert UTC time to local.
   /// \see LocalToUtc - Convert local time to UTC.
   /// \see StandardToUtc - Convert Standard time to UTC, without respecting the DST offset and switch time.
   /// \see LocalToStandard - Convert Local time to Standard by possibly applying a DST shift.
   /// \see StandardToLocal - Convert Standard time to Local by possibly applying a DST shift.
   ///
   MTime UtcToStandard(const MTime& time) const;

   /// Convert the given time from Standard to UTC, without respecting the DST offset and switch time.
   ///
   /// The standard offset is applied. DST has no effect irregardless of whether or not it is defined.
   ///
   /// \param time
   ///      Standard time to convert to UTC according to current timezone rules.
   ///      It is an expectation that the given time is a standard one, while of course this cannot be checked.
   ///      The object shall not be null, or an exception is thrown.
   ///
   /// \see UtcToLocal - Convert UTC time to local.
   /// \see LocalToUtc - Convert local time to UTC.
   /// \see UtcToStandard - Convert UTC time to Standard, without respecting the DST offset and switch time.
   /// \see StandardToLocal - Convert Standard time to Local by possibly applying a DST shift.
   /// \see LocalToStandard - Convert Local time to Standard by possibly applying a DST shift.
   ///
   MTime StandardToUtc(const MTime& time) const;

   /// Convert the given time from Standard to Local, by possibly applying a DST shift.
   ///
   /// The DST offset is applied only if it is defined, and the given time is in the DST range.
   ///
   /// \param time
   ///      Standard time to convert to local according to current timezone rules.
   ///      It is an expectation that the given time is a standard one, while of course this cannot be checked.
   ///      The object shall not be null, or an exception is thrown.
   ///
   /// \see UtcToLocal - Convert UTC time to local.
   /// \see LocalToUtc - Convert local time to UTC.
   /// \see UtcToStandard - Convert UTC time to Standard, without respecting the DST offset and switch time.
   /// \see StandardToUtc - Convert Standard time to UTC, without respecting the DST offset and switch time.
   /// \see LocalToStandard - Convert Local time to Standard by possibly applying a DST shift.
   ///
   MTime StandardToLocal(const MTime& time) const;

   /// Convert the given time from Local to Standard, by possibly applying a DST shift.
   ///
   /// The DST offset is applied only if it is defined, and the given time is in the DST range.
   ///
   /// \param time
   ///      Local time to convert to Standard according to current timezone rules.
   ///      It is an expectation that the given time is a local one, while of course this cannot be checked.
   ///      The object shall not be null, or an exception is thrown.
   ///
   /// \see UtcToLocal - Convert UTC time to local.
   /// \see LocalToUtc - Convert local time to UTC.
   /// \see UtcToStandard - Convert UTC time to Standard, without respecting the DST offset and switch time.
   /// \see StandardToUtc - Convert Standard time to UTC, without respecting the DST offset and switch time.
   /// \see StandardToLocal - Convert Standard time to Local by possibly applying a DST shift.
   ///
   MTime LocalToStandard(const MTime& time) const;

   /// Sets the current timezone from the current system timezone.
   ///
   /// This call updates this object from the computer's current timezone information.
   ///
   void SetFromCurrentSystem();

   /// Sets the current timezone using the name given.
   ///
   /// The operating system timezone names can be obtained by \refprop{GetAllTimeZoneNames, AllTimeZoneNames}.
   /// Also, the timezone name is not the same as standard timezone name or daylight timezone name as all three are different.
   /// This class does not set the current timezone of the operating system.
   ///
   /// \param name
   ///   The timezone name, as known by the operating system.
   ///   If the name is unknown, an exception is thrown.
   ///
   /// \post \ref InitializedFromDatabase property will be true.
   ///
   void SetByName(const MStdString& name);

   /// Reflection-enabled copy constructor, clone service
   ///
   MTimeZone* NewClone() const
   {
      return M_NEW MTimeZone(*this);
   }

#if defined(M__TIMEZONE_USE_TZ_IMPLEMENTATION) && M__TIMEZONE_USE_TZ_IMPLEMENTATION != 0

   /// Return the full file name where the current timezone file is residing.
   ///
   /// This is typically "/etc/localtime"
   ///
   static const MStdString& GetCurrentTimezoneFilePath();

   /// Return the full directory name where all timezone files are residing.
   ///
   /// This is typically "/usr/share/zoneinfo", "/usr/lib/zoneinfo" or "/usr/local/etc/zoneinfo"
   ///
   static const MStdString& GetZoneInfoDirectoryPath();

#endif

   /// Return a correspondent IANA timezone name from Windows timezone name.
   ///
   /// Windows and IANA timezone names are both locale independent, always in English.
   /// The mapping is hard-coded, and can get outdated in old software versions.
   /// Therefore, it is not guaranteed that the given or returned name
   /// exist in the current operating system.
   /// Empty string is returned if the correspondent name does not exist.
   ///
   /// \param windowsName Windows timezone name such as "Eastern Standard Time".
   /// \return IANA name, a string such as "America/New_York"
   ///         or an empty string if there is no mapping for a given name.
   ///
   static MStdString StandardNameWindowsToIana(const MStdString& windowsName);

   /// Return a correspondent Windows timezone name from IANA timezone name.
   ///
   /// Windows and IANA timezone names are both locale independent, always in English.
   /// The mapping is hard-coded, and can get outdated in old software versions.
   /// Therefore, it is not guaranteed that the given or returned name
   /// exist in the current operating system.
   /// Empty string is returned if the correspondent name does not exist.
   ///
   /// \param ianaName IANA timezone name such as "America/New_York".
   /// \return Windows name, a string such as "Eastern Standard Time"
   ///         or an empty string if there is no mapping for a given name.
   ///
   static MStdString StandardNameIanaToWindows(const MStdString& ianaName);

public: // Semi-private reflected methods
/// \cond SHOW_INTERNAL
#if !M_NO_REFLECTION

   /// True will mean that according to this timezone, the given local time is in DST.
   ///
   /// Note the service takes the local time.
   /// When timezone has DST, and when the switch from Standard to DST occurs,
   /// there is a range of impossible local time values (typically an hour).
   /// By convention, since it is already after the DST switch,
   /// this time range resolves into a time as if DST switch has occurred, and for such time IsDST will return True.
   /// When DST switches into Local, and the clock is moved backwards, there is an ambiguous local time,
   /// and for such time IsDST will again return True.
   ///
   /// \param time
   ///      The local time for which to determine if it fits within DST period.
   ///      If the current time is null, an exception is thrown.
   ///
   /// \return bool Whether the given local time is within the Daylight Saving Time range.
   ///
   bool DoIsDST(const MTime& time) const;

   /// Calculate time at which the time will be switched.
   ///
   /// Notice that the switch time can appear as a result of a DST rule, or
   /// when the timezone moves its offset in an event unrelated to DST.
   ///
   /// If DST is not supported by the timezone, this method returns null time.
   /// Otherwise return the moment at which DST switch is performed.
   /// On the operating systems, such as Android, that do not offer an API to
   /// access switch times, the method will look up UTC offsets for the future dates
   /// up to approximately three years ahead using 64 day intervals.
   /// Notice it is very possible that the next switch time will appear
   /// in more than a year interval from the anchor date -
   /// should some government plan the change in advance.
   ///
   /// Different from \refprop{GetSwitchToStandardTime, SwitchToStandardTime} and
   /// \refprop{GetSwitchToDaylightTime, SwitchToDaylightTime},
   /// this API is supported on all operating systems.
   ///
   /// \param anchorTime Local time from which to start searching for the switch time.
   /// \return Local time when the next switch will be performed by the timezone. It can be null if there is no switch time. 
   ///
   /// \seeprop{GetSwitchToStandardTime, SwitchToStandardTime} for the switch from DST to standard in recurring format.
   /// \seeprop{GetSwitchToDaylightTime, SwitchToDaylightTime} for the switch from standard to DST time in recurring format.
   ///
   MTime DoGetNextSwitchTime1(const MTime& anchorTime) const;

   /// Return the number of seconds added or subtracted around the given switch local time.
   ///
   /// \param time
   ///      Switch local time coming from \ref GetNextSwitchTime or by any other means.
   ///      It is not an error to give a time at which no switch is performed,
   ///      in this case the returned value will be zero.
   /// \return
   ///      Seconds added or subtracted by the current timezone at a given time.
   ///      If the given time is not a switch time then the returned value is zero.
   ///
   int DoGetSwitchTimeOffsetChange1(const MTime& time) const;

#endif // !M_NO_REFLECTION
/// \endcond SHOW_INTERNAL
private: // Methods:

   // Check if the time, whether local or UTF, is at DST
   //
   static bool DoStaticTestIfDST(const MTime& ti, const MTimeRecurrentYearly& switchToDaylight, const MTimeRecurrentYearly& switchToStandard, int standardOffset, int daylightOffset, bool isUtc);
   bool DoSetByName(const MStdString& name);
   void DoComputeRecurringSwitchTimes();
   void DoCalculateRecurrentFromTime(MTimeRecurrentYearly& result, const MTime& time);

#if defined(M__TIMEZONE_USE_WINDOWS_REGISTRY) && M__TIMEZONE_USE_WINDOWS_REGISTRY != 0

   // Set the time zone from the registry access object given.
   //
   // This private setting uses the information about placement of
   // time zone in the system registry.
   //
   // The registry shall be open and valid,
   // otherwise any registry-related exception can be thrown.
   //
   void DoSetByRegistry(MRegistry&);

#elif defined(M__TIMEZONE_USE_ANDROID_IMPLEMENTATION) && M__TIMEZONE_USE_ANDROID_IMPLEMENTATION != 0

   void DoSetFromLocalJavaObject(MJavaEnv& env, jobject zone);

#endif

private: // Data:

   // Dynamic TZ information, if present
   //
   DynamicTimeZone m_dynamic;

   // Standard timezone name
   //
   MStdString m_standardName;

   // Daylight timezone name
   //
   MStdString m_daylightName;

   // Display name
   //
   MStdString m_displayName;

   // Bias in seconds from UTC to standard.
   //
   int m_standardOffset;

   // Daylight bias from standard time in seconds.
   // Most timezones have this value equal to -3600, which is an offset of one hour.
   //
   int m_daylightOffset;

   // Recurring date and time of switch to Daylight saving time.
   // This value can have no effect if the daylight shift is zero.
   //
   MTimeRecurrentYearly m_switchToDaylightTime;

   // Recurring date and time of switch to Standard from Daylight saving time.
   // This value can have no effect if the daylight shift is zero.
   //
   MTimeRecurrentYearly m_switchToStandardTime;

   M_DECLARE_CLASS(TimeZone)
};

#endif
///@}
#endif
