// File MCORE/MIso8825.cpp

#include "MCOREExtern.h"
#include "MIso8825.h"

M_START_PROPERTIES(Iso8825)
M_START_METHODS(Iso8825)
   M_CLASS_SERVICE                  (Iso8825, IsTagRelative,         ST_bool_S_byte)
   M_CLASS_SERVICE                  (Iso8825, IsUidRelative,         ST_bool_S_constMStdStringA)
   M_CLASS_SERVICE                  (Iso8825, DecodeShortLength,     ST_unsigned_S_byte)
   M_CLASS_SERVICE                  (Iso8825, DecodeLength,          ST_unsigned_S_constMByteStringA)
   M_CLASS_SERVICE                  (Iso8825, DecodedLengthByteSize, ST_unsigned_S_constMByteStringA)
   M_CLASS_SERVICE                  (Iso8825, EncodeLength,          ST_MByteString_S_unsigned)
   M_CLASS_SERVICE                  (Iso8825, DecodeUid,             ST_MStdString_S_constMByteStringA_bool)
   M_CLASS_SERVICE                  (Iso8825, EncodeUid,             ST_MByteString_S_constMStdStringA)
M_END_CLASS(Iso8825, Object)

   static M_NORETURN_FUNC void DoThrowISO8825StringBad()
   {
      MException::Throw(M_CODE_STR(M_ERR_ISO8825_STRING_REPRESENTATION_OF_UNIVERSAL_IDENTIFIER_IS_BAD, M_I("ISO 8825 string representation of universal identifier is bad")));
      M_ENSURED_ASSERT(0);
   }

   static M_NORETURN_FUNC void DoThrowISO8825BinaryBad()
   {
      MException::Throw(M_CODE_STR(M_ERR_ISO8825_BINARY_REPRESENTATION_OF_UNIVERSAL_IDENTIFIER_IS_BAD, M_I("ISO 8825 binary representation of universal identifier is bad")));
      M_ENSURED_ASSERT(0);
   }

bool MIso8825::IsTagRelative(char tag)
{
   if ( tag == '\x06' ) // absolute, data and ACSE
      return false;
   else if ( tag != '\x0D' && tag != '\x80' ) // relative, data and ACSE
   {
      DoThrowISO8825BinaryBad();
      M_ENSURED_ASSERT(0);
   }
   return true;
}

bool MIso8825::IsUidRelative(const MStdString& uid)
{
   return !uid.empty() && *uid.begin() == '.';
}

unsigned MIso8825::DecodeShortLength(char tag)
{
   if ( (tag & 0x80u) != 0u )
   {
      MException::Throw(M_CODE_STR(M_ERR_ISO8825_SHORT_LENGTH_IS_BAD, M_I("ISO 8825 short length is bad, does not fit in one byte")));
      M_ENSURED_ASSERT(0);
   }
   return (unsigned)(unsigned char)tag;
}

M_NORETURN_FUNC void MIso8825::ThrowBadISOLength()
{
   MException::Throw(M_CODE_STR(M_ERR_ISO8825_LENGTH_IS_BAD, M_I("ISO 8825 length is bad")));
   M_ENSURED_ASSERT(0);
}

unsigned MIso8825::DecodeLengthFromBuffer(const char* buff, unsigned size, unsigned* runningIndex)
{
   unsigned idx = (runningIndex == NULL) ? 0 : *runningIndex;
   if ( idx >= size )
   {
      ThrowBadISOLength();
      M_ENSURED_ASSERT(0);
   }
   unsigned length = (unsigned)(unsigned char)(buff[idx++]);
   if ( (length & 0x80u) != 0u ) // otherwise this is short length, return immediately
   {
      unsigned numBytes = (length & 0x7Fu);      // clear the bit and get the number of the following octets
      if ( unsigned(size - idx) < numBytes || numBytes >= 4u ) // we do not support length bigger than four bytes
      {
         ThrowBadISOLength();
         M_ENSURED_ASSERT(0);
      }
      length = 0u;
      for ( unsigned i = 0u; i < numBytes; ++i )
      {
         length <<= 8;
         length += (unsigned)(unsigned char)(buff[idx++]);
      }
   }
   if ( runningIndex != NULL )
      *runningIndex = idx;
   return length;
}

