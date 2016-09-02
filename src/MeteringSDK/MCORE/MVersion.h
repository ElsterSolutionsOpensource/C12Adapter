#ifndef MCORE_MVERSION_H
#define MCORE_MVERSION_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MVersion.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MObject.h>

/// Version that has multiple fraction (subversion) numbers, up to a certain limit.
///
/// This is to represent a version of a software or firmware release.
/// Restrictions apply on what version can be, as the number of fractions cannot exceed five,
/// and each fraction shall be an unsigned number limited to value 65535.
/// Version is a value type object that can be stored within Variant as is.
///
/// Version can be read-only (IsReadOnly is true), which means it cannot be modified by software
/// unless it explicitly changes IsReadOnly into false. This attribute is not a security measure
/// but an easy way of catching software errors.
///
/// \anchor MVersion_Format
/// Version can have a format that defines a number of fractions, their range, and whether the version is read-only.
/// The format is a string consisting of the following case insensitive characters:
///    - b     - (or B) The entries in the version are bytes. If not specified, version entries are two-byte words
///    - r     - (or R) The version is Read-Only - its entries cannot be changed. This is another way of setting IsReadOnly.
///    - [1-5] - Number of entries is constant, as specified by the given digit. If not specified, the number of entries is variable.
///
/// By default, the format is an empty string for read-write version, and "r" for IsReadOnly version.
/// In either case such default format will mean the version can have any number of fractions from 0 to 5,
/// and each fraction can be in range 0 to 65535. Other examples of format string are:
///    - "b2" - version can only have two fractions, each in range 0 .. 255.
///    - "4" - version is exactly four fractions, each in range 0 .. 65535.
///    - "RB3" - version is read-only, exactly three fractions each in range 0 .. 255.
///
class M_CLASS MVersion : public MObject
{
public: // Constants:

   enum
   {
      VersionMaximumNumberOfEntries = 5,    ///< Maximum number of version entries.
   };

/// \cond
   enum
   {
      VersionFlagReadOnly           = 0x10, // Flag: the version is read-only
      VersionFlagByteEntries        = 0x20, // Flag: the version consists of bytes
      VersionMaskNumberOfEntries    = 0x07  // Mask to fetch the constant number of entries in the version
   };
/// \endcond

public: // Constructors and destructor:

   /// Construct an empty version object.
   ///
   /// \post The result object will have no fractions, IsEmpty will be true, format empty,
   ///       IsReadOnly false, and the count of fractions will be zero.
   ///
   MVersion();

   /// Construct Version object from a string representation of version.
   ///
   /// \param s
   ///     String representation of a version such as "3.1".
   ///     The string should comprise of a correct representation of a version,
   ///     such that it shall have numbers separated by periods.
   /// \param readonly
   ///     Whether the result version object will be read-only,
   ///     means the object cannot be changed unless read-only flag is cleared.
   ///
   MVersion(const MStdString& s, bool readonly = false);

   /// Construct Version object from a string.
   ///
   /// \param s
   ///     String representation of a version such as "3.1".
   ///     The string should comprise of a correct representation of a version,
   ///     such that it shall have numbers separated by periods.
   /// \param format
   ///     Version format.
   ///
   MVersion(const MStdString& s, const MStdString& format);

   /// Construct a new version object from a given version object.
   ///
   /// An exact same object is constructed. That includes both the set of subversions
   /// (fractions) and attributes such as format.
   ///
   /// \param other
   ///     Version object from which a copy shall be made.
   ///
   MVersion(const MVersion& other);

   /// Destroy the Version object.
   ///
   /// Following value type object rules, this destructor does nothing.
   ///
   virtual ~MVersion() M_NO_THROW
   {
   }

public: // Operators:

   /// Assignment operator that takes variable of type MVersion.
   ///
   /// The value of this object will be exactly like one given. That includes both the set of subversions
   /// (fractions) and attributes such as format.
   ///
   /// \pre This version shall not be read-only for the method to succeed.
   ///      When the given object is read-only, this one becomes IsReadOnly too.
   ///
   /// \param other
   ///     Version object which whole value is to be copied.
   ///
   MVersion& operator=(const MVersion& other)
   {
      Assign(other);
      return *this;
   }

