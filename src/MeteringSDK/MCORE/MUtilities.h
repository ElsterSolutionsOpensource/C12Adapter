#ifndef MCORE_MUTILITIES_H
#define MCORE_MUTILITIES_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MUtilities.h

#include <MCORE/MTimer.h>
#include <MCORE/MVersion.h>

/// Provides useful services like timers, data type conversions, path and file manipulation, etc.
///
/// The MUtilities class inherits from MTimer for compatibility reasons.
///
/// \anchor PathSyntax
/// MeteringSDK extends regular file path syntax:
///
/// On every operating system it is possible to use both forward slash and back slash as file name separators.
/// It is recommended though to always use a regular slash /.
///
class M_CLASS MUtilities : public MTimer
{
public: // Services:

   /// Construct utilities object.
   ///
   /// There is little reason to create the utilities object,
   /// and all the other methods of utilities class are static.
   ///
   MUtilities() M_NO_THROW;

   /// Destroy utilities object.
   ///
   virtual ~MUtilities() M_NO_THROW;

public: // Properties:

#if !M_NO_FILESYSTEM
   ///@{
   /// Current application path.
   ///
   /// Most often, the current path is a global property of the process, as determined by OS.
   /// Some operating systems such as Windows CE do not have per process current path,
   /// and for such cases there is a special global variable to emulate this functionality.
   /// Property manipulation can result in a number of system errors.
   ///
   static MStdString GetCurrentPath();
   static void SetCurrentPath(const MStdString& path);
   ///@}
#endif // !M_NO_FILESYSTEM

   /// Returns the host name for the local machine, identification for the computer.
   ///
   /// Either the sockets library is used, if available, or if it is not, "localhost" is returned.
   ///
   static MStdString GetLocalHostName();


public: // Services:

   /// Converts the number given as unsigned integer into a BCD buffer.
   ///
   /// \pre buffer must be large enough for given number.
   ///
   static void ToBCDBuffer(unsigned intValue, char* buffer, unsigned size, bool littleEndian = false);

   /// Converts the number given as double into a BCD buffer.
   ///
   /// Binary coded decimal is a way of representing decimal numbers
   /// so that each byte has a pair of numbers, each in range 0 to 9.
   /// A BCD with hex representation x"00123456" represents a number 123456.
   /// No special codes are allowed in the BCD, only 0 .. 9.
   /// Note that the double can keep only 53 bits of mantissa.
   ///
   /// \pre Buffer must be large enough for the given number.
   ///
   static void ToBCDBuffer(double value, char* buffer, unsigned size, bool littleEndian = false);

   /// Convert the Binary Coded Decimal number given as data buffer and its size to double.
   ///
   /// Binary coded decimal is a way of representing decimal numbers
   /// so that each byte has a pair of numbers, each in range 0 to 9.
   /// A BCD with hex representation x"00123456" represents a number 123456.
   /// No special codes allowed in the BCD, only 0 .. 9.
   ///
   /// \param data
   ///    Pointer to the beginning of BCD number in memory.
   ///
   /// \param size
   ///    Size of the data in memory.
   ///
   /// \param littleEndian
   ///    Whether the BCD number is little endian, which is the default.
   ///    Little endian BCD number is by far the most usual case.
   ///
   /// \pre The given BCD shall be a proper BCD number consisting of
   /// an even number of digits. Otherwise an exception is thrown.
   /// The double overflow is not watched.
   ///
   static double FromBCDBuffer(const char* data, unsigned size, bool littleEndian = false);

   /// Convert BCD number given as byte string to double.
   ///
   /// Binary coded decimal is a way of representing decimal numbers
   /// so that each byte has a pair of numbers, each in range 0 to 9.
   /// A BCD with hex representation x"00123456" represents a number 123456.
   /// No special codes allowed in the BCD, only 0 .. 9.
   ///
   /// \param bytes
   ///    Bytes of the BCD number.
   /// \param littleEndian
   ///    Whether the BCD number is little endian, which is the default.
   ///    Little endian BCD number is by far the most usual case.
   /// \return The result number.
   ///
   /// \pre The given BCD shall be a proper BCD number consisting of
   /// an even number of digits. Otherwise an exception is thrown.
   /// The double overflow is not watched.
   ///
   static double FromBCD(const MByteString& bytes, bool littleEndian = false);

