#ifndef MCORE_MSTREAMFILE_H
#define MCORE_MSTREAMFILE_H
/// \file MCORE/MStreamFile.h
/// \addtogroup MCORE
///@{

#if !M_NO_FILESYSTEM

#include <MCORE/MStream.h>
#include <MCORE/MTime.h>

#if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
   struct AAsset;
#endif

/// Classic file stream capable of manipulating files in the file system.
///
/// The class inherits most of its power from the base MStream.
///
/// \see MStream
///
class M_CLASS MStreamFile : public MStream
{
public: // Type:

   /// These extra open mode flags are added to flags in MStream
   ///
   enum OpenFlags
   {
      FlagCreate    = 0x10000, ///< Always create a new file. This is the analog of standard POSIX flag O_CREAT.
      FlagNoReplace = 0x20000, ///< Only effective with FlagCreate, which when set, throws an error if file exists. This is the analog of standard POSIX flag O_EXCL.
      FlagTruncate  = 0x40000, ///< Open existing file and truncate it to an empty one. This is the analog of standard POSIX flag O_TRUNC.
      FlagAppend    = 0x80000  ///< Set file pointer at the end
   };

   /// Sharing flags, relevant only to Windows operating system.
   ///
   /// For non-Windows operating systems, the flag has no effect.
   ///
   enum SharingFlags
   {
      SharingAllowNone  = 0, ///< No sharing is allowed, an attempt to open the same file will fail
      SharingAllowRead  = 1, ///< While this file is open, same or another process or user can open file for reading
      SharingAllowWrite = 2, ///< While this file is open, same or another process or user can open file for writing
      SharingAllowAll   = 3  ///< While this file is open, same or another process or user can open file in any mode
   };

   /// Operating system dependent stream handle.
   ///
   /// For advanced uses, there is a way of accessing file handle directly.
   /// However caution should be taken by the developers, as the stream class
   /// can do its own buffering and data manipulation.
   ///
   /// \see MStreamFile(StreamFileHandle handle, bool handleOwned, unsigned flags, MConstChars name, bool handleStandardConsole)
   ///
   #if (M_OS & M_OS_POSIX) != 0
      typedef int StreamFileHandle;
   #else
      typedef HANDLE StreamFileHandle;
   #endif

public:

#if !M_NO_CONSOLE

   /// Standard input stream, analog of \c stdin that supports MStreamFile interface.
   ///
   /// Some operating systems, such as Windows CE, do not support standard streams and they do not have this static property.
   ///
   static MStreamFile* GetStdIn();

   /// Standard output stream, analog of \c stdout that supports MStreamFile interface.
   ///
   /// Some operating systems, such as Windows CE, do not support standard streams and they do not have this static property.
   ///
   static MStreamFile* GetStdOut();

   /// Standard error stream, analog of \c stderr that supports MStreamFile interface.
   ///
   /// Some operating systems, such as Windows CE, do not support standard streams and they do not have this static property.
   ///
   static MStreamFile* GetStdErr();

#endif

public: // Constructors and destructor:

   /// Default constructor that creates an uninitialized file object.
   ///
   /// The result file object is not open, no streaming operations are possible until \ref Open is called.
   ///
   /// \see Open
   ///
   MStreamFile();

   /// Create object and open a file stream by name.
   ///
   /// Depending on the flags given, the file can be created.
   ///
   /// \par fileName
   ///    File name, either relative or absolute.
   ///     - When relative path is specified, current directory of the process is taken into consideration.
   ///     - Absolute path will not depend on the current path of the process.
   ///     - UNIX-style forward slashes can be used for directory separation on any operating system, including Windows.
   ///     - On windows only, the Windows specific back slashes will work as usual.
   ///
   /// \par flags
   ///    Flags applicable to file operation.
   ///    Those are both, the generic stream specific flags, and flags relevant only to file stream.
   ///    Bitwise OR operation can be used to apply many flags together.
   ///    \if CPP
   ///      - See \ref MStream::OpenFlags for details of generic stream flags
   ///      - See \ref MStreamFile::OpenFlags for details of file stream flags
   ///    \endif
   /// \par sharing
   ///    This has effect only on Windows operating system, file sharing mode tells
   ///    if the same file can be manipulated at the same time by this or other process or user.
   ///    See \ref MStreamFile::SharingFlags for possible values.
   ///    This mode is a set of enumeration values, not a bit mask.
   ///
   /// Many types of errors can be reported by this constructor to indicate impossibility of the operation.
   /// The exact precondition will depend on the presence of the file, flags supplied, and permissions of the user
   /// who runs the process. Typical uses:
   /// \code
   ///    // Open an existing text file for reading.
   ///    // File with such name shall exist in the current directory.
   ///    MStreamFile f1("myfile.txt", MStream::FlagReadOnly | MStream::FlagText);
   ///
   ///    // Open a new binary file for writing.
   ///    // This has to be a new file, and if such file is present already there will be an error.
   ///    MStreamFile f2("/tmp/myfile.db", MStream::FlagWriteOnly | MStreamFile::FlagNoReplace);
   ///
   ///    // Open a new binary file for reading and writing, truncate it to zero size if it exists already.
   ///    MStreamFile f3("/tmp/myfile.db", MStream::FlagReadWrite | MStreamFile::FlagTruncate);
   /// \endcode
   ///
   /// \see Open
   ///
   MStreamFile(const MStdString& fileName, unsigned flags = FlagReadOnly, unsigned sharing = SharingAllowAll);  // SWIG_HIDE

