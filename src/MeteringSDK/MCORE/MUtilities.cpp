// File MCORE/MUtilities.cpp

#include "MCOREExtern.h"
#include "MUtilities.h"
#include "MException.h"
#include "MStreamSocket.h"
#include "MStreamFile.h"
#include "MMath.h"
#include "MStr.h"
#include "MAlgorithm.h"
#include "MFindFile.h"
#include "MCriticalSection.h"
#include "MVariantParser.h"
#include "MJavaEnv.h"

#if (M_OS & M_OS_LINUX) != 0
   #include <unistd.h>
   #include <sys/utsname.h>
#endif

// If not so, one has to re-declare and reimplement
//    virtual unsigned GetEmbeddedSizeof() const;
M_COMPILED_ASSERT(sizeof(MTimer) == sizeof(MUtilities));

   #if !M_NO_REFLECTION

      static MVariant DoNew()
      {
         MUtilities util;
         return MVariant(&util, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      /// Convert the Binary Coded Decimal number given as byte string to double.
      ///
      /// This method works with little endian BCD numbers.
      /// Binary coded decimal is a way of representing decimal numbers
      /// so that each byte has a pair of numbers, each in range 0 to 9.
      /// A BCD with hex representation x"00123456" represents a number 123456.
      /// No special codes are allowed in the BCD, only 0 .. 9.
      ///
      /// \param bytes
      ///    Bytes of the BCD number.
      /// \return result number
      ///
      /// \pre The given BCD shall be a proper BCD number consisting of
      /// an even number of digits. Otherwise an exception is thrown.
      /// The double overflow is not watched.
      ///
      static double DoFromBCD1(const MByteString& bytes)
      {
         return MUtilities::FromBCD(bytes);
      }

      /// Convert a positive double to a BCD coded byte string.
      ///
      /// This method produces little endian BCD numbers of minimum size into which the given number fits.
      /// Size, if specified, shall be the length of the result data in bytes.
      /// If the size is not specified, it will be calculated to be the minimum where the value fits.
      ///
      /// \param value
      /// \return the result number
      ///
      /// \pre The given double number shall be positive, and if size
      /// is specified, it shall fit in the number of bytes given.
      /// Otherwise an exception is thrown.
      ///
      static MByteString DoToBCD1(double value)
      {
         return MUtilities::ToBCD(value);
      }

      /// Convert a positive double to a BCD coded byte string.
      ///
      /// This method produces little endian BCD numbers.
      /// Size, if specified, shall be the length of the result data in bytes.
      /// If the size is not specified, it will be calculated to be the minimum where the value fits.
      ///
      /// \param value
      ///    Value to convert into BCD string.
      /// \param size
      ///    Size of the result. If zero, the size will be the minimum, which fits.
      /// \return the result number
      ///
      /// \pre The given double number shall be positive, and if size
      /// is specified, it shall fit in the number of bytes given.
      /// Otherwise an exception is thrown.
      ///
      static MByteString DoToBCD2(double value, unsigned size)
      {
         return MUtilities::ToBCD(value, size);
      }

   #endif

M_START_PROPERTIES(Utilities)
#if !M_NO_FILESYSTEM
   M_CLASS_PROPERTY_STRING                  (Utilities, CurrentPath,           ST_MStdString_S, ST_S_constMStdStringA)
   M_CLASS_PROPERTY_READONLY_STRING         (Utilities, ModulePath,            ST_MStdString_S)
   M_CLASS_PROPERTY_READONLY_STRING         (Utilities, InstallationPath,      ST_MStdString_S)
   M_CLASS_PROPERTY_READONLY_STRING         (Utilities, TempDirectory,         ST_MStdString_S)
   M_CLASS_PROPERTY_READONLY_STRING         (Utilities, HomeDirectory,         ST_MStdString_S)
#endif
   M_CLASS_PROPERTY_READONLY_STRING         (Utilities, LocalHostName,         ST_MStdString_S)
   M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED(Utilities, Version,               ST_MObjectP_S)
   M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED(Utilities, ProductVersion,        ST_MObjectP_S)
   M_CLASS_PROPERTY_READONLY_STRING         (Utilities, ProductName,           ST_MStdString_S)
   M_CLASS_PROPERTY_READONLY_STRING         (Utilities, OperatingSystemName,   ST_MStdString_S)
   M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED(Utilities, OperatingSystemVersion,ST_MObjectP_S)
   M_CLASS_PROPERTY_READONLY_INT            (Utilities, NumberOfProcessors)
   M_CLASS_PROPERTY_READONLY_INT            (Utilities, NumberOfAddressBits)
M_START_METHODS(Utilities)
   M_CLASS_SERVICE_OVERLOADED       (Utilities, FromBCD,    FromBCD,    2,    ST_double_S_constMByteStringA_bool)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(Utilities, FromBCD,    DoFromBCD1, 1,    ST_double_S_constMByteStringA)          // SWIG_HIDE default parameter is done by Compare
   M_CLASS_SERVICE_OVERLOADED       (Utilities, ToBCD,      ToBCD,      3,    ST_MByteString_S_double_unsigned_bool)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(Utilities, ToBCD,      DoToBCD2,   2,    ST_MByteString_S_double_unsigned)       // SWIG_HIDE default parameter is done by Compare
   M_CLASS_FRIEND_SERVICE_OVERLOADED(Utilities, ToBCD,      DoToBCD1,   1,    ST_MByteString_S_double)                // SWIG_HIDE default parameter is done by Compare
   M_CLASS_SERVICE                  (Utilities, FromUINT,                     ST_MVariant_S_constMByteStringA_bool)
   M_CLASS_SERVICE                  (Utilities, ToUINT,                       ST_MByteString_S_constMVariantA_unsigned_bool)
   M_CLASS_SERVICE                  (Utilities, FromINT,                      ST_MVariant_S_constMByteStringA_bool)
   M_CLASS_SERVICE                  (Utilities, ToINT,                        ST_MByteString_S_constMVariantA_unsigned_bool)
   M_CLASS_SERVICE                  (Utilities, FromINSTR,                    ST_double_S_constMByteStringA)
   M_CLASS_SERVICE                  (Utilities, ToINSTR,                      ST_MByteString_S_double)
   M_CLASS_SERVICE                  (Utilities, FromDSPFloat,                 ST_double_S_constMByteStringA)
   M_CLASS_SERVICE                  (Utilities, FromDSPInt,                   ST_double_S_constMByteStringA)
   M_CLASS_SERVICE                  (Utilities, ToDSPFloat,                   ST_MByteString_S_double_unsigned)
   M_CLASS_SERVICE                  (Utilities, ToDSPInt,                     ST_MByteString_S_double_unsigned)
   M_CLASS_SERVICE                  (Utilities, FromRAD40,                    ST_MStdString_S_constMByteStringA)
   M_CLASS_SERVICE                  (Utilities, ToRAD40,                      ST_MByteString_S_constMStdStringA_unsigned)
   M_CLASS_SERVICE                  (Utilities, NumberToHexByte,              ST_byte_S_unsigned)
   M_CLASS_SERVICE                  (Utilities, NumberToHexChar,              ST_MChar_S_unsigned)
   M_CLASS_SERVICE                  (Utilities, HexByteToNumber,              ST_unsigned_S_byte)
   M_CLASS_SERVICE                  (Utilities, HexCharToNumber,              ST_unsigned_S_MChar)
   M_CLASS_SERVICE                  (Utilities, BytesToHex,                   ST_MByteString_S_constMByteStringA_constMVariantA)
   M_CLASS_SERVICE                  (Utilities, HexToBytes,                   ST_MByteString_S_constMByteStringA)
   M_CLASS_SERVICE                  (Utilities, BytesToHexString,             ST_MStdString_S_constMByteStringA_constMVariantA)
   M_CLASS_SERVICE                  (Utilities, HexStringToBytes,             ST_MByteString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, BytesToNumericString,         ST_MStdString_S_constMByteStringA_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, NumericStringToBytes,         ST_MByteString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, ToMDLConstant,                ST_MStdString_S_constMVariantA)
   M_CLASS_SERVICE                  (Utilities, FromMDLConstant,              ST_MVariant_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, ToRelaxedMDLConstant,         ST_MStdString_S_constMVariantA)
#if !M_NO_FILESYSTEM
   M_CLASS_SERVICE                  (Utilities, GetPath,                      ST_MStdString_S_constMStdStringA_constMStdStringA_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, MergePaths,                   ST_MStdString_S_constMStdStringA_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, GetPathExtension,             ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, GetPathFileName,              ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, GetPathFileNameAndExtension,  ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, GetPathDirectory,             ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, GetFullPath,                  ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, IsPathFull,                   ST_bool_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, IsPathDirectory,              ST_bool_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, IsPathExisting,               ST_bool_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, CopyFile,                     ST_S_constMStdStringA_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, DeleteFile,                   ST_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, MoveFile,                     ST_S_constMStdStringA_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, CreateDirectory,              ST_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, EnsureDirectoryExistsForFile, ST_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, GetEnv,                       ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, SetEnv,                       ST_S_constMStdStringA_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, ExpandEnvVars,                ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, MakeTempFileName,             ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, MakeTempDirectoryName,        ST_MStdString_S_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, FindFiles,                    ST_MStdStringVector_S_constMStdStringA_constMStdStringA)
   M_CLASS_SERVICE                  (Utilities, FindDirectories,              ST_MStdStringVector_S_constMStdStringA_constMStdStringA)
#endif // !M_NO_FILESYSTEM
#if !M_NO_BASE64
   M_CLASS_SERVICE                  (Utilities, Base64Encode,                 ST_MStdString_S_constMByteStringA)
   M_CLASS_SERVICE                  (Utilities, Base64Decode,                 ST_MByteString_S_constMStdStringA)
#endif
   M_CLASS_FRIEND_SERVICE           (Utilities, New,           DoNew,         ST_MVariant_S)
M_END_CLASS(Utilities, Timer)

   using namespace std;

   const unsigned s_MAX_BCD_SIZE  = 64;     // maximum size of the BCD buffer
   const double   s_MAX_BCD_VALUE = 1.0e22; // after such number the algorithm becomes imprecise and starts to give bad values

   inline MChar ToAsciiChar(unsigned radix)
   {
      static const MChar s_radixChars[41] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-.?";
      M_ASSERT(radix < 40u);
      return s_radixChars[radix];
   }

   static void M_NORETURN_FUNC DoThrowBadRAD40Char(MChar illegalChar)
   {
      MException::Throw(M_CODE_STR_P1(M_ERR_BAD_RAD40_CHARACTER_S1, M_I("Character '%s' is not allowed in RAD40"), MStr::CharToEscapedString(illegalChar).c_str()));
      M_ENSURED_ASSERT(0);
   }


MUtilities::MUtilities() M_NO_THROW
:
   MTimer()
{
}

MUtilities::~MUtilities() M_NO_THROW
{
}

MStdString MUtilities::FromRAD40Buffer(const char* data, int byteLen)
{
   MStdString result;

   const Muint16* buffer = (const Muint16*)data;
   const Muint16* bufferEnd = (const Muint16*)(data + byteLen);
   for ( ; buffer < bufferEnd; ++buffer )
   {
      MChar cs [ 3 ];
      unsigned triple = (unsigned)*buffer;
      for ( int i = 2; i >= 0; --i, triple /= 40 ) // this loop goes backwards, this is why we cannot add to result immediately
         cs[i] = ToAsciiChar(triple % 40);
      if ( triple > 0 )
      {
         DoThrowBadRAD40Char(MChar(*buffer / (40 * 40)));
         M_ENSURED_ASSERT(0);
      }
      result.append(cs, 3);
   }
   return result;
}

MStdString MUtilities::FromRAD40(const MByteString& data)
{
   return FromRAD40Buffer(data.data(), M_64_CAST(unsigned, data.size()));
}

   inline unsigned ToRadixChar(MChar ascii)
   {
      if ( ascii == ' ' )
         return 0u;
      if ( ascii >= '0' && ascii <= '9' )
         return (unsigned)ascii - (unsigned)('0' - 1);
      if ( ascii >= 'A' && ascii <= 'Z' )
         return (unsigned)ascii - (unsigned)('A' - 11);
      if ( ascii == '-' )
         return 37u;
      if ( ascii == '.' )
         return 38u;
      if ( ascii == '?' )
         return 39u;
      return UINT_MAX;
   }

void MUtilities::ToRAD40Buffer(const MStdString& str, char* rad, unsigned radSize)
{
   size_t strSize = str.size();
   size_t radByteSize = radSize * 3 / 2;
   if ( strSize > radByteSize )
   {
      MException::ThrowStringTooLong((int)strSize, (int)radByteSize);
      M_ENSURED_ASSERT(0);
   }

   Muint16* buffer = (Muint16*)rad;
   Muint16* bufferEnd = (Muint16*)(rad + radSize);

   // Convert to radix string
   unsigned radixTriple = 0;                       // keep it in unsigned for performance, cast later
   unsigned i = 0;
   while ( i < strSize )
   {
      MChar asciiChar = str[i];
      unsigned radixChar = ToRadixChar(asciiChar); // keep it in unsigned for performance, cast later
      if ( radixChar == UINT_MAX )
      {
         DoThrowBadRAD40Char(asciiChar);
         M_ENSURED_ASSERT(0);
      }

      radixTriple *= 40;
      radixTriple += radixChar;
      ++i;
      if ( (i % 3) == 0 )
      {
         M_ENSURED_ASSERT((radixTriple & 0xFFFF0000) == 0);  // we fit in 2 bytes
         *buffer++ = (Muint16)radixTriple;
         radixTriple = 0;
      }
   }

   if ( (i % 3) != 0  )      // Add space to the current character.
   {
      do
      {
         radixTriple *= 40;
         ++i;
      } while ( (i % 3) != 0 );
      M_ENSURED_ASSERT((radixTriple & 0xFFFF0000) == 0);     // we fit in 2 bytes
      *buffer++ = (Muint16)radixTriple;
   }

   while ( buffer < bufferEnd )          // Fill the rest with ' '
      *buffer++ = 0u;
}

MByteString MUtilities::ToRAD40(const MStdString& str, unsigned radSize)
{
   char radBuffer [ 256 ]; // we will never have a bigger RAD number
   if ( radSize > sizeof(radBuffer) ) // Currently just silently truncate the string
      radSize = sizeof(radBuffer);
   ToRAD40Buffer(str, radBuffer, radSize);
   return MByteString(radBuffer, radSize);
}

   const char s_numberToHexByteMapping[] =
   {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
   };

   const char s_numberToHexByteMappingLowerCase[] =
   {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
   };

   inline char DoNumberToHexByte(unsigned n)
   {
      M_ASSERT(n < 0x10);
      return s_numberToHexByteMapping[n];
   }

   inline char DoNumberToHexByteLowerCase(unsigned n)
   {
      M_ASSERT(n < 0x10);
      return s_numberToHexByteMappingLowerCase[n];
   }

Muint8 MUtilities::NumberToHexByte(unsigned n)
{
   MENumberOutOfRange::CheckInteger(0, 15, n);
   return (Muint8)DoNumberToHexByte(n);
}

unsigned MUtilities::HexCharToNumber(MChar cc)
{
   // This sequence works much faster than isxdigit, etc
   //
   const unsigned u = (unsigned)(unsigned char)cc;
   if ( u >= '0' && u <= '9' )
      return u  - unsigned('0');
   if ( u >= 'A' && u <= 'F' )
      return u  - unsigned('A' - 10);
   if ( u < 'a' || u > 'f' )
   {
      MException::Throw(M_CODE_STR_P1(M_ERR_CANNOT_CONVERT_CHARACTER_WITH_CODE_X1_INTO_HEX, M_I("Cannot convert character with code 0x%X into a hexadecimal number"), u));
      M_ENSURED_ASSERT(0);
   }
   return u  - unsigned('a' - 10);
}

   static void DoFromBCDBuffer(double& result, double& multiplier, unsigned ch)
   {
      unsigned x = ch & 0xF;
      unsigned y = ch >> 4;
      if ( x > 9 || y > 9 )
      {
         MException::Throw(M_CODE_STR_P1(MErrorEnum::BadBcd, M_I("Cannot convert byte with the value 0x%X to BCD"), ch));
         M_ENSURED_ASSERT(0);
      }
      result += multiplier * x;
      multiplier *= 10.0;
      result += multiplier * y;
      multiplier *= 10.0;
   }

double MUtilities::FromBCDBuffer(const char* data, unsigned size, bool littleEndian)
{
   double result = 0.0;
   double multiplier = 1.0;
   if ( littleEndian )
   {
      for ( int i = 0; i < (int)size; ++i )
         DoFromBCDBuffer(result, multiplier, (unsigned)(unsigned char)data[i]);
   }
   else
   {
      for ( int i = (int)size - 1; i >= 0; --i )
         DoFromBCDBuffer(result, multiplier, (unsigned)(unsigned char)data[i]);
   }
   return result;
}

double MUtilities::FromBCD(const MByteString& bytes, bool littleEndian)
{
   return FromBCDBuffer(bytes.data(), M_64_CAST(unsigned, bytes.size()), littleEndian);
}

   static void DoFinalizeBuffer(char* buffer, int index, unsigned size, bool littleEndian) M_NO_THROW
   {
      for ( ; index >= 0; --index )
         buffer[index] = '\0';
      if ( littleEndian )
      {
         char* low = buffer;
         char* high = buffer + size - 1;
         for ( ; low < high; ++low, --high )
            swap(*low, *high);
      }
   }

void MUtilities::ToBCDBuffer(unsigned value, char* buffer, unsigned size, bool littleEndian)
{
   M_ASSERT(size > 0);
   int index = int(size) - 1;
   do
   {
      M_ASSERT(index >= 0);
      unsigned char p1 = static_cast<unsigned char>(value % 10);
      value = value / 10;
      unsigned char p2 = static_cast<unsigned char>(value % 10);
      buffer[index--] = char(p1 | (p2 << 4));
      value = value / 10;
   } while ( value > 0 );
   DoFinalizeBuffer(buffer, index, size, littleEndian);
}

void MUtilities::ToBCDBuffer(double value, char* buffer, unsigned size, bool littleEndian)
{
   M_ASSERT(size > 0);
   M_ASSERT(value < 1.0e32); // for bigger numbers the precision becomes a bad issue and we start to output wrong numbers
   int index = int(size) - 1;
   do
   {
      M_ASSERT(index >= 0);
      unsigned char p1 = static_cast<unsigned char>(fmod(value, 10));
      value = value / 10.0;
      unsigned char p2 = static_cast<unsigned char>(fmod(value, 10));
      buffer[index--] = char(p1 | (p2 << 4));
      value = value / 10.0;
   } while ( value >= 1.0 );
   DoFinalizeBuffer(buffer, index, size, littleEndian);
}

MByteString MUtilities::ToBCD(double value, unsigned size, bool littleEndian)
{
   value = MMath::Round0(value);

   if ( size == 0 )  // calculate size if it is not supplied
   {
      if ( value > 0.0 )
      {
         size = static_cast<unsigned int>(ceil(log10(value + 1.0)));
         if ( (size & 0x1) != 0 )  // check if even (in general, it is faster than size % 2)
            ++size;
         size >>= 1;
         if ( size > s_MAX_BCD_SIZE )
            size = s_MAX_BCD_SIZE; // exception will be thrown later on range check
      }
      else
         size = 1; // case if value == 0, or if it is negative (an exception will happen later)
   }
   else if ( size > s_MAX_BCD_SIZE )
   {
      MENumberOutOfRange::ThrowNamedRange(0, s_MAX_BCD_SIZE, size, "BcdSize");
      M_ENSURED_ASSERT(0);
   }

   // Check whether we fit into the supplied size
   //
   double maxValue = MMath::Pow10(size << 1) - 1.0;
   if ( maxValue > s_MAX_BCD_VALUE )
      maxValue = s_MAX_BCD_VALUE;
   MENumberOutOfRange::Check(0.0, maxValue, value);

   M_ASSERT(size > 0 && size <= s_MAX_BCD_SIZE);

   char rawBuffer [ s_MAX_BCD_SIZE ];
   if ( value <= static_cast<double>(UINT_MAX) )
      ToBCDBuffer(static_cast<unsigned>(value), rawBuffer, size, littleEndian);
   else
      ToBCDBuffer(value, rawBuffer, size, littleEndian);
   return MByteString(rawBuffer, size);
}

   static void DoCopyWithPossibleSwap(char* toBuffer, const char* fromBuffer, unsigned size, bool littleEndian)
   {
      if ( size == 0 || size > 8 )
      {
         MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_SIZE_OF_NUMBER_OUTSIDE_RANGE, "Size of byte string representation of a number shall be in range 1 to 8"));
         M_ENSURED_ASSERT(0);
      }

      if ( littleEndian == (M_LITTLE_ENDIAN != 0) )
      {
         for ( ; size > 0; --size )
           *toBuffer++ = *fromBuffer++;
      }
      else
      {
         for ( fromBuffer += size - 1; size > 0; --size )
            *toBuffer++ = *fromBuffer--;
      }
   }

