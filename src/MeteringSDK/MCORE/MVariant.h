#ifndef MCORE_MVARIANT_H
#define MCORE_MVARIANT_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MVariant.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MSharedString.h>

#if !M_NO_VARIANT

/// Variant data type, where the particular value type is dynamically determined at runtime.
///
/// The uninitialized variant variable would be of empty type, and should not
/// participate in operations other than the assignment into it, or an exception is thrown.
/// When performing value based operations, the class performs type conversions.
///
class M_CLASS MVariant
{
public: // Types:

   /// Possible value types, sorted in the order of their promotional conversion.
   ///
   enum Type
   {
      VAR_EMPTY,              ///< No value in the variant.
      VAR_BOOL,               ///< Variant has Boolean value.
      VAR_BYTE,               ///< Variant has Byte value.
      VAR_CHAR,               ///< Variant has Character value.
      VAR_UINT,               ///< Variant has unsigned integer value. Its conversion rate is smaller than INT, not like in C.
      VAR_INT,                ///< Variant has integer value.
      VAR_DOUBLE,             ///< Variant has double value.
      VAR_BYTE_STRING,        ///< Variant has the byte string.
      VAR_STRING,             ///< Variant has string value.
      VAR_STRING_COLLECTION,  ///< Variant has string collection value.
      VAR_OBJECT,             ///< Variant has an object.
      VAR_OBJECT_EMBEDDED,    ///< Object, embedded into the variant, can be copied with memory copy operation.
      VAR_VARIANT_COLLECTION, ///< Array of any type of elements, array of variants.
      VAR_MAP,                ///< Map of key:value pairs, both are variants.
      VAR_VARIANT             ///< Variant type by itself. Used externally by applications to denote variant as the whole type.
   };

// SWIG_HIDE private:

   /// Tag that allows telling byte string constructor from the standard string constructor.
   ///
   enum AcceptByteStringType
   {
      ACCEPT_BYTE_STRING
   };

   /// Tag that allows telling byte string constructor from the standard string constructor.
   ///
   enum AcceptByteStringCollectionType
   {
      ACCEPT_BYTE_STRING_COLLECTION
   };

   /// Tag that allows telling string constructor from the byte string constructor.
   ///
   enum AcceptStringType
   {
      ACCEPT_STRING
   };

   /// Tag that allows telling an embedded object from an ordinary object.
   ///
   enum AcceptObjectEmbedded
   {
      ACCEPT_OBJECT_EMBEDDED
   };

   /// Externally visible collection of variants.
   ///
   typedef std::vector<MVariant>
      VariantVector;

   /// Local placeholder class, used for passing small objects by value.
   /// There are only private members here, indeed a placeholder.
   ///
   class ObjectByValue
   {
      // Maximum size of object by value
      //
      static const int MaximumObjectByValueSize = 32;

      // Placeholder field of the maximum object by value size
      //
      Muint64 m_placeholder [ MaximumObjectByValueSize / (M_POINTER_BIT_SIZE == 8 ? sizeof(Muint64) : sizeof(Muint32)) ] M_UNUSED_FIELD;
   };

   /// A hidden type of pointer size.
   ///
   /// Error checking convenience, as it is a pointer, but not an integral type.
   ///
   class PointerBytesType
   {
      char m_dummy[sizeof(void*)];
   };

#ifdef __BORLANDC__
public: // Types and constants: (public due to a bug in Borland, visibility for unnamed types)
#else
private: // Types and constants:
#endif
// SWIG_HIDE private:

   enum BufferType
   {
      BUFFERTYPE_NONE,
      BUFFERTYPE_COPY,
      BUFFERTYPE_REFCOUNT
   };

   enum
   {
      EMBEDDED_BUFFER_SIZE = sizeof(Muint64) // together with zero terminator character
   };

public: // Constructor and destructor:

   /// Default object constructor.
   ///
   MVariant()
   :
      m_type(VAR_EMPTY),
      m_bufferType(BUFFERTYPE_NONE)
   {
   }

   /// Object constructor that creates an empty object of a given type.
   ///
   MVariant(Type type) // SWIG_HIDE
   :
      m_type(VAR_EMPTY),
      m_bufferType(BUFFERTYPE_NONE)
   {
      SetToNull(type);
   }

   /// Construct the value of type bool with the value specified.
   ///
   MVariant(bool n)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(n);
   }

   /// Construct the value of type char with the value specified.
   ///
   MVariant(char c)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(c);
   }

#if !M_NO_WCHAR_T
   /// Construct the value of type char with the value specified.
   ///
   MVariant(wchar_t c) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(c);
   }
#endif

   /// Construct the value of type Muint8 with the value specified.
   ///
   MVariant(Muint8 b)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(b);
   }

   /// Construct the value of type int with the value specified.
   ///
   MVariant(int n)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(n);
   }

   /// Construct the value of type unsigned int with the value specified.
   ///
   MVariant(unsigned n)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(n);
   }

   /// Construct the value of type long with the value specified.
   ///
   MVariant(long n) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(n);
   }

   /// Construct the value of type unsigned long with the value specified.
   ///
   MVariant(unsigned long n) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(n);
   }

   /// Construct the value from 64-bit signed integer.
   ///
   MVariant(Mint64 n)  // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(n);
   }

   /// Construct the value from 64-bit signed integer.
   ///
   MVariant(Muint64 n) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(n);
   }

   /// Construct the value of type double with the value specified.
   ///
   MVariant(double n)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(n);
   }

   ///@{
   /// Construct the value of type string with the value specified
   /// as the constant character pointer.
   ///
   MVariant(MConstChars s) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(s);
   }
   MVariant(MConstChars s, unsigned len, AcceptStringType) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(s, len);
   }
   ///@}

#if !M_NO_WCHAR_T
   ///@{
   /// Construct the value of type string with the value specified
   /// as the constant character pointer.
   ///
   MVariant(const wchar_t* s) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(s);
   }
   MVariant(const wchar_t* s, unsigned len, AcceptStringType = ACCEPT_STRING) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(s, len);
   }
   MVariant(const MWideString& s) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(s);
   }
   ///@}