   /// Creates the file stream based on the given operating system handle that was opened previously.
   ///
   /// Since by contract this constructor accepts an open handle, no system errors will be reported
   /// in this particular call, however those are possible when the object starts manipulating the handle.
   /// If the handle is owned, the stream will be closed at Close event or at object destruction,
   /// otherwise the stream will not be closed.
   ///
   /// \param handle
   ///     Handle that has to be open prior to calling this constructor. The particular type of the handle is operating system dependent.
   ///
   /// \param handleOwned
   ///     Whether the handle will be owned by the stream. It only matters at the time stream destructor is called.
   ///     If the handle is owned, it will be closed in stream destructor.
   ///
   /// \param flags
   ///     Which flags to use on an opened handle.
   ///
   /// \param name
   ///     Stream name, given so that the error messages that mention stream name can print it.
   ///     For streams without a place in the file hierarchy, or for generic streams, any sort of artificial name can be given.
   ///     For example, for \c stdin one can give "<stdin>". This parameter will only be used in error reporting.
   ///
   /// \param handleStandardConsole
   ///     This parameter should only be true if this stream is created to handle standard console.
   ///     Standard consoles have special behavior in the way their buffers are handled.
   ///
   MStreamFile(StreamFileHandle handle, bool handleOwned, unsigned flags, MConstChars name, bool handleStandardConsole = false); // SWIG_HIDE

   /// Destroy object, close the stream if it was opened previously.
   ///
   /// In certain cases, such as if this is a standard console input or output, the streams will not be closed.
   ///
   virtual ~MStreamFile() M_NO_THROW;

public: // properties:

   /// Open the file.
   ///
   /// If the object was an open file already, it will be closed first by this call.
   /// Depending on the flags given, the file can be created.
   ///
   /// \par fileName
   ///    File name, either relative or absolute.
   ///     - When relative path is specified, current directory of the process is taken into consideration.
   ///     - Absolute path will not depend on the current path of the process.
   ///     - UNIX-style forward slashes can be used for directory separation on any operating system, including Windows.
   ///     - On windows only, the Windows specific back slashes will work as usual.
   ///
   /// \par flags
   ///    Flags applicable to file operation.
   ///    Those are both the generic stream specific flags, and flags relevant only to file stream.
   ///    Bitwise OR operation can be used to apply many flags together.
   ///    \if CPP
   ///      - See \ref MStream::OpenFlags for details of generic stream flags
   ///      - See \ref MStreamFile::OpenFlags for details of file stream flags
   ///    \endif
   ///
   /// \if CPP
   /// \par sharing
   ///    This has effect only on Windows operating system, file sharing mode tells
   ///    if the same file can be manipulated at the same time by this or other process or user.
   ///    See \ref MStreamFile::SharingFlags for possible values.
   ///    This mode is a set of enumeration values, not a bit mask.
   /// \endif
   ///
   /// Many types of errors can be reported by this constructor to indicate impossibility of the operation.
   /// The exact precondition will depend on the presence of the file, flags supplied, and permissions of the user
   /// who runs the process. Typical uses:
   /// \code
   ///    // Open an existing text file for reading.
   ///    // File with such name shall exist in the current directory.
   ///    f1.Open("myfile.txt", MStream::FlagReadOnly | MStream::FlagText);
   ///
   ///    // Open a new binary file for writing.
   ///    // This has to be a new file, and if such file is present already there will be an error.
   ///    f2.Open("/tmp/myfile.db", MStream::FlagWriteOnly | MStreamFile::FlagNoReplace);
   ///
   ///    // Open a new binary file for reading and writing, truncate it to zero size if it exists already.
   ///    f3.Open("/tmp/myfile.db", MStream::FlagReadWrite | MStreamFile::FlagTruncate);
   /// \endcode
   ///
   void Open(const MStdString& filename, unsigned flags = FlagReadOnly, unsigned sharing = SharingAllowAll);

   /// Convenience static method that reads the whole file at once and returns it as string.
   ///
   /// This method is an equivalent of the following operations,
   /// but it does not expose the temporary file object:
   /// \code
   ///     MStreamFile(fileName, MStream::FlagReadOnly | MStream::FlagUseHeader).ReadAll();
   /// \endcode
   /// The file with such name shall exist for the operation to succeed.
   /// Notice that the file is not open in text mode.
   /// If file is the text, and it shall be manipulated string by string, \ref StaticReadAllLines can be more convenient.
   ///
   /// \see StaticReadAllLines - return the contents of a text file as a collection of strings
   ///
   static MByteString StaticReadAll(const MStdString& fileName);