   /// Convert a positive double to a BCD coded byte string.
   ///
   /// Size, if specified, shall be the length of the result data in bytes.
   /// If the size is not specified, it will be calculated to be the minimum where the value fits.
   ///
   /// \param value
   ///    Value to convert into BCD string.
   /// \param size
   ///    Size of the result. If zero or default, the size will be the minimum which fits.
   /// \param littleEndian
   ///    Whether the produced BCD number will be little endian, which is the default.
   ///    Little endian BCD number is by far the most usual case.
   /// \return The result number.
   ///
   /// \pre The given double number shall be positive, and if size
   /// is specified, it shall fit in the number of bytes given.
   /// Otherwise an exception is thrown.
   ///
   static MByteString ToBCD(double value, unsigned size = 0u, bool littleEndian = false);

   /// Converts given byte string into an unsigned.
   ///
   /// \param bytes Byte string to convert, size is in range 1 to 4 bytes.
   /// \param littleEndian Whether the given bytes are of little endian format, big endian otherwise.
   /// \return Result unsigned value.
   ///
   /// \see FromUINT which is capable of converting up to eight bytes as it returns a variant.
   ///
   static unsigned UnsignedFromUINT(const MByteString& bytes, bool littleEndian);

#if !M_NO_VARIANT
   /// Converts given byte string into the number it represents according to rules for UINT fields.
   ///
   /// \param bytes Byte string to convert, size is in range 1 to 8 bytes.
   /// \param littleEndian Whether the given bytes are of little endian format, big endian otherwise.
   /// \return MVariant of type UINT (unsigned) or double, if the bytes given are bigger than 4-byte size.
   ///
   static MVariant FromUINT(const MByteString& bytes, bool littleEndian);

   /// Convert a positive number to byte string representation of this number as UINT.
   ///
   /// \param value Value to convert. Depending on its type:
   ///      - If the value is numeric it shall be positive,
   ///        and its size shall fit in the number of bytes given by size parameter.
   ///      - If value is a byte string its bytes will be reversed if littleEndian is false,
   ///        or it will be copied as it is if littleEndian is true.
   /// \param size Desired byte size of the result, value within range 1 to 8.
   ///        The number given should be enough to accommodate the value.
   /// \param littleEndian Whether the bytes returned should be in little endian format, big endian otherwise.
   /// \return MByteString result byte representation of the value.
   ///
   static MByteString ToUINT(const MVariant& value, unsigned size, bool littleEndian);

   /// Converts given byte string into the number it represents according to rules for INT fields.
   ///
   /// \param bytes Byte string to convert, size is in range 1 to 8 bytes.
   /// \param littleEndian Whether the given bytes are of little endian format, big endian otherwise.
   /// \return MVariant of type INT (int) or double, if the bytes given are bigger than 4-byte size.
   ///
   static MVariant FromINT(const MByteString& bytes, bool littleEndian);

   /// Convert an integer number to byte string representation of this number as INT.
   ///
   /// \param value Value to convert. Depending on its type:
   ///      - If the value is numeric its size shall fit in the number of bytes given by size parameter.
   ///      - If value is a byte string its bytes will be reversed if littleEndian is false,
   ///        or it will be copied as it is if littleEndian is true,
   /// \param size Desired byte size of the result, value within range 1 to 8.
   ///        The number given should be enough to accommodate the value.
   /// \param littleEndian Whether the bytes returned should be in little endian format, big endian otherwise.
   /// \return MByteString result byte representation of the value.
   ///
   static MByteString ToINT(const MVariant& value, unsigned size, bool littleEndian);
#endif // !M_NO_VARIANT

   /// Convert DSP specific floating point number given as data buffer to double.
   /// The size is in bytes, either 3 or 4.
   ///
   /// \pre The given DSP_FLOAT shall be a proper DSP float number as defined by
   /// the hardware specifications. Otherwise an exception is thrown.
   /// The double overflow is not watched.
   ///
   static double FromDSPFloatBuffer(const char* buffer, unsigned size);

   /// Convert DSP specific floating point number given as byte string to double.
   ///
   /// \pre The given DSP_FLOAT shall be a proper DSP float number as defined by
   /// the hardware specifications. Otherwise an exception is thrown.
   /// The double overflow is not watched.
   ///
   static double FromDSPFloat(const MByteString& buffer);

   /// Convert DSP specific "integer" number given as data buffer to double.
   /// The length is in bytes, the DSP integer is really a double in range -1.0 inclusive to
   /// 1.0 not inclusive.
   ///
   /// \pre The given DSP_INT shall be a proper DSP integer number as defined by
   /// the hardware specifications. Otherwise an exception is thrown.
   /// The double overflow is not watched.
   ///
   static double FromDSPIntBuffer(const char* buffer, unsigned size);