#endif

   /// Construct the value of type string with the value specified
   /// as MStdString. Note that MByteString might have the same implementation
   /// as MStdString, and it would be not obvious that the constructor from
   /// byte string creates the variant with type VAR_STRING.
   ///
   MVariant(const MStdString& s)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(s);
   }

   ///@{
   /// Construct the value of type byte string,
   /// with the value specified as the constant pointer with length.
   /// Note that this is the way to create a variant of type VAR_BYTE_STRING
   /// with the non-copy constructor. This is because the implementation
   /// with MStdString might be the same as one of MByteString.
   ///
   MVariant(const char* p, unsigned len) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignByteStringToEmpty(p, len);
   }
   MVariant(const Muint8* p, unsigned len)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignByteStringToEmpty((const char*)p, len);
   }
   ///@}

   /// Construct the value of type byte string,
   /// giving the byte string as first parameter.
   /// The second parameter should always be MVariant::ACCEPT_BYTE_STRING.
   /// Note that this is the way to create a variant of type VAR_BYTE_STRING
   /// with the non-copy constructor. This is because the implementation
   /// with MStdString might be the same as one of MByteString.
   ///
   MVariant(const MByteString& s, AcceptByteStringType) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignByteStringToEmpty(s);
   }

   /// Construct the value of type byte string vector,
   /// giving the byte string vector as first parameter.
   /// The second parameter should always be MVariant::ACCEPT_BYTE_STRING_VECTOR.
   /// The internal representation of a byte string vector is a variant collection.
   ///
   MVariant(const MByteStringVector& v, AcceptByteStringCollectionType) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignByteStringCollectionToEmpty(v);
   }

   /// Construct the value of type string collection with the value specified
   /// as parameter.
   ///
   MVariant(const MStdStringVector& v)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(v);
   }

   /// Construct the value of type variant collection with the value specified
   /// as parameter.
   ///
   MVariant(const VariantVector& v) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(v);
   }

   /// Construct the value of type object, which references the object given.
   /// Note that the variant does not own the object,
   /// and it is the responsibility of the application to ensure
   /// that the object is not discarded before its reference.
   ///
   MVariant(MObject* o);

   /// Copy the embedded object to the variant.
   ///
   /// \pre The given embedded object shall be nonzero, or there is a debug check.
   ///
   template
      <class C>
   MVariant(const C* o, AcceptObjectEmbedded) // SWIG_HIDE
   :
      m_type(VAR_EMPTY)
   {
      DoAssignObjectEmbeddedToEmpty(o);
   }

   /// Construct the value from the copy. Initialize the object
   /// with the attributes of the other object.
   ///
   MVariant(const MVariant& v)
   :
      m_type(VAR_EMPTY)
   {
      DoAssignToEmpty(v);
   }

   /// Destroy the variant object, reclaim memory if the value is of type string.
   ///
   ~MVariant() M_NO_THROW
   {
      DoCleanup();
   }

public: // Services that work with type of the variant:

   /// Get the type of the variant object.
   ///
   MVariant::Type GetType() const
   {
      return static_cast<Type>(m_type);
   }

   /// Tells whether the variant is of type VAR_EMPTY, which means it is not initialized
   /// with any value.
   ///
   bool IsEmpty() const
   {
      return m_type == VAR_EMPTY || (m_type == VAR_OBJECT && m_object == NULL);
   }

   /// Whether the variant is of numeric type, so the arithmetic
   /// operations can be performed.
   /// The following types are arithmetic: VAR_BOOL,
   /// VAR_CHAR, VAR_INT, VAR_UINT, and VAR_DOUBLE.
   /// Empty and string types are not numeric.
   ///
   bool IsNumeric() const
   {
      return m_type > VAR_EMPTY && m_type <= VAR_DOUBLE;
   }

   /// Whether the variant can be indexed.
   ///
   bool IsIndexed() const
   {
      return m_type == VAR_BYTE_STRING        ||
             m_type == VAR_STRING             ||
             m_type == VAR_STRING_COLLECTION  ||
             m_type == VAR_VARIANT_COLLECTION ||
             m_type == VAR_MAP;
   }

   /// Whether the variant is a collection, array, or a map (which is a key-indexed collection).
   ///
   bool IsCollection() const
   {
      return m_type == VAR_STRING_COLLECTION  ||
             m_type == VAR_VARIANT_COLLECTION ||
             m_type == VAR_MAP;
   }

   ///@{
   /// Count of elements in the variant, if they can be indexed.
   ///
   /// \pre IsIndexed should be true, or the attempt to use this
   /// operation will cause an exception to be thrown.
   ///
   int GetCount() const;
   void SetCount(int count);
   ///@}

   /// Whether the variant has an object reference.
   ///
   bool IsObject() const
   {
      return m_type == VAR_OBJECT || m_type == VAR_OBJECT_EMBEDDED;
   }

   ///@{
   /// Whether the pointer to variant is an object reference.
   ///
   /// Different from just var->IsObject, this call checks if the variable is not NULL.
   /// The call that mentions reference avoids a problem when newer C++ compilers
   /// assume that object reference is never NULL.
   ///
   static bool StaticIsObject(const MVariant* var); // SWIG_HIDE
   static bool StaticIsObject(const MVariant& var)  // SWIG_HIDE
   {
      return StaticIsObject(&var);
   }
   ///@}

public: // Assignment operators:

   /// Assignment operator that takes variable of type bool.
   ///
   MVariant& operator=(bool b)
   {
      return DoSetInt(b, VAR_BOOL);
   }

   /// Assignment operator that takes variable of type char.
   ///
   MVariant& operator=(char c)
   {
      return DoSetInt((int)(Muint32)(Muint8)c, VAR_CHAR);
   }

#if !M_NO_WCHAR_T
   /// Assignment operator that takes variable of type char.
   ///
   MVariant& operator=(wchar_t c)
   {
      #if (M_OS & M_OS_WINDOWS) != 0
         return DoSetInt((int)(Muint32)(unsigned short)c, VAR_CHAR);
      #else
         if ( sizeof(wchar_t) == 2 )
            return DoSetInt((int)(Muint32)(unsigned short)c, VAR_CHAR);
         else
         {
            M_ASSERT(sizeof(wchar_t) == 4);
            return DoSetInt((int)c, VAR_CHAR);
         }
      #endif
   }