   /// Convenience static method that reads the whole file at once and returns it as a collection of strings.
   ///
   /// This method is an equivalent of the following operations,
   /// but it does not expose the temporary file object:
   /// \code
   ///     MStreamFile(fileName, MStream::FlagReadOnly | MStream::FlagBuffered | MStream::FlagUseHeader).ReadAllLines();
   /// \endcode
   /// The file with such name shall exist for the operation to succeed.
   /// It is assumed that the file is text, otherwise the behavior might be unexpected as the attempt to handle
   /// carriage return and line feed characters will be made.
   /// For binary streams \ref StaticReadAll will be more appropriate.
   ///
   /// \see StaticReadAll - Returns the contents of the file as one single chunk of bytes.
   ///
   static MStdStringVector StaticReadAllLines(const MStdString& fileName);

#if !M_NO_TIME
   /// Static method to access time when the given file was last modified.
   ///
   /// This method is static, and it takes file name, as typically such operation does not have to open the file.
   /// The file shall exist for the operation to succeed and the path shall be accessible to user who shall
   /// have proper permissions, or an exception is thrown.
   ///
   static MTime GetModifyTime(const MStdString& fileName);
#endif

   /// Return a representative name of a stream.
   ///
   /// This method overwrites the parent implementation and gives the file name as supplied at Open call.
   ///
   virtual MStdString GetName() const; // See documentation in the base class

public: // Semi-private reflected methods:
/// \cond SHOW_INTERNAL

   /// Open a file with given name and open flags
   ///
   /// If the object was an open file already, it will be closed first by this call.
   /// Depending on the flags given, the file can be created.
   ///
   /// \par fileName
   ///    File name, either relative or absolute.
   ///     - When relative path is specified, current directory of the process is taken into consideration.
   ///     - Absolute path will not depend on the current path of the process.
   ///     - UNIX-style forward slashes can be used for directory separation on any operating system, including Windows.
   ///     - On windows only, the Windows specific back slashes will work as usual.
   ///
   /// \par flags
   ///    Flags applicable to file operation.
   ///    Those are both the generic stream specific flags, and flags relevant only to file stream.
   ///    Bitwise OR operation can be used to apply many flags together.
   ///    \if CPP
   ///      - See \ref MStream::OpenFlags for details of generic stream flags
   ///      - See \ref MStreamFile::OpenFlags for details of file stream flags
   ///    \endif
   ///
   /// Many types of errors can be reported by this constructor to indicate impossibility of the operation.
   /// The exact precondition will depend on the presence of the file, flags supplied, and permissions of the user
   /// who runs the process. Typical uses:
   /// \code
   ///    // Open an existing text file for reading.
   ///    // File with such name shall exist in the current directory.
   ///    f1.Open("myfile.txt", MStream::FlagReadOnly | MStream::FlagText);
   ///
   ///    // Open a new binary file for writing.
   ///    // This has to be a new file, and if such file is present already there will be an error.
   ///    f2.Open("/tmp/myfile.db", MStream::FlagWriteOnly | MStreamFile::FlagNoReplace);
   ///
   ///    // Open a new binary file for reading and writing, truncate it to zero size if it exists already.
   ///    f3.Open("/tmp/myfile.db", MStream::FlagReadWrite | MStreamFile::FlagTruncate);
   /// \endcode
   ///
   void DoOpen2(const MStdString& fileName, unsigned flags = FlagReadOnly);

   /// Open a file by name for reading.
   ///
   /// If the object was an open file already, it will be closed first by this call.
   /// The flags used by this method are FlagReadOnly.
   ///
   /// \par fileName
   ///    File name, either relative or absolute.
   ///     - When relative path is specified, current directory of the process is taken into consideration.
   ///     - Absolute path will not depend on the current path of the process.
   ///     - UNIX-style forward slashes can be used for directory separation on any operating system, including Windows.
   ///     - On windows only, the Windows specific back slashes will work as usual.
   ///
   /// Many types of errors can be reported by this constructor to indicate impossibility of the operation.
   /// The exact precondition will depend on the presence of the file, and permissions of the user
   /// who runs the process. Typical use:
   /// \code
   ///    // Open a new binary file for reading.
   ///    f2.Open("/tmp/myfile.db");
   /// \endcode
   ///
   void DoOpen1(const MStdString& fileName);

protected: // Parent overloads:

   virtual unsigned DoReadAvailableBytesImpl(char* buffer, unsigned count);

   virtual void DoWriteBytesImpl(const char* buffer, unsigned count);

   virtual void DoCloseImpl();

   virtual void DoFlushImpl(bool softFlush);
   bool DoIsOpenImpl() const;

   virtual unsigned DoGetPosition() const;
   virtual void DoSetPosition(unsigned position);
   virtual unsigned DoGetSize() const;
   virtual void DoSetSize(unsigned length);

/// \endcond SHOW_INTERNAL
private:

   StreamFileHandle m_handle;

#if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
   AAsset* m_asset;
#endif

   bool m_handleOwned;

   bool m_handleStandardConsole;

   MStdString m_fileName;

   M_DECLARE_CLASS(StreamFile)
};

#endif // !M_NO_FILESYSTEM

///@}
#endif
