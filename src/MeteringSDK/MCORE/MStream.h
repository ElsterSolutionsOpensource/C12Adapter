#ifndef MCORE_MSTREAM_H
#define MCORE_MSTREAM_H

#include <MCORE/MException.h>

class MStreamProcessor;

/// MStream is the base class of all streams.
///
/// A stream is an abstraction of a sequence of bytes, such as a file,
/// an input/output device, an interprocess communication pipe, or a
/// TCP/IP socket. The MStream class and its derived classes provide
/// a generic view of these different types of input and output,
/// isolating the programmer from the specific details of the operating
/// system and the underlying devices.
///
/// Some stream implementations perform local buffering of the underlying
/// data to improve performance. For such streams, the Flush method can
/// be used to clear any internal buffers and ensure that all data has
/// been written to the underlying data source or repository.
///
/// Calling Close on a MStream flushes any buffered data, essentially
/// calling Flush for you. Close also releases operating system resources
/// such as file handles, network connections, or memory used for any
/// internal buffering.
///
class M_CLASS MStream : public MObject
{
   friend class MStreamProcessor;
   friend class MStreamProcessorBuffered;
   #if (M_OS & M_OS_WINDOWS) != 0
      friend class MStreamProcessorText;
   #endif

public: // Types:

   /// Flags that set modes of stream manipulation.
   /// They are given at stream open call.
   ///
   enum OpenFlags
   {
      /// Only read operations are allowed by this stream.
      ///
      FlagReadOnly = 0x0001,

      /// Only write operations are allowed by this stream.
      ///
      FlagWriteOnly = 0x0002,

      /// Both reads and writes are allowed for this stream.
      ///
      FlagReadWrite = 0x0003,

      /// On non-POSIX platforms, this flag will translate from new line character to carriage return and new line.
      ///
      /// Text streams can be buffered or not.
      ///
      FlagText = 0x00010,

      /// Real device read-write operations will be minimized, buffer used.
      ///
      FlagBuffered = 0x0020,

   };

   class Impl;

protected: // Type:
/// \cond SHOW_INTERNAL

   enum StreamOpType
   {
      STREAMOP_NONE,
      STREAMOP_READ,
      STREAMOP_WRITE
   };

/// \endcond SHOW_INTERNAL
protected: // Constructor:

   /// Default constructor, called from children of stream.
   ///
   MStream();

public: // Destructor:

   /// Closes the stream and destroys the object.
   ///
   /// This destructor never throws an exception at a close,
   /// even if during this operation any problems were encountered,
   /// such as IO error at flushing the buffer.
   ///
   /// \see Close This method will not silently swallow any exceptions during close operation.
   ///
   virtual ~MStream() M_NO_THROW;

public: // Methods:

   /// Whether the stream object is successfully open.
   ///
   /// The implementation of this method depends on the particular stream class.
   ///
   /// \see Close - closing the stream
   ///
   bool IsOpen() const;

   /// Return a representative name of a stream.
   ///
   /// This method is convenient for reporting the user recognized names in
   /// generic stream operations, for example it can be used for error reporting.
   /// The implementation of this method depends on the particular stream class.
   /// For files the file name will be returned.
   ///
   /// \pre The stream shall be open for this operation to be successful.
   ///
   virtual MStdString GetName() const = 0;

   /// Return flags associated with stream.
   ///
   /// By convention, if the stream is not open, the returned value will be zero.
   ///
   /// \see IsOpen to check whether the stream is open.
   ///
   unsigned GetFlags() const
   {
      return m_flags;
   }

   ///@{
   /// Key to use in AES encrypted streams.
   ///
   /// The key shall be a hexadecimal string that represents 16 binary bytes.
   /// Key can be set to the stream even if it is not open yet.
   /// In this case, it will affect the open operation.
   ///
   /// \see FlagAesEncrypted - flag that controls whether the stream is encrypted
   /// \see FlagAesEncryptedCompressed - flag that controls whether the stream shall be compressed before encryption
   ///
   MStdString GetKey() const;
   void SetKey(const MStdString&);
   ///@}

   ///@{
   /// Position of the current stream pointer.
   ///
   /// If the given position exceeds stream size the stream pointer will be moved to the end of stream.
   ///
   /// \pre The stream must be opened for these operations to succeed.
   ///      Also, the stream type and flags shall allow manipulation of its position.
   ///
   unsigned GetPosition() const;
   void SetPosition(unsigned);
   ///@}