   /// Convert DSP specific "integer" number given as byte string to double.
   /// The length is in bytes, the DSP integer is really a double in range -1.0 .. 1.0.
   ///
   /// \pre The given DSP_INT shall be a proper DSP integer number as defined by
   /// the hardware specifications. Otherwise an exception is thrown.
   /// The double overflow is not watched.
   ///
   static double FromDSPInt(const MByteString& buffer);

   /// Converts INSTR value stored in the buffer to double.
   ///
   /// \pre Size must be equal to 2.
   ///
   static double FromINSTRBuffer(const char* data, unsigned size);

   /// Converts double value to given buffer.
   ///
   /// \pre Size must be equal to 2.
   ///
   static void ToINSTRBuffer(double value, char* data, unsigned size);

   /// Converts INSTR value given as byte string to double.
   ///
   /// \pre Length of the string must be equal to 2.
   ///
   static double FromINSTR(const MByteString& buffer);

   /// Converts double value to the byte string.
   ///
   static MByteString ToINSTR(double value);

   /// Convert a DSP float double to a byte string.
   /// Size shall be the length of the result data in bytes.
   /// Size can be either 3 or 4.
   ///
   /// \pre The given double number shall fit within the range denoted by the size of the buffer,
   /// otherwise an exception is thrown.
   ///
   static void ToDSPFloatBuffer(double value, char* buffer, unsigned size);

   /// Convert a DSP float double to a byte string.
   /// Size shall be the length of the result data in bytes.
   ///
   /// \pre The given double number shall be within range denoted by the size of the buffer,
   /// otherwise an exception is thrown. Size can be either 3 or 4. 
   ///
   static MByteString ToDSPFloat(double value, unsigned size);

   /// Convert a DSP_INT double to a byte buffer.
   /// Size shall be the length of the result data in bytes.
   ///
   /// \pre The given double number shall be within range denoted by the buffer size,
   /// otherwise an exception is thrown.
   ///
   static void ToDSPIntBuffer(double value, char* buffer, unsigned size);

   /// Convert a DSP float double to a byte string.
   /// Size shall be the length of the result data in bytes.
   ///
   /// \pre The given double number shall fit within the range denoted by the size of the buffer,
   /// otherwise an exception is thrown.
   ///
   static MByteString ToDSPInt(double value, unsigned size);

   /// Convert RAD40 buffer given as data and length to a standard string which it represents.
   /// It is not an error if the byte length is not an even number as the last odd byte will be truncated.
   ///
   /// \pre The given buffer shall have a valid RAD40 representation,
   /// or a bad character is reported.
   ///
   static MStdString FromRAD40Buffer(const char* data, int byteLen);

   /// Convert RAD40 buffer given as byte string to a standard string which it represents.
   /// It is not an error if the byte length is not an even number as the last odd byte will be truncated.
   ///
   /// \pre The given buffer shall have a valid RAD40 representation,
   /// or a bad character is reported.
   ///
   static MStdString FromRAD40(const MByteString& data);

   /// Convert the given string of characters to RAD40 buffer.
   /// It is not an error if the RAD length is not an even number as the last odd byte will be truncated.
   ///
   /// \pre The given string shall have a valid RAD40 representation,
   /// or a bad character is reported. The given buffer and size shall denote a
   /// valid writable byte chunk.
   ///
   static void ToRAD40Buffer(const MStdString& str, char* rad, unsigned radSize);

   /// Convert the given string of characters to RAD40 number returned as byte string.
   /// It is not an error if the RAD length is not an even number as the last odd byte will be truncated.
   ///
   /// \pre The given string shall have a valid RAD40 representation,
   /// or a bad character is reported. The given buffer and size shall denote a
   /// valid writable byte chunk.
   ///
   static MByteString ToRAD40(const MStdString& str, unsigned radSize);

#if !M_NO_FILESYSTEM

   /// Get the extension from the path specified, if the extension is present.
   ///
   /// \param path
   ///    Path, relative or absolute.
   ///    For more information, refer to \ref PathSyntax "MeteringSDK path syntax".
   ///
   /// \see GetPathFileName
   /// \see GetPathFileNameAndExtension
   /// \see GetPathDirectory
   ///
   static MStdString GetPathExtension(const MStdString& path);

   /// Get the file name from the path specified, if the file name is present.
   ///
   /// \param path
   ///    Path, relative or absolute.
   ///    For more information, refer to \ref PathSyntax "MeteringSDK path syntax".
   ///
   /// \see GetPathFileNameAndExtension
   /// \see GetPathDirectory
   /// \see GetPathExtension
   ///
   static MStdString GetPathFileName(const MStdString& path);