   /// Assignment operator that takes variable of type MStdString.
   ///
   /// Only the value of the string is copied while the read-only status and format remain
   /// as they were prior to the assignment.
   ///
   /// \pre The version shall not be read-only for the method to succeed.
   ///
   /// \param s
   ///     String representation of a version such as "3.1".
   ///     The string should comprise a correct representation of a version,
   ///     such as it shall have numbers separated by periods.
   ///
   MVersion& operator=(const MStdString& s)
   {
      SetAsString(s);
      return *this;
   }

   /// Equality boolean operator.
   ///
   /// Versions are considered equal if their number of fractions are the same,
   /// and each fraction has the same value. Read-only and format flags do not
   /// participate in comparison.
   ///
   /// \param other
   ///     Other version object with which to make the comparison.
   ///
   /// Examples of true equality (string representation of version is used):
   /// \code
   ///   MVersion("3.2.15.0") = MVersion("3.2.15.0")
   ///   MVersion("15") = MVersion("15")
   ///   MVersion("") = MVersion("")
   /// \endcode
   ///
   /// \ifnot M_NO_VARIANT
   /// \see Compare - generic comparison method.
   /// \endif
   ///
   bool operator==(const MVersion& other) const
   {
      return CompareVersion(other) == 0;
   }

   /// Inequality boolean operator.
   ///
   /// Versions are considered not equal if their number of fractions are different,
   /// or any fraction has a different value as the corresponding fraction in the other version.
   /// Read-only and format flags do not participate in comparison.
   ///
   /// \param other
   ///     Other version object with which to make the comparison.
   ///
   /// Examples of true inequality (string representation of version is used):
   /// \code
   ///   MVersion("3.2.15.0") != MVersion("3.2.15.1")
   ///   MVersion("3.2.15.0") != MVersion("3.2.15")
   ///   MVersion("3.2.15.0") != MVersion("3")
   ///   MVersion("15") != MVersion("20")
   ///   MVersion("") != MVersion("1.2.3")
   /// \endcode
   ///
   /// \see Compare - Generic comparison method.
   /// \see Matches - Comparison method that allows one of the versions to have smaller number of fractions.
   ///
   bool operator!=(const MVersion& other) const
   {
      return !operator==(other);
   }

   /// Less-than boolean operator.
   ///
   /// The comparison of two versions is very similar to string comparison,
   /// except that each fraction is compared, not each character of a string.
   /// The comparison proceeds from left to right, fraction by fraction,
   /// until not matching fractions are found, or one of the versions does not have the corresponding
   /// fraction because its Count is smaller.
   ///
   /// \param other
   ///     Other version object with which to make the comparison.
   ///
   /// Examples of comparison:
   /// \code
   ///   MVersion("3") < MVersion("3.2.15.0")
   ///   MVersion("3.2.8") < MVersion("3.2.15.0")
   ///   MVersion("3.0.0.0") < MVersion("3.2.15.0")
   ///   MVersion("") < MVersion("3.0.0.0")
   ///   MVersion("3.2.15.0") < MVersion("3.3")
   /// \endcode
   ///
   /// \see Compare - Generic comparison method.
   /// \see Matches - Comparison method that allows one of the versions to have smaller number of fractions.
   ///
   bool operator<(const MVersion& other) const
   {
      return CompareVersion(other) < 0;
   }

   /// Greater-than boolean operator.
   ///
   /// The comparison of two versions is very similar to string comparison,
   /// except that each fraction is compared, not each character of a string.
   /// The comparison proceeds from left to right, fraction by fraction,
   /// until not matching fractions are found, or one of the versions does not have the corresponding
   /// fraction because its Count is smaller.
   ///
   /// \param other
   ///     Other version object with which to make the comparison.
   ///
   /// Examples of comparison:
   /// \code
   ///   MVersion("3.2.15.0") > MVersion("3")
   ///   MVersion("3.2.15.0") > MVersion("3.2.8")
   ///   MVersion("3.2.15.0") > MVersion("3.0.0.0")
   ///   MVersion("3.0.0.0") > MVersion("")
   ///   MVersion("3.3") > MVersion("3.2.15.0")
   /// \endcode
   ///
   /// \see Compare - Generic comparison method.
   ///
   bool operator>(const MVersion& other) const
   {
      return CompareVersion(other) > 0;
   }