#endif

   /// Assignment operator that takes variable of type byte.
   ///
   MVariant& operator=(Muint8 b)
   {
      return DoSetInt(b, VAR_BYTE);
   }

   /// Assignment operator that takes variable of type int.
   ///
   MVariant& operator=(int n)
   {
      return DoSetInt(n, VAR_INT);
   }

   /// Assignment operator that takes variable of type unsigned int.
   ///
   MVariant& operator=(unsigned n)
   {
      return DoSetInt(n, VAR_UINT);
   }

   /// Assignment operator that takes variable of type long.
   ///
   MVariant& operator=(long n)
   {
      #if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
         M_COMPILED_ASSERT(sizeof(int) == sizeof(long) && sizeof(long) == 4);
         return operator=(static_cast<int>(n));
      #else // true 64-bit platform such as Linux
         M_COMPILED_ASSERT(sizeof(Mint64) == sizeof(long) && sizeof(long) == 8);
         return operator=(static_cast<Mint64>(n));
      #endif
   }

   /// Assignment operator that takes variable of type unsigned long.
   ///
   MVariant& operator=(unsigned long n)
   {
      #if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
         M_COMPILED_ASSERT(sizeof(unsigned) == sizeof(unsigned long) && sizeof(unsigned long) == 4);
         return operator=(static_cast<unsigned>(n));
      #else // true 64-bit platform such as Linux
         M_COMPILED_ASSERT(sizeof(Muint64) == sizeof(unsigned long) && sizeof(unsigned long) == 8);
         return operator=(static_cast<Muint64>(n));
      #endif
   }

   /// Assignment operator that takes variable of 64-bit integer type.
   ///
   MVariant& operator=(Mint64 v);

   /// Assignment operator that takes variable of 64-bit unsigned integer type.
   ///
   MVariant& operator=(Muint64 v);

   /// Assignment operator that takes variable of type double.
   ///
   MVariant& operator=(double f);

   /// Assignment operator that takes variable of type
   /// pointer to the constant zero terminated string.
   ///
   MVariant& operator=(MConstChars p);

   /// Assignment operator that takes variable of type string.
   ///
   MVariant& operator=(const MStdString& s);

   /// Assignment operator that takes variable of type string collection.
   ///
   MVariant& operator=(const MStdStringVector& s);

   /// Assignment operator that takes variable of type variant vector.
   ///
   MVariant& operator=(const VariantVector& s);

   /// Assignment operator that takes variable of type MVariant.
   ///
   MVariant& operator=(const MVariant& v);

   /// Assignment operator that takes ObjectByValue stub, as handled by reflection.
   ///
   MVariant& operator=(const ObjectByValue& o);

   /// Assign the byte string to the variant type.
   ///
   void AssignByteString(const MByteString& v);

   /// Assign the byte string vector to the variant type.
   ///
   void AssignByteStringCollection(const MByteStringVector& v);

   /// Assign the byte string to the variant type.
   ///
   void Assign(const Muint8* p, unsigned len);

   /// Assign the byte string to the variant type.
   ///
   void Assign(const char* p, unsigned len)
   {
      Assign((const Muint8*)p, len);
   }

   /// Assign the string to the variant type.
   ///
   void AssignString(MConstChars p, unsigned len);

   /// Assign the byte embedded object to the variant type.
   ///
   /// \pre The given object value is not null, there is a debug check.
   ///
   template
      <class C>
   void AssignObjectEmbedded(const C* o) // SWIG_HIDE
   {
      M_ASSERT(o->GetEmbeddedSizeof() > 0);
      operator=(o);
   }

   /// Assignment operator that takes variable of type MObject.
   /// Note that the variant does not own the object,
   /// and it is the responsibility of the application to ensure
   /// that the object is not discarded before its reference.
   ///
   MVariant& operator=(const MObject* v);