   /// Get the file name and extension from the path specified, if the file name and extension is present.
   ///
   /// \param path
   ///    Path, relative or absolute.
   ///    For more information, refer to \ref PathSyntax "MeteringSDK path syntax".
   ///
   /// \see GetPathFileName
   /// \see GetPathDirectory
   /// \see GetPathExtension
   ///
   static MStdString GetPathFileNameAndExtension(const MStdString& path);

   /// Get the directory path from the path specified, if the directory path is present.
   ///
   /// The disk letter is returned together with the path, done for OS independence.
   ///
   /// \param path
   ///    Path, relative or absolute.
   ///    For more information, refer to \ref PathSyntax "MeteringSDK path syntax".
   ///
   /// \see GetPathFileName
   /// \see GetPathFileNameAndExtension
   /// \see GetPathExtension
   ///
   static MStdString GetPathDirectory(const MStdString& path);

   /// Construct the path from the directory, file name and extension.
   ///
   static MStdString GetPath(const MStdString& dir, const MStdString& name, const MStdString& extension);

   /// Merge two paths together. If a second parameter is a full path, it is returned as is.
   ///
   /// If it is not a full path, directory parameter from the first argument is used.
   ///
   static MStdString MergePaths(const MStdString& fullDirOrName, const MStdString& dirOrName);

   /// Tells if a given path is a full path.
   ///
   /// \param path
   ///    Path, relative or absolute.
   ///    For more information, refer to \ref PathSyntax "MeteringSDK path syntax".
   ///
   static bool IsPathFull(const MStdString& path);

   /// Return true if a given path is present, and it is a directory.
   ///
   /// \param path
   ///    Path, relative or absolute.
   ///    For more information, refer to \ref PathSyntax "MeteringSDK path syntax".
   ///
   static bool IsPathDirectory(const MStdString& path) M_NO_THROW;

   /// Tell if a given path is an existing file of any sort.
   ///
   /// \param path
   ///    Path of a file or directory, relative or absolute.
   ///    For more information, refer to \ref PathSyntax "MeteringSDK path syntax".
   ///
   static bool IsPathExisting(const MStdString& path) M_NO_THROW;

   /// Get the full path of a file.
   ///
   /// \pre A file should exist, otherwise the result file name will be empty.
   ///
   static MStdString GetFullPath(const MStdString& fileName);

   /// Copy a file to a destination file.
   ///
   /// \param source
   ///     Source file, either relative or absolute path.
   ///     For more information, refer to \ref PathSyntax "MeteringSDK path syntax".
   ///     File should exist, or an exception is thrown.
   ///
   /// \param destination
   ///     Path where to copy the source.
   ///     File name should be valid, and it should be possible to create files in the destination,
   ///     or an exception is thrown. This way, MeteringSDK path syntax is not valid for this argument.
   ///
   static void CopyFile(const MStdString& source, const MStdString& destination); 

   /// Delete a file with the given name.
   ///
   /// \param path
   ///     Path to a file or directory.
   ///     File or directory with such path should exist, and it should be possible to delete it.
   ///
   static void DeleteFile(const MStdString& path);

   /// Move a file to a new location.
   ///
   /// \param source
   ///     Source file, either relative or absolute path.
   ///     File should exist, or an exception is thrown.
   ///
   /// \param destination
   ///     Path where to move the source.
   ///     File name should be valid, and it should be possible to create files in the destination,
   ///     or an exception is thrown. 
   ///
   static void MoveFile(const MStdString& source, const MStdString& destination);

   /// Create a directory with the given name.
   ///
   /// Multilevel creation is supported, so if a deep path of directories has to be created, it will be.
   ///
   /// \param path
   ///     Path for the new directory. If this is a relative path, current process directory is used.
   ///     It shall be possible to create directory with such path, or an exception is thrown.
   ///
   static void CreateDirectory(const MStdString& path);

   /// Ensures the necessary directory tree is created so the file with the given path can be created.
   ///
   /// Multilevel creation is supported, so if a deep path of directories has to be created, it will be.
   ///
   /// \param path
   ///     Full path to a file.
   ///     It shall be possible to create a directory for such file, or an exception is thrown.
   ///
   static void EnsureDirectoryExistsForFile(const MStdString& path);

   /// Find all files under the specified directory using the given mask.
   ///
   /// Only files are returned. To get directories use \ref FindDirectories.
   /// Files that start with a period, such as ".myconfig", are not returned.
   ///
   /// \param directory
   ///     Directory to search for files.
   ///
   /// \param fileMask
   ///     File mask to match.
   ///
   static MStdStringVector FindFiles(const MStdString& directory, const MStdString& fileMask);