unsigned MIso8825::DecodeLength(const MByteString& byteString)
{
   return DecodeLengthFromBuffer(byteString.data(), static_cast<unsigned>(byteString.size()));
}

unsigned MIso8825::DecodedLengthByteSize(const MByteString& byteString)
{
   unsigned len = 0;
   DecodeLengthFromBuffer(byteString.data(), static_cast<unsigned>(byteString.size()), &len);
   return len;
}

unsigned MIso8825::EncodeLengthIntoBuffer(unsigned len, char* buff)
{
   M_ENSURED_ASSERT(sizeof(unsigned) == 4); // precondition of the below code

   if ( len > 0xFFFFFFu )
   {
      buff[0] = '\x84'; // length of the length is four bytes, upper bit is set
      buff[1] = char(len >> 24);
      buff[2] = char(len >> 16);
      buff[3] = char(len >> 8);
      buff[4] = char(len);
      return 5;
   }
   if ( len > 0xFFFFu )
   {
      buff[0] = '\x83'; // length of the length is three bytes, upper bit is set
      buff[1] = char(len >> 16);
      buff[2] = char(len >> 8);
      buff[3] = char(len);
      return 4;
   }
   if ( len > 0xFFu )
   {
      buff[0] = '\x82'; // length of the length is two bytes, upper bit is set
      buff[1] = char(len >> 8);
      buff[2] = char(len);
      return 3;
   }
   else if ( len > 0x7Fu ) // careful! Here the number is 0x7Fu, and not 0xFFu
   {
      buff[0] = '\x81'; // length of the length is two bytes, upper bit is set
      buff[1] = char(len);
      return 2;
   }
   buff[0] = static_cast<char>(len); // no length is given, size is as it is, upper bit is clear
   return 1;
}

MByteString MIso8825::EncodeLength(unsigned len)
{
   MByteString result;
   char buff [ 8 ];
   unsigned size = EncodeLengthIntoBuffer(len, buff);
   result.assign(buff, size);
   return result;
}

void MIso8825::DecodeUidFromBuffer(MStdString& result, const char* uid, unsigned length, bool isRelative)
{
   M_ASSERT(length > 0 && length < 128); // properly written code should not allow that

   result.clear();
   result.reserve(length * 2); // this is always the case for GUID

   MChar buffer [ 64 ];
   const char* uidLast = uid + length;
   if ( !isRelative )
   {
      Muint8 b = (Muint8)*uid;
      result += MChar(unsigned(b / 40u) + unsigned('0')); // we know range, know we are ASCII...
      b %= 40u; // second digit 0 .. 39 is encoded with base 40
      result += '.';
      result += MToChars((unsigned long)b, buffer);
      ++uid;
   }
   unsigned number = 0u;
   while ( uid != uidLast )
   {
      number <<= 7;    // shift with the digit base
      unsigned char b = (unsigned char)*uid++;
      if ( (b & 0x80u) == 0 ) // the end of the digit encountered 
      {
         result += '.';
         number += b;
         result += MToChars((unsigned long)number, buffer);
         number = 0u;
      }
      else // proceed with a single number
         number += (b & 0x7Fu);
   }
}

MStdString MIso8825::DecodeUid(const MByteString& uid, bool isRelative)
{
   MStdString result;
   DecodeUidFromBuffer(result, uid.data(), M_64_CAST(unsigned, uid.size()), isRelative);
   return result;
}

   // Append the encoded number to the end of the result byte string
   //
   static unsigned DoFetchDigit(MStdString::const_iterator& it, MStdString::const_iterator itEnd)
   {
      if ( it == itEnd )
      {
         DoThrowISO8825StringBad();
         M_ENSURED_ASSERT(0);
      }
      if ( *it == '.' )
         ++it;
      MStdString::const_iterator period = std::find(it, itEnd, '.');
      itEnd = period; // this will work in either way - whether or not the period is found
      unsigned result;
      try
      {
         result = MToUnsignedLong(MStdString(it, itEnd));
      }
      catch ( ... )
      {
         DoThrowISO8825StringBad();
         M_ENSURED_ASSERT(0);
      }
      it = itEnd;
      return result;
   }

