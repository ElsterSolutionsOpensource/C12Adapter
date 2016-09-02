#ifndef MCOM_IDENTIFYSTRING_H
#define MCOM_IDENTIFYSTRING_H
// File MCOM/IdentifyString.h

#include <MCOM/MCOMDefs.h>

#if !M_NO_MCOM_IDENTIFY_METER

/// \cond SHOW_INTERNAL

// Identify string is to facilitate building of the identify string information.
// It is a semi-private class, which is unlikely to be used outside of the library.
// Not exported currently.
//
class MIdentifyString : public MStdString
{
public: // Constructors, services:

   // Default constructor.
   // Creates an empty identify string (initialized with the J command starter).
   //
   MIdentifyString();

   // Append Identify String with the tag that consists of the given name and value.
   //
   void AppendTag(MConstChars tag, const MStdString& value);

   // Append Identify String with the tag that consists of the given name and byte string value.
   // The value is the pointer and its length, written as it is with possible byte widening to the size of wide char.
   // If the length is zero, then zero terminated string is assumed.
   //
   void AppendTag(MConstChars tag, const char* value, unsigned len = 0);

   // Append Identify String with the tag that is a number of hexadecimal bytes of a given length.
   // The value is the pointer and its length. The service can be called either for HEX, or for BCD data.
   // The number can be padded with zeros from either end.
   //
   void AppendHexTag(MConstChars tag, const char* value, unsigned len);

   // Append Identify String with the tag that consists of the given name and unsigned value.
   //
   void AppendTag(MConstChars tag, unsigned v);

   // Append Identify String with the tag that consists of the given name and two unsigned values.
   // The values are separated with the period character.
   //
   void AppendTag(MConstChars tag, unsigned v1, unsigned v2);

   // Append Identify String with the tags that comprise the option board.
   // If the given values stand for no option board, do not append anything.
   // If type is not specified, or it is NULL, the type will be the string NOT_AVAILABLE.
   //
   // \post The string is appended with the given data.
   // If the option board is present, m_obTagOrdinalNumber is incremented and true is returned.
   //
   bool AppendObTags(int position, const char* sspec, unsigned group, unsigned revnum, const char* type = NULL);

   // Insert NUMBER_OF_OPTION_BOARDS tag into a proper position within the identify string.
   // The internal Number of Option Boards is used.
   //
   // \pre Appropriate calls of AppendObTags are made for each option board placeholder.
   //
   void InsertNumberOfObTags();

   // Start a new J string as part of the existing one.
   //
   // \pre The string has some items already, otherwise there is a debug check.
   //
   // POSTRCONDITION: String ";J00" is added to the end of the identify string,
   // and m_obTagOrdinalNumber is nullified, so we are ready 
   // to build a new J string at the end of the current.
   //
   void AppendNew();

public: // fields:

   // String that defines an identification for an unsupported meter type.
   //
   static const MConstChars s_unsupported;

private: // Properties:
   
   // Ordinal number of the Option Board tags
   //
   int m_obTagOrdinalNumber;
};

#endif // !M_NO_MCOM_IDENTIFY_METER

/// \endcond SHOW_INTERNAL
#endif