   /// Find all subdirectories under the specified directory using the given mask.
   ///
   /// Only directories are returned. To get files use \ref FindFiles.
   /// Files that start with a period, such as ".." or ".svn", are not returned.
   ///
   /// \param directory
   ///     Directory where to search for subdirectories.
   ///
   /// \param directoryMask
   ///     Directory file name mask to match.
   ///
   static MStdStringVector FindDirectories(const MStdString& directory, const MStdString& directoryMask);

   /// Return the path to this module, one which is currently running.
   ///
   static MStdString GetModulePath() M_NO_THROW;

   /// Return the path to installation directory.
   ///
   /// This call does a certain amount of guessing, and might not return the correct data.
   /// In a simplest case it is equivalent to \refprop{GetModulePath,ModulePath}.
   /// For products that store their executables under "bin" subdirectory,
   /// the directory above "bin" is returned.
   /// Further, if products define registry entry "PATH", its value will be taken.
   ///
   static MStdString GetInstallationPath() M_NO_THROW;

#endif // !M_NO_FILESYSTEM

   /// Return a single hexadecimal character that represents the given unsigned number.
   ///
   /// In case the number given is 1, '1' will be returned,
   /// in case the number given is 10, 'A' will be returned, and so on.
   ///
   /// \pre If the number is bigger than 0xF, an exception is thrown.
   ///
   static MChar NumberToHexChar(unsigned n)
   {
      return (MChar)NumberToHexByte(n);
   }

   /// Return a single hexadecimal byte that represents the given unsigned number.
   ///
   /// In case the number given is 1, byte representation of '1' will be returned,
   /// in case the number given is 10, byte representation of 'A' will be returned, and so on.
   ///
   /// \pre If the number is bigger than 0xF, an error is thrown.
   ///
   static Muint8 NumberToHexByte(unsigned n);

   /// Return a number for a single hexadecimal character given.
   ///
   /// In case the character given is '1', 1 will be returned,
   /// in case the character given is 'A', 10 will be returned, and so on.
   ///
   /// The service works only with ASCII subset.
   ///
   /// \pre If the number is not a hex char, an error is thrown.
   ///
   static unsigned HexCharToNumber(MChar c);

   /// Return a number for a single hexadecimal byte given.
   ///
   /// In case the byte given has character value '1', 1 will be returned,
   /// in case the byte given has character value 'A', 10 will be returned, and so on.
   ///
   /// The service works only with ASCII subset.
   ///
   /// \pre If the number is not a hex byte, an error is thrown.
   ///
   static unsigned HexByteToNumber(Muint8 c)
   {
      return HexCharToNumber((MChar)c);
   }

   ///@{
   /// Convert the given byte string into hexadecimal string representation.
   ///
   /// Parameter 'format' can be either a string, or anything convertible to a boolean.
   /// With boolean format, TRUE means that the bytes will be separated with blanks.
   ///
   /// String format specifies a conversion template, which is interpreted as follows:
   /// <ul>
   /// <li> Each 'X' or 'x' character is replaced with a single hexadecimal digit.  </li>
   /// <li> Any white space character is copied to the output string. </li>
   /// <li> Any other characters are illegal. </li>
   /// </ul>
   ///
   /// Format string can be of any length. If it is shorter than the available input, 
   /// it is cyclically repeated until the input is exhausted. If it is longer than 
   /// the available input, the rest of format string is silently ignored.
   ///
   /// Example of format string: "XXXX XXXX  "; produces output like: "1234 5678  9ABC DEF0"
   /// Another example: "xx "; produces: "12 34 56 78 9a bc de f0"
   /// The following formats are illegal: "X X", "Xx"
   ///
   /// \pre Format string must conform to the above mentioned rules.
   ///
#if !M_NO_VARIANT
   static MByteString BytesToHex(const MByteString& bytes, const MVariant& format = true);
   static MStdString BytesToHexString(const MByteString& bytes, const MVariant& format = true);
#else
   static MByteString BytesToHex(const MByteString& bytes, bool useBlanks = true)
   {
      return BufferToHex(bytes.data(), bytes.size(), useBlanks);
   }
#endif
   ///@}