unsigned MUtilities::UnsignedFromUINT(const MByteString& bytes, bool littleEndian)
{
   unsigned val = 0;
   unsigned size = static_cast<unsigned>(bytes.size());
   MENumberOutOfRange::Check(1u, 4u, size, "size");
   DoCopyWithPossibleSwap((char*)&val, bytes.data(), size, littleEndian);
   return val;
}

#if !M_NO_REFLECTION

MVariant MUtilities::FromUINT(const MByteString& bytes, bool littleEndian)
{
   MVariant result;
   unsigned size = static_cast<unsigned>(bytes.size());
   MENumberOutOfRange::Check(1u, 8u, size, "size");
   if ( size <= 4 )
   {
      unsigned val = 0;
      DoCopyWithPossibleSwap((char*)&val, bytes.data(), size, littleEndian);
      result.DoAssignToEmpty(val);
   }
   else
   {
      Muint64 val = MUINT64C(0);
      DoCopyWithPossibleSwap((char*)&val, bytes.data(), size, littleEndian);
      result.DoAssignToEmpty(double(val));
   }
   return result;
}

   static const Muint32 s_uintBits32[] =
   {
      0x00000000u, // not used
      0x000000ffu,
      0x0000ffffu,
      0x00ffffffu,
      0xffffffffu
   };

   static const Muint64 s_uintBits64[] =
   {
      MUINT64C(0x0000000000000000), // not used
      MUINT64C(0x00000000000000ff),
      MUINT64C(0x000000000000ffff),
      MUINT64C(0x0000000000ffffff),
      MUINT64C(0x00000000ffffffff),
      MUINT64C(0x000000ffffffffff),
      MUINT64C(0x0000ffffffffffff),
      MUINT64C(0x00ffffffffffffff),
      MUINT64C(0xffffffffffffffff)
   };

   static void DoCopyResultWithPossibleSwap(MByteString& result, const MVariant& value, unsigned size, bool littleEndian, bool isUnsigned)
   {
      M_ASSERT(result.empty());
      result = value.AsByteString();
      int diff = static_cast<int>(static_cast<MByteString::size_type>(size) - result.size());
      char c = (isUnsigned || (*result.rbegin() & 0x80) == 0)
               ? '\0'     // positive
               : '\xFF';  // negative
      if ( diff > 0 ) // pad bytes
         result.append(diff, c);
      else if ( diff < 0 ) // truncate bytes unless they are not padding
      {
         MByteString::size_type nonzeroPos = result.find_last_not_of(c);
         if ( nonzeroPos != MByteString::npos )
            MENumberOutOfRange::CheckNamedUnsignedRange(1u, size, static_cast<unsigned>(nonzeroPos + 1), "result.size");
         result.erase(result.begin() + size, result.end());
      }
      if ( !littleEndian ) // swap bytes
         std::reverse(result.begin(), result.end());
   }

