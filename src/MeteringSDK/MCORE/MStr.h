#ifndef MCORE_MSTR_H
#define MCORE_MSTR_H
/// \file MCORE/MStr.h
/// \addtogroup MCORE
///@{

#include <MCORE/MCOREDefs.h>
#include <MCORE/MObject.h>
#include <MCORE/MVariant.h>

/// Provides static methods for string manipulation.  
///
class M_CLASS MStr : public MObject
{
private:

   // Hide constructor and destructor, prevent copying
   MStr();
   MStr(const MStr&);
   ~MStr();

public: // Types:

   /// Enumeration type that defines string transformation.
   ///
   enum Masks
   {
      StrNone              = 0x00, ///< No extra string processing, only C string escapes (backslashes)
      StrXML               = 0x01, ///< Handle string for XML or HTML storage
      StrInternational     = 0x02, ///< Allow non-ASCII characters in string
      StrQuote             = 0x04, ///< Expect or produce quotes around string
      StrKeepSideBlanks    = 0x08, ///< Whether to keep blank characters at the sides, or change them into '\\x20'
      StrNoBackslashEscape = 0x10, ///< For StrXML only, instead of backslashes use XML escapes where possible
      StrShortEscapes      = 0x20  ///< Instead of numerics such as '\\x0A' use standard C escapes '\\n', '\\r', and so on
   };

   /// Constants used in word wrapping algorithm.
   ///
   enum WordWrappingConstants
   {
      MinimumLineWidth = 20,  ///< Minimum line width for word wrapping. A word 20 characters wide is "internationalization".
      DefaultLineWidth = 100  ///< Default line width for word wrapping.
   };

public: // Methods:

   ///@{
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
   static MStdString ToString(const MStdString& str, unsigned mask);
#if !M_NO_WCHAR_T
   static MWideString ToString(const MWideString& str, unsigned mask);
#endif
   ///@}

   ///@{
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
   static MStdString FromString(const MStdString& str, unsigned mask);
#if !M_NO_WCHAR_T
   static MWideString FromString(const MWideString& str, unsigned mask);
#endif
   ///@}

   ///@{
   /// Translate the contents of the string to C escaped string for XML representation.
   ///
   /// This converts '<', '>', '&' and '"' to appropriate XML escape sequences.
   ///
   /// \param str Binary string with escapes to convert into binary.
   /// \return Result binary string, prepared for XML output.
   ///
   /// \see FromXMLString for reverse conversion.
   ///
   static MStdString ToXMLString(const MStdString& str);
#if !M_NO_WCHAR_T
   static MWideString ToXMLString(const MWideString& str);
#endif
   ///@}

   ///@{
   /// Convert the XML string with C escapes back into binary.
   ///
   /// \param str Text string with possible C and XML escapes to convert into binary.
   /// \return Result binary string, prepared for XML output.
   ///
   /// \see ToXMLString for reverse conversion.
   ///
   static MStdString FromXMLString(const MStdString& str);
#if !M_NO_WCHAR_T
   static MWideString FromXMLString(const MWideString& str);
#endif
   ///@}

   ///@{
   /// Convert the string that can contain a C-style string escape sequence into a character. 
   ///
   /// \param str Pointer to string, text that contains a C escape.
   /// \param strEnd Output, result pointer to the end of C escape.
   /// \return Result binary character.
   ///
   /// \pre The first parameter is not NULL, otherwise the behavior is undefined.
   ///
   /// \see FromEscapedString
   /// \see ToEscapedString
   ///
   static char EscapeToChar(const char* str, const char** strEnd = NULL);
#if !M_NO_WCHAR_T
   static wchar_t EscapeToChar(const wchar_t* str, const wchar_t** strEnd = NULL);
#endif
   ///@}