   ///@{
   /// Convert a buffer given as pointer and length into hexadecimal string.
   ///
   /// The function behaves in the same way as BytesToHex except it takes character pointer and length as parameters.
   ///
   /// \param bytes Pointer to buffer.
   /// \param length Length of the buffer.
   /// \param useBlanks Whether to separate bytes with blanks like '00 00 00' or have them like '000000'.
   /// \return Result string.
   ///
   static MByteString BufferToHex(const char* bytes, unsigned length, bool useBlanks = true);
   static MByteString BufferToHexString(const char* bytes, unsigned length, bool useBlanks = true)
   {
      return BufferToHex(bytes, length, useBlanks); // this is a legacy method
   }
   ///@}

   ///@{
   /// Convert a buffer given as pointer and length into hexadecimal string.
   ///
   /// The function behaves in the same way as BytesToHex except it takes character pointer and length as parameters.
   ///
   /// \param bytes Pointer to buffer.
   /// \param length Length of the buffer.
   /// \param format Format, as described in \ref BytesToHex.
   /// \return Result string.
   ///
   static MByteString BufferToHex(const char* bytes, unsigned length, const MByteString& format);
   static MStdString BufferToHexString(const char* bytes, unsigned length, const MByteString& format)
   {
      return BufferToHex(bytes, length, format); // this is a legacy method
   }
   ///@}

   /// Convert hexadecimal byte string into binary byte string.
   /// Any alphanumerical character in the input string must represent a hexadecimal digit.
   /// Any non-alphanumerical characters are skipped by the conversion.
   ///
   /// \pre If the hexadecimal string has some bad characters,
   /// the exception thrown contains information about what was bad.
   ///
   /// \param hexString String that has hex characters and possible blank separators.
   /// \return Binary representation of the given string.
   ///
   static MByteString HexToBytes(const MByteString& hexString);

   /// Convert hexadecimal string into binary byte string.
   /// Any alphanumerical character in the input string must represent a hexadecimal digit.
   /// Any non-alphanumerical characters are skipped by the conversion.
   ///
   /// \pre If the hexadecimal string has some bad characters,
   /// the exception thrown contains information about what was bad.
   ///
   /// \param hexString Byte string that has hex characters and possible blank separators.
   /// \return Binary representation of the given string.
   ///
   static MByteString HexStringToBytes(const MStdString& hexString);

   /// Convert hexadecimal string into byte string.
   ///
   /// \pre If the hexadecimal string has some bad characters,
   /// the exception thrown contains information about what was bad.
   ///
   static MByteString HexBufferToBytes(const char* buff, unsigned length);

   /// Convert the given byte string into numeric string representation.
   ///
   /// \anchor BytesToNumeric_format
   /// String format specifies a conversion template, which is interpreted as follows:
   /// <ul>
   /// <li> Empty format string is equivalent to format "b.", bytes separated with '.', as in the IP address.</li>
   /// <li> Each 'B' or 'b' character is replaced with a decimal value of the byte.
   ///      These cannot appear in sequence such as "bb", "BB" or "bB"
   ///      as there has to be a separator in between.</li>
   /// <li> Non ASCII codes (above 0x7F), and letters or digits are not permitted in the format.</li>
   /// <li> Any other characters, such as punctuation or space, are copied to the output.</li>
   /// </ul>
   /// At present only decimal representation of single bytes is supported,
   /// while in the future the other formats for words and double words can be added.
   /// Hexadecimal, binary or octal numbers can also be considered.
   ///
   /// Format string can be of any length. If it is shorter than the available input,
   /// it is cyclically repeated until the input is exhausted. If it is longer than
   /// the available input, the rest of format string is silently ignored.
   ///
   /// Example of format string: "[b:b]"; produces output like: "[123:255][0:0]"
   /// Another example: "b."; produces: "12.34.56.78.91.234"
   /// An example format for an OBIS code with class number is "b.b.b.b.b.b b-b"
   ///
   /// \param bytes Raw bytes to convert.
   /// \param format Conversion format, see \ref BytesToNumeric_format "the description".
   /// \return Result string such as "1.0.64.0.0.255".
   ///
   static MStdString BytesToNumericString(const MByteString& bytes, const MStdString& format = MVariant::s_emptyString);

   /// Convert the given bytes array and length into numeric string representation.
   ///
   /// \param buff Pointer to the buffer.
   /// \param length Length of the buffer.
   /// \param format Conversion format, see \ref BytesToNumeric_format "the description".
   /// \return Result byte string.
   ///
   /// \see BytesToNumericString Equivalent of this call that takes string objects.
   ///
   static MStdString BufferToNumericString(const char* buff, unsigned length, const char* format = NULL);