   /// Less-than or equal-to boolean operator.
   ///
   /// The comparison of two versions is very similar to string comparison,
   /// except that each fraction is compared, not each character of a string.
   /// The comparison proceeds from left to right, fraction by fraction,
   /// until not matching fractions are found, or one of the versions does not have the corresponding
   /// fraction because its Count is smaller.
   ///
   /// \param other
   ///     Other version object with which to make the comparison.
   ///
   /// Examples of comparison:
   /// \code
   ///   MVersion("3") <= MVersion("3.2.15.0")
   ///   MVersion("3.2.8") <= MVersion("3.2.15.0")
   ///   MVersion("3.2.15.0") <= MVersion("3.2.15.0") // because they are equal
   ///   MVersion("") <= MVersion("3.0.0.0")
   ///   MVersion("") <= MVersion("")        // because they are equal
   /// \endcode
   ///
   /// \see Compare - Generic comparison method.
   ///
   bool operator<=(const MVersion& other) const
   {
      return !operator>(other);
   }

   /// Greater-than or equal-to boolean operator.
   ///
   /// The comparison of two versions is very similar to string comparison,
   /// except that each fraction is compared, not each character of a string.
   /// The comparison proceeds from left to right, fraction by fraction,
   /// until not matching fractions are found, or one of the versions does not have the corresponding
   /// fraction because its Count is smaller.
   ///
   /// \param other
   ///     Other version object with which to make the comparison.
   ///
   /// Examples of comparison:
   /// \code
   ///   MVersion("3.2.15.0") >= MVersion("3")
   ///   MVersion("3.2.15.0") >= MVersion("3.2.8")
   ///   MVersion("3.2.15.0") >= MVersion("3.2.15.0") // because they are equal
   ///   MVersion("") >= MVersion("") // because they are equal
   /// \endcode
   ///
   /// \see Compare - Generic comparison method.
   ///
   bool operator>=(const MVersion& other) const
   {
      return !operator<(other);
   }

public: // Public properties:

   ///@{
   /// Indicates if the version object is writable.
   ///
   /// Read-only objects cannot be modified in any way until IsReadOnly is set to false.
   ///
   bool IsReadOnly() const
   {
      return (m_flags & VersionFlagReadOnly) != 0;
   }
   void SetIsReadOnly(bool readonly)
   {
      if ( readonly )
         m_flags |= VersionFlagReadOnly;
      else
         m_flags &= ~VersionFlagReadOnly;
   }
   ///@}

   /// Whether the version number is empty, has no subversions.
   ///
   /// This is a test that returns true if the version has no fractions (subversions).
   /// Flags such as IsReadOnly or Format are not taken into consideration.
   /// A string representation of an empty MVersion object is an empty string, "".
   /// When the version has a fixed format it can never be empty.
   ///
   /// \see SetEmpty - Resetting the version value.
   ///
   bool IsEmpty() const
   {
      return m_count == 0;
   }

   ///@{
   /// Count of subversions (fractions) in version.
   ///
   /// For example, Count = 3 is for version "10.20.30".
   /// If the referenced object IsEmpty then Count returns zero.
   /// When setting Count property, the existing version is either truncated or extended with zeros.
   ///
   /// \possible_values
   ///  - When format does not specify the exact count, this can be 0 to 5.
   ///  - When an exact count is specified by format, this can only be that number specified.
   ///
   /// \return int - the number of fractions present in the version.
   ///
   int GetCount() const
   {
      return (int)m_count;
   }
   void SetCount(int newCount);
   ///@}

