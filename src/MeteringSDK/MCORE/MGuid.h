#ifndef MCORE_MGUID_H
#define MCORE_MGUID_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MGuid.h

#include <MCORE/MObject.h>

/// Global identifier (GUID) object.
///
/// The class handles Microsoft style GUID objects, as used by Windows platform.
/// The implementation is generic, and it behaves in the same way in non-Microsoft platforms.
///
/// \anchor MGuid_format
/// String representation of GUID class has a fixed format:
/// \code
///    xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
/// \endcode
/// where x is a hexadecimal digit. Only this exact string format is consumed at initialization,
/// and produced by AsString.
///
class M_CLASS MGuid : public MObject
{
public: // Types:

#if (M_OS & M_OS_WINDOWS) == 0
   /// Define Windows-like GUID type on non-Windows platforms
   ///
   /// Its size is 16 bytes, like in Windows, and the definition matches
   ///
   /// \see RawGuidType System independent 16-byte GUID definition
   ///
   struct GUID {
       Muint32 Data1;          ///< First 4 bytes
       Muint16 Data2;          ///< Next 2 bytes
       Muint16 Data3;          ///< Next 2 bytes
       char    Data4[ 8 ];     ///< The rest 8 bytes
   };
#endif

   /// System independent representation of GUID, 16 bytes long randomized byte string
   ///
   union RawGuidType
   {
      Muint8  m_bytes [ 16 ];  ///< Byte array representation, 16 bytes
      char    m_chars [ 16 ];  ///< Char array representation, 16 characters
      Muint32 m_dwords [ 4 ];  ///< Four-byte four-element array
      GUID    m_guid;          ///< Windows GUID representation
   };

public: // Constructors and destructor:

   /// GUID generator constructor, null GUID is generated.
   ///
   /// One has to be sure not to use a null GUID for anything that requires a
   /// cryptographically unique GUID number.
   ///
   MGuid()
   {
      SetToNull();
   }

   /// Construct GUID from a given string.
   ///
   /// \param str
   ///   GUID string in the format as described \ref MGuid_format "here in the class description",
   ///   or an exception is thrown.
   ///
   MGuid(const MStdString& str)
   {
      SetAsString(str);
   }

   /// Copy one GUID from another.
   ///
   /// \param other
   ///    GUID from which value to create this value.
   ///
   MGuid(const MGuid& other)
   {
      Assign(other);
   }

   /// Create GUID object from its internal in-memory representation
   ///
   /// \param guid
   ///    GUID type from which value to create this value.
   ///
   MGuid(const RawGuidType& guid) // SWIG_HIDE
   {
      operator=(guid);
   }

   /// Create GUID object from its internal in-memory representation
   ///
   /// \param guid
   ///    Sixteen bytes that will be copied into this GUID value
   ///
   MGuid(const Muint8* guid) // SWIG_HIDE
   {
      operator=(guid);
   }

   /// Create GUID object from value of Windows specific type
   ///
   /// \param guid
   ///    Initialized GUID value, no checks of any sort is performed
   ///
   MGuid(const GUID& guid) // SWIG_HIDE
   {
      operator=(guid);
   }

   /// Reclaim resources allocated by the object
   ///
   virtual ~MGuid()
   {
   }

public: // Methods:

   /// Generate a new GUID using cryptographically strong random number generation algorithm.
   ///
   static MGuid Generate();

   ///@{
   /// Read-write access of the internal raw GUID binary representation.
   ///
   RawGuidType& AsRawGuid()
   {
      return m_value;
   }
   const RawGuidType& AsRawGuid() const
   {
      return m_value;
   }
   ///@}

   ///@{
   /// Read-write access to Windows GUID type, available only on Windows.
   ///
   /// This is the same as AsRawGuid().m_guid
   ///
   GUID& AsWindowsGuid()
   {
      return m_value.m_guid;
   }
   const GUID& AsWindowsGuid() const
   {
      return m_value.m_guid;
   }
   operator GUID() const
   {
      return m_value.m_guid;
   }
   ///@}

   /// Check whether the GUID is null, not generated.
   ///
   bool IsNull() const;

   /// Check the GUID is not NULL, otherwise throw a No Value exception.
   ///
   void CheckIfNotNull() const;

   /// Reset the value into null, meaning there is no value present in the GUID object.
   ///
   void SetToNull();

   ///@{
   /// Read write property to represent string representation of GUID.
   ///
   /// The string representation of GUID is given \ref MGuid_format "here in the class description".
   /// When assigning to this property, the format shall be exactly as the one specified.
   ///
   MStdString AsString() const;
   void SetAsString(const MStdString& str);
   ///@}

   ///@{
   /// Assign GUID object using value of another GUID object.
   ///
   /// \param guid
   ///    Value to copy from. Null GUID will produce a null GUID.
   ///
   void Assign(const MGuid& guid);
   MGuid& operator=(const MGuid& guid)
   {
      Assign(guid);
      return *this;
   }
   MGuid& operator=(const RawGuidType& guid)
   {
      return operator=(guid.m_bytes);
   }
   MGuid& operator=(const GUID& guid)
   {
      return operator=(reinterpret_cast<const Muint8*>(&guid));
   }
   MGuid& operator=(const Muint8* guid);
   ///@}

   /// Ternary GUID comparison function.
   ///
   /// Partial order comparison of GUID objects, while it does not make much sense semantically,
   /// is the way of building efficient data structures that require presence of less-than or bigger-than operators.
   /// Null GUID is smaller than any other non-null GUID.
   /// Two null GUIDs are equal to each other.
   ///
   /// \param other
   ///    Other GUID object to compare with.
   ///
   /// \return
   ///   - Null is returned if GUIDs have exactly the same values.
   ///   - Value less than zero means the guid is smaller than the other guid given.
   ///   - Value bigger than zero means the guid is bigger than the other guid given.
   ///
   int Compare(const MGuid& other);

   /// Whether the GUID objects have the same value.
   ///
   /// Two null GUIDs are equal to each other.
   ///
   /// \param other
   ///    Other GUID object to compare with.
   ///
   bool operator==(const MGuid& other)
   {
      return Compare(other) == 0;
   }

   /// Whether the GUID objects have different values.
   ///
   /// Two null GUIDs are not equal to each other.
   ///
   /// \param other
   ///    Other GUID object to compare with.
   ///
   bool operator!=(const MGuid& other)
   {
      return Compare(other) != 0;
   }

   /// Whether the GUID object value is less than the given value.
   ///
   /// \param other
   ///    Other GUID object to compare with.
   ///
   bool operator<(const MGuid& other)
   {
      return Compare(other) < 0;
   }

   /// Whether the GUID object value is bigger than the given value.
   ///
   /// \param other
   ///    Other GUID object to compare with.
   ///
   bool operator>(const MGuid& other)
   {
      return Compare(other) > 0;
   }

   /// Whether the GUID object value is less or equal than the given value.
   ///
   /// \param other
   ///    Other GUID object to compare with.
   ///
   bool operator<=(const MGuid& other)
   {
      return Compare(other) <= 0;
   }

   /// Whether the GUID object value is bigger or equal than the given value.
   ///
   /// \param other
   ///    Other GUID object to compare with.
   ///
   bool operator>=(const MGuid& other)
   {
      return Compare(other) >= 0;
   }

#if !M_NO_VARIANT
   /// This is an embedded object type, therefore return its size in bytes.
   ///
   /// \return size of MGuid in bytes.
   ///
   virtual unsigned GetEmbeddedSizeof() const;
#endif

private: // Data:

   RawGuidType m_value;

   M_DECLARE_CLASS(Guid)
};

///@}
#endif