   /// Convert numeric string into binary byte string.
   ///
   /// Any sequence of digits in the input string must represent a decimal value of a byte.
   /// No letters or non-ASCII codes are permitted, and any other character such as space or punctuation is ignored.
   /// As an example, the input 1.0.64.0.0.255 1-5" will be converted into x"01 00 40 00 00 FF 01 05".
   ///
   /// \param numericString String that has bytes represented in decimal form.
   /// \return Binary representation of the given numeric string.
   ///
   static MByteString NumericStringToBytes(const MStdString& numericString);

   /// Convert numeric byte buffer into byte string.
   ///
   /// Any sequence of digits in the input string must represent a decimal value of a byte.
   /// No letters or non-ASCII codes are permitted, and any other character such as space or punctuation is ignored.
   /// As an example, the input 1.0.64.0.0.255 1-5" will be converted into "01 00 40 00 00 FF 01 05".
   ///
   /// \param buff Pointer to a buffer that has bytes represented in decimal form.
   /// \param length Length of the buffer.
   /// \return Binary representation of the given numeric buffer.
   ///
   static MByteString NumericBufferToBytes(const char* buff, unsigned length);

#if !M_NO_VARIANT
   /// Convert the variant given to a constant with a proper MDL syntax.
   /// For example, numeric values are printed as they are,
   /// while characters are escaped and enclosed with apostrophes,
   /// and strings are escaped and enclosed with double quotes.
   ///
   /// \pre The variant should not be empty, otherwise an
   /// exception will be raised.
   ///
   /// \see \ref ToRelaxedMDLConstant for conversion that meets requirements of the previous implementation. 
   ///
   static MStdString ToMDLConstant(const MVariant&);

   /// Convert the variant given to a constant with a relaxed MDL syntax.
   /// The difference from ToMDLConstant is that booleans are 0 or 1,
   /// and unsigned numbers do not have u added.
   ///
   /// \pre The variant should not be empty, otherwise an
   /// exception will be raised.
   ///
   static MStdString ToRelaxedMDLConstant(const MVariant&);

   /// Convert the given string into a variant with a proper MDL syntax.
   ///
   /// \pre The string shall represent an MDL constant, otherwise an
   /// exception will be raised.
   ///
   static MVariant FromMDLConstant(const MStdString&);
#endif // !M_NO_VARIANT

   /// Get the version of the MeteringSDK library.
   ///
   /// MeteringSDK version is the same for all clients, that are built on top of it.
   /// By convention, the variable is not constant, however changing the
   /// version has no side effect on the software behavior.
   ///
   static MVersion& GetVersion();

   /// Get the version of the product that is the client of this library.
   ///
   /// This is based on C++ macro string M_PRODUCT_VERSION, but returns a MVersion object.
   ///
   /// The returned version object will have IsReadOnly property on,
   /// therefore, the changes to this object are protected.
   /// The caller can remove such protection by making the version
   /// read/write and modify the version, should it be considered a good idea.
   ///
   /// \return Read-only product version object.
   ///
   static MVersion& GetProductVersion();

   /// Return product name, as available at compile time.
   ///
   /// This is a reflected equivalent of C++ macro M_PRODUCT_NAME.
   /// Product name is defined at compile time, a string.
   ///
   /// \return String that represents the product name.
   ///
   static MStdString GetProductName();

   /// Return the operating system name.
   ///
   /// The name of the operating system is case sensitive, as given in the below list.
   ///
   /// \possible_values
   ///    - "Android"
   ///    - "NuttX" - Embedded/real-time POSIX compatible operating system
   ///    - "Linux"
   ///    - "BSD" - Berkley Software Distribution *NIX
   ///    - "CMX" - Embedded operating system
   ///    - "QNX" - Embedded/real-time POSIX compatible operating system
   ///    - "Windows CE" - Windows CE, Pocket PC, and so on
   ///    - "Windows" - Any windows, including Windows Server, but not Windows CE
   ///
   /// \return String that represents the operating system.
   ///
   static MStdString GetOperatingSystemName();