   ///@{
   /// Returns total size of the stream in bytes.
   ///
   /// For compressed streams this size can be a lot bigger than the actual file size,
   /// as this size is whatever can be read from the stream.
   /// It is possible to truncate a stream that support such operation
   /// to a size that is smaller than its current size.
   ///
   /// \pre The stream must be opened for these operations to succeed.
   ///      The stream type and flags shall allow manipulation of its size.
   ///      When set, the size given shall not be bigger than the current size of the stream.
   ///
   unsigned GetSize() const;
   void SetSize(unsigned);
   ///@}

   ///@{
   /// Reads a byte from the stream and advances the position within the stream by one byte.
   ///
   /// If a stream is at the end, return integer -1, otherwise, a byte with unsigned value 0 .. 255 is returned.
   /// Therefore, to check whether the stream is at the end, one verifies the variant type is an integer,
   /// in which case its value will be -1.
   ///
   /// \return
   ///    - Byte in range 0 .. 225 for a read byte
   ///    - Integer with value -1 if the stream is at the end
   ///
   /// \pre The stream must be opened, and its access is possible.
   ///
#if !M_NO_VARIANT
   MVariant ReadByte();
#else
   int ReadByte();
#endif
   ///@}

   /// Reads a byte string of a given length from the stream.
   ///
   /// In case the stream does not have the given count of bytes, an end of stream exception is thrown.
   ///
   /// \param count
   ///     The number of bytes to read from the stream.
   ///
   /// \return
   ///     Binary contents of exact specified size.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see ReadAvailable - a method that can return lesser amount of bytes than given.
   /// \see ReadAll - return all bytes available, such as read the whole file into a byte buffer.
   ///
   MByteString Read(unsigned count);

   /// Reads a byte string up to a given length from the stream.
   ///
   /// In case the stream does not have the given count of bytes, whatever is there is returned,
   /// therefore, the result number of bytes can be smaller than what is requested.
   /// When this operation is attempted at the end of the stream it will return an empty byte string.
   ///
   /// \param count
   ///     The number of bytes to attempt to read from the stream.
   ///
   /// \return
   ///     Binary contents of specified size, or smaller.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see Read for reading a fixed count of bytes from stream
   /// \see ReadAll - return all bytes available, such as read the whole file into a byte buffer.
   ///
   MByteString ReadAvailable(unsigned count);

   /// Reads all bytes available in stream.
   ///
   /// For a file it returns all bytes up to the end of the stream.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see Read - Reads a fixed count of bytes from stream.
   /// \see ReadAvailable - A method that can return lesser amount of bytes than given.
   /// \see ReadAllLines - Like ReadAll, but returns array of lines rather than one whole byte array.
   ///
   MByteString ReadAll();

   /// Reads a byte buffer of a given length from the stream.
   ///
   /// This is a C++ only method.
   /// In case the stream does not have the given count of bytes, an end of stream exception is thrown.
   ///
   /// \param buffer
   ///     The buffer to hold the result, shall have at least count bytes.
   ///
   /// \param count
   ///     The number of bytes to read from the stream.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see Read - Returns a byte string rather than fills buffer.
   /// \see ReadAvailableBytes - A method that can populate lesser amount of bytes than given.
   ///
   void ReadBytes(char* buffer, unsigned count);

   /// Reads a byte buffer up to a given length from the stream.
   ///
   /// This is a C++ only method.
   /// In case the stream does not have the given count of bytes, whatever is there is populated,
   /// therefore, the result number of bytes can be smaller than what is requested.
   /// When this operation is attempted at the end of the stream, it will populate an empty byte string.
   ///
   /// \param buffer
   ///     The buffer to hold the result, shall have at least count bytes.
   ///
   /// \param count
   ///     The number of bytes to attempt to read from the stream.
   ///
   /// \return
   ///     Number of bytes read from the stream and set into buffer.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see ReadAvailable - Returns a byte string rather than fills buffer.
   /// \see ReadBytes - A method that reads an exact count of bytes or throws an exception.
   ///
   unsigned ReadAvailableBytes(char* buffer, unsigned count);

#if !M_NO_VARIANT
   /// Read one line from text file.
   ///
   /// A line is expected to be separated with new line character, while carriage return is ignored.
   /// The returned line ends with line feed unless it is last in the file, and the last line does not end with line feed character.
   /// When the file has no more lines, an empty variant type is returned.
   /// The line is read from the current position.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see ReadAllLines - Returns all lines in a file in a collection.
   ///
   MVariant ReadLine();
#endif

