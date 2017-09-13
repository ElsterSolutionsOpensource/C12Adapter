// File MCORE/MStr.cpp

#include "MCOREExtern.h"
#include "MStr.h"
#include "MException.h"
#include "MUtilities.h"
#include "MAlgorithm.h"
#include "MCriticalSection.h"
#include "MMath.h"

   #if !M_NO_REFLECTION

      inline bool IsTypeByte(const MVariant& s)
      {
         MVariant::Type t = s.GetType();
         return t == MVariant::VAR_BYTE_STRING || t == MVariant::VAR_BYTE;
      }

      /// Compare two given strings.
      ///
      /// \param s1 First string to compare.
      /// \param s2 Second string to compare.
      /// \param ignoreCase When true, ignore the character case.
      /// \return Zero if strings are equal, positive if the first string is bigger, negative is the second is bigger.
      ///
      static int DoCompare3(const MVariant& s1, const MVariant& s2, bool ignoreCase)
      {
         if ( IsTypeByte(s1) && IsTypeByte(s2) )
            return MStr::Compare(s1.AsByteString(), s2.AsByteString(), ignoreCase);
         return MStr::Compare(s1.AsString(), s2.AsString(), ignoreCase);
      }

      /// Compare two given strings, respecting the letter case.
      ///
      /// \param s1 First string to compare.
      /// \param s2 Second string to compare.
      /// \return Zero if strings are equal, positive if the first string is bigger, negative is the second is bigger.
      ///
      static int DoCompare2(const MVariant& s1, const MVariant& s2)
      {
         return DoCompare3(s1, s2, false);
      }

      /// General ToString method that takes a mask that specifies what exactly needs to be done to a string.
      ///
      /// C-like escapes are used for nonprintables in any case.
      /// Converts the standard string that can contain any character to a string
      /// that is printable, and possibly containing C-like escape characters in
      /// places of non-printable characters.
      ///
      /// \param str Binary string to convert from using mask transformations.
      /// \param mask Mask conversion flags, such as StrNone or StrXML.
      /// \return Result escaped string, text.
      ///
      /// \see FromString for reverse conversion.
      ///
      static MVariant DoToString(const MVariant& s1, unsigned mask)
      {
         if ( IsTypeByte(s1) )
            return MVariant(MStr::ToString(s1.AsByteString(), mask), MVariant::ACCEPT_BYTE_STRING);
         return MStr::ToString(s1.AsString(), mask);
      }

      /// General FromString method that takes a mask that specifies what exactly needs to be done to a string.
      ///
      /// Converts a general purpose escaped string back to value string.
      ///
      /// \param str Text string with escapes to convert into binary form using mask transformations.
      /// \param mask Mask conversion flags, such as StrNone or StrXML.
      /// \return Result binary string.
      ///
      /// \see ToString for reverse conversion.
      ///
      static MVariant DoFromString(const MVariant& s1, unsigned mask)
      {
         if ( IsTypeByte(s1) )
            return MVariant(MStr::FromString(s1.AsByteString(), mask), MVariant::ACCEPT_BYTE_STRING);
         return MStr::FromString(s1.AsString(), mask);
      }

      /// Translate the contents of the string to C escaped string for XML representation.
      ///
      /// This converts '<', '>', '&' and '"' to appropriate XML escape sequences.
      ///
      /// \param str Binary string with escapes to convert into binary.
      /// \return Result binary string, prepared for XML output.
      ///
      /// \see FromXMLString for reverse conversion.
      ///
      static MVariant DoToXMLString(const MVariant& str)
      {
         if ( IsTypeByte(str) )
            return MVariant(MStr::ToXMLString(str.AsByteString()), MVariant::ACCEPT_BYTE_STRING);
         return MStr::ToXMLString(str.AsString());
      }

      /// Convert the XML string with C escapes back into binary.
      ///
      /// \param str Text string with possible C and XML escapes to convert into binary.
      /// \return Result binary string, prepared for XML output.
      ///
      /// \see ToXMLString for reverse conversion.
      ///
      static MVariant DoFromXMLString(const MVariant& str)
      {
         if ( IsTypeByte(str) )
            return MVariant(MStr::FromXMLString(str.AsByteString()), MVariant::ACCEPT_BYTE_STRING);
         return MStr::FromXMLString(str.AsString());
      }

      /// Converts the string that can contain any character to a string with possible C-like escapes.
      ///
      /// The starting and trailing blanks are substituted to their escape sequences,
      /// so they can be seen by the interfaces which do not have them exposed in quotes.
      /// The blanks which are surrounded by the other characters are presented as just blanks.
      ///
      /// \param str Binary string to represent as a text.
      /// \return The result text string.
      ///
      /// \see ToQuotedEscapedString for returning string with escape sequences and enclosed with quotes.
      /// \see FromEscapedString for reverse conversion.
      ///
      static MVariant DoToEscapedString(const MVariant& str)
      {
         if ( IsTypeByte(str) )
            return MVariant(MStr::ToEscapedString(str.AsByteString()), MVariant::ACCEPT_BYTE_STRING);
         return MStr::ToEscapedString(str.AsString());
      }

      /// Convert the text string with possible C-like escapes to binary string.
      ///
      /// \param str String with possible C escapes.
      /// \return Result binary string.
      ///
      /// \see ToEscapedString
      ///
      static MVariant DoFromEscapedString(const MVariant& str)
      {
         if ( IsTypeByte(str) )
            return MVariant(MStr::FromEscapedString(str.AsByteString()), MVariant::ACCEPT_BYTE_STRING);
         return MStr::FromEscapedString(str.AsString());
      }

      /// Acts as ToEscapedString function, plus it puts the result string into quotes.
      ///
      /// \param str Binary string to represent as a quoted text.
      /// \return The result text string enclosed into quotes.
      ///
      /// \see ToEscapedString
      ///
      static MVariant DoToQuotedEscapedString(const MVariant& str)
      {
         if ( IsTypeByte(str) )
            return MVariant(MStr::ToQuotedEscapedString(str.AsByteString()), MVariant::ACCEPT_BYTE_STRING);
         return MStr::ToQuotedEscapedString(str.AsString());
      }

      /// Converts the given string to upper case depending on current system locale.
      ///
      /// \param str String or character to convert.
      /// \return Uppercased string.
      ///
      static MVariant DoToUpper(const MVariant& str)
      {
         MVariant::Type t = str.GetType();
         switch ( t )
         {
         case MVariant::VAR_BYTE:
            return (Muint8)MStr::ToUpper((char)str.AsByte());
         case MVariant::VAR_CHAR:
            return MStr::ToUpper(str.AsChar());
         case MVariant::VAR_BYTE_STRING:
            return MVariant(MStr::ToUpper(str.AsByteString()), MVariant::ACCEPT_BYTE_STRING);
         default:
            return MStr::ToUpper(str.AsString());
         }
      }

      /// Converts the given string to lower case depending on current system locale.
      ///
      /// \param str String or character to convert.
      /// \return Lowercased string.
      ///
      static MVariant DoToLower(const MVariant& str)
      {
         MVariant::Type t = str.GetType();
         switch ( t )
         {
         case MVariant::VAR_BYTE:
            return (Muint8)MStr::ToLower((char)str.AsByte());
         case MVariant::VAR_CHAR:
            return MStr::ToLower(str.AsChar());
         case MVariant::VAR_BYTE_STRING:
            return MVariant(MStr::ToLower(str.AsByteString()), MVariant::ACCEPT_BYTE_STRING);
         default:
            return MStr::ToLower(str.AsString());
         }
      }

      /// Make a multiline word wrapped representation of a given string.
      ///
      /// \param str String, which has to be word wrapped.
      /// \param indentCount How many blanks to use for indentation of the whole string when word wrapping.
      /// \param lineWidth Line width around which the text has to be wrapped, default is 100.
      /// \return The wrapped string.
      ///
      static MVariant DoWordWrap3(const MVariant& str, unsigned indentCount, unsigned lineWidth)
      {
         if ( IsTypeByte(str) )
            return MVariant(MStr::WordWrap(str.AsByteString(), indentCount, lineWidth), MVariant::ACCEPT_BYTE_STRING);
         return MStr::WordWrap(str.AsString(), indentCount, lineWidth);
      }

      /// Make a multiline word wrapped representation of a given string.
      ///
      /// This call uses zero indentation characters and 100 as line width.
      ///
      /// \param str String, which has to be word wrapped.
      /// \return The wrapped string.
      ///
      static MVariant DoWordWrap1(const MVariant& str)
      {
         return DoWordWrap3(str, 0, MStr::DefaultLineWidth);
      }

      #if !M_NO_ENCODING

         /// Decode a byte string in a specific encoding into a UTF-8 string.
         ///
         /// The list of all encodings supported by the library is available
         /// in \refprop{GetAllSupportedEncodings,AllSupportedEncodings} property.
         /// Currently, only one-byte encodings are supported plus UTF-8.
         /// When UTF-8 is specified the method does nothing.
         ///
         /// \param encoding Encoding of the given byte string, see \ref encodingNames for supported names.
         /// \param str Byte string in an encoding given as first parameter.
         /// \param badChar Which character to use in case the character cannot be represented.
         ///         If it is an empty string an error will be thrown if a character that cannot be represented is seen.
         /// \return UTF-8 string.
         ///
         static MStdString DoDecode3(const MStdString& encoding, const MByteString& str, const MStdString& badChar)
         {
            return MStr::Decode(encoding, str, badChar);
         }

         /// Decode a byte string in a specific encoding into a UTF-8 string.
         ///
         /// The list of all encodings supported by the library is available in \ref AllSupportedEncodings property.
         /// Currently, only one-byte encodings are supported plus UTF-8.
         /// When UTF-8 is specified the method does nothing.
         ///
         /// \param encoding Encoding of the given byte string, see \ref encodingNames for supported names.
         /// \param str Byte string in an encoding given as first parameter.
         /// \return UTF-8 string.
         ///
         static MStdString DoDecode2(const MStdString& encoding, const MByteString& str)
         {
            return MStr::Decode(encoding, str);
         }

         /// Encode a string to a byte string that represents a specific encoding.
         ///
         /// \param encoding Desired encoding of the result byte string, see \ref encodingNames for supported names.
         /// \param str String in an encoding given as first parameter.
         /// \param badChar Which character to use in case the character cannot be represented.
         ///         If it is an empty string an error will be thrown if a character that cannot be represented is seen.
         /// \return Byte string in a desired encoding.
         ///
         static MByteString DoEncode3(const MStdString& encoding, const MStdString& str, const MByteString& badChar)
         {
            return MStr::Encode(encoding, str, badChar);
         }

         ///@{
         /// Encode a string to a byte string that represents a specific encoding.
         ///
         /// When this method encounters a character that cannot be represented
         /// in the given encoding, it throws an error.
         ///
         /// \param encoding Desired encoding of the result byte string, see \ref encodingNames for supported names.
         /// \param str String in an encoding given as first parameter.
         /// \return Byte string in a desired encoding.
         ///
         static MByteString DoEncode2(const MStdString& encoding, const MStdString& str)
         {
            return MStr::Encode(encoding, str);
         }

      #endif // !M_NO_ENCODING

   #endif