   ///@{
   /// String representation of version, accessible as read/write property.
   ///
   /// If Count is zero, then AsString returns an empty string, "".
   /// Not all versions are writable, use IsReadOnly to determine if AsString is writable.
   /// When assigned, the given string should comprise a correct version representation.
   /// If the format is specified, the string should correspond to such format.
   ///
   /// \return String such as "3.1".
   ///
   MStdString AsString() const;
   void SetAsString(const MStdString&);
   ///@}

   ///@{
   /// Version format property.
   ///
   /// The format is described in detail in \ref MVersion_Format "version class description"
   ///
   MStdString GetFormat() const;
   void SetFormat(const MStdString&);
   ///@}

public: // Public methods:

   /// Discard the value of the version, make it empty.
   ///
   /// \pre This version shall not be read-only and the version format shall not be fixed size
   ///      for the method to succeed.
   ///
   /// \post MVersion object will be IsEmpty and AsString will return an empty string after this call.
   ///
   /// \see IsEmpty - emptiness checker.
   ///
   void SetEmpty();

   /// Assignment method.
   ///
   /// The value of this object will be exactly like one given. That includes both the set of subversions
   /// (fractions) and attributes such as format.
   ///
   /// \pre This version shall not be read-only for the method to succeed.
   ///      When the given object is read-only, this one becomes IsReadOnly too.
   ///
   /// \param other
   ///     Version object which whole value is to be copied.
   ///
   void Assign(const MVersion& other);

#if !M_NO_VARIANT

   /// Most generic reflected version of assignment operator.
   ///
   /// The behavior of this method depends on the type of the given parameter.
   ///
   /// \pre This version shall not be read-only for the method to succeed.
   ///      When the given object is read-only, this one becomes IsReadOnly too.
   ///
   /// \param versionOrString
   ///     Parameter from which the assignment shall be made. Depending on the type, the behavior is as follows:
   ///         - When an object is given, it shall be of type MVersion, or a conversion exception is thrown.
   ///           For the case of such MVersion object, the result value of this object will be exactly like one given.
   ///           That includes both the set of subversions (fractions) and attributes such as format.
   ///         - Otherwise an attempt to convert the given parameter into string is made, and such
   ///           string version representation gets assigned to this version.
   ///           In such case the flags and format of this object are not changed, but only the fractions.
   ///           If format is present, the string shall comprise a correct representation of a version according to format,
   ///           or an exception is thrown. Such behavior allows easy manipulation of
   ///           two-fraction versions as double precision numbers as they can be converted into string
   ///           representation similar to one of MVersion.
   ///
   void AssignVariant(const MVariant& versionOrString);
#endif // !M_NO_VARIANT

   ///@{
   /// Check whether this version matches the one given.
   ///
   /// This is a convenience method that determines whether the given version number
   /// is a subset of a more generic major version (5.0.0.123 is a subset of 5.0). 
   /// This method is similar to Compare except that it
   /// behaves differently when the peer versions have a different number of fractions.
   /// If this version has a smaller number of fractions, then true is returned 
   /// when all existing fractions of this version are equal to those in the other version.
   /// Here are the examples:
   /// \code
   ///    MVersion("5.0").Matches("5.0.0.123") is TRUE // version 5.0.0.123 is of the 5.0 kind
   ///    MVersion("4.1").Matches("4.1.1.543") is TRUE // version 4.1.1.543 is of major kind 4.1
   ///    MVersion("5.1").Matches("5.1") is TRUE       // same result as simple comparison
   ///    MVersion("4.1").Matches("4.2.0.0") is FALSE
   /// \endcode
   /// The operation is not commutative and <tt>v1.Matches(v2)</tt> is not necessarily
   /// the same result as <tt>v2.Matches(v1)</tt>. In fact, they will give different results
   /// if and only if v1 and v2 have a different number of fractions. Therefore:
   /// \code
   ///    MVersion("5.0.0.123").Matches("5.0") is FALSE
   ///    MVersion("4.1.1.543").Matches("4.1") is FALSE
   /// \endcode
   ///
   /// \param other
   ///    Version with which this version has to be matched.
   ///
   /// \return bool value to tell if the given version matches this version.
   ///
   /// \ifnot M_NO_VARIANT
   /// \see \ref Compare method that takes a second precision parameter is similar to this service.
   /// \endif
   ///
#if !M_NO_VARIANT
   bool Matches(const MVariant& other) const;
#endif
   bool MatchesVersion(const MVersion& other) const;
   ///@}