   /// Read one line from text file, return false at end of file.
   ///
   /// A line is expected to be separated with new line character, while carriage return is ignored.
   /// The returned line ends with line feed unless it is last in the file, and the last line does not end with line feed character.
   /// When the file has no more lines, an empty variant type is returned.
   /// The line is read from the current position.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see ReadAllLines - Returns all lines in a file in a collection.
   ///
   bool ReadOneLine(MStdString& line);

   /// Read all lines from text file.
   ///
   /// A line is expected to be separated with new line character, while carriage return is ignored.
   /// The lines in collection end with line feed unless it is last in the file, and the last line does not end with line feed.
   /// When the file has no more lines an empty variant type is returned.
   /// The line is read from the current position.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see ReadLine - Returns next line in a file.
   ///
   MStdStringVector ReadAllLines();

   /// Skips over and discards specified number of bytes of data from this stream.
   ///
   /// The operation is performed by reading and ignoring whatever is read.
   /// If the specified number is zero, no bytes are skipped.
   /// If the stream does not have the specified number of bytes, an end of line exception is thrown.
   ///
   /// \param count
   ///     How many bytes to skip.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void Skip(unsigned count);

   /// Writes a byte to the current position in the stream and advances the position by one byte.
   ///
   /// \param byte
   ///     A byte to place into stream.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void WriteByte(Muint8 byte);

   /// Writes a char to the current position in the stream and advances the position by one byte.
   ///
   /// This method is exactly the same as WriteByte but for C++ type char, no UNICODE characters are processed.
   ///
   /// \param c
   ///     A character to place into stream.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void WriteChar(char c)
   {
      WriteByte((Muint8)c);
   }

   /// Writes a given byte string to the current position in the stream and advances the position.
   ///
   /// \param bytes
   ///     Bytes to write into the stream.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void Write(const MByteString& bytes);

   /// Writes a given byte buffer to the current position in the stream and advances the position.
   ///
   /// This is a C++ only method.
   ///
   /// \param bytes
   ///     Pointer to the beginning of bytes buffer to write into the stream.
   ///
   /// \param count
   ///     Number of bytes in the buffer that shall be written.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void WriteBytes(const char* bytes, unsigned count);

   /// Writes a given zero terminated character string to the current position in the stream and advances the position.
   ///
   /// This is a C++ only method. The terminating zero character is not written to the stream.
   ///
   /// \param chars
   ///     Pointer to the beginning of zero terminated string.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void WriteChars(const char* chars);

   /// Write a line into file.
   ///
   /// If the given line does not end with line feed, the line feed is added to the stream.
   /// If the stream is open in text mode by having FlagText at open, and the platform is Windows,
   /// the carriage return is also added to comply with the text file rules on that platform.
   ///
   /// \param str
   ///    A line to write into stream.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void WriteLine(const MStdString& str);

   /// Write given lines into file.
   ///
   /// If any of the given lines do not end with line feed, the line feed is added to the stream.
   /// If the stream is open in text mode by having FlagText at open, and the platform is Windows,
   /// the carriage return is also added to comply with the text file rules on that platform.
   ///
   /// \param lines
   ///    Lines to write into stream.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void WriteAllLines(const MStdStringVector& lines);

   /// Write a formatted string into a file
   ///
   /// This is a C++ only method.
   ///
   /// \param format
   ///    Format that shall comply to formatting rules defined by global function \ref MFormat.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see MFormat for supported format specifiers.
   ///
   void WriteFormat(MConstChars format, ...);

   /// Write a formatted string into a file using va_list.
   ///
   /// This is a C++ only method.
   ///
   /// \param format
   ///    Format that shall comply to formatting rules defined by global function \ref MFormat.
   ///
   /// \param args
   ///    Format arguments.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   /// \see MFormat for supported format specifiers.
   ///
   void WriteFormatVA(MConstChars format, va_list args);

   /// Causes any buffered data to be written into stream.
   ///
   /// The behavior of this method depends on the particular stream.
   /// In some cases, the method has to do nothing until the stream is closed.
   ///
   /// \pre The stream must be opened for the operation to succeed, and the proper access rights are present.
   ///
   void Flush();