   /// Return the operating system version.
   ///
   /// The operating system version is determined at runtime.
   /// The version can have a different number of fractions,
   /// typically two or three, depending on the operating system:
   ///   - "Android" returns the Android version such as 4.0.2.
   ///   - "Linux" returns the version of the kernel, such as "3.14.1".
   ///   - "Windows" and "Windows CE" return the output of GetVersionEx call, two digits.
   ///
   /// In case of Windows there is a mapping between the version and the OS release,
   /// as defined here:
   ///
   ///   https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724833%28v=vs.85%29.aspx
   ///
   /// The version value of Windows 8.1 and later will depend on whether the manifest is present.
   /// If there is no manifest, these operating systems return the version of "Windows 8".
   /// The short version of the page is:
   ///   - Windows 10 and Windows Server 2016: "10.0" ("6.2" if the manifest is absent)
   ///   - Windows 8.1 and Windows Server 2012 R2: "6.3" ("6.2" if the manifest is absent)
   ///   - Windows 8 and Windows Server 2012: "6.2"
   ///   - Windows 7 and Windows Server 2008 R2: "6.1"
   ///   - Windows Vista and Windows Server 2008: "6.0"
   ///   - Windows XP Professional x64 and Windows Server 2003: "5.2"
   ///   - Windows XP: "5.1"
   ///
   /// The returned version object will have IsReadOnly property on,
   /// therefore, the changes to this object are protected.
   /// The caller can remove such protection by making the version
   /// read/write and modify the version, should it be considered a good idea.
   ///
   /// \return Read-only product version object.
   ///
   static MVersion& GetOperatingSystemVersion();

#if !M_NO_FILESYSTEM

   /// This function retrieves the path of the directory designated for
   /// temporary files.
   ///
   static MStdString GetTempDirectory();

   /// This function retrieves the user's home directory.
   ///
   static MStdString GetHomeDirectory();

   /// This function searches the environment list to find the
   /// environment variable, and returns a corresponding value string.
   ///
   static MStdString GetEnv(const MStdString& variable);

   /// This function changes or adds an environment variable.
   ///
   static void SetEnv(const MStdString& variable, const MStdString& value);

   /// Replace environment variables with their values. The format is
   /// $VARNAME or ${VARNAME} or $(VARNAME) on Posix and %VARNAME% on
   /// Windows, where VARNAME contains alphanumeric characters and '_'
   /// only.
   static MStdString ExpandEnvVars(const MStdString& source);

   /// This function generates a unique temporary filename from
   /// template 'prefix', creates the file, and returns a file
   /// pathname.
   ///
   static MStdString MakeTempFileName(const MStdString& prefix);

   /// This function generates a uniquely named temporary directory
   /// from templates 'prefix', creates the directory and return a
   /// directory pathname.
   ///
   static MStdString MakeTempDirectoryName(const MStdString& prefix);

#endif // !M_NO_FILESYSTEM

   /// Get the number of processors or simultaneously handled threads.
   ///
   /// This returns the number of simultaneously executing units in the machine.
   /// Therefore, this is not necessarily the number of chips or the number of kernels.
   /// The chip can have many kernels, and each kernel can support 
   /// many simultaneously executing threads in hyperthreading architecture.
   /// For example, an Intel i7 processor,  which is a single chip with four kernels
   /// can return eight as its "number of processors."
   ///
   /// \return int, a positive number, maximum number of simultaneously executing threads in the system.
   ///
   static int GetNumberOfProcessors();
   
   /// Get the number of address bits for target platform.
   ///
   /// \return int, a positive number (currently 32 or 64), amount of bits in address field.
   static int GetNumberOfAddressBits();

#if !M_NO_BASE64

   /// Encode binary data given as Base 64.
   ///
   /// This method for encoding binary data into text that can have only a very limited 
   /// set of ASCII characters that can be safely transferred through any media.
   /// The correspondent standard for this encoding is RFC-4648, http://tools.ietf.org/html/rfc4648.
   ///
   /// \param data Binary data to encode.
   /// \return Result text data encoded with Base 64.
   ///
   /// \see Base64Decode
   ///
   static MStdString Base64Encode(const MByteString& data);

   /// Decode Base 64 string into binary data.
   ///
   /// This method for decoding Base 64 text into binary.
   /// The correspondent standard for this encoding is RFC-4648, http://tools.ietf.org/html/rfc4648.
   ///
   /// \param text Text encoded with Base 64.
   /// \return Result binary data.
   ///
   /// \see Base64Encode
   ///
   static MByteString Base64Decode(const MStdString& text);

#endif // !M_NO_BASE64

private: // Prevent certain operations on utilities:
   
   static MStdString DoMakeTempFileName(const MStdString& prefix, bool isDir);

   // prevent certain operators inherited from timer
   MUtilities(const MTimer&);
   MUtilities(const MUtilities&);
   void operator=(const MTimer&);
   void operator=(const MUtilities&);
   bool operator==(int) const;
   bool operator!=(int) const;
   bool operator>=(int) const;
   bool operator<=(int) const;
   bool operator>(int) const;
   bool operator<(int) const;

private: // Data:

   M_DECLARE_CLASS(Utilities)
};

///@}
#endif 