   ///@{
   /// Convert the string that can contains a XML-style string escape sequence into a character.
   ///
   /// \param str Pointer to string, text that contains an XML escape.
   /// \param strEnd Output, result pointer to the end of XML escape.
   /// \return Result binary character.
   ///
   /// \pre The first parameter is not NULL, otherwise the behavior is undefined.
   ///
   static char XMLEscapeToChar(const char* str, const char** strEnd = NULL);
#if !M_NO_WCHAR_T
   static wchar_t XMLEscapeToChar(const wchar_t* str, const wchar_t** strEnd = NULL);
#endif
   ///@}

   ///@{
   /// Acts as ToEscapedString function, plus it puts the result string into quotes.
   ///
   /// \param str Binary string to represent as a quoted text.
   /// \return The result text string enclosed into quotes.
   ///
   /// \see ToEscapedString
   ///
   static MStdString  ToQuotedEscapedString(const MStdString& str);
#if !M_NO_WCHAR_T
   static MWideString ToQuotedEscapedString(const MWideString& str);
#endif
   ///@}

   ///@{
   /// Converts the string that can contain any character to a string with possible C-like escapes.
   ///
   /// The starting and trailing blanks are substituted to their escape sequences,
   /// so they can be seen by the interfaces which do not have them exposed in quotes.
   /// The blanks which are surrounded by the other characters are presented as just blanks.
   ///
   /// \param str Binary string to represent as a text.
   /// \return The result text string.
   ///
   /// \see ToQuotedEscapedString
   /// \see FromEscapedString
   ///
   static MStdString  ToEscapedString(const MStdString& str);
#if !M_NO_WCHAR_T
   static MWideString ToEscapedString(const MWideString& str);
#endif
   ///@}

   ///@{
   /// Acts as ToEscapedString function, plus converts XML service symbols to escape sequences.
   ///
   /// \param str Binary string to represent as a text.
   /// \return The result text string, with possible C and XML escapes.
   ///
   /// \see ToEscapedString
   ///
   static MStdString  ToEscapedXmlString(const MStdString& str);
#if !M_NO_WCHAR_T
   static MWideString ToEscapedXmlString(const MWideString& str);
#endif
   ///@}

   ///@{
   /// Convert any character to a printable string, possibly a C-like escape character.
   ///
   /// If the character is a blank, it is represented as "\\x20".
   ///
   /// \param ch Character to represent.
   /// \return Result string.
   ///
   static MStdString  CharToEscapedString(char ch);
#if !M_NO_WCHAR_T
   static MWideString CharToEscapedString(wchar_t ch);
#endif
   ///@}

   ///@{
   /// Convert any character to a quoted printable string, possibly a C-like escape character.
   ///
   /// If the character is a blank, it is represented as "'\\x20'".
   /// Apostrophe is used for quoting a character.
   ///
   /// \param ch Character to represent.
   /// \return Result quoted string.
   ///
   static MStdString  CharToQuotedEscapedString(char ch);
#if !M_NO_WCHAR_T
   static MWideString CharToQuotedEscapedString(wchar_t ch);
#endif
   ///@}

   ///@{
   /// Convert the text string with possible C-like escapes to binary string.
   ///
   /// \param str String with possible C escapes.
   /// \return Result binary string.
   ///
   /// \see ToEscapedString
   ///
   static MStdString  FromEscapedString(const MStdString& str);
#if !M_NO_WCHAR_T
   static MWideString FromEscapedString(const MWideString& str);
#endif
   ///@}

   ///@{
   /// Converts the given string to upper case depending on current system locale.
   ///
   /// \param par String or character to convert.
   /// \return Uppercased strings.
   ///
   static MStdString  ToUpper(const MStdString& par);
   static char        ToUpper(char par);
#if !M_NO_WCHAR_T
   static MWideString ToUpper(const MWideString& par);
   static wchar_t     ToUpper(wchar_t par);
#endif
   ///@}

   ///@{
   /// Converts the given string to lower case depending on current system locale.
   ///
   /// \param par String or character to convert.
   /// \return Lowercased string or character.
   ///
   static MStdString  ToLower(const MStdString& par);
   static char        ToLower(char par);
#if !M_NO_WCHAR_T
   static MWideString ToLower(const MWideString& par);
   static wchar_t     ToLower(wchar_t par);
#endif
   ///@}