MByteString MUtilities::ToUINT(const MVariant& value, unsigned size, bool littleEndian)
{
   MByteString result;
   MENumberOutOfRange::Check(1, 8, size, "size");
   if ( value.GetType() == MVariant::VAR_BYTE_STRING )
      DoCopyResultWithPossibleSwap(result, value, size, littleEndian, true);
   else
   {
      char buff [ 8 ];
      if ( size <= 3 )
      {
         Muint32 val = value.AsUInt();
         Muint32 maximum = s_uintBits32[size];
         MENumberOutOfRange::CheckUnsignedRange(0u, maximum, val);
         DoCopyWithPossibleSwap(buff, (const char*)&val, size, littleEndian);
      }
      else if ( size == 4 )
      {
         Muint32 val = value.AsUInt();
         DoCopyWithPossibleSwap(buff, (const char*)&val, size, littleEndian);
      }
      else
      {
         double doubleValue = MMath::Round0(value.AsDouble()); // we've rounded the value already, just convert
         double maximum = (double)s_uintBits64[size];
         MENumberOutOfRange::Check(0.0, maximum, doubleValue);
         Muint64 val = (Muint64)doubleValue; // we've rounded the value already, just convert
         DoCopyWithPossibleSwap(buff, (const char*)&val, size, littleEndian);
      }
      result.assign(buff, size);
   }
   return result;
}

   static const Mint32 s_intBits32[] =
   {
      static_cast<Mint32>(0x00000000), // not used
      static_cast<Mint32>(0xffffff80),
      static_cast<Mint32>(0xffff8000),
      static_cast<Mint32>(0xff800000),
      static_cast<Mint32>(0x80000000)
   };

   static const Mint64 s_intBits64[] =
   {
      static_cast<Mint64>(MUINT64C(0x0000000000000000)), // not used
      static_cast<Mint64>(MUINT64C(0xffffffffffffff80)),
      static_cast<Mint64>(MUINT64C(0xffffffffffff8000)),
      static_cast<Mint64>(MUINT64C(0xffffffffff800000)),
      static_cast<Mint64>(MUINT64C(0xffffffff80000000)),
      static_cast<Mint64>(MUINT64C(0xffffff8000000000)),
      static_cast<Mint64>(MUINT64C(0xffff800000000000)),
      static_cast<Mint64>(MUINT64C(0xff80000000000000)),
      static_cast<Mint64>(MUINT64C(0x8000000000000000))
   };

MVariant MUtilities::FromINT(const MByteString& bytes, bool littleEndian)
{
   MVariant result;
   unsigned size = static_cast<unsigned>(bytes.size());
   MENumberOutOfRange::Check(1u, 8u, size, "size");
   if ( size < 4 )
   {
      int val = 0;
      DoCopyWithPossibleSwap((char*)&val, bytes.data(), size, littleEndian);
      if ( (val & s_intBits32[size]) != 0 )
         val |= s_intBits32[size];
      result.DoAssignToEmpty(val);
   }
   else if ( size == 4 )
   {
      int val = 0;
      DoCopyWithPossibleSwap((char*)&val, bytes.data(), size, littleEndian);
      result.DoAssignToEmpty(val);
   }
   else if ( size == 8 )
   {
      Mint64 val = MUINT64C(0);
      DoCopyWithPossibleSwap((char*)&val, bytes.data(), size, littleEndian);
      result.DoAssignToEmpty(double(val));
   }
   else
   {
      Mint64 val = MUINT64C(0);
      DoCopyWithPossibleSwap((char*)&val, bytes.data(), size, littleEndian);
      if ( (val & s_intBits64[size]) != 0 )
         val |= s_intBits64[size];
      result.DoAssignToEmpty(double(val));
   }
   return result;
}

MByteString MUtilities::ToINT(const MVariant& value, unsigned size, bool littleEndian)
{
   MByteString result;
   MENumberOutOfRange::Check(1, 8, size, "size");
   if ( value.GetType() == MVariant::VAR_BYTE_STRING )
      DoCopyResultWithPossibleSwap(result, value, size, littleEndian, false);
   else
   {
      char buff [ 8 ];
      if ( size <= 3 ) // we can't use this branch for size == 4 due to unsigned types that are larger than INT
      {
         int val = value.AsInt();
         int minimum = s_intBits32[size];
         int maximum = ~minimum;
         MENumberOutOfRange::CheckIntegerRange(minimum, maximum, val);
         DoCopyWithPossibleSwap(buff, (const char*)&val, size, littleEndian);
      }
      else if ( size == 4 )
      {
         int val = value.AsInt();
         DoCopyWithPossibleSwap(buff, (const char*)&val, size, littleEndian);
      }
      else
      {
         double doubleValue = MMath::Round0(value.AsDouble());
         double minimum = (double)s_intBits64[size];
         double maximum = (double)~s_intBits64[size];
         MENumberOutOfRange::Check(minimum, maximum, doubleValue);
         Mint64 val = (Mint64)doubleValue;  // we've rounded the value already, just convert
         DoCopyWithPossibleSwap(buff, (const char*)&val, size, littleEndian);
      }
      result.assign(buff, size);
   }
   return result;
}

#endif // !M_NO_REFLECTION

double MUtilities::FromDSPFloatBuffer(const char* buffer, unsigned size)
{
   if ( size == 3 )
   {
      int exponent = static_cast<int>(static_cast<signed char>(buffer[2]));
      M_ASSERT(exponent >= -128 && exponent < 128);
      return FromDSPIntBuffer(buffer, 2) * pow(2.0, static_cast<double>(exponent));
   }
   if ( size != 4 )
   {
      MException::Throw(M_CODE_STR(M_ERR_BAD_VALUE_FOR_DSP_TYPE, M_I("Bad value for DSP type")));
      M_ENSURED_ASSERT(0);
   }
   int exponent = static_cast<int>(static_cast<signed char>(buffer[0]));
   M_ASSERT(exponent >= -128 && exponent < 128);
   unsigned wholeMantissa = (unsigned)(unsigned char)buffer[1] |
                           ((unsigned)(unsigned char)buffer[2] << 8) |
                           ((unsigned)(unsigned char)buffer[3] << 16);
   if ( buffer[3] & 0x80 ) // negative
      wholeMantissa |= 0xFF000000;
   double mantissa = static_cast<double>(static_cast<int>(wholeMantissa)) / (double)0x800000;
   return mantissa * MMath::Pow2(exponent);
}

double MUtilities::FromDSPFloat(const MByteString& buffer)
{
   return FromDSPFloatBuffer(buffer.data(), static_cast<unsigned>(buffer.size()));
}

double MUtilities::FromDSPIntBuffer(const char* buffer, unsigned size)
{
   const char* c = buffer + size - 1;
   double result = static_cast<double>(*c);  // first value is not watched for range
   double factor = 128.0;
   for ( --c ; c >= buffer; --c )
   {
      unsigned char byte = static_cast<unsigned char>(*c);
      byte &= ~0x80; // the upper bit has to be ignored
      result = 128.0 * result + static_cast<double>(byte);
      factor *= 128.0;
   }
   result /= factor;
   return (result >= 1.0) ? result - 2.0 : result;
}