   /// Flushes and closes the current stream and releases any resources.
   ///
   /// If a stream is not open, this method does nothing.
   /// Different from object destructor, this method can report any errors such as failure to flush a stream buffer.
   /// After such error, the file will remain closed
   ///
   /// \see IsOpen - tells if the stream is open
   ///
   void Close();

public: // Raw data readers and writers:
/// \cond SHOW_INTERNAL

#if !M_NO_VARIANT
   int ReadRawInt();
   char ReadRawChar();
   bool ReadRawBool();
   Muint8 ReadRawByte();
   double ReadRawDouble();
   MByteString ReadRawByteString();
   MStdString ReadRawString();

   MVariant ReadRawVariant();

   void WriteRawByte(Muint8 value)
   {
      WriteByte(value);
   }

   void WriteRawInt(int value);
   void WriteRawChar(char value);
   void WriteRawBool(bool value);
   void WriteRawDouble(double value);
   void WriteRawByteString(const MByteString& value);
   void WriteRawString(const MStdString& value);

#if !M_NO_REFLECTION
   void WriteRawObject(const MObject* value);
#endif

   void WriteRawVariant(const MVariant& value);

#endif // !M_NO_VARIANT

protected: // Helper methods:
/// \cond SHOW_INTERNAL

   virtual unsigned DoGetSize() const;
   virtual void DoSetSize(unsigned length);
   virtual unsigned DoGetPosition() const;
   virtual void DoSetPosition(unsigned length);

   void DoStartOpen(unsigned flags);
   void DoFinishOpen();
   void DoInsertProcessor(MStreamProcessor* processor);
   void DoDeleteProcessors();
   void DoPrepareForOp(StreamOpType op);
   void DoCloseWithNoFlush();

   virtual void DoSetKeyImpl(const MByteString&);
   virtual unsigned DoReadAllAvailableBytesImpl(char* buffer, unsigned count);
   virtual unsigned DoReadAvailableBytesImpl(char* buffer, unsigned count) = 0;
   virtual void DoWriteBytesImpl(const char* buffer, unsigned count) = 0;
   virtual void DoFlushImpl(bool softFlush);
   virtual bool DoIsOpenImpl() const = 0;
   virtual void DoCloseImpl();

   M_NORETURN_FUNC void DoThrowStreamSoftwareError(MErrorEnum::Type err, const char* msg);
   M_NORETURN_FUNC void DoThrowStreamError(MErrorEnum::Type err, MConstLocalChars msg);
   M_NORETURN_FUNC virtual void DoThrowEndOfStream();

   virtual void DoSwap(MStream& stream);

private: // Disallow copy constructor and operators:

   MStream(const MStream&);
   const MStream& operator=(const MStream&);
   bool operator==(const MStream&) const;
   bool operator!=(const MStream&) const;

protected: // Data:

   unsigned          m_flags;
   StreamOpType      m_lastOp;
   MStream*          m_processor;
   unsigned          m_bytesSavedCount;
   char              m_bytesSaved [2];

   // Key, if the library is encrypted
   //
   MByteString m_key;

/// \endcond SHOW_INTERNAL

   M_DECLARE_CLASS(Stream)
};

/// Stream output utility operation that works on strings
///
M_FUNC MStream& operator<<(MStream&, const MStdString&);

/// Stream output utility operation that works on zero terminated strings
///
M_FUNC MStream& operator<<(MStream&, const char*);

/// Stream output utility operation that works on characters
///
/// This method writes a given character into a stream.
///
M_FUNC MStream& operator<<(MStream&, char);

#if !M_NO_WCHAR_T
/// Stream output utility operation that works on wide strings
///
M_FUNC MStream& operator<<(MStream&, const MWideString&);

/// Stream output utility operation that works on zero terminated wide strings
///
M_FUNC MStream& operator<<(MStream&, const wchar_t*);

/// Stream output utility operation that works on wide characters
///
/// This method writes a given character into a stream.
///
M_FUNC MStream& operator<<(MStream&, wchar_t);
#endif

/// Stream output utility operation that works on integers
///
/// This method writes a string representation of a given number into a stream.
///
M_FUNC MStream& operator<<(MStream&, int);

/// Stream output utility operation that works on unsigned integers
///
/// This method writes a string representation of a given number into a stream.
///
M_FUNC MStream& operator<<(MStream&, unsigned);

/// Stream output utility operation that works on doubles.
///
/// This method writes a string representation of double into a given stream.
///
M_FUNC MStream& operator<<(MStream&, double);

///@}
#endif