   ///@{
   /// Make a multiline word wrapped representation of a given string.
   ///
   /// \param str String, which has to be word wrapped.
   /// \param indentCount How many blanks to use for indentation of the whole string when word wrapping.
   /// \param lineWidth Line width around which the text has to be wrapped, default is 100.
   /// \return The wrapped string.
   ///
   static MStdString WordWrap(const MStdString& str, unsigned indentCount = 0, unsigned lineWidth = DefaultLineWidth);
#if !M_NO_WCHAR_T
   static MWideString WordWrap(const MWideString& str, unsigned indentCount = 0, unsigned lineWidth = DefaultLineWidth);
#endif
   ///@}

   ///@{
   /// Compare two given strings.
   ///
   /// \param s1 First string to compare.
   /// \param s2 Second string to compare.
   /// \param ignoreCase When true, ignore the character case, default value is false.
   /// \return Zero if strings are equal, positive if the first string is bigger, negative is the second is bigger.
   ///
   static int Compare(const MStdString& s1, const MStdString& s2, bool ignoreCase = false);
#if !M_NO_WCHAR_T
   static int Compare(const MWideString& s1, const MWideString& s2, bool ignoreCase = false);
#endif
   ///@}

#if !M_NO_ENCODING

   /// Decode a byte string in a specific encoding into a UTF-8 string.
   ///
   /// The list of all encodings supported by the library is available in
   /// \refprop{GetAllSupportedEncodings,AllSupportedEncodings} property.
   /// Currently, only one-byte encodings are supported plus UTF-8.
   /// When UTF-8 is specified the method does nothing.
   ///
   /// \param encoding Encoding of the given byte string, see \ref encodingNames for supported names.
   /// \param str Byte string in an encoding given as first parameter.
   /// \param badChar Which character to use in case the character cannot be represented.
   ///         If it is an empty string an error will be thrown if a character that cannot be represented is seen.
   /// \return UTF-8 string.
   ///
   static MStdString Decode(const MStdString& encoding, const MByteString& str, const MStdString& badChar = MVariant::s_emptyString);

#if !M_NO_WCHAR_T
   /// Decode a byte string in a specific encoding into a Unicode string.
   ///
   /// The list of all encodings supported by the library is available in
   /// \refprop{GetAllSupportedEncodings,AllSupportedEncodings} property.
   /// Currently, only one-byte encodings are supported plus UTF-8.
   /// When UTF-8 is specified the method does nothing.
   ///
   /// \param encoding Encoding of the given byte string, see \ref encodingNames for supported names.
   /// \param str Byte string in an encoding given as first parameter.
   /// \param badChar Which character to use in case the character cannot be represented.
   ///         If it is an empty string an error will be thrown if a character that cannot be represented is seen.
   /// \return Unicode wide string, UTF-16 on Windows, or UTF-32 on Posix compatible OS such as Linux.
   ///
   static MWideString Decode(const MStdString& encoding, const MByteString& str, const MWideString& badChar);
#endif