unsigned MIso8825::EncodeUidIntoBuffer(const MStdString& str, char* buff)
{
   if ( str.size() < ShortestUidStringSize || str.size() > LongestUidStringSize )
   {
      DoThrowISO8825StringBad();
      M_ENSURED_ASSERT(0);
   }

   char* buffPtr = buff;

   MStdString::const_iterator begin = str.begin();
   MStdString::const_iterator end = str.end();
   if ( *begin == '.' ) // relative
      ++begin;
   else // absolute
   {
      unsigned firstDigit = DoFetchDigit(begin, end);
      unsigned secondDigit = DoFetchDigit(begin, end);
      if ( firstDigit > 2 || secondDigit > 39 ) // inherent restriction of the universal identifier
      {
         DoThrowISO8825StringBad();
         M_ENSURED_ASSERT(0);
      }
      *buffPtr++ = (char)(Muint8)(firstDigit * 40u + secondDigit); // this is the way the first byte is assembled
   }

   while ( begin < end )
   {
      unsigned number = DoFetchDigit(begin, end);
      if ( number > 0x0FFFFFFFu ) // we do not support more than four septets of digits, or no-digits, only what fits into uint32
      {
         DoThrowISO8825StringBad();
         M_ENSURED_ASSERT(0);
      }

      // Hope the compiler does some goto-s in the efficient way here to avoid unnecessary comparisons
      if ( number >= 0x200000u )
         *buffPtr++ = (char)(unsigned char)((number >> 21) | 0x80);
      if ( number >= 0x4000u )
         *buffPtr++ = (char)(unsigned char)((number >> 14) | 0x80);
      if ( number >= 0x80u )
         *buffPtr++ = (char)(unsigned char)((number >> 7)  | 0x80);
      *buffPtr++  = (char)(unsigned char)(number & 0x7F);
   }
   unsigned resultLen = static_cast<unsigned>(buffPtr - buff);
   M_ASSERT(resultLen <= LongestUidBinarySize);
   M_ASSERT(resultLen <= str.size() / 2); // this arrives from string representation
   return resultLen;
}

MByteString MIso8825::EncodeUid(const MStdString& str)
{
   MByteString result;
   char buff [ 64 ];
   unsigned size = EncodeUidIntoBuffer(str, buff);
   result.assign(buff, size);
   return result;
}

unsigned MIso8825::EncodeTaggedUidIntoBuffer(char acseTag, const MStdString& uid, char* buff)
{
   unsigned size = MIso8825::EncodeUidIntoBuffer(uid, buff + 4);
   buff[0] = acseTag;
   buff[1] = static_cast<char>(size + 2); // as we know it is always one-byte
   buff[2] = IsUidRelative(uid) ? '\x80' : '\x06';
   buff[3] = static_cast<char>(size);     // as we know it is always one-byte
   return size + 4;
}

unsigned MIso8825::EncodeTaggedUnsignedIntoBuffer(char acseTag, unsigned value, char* buff)
{
   buff[0] = acseTag;
   buff[2] = '\x02'; // integer
   if ( value <= 0xFFu )
   {
      buff[1] = 3;
      buff[3] = 1;
      buff[4] = (char)value;
      return 5;
   }
   if ( value <= 0xFFFFu )
   {
      buff[1] = 4;
      buff[3] = 2;
      buff[4] = (char)(value >> 8); // most significant byte first
      buff[5] = (char)value;
      return 6;
   }
   if ( value <= 0xFFFFFFu )
   {
      buff[1] = 5;
      buff[3] = 3;
      buff[4] = (char)(value >> 16); // most significant byte first
      buff[5] = (char)(value >> 8);
      buff[6] = (char)value;
      return 7;
   }
   buff[1] = 6;
   buff[3] = 4;
   buff[4] = (char)(value >> 24); // most significant byte first
   buff[5] = (char)(value >> 16);
   buff[6] = (char)(value >> 8);
   buff[7] = (char)value;
   return 8;
}