   ///@{
   /// Compare version with the given other version object.
   ///
   /// The comparison of two versions is very similar to string comparison,
   /// except that each fraction is compared not each character of a string.
   /// The result of the comparison is an integer value that will be zero for equal versions,
   /// minus one for the case when this version is smaller than the one given,
   /// and one if this version is bigger. The second optional parameter allows for comparison to
   /// ignore less significant fractions, which is convenient for cases when the comparison
   /// has to check major releases.
   ///
   /// \param other
   ///    Version with which this version has to be compared.
   ///
   /// \param precision
   ///    When comparing, whether to take into consideration a limited set of version fractions. The values are:
   ///       - When zero or absent, all version fractions will be compared.
   ///       - When this parameter is 1 to 5 the peer versions will be compared as if prior to comparison
   ///         their counts are truncated to such number. For example, if the precisions
   ///         is limited by two, the version "1.2.5" will be equal to "1.2".
   ///         In other words, version "1.2.5" is the kind of major release "1.2".
   ///
   /// Examples:
   /// \code
   ///   MVersion("3.2.15.0").Compare("3") > 0
   ///   MVersion("3.2.15.0").Compare("3.2.8") > 0
   ///   MVersion("3.0.0.0").Compare("3.2.15.0") < 0
   ///   MVersion("").Compare("") = 0
   ///   MVersion("3.0.0.0").Compare("3.0.123.123", 2) = 0  // compare up to two fractions
   /// \endcode
   ///
   /// \return Integer with the following possible values:
   ///  - 0 is returned if the versions are equal
   ///  - -1, less than zero, means the object is smaller than parameter
   ///  - 1, bigger than zero, the object is bigger than parameter
   ///
   /// \see Matches - comparison method that allows one of the versions to have smaller number of fractions.
   ///
#if !M_NO_VARIANT
   int Compare(const MVariant& other, int precision = 0) const;
#endif
   int CompareVersion(const MVersion& other, int precision = 0) const;
   ///@}

#if !M_NO_VARIANT
   /// \see Compare
   int CompareWhole(const MVariant& other) const;
#endif

   /// Returns the numeric value of the specified zero indexed fraction.
   ///
   /// The numeric value of each sub version can be retrieved using the Item method.
   /// Assuming the version is "10.20.30.40", the following are fractions:
   /// \code
   ///   ver.Item(1) = 10
   ///   ver.Item(2) = 20
   ///   ver.Item(3) = 30
   ///   ver.Item(4) = 40
   /// \endcode
   ///
   /// \param index
   ///    The index of the sub version to retrieve. The valid range is 0 to Count - 1.
   ///
   /// \return unsigned, the value of a fraction
   ///
   unsigned Item(unsigned index) const;

   /// Sets the fraction with the given index to a value specified.
   ///
   /// \param index
   ///    The index of the sub version to set. The valid range is 0 to Count - 1.
   /// \param value
   ///    The value to which the fraction has to be set.
   ///    For byte size format "b" this is 0 to 255. Otherwise the allowed range is 0 to 65535.
   ///
   /// \pre This version shall not be read-only for the method to succeed.
   ///
   void SetItem(unsigned index, unsigned value);

#if !M_NO_VARIANT
   /// Version is an embedded object type, therefore return its size in bytes
   ///
   /// \return Size of MVersion in bytes.
   ///
   virtual unsigned GetEmbeddedSizeof() const;
#endif

private: // Data:

   // Version flags.
   //
   Muint8 m_flags;

   // Number of version entries
   //
   Muint8 m_count;

   // Entries
   //
   Muint16 m_entries [ VersionMaximumNumberOfEntries ];

   M_DECLARE_CLASS(Version)
};

///@}
#endif