double MUtilities::FromDSPInt(const MByteString& buffer)
{
   return FromDSPIntBuffer(buffer.data(), static_cast<unsigned>(buffer.size()));
}

   static bool DoToDSPIntBuffer(double value, char* buffer, unsigned size)
   {
      M_ASSERT(size < 8); // 8 and bigger is not supported

      double result = value;
      if ( value < 0.0 )
         result += 2.0;

      for ( unsigned i = size; i > 0; --i ) // Scale up, this is faster and more precise than pow for small size
         result *= 128;
      result += 0.5;                  // Round the number
      Muint64 mask = (Muint64)result; // This will truncate the number
      char* c = buffer;
      const char* end = c + size - 1;
      for ( ; c < end; ++c )
      {
         *c = static_cast<char>(static_cast<unsigned char>(mask & 0x7Fu));
         mask >>= 7;
      }
      *c = static_cast<char>(static_cast<unsigned char>(mask)); // last char has a sign
      mask >>= (value < 0.0) ? 8 : 7;
      return mask == 0;
   }

void MUtilities::ToDSPFloatBuffer(double value, char* buffer, unsigned size)
{
   int exponent;
   double mantissa = frexp(value, &exponent);
   if ( exponent < 128 || exponent >= -128 ) // otherwise throw
   {
      if ( size == 3 ) // three-byte format
      {
         unsigned last = size - 1;
         if ( !DoToDSPIntBuffer(mantissa, buffer, last) ) // if we did not fit due to rounding:
         {                                                // 0.0999999 would round to 0.1 and yield bad mantissa above
            mantissa /= 2.0;
            ++exponent;
            bool success = DoToDSPIntBuffer(mantissa, buffer, last);
            M_USED_VARIABLE(success);
            M_ASSERT(success); // now we should fit unconditionally, only one bit could be up
         }
         if ( exponent < 0 )
            exponent += 256;
         buffer[last] = static_cast<char>(static_cast<unsigned char>(exponent));
         return; // success
      }
      else if ( size == 4 ) // four-byte format (otherwise, if no three- or four-byte throw an error
      {
         buffer[0] = static_cast<char>(static_cast<signed char>(exponent));

         Muint32 mask = static_cast<Muint32>(static_cast<int>(mantissa * (double)0x800000));
         char* c = buffer + 1;
         const char* end = c + 3;
         for ( ; c < end; ++c )
         {
            *c = static_cast<char>(static_cast<unsigned char>(mask & 0xFFu));
            mask >>= 8;
         }
         return; // success
      }
   }
   // otherwise throw

   MException::Throw(M_CODE_STR(M_ERR_BAD_VALUE_FOR_DSP_TYPE, M_I("Bad value for DSP type")));
   M_ENSURED_ASSERT(0);
}

MByteString MUtilities::ToDSPFloat(double value, unsigned size)
{
   char buffer [ 64 ];
   if ( size > sizeof(buffer) )
      size = sizeof(buffer);    // restrict the buffer, exception will be thrown anyway
   ToDSPFloatBuffer(value, buffer, size);
   return MByteString(buffer, size);
}

void MUtilities::ToDSPIntBuffer(double value, char* buffer, unsigned size)
{
   if ( !DoToDSPIntBuffer(value, buffer, size) )
   {
      MENumberOutOfRange::ThrowRange(-1.0, 1.0, value);
      M_ENSURED_ASSERT(0);
   }
}

MByteString MUtilities::ToDSPInt(double value, unsigned size)
{
   char buffer [ 64 ];
   if ( size > sizeof(buffer) )
      size = sizeof(buffer);    // restrict the buffer, exception will be thrown anyway
   ToDSPIntBuffer(value, buffer, size);
   return MByteString(buffer, size);
}

#if !M_NO_FILESYSTEM

   #if (M_OS & M_OS_WIN32_CE) != 0

      static MCriticalSection s_savedCurrentPathCriticalSection;
      static MStdString       s_savedCurrentPath(1, '\\');

   #endif

MStdString MUtilities::GetCurrentPath()
{
   #if (M_OS & M_OS_WIN32_CE) != 0
      MCriticalSection::Locker lock(s_savedCurrentPathCriticalSection);
      return s_savedCurrentPath;
   #else
      #if (M_OS & M_OS_WINDOWS) != 0
         #if M_UNICODE
            wchar_t dir [ M_MAX_PATH ];
         #else
            char dir [ M_MAX_PATH ];
         #endif
         DWORD ret = ::GetCurrentDirectory(M_NUMBER_OF_ARRAY_ELEMENTS(dir), dir);
      #elif (M_OS & M_OS_POSIX) != 0
         char dir [ M_MAX_PATH ];
         char* ret = getcwd(dir, M_NUMBER_OF_ARRAY_ELEMENTS(dir));
      #else
         #error "Operating system is not supported"
      #endif
      MESystemError::CheckLastSystemError(ret == 0);
      MStdString path = MToStdString(dir);
      MAddDirectorySeparatorIfNecessary(path);
      return path;
   #endif
}

void MUtilities::SetCurrentPath(const MStdString& thePath)
{
   MStdString path = thePath;

#if (M_OS & M_OS_ANDROID) != 0
   if ( !path.empty() && *path.begin() == ':' ) // path in the assets, cannot set
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_INVALID_OPERATION_ON_APK_ASSET, "Invalid operation on apk asset"));
      M_ENSURED_ASSERT(0);
   }
#endif

   if ( IsPathExisting(path) && !IsPathDirectory(path) ) // if a file is given extract directory, handle "::" differently than GetPathDirectory!
      path = GetPathDirectory(path);

   #if (M_OS & M_OS_WIN32_CE) != 0
      MCriticalSection::Locker lock(s_savedCurrentPathCriticalSection);
      s_savedCurrentPath = path;
      MAddDirectorySeparatorIfNecessary(s_savedCurrentPath);
   #else
      #if (M_OS & M_OS_WINDOWS) != 0
         #if M_UNICODE
            DWORD ret = ::SetCurrentDirectory(MToWideString(path).c_str());
         #else
            DWORD ret = ::SetCurrentDirectory(path.c_str());
         #endif
      #elif (M_OS & M_OS_POSIX) != 0
         int ret = !chdir(path.c_str());  // chdir() returns zero on success!!!
      #else
         #error "Operating system is not supported"
      #endif
      MESystemError::CheckLastSystemError(ret == 0);
   #endif
}
#endif

MStdString MUtilities::GetLocalHostName()
{
   #if !M_NO_SOCKETS
      try
      {
         return MStreamSocket::GetLocalName();
      }
      catch ( MESocketError& )
      {
      }
   #endif
   return "localhost";
}

#if !M_NO_FILESYSTEM

   static void DoSplitPath(const MStdString& path, MStdString* dir = NULL, MStdString* name = NULL, MStdString* ext = NULL)
   {
      if ( path.size() > M_MAX_PATH )
      {
         MException::Throw(M_ERR_FILE_PATH_TOO_LONG, M_I("File path too long"));
         M_ENSURED_ASSERT(0);
      }

      MStdString::const_iterator itEnd = path.end();
      MStdString::const_iterator lastPeriod = itEnd;
      MStdString::const_iterator lastSlash = itEnd;
      MStdString::const_iterator it = path.begin();

#if (M_OS & M_OS_ANDROID) != 0
      if ( !path.empty() && *path.begin() == ':' ) // full path in the assets
         lastSlash = it++;                         // is like a directory separator
#endif

      for ( ; it != itEnd; ++it )
      {
         switch ( *it )
         {
         case '.':
            lastPeriod = it;
            break;
         case '/':  // treat both POSIX and Windows paths...
         case '\\':
            lastPeriod = path.end(); // invalidate periods prior to slash
            lastSlash = it;
            break;
         default:
            ;
         }
      }

      if ( dir != NULL )
      {
         if ( lastSlash == itEnd )
            dir->clear();
         else
            dir->assign(path.begin(), lastSlash + 1); // directory name ends with a slash
      }

      if ( name != NULL )
      {
         MStdString::const_iterator nameStart = (lastSlash == itEnd) ? path.begin() : lastSlash + 1;
         name->assign(nameStart, lastPeriod); // lastPeriod can be equal to itEnd
      }

      if ( ext != NULL )
      {
         if ( lastPeriod == itEnd )
            ext->clear();
         else
            ext->assign(lastPeriod, itEnd);
      }
   }

MStdString MUtilities::GetPathExtension(const MStdString& path)
{
   MStdString ext;
   DoSplitPath(path, NULL, NULL, &ext);
   return ext;
}

MStdString MUtilities::GetPathFileName(const MStdString& path)
{
   MStdString name;
   DoSplitPath(path, NULL, &name, NULL);
   return name;
}

MStdString MUtilities::GetPathFileNameAndExtension(const MStdString& path)
{
   MStdString name;
   MStdString ext;
   DoSplitPath(path, NULL, &name, &ext);
   name += ext;
   return name;
}

MStdString MUtilities::GetPathDirectory(const MStdString& path)
{
   MStdString dir;
   DoSplitPath(path, &dir, NULL, NULL);
   return dir;
}

MStdString MUtilities::GetPath(const MStdString& dir, const MStdString& name, const MStdString& extension)
{
   MStdString result = dir;
   MAddDirectorySeparatorIfNecessary(result);
   result += name;
   if ( !extension.empty() )
   {
      if ( extension[0] != '.' )
         result += '.';
      result += extension;
   }
   return result;
}

MStdString MUtilities::MergePaths(const MStdString& dir, const MStdString& name)
{
   if ( IsPathFull(name) )
      return name;

   MStdString result;
   MStdString::size_type doubleColon = dir.find("::");
   if ( doubleColon != MStdString::npos )
      result.assign(dir.begin(), dir.begin() + doubleColon + 2); // assign name up to ::
   else
   {
      if ( IsPathExisting(dir) && !IsPathDirectory(dir) )
         result = GetPathDirectory(dir);
      else
         result = dir;
      MAddDirectorySeparatorIfNecessary(result);
   }
   result += name;
   return result;
}

