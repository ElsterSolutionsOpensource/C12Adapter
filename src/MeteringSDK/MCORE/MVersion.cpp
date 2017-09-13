// File MCORE/MVersion.cpp

#include "MCOREExtern.h"
#include "MVersion.h"
#include "MException.h"
#include "MAlgorithm.h"

#if !M_NO_VARIANT
   static MVersion* DoGetVersionObjectOrNull(const MVariant& var)
   {
      if ( var.GetType() == MVariant::VAR_OBJECT_EMBEDDED )
         return M_DYNAMIC_CAST_WITH_NULL_CHECK(MVersion, var.AsObject());
      return 0;
   }
#endif

   static void DoCheckIfNotReadonly(const MVersion* ver)
   {
      if ( ver->IsReadOnly() )
      {
         MException::Throw(MException::ErrorSoftware, M_CODE_STR(MErrorEnum::CannotModifyConstantOrReadonly, "Cannot modify a readonly object"));
         M_ENSURED_ASSERT(0);
      }
   }

   static void DoCheckVersionIndexInRange(int upper, int value)
   {
      MENumberOutOfRange::CheckInteger(0, upper, value, M_OPT_STR("VersionIndex"));
   }

   static M_NORETURN_FUNC void DoThrowBadVersionNumber(Muint8 flags)
   {
      unsigned count = (flags & MVersion::VersionMaskNumberOfEntries);
      unsigned maxVal = (flags & MVersion::VersionFlagByteEntries) != 0 ? UCHAR_MAX : USHRT_MAX;
      if ( count == 0 )
         MException::Throw(MException::ErrorSoftware, M_CODE_STR_P2(M_ERR_BAD_VERSION_NUMBER_FORMAT_S1, M_I("Version shall consist of up to %d numbers in range 0 to %d, separated by period"), MVersion::VersionMaximumNumberOfEntries, maxVal));
      else
      {
         char format [ 32 ];
         char* c = format;
         for ( --count; count != 0; --count )
         {
            *c++ = '0';
            *c++ = '.';
         }
         *c++ = '0';
         *c   = '\0';
         MException::Throw(MException::ErrorSoftware, M_CODE_STR_P2(M_ERR_BAD_VERSION_NUMBER_FORMAT_S1, M_I("Version shall have format %s where each number is in range 0 to %d"), format, maxVal));
      }
      M_ENSURED_ASSERT(0);
   }

   static Muint16 DoGetVersionEntry(unsigned value, Muint8 flags)
   {
      if ( ((flags & MVersion::VersionFlagByteEntries) != 0 && value > UCHAR_MAX) || value > USHRT_MAX )
      {
         DoThrowBadVersionNumber(flags);
         M_ENSURED_ASSERT(0);
      }
      return (Muint16)value;
   }

   #if !M_NO_REFLECTION

      static MVariant DoNew0()
      {
         MVersion ver;
         return MVariant(&ver, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      /// Construct a new version object from one parameter given.
      ///
      /// The behavior of this constructor depends on the given parameter.
      ///
      /// \param versionOrString
      ///     Parameter from which the version shall be created. Depending on the type the behavior is as follows:
      ///         - When an object is given, it shall be of type MVersion, or a conversion exception is thrown.
      ///           For the case of such MVersion object, the result value of this object will be exactly like one given.
      ///           That includes both the set of subversions (fractions) and attributes such as format.
      ///         - Otherwise an attempt to convert the given parameter into string is made, and such
      ///           string version representation gets assigned to this version.
      ///           In such case the flags and format of this object are not changed, but only the fractions.
      ///           If the format is present, the string shall comprise a correct representation of a version according to format,
      ///           or an exception is thrown. Such behavior allows easy manipulation of
      ///           two-fraction versions as double precision numbers as they can be converted into string
      ///           representation similar that of MVersion.
      ///
      static MVariant DoNew1(const MVariant& versionOrString)
      {
         MVersion ver;
         MVersion* parVer = DoGetVersionObjectOrNull(versionOrString);
         if ( parVer != NULL )
            ver = *parVer;
         else
            ver.SetAsString(versionOrString.AsString());
         return MVariant(&ver, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      /// Construct Version object from two given parameters.
      ///
      /// \param s
      ///     String representation of a version such as "3.1".
      ///     The string should comprise a correct representation of a version,
      ///     such as it shall have numbers separated by periods.
      /// \param readonlyOrFormat
      ///     The behavior depends on parameter type as follows:
      ///       - When this is a boolean, it means the second parameter is a read-only flag.
      ///         If true, the created version object cannot be modified unless the flag is cleared.
      ///       - Otherwise the parameter is interpreted as format string, see
      ///         \ref MVersion_Format "version class description" for details.
      ///
      static MVariant DoNew2(const MVariant& s, const MVariant& readonlyOrFormat)
      {
         MVersion ver;
         if ( readonlyOrFormat.GetType() != MVariant::VAR_BOOL ) // 2nd parameter is possibly a string
            ver = MVersion(s.AsString(), readonlyOrFormat.AsString());
         else
         {
            ver.SetAsString(s.AsString());
            ver.SetIsReadOnly(readonlyOrFormat.AsBool());
         }
         return MVariant(&ver, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }
   #endif

M_START_PROPERTIES(Version)
   M_OBJECT_PROPERTY_INT                (Version, Count)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT(Version, IsEmpty)
   M_OBJECT_PROPERTY_BOOL_EXACT         (Version, IsReadOnly)
   M_OBJECT_PROPERTY_STRING             (Version, Format,   ST_MStdString_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_STRING_EXACT       (Version, AsString, ST_MStdString_X, ST_X_constMStdStringA)
M_START_METHODS(Version)
   M_OBJECT_SERVICE_NAMED           (Version, Assign, AssignVariant,    ST_X_constMVariantA)
   M_OBJECT_SERVICE                 (Version, Item,                     ST_unsigned_X_unsigned)
   M_OBJECT_SERVICE                 (Version, SetItem,                  ST_X_unsigned_unsigned)
   M_OBJECT_SERVICE                 (Version, Matches,                  ST_bool_X_constMVariantA)
   M_OBJECT_SERVICE_OVERLOADED      (Version, Compare, Compare,      2, ST_int_X_constMVariantA_int)
   M_OBJECT_SERVICE_OVERLOADED      (Version, Compare, CompareWhole, 1, ST_int_X_constMVariantA)    // SWIG_HIDE, default second parameter 0
   M_CLASS_FRIEND_SERVICE_OVERLOADED(Version, New,     DoNew2,       2, ST_MVariant_S_constMVariantA_constMVariantA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(Version, New,     DoNew1,       1, ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(Version, New,     DoNew0,       0, ST_MVariant_S)
M_END_CLASS(Version, Object)

MVersion::MVersion()
:
   m_flags(0),
   m_count(0)
{
   m_entries[0] = 0;
   m_entries[1] = 0;
   m_entries[2] = 0;
   m_entries[3] = 0;
   m_entries[4] = 0;
}

MVersion::MVersion(const MStdString& s, bool readonly)
:
   m_flags(0) // sets read-only to false
{
   SetAsString(s);
   SetIsReadOnly(readonly);
}

MVersion::MVersion(const MStdString& s, const MStdString& format)
{
   SetFormat(format);
   if ( IsReadOnly() )
   {
      SetIsReadOnly(false); // otherwise we cannot set it
      SetAsString(s);
      SetIsReadOnly(true);
   }
   else
      SetAsString(s);
}

MVersion::MVersion(const MVersion& other)
:
   m_flags(0),
   m_count(0)
{
   m_entries[0] = 0;
   m_entries[1] = 0;
   m_entries[2] = 0;
   m_entries[3] = 0;
   m_entries[4] = 0; // this pacifies lint

   Assign(other);
}

void MVersion::SetEmpty()
{
   SetCount(0); // this call does all necessary checks
}

void MVersion::Assign(const MVersion& other)
{
   DoCheckIfNotReadonly(this);
   if ( &other != this )
   {
      M_DYNAMIC_CAST_WITH_THROW(MVersion, &other);
      M_ASSERT(GetClass() == MVersion::GetStaticClass());  // verify that this is a final version class
      memcpy(&m_flags, &other.m_flags, sizeof(m_flags) + sizeof(m_count) + sizeof(m_entries)); // we are a mem-copy class!
   }
}

MStdString MVersion::GetFormat() const
{
   MStdString result;
   Muint8 digits = (m_flags & VersionMaskNumberOfEntries);
   if ( digits != 0 )
#ifdef M_USE_USTL
      result.append(1, char(digits + '0'));
#else
      result.assign(1, char(digits + '0'));
#endif
   if ( m_flags & VersionFlagByteEntries )
      result += 'b';
   if ( IsReadOnly() )
      result += 'r';
   return result;
}

   static M_NORETURN_FUNC void DoThrowBadVersionFormat()
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_BAD_VERSION_NUMBER_FORMAT_S1, "Bad version format")); // no need to internationalize
      M_ENSURED_ASSERT(0);
   }

void MVersion::SetFormat(const MStdString& format)
{
   m_flags = 0;
   MStdString::const_iterator it = format.begin();
   MStdString::const_iterator itEnd = format.end();
   for ( ; it != itEnd; ++it )
   {
      char c = *it;
      if ( m_isdigit(c) )
      {
         m_flags &= ~VersionMaskNumberOfEntries;
         Muint8 count = (Muint8)(c - '0');
         if ( count == 0 || count > VersionMaximumNumberOfEntries )
         {
            DoThrowBadVersionFormat();
            M_ENSURED_ASSERT(0);
         }
         m_flags |= count;
         if ( m_count != count )
         {
            for ( int i = static_cast<int>(count) - 1; i >= static_cast<int>(m_count); --i )
               m_entries[i] = 0u;
            m_count = count;
         }
      }
      else
      {
         c = m_toupper(c);
         if ( c == 'B' )
            m_flags |= VersionFlagByteEntries;
         else if ( c == 'R' )
            m_flags |= VersionFlagReadOnly;
         else
         {
            DoThrowBadVersionFormat();
            M_ENSURED_ASSERT(0);
         }
      }
   }
}

void MVersion::SetCount(int count)
{
   DoCheckIfNotReadonly(this);
   unsigned limitCount = (m_flags & VersionMaskNumberOfEntries);
   if ( limitCount == 0 )
      DoCheckVersionIndexInRange(VersionMaximumNumberOfEntries, count);
   else if ( count != (int)limitCount )
      DoThrowBadVersionNumber(m_flags);
   for ( unsigned i = (unsigned)m_count; i < (unsigned)count; ++i )
      m_entries[i] = 0u;
   m_count = count;
}

#if !M_NO_VARIANT
void MVersion::AssignVariant(const MVariant& other)
{
   MVersion* ver = DoGetVersionObjectOrNull(other);
   if ( ver != NULL )
      Assign(*ver);
   else
      SetAsString(other.AsString());
}
#endif // !M_NO_VARIANT

MStdString MVersion::AsString() const
{
   MStdString str;
   int last = (int)m_count - 1; // use "last" instead of "size" due to an "if" below
   for ( int i = 0; i <= last; ++i ) // also use int, not unsigned, because "last" can be -1.
   {
      char buff [ 16 ];
      MToChars((int)m_entries[i], buff);
      str.append(buff);
      if ( i == last )
         break;
      str += '.';
   }
   return str;
}

void MVersion::SetAsString(const MStdString& s)
{
   DoCheckIfNotReadonly(this);

   m_count = 0;
   if ( !s.empty() ) // empty string becomes an empty version
   {
      unsigned numDigits = 0;
      char buff [ 8 ];
      MStdString::const_iterator it = s.begin();
      MStdString::const_iterator itEnd = s.end();
      for ( ; ; ++it )
      {
         if ( it == itEnd || *it == '.' )
         {
            if ( numDigits == 0 || m_count == VersionMaximumNumberOfEntries )
            {
               DoThrowBadVersionNumber(m_flags);
               M_ENSURED_ASSERT(0);
            }
            buff[numDigits] = '\0';
            numDigits = 0;
            unsigned num = MToUnsignedLong(buff);
            m_entries[m_count] = DoGetVersionEntry(num, m_flags);
            ++m_count;
            if ( it == itEnd )
               break;
         }
         else
         {
            char c = *it;
            if ( !m_isdigit(c) || numDigits >= (sizeof(buff) / sizeof(char) - 1) )
            {
               DoThrowBadVersionNumber(m_flags);
               M_ENSURED_ASSERT(0);
            }
            buff[numDigits] = c;
            ++numDigits;
         }
      }
   }
   unsigned entriesLimit = (m_flags & VersionMaskNumberOfEntries);
   if ( entriesLimit != 0u && entriesLimit != (unsigned)m_count )
   {
      DoThrowBadVersionNumber(m_flags);
      M_ENSURED_ASSERT(0);
   }
}

#if !M_NO_VARIANT
bool MVersion::Matches(const MVariant& var) const
{
   MVersion* verObject = DoGetVersionObjectOrNull(var);
   if ( verObject != NULL )
      return MatchesVersion(*verObject);
   MVersion version(var.AsString());
   return MatchesVersion(version);
}
#endif

bool MVersion::MatchesVersion(const MVersion& other) const
{
   unsigned iterationNumber = (m_count < other.m_count) ? m_count : other.m_count;
   for ( unsigned i = 0; i < iterationNumber; ++i )
      if ( m_entries[i] != other.m_entries[i] )
         return false;
   return m_count <= other.m_count;
}

#if !M_NO_VARIANT
int MVersion::Compare(const MVariant& other, int subVersionIndex) const
{
   M_ASSERT(this != NULL); // otherwise the comparison would not be correct

   MVersion* version = DoGetVersionObjectOrNull(other);
   if ( version != NULL )
      return CompareVersion(*version, subVersionIndex);
   if ( other.IsEmpty() )
      return 1; // Any object is greater than nothing, even an empty version
   MVersion otherVersion(other.AsString());
   return CompareVersion(otherVersion, subVersionIndex);
}

int MVersion::CompareWhole(const MVariant& other) const
{
   return Compare(other);
}
#endif // !M_NO_VARIANT

int MVersion::CompareVersion(const MVersion& other, int subVersionIndex) const
{
   unsigned shortest;
   int def;
   if ( m_count < other.m_count )
   {
      shortest = m_count;
      def = -1;
   }
   else if ( m_count > other.m_count )
   {
      shortest = other.m_count;
      def = 1;
   }
   else
   {
      shortest = m_count;
      def = 0;
   }

   if ( subVersionIndex > 0 && subVersionIndex <= int(shortest) ) // It is '<=', not just '<'. Needed to nullify def
   {
      shortest = subVersionIndex;
      def = 0;
   }

   for ( unsigned i = 0u; i < shortest; i++ )
   {
      unsigned m_v = (unsigned)m_entries[i];
      unsigned v = (unsigned)other.m_entries[i];
      if ( m_v > v )
         return 1;
      if ( m_v < v )
         return -1;
   }
   return def;
}

unsigned MVersion::Item(unsigned index) const
{
   DoCheckVersionIndexInRange((int)m_count - 1, (int)index);
   return (unsigned)m_entries[index];
}

void MVersion::SetItem(unsigned index, unsigned value)
{
   DoCheckIfNotReadonly(this);
   DoCheckVersionIndexInRange((int)m_count - 1, (int)index);

   int max = ((m_flags & MVersion::VersionFlagByteEntries) != 0) ? (int)UCHAR_MAX : (int)USHRT_MAX;
   MENumberOutOfRange::CheckInteger(0, max, (int)value, M_OPT_STR("VersionDigit"));
   m_entries[index] = Muint16(value);
}

#if !M_NO_VARIANT
unsigned MVersion::GetEmbeddedSizeof() const
{
   M_COMPILED_ASSERT(sizeof(MVersion) <= sizeof(MVariant::ObjectByValue));
   return sizeof(MVersion);
}
#endif