M_START_PROPERTIES(Str)
   M_CLASS_ENUMERATION_UINT                   (Str, StrNone)
   M_CLASS_ENUMERATION_UINT                   (Str, StrXML)
   M_CLASS_ENUMERATION_UINT                   (Str, StrInternational)
   M_CLASS_ENUMERATION_UINT                   (Str, StrQuote)
   M_CLASS_ENUMERATION_UINT                   (Str, StrKeepSideBlanks)
   M_CLASS_ENUMERATION_UINT                   (Str, StrNoBackslashEscape)
   M_CLASS_ENUMERATION_UINT                   (Str, StrShortEscapes)
#if !M_NO_ENCODING
   M_CLASS_PROPERTY_READONLY_STRING_COLLECTION(Str, AllSupportedEncodings, ST_MStdStringVector_S)
#endif
M_START_METHODS(Str)
   M_CLASS_FRIEND_SERVICE            (Str, ToString,              DoToString,              ST_MVariant_S_constMVariantA_unsigned)
   M_CLASS_FRIEND_SERVICE            (Str, FromString,            DoFromString,            ST_MVariant_S_constMVariantA_unsigned)
   M_CLASS_FRIEND_SERVICE            (Str, ToXMLString,           DoToXMLString,           ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE            (Str, FromXMLString,         DoFromXMLString,         ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE            (Str, ToEscapedString,       DoToEscapedString,       ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE            (Str, FromEscapedString,     DoFromEscapedString,     ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE            (Str, ToQuotedEscapedString, DoToQuotedEscapedString, ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE            (Str, ToUpper,               DoToUpper,               ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE            (Str, ToLower,               DoToLower,               ST_MVariant_S_constMVariantA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Str, Compare,               DoCompare3,  3,          ST_int_S_constMVariantA_constMVariantA_bool)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Str, Compare,               DoCompare2,  2,          ST_int_S_constMVariantA_constMVariantA)          // SWIG_HIDE default parameter is done by Compare
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Str, WordWrap,              DoWordWrap3, 3,          ST_MVariant_S_constMVariantA_unsigned_unsigned)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Str, WordWrap,              DoWordWrap1, 1,          ST_MVariant_S_constMVariantA)                    // SWIG_HIDE default parameter is done by WordWrap
#if !M_NO_ENCODING
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Str, Decode,                DoDecode3,   3,          ST_MStdString_S_constMStdStringA_constMByteStringA_constMStdStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Str, Decode,                DoDecode2,   2,          ST_MStdString_S_constMStdStringA_constMByteStringA) // SWIG_HIDE default parameter is done by Decode
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Str, Encode,                DoEncode3,   3,          ST_MByteString_S_constMStdStringA_constMStdStringA_constMByteStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED (Str, Encode,                DoEncode2,   2,          ST_MByteString_S_constMStdStringA_constMStdStringA) // SWIG_HIDE default parameter is done by Decode
#endif // !M_NO_ENCODING
M_END_CLASS(Str, Object)

   inline void DoCheckCharRange(int c)
   {
      M_COMPILED_ASSERT(SCHAR_MIN < 0); // Watch that the char is a signed type
      if ( c < static_cast<int>(SCHAR_MIN) || c > static_cast<int>(UCHAR_MAX) ) // Check -127 .. 255
      {
         MException::Throw(M_CODE_STR_P1(M_ERR_WIDE_CHARACTER_WITH_CODE_X1_IN_PLACE_WHERE_ONLY_ANSI_ALLOWED, M_I("Wide character with code 0x%X encountered in place where only eight-bit characters allowed"), (unsigned)c));
         M_ENSURED_ASSERT(0);
      }
   }

#if !M_NO_ENCODING

   // avoid including "private/utf8/checked.h" to reduce possibility of a bad use
   #include "private/utf8/core.h"
   #include "private/utf8/unchecked.h"

   // Maximum size of the encoding name, including the trailing zero
   //
   static const size_t s_encodingNameSize = 8;
   M_COMPILED_ASSERT(s_encodingNameSize == sizeof(Muint64));

   // Character that cannot be represented in this one-byte encoding
   // is an invalid unicode value 0xFFFFFFFF
   //
   static const Muint32 s_impossibleChar = static_cast<const Muint32>(0xFFFFFFFF);

   struct MOneByteCodepage
   {
      // Encoding name such as "ASCII", "CP1252" or "8859-5"
      //
      // Binary name is used for fast searching of the encoding in the array.
      // Currently, the implementation is for little endian architectures only
      //
      union
      {
         char m_name [ s_encodingNameSize ];
         Muint64 m_binaryName;
      };

      // First character of one-byte codepage that is not ASCII
      //
      Muint8 m_first;

      // Last character of one-byte codepage, all the rest are invalid
      //
      Muint8 m_last;

      // Map of unicode characters for the range of m_first to m_last character, non ASCII codes
      // The type is not wchar_t because wchar_t is platform dependent
      //
      Muint16* m_map;

      Muint32 DoGetChar32(char c) const
      {
         Muint8 byte = static_cast<Muint8>(c);
         if ( byte < m_first )
            return static_cast<Muint32>(byte);
         if ( byte > m_last )
            return s_impossibleChar;
         size_t i = static_cast<size_t>(byte - m_first);
         Muint32 result = static_cast<Muint32>(m_map[i]);
         if ( result == 0 ) // impossible char in the table
            return s_impossibleChar;
         return result;
      }

      Muint32 DoGetChar8(Muint32 c) const
      {
         if ( c < static_cast<Muint32>(m_first) )
            return static_cast<Muint32>(c);
         int lastIndex = static_cast<int>(m_last) - static_cast<int>(m_first);
         for ( int i = 0; i <= lastIndex; ++i ) // perform signed comparison as size can be negative
            if ( static_cast<Muint32>(m_map[i]) == c )
               return static_cast<Muint32>(i) + m_first; // ordinal number of the character is the character itself
         return s_impossibleChar;
      }
   };

   #include "private/encodings.cxx" // include the file generated by script bin/update_encodings.py

   static M_NORETURN_FUNC void DoThrowBadString(const MStdString& encoding)
   {
      MException::Throw(M_ERR_BAD_STRING_FOR_ENCODING_S1, M_I("Bad string for encoding '%s'"), encoding.c_str());
      M_ENSURED_ASSERT(0);
   }

   // Find codepage descriptor structure that has the given name
   // Throw an exception if such name is not defined
   //
   static const MOneByteCodepage* DoFindCodepage(const MStdString& encoding)
   {
      MStdString::size_type encodingSize = encoding.size();
      if ( encodingSize < s_encodingNameSize )
      {
         MStdString::const_reverse_iterator it = encoding.rbegin();
         MStdString::const_reverse_iterator itEnd = encoding.rend();
         Muint64 encodingBinary = 0;
         for ( ; it < itEnd; ++it )
         {
            encodingBinary <<= 8;
            char c = *it;
            if ( c >= 'a' && c <= 'z' ) // reveal some high performance knowledge about ASCII
               c &= ~0x20u;
            encodingBinary |= static_cast<Muint8>(c);
         }
         for ( size_t i = 0; i < M_NUMBER_OF_ARRAY_ELEMENTS(s_oneByteCodepages); ++i )
         {
            const MOneByteCodepage* page = &s_oneByteCodepages[i];
            if ( page->m_binaryName == encodingBinary )
               return page;
         }
      }
      MException::Throw(MException::ErrorSoftware, MErrorEnum::UnknownItem, "Codepage '%s' is unknown", encoding.c_str());
      M_ENSURED_ASSERT(0);
      return NULL; // pacify compilers
   }

   // Append wide character to a char utf8 string
   //
   inline void DoAppendChar32(MStdString& result, Muint32 c)
   {
      utf8::unchecked::append(c, std::back_inserter(result));
   }

   static bool IsNameUtf8(const MStdString& name)
   {
      if ( name.size() != 5 )  // fast test to sort out most of the names
         return false;
      const char* p = name.data();
      if ( p[4] != '8' )
         return false;
      // fast comparison for "UTF-"
      Muint32 number = MFromLittleEndianUINT32(name.data());
      number &= ~0x00202020; // make the three first letters surely uppercase
      return number == 0x2D465455u; // "UTF-" (byte order is reverse)
   }

MStdStringVector MStr::GetAllSupportedEncodings()
{
   MStdStringVector result;
#if (M_OS & M_OS_WINDOWS) != 0
   result.push_back("ACP");
#endif
   for ( size_t i = 0; i < M_NUMBER_OF_ARRAY_ELEMENTS(s_oneByteCodepages); ++i )
   {
      const MOneByteCodepage* page = &s_oneByteCodepages[i];
      result.push_back(page->m_name);
   }
   result.push_back("UTF-8");
   MAlgorithm::InplaceSort(result, false, true); // unique already, use natural sort
   return result;
}

#if (M_OS & M_OS_WINDOWS) != 0

   static bool IsNameAcp(const MStdString& name)
   {
      if ( name.size() != 3 )  // fast test to sort out most of the names
         return false;
      const char* p = name.c_str();
      // fast comparison for "ACP\0"
      Muint32 number = MFromLittleEndianUINT32(p);
      number &= ~0x00202020; // make the three first letters surely uppercase
      return number == 0x00504341u; // "ACP" (byte order is reverse)
   }

   static void DoCodepageConversion(MByteString& result, const MByteString& encoding, const MByteString& str, const MByteString& badChar, UINT fromCodepage, UINT toCodepage)
   {
      M_ASSERT(result.empty());
      if ( !str.empty() )
      {
         BOOL usedDefaultChar = FALSE;
         int resultWideSize = MultiByteToWideChar(fromCodepage, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
         MESystemError::CheckLastSystemError(resultWideSize <= 0);
         std::wstring wideStr; // use wstring instead of MWideString to avoid dependency on M_NO_WCHAR_T
         wideStr.resize(resultWideSize);
         int resultWideSizeAlt = MultiByteToWideChar(fromCodepage, 0, str.data(), static_cast<int>(str.size()), &(wideStr[0]), resultWideSize);
         MESystemError::CheckLastSystemError(resultWideSizeAlt <= 0);
         M_ASSERT(resultWideSizeAlt == resultWideSize);
         int resultSize;
         if ( toCodepage == CP_UTF8 )
            resultSize = WideCharToMultiByte(toCodepage, 0, wideStr.c_str(), resultWideSize, NULL, 0, NULL, NULL);
         else
            resultSize = WideCharToMultiByte(toCodepage, 0, wideStr.c_str(), resultWideSize, NULL, 0, badChar.c_str(), &usedDefaultChar);
         MESystemError::CheckLastSystemError(resultSize <= 0);
         result.resize(resultSize);
         int resultSizeAlt;
         if ( toCodepage == CP_UTF8 )
            resultSizeAlt = WideCharToMultiByte(toCodepage, 0, wideStr.c_str(), resultWideSize, &(result[0]), resultSize, NULL, NULL);
         else
            resultSizeAlt = WideCharToMultiByte(toCodepage, 0, wideStr.c_str(), resultWideSize, &(result[0]), resultSize, badChar.c_str(), &usedDefaultChar);
         MESystemError::CheckLastSystemError(resultSizeAlt <= 0);
         if ( badChar.empty() && usedDefaultChar )
         {
            DoThrowBadString(encoding);
            M_ENSURED_ASSERT(0);
         }
         M_ASSERT(resultSizeAlt == resultSize);
      }
   }

   inline void DoEncodeAcp(MStdString& result, const MByteString& encoding, const MByteString& str, const MByteString& badChar)
   {
      DoCodepageConversion(result, encoding, str, badChar, CP_UTF8, CP_ACP);
   }

   inline void DoDecodeAcp(MStdString& result, const MByteString& encoding, const MByteString& str, const MByteString& badChar)
   {
      DoCodepageConversion(result, encoding, str, badChar, CP_ACP, CP_UTF8);
   }

#endif // WINDOWS

#endif // !M_NO_ENCODING

#define _M_UNICODE 0
#include "MStr.inc"
#undef _M_UNICODE

#if !M_NO_WCHAR_T

   // Append wide character to an wchar_t string
   //
   inline void DoAppendChar32(MWideString& result, Muint32 c)
   {
      M_ASSERT(utf8::internal::is_code_point_valid(c)); // as we had checked before
      M_ASSERT(c < 0xFFFF); // cannot appear anything bigger from a code table, and even 0xFFFF is invalid
      result += static_cast<wchar_t>(c);
   }

#if (M_OS & M_OS_WINDOWS) != 0

   inline void DoDecodeAcp(MWideString& result, const MByteString&, const MByteString& str, const MWideString&) // making a wide string from ACP never results in an error
   {
      M_ASSERT(result.empty());
      if ( !str.empty() )
      {
         int resultWideSize = MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
         MESystemError::CheckLastSystemError(resultWideSize <= 0);
         result.resize(resultWideSize);
         int resultWideSizeAlt = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &(result[0]), resultWideSize);
         MESystemError::CheckLastSystemError(resultWideSize <= 0);
         M_ASSERT(resultWideSizeAlt == resultWideSize);
      }
   }

   inline void DoEncodeAcp(MStdString& result, const MByteString& encoding, const MWideString& str, const MByteString& badChar)
   {
      M_ASSERT(result.empty());
      if ( !str.empty() )
      {
         BOOL usedDefaultChar = FALSE;
         int resultSize = WideCharToMultiByte(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0, badChar.c_str(), &usedDefaultChar);
         MESystemError::CheckLastSystemError(resultSize <= 0);
         result.resize(resultSize);
         int resultSizeAlt = WideCharToMultiByte(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), &(result[0]), resultSize, badChar.c_str(), &usedDefaultChar);
         MESystemError::CheckLastSystemError(resultSizeAlt <= 0);
         M_ASSERT(resultSizeAlt == resultSize);
         if ( badChar.empty() && usedDefaultChar )
         {
            DoThrowBadString(encoding);
            M_ENSURED_ASSERT(0);
         }
      }
   }

#endif // WINDOWS

#define _M_UNICODE 1
#include "MStr.inc"
#undef _M_UNICODE

#endif

