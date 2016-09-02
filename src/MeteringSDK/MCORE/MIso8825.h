#ifndef MCORE_MISO8825_H
#define MCORE_MISO8825_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MIso8825.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MException.h>

/// A set of utilities to work with ISO 8825 data types
///
class M_CLASS MIso8825 : public MObject
{
public: // constants:

   enum
   {
      LongestUidBinarySize = 64,    ///< Longest size of binary representation of UID
      LongestUidStringSize = 128,   ///< Longest size of string representation of UID
      ShortestUidStringSize = 2     ///< Shortest size of string representation of UID
   };

public: // methods:

   /// Return true if the ISO8825 Universal Identifier tag stands for relative identifier.
   /// Recognized tags are these:
   ///   - 0x0D : relative, as used in the data
   ///   - 0x80 : relative, as used in ACSE
   ///   - 0x06 : absolute, as used in both data and ACSE
   ///
   /// \param tag
   ///    The tag to check.
   ///    The tag should be one of the above mentioned bytes,
   ///    otherwise an exception is thrown that tells that the tag is bad
   ///
   static bool IsTagRelative(char tag);

   /// True will mean the given UID string is relative.
   ///
   /// To be relative, the UID shall have a period at the beginning.
   ///
   /// \param uid
   ///     UID to check. The given UID shall have a good UID string format, or an exception is thrown.
   ///
   static bool IsUidRelative(const MStdString& uid);

   /// Return short length of the data decoded according to ISO 8825 rules.
   ///
   /// \param tag
   ///    ISO 8825 length, its byte value shall not have the upper bit set,
   ///    otherwise an exception is thrown.
   ///
   static unsigned DecodeShortLength(char tag);

   /// Return length of the data decoded according to ISO 8825 rules, which is either a short length, or a long length.
   /// Use buffer, buffer size, and running index in the buffer.
   ///
   /// \pre The length should have proper format, and it should not be bigger than
   /// four bytes, otherwise an exception is thrown.
   ///
   static unsigned DecodeLengthFromBuffer(const char* buff, unsigned size, unsigned* runningIndex = NULL);

   /// Return length of the data decoded according to ISO 8825 rules, which is either a short length, or a long length.
   /// Use the given string as the source. The given string can be longer than length.
   /// Use DecodedLengthByteSize to determine how long is the length in the given byte string.
   ///
   /// \pre The length should have proper format, and it should not be bigger than
   /// four bytes, otherwise an exception is thrown.
   ///
   static unsigned DecodeLength(const MByteString& byteString);

   /// Return the size of the length.
   /// Use this method together with DecodeLength.
   ///
   /// \pre The length should have proper format, and it should not be bigger than
   /// four bytes, otherwise an exception is thrown.
   ///
   static unsigned DecodedLengthByteSize(const MByteString& byteString);

   /// Return length of the data encoded according to ISO 8825 rules, use the given buffer.
   ///
   /// The length can be a short length, or a long length.
   ///
   /// \param len the length to encode
   /// \param buff buffer into which the length is to be encoded, at least 5 characters
   /// \return result length of the buffer, 0 to 5
   ///
   static unsigned EncodeLengthIntoBuffer(unsigned len, char* buff);

   /// Return length of the data encoded according to ISO 8825 rules.
   ///
   /// The length can be a short length, or a long length.
   ///
   /// \pre length of the data
   ///
   static MByteString EncodeLength(unsigned len);

   /// Return a string representation of the universal identifier, using pointer to denote start of the identifier.
   /// This call interprets only the universal identifier. Length shall be parsed separately.
   ///
   /// The identifier returned for A2 0B A2 09 2A 86 48 CE 52 03 38 AA 4E would be 1.2.840.10066.3.56.5454.
   ///
   /// \pre The universal identifier in the binary data is correct, 
   /// otherwise an exception is thrown.
   ///
   static void DecodeUidFromBuffer(MStdString& result, const char* uidBegin, unsigned length, bool isRelative);

   /// Return a string representation of the whole packed ISO 8825 universal identifier.
   /// This call interprets the start character, length and the body of the universal identifier.
   ///
   /// The identifier returned for A2 0B A2 09 2A 86 48 CE 52 03 38 AA 4E would be 1.2.840.10066.3.56.5454.
   ///
   /// \pre The universal identifier in the binary data is correct, 
   /// otherwise an exception is thrown.
   ///
   static MStdString DecodeUid(const MByteString& uid, bool isRelative);

   /// Return a packed binary representation from the ISO 8825 universal identifier given as a string.
   ///
   /// The identifier returned for 1.2.840.10066.3.56.5454 would be A2 0B A2 09 2A 86 48 CE 52 03 38 AA 4E.
   ///
   /// \pre The universal identifier in the string should be correct, otherwise an exception is
   /// thrown
   ///
   /// \param str string representation of the buffer
   /// \param buff output buffer, at least 60 bytes long
   /// \return the result length of the buffer
   ///
   static unsigned EncodeUidIntoBuffer(const MStdString& str, char* buff);

   /// Return a packed binary representation from the ISO 8825 universal identifier given as a string.
   ///
   /// The identifier returned for 1.2.840.10066.3.56.5454 would be A2 0B A2 09 2A 86 48 CE 52 03 38 AA 4E.
   ///
   /// \pre The universal identifier in the string should be correct, otherwise an exception is
   /// thrown
   ///
   static MByteString EncodeUid(const MStdString& str);

   /// Return a tagged packed binary representation from the ISO 8825 universal identifier given as a string.
   ///
   /// The identifier returned for 1.2.840.10066.3.56.5454 would be A2 0B A2 09 2A 86 48 CE 52 03 38 AA 4E.
   ///
   /// \pre The universal identifier in the string should be correct, otherwise an exception is
   /// thrown
   ///
   /// \param acseTag the ACSE tag assigned to this UID
   /// \param str string representation of the buffer
   /// \param buff output buffer, at least 64 bytes long
   /// \return the result length of the buffer
   ///
   static unsigned EncodeTaggedUidIntoBuffer(char acseTag, const MStdString& str, char* buff);

   /// Return a tagged packed binary representation of a given unsigned number.
   ///
   /// \param acseTag The ACSE tag assigned to this unsigned integer
   /// \param value Value to encode
   /// \param buff output buffer, at least 8 bytes long
   /// \return the result length of the buffer
   ///
   static unsigned EncodeTaggedUnsignedIntoBuffer(char acseTag, unsigned value, char* buff);

   /// Throws 'ISO length is bad' exception
   ///
   static M_NORETURN_FUNC void ThrowBadISOLength();

private: // Disable instantiation of this utility class:

   // No instances are possible
   MIso8825();
   MIso8825(const MIso8825&);
   ~MIso8825();

   M_DECLARE_CLASS(Iso8825)
};

///@}
#endif