   ///@{
   /// Encode a string to a byte string that represents a specific encoding.
   ///
   /// \param encoding Desired encoding of the result byte string, see \ref encodingNames for supported names.
   /// \param str String in UTF-8.
   /// \param badChar Which character to use in case the character cannot be represented.
   ///         If it is an empty string an error will be thrown if a character that cannot be represented is seen.
   /// \return Byte string in a desired encoding.
   ///
   static MByteString Encode(const MStdString& encoding, const MStdString& str, const MByteString& badChar = MVariant::s_emptyString);
#if !M_NO_WCHAR_T
   static MByteString Encode(const MStdString& encoding, const MWideString& str, const MByteString& badChar);
#endif
   ///@}
   /// Return all encodings supported.
   ///
   /// \anchor encodingNames
   /// Names consist of case insensitive English letters, digits and dash ('-').
   /// The names returned by this method are always uppercase:
   ///    - "8859-1"   ISO/IEC 8859-1  Western European: German, Icelandic, Irish, Italian, Norwegian, Portuguese, Spanish
   ///    - "8859-2"   ISO/IEC 8859-2  Central European: Bosnian, Polish, Croatian, Czech, Slovak, Slovene, Serbian, Hungarian
   ///    - "8859-3"   ISO/IEC 8859-3  South European: Maltese, Esperanto
   ///    - "8859-4"   ISO/IEC 8859-4  North European: Estonian, Latvian, Lithuanian, Greenlandic, and Sami
   ///    - "8859-5"   ISO/IEC 8859-5  Latin/Cyrillic: Belarusian, Bulgarian, Macedonian, Russian, Serbian, Ukrainian
   ///    - "8859-6"   ISO/IEC 8859-6  Latin/Arabic
   ///    - "8859-7"   ISO/IEC 8859-7  Latin/Greek
   ///    - "8859-8"   ISO/IEC 8859-8  Latin/Hebrew
   ///    - "8859-9"   ISO/IEC 8859-9  Latin-5 Turkish
   ///    - "8859-10"  ISO/IEC 8859-10 Latin-6 Nordic: better fit for some North European languages than 8859-4
   ///    - "8859-11"  ISO/IEC 8859-11 Latin/Thai
   ///    - "8859-13"  ISO/IEC 8859-13 Latin-7 Baltic Rim: other variation of 8859-4 and 8859-10
   ///    - "8859-14"  ISO/IEC 8859-14 Latin-8 Celtic
   ///    - "8859-15"  ISO/IEC 8859-15 Latin-9 Better 8859-1 with the Euro sign
   ///    - "8859-16"  ISO/IEC 8859-16 Latin-10 South-Eastern European
   ///    - "ACP"      Windows only, active system 8-bit code page. There is no such encoding in the other operating systems
   ///    - "ASCII"    Standard 7-bit codepage
   ///    - "CP1250"   Microsoft ANSI/OEM Eastern Europe
   ///    - "CP1251"   Microsoft ANSI/OEM Cyrillic
   ///    - "CP1252"   Microsoft ANSI/OEM Latin I
   ///    - "CP1253"   Microsoft ANSI/OEM Greek
   ///    - "CP1254"   Microsoft ANSI/OEM Turkish
   ///    - "CP1255"   Microsoft ANSI/OEM Hebrew
   ///    - "CP1256"   Microsoft ANSI/OEM Arabic
   ///    - "CP1257"   Microsoft ANSI/OEM Baltic
   ///    - "CP437"    Microsoft DOS Latin US
   ///    - "CP737"    Microsoft DOS Greek
   ///    - "CP775"    Microsoft DOS Baltic Rim
   ///    - "CP850"    Microsoft DOS Latin1
   ///    - "CP852"    Microsoft DOS Latin2
   ///    - "CP855"    Microsoft DOS Cyrillic
   ///    - "CP857"    Microsoft DOS Turkish
   ///    - "CP860"    Microsoft DOS Portuguese
   ///    - "CP861"    Microsoft DOS Icelandic
   ///    - "CP862"    Microsoft DOS Hebrew
   ///    - "CP863"    Microsoft DOS French Canada
   ///    - "CP864"    Microsoft DOS Arabic
   ///    - "CP865"    Microsoft DOS Nordic
   ///    - "CP866"    Microsoft DOS Cyrillic Russian
   ///    - "CP869"    Microsoft DOS Greek2
   ///    - "CP874"    Microsoft DOS Thai
   ///    - "UTF-8"    UNICODE byte encoding
   ///
   /// \return String array of supported encodings.
   ///
   static MStdStringVector GetAllSupportedEncodings();

#endif // !M_NO_ENCODING

public: // Declaration:

   M_DECLARE_CLASS(Str)
};

///@}
#endif