public: // Conversion services:

   /// Interpret string related values of this variant as zero terminated string.
   ///
   /// The type of the value should only be a string, a byte string, a char, or a byte.
   ///
   /// \pre The conversion should be possible.
   /// If the current value is of incompatible type,
   /// bad conversion is thrown.
   ///
   const char* AsConstChars() const;

   /// Interpret the variant value as type bool, if possible.
   /// The type of the value should allow conversion
   /// of it into boolean. It should either be boolean itself,
   /// or numeric. If the value is numeric, nonzero would mean TRUE.
   /// Also, string, byte string, and string collection, if empty,
   /// will yield to False. If they are not empty, their conversion to
   /// Long will be attempted as the result compared with zero.
   ///
   /// \pre The conversion should be possible.
   /// If the current value is of incompatible type,
   /// bad conversion is thrown.
   ///
   bool AsBool() const;

   /// Interpret the variant value as byte, if possible.
   /// The type of the value should fit within one byte.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases the type is incompatible
   /// or the range is bad.
   ///
   Muint8 AsByte() const;

   /// Interpret the variant value as type MChar, if possible.
   /// The type of the value should allow conversion
   /// of it into MChar. It should either be VAR_CHAR itself,
   /// or it should be numeric with the allowed range,
   /// or it should be a string with the size 1.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible 
   /// or the range is bad.
   ///
   MChar AsChar() const;

   /// Interpret the variant value as double word.
   /// This service is like AsInt or AsUInt, but it will ignore
   /// the sign and never throw an exception or overflow.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible 
   /// or the range is bad.
   ///
   Muint32 AsDWord() const;

   /// Interpret the variant value as integer type, if possible.
   /// The type of the value should allow conversion
   /// of it into integer. The numeric type has to fit within
   /// the range of integer, and the string has to be the valid
   /// string representation of integer.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible 
   /// or the range is bad.
   ///
   int AsInt() const;

   /// Interpret the variant value as unsigned integer type, if possible.
   /// The type of the value should allow conversion
   /// of it into unsigned integer. The numeric type has to fit within
   /// the range of integer, and the string has to be the valid
   /// string representation of unsigned integer.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible 
   /// or the range is bad.
   ///
   unsigned AsUInt() const;

   /// Interpret the variant value as long integer type, if possible.
   /// The type of the value should allow conversion
   /// of it into long integer. The numeric type has to fit within
   /// the range of integer, and the string has to be the valid
   /// string representation of long integer.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible 
   /// or the range is bad.
   ///
   long AsLong() const
   {
      #if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
         M_COMPILED_ASSERT(sizeof(int) == sizeof(long) && sizeof(long) == 4);
         return static_cast<long>(AsInt());
      #else // true 64-bit platform such as Linux
         M_COMPILED_ASSERT(sizeof(Mint64) == sizeof(long) && sizeof(long) == 8);
         return static_cast<long>(AsInt64());
      #endif
   }

   /// Interpret the variant value as unsigned long integer type, if possible.
   /// The type of the value should allow conversion
   /// of it into unsigned long integer. The numeric type has to fit within
   /// the range of integer, and the string has to be the valid
   /// string representation of unsigned long integer.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible 
   /// or the range is bad.
   ///
   unsigned long AsULong() const
   {
      #if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
         M_COMPILED_ASSERT(sizeof(unsigned) == sizeof(unsigned long) && sizeof(unsigned long) == 4);
         return static_cast<unsigned long>(AsUInt());
      #else // true 64-bit platform such as Linux
         M_COMPILED_ASSERT(sizeof(Muint64) == sizeof(unsigned long) && sizeof(unsigned long) == 8);
         return static_cast<unsigned long>(AsUInt64());
      #endif
   }

   /// Interpret the variant value as 64-bit integer type, if possible.
   /// The type of the value should allow conversion
   /// of it into 64-bit integer. The numeric type has to fit within
   /// the range of 64-bit integer, and the string has to be the valid
   /// string representation of such integer.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible 
   /// or the range is bad.
   ///
   Mint64 AsInt64() const;

   /// Interpret the variant value as unsigned long integer type, if possible.
   /// The type of the value should allow conversion
   /// of it into unsigned long integer. The numeric type has to fit within
   /// the range of integer, and the string has to be the valid
   /// string representation of unsigned long integer.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible
   /// or the range is bad.
   ///
   Muint64 AsUInt64() const;

   /// Interpret the variant value as an integer type equivalent to size_t, if possible.
   /// The type of the value should allow conversion
   /// of it into size_t. The numeric type has to fit within
   /// the range of size_t, and the string has to be the valid
   /// string representation of size_t.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible
   /// or the range is bad.
   ///
   size_t AsSizeT() const
   {
      #if M_POINTER_BIT_SIZE == 32 // case of 32-bit platforms
         M_COMPILED_ASSERT(sizeof(unsigned) == sizeof(size_t) && sizeof(size_t) == 4);
         return static_cast<size_t>(AsUInt());
      #else // case of 64-bit platforms
         M_COMPILED_ASSERT(sizeof(Muint64) == sizeof(size_t) && sizeof(size_t) == 8);
         return static_cast<size_t>(AsUInt64());
      #endif
   }

   /// Interpret the variant value as double precision floating point, if possible.
   /// The type of the value should allow conversion
   /// of it into double precision floating point. If this is the string,
   /// the string has to be the valid string representation of double precision number.
   ///
   /// \pre The conversion should be possible.
   /// Bad conversion can be thrown in cases such as the type is incompatible
   /// or the range is bad.
   ///
   double AsDouble() const;

   /// Interpret the variant value as byte string, if possible.
   /// The only value that cannot be interpreted as byte string is an empty value.
   ///
   /// \pre If the value of the variant is not initialized,
   /// the No Value exception is thrown.
   ///
   MByteString AsByteString() const;

   /// Interpret the variant value as string, if possible.
   ///
   /// The only value that cannot be interpreted as string is an empty value.
   /// If the variant is an object, its AsString method is called.
   /// For Boolean value, its string representations are numeric: 1 or 0.
   ///
   /// \pre If the value of the variant is not initialized,
   /// the No Value exception is thrown.
   ///
   MStdString AsString() const;

   /// Interpret the variant value as shared string, if possible.
   /// The only value that cannot be interpreted as shared string is an empty value.
   /// If the variant is an object, its AsString method is called.
   /// For Boolean value, its string representations are numeric: 1 or 0.
   ///
   /// \pre If the value of the variant is not initialized,
   /// the No Value exception is thrown.
   ///
   MSharedString AsSharedString() const; // SWIG_HIDE

   /// Interpret the variant value as string, if possible, using mask that specifies the conversions to make.
   ///
   /// The only value that cannot be interpreted as string is an empty value.
   /// If the variant is an object, its AsString method is called.
   /// For Boolean value, its string representations are numeric: 1 or 0.
   ///
   /// \param mask The mask of type MStr::Mask
   ///
   /// \pre If the value of the variant is not initialized,
   /// the No Value exception is thrown.
   ///
   MStdString AsString(unsigned mask) const;

   /// Interpret the variant value as a string with C escapes, if possible.
   /// The only value that cannot be interpreted as string is an empty value.
   /// If the variant is an object, its AsString method is called, then
   /// converted to escaped string.
   /// For Boolean value, its string representations are numeric: 1 or 0.
   ///
   /// \pre If the value of the variant is not initialized,
   /// the No Value exception is thrown.
   ///
   MStdString AsEscapedString() const;

   /// Interpret the variant value as string collection, if possible.
   /// The string collection returns directly.
   /// The only value that cannot be interpreted as string is an empty value.
   /// For the rest, AsString is attempted and the resulting collection
   /// will have only one string in it.
   ///
   /// \pre If the value of the variant is not initialized,
   /// the No Value exception is thrown.
   ///
   MStdStringVector AsStringCollection() const;

   /// Interpret the variant value as byte string collection, if possible.
   /// The only value that cannot be interpreted as string is an empty value.
   /// For the rest, AsByteString is attempted for each element.
   ///
   /// \pre If the value of the variant is not initialized,
   /// the No Value exception is thrown.
   ///
   MByteStringVector AsByteStringCollection() const;

   /// Interpret the variant value as variant collection, if possible.
   /// The variant collection returns directly.
   /// The only value that cannot be interpreted as variant is an empty value.
   /// For the rest, the resulting collection will have only one element.
   ///
   /// \pre If the value of the variant is not initialized,
   /// the No Value exception is thrown.
   ///
   VariantVector AsVariantCollection() const; // SWIG_HIDE

   ///@{
   /// Interpret the variant value as object reference, if possible.
   /// The only value that can be interpreted as object is an object itself.
   ///
   /// \pre An exception is thrown in case the value is not of type object.
   ///
   MObject* AsObject();
   MObject* AsObject() const // SWIG_HIDE
   {
      return const_cast<MVariant*>(this)->AsObject();
   }
   ///@}

   ///@{
   /// Interpret the variant value as an existing non-NULL object reference, if possible.
   /// The only value that can be interpreted as object is an object itself.
   ///
   /// \pre An exception is thrown in case the value is not of type object.
   /// If the object is NULL, the No Value exception is thrown.
   ///
   MObject* AsExistingObject();
   MObject* AsExistingObject() const // SWIG_HIDE
   {
      return const_cast<MVariant*>(this)->AsExistingObject();
   }
   ///@}

   /// Discard the value of the variant type.
   ///
   void SetEmpty() M_NO_THROW;

   /// Discard the value of the variant type, and if it is an object, delete it.
   /// This corresponds to a concept of owned variant.
   ///
   void SetEmptyWithObjectDelete();

   /// Discard the value of the variant type, and set it to null, 
   /// either empty or zero value depending on the given type.
   ///
   /// The null value will depend on the given type.
   /// For string it is an empty string, and for an integer it is zero.
   /// If no parameter is given, the type is preserved while its value becomes NULL.
   ///
   /// \param type 
   ///     The type to use to set the value to null, where the type 
   ///     determines whether that is an empty or zero value.
   ///
   void SetToNull(MVariant::Type type);

   /// Reserve the number of elements in the variant when variant is indexed.
   ///
   /// \param count
   ///     The number of elements to reserve.
   ///
   /// \pre The variant type should be indexed.
   ///
   void ReserveElements(int count);

   /// Efficiently swap the value with the given value.
   ///
   void Swap(MVariant&) M_NO_THROW;

   /// Fast method that moves the value to another variant, and sets the other variant type to Empty.
   /// This is possible only because variant values can always be moved.
   ///
   void MoveFrom(MVariant& other) M_NO_THROW;

   /// Return this object with the power of the object given.
   /// The power type returned is always DOUBLE.
   ///
   /// \pre The type should be compatible with the power operation.
   /// The compatible types are those that can be cast to DOUBLE.
   /// If not, the bad conversion is thrown. If any of the values
   /// are not initialized, the No Value exception is thrown.
   ///
   MVariant Pow(const MVariant&) const;

   ///@{
   /// Get element by index.
   /// One can check IsIndexed property to know if the variant can be
   /// indexed. Also there is a GetCount, which is callable only for
   /// IsIndexed, and will return the number of items in the variant.
   ///
   /// \pre The type should allow subscripting,
   /// such as VAR_STRING, VAR_BYTE_STRING or VAR_STRING_COLLECTION.
   /// If an item is an object, it shall have a reflected service with the name Item.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant GetItem(const MVariant& index) const;
   MVariant GetItem(int index) const;
   MVariant GetItem(unsigned index) const
   {
      return GetItem(static_cast<int>(index));
   }