bool MUtilities::IsPathFull(const MStdString& path)
{
   if ( !path.empty() && (*path.begin() == '\\' || *path.begin() == '/') ) // full path already
      return true;

   #if (M_OS & M_OS_ANDROID) != 0
      if ( !path.empty() && *path.begin() == ':' ) // full path in the assets
         return true;
   #endif
   #if (M_OS & M_OS_WINDOWS) != 0
      if ( path.size() >= 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/') )
      {
         MChar c = (MChar)m_toupper(*path.begin());
         if ( c >= 'A' && c <= 'Z' )
            return true;
      }
   #endif
   return false;
}

   static bool DoTestPathExists(const MStdString& path, bool andAlsoDirectory)
   {
      try
      {
         {
            #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
               if ( !path.empty() && *path.begin() == ':' ) // path in the assets
               {
                  if ( path.size() == 1 )
                     return true; // path exists, and it is a directory

                  AAssetManager* assetManager = MJavaEnv::GetJniAssetManager();
                  const char* name = path.c_str() + 1;
                  AAsset* asset = AAssetManager_open(assetManager, name, AASSET_MODE_STREAMING);
                  if ( asset != NULL )
                  {
                     AAsset_close(asset);
                     return andAlsoDirectory ? false : true;
                  }
                  const MStdString& superdir = MUtilities::GetPathDirectory(path);
                  const MStdStringVector& dirs = MUtilities::FindDirectories(superdir, "*");
                  const MStdString& subfile = MUtilities::GetPathFileNameAndExtension(path);
                  return std::find(dirs.begin(), dirs.end(), subfile) != dirs.end(); // exists and directory
               }
            #endif
            #if (M_OS & M_OS_POSIX) != 0
               struct stat st;
               int ret = stat(path.c_str(), &st);
               MESystemError::ClearGlobalSystemError();
               if ( ret != 0 )
                  return false;
               if ( andAlsoDirectory && !S_ISDIR(st.st_mode) )
                  return false;
            #else
               #if M_UNICODE
                  DWORD attributes = ::GetFileAttributes(MToWideString(path).c_str());
               #else
                  DWORD attributes = ::GetFileAttributes(path.c_str());
               #endif
               MESystemError::ClearGlobalSystemError();
               if ( attributes == 0xFFFFFFFFu )
                  return false;
               if ( andAlsoDirectory &&  (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
                  return false;
            #endif
         }
         return true;
      }
      catch ( ... )
      {
         return false;
      }
   }

bool MUtilities::IsPathDirectory(const MStdString& path) M_NO_THROW
{
   return DoTestPathExists(path, true);
}

bool MUtilities::IsPathExisting(const MStdString& path) M_NO_THROW
{
   return DoTestPathExists(path, false);
}

MStdString MUtilities::GetFullPath(const MStdString& path)
{
   MStdString result;
   if ( IsPathFull(path) )
      result = path;
   else
   {
      result = GetCurrentPath();
      result += path;
   }
   return result;
}

void MUtilities::CopyFile(const MStdString& source, const MStdString& destination)
{
   MStreamFile from(source, MStreamFile::FlagReadOnly, MStreamFile::SharingAllowRead);
   MStreamFile to(destination, MStreamFile::FlagWriteOnly | MStreamFile::FlagCreate | MStreamFile::FlagTruncate, MStreamFile::SharingAllowNone);

   char buffer [ 0x1000 ]; // 4k
   unsigned len;
   do
   {
      len = from.ReadAvailableBytes(buffer, sizeof(buffer));
      to.WriteBytes(buffer, len); // len can be zero, which is okay
   } while ( len == sizeof(buffer) );
}

void MUtilities::MoveFile(const MStdString& source, const MStdString& destination)
{
   #if (M_OS & M_OS_POSIX) != 0
      int result = ::m_rename(source.c_str(), destination.c_str());
      MESystemError::CheckLastSystemError(result != 0);
   #else
      #if M_UNICODE
         BOOL result = ::MoveFile(MToWideString(source).c_str(), MToWideString(destination).c_str());
      #else
         BOOL result = ::MoveFile(source.c_str(), destination.c_str());
      #endif
      MESystemError::CheckLastSystemError(result == FALSE);
   #endif
}

   static void DoDeleteOneFile(MConstChars name)
   {
      #if (M_OS & M_OS_POSIX) != 0
         int ret = !unlink(name);
      #else
         #if M_UNICODE
            DWORD ret = ::DeleteFile(MToWideString(name).c_str());
         #else
            DWORD ret = ::DeleteFile(name);
         #endif
      #endif
      MESystemError::CheckLastSystemError(!ret);
   }

   static void DoDeleteOneDirectory(MConstChars name)
   {
      MConstChars subName;
      MFindFile f(name, "*", true, true); // find directories taking libraries as regular files
      while ( (subName = f.FindNext(true)) != NULL )
         DoDeleteOneDirectory(subName); // recurse

      f.Init(name, "*", false, true); // find files taking libraries as regular files
      while ( (subName = f.FindNext(true)) != NULL )
         DoDeleteOneFile(subName); // recurse

      #if (M_OS & M_OS_POSIX) != 0
         int ret = !rmdir(name);
      #else
         #if M_UNICODE
            DWORD ret = ::RemoveDirectory(MToWideString(name).c_str());
         #else
            DWORD ret = ::RemoveDirectory(name);
         #endif
      #endif
      MESystemError::CheckLastSystemError(!ret);
   }

void MUtilities::DeleteFile(const MStdString& name)
{
   try
   {
      MConstChars cName = name.c_str();
      if ( IsPathDirectory(name) )
         DoDeleteOneDirectory(cName);
      else
         DoDeleteOneFile(cName);
   }
   catch ( MException& ex )
   {
      ex.AppendToString(M_I(" when deleting '%s'"), name.c_str());
      throw;
   }
}

   static bool DoCreateDirectory(MStdString name) // string is modifiable inside
   {
      if ( name.empty() )
      {
         MException::ThrowNoValue();
         M_ENSURED_ASSERT(0);
      }

      if ( *name.rbegin() == '\\' || *name.rbegin() == '/' ) // erase trailing slash
         name.erase(name.size() - 1);

      if ( name.empty() )
      {
         MException::ThrowNoValue();
         M_ENSURED_ASSERT(0);
      }

      const MStdString& subName = MUtilities::GetPathDirectory(name);
      if ( !subName.empty() && !MUtilities::IsPathDirectory(subName) && !DoCreateDirectory(subName) ) // recurse
         return false;

      // Now we are ready to back-recurse the path creation
      //
      #if (M_OS & M_OS_POSIX) != 0
         return mkdir(name.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == 0; // POSIX call succeeds with returning zero
      #else
         #if M_UNICODE
            MWideString str = MToWideString(name);
            return ::CreateDirectoryW(str.c_str(), NULL) != 0; // Windows call succeeds with returning nonzero
         #else
            return ::CreateDirectoryA(name.c_str(), NULL) != 0; // Windows call succeeds with returning nonzero
         #endif
      #endif
   }

void MUtilities::CreateDirectory(const MStdString& name)
{
   bool done = DoCreateDirectory(name);
   MESystemError::CheckLastSystemError(!done);
}

void MUtilities::EnsureDirectoryExistsForFile(const MStdString& name)
{
   MStdString dir = MUtilities::GetPathDirectory(name);
   if ( !MUtilities::IsPathDirectory(dir) )
      CreateDirectory(dir);
}

MStdStringVector MUtilities::FindFiles(const MStdString& directory, const MStdString& mask)
{
   MStdStringVector result;
   MFindFile::Populate(result, directory, mask, false, false, false);
   return result;
}

MStdStringVector MUtilities::FindDirectories(const MStdString& directory, const MStdString& mask)
{
   MStdStringVector result;
   MFindFile::Populate(result, directory, mask, true, false, false);
   return result;
}

MStdString MUtilities::GetModulePath() M_NO_THROW
{
   MStdString result;
#if M_OS & M_OS_WINDOWS
#if M_UNICODE
   wchar_t buff [ M_MAX_PATH ];
   if ( ::GetModuleFileName(NULL, buff, M_NUMBER_OF_ARRAY_ELEMENTS(buff)) != 0 )
      result = MToStdString(buff);
#else
   char buff [ M_MAX_PATH ];
   if ( ::GetModuleFileName(NULL, buff, M_NUMBER_OF_ARRAY_ELEMENTS(buff)) != 0 )
      result = buff;
#endif

   MStdString::size_type backslash = result.rfind('\\');
   if ( backslash < result.size() ) // cut the string next to the trailing backslash, return directory only
      result.erase(result.begin() + backslash + 1, result.end());
#elif M_OS & M_OS_NUTTX
   return result;
#elif M_OS & M_OS_POSIX
   char buf[M_MAX_PATH] = {0};
   MFormat(buf, M_MAX_PATH, "/proc/%d/exe", getpid());
   ssize_t status = readlink(buf, buf, M_MAX_PATH);
   if ( status >= 0 )
   {
       char* lastslash = strrchr(buf, '/');
       if ( lastslash != NULL )
           result.assign(buf, lastslash + 1);
   }
#endif
   return result;
}

MStdString MUtilities::GetInstallationPath() M_NO_THROW
{
   MStdString path;
#if !(M_OS & M_OS_NUTTX)
   try
   {
      path = GetModulePath(); // this will be used as a default value in case of absence of the key
      int possibleSlashCharOffset = (int)path.size() - 5;
      if ( possibleSlashCharOffset > 0 && m_stricmp(path.c_str() + possibleSlashCharOffset, "\\bin\\") == 0 )
         path.erase(possibleSlashCharOffset + 1, path.size());

      MAddDirectorySeparatorIfNecessary(path);
   }
   catch ( ... )
   {
      path.clear();
   }
#endif
   return path;
}
#endif // !M_NO_FILESYSTEM

#if !M_NO_VARIANT
MByteString MUtilities::BytesToHex(const MByteString& bytes, const MVariant& format)
{
   MByteString result;
   if ( format.IsNumeric() ) // also boolean
      result = BufferToHex(bytes.data(), M_64_CAST(unsigned, bytes.size()), format.AsBool());
   else
      result = BufferToHex(bytes.data(), M_64_CAST(unsigned, bytes.size()), format.AsString());
   return result;
}

MStdString MUtilities::BytesToHexString(const MByteString& bytes, const MVariant& format)
{
   return BytesToHex(bytes, format); // this is the same currently
}
#endif // !M_NO_VARIANT

MByteString MUtilities::BufferToHex(const char* bytes, unsigned len, bool useBlanks)
{
   MByteString result;
   if ( len > 0 )
   {
      const size_t unitSize = useBlanks ? 3 : 2;
      result.reserve(unitSize * len);
      char temp[3];
      temp[2] = ' ';
      for ( ; ; )
      {
         unsigned byte = (unsigned)(unsigned char)(*bytes++);
         temp[0] = DoNumberToHexByte(byte >> 4);
         temp[1] = DoNumberToHexByte(byte & 0xF);
         --len;
         if ( len == 0 )
         {
            result.append(temp, 2); // avoid adding a trailing blank when useBlanks is true (tests were broken)
            break;
         }
         result.append(temp, unitSize);
      }
   }
   return result;
}

   inline bool IsHexSpace(char c)
   {
      return isspace(c) || c == '_' || c == '-';
   }

#if !M_NO_VARIANT

MByteString MUtilities::BufferToHex(const char* bytes, unsigned length, const MByteString& format)
{
   MByteString result;

   bool fmtGood = false; // until we see '[xX]' the format is bad
   MByteString::const_iterator itFormat = format.begin();
   MByteString::const_iterator itFormatEnd = format.end();
   for ( ; itFormat < itFormatEnd; ++itFormat )
   {
      if ( *itFormat == 'x' || *itFormat == 'X' )
      {
         ++itFormat;
         if ( itFormat == itFormatEnd || (*itFormat != *(itFormat - 1)) )
         {
            fmtGood = false;
            break;
         }
         fmtGood = true; // saw "XX" or "xx"
      }
      else if ( !IsHexSpace(*itFormat) )
      {
         fmtGood = false;
         break;
      }
   }
   if ( !fmtGood )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_INVALID_HEX_FORMAT, "Invalid HEX display format: '%s'", format.c_str()));
      M_ENSURED_ASSERT(0);
   }

   if ( length > 0 )
   {
      char tmp [ 2 ];
      itFormat = format.begin(); // reset the format
      const char* bytesEnd = bytes + length;
      for ( ; ; )
      {
         if ( *itFormat == 'X' )
         {
            M_ASSERT(*(itFormat + 1) == 'X'); // checked the format validity already
            unsigned byte = static_cast<unsigned>(static_cast<Muint8>(*bytes++));
            tmp[0] = DoNumberToHexByte(byte >> 4);
            tmp[1] = DoNumberToHexByte(byte & 0x0F);
            result.append(tmp, 2);
            if ( bytes == bytesEnd )
               break;
            itFormat += 2;
         }
         else if ( *itFormat == 'x' )
         {
            M_ASSERT(*(itFormat + 1) == 'x'); // checked the format validity already
            unsigned byte = static_cast<unsigned>(static_cast<Muint8>(*bytes++));
            tmp[0] = DoNumberToHexByteLowerCase(byte >> 4);
            tmp[1] = DoNumberToHexByteLowerCase(byte & 0x0F);
            result.append(tmp, 2);
            if ( bytes == bytesEnd )
               break;
            itFormat += 2;
         }
         else
         {
            result += (char)*itFormat;
            ++itFormat;
         }
         if ( itFormat == itFormatEnd )
            itFormat = format.begin();
      }
   }
   return result;
}

#endif // !M_NO_VARIANT

MByteString MUtilities::HexToBytes(const MByteString& hexString)
{
   return HexBufferToBytes(hexString.data(), M_64_CAST(unsigned, hexString.size()));
}

MByteString MUtilities::HexStringToBytes(const MStdString& hexString)
{
   return HexToBytes(hexString);
}

MByteString MUtilities::HexBufferToBytes(const char* buff, unsigned length)
{
   MByteString result;
   result.reserve(length / 2u);

   const char* buffEnd = buff + length;
   for ( ; buff < buffEnd; ++buff )
   {
      if ( !IsHexSpace(*buff) ) // skip whitespace
      {
         unsigned byte = HexCharToNumber(*buff);
         do // skip whitespace
         {
            ++buff;
            if ( buff == buffEnd )
            {
               MException::Throw(M_CODE_STR(M_ERR_SUPPLY_EVEN_NUMBER_OF_HEX_CHARACTERS_TWO_FOR_EACH_BYTE, M_I("Supply even number of hexadecimal characters, two for each byte")));
               M_ENSURED_ASSERT(0);
            }
         } while ( IsHexSpace(*buff) );
         byte = (byte << 4) | HexCharToNumber(*buff);
         result += static_cast<char>(byte);
      }
   }
   return result;
}

MStdString MUtilities::BytesToNumericString(const MByteString& bytes, const MStdString& format)
{
   return BufferToNumericString(bytes.data(), static_cast<unsigned>(bytes.size()), format.c_str());
}

MStdString MUtilities::BufferToNumericString(const char* buff, unsigned length, const char* format)
{
   MStdString result;

   if ( format == NULL || *format == '\0' )
      format = "b.";
   else // otherwise check the format
   {
      bool fmtGood = false; // until we see '[bB]' the format is bad
      const char* p = format;
      for ( ; *p != '\0'; ++p )
      {
         if ( *p == 'b' || *p == 'B' )
         {
            if ( p != format && (*(p - 1) == 'b' || *(p - 1) == 'B' ) ) // two digits cannot appear together
            {
               fmtGood = false;
               break;
            }
            fmtGood = true;
         }
         else if ( !isascii(*p) || isalnum(*p) )
         {
            fmtGood = false;
            break;
         }
      }
      if ( !fmtGood )
      {
         MException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_INVALID_NUMERIC_STRING_FORMAT_S1, "Invalid numeric string format '%s'", format));
         M_ENSURED_ASSERT(0);
      }
   }

   if ( length > 0 )
   {
      const char* buffEnd = buff + length;
      const char* p = format;
      for ( ; ; )
      {
         if ( *p == 'b' || *p == 'B' )
         {
            char tmp [ 16 ];
            unsigned value = (unsigned)(Muint8)*buff;
            result.append(MToChars(value, tmp));
            ++buff;
            if ( buff == buffEnd )
               break;
         }
         else
            result += *p;
         ++p;
         if ( *p == '\0' )
         {
            M_ASSERT(p != format); // format is sure not empty
            p = format; // start over
         }
      }
   }
   return result;
}