#if M_POINTER_BIT_SIZE == 64
   MVariant GetItem(size_t index) const
   {
      return GetItem(static_cast<int>(index));
   }
#endif
   ///@}

   ///@{
   /// Set element by index.
   /// One can check IsIndexed property to know if the variant can be
   /// indexed. Also there is a GetCount, which is callable only for
   /// IsIndexed, and will return the number of items in the variant.
   ///
   /// \pre The type should allow subscripting,
   /// such as VAR_STRING, VAR_BYTE_STRING or VAR_STRING_COLLECTION.
   /// If an item is an object, it shall have a reflected service with the name SetItem.
   /// Otherwise the conversion exception is thrown.
   ///
   void SetItem(const MVariant& index, const MVariant& value);
   void SetItem(int index, const MVariant& value);
   void SetItem(unsigned index, const MVariant& value)
   {
      SetItem(static_cast<int>(index), value);
   }
#if M_POINTER_BIT_SIZE == 64
   void SetItem(size_t index, const MVariant& value)
   {
      SetItem(static_cast<int>(index), value);
   }
#endif
   ///@}

   ///@{
   /// Access constant element by index, efficient call.
   ///
   /// This is the same as GetItem, but it works only for variant collection and a map.
   /// Constant reference is returned, therefore, it is more efficient than GetItem.
   ///
   /// \see GetItem
   ///
   MVariant& AccessItem(const MVariant& index);
   MVariant& AccessItem(int index);
   MVariant& AccessItem(unsigned index)
   {
      return AccessItem(static_cast<int>(index));
   }
   const MVariant& AccessItem(const MVariant& index) const;      // SWIG_HIDE
   const MVariant& AccessItem(int index) const;                  // SWIG_HIDE
   const MVariant& AccessItem(unsigned index) const              // SWIG_HIDE
   {
      return AccessItem(static_cast<int>(index));
   }
#if M_POINTER_BIT_SIZE == 64
   MVariant& AccessItem(size_t index)
   {
      return AccessItem(static_cast<int>(index));
   }
   const MVariant& AccessItem(size_t index) const    // SWIG_HIDE
   {
      return AccessItem(static_cast<int>(index));
   }
#endif
   ///@}

   /// Return a variant vector of keys in this map.
   ///
   /// Keys are unique in the result collection.
   ///
   /// \pre The object should be of map type, or an error is thrown.
   ///
   /// \return Variant of type collection of objects, possibly empty.
   ///
   /// \see GetAllMapValues
   ///
   MVariant GetAllMapKeys() const
   {
      return DoGetAllMapItems(false);
   }

   /// Return a variant vector of values in this map
   ///
   /// Values are not necessarily unique, and therefore the count of values
   /// returned is the same as the count of keys of the same map.
   ///
   /// \pre The object should be of map type, or an error is thrown.
   ///
   /// \return Variant of type collection of objects, possibly empty.
   ///
   /// \see GetAllMapKeys
   ///
   MVariant GetAllMapValues() const
   {
      return DoGetAllMapItems(true);
   }

   /// Return the value of a key in the map by its index.
   ///
   /// \param i
   ///     Index in range 0 .. GetCount.
   ///
   /// \return Key with the given index.
   ///
   const MVariant& GetMapKeyByIndex(int i) const
   {
      return DoGetMapItemByIndex(false, i);
   }

   /// Return the value of a key in the map by its index.
   ///
   /// \param i
   ///     Index in range 0 .. GetCount.
   ///
   /// \return Value with the given index.
   ///
   const MVariant& GetMapValueByIndex(int i) const
   {
      return DoGetMapItemByIndex(true, i);
   }

   /// Swap two indexed items in the array or collection.
   ///
   /// As the result of the operation, this variant will have two items with such indexes swapped.
   /// Index adjustments are performed according to standard rules, such as -1 will mean the last element.
   ///
   /// \param index1
   ///     Index of the first item with which to perform a swap.
   ///
   /// \param index2
   ///     Index of the second item with which to perform a swap.
   ///
   /// \pre The type should allow integer subscripting,
   /// such as VAR_STRING, VAR_BYTE_STRING or VAR_STRING_COLLECTION.
   /// Otherwise the conversion exception is thrown.
   ///
   void SwapItems(int index1, int index2);

   /// Returns true if the given item is in the variant.
   ///
   /// If this variant is a map, this method checks if the given key is present.
   ///
   /// If this variant is a collection of any kind,
   /// true will mean the parameter is contained it in.
   ///
   /// It does not matter how many duplicate items are present in either of the values.
   ///
   bool IsPresent(const MVariant& it) const;

   /// Find index of the given element in the indexed variant.
   ///
   /// If there is no such item, -1 is returned.
   ///
   /// If this variant is a map, this method returns ordinal index of the given key.
   ///
   /// If this variant is a collection of any kind, index of element is returned.
   ///
   int FindIndexOf(const MVariant&, bool reverse = false) const;

   /// Add the given variant as a whole to the collection.
   /// This call is different from operator+=, as it does not
   /// unroll the collection items if the given parameter is a collection.
   ///
   /// \pre The type of the variant shall be VARIANT_COLLECTION,
   /// there is a debug check.
   ///
   void AddToVariantCollection(const MVariant& v);

   /// Adjust a given index 
   /// so the negative index will mean counting from the end of the array.
   ///
   /// \pre The index shall be in range -count to count - 1,
   /// where count is a signed integer, otherwise an exception is thrown.
   ///
   static void AdjustIndex(int& index, unsigned count); // SWIG_HIDE

   /// Adjust a given slice 
   /// so the negative index will mean counting from the end of the array.
   /// A negative slice will mean no elements.
   ///
   /// \pre Slice is always adjusted correctly.
   ///
   static int AdjustSlice(int &from, int &to, unsigned count); // SWIG_HIDE

   /// Return the slice of values for types that support subscripts.
   /// One can check IsIndexed property to know if the variant can be
   /// indexed. Also there is a GetCount, which is callable only for
   /// IsIndexed, and will return the number of items in the variant.
   ///
   /// \pre The type should allow subscripting,
   /// such as VAR_STRING, VAR_BYTE_STRING or VAR_STRING_COLLECTION.
   /// If an item is an object, it shall have a reflected service with the name SetItem.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant GetSlice(int from, int to) const;

   /// Set the slice of values for types that support subscripts.
   /// Shrink or grow the number of items if necessary.
   /// One can check IsIndexed property to know if the variant can be
   /// indexed. Also there is a GetCount, which is callable only for
   /// IsIndexed, and will return the number of items in the variant.
   ///
   /// \pre The type should allow subscripting,
   /// such as VAR_STRING, VAR_BYTE_STRING or VAR_STRING_COLLECTION.
   /// If an item is an object, it shall have a reflected service with the name SetItem.
   /// Otherwise the conversion exception is thrown.
   ///
   void SetSlice(int from, int to, const MVariant& values);

   /// Equality operator. Standard conversions apply.
   ///
   /// \pre If any of the values are not initialized,
   /// the No Value exception is thrown.
   ///
   bool operator==(const MVariant&) const;

   /// Inequality operator. Standard conversions apply.
   ///
   /// \pre If any of the values are not initialized,
   /// the No Value exception is thrown.
   ///
   bool operator!=(const MVariant& other) const
   {
      return !operator==(other);
   }

   /// Less-than operator. Standard conversions apply.
   ///
   /// \pre If any of the values are not initialized,
   /// the No Value exception is thrown.
   ///
   bool operator<(const MVariant&) const;

   /// Greater-than operator. Standard conversions apply.
   ///
   /// \pre If any of the values are not initialized,
   /// the No Value exception is thrown.
   ///
   bool operator>(const MVariant&) const;

   /// Less-or-equal-than operator. Standard conversions apply.
   ///
   /// \pre If any of the values are not initialized,
   /// the No Value exception is thrown.
   ///
   bool operator<=(const MVariant& v) const
   {
      return !operator>(v);
   }

   /// Greater-or-equal-than operator. Standard conversions apply.
   ///
   /// \pre If any of the values are not initialized,
   /// the No Value exception is thrown.
   ///
   bool operator>=(const MVariant& v) const
   {
      return !operator<(v);
   }

   /// Operator OR. Standard conversions apply.
   ///
   /// Note that there is no difference between logical OR and bitwise OR.
   /// Logical OR is applied for bool operands, and the resulting value is bool.
   /// For all the other numeric values the bitwise OR is performed.
   ///
   /// \pre The operand types should be convertible to BOOL or numeric.
   /// Otherwise a bad conversion exception is raised.
   /// If any of the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator|(const MVariant&) const;

   /// Operator AND. Standard conversions apply.
   /// Note that there is no difference between logical AND and bitwise AND.
   /// Logical AND is applied for bool operands, and the resulting value is bool.
   /// For all the other numeric values the bitwise AND is performed.
   ///
   /// \pre The operand types should be convertible to BOOL or numeric.
   /// Otherwise a bad conversion exception is raised.
   /// If any of the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator&(const MVariant&) const;

   /// Operator XOR. Standard conversions apply.
   /// Note that there is no difference between logical XOR and bitwise XOR.
   /// Logical XOR is applied for bool operands, and the resulting value is bool.
   /// For all the other numeric values the bitwise XOR is performed.
   ///
   /// \pre The operand types should be convertible to BOOL or numeric.
   /// Otherwise a bad conversion exception is raised.
   /// If any of the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator^(const MVariant&) const;

   /// Unary operator NOT.
   /// Note that there is no difference between logical NOT and bitwise NOT.
   /// Logical NOT is applied for bool operands, and the resulting value is bool.
   /// For all the other numeric values the bitwise NOT is performed.
   ///
   /// \pre The type should be convertible to either bool
   /// or unsigned integer. Otherwise a bad conversion exception is raised.
   /// If any of the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator!() const;

   /// Unary operator minus.
   /// Please note that the unary operator plus is not defined for MVariant.
   ///
   /// \pre The type should be compatible with the operation.
   /// It should be arithmetic. Otherwise a bad conversion exception is raised.
   /// If any of the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator-() const;

   /// Binary operator plus. The interpretation depends on the context.
   ///
   /// First the conversion is applied.
   /// Preconditions apply for the conversion. If the converted values
   /// are of type TYPE_STRING, then the strings are concatenated.
   /// If the converted values are of the numeric type, the values are added.
   /// The result value is returned, and this object is not changed.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown. If any of
   /// the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator+(const MVariant&) const;

   /// Binary operator minus. Applicable for numerics or sets only.
   ///
   /// First the conversion is applied.
   /// Preconditions apply for the conversion.
   /// The converted values should be of numeric type.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown. If any of
   /// the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator-(const MVariant&) const;

   /// Binary multiplication operator. Applicable for numerics or sets only.
   ///
   /// First the conversion is applied.
   /// Preconditions apply for the conversion.
   /// The converted values should be of numeric type.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown. If any of
   /// the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator*(const MVariant&) const;

   /// Binary division operator. Applicable for numerics only.
   ///
   /// First the conversion is applied.
   /// Preconditions apply for the conversion.
   /// The converted values should be of numeric type.
   ///
   /// \pre The type should be compatible with the operation,
   /// which means to be convertible to numeric type.
   /// Otherwise the conversion exception is thrown. If any of
   /// the values are not initialized, the No Value exception is thrown.
   /// If the second argument is zero, the Division By Zero exception is thrown.
   ///
   MVariant operator/(const MVariant&) const;

   /// Binary modulus operator. Applicable for numerics only.
   /// Modulus produces a reminder value from the division operator if both
   /// arguments are positive.
   ///
   /// First the conversion is applied.
   /// Preconditions apply for the conversion.
   /// The converted values should be of numeric type.
   ///
   /// \pre The type should be compatible with the operation
   /// (be numeric). Otherwise the conversion exception is thrown. If any of
   /// the values are not initialized, the No Value exception is thrown.
   /// If the second argument is zero, the Division By Zero exception is thrown.
   ///
   MVariant operator%(const MVariant&) const;

   /// Bitwise left shift operator. Standard conversions apply.
   ///
   /// \pre The type should be compatible with the operation,
   /// which means to be numeric.
   /// Otherwise the conversion exception is thrown. If any of
   /// the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator<<(const MVariant&) const;

   /// Bitwise right shift operator. Standard conversions apply.
   ///
   /// \pre The type should be compatible with the operation,
   /// which means to be numeric.
   /// Otherwise the conversion exception is thrown. If any of
   /// the values are not initialized, the No Value exception is thrown.
   ///
   MVariant operator>>(const MVariant&) const;

   /// Prefix increment operator.
   /// It never attempts to change the type of this variant.
   ///
   /// \pre The type should allow incrementing,
   /// which means it is numeric. Otherwise the exception is thrown.
   ///
   MVariant& operator++();

   /// Prefix decrement operator.
   /// It never attempts to change the type of this variant.
   ///
   /// \pre The type should allow incrementing,
   /// which means it is numeric. Otherwise the exception is thrown.
   ///
   MVariant& operator--();

   /// Binary operator increment self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator+=(const MVariant&);

   /// Binary operator decrement self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator-=(const MVariant&);

   /// Binary operator multiply self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator*=(const MVariant&);

   /// Binary operator divide self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator/=(const MVariant&);

   /// Binary operator modulus self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator%=(const MVariant&);

   /// Binary operator right shift self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator>>=(const MVariant&);

   /// Binary operator left shift self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator<<=(const MVariant&);

   /// Binary operator, binary or logical 'or' of self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator|=(const MVariant&);

   /// Binary operator, binary or logical 'and' of self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator&=(const MVariant&);

   /// Binary operator, binary or logical 'xor' of self by value.
   ///
   /// \pre The type should be compatible with the operation.
   /// Otherwise the conversion exception is thrown.
   ///
   MVariant& operator^=(const MVariant&);