MByteString MUtilities::NumericStringToBytes(const MStdString& numericString)
{
   return NumericBufferToBytes(numericString.data(), static_cast<unsigned>(numericString.size()));
}

MByteString MUtilities::NumericBufferToBytes(const char* buff, unsigned length)
{
   MByteString result;
   const char* buffEnd = buff + length;
   while ( buff < buffEnd )
   {
      char c = *buff;
      if ( c <= '9' && c >= '0' ) // avoid internationalization issues of standard isdigit() call and hard-code the efficient ASCII check
      {
         unsigned long value = 0;
         do // hand made conversion to avoid copying of buffers
         {
            value *= 10;
            value += c - '0';
            ++buff;
            if ( buff == buffEnd || value >= 0x0FFFFFFF ) // avoid bad match due to overflow
               break;
            c = *buff;
         } while ( c <= '9' && c >= '0' );
         MENumberOutOfRange::CheckNamedUnsignedLongRange(0, 255, value, "byte");
         result += static_cast<char>(value);
      }
      else if ( !isascii(c) || isalpha(c) )
      {
         MException::Throw(M_CODE_STR(M_ERR_INVALID_CHARACTER_IN_NUMERIC_STRING, M_I("Invalid character in numeric string")));
         M_ENSURED_ASSERT(0);
      }
      else
          ++buff;
   }
   return result;
}

#if !M_NO_VARIANT
   static void DoAppendMDLConstant(MStdString& result, const MVariant& v, bool relaxed)
   {
      MVariant::Type type = v.GetType();
      switch ( type )
      {
      case MVariant::VAR_EMPTY:
         result.append("EMPTY", 5);
         break;
      case MVariant::VAR_BOOL:
         if ( relaxed )
            result += v.AsString();
         else
         {
            if ( v.DoInterpretAsBool() )
               result.append("TRUE", 4);
            else
               result.append("FALSE", 5);
         }
         break;
      case MVariant::VAR_BYTE:
      case MVariant::VAR_CHAR:
         result += MStr::CharToQuotedEscapedString((char)v.AsByte());        // 'quoted' string. . Notice the char is always a byte
         break;
      case MVariant::VAR_UINT:
         result += v.AsString();
         if ( !relaxed )
            result += 'u';
         break;
      case MVariant::VAR_BYTE_STRING:
         if ( !relaxed )
            result += 'b'; // byte string prefix is only necessary for nonrelaxed
         result += MStr::ToString(v.DoInterpretAsByteString(), MStr::StrQuote | MStr::StrKeepSideBlanks);
         break;
      case MVariant::VAR_STRING:
         result += MStr::ToString(v.DoInterpretAsString(), MStr::StrQuote | MStr::StrKeepSideBlanks);  // "quoted" string
         break;
      case MVariant::VAR_STRING_COLLECTION:
      case MVariant::VAR_VARIANT_COLLECTION:
         {
            result += (type == MVariant::VAR_STRING_COLLECTION) ? '[' : '{';
            int num = v.GetCount();
            if ( num != 0 )
            {
               for ( int i = 0; ; )
               {
                  DoAppendMDLConstant(result, v.GetItem(i), relaxed);
                  ++i;
                  if ( i == num )
                     break;
                  result.append(", ", 2);
               }
            }
            result += (type == MVariant::VAR_STRING_COLLECTION) ? ']' : '}';
            break;
         }
      case MVariant::VAR_MAP:
         {
            result += '{';
            int num = v.GetCount();
            if ( num == 0 )
               result += ':';
            else
            {
               for ( int i = 0; ; )
               {
                  DoAppendMDLConstant(result, v.GetMapKeyByIndex(i), relaxed);
                  result.append(" : ", 3);
                  DoAppendMDLConstant(result, v.GetMapValueByIndex(i), relaxed);
                  ++i;
                  if ( i == num )
                     break;
                  result.append(", ", 2);
               }
            }
            result += '}';
            break;
         }
      default:
         // such as MVariant::VAR_INT, MVariant::VAR_DOUBLE and so on
         result += v.AsString();
         break;
      }
   }

MStdString MUtilities::ToMDLConstant(const MVariant& v)
{
   MStdString result;
   DoAppendMDLConstant(result, v, false);
   return result;
}

MStdString MUtilities::ToRelaxedMDLConstant(const MVariant& v)
{
   MStdString result;
   DoAppendMDLConstant(result, v, true);
   return result;
}

MVariant MUtilities::FromMDLConstant(const MStdString& v)
{
   return MVariantParser::FromMDLConstant(v);
}
#endif // !M_NO_VARIANT

MVersion& MUtilities::GetVersion()
{
   static MVersion ver(M_SDK_VERSION_STRING, true);
   return ver;
}

MVersion& MUtilities::GetProductVersion()
{
   static MVersion ver(M_PRODUCT_VERSION_STRING, true); // read-only version
   return ver;
}