private: // Services:

   /// Internal assignment service that constructs integer-based variant value
   /// with the particular type given.
   ///
   /// \pre The type has to be of generic integer type. Otherwise the
   /// behavior is undefined.
   ///
   MVariant& DoSetInt(int value, Type type) M_NO_THROW;

public: // Semi-public services that have to be used carefully:
/// \cond SHOW_INTERNAL

   /// Interpret the internals of the variant as bool.
   ///
   /// \pre The type is VAR_BOOL, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   bool DoInterpretAsBool() const
   {
      M_ASSERT(m_type == VAR_BOOL);
      return m_int32 != 0;
   }

   /// Interpret the internals of the variant as byte.
   ///
   /// \pre The type is VAR_BOOL, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   Muint8 DoInterpretAsByte() const
   {
      M_ASSERT(m_type == VAR_BYTE);
      return static_cast<Muint8>(m_uint32);
   }

   /// Interpret the internals of the variant as UINT.
   ///
   /// \pre The type is VAR_UINT, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   MChar DoInterpretAsChar() const
   {
      M_ASSERT(m_type == VAR_CHAR);
      return static_cast<MChar>(m_uint32);
   }

   /// Interpret the internals of the variant as INT.
   ///
   /// \pre The type is VAR_INT, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   int DoInterpretAsInt() const
   {
      M_ASSERT(m_type == VAR_INT);
      return static_cast<int>(m_int32);
   }

   /// Interpret the internals of the variant as UINT.
   ///
   /// \pre The type is VAR_UINT, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   unsigned DoInterpretAsUInt() const
   {
      M_ASSERT(m_type == VAR_UINT);
      return m_uint32;
   }

   /// Interpret the internals of the variant as standard string.
   ///
   /// \pre The type is VAR_STRING, otherwise the behavior
   /// is undefined. The non-UNICODE version has a flexibility that
   /// VAR_BYTE_STRING is also allowed. The debugger version has the assert operator.
   ///
   MStdString DoInterpretAsString() const
   {
      M_ASSERT(m_type == VAR_STRING || m_type == VAR_BYTE_STRING);
      return AsString();
   }

   /// Interpret the internals of the variant as byte string.
   ///
   /// \pre The type is VAR_BYTE_STRING, otherwise the behavior
   /// is undefined. The non-UNICODE version has a flexibility that
   /// VAR_STRING is also allowed. The debugger version has the assert operator.
   ///
   MByteString DoInterpretAsByteString() const
   {
      M_ASSERT(m_type == VAR_BYTE_STRING || m_type == VAR_STRING);
      return AsByteString();
   }

   /// Interpret the internals of the variant as string collection.
   ///
   /// \pre The type is VAR_STRING_COLLECTION, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   MStdStringVector DoInterpretAsStringCollection() const
   {
      M_ASSERT(m_type == VAR_STRING_COLLECTION);
      return AsStringCollection();
   }

   /// Interpret the internals of the variant as variant collection.
   ///
   /// \pre The type is VAR_VARIANT_COLLECTION, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   VariantVector DoInterpretAsVariantCollection() const
   {
      M_ASSERT(m_type == VAR_VARIANT_COLLECTION);
      return AsVariantCollection();
   }

   /// Interpret the internals of the variant as an object.
   ///
   /// \pre The type is VAR_OBJECT, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   MObject* DoInterpretAsObject()
   {
      M_ASSERT(m_type == VAR_OBJECT || m_type == VAR_OBJECT_EMBEDDED);
      if ( m_type == VAR_OBJECT_EMBEDDED )
         DoAccessSharedString().unshare();
      else
      {
         M_ASSERT(m_type == VAR_OBJECT);
      }
      return m_object;
   }

   /// Interpret the internals of the variant as a constant object.
   ///
   /// \pre The type is VAR_OBJECT, otherwise the behavior
   /// is undefined. The debugger version has the assert operator.
   ///
   const MObject* DoInterpretAsObject() const
   {
      M_ASSERT(m_type == VAR_OBJECT || m_type == VAR_OBJECT_EMBEDDED);
      return m_object;
   }

   void DoAssignToEmpty(bool b)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      m_type = VAR_BOOL;
      m_bufferType = BUFFERTYPE_NONE;
      m_int32 = b ? 1 : 0;
   }

   void DoAssignToEmpty(Muint8 b)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      m_type = VAR_BYTE;
      m_bufferType = BUFFERTYPE_NONE;
      m_uint32 = b;
   }

   void DoAssignToEmpty(char c)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      m_type = VAR_CHAR;
      m_bufferType = BUFFERTYPE_NONE;
      m_uint32 = (Muint32)(Muint8)c;
   }

#if !M_NO_WCHAR_T
   void DoAssignToEmpty(wchar_t c) // SWIG_HIDE
   {
      M_ASSERT(m_type == VAR_EMPTY);
      m_type = VAR_UINT;
      m_bufferType = BUFFERTYPE_NONE;
      #if M_WCHAR_T_BIT_SIZE == 16
         m_uint32 = (Muint32)(unsigned short)c;
      #else
         M_ASSERT(sizeof(wchar_t) == 4);
         m_uint32 = (Muint32)c;
      #endif
   }
#endif

   void DoAssignToEmpty(int n)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      m_type = VAR_INT;
      m_bufferType = BUFFERTYPE_NONE;
      m_int32 = n;
   }

   void DoAssignToEmpty(unsigned n)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      m_type = VAR_UINT;
      m_bufferType = BUFFERTYPE_NONE;
      m_uint32 = n;
   }

   void DoAssignToEmpty(long n)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      #if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
         m_type = VAR_INT;
         m_bufferType = BUFFERTYPE_NONE;
         m_int32 = static_cast<Mint32>(n);
      #else // true 64-bit platform such as Linux
         if ( n < INT_MIN || n > INT_MAX )
         {
            m_type = VAR_DOUBLE;
            m_bufferType = BUFFERTYPE_COPY;
            m_double = static_cast<double>(n);
         }
         else
         {
            m_type = VAR_INT;
            m_bufferType = BUFFERTYPE_NONE;
            m_int32 = static_cast<Mint32>(n);
         }
      #endif
   }

   void DoAssignToEmpty(unsigned long n)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      #if INT_MAX == LONG_MAX // case of all Windows or Linux 32-bit
         m_type = VAR_UINT;
         m_bufferType = BUFFERTYPE_NONE;
         m_uint32 = static_cast<Muint32>(n);
      #else // true 64-bit platform such as Linux
         if ( n > UINT_MAX )
         {
            m_type = VAR_DOUBLE;
            m_bufferType = BUFFERTYPE_NONE;
            m_double = static_cast<double>(n);
         }
         else
         {
            m_type = VAR_INT;
            m_bufferType = BUFFERTYPE_NONE;
            m_int32 = static_cast<Mint32>(n);
         }
      #endif
   }

   /// Construct the value from 64-bit signed integer.
   ///
   void DoAssignToEmpty(Mint64 n)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      if ( n < INT_MIN || n > INT_MAX )
      {
         m_type = VAR_DOUBLE;
         m_bufferType = BUFFERTYPE_COPY;
         m_double = static_cast<double>(n);
      }
      else
      {
         m_type = VAR_INT;
         m_bufferType = BUFFERTYPE_NONE;
         m_int32 = static_cast<Mint32>(n);
      }
   }

   /// Construct the value from 64-bit signed integer.
   ///
   void DoAssignToEmpty(Muint64 n)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      if ( n > INT_MAX )
      {
         m_type = VAR_DOUBLE;
         m_bufferType = BUFFERTYPE_COPY;
         m_double = static_cast<double>(n);
      }
      else
      {
         m_type = VAR_UINT;
         m_bufferType = BUFFERTYPE_NONE;
         m_uint32 = static_cast<Muint32>(n);
      }
   }

   void DoAssignToEmpty(double f)
   {
      M_ASSERT(m_type == VAR_EMPTY);
      m_type = VAR_DOUBLE;
      m_bufferType = BUFFERTYPE_COPY;
      m_double = f;
   }

   void DoAssignToEmpty(MConstChars s);

   void DoAssignToEmpty(MConstChars s, unsigned len);

   void DoAssignToEmpty(const MStdString& s);