MStdString MUtilities::GetProductName()
{
   return M_PRODUCT_NAME;
}

   class OsNameAndVersionHelper
   {
      MCriticalSection m_section;
      MStdString       m_name;
      MVersion         m_version;

   public:

      OsNameAndVersionHelper()
      {
         MCriticalSection::Locker locker(m_section);
         #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
            MStdString versionString;
            m_name = "Android";
            MJavaEnv env;
            jclass clazz = env.FindClass("android/os/Build$VERSION");  // or "android/os/Build/VERSION/RELEASE"?
            jfieldID fid = env.GetStaticFieldID(clazz, "RELEASE", "Ljava/lang/String;");
            jstring jVersionStr = (jstring) env->GetStaticObjectField(clazz, fid);
            if ( jVersionStr != NULL )
            {
               const char* cVersionStr = env->GetStringUTFChars(jVersionStr, NULL);
               env.CheckForJavaException();
               versionString = cVersionStr;
               env->ReleaseStringUTFChars(jVersionStr, cVersionStr);
               env->DeleteLocalRef(jVersionStr);
            }
            m_version.SetAsString(versionString);
         #elif (M_OS & M_OS_POSIX) != 0
             MStdString versionString;
             struct utsname un;
             if ( uname(&un) != 0 ) // success
             {
                // There is an alternative Linux-only way of reading
                // "/proc/sys/kernel/osrelease" and "/proc/sys/kernel/ostype",
                // skip the hassle and report an error if uname fails
                MESystemError::ThrowLastSystemError();
                M_ENSURED_ASSERT(0);
             }
             m_name = un.sysname;
             versionString = un.release;
             MStdString::size_type pos = versionString.find_first_not_of("012345678.");
             if ( pos != std::string::npos )
             {  // Then remove trailing '.' as in case of versions like "3.10.1.a" so it becomes "3.10.1"
                while ( pos > 0 && versionString[pos - 1] == '.' )
                   --pos;
                versionString.resize(pos);
             }
             m_version.SetAsString(versionString);
         #elif (M_OS & M_OS_WINDOWS) != 0
            OSVERSIONINFO osvi;
            ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

            #if defined(_MSC_VER) && _MSC_VER >= 1600   // if this is Visual C++ 2010 or later
               #pragma warning(push)
               #pragma warning(disable : 4996) // disable "GetVersionEx was declared deprecated"
            #endif
            ::GetVersionEx(&osvi);
            #if defined(_MSC_VER) && _MSC_VER >= 1600   // if this is Visual C++ 2010 or later
               #pragma warning(pop)
            #endif

            m_version.SetCount(2);
            m_version.SetItem(0, osvi.dwMajorVersion);
            m_version.SetItem(1, osvi.dwMinorVersion);
            #if (M_OS & M_OS_WIN32_CE) != 0
               m_name = "Windows CE";
            #else
               m_name = "Windows";
            #endif
         #else
            #error "Add OS name and version code!"
         #endif
         m_version.SetIsReadOnly(true);
      }

      MVersion& GetVersion()
      {
         return m_version;
      }

      const MStdString& GetName() const
      {
         return m_name;
      }

      static OsNameAndVersionHelper& GetSelf();
   };

   OsNameAndVersionHelper& OsNameAndVersionHelper::GetSelf()
   {
      static OsNameAndVersionHelper s_self; // Meyer's singleton
      return s_self;
   }

MStdString MUtilities::GetOperatingSystemName()
{
   return OsNameAndVersionHelper::GetSelf().GetName();
}

MVersion& MUtilities::GetOperatingSystemVersion()
{
   return OsNameAndVersionHelper::GetSelf().GetVersion();
}

double MUtilities::FromINSTRBuffer(const char* data, unsigned size)
{
   M_ASSERT(size == 2);
   unsigned rawNumber = 0;
   unsigned rawFactor = 0x1;
   for ( unsigned int i = 0; i < size ; ++i )
   {
      rawNumber += (unsigned char)(*data++) * rawFactor;
      rawFactor *= 0x100;
   }

   // Calculate the result
   double mantisa = (double)(rawNumber >> 4) / 0x1000;
   int exponent = rawNumber & 0xF;
   if ( exponent > 0x7 )
      exponent -= 0x10;
   double result = mantisa * MMath::Pow10(exponent);
   return result;
}

void MUtilities::ToINSTRBuffer(double value, char* data, unsigned size)
{
   M_ASSERT(size == 2);
   // Calculate mantisa and exponent
   //
   int exponent = 0;
   double mantisa = 0.0;
   if ( value > 0.0 )
   {
      int e = static_cast<int>(floor(log10(value)) + 1.0);
      if ( e >= -4 ) // otherwise the number is rounded out (mantisa, exponent are 0.0)
      {
         exponent = e;
         mantisa = value * MMath::Pow10(-e);
      }
   }

   // Calculate raw number
   //
   unsigned rawNumber = 0u;
   if ( exponent >= -8 )
   {
      unsigned unsignedMantissa = (unsigned)(mantisa * double(0x1000) + 0.5);
      if ( unsignedMantissa > 0x0FFFu ) // case of the rounding up
      {
         M_ASSERT(unsignedMantissa == 0x1000u); // only one bit can be rounded up by the algorithm
         unsignedMantissa = 0x0FFFu; // well, live with a bad rounding in this case
      }
      rawNumber = (unsignedMantissa << 4) | ((unsigned)(exponent < 0 ? exponent + 0x10 : exponent));
   }

   M_ASSERT(exponent >= -4 || exponent <= 7);
   M_ASSERT(rawNumber <= 0xFFFFu);

   // Set buffer (insensitive to endianity)
   //
   for ( unsigned int i = 0; i < size ; ++i )
   {
      *data++ = (char)(rawNumber % 0x100);
      rawNumber /= 0x100;
   }
}

double MUtilities::FromINSTR(const MByteString& buffer)
{
   return FromINSTRBuffer(buffer.c_str(), static_cast<unsigned>(buffer.size()));
}

MByteString MUtilities::ToINSTR(double value)
{
   MByteString data(2, '0');
   ToINSTRBuffer(value, &(data[0]), static_cast<unsigned>(data.size()));
   return data;
}

#if !M_NO_FILESYSTEM // By convention, consider environment variables part of file system

   #ifndef M_USE_LOCAL_ENVIRONMENT_VARIABLES
      // by default, use local environment variables implementation only for CE
      #define M_USE_LOCAL_ENVIRONMENT_VARIABLES ((M_OS & M_OS_WIN32_CE) == M_OS_WIN32_CE)
   #endif

   #if M_USE_LOCAL_ENVIRONMENT_VARIABLES
      typedef std::map<MStdString, MStdString>
          MLocalEnvironment;
      static MCriticalSection s_environmentCriticalSection;
      static MLocalEnvironment s_environment;
   #endif

   static MStdString DoGetEnv(MConstChars path)
   {
      MStdString response;

      #if M_USE_LOCAL_ENVIRONMENT_VARIABLES
         {
            MCriticalSection::Locker lock(s_environmentCriticalSection);
            MLocalEnvironment::const_iterator it = s_environment.find(path);
            if ( it != s_environment.end() )
               response = (*it).second;
         }
      #elif M_OS & M_OS_WINDOWS
         #if M_UNICODE
            MWideString widePath = MToWideString(path);
            DWORD requiredSize = ::GetEnvironmentVariable(widePath.c_str(), NULL, 0);
            if ( requiredSize > 1 ) // when called with zeros, ::GetEnvironmentVariable returns size including zero terminating character
            {
               MWideString wideResponse;
               wideResponse.resize(requiredSize - 1);
               ::GetEnvironmentVariable(widePath.c_str(), &wideResponse[0], requiredSize);
               response = MToStdString(wideResponse);
            }
         #else
            DWORD requiredSize = ::GetEnvironmentVariable(path, NULL, 0);
            if ( requiredSize > 1 ) // when called with zeros, ::GetEnvironmentVariable returns size including zero terminating character
            {
               response.resize(requiredSize - 1);
               ::GetEnvironmentVariable(path, &response[0], requiredSize);
            }
         #endif
      #else

         const char* ptr = getenv(path);
         if ( ptr != NULL )
            response = ptr;

      #endif

      return response;
   }

   static MStdString DoGetAnyEnv(int count, ...)
   {
      MStdString response;
      M_ASSERT(count != 0);
      va_list va;
      va_start(va, count);
      for ( int i = 0; i < count; ++i )
      {
         response = DoGetEnv(va_arg(va, MConstChars));
         if ( !response.empty() )
            break;
      }
      va_end(va);
      return response;
   }

MStdString MUtilities::GetEnv(const MStdString& variable)
{
   return DoGetEnv(variable.c_str());
}

void MUtilities::SetEnv(const MStdString& variable, const MStdString& value)
{
   if ( !variable.empty() )
   {
      #if M_USE_LOCAL_ENVIRONMENT_VARIABLES
         MCriticalSection::Locker lock(s_environmentCriticalSection);
         s_environment[variable] = value;
      #elif M_OS & M_OS_WINDOWS
         #if M_UNICODE
            const BOOL status = ::SetEnvironmentVariable(MToWideString(variable).c_str(), MToWideString(value).c_str());
            MESystemError::CheckLastSystemError(status == 0);
         #else
            const BOOL status = ::SetEnvironmentVariable(variable.c_str(), value.c_str());
            MESystemError::CheckLastSystemError(status == 0);
         #endif
      #else
         const int status = setenv(variable.c_str(), value.c_str(), 1);
         MESystemError::CheckLastSystemError(status);
      #endif
   }
}

MStdString MUtilities::ExpandEnvVars(const MStdString& source)
{
   MStdString response;
   response.reserve(source.size() + 1);
   MStdString variable;
   variable.reserve(256);

   enum
   {
      EEV_S_CLOSED,
      EEV_S_PRE_OPENED,
      EEV_S_OPENED
   } state = EEV_S_CLOSED;

   MChar ch = 0;
#if M_OS & M_OS_WINDOWS
   for ( MStdString::size_type i = 0; i <= source.length(); ++i )
   {
      ch = source[i];
      switch ( state )
      {
      case EEV_S_PRE_OPENED:
         if ( ch == '%' )
         {
            response += '%';
         }
         else if ( ch )
         {
            if ( m_isalpha(ch) || ch == '_' )
            {
               state = EEV_S_OPENED;
               variable += ch;
            }
            else
            {
               state = EEV_S_CLOSED;
               response += '%';
               response += ch;
            }
         }
         else
         {
            state = EEV_S_CLOSED;
            response += '%';
         }
         break;
      case EEV_S_OPENED:
         if ( ch == '%' )
         {
            state = EEV_S_CLOSED;
            if ( variable.length() )
            {
               response += GetEnv(variable);
               variable.clear();
            }
            else
            {
               response += "$$";
            }
         }
         else if ( ch )
         {
            if ( m_isalnum(ch) || ch == '_' )
            {
               variable += ch;
            }
            else
            {
               state = EEV_S_CLOSED;
               response += '%';
               response += variable;
               response += ch;
               variable.clear();
            }
         }
         break;
      case EEV_S_CLOSED:
         if ( ch == '%' )
         {
            state = EEV_S_PRE_OPENED;
         }
         else if ( ch )
         {
            response += ch;
         }
         break;
      };
   }
#else // !M_OS & M_OS_WINDOWS
   MChar bk = 0;
   int nesting = 0;
   for ( MStdString::size_type i = 0; i <= source.length(); ++i )
   {
      ch = source[i];
      switch ( state )
      {
      case EEV_S_PRE_OPENED:
         switch ( ch )
         {
         case '$':
            response += ch;
            break;
         case '(':
         case '{':
            state = EEV_S_OPENED;
            bk = ch;
            ++nesting;
            break;
         case ')':
         case '}':
            state = EEV_S_CLOSED;
            response += '$';
            response += ch;
            bk = 0;
            nesting = 0;
            break;
         default:
            if ( m_isalpha(ch) || ch == '_' )
            {
               state = EEV_S_OPENED;
               variable += ch;
            }
            else
            {
               state = EEV_S_CLOSED;
               response += '$';
               response += ch;
               bk = 0;
               nesting = 0;
            }
         };
         break;
      case EEV_S_OPENED:
         switch ( ch )
         {
         case '$':
            if ( bk == 0 )
            {
               state = EEV_S_PRE_OPENED;
               response += GetEnv(ExpandEnvVars(variable));
               variable.clear();
            }
            else
            {
               variable += ch;
            }
            break;
         case '(':
         case '{':
            if ( variable.length() && *variable.rbegin() == '$' )
            {
               variable += ch;
               ++nesting;
            }
            else
            {
               state = EEV_S_CLOSED;
               response += '$';
               response += bk;
               response += variable;
               response += ch;
               nesting = 0;
               bk = 0;
               variable.clear();
            }
            break;
         case ')':
         case '}':
            --nesting;
            if ( nesting == 0 )
            {
               if ( (bk == '(' && ch == ')') || (bk == '{' && ch == '}') )
               {
                  state = EEV_S_CLOSED;
                  bk = 0;
                  response += GetEnv(ExpandEnvVars(variable));
                  variable.clear();
               }
            }
            else
            {
               variable += ch;
            }
            break;
         default:
            if ( variable.length() )
            {
               if ( *variable.rbegin() == '(' || *variable.rbegin() == '{' )
               {
                  if ( m_isalpha(ch) || ch == '_' )
                  {
                     variable += ch;
                  }
                  else
                  {
                     state = EEV_S_CLOSED;
                     response += '$';
                     response += bk;
                     response += variable;
                     response += ch;
                     nesting = 0;
                     bk = 0;
                     variable.clear();
                  }
               }
               else if ( m_isalnum(ch) || ch == '_' )
               {
                  variable += ch;
               }
               else
               {
                  if ( bk == 0 )
                  {
                     state = EEV_S_CLOSED;
                     response += GetEnv(ExpandEnvVars(variable));
                     if ( ch )
                     {
                        response += ch;
                     }
                     nesting = 0;
                     variable.clear();
                  }
                  else
                  {
                     state = EEV_S_CLOSED;
                     if ( ch )
                     {
                        variable += ch;
                     }
                     response += '$';
                     response += bk;
                     response += variable;
                     nesting = 0;
                     bk = 0;
                     variable.clear();
                  }
               }
            }
            else
            {
               if ( m_isalpha(ch) || ch == '_' )
               {
                  variable += ch;
               }
               else
               {
                  state = EEV_S_CLOSED;
                  response += '$';
                  response += bk;
                  if ( ch )
                  {
                     response += ch;
                  }
                  bk = 0;
                  nesting = 0;
               }
            }
         };
         break;
      case EEV_S_CLOSED:
         if ( ch == '$' )
         {
            state = EEV_S_PRE_OPENED;
         }
         else if ( ch )
         {
            response += ch;
         }
         break;
      }; // switch
   } // for
#endif // !(M_OS & M_OS_WINDOWS)

   return response;
}

MStdString MUtilities::GetTempDirectory()
{
   // For Windows, this is exactly how GetTempPath() works
   MStdString response = DoGetAnyEnv(3, "TMPDIR", "TMP", "TEMP");
   if ( response.empty() )
   {
      #if M_OS & M_OS_WINDOWS

         #if M_UNICODE
            wchar_t path [ M_MAX_PATH + 1 ];
         #else
            char path [ M_MAX_PATH + 1 ];
         #endif
         DWORD len = ::GetTempPath(M_MAX_PATH, path);
         if ( len > 0 && len < M_MAX_PATH )
            response = MToStdString(path, len);
         else
            response = GetHomeDirectory();

      #elif defined(__ANDROID__) // Check for __ANDROID__ platform, not Bionic library

         response.assign("/data/local/tmp", 15);

      #else

         response.assign("/tmp", 4);

      #endif
   }
   MAddDirectorySeparatorIfNecessary(response);
   return response;
}

MStdString MUtilities::GetHomeDirectory()
{
   MStdString response;
   #if M_OS & M_OS_WINDOWS
      #if M_OS & M_OS_WIN32_CE
         response.assign(1, '\\');
      #else
         response = DoGetAnyEnv(2, "USERPROFILE", "HOME"); // userprofile takes propriety on Home on Windows
      #endif
   #else
      response = DoGetEnv("HOME");
   #endif
   response = ExpandEnvVars(response);
   if ( response.empty() )
      response = GetModulePath();

   MAddDirectorySeparatorIfNecessary(response);
   return response;
}

MStdString MUtilities::DoMakeTempFileName(const MStdString& prefix, bool isDir)
{
   MStdString fileDir, fileName, response;
   DoSplitPath(prefix, &fileDir, &fileName);
   if ( fileDir.empty() )
   {
      fileDir = GetTempDirectory();
   }

   bool fail = true;

#if (M_OS & M_OS_WINDOWS) != 0
   if ( isDir )
   {
      MStdString tmp(MergePaths(fileDir, fileName));
      for ( ;; )
      {
         MStdString path;
         path.reserve(32);
         do
         {
            MChar buf[32];
            MFormat(buf, 32, "%d", MMath::Rand());
            path += buf;
         }
         while ( path.length() < 6 );

         path.resize(6);
         path = tmp + path;

         if (
            #if M_UNICODE
               ::CreateDirectory(MToWideString(path).c_str(), 0)
            #else
               ::CreateDirectory(path.c_str(), 0)
            #endif
            )
         {
            response = path;
            fail = false;
            break;
         }
         else if ( ::GetLastError() != ERROR_ALREADY_EXISTS )
         {
            break;
         }
      }
   }
   else
   {
      #if M_UNICODE
         wchar_t buf [ M_MAX_PATH ];
         if ( ::GetTempFileName(MToWideString(fileDir).c_str(), MToWideString(fileName).c_str(), 0, buf) )
         {
            response = MToStdString(buf);
            fail = false;
         }
      #else
         char buf [ M_MAX_PATH ];
         if ( ::GetTempFileName(fileDir.c_str(), fileName.c_str(), 0, buf) )
         {
            response.assign(buf);
            fail = false;
         }
      #endif
   }
#elif (M_OS & M_OS_NUTTX)
   MByteString tmp(MergePaths(fileDir, fileName));
   tmp += ".tmp";
   if ( isDir )
   {
      if ( !mkdir(tmp.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) )
      {
         response = tmp;
         fail = false;
      }
   }
   else
   {
     response = tmp;
     fail = false;
   }
#elif (M_OS & M_OS_POSIX)
   MByteString tmp(MergePaths(fileDir, fileName));
   tmp += "XXXXXX";

   if ( isDir )
   {
#if (M_OS & (M_OS_QNXNTO | M_OS_ANDROID)) != 0
      for ( std::size_t i = 0; i < 32; ++i )
      {
         mktemp(&tmp[0]);
         if ( mkdir(tmp.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) < 0 )
         {
            if ( errno != EEXIST )
               break;
         }
         else
         {
            response = tmp;
            fail = false;
            break;
         }
      }
#else
      if ( mkdtemp(&tmp[0]) != NULL )
      {
         response = tmp;
         fail = false;
      }
#endif
   }
   else
   {
      int fd = mkstemp(&tmp[0]);
      if ( fd != -1 )
      {
         response = tmp;
         close(fd);
         fail = false;
      }
   }
#else
   #error "isn't implemented"
#endif
   MESystemError::CheckLastSystemError(fail);

   return response;
}

MStdString MUtilities::MakeTempFileName(const MStdString& prefix)
{
   return DoMakeTempFileName(prefix, false);
}

MStdString MUtilities::MakeTempDirectoryName(const MStdString& prefix)
{
   return DoMakeTempFileName(prefix, true);
}
#endif // !M_NO_FILESYSTEM

int MUtilities::GetNumberOfProcessors()
{
   #if (M_OS & M_OS_WINDOWS) != 0
      SYSTEM_INFO sysinfo;
      GetSystemInfo(&sysinfo);
      return (int)sysinfo.dwNumberOfProcessors;
   #elif (M_OS & M_OS_LINUX) != 0
      return (int)sysconf(_SC_NPROCESSORS_ONLN);
   #else
      return 1;
   #endif
}

int MUtilities::GetNumberOfAddressBits()
{
   return sizeof(void *) * 8;
}

#if !M_NO_BASE64
namespace { // make sure there will be no link problem with popular names of functions
   #include "private/base64.cxx"
}

MStdString MUtilities::Base64Encode(const MByteString& data)
{
   MStdString result;
   if ( !data.empty() )
   {
      size_t codeLength = Base64encode_len((int)data.size());
      result.resize(codeLength, '\0');
      size_t ret = Base64encode(&result[0], data.c_str(), (int)data.size());
      M_ASSERT(codeLength == ret); // implementation includes zero byte at the end
      M_USED_VARIABLE(ret);
   }
   return result;
}

MByteString MUtilities::Base64Decode(const MStdString& text)
{
   MByteString result;
   if ( !text.empty() )
   {
      char* tmp = M_NEW char[text.size()];
      size_t size = Base64decode(tmp, text.c_str());
      result.assign(tmp, size);
      delete [] tmp;
   }
   return result;
}
#endif // !M_NO_BASE64