#if !M_NO_WCHAR_T
   void DoAssignToEmpty(const wchar_t* s)
   {
      DoAssignToEmpty(s, static_cast<unsigned>(wcslen(s)));
   }

   void DoAssignToEmpty(const wchar_t* s, unsigned len)
   {
      DoAssignToEmpty(MToStdString(s, len));
   }

   void DoAssignToEmpty(const MWideString& str)
   {
      DoAssignToEmpty(MToStdString(str));
   }
#endif

   void DoAssignToEmpty(const MStdStringVector& s);

   void DoAssignToEmpty(const VariantVector& s);

   void DoAssignToEmpty(const MVariant& other);

   void DoAssignToEmpty(const ObjectByValue& o)
   {
      DoAssignObjectEmbeddedToEmpty((MObject*)&o);
   }

   void DoAssignByteStringToEmpty(const MByteString& v);

   void DoAssignByteStringToEmpty(const char* bytes, unsigned size);

   void DoAssignByteStringCollectionToEmpty(const MByteStringVector& v);

   void DoAssignObjectEmbeddedToEmpty(const MObject* o);

   void DoAssignToEmpty(MObject* o)
   {
      DoAssignObjectToEmpty(o);
   }

   void DoAssignObjectToEmpty(MObject* o) // call this for all children of MObject
   {
      M_ASSERT(m_type == VAR_EMPTY);
      m_type = VAR_OBJECT;
      m_bufferType = BUFFERTYPE_COPY;
      m_object = o;
   }

   template
       <typename T>
   void DoAssignToEmpty(T t)
   {
      M_COMPILED_ASSERT(sizeof(t) == 123456789); // prevent all inexact calls
   }

/// \endcond SHOW_INTERNAL

public: // Data:

   /// Empty string that is used in many places through the code.
   ///
   static const MStdString s_emptyString;

   /// Empty variant, the same as NULL for pointers
   ///
   static const MVariant s_null;

private: // Methods:

   // Set the type of the variant, private.
   //
   void DoSetType(Type type, BufferType bufferType) M_NO_THROW
   {
      DoCleanup();
      m_type = type;
      m_bufferType = bufferType;
   }

   void DoCleanup() M_NO_THROW;

   void DoCreateSharedBuffer(const char* buff, unsigned size);
   void DoCreateSharedBuffer(unsigned size);
   void DoConvertInternalToSharedBuffer(unsigned reserve) const;

   MVariant& DoAccessVariantItem(int index)
   {
      M_ASSERT(m_type == VAR_STRING_COLLECTION || m_type == VAR_VARIANT_COLLECTION || m_type == VAR_MAP);
      M_ASSERT(index >= 0 && index < int(DoAccessSharedString().size() / sizeof(MVariant))); // check bounds
      return m_variants[index];
   }
   const MVariant& DoAccessVariantItem(int index) const
   {
      M_ASSERT(m_type == VAR_STRING_COLLECTION || m_type == VAR_VARIANT_COLLECTION || m_type == VAR_MAP);
      M_ASSERT(index >= 0 && index < int(DoAccessSharedString().size() / sizeof(MVariant))); // check bounds
      return m_variants[index];
   }

   MSharedString& DoAccessSharedString()
   {
      M_ASSERT(m_bufferType == BUFFERTYPE_REFCOUNT);
      M_ASSERT(m_type == VAR_BYTE_STRING || m_type == VAR_STRING || m_type == VAR_STRING_COLLECTION || m_type == VAR_VARIANT_COLLECTION || m_type == VAR_MAP || m_type == VAR_OBJECT_EMBEDDED);
      return *reinterpret_cast<MSharedString*>(&m_pointerBytes);
   }
   const MSharedString& DoAccessSharedString() const
   {
      M_ASSERT(m_bufferType == BUFFERTYPE_REFCOUNT);
      M_ASSERT(m_type == VAR_BYTE_STRING || m_type == VAR_STRING || m_type == VAR_STRING_COLLECTION || m_type == VAR_VARIANT_COLLECTION || m_type == VAR_MAP || m_type == VAR_OBJECT_EMBEDDED);
      return *reinterpret_cast<const MSharedString*>(&m_pointerBytes);
   }

   void DoMakeCollectionUnique();

   MVariant DoGetAllMapItems(bool returnValues) const;
   const MVariant& DoGetMapItemByIndex(bool returnValues, int i) const;

private: // Data:

   // Type of the current value within variant.
   //
   Type m_type : 8;

   // BufferType constant, one byte
   //
   mutable BufferType m_bufferType : 8;

   // Multipurpose integer or unsigned value, either value or array size
   //
   union
   {
      unsigned m_uint32;
      int      m_int32; // also, count
   };

   // Variable to hold the variant value.
   // It is the first data in the class for making sure it is aligned to the eight byte boundary.
   //
   union
   {
      // Used for storing of object pointer
      //
      MObject* m_object;

      // Used to store all types of shared data
      //
      char* m_bytes;

      // Used for conveniently accessing array of variants
      //
      MVariant* m_variants;

      // Used to store double precision numbers
      //
      double m_double;

      // Placeholder
      //
      Muint64 m_uint64;

      // Placeholder for the pointer
      //
      mutable PointerBytesType m_pointerBytes;

      // Immediate bytes of this value, used for in-place strings
      //
      mutable char m_placeholder [ EMBEDDED_BUFFER_SIZE ];
   };
};

#else // !M_NO_VARIANT

/// Dummy mini-class for convenience
///
class MVariant
{
public:

   /// Empty string that is used in many places through the code.
   ///
   static const MStdString s_emptyString;
};

#endif // !M_NO_VARIANT

///@}

#endif
