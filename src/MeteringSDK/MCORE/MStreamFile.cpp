// File MCORE/MStreamFile.cpp

#include "MCOREExtern.h"
#include "MStreamFile.h"
#include "MException.h"
#include "MStreamProcessor.h"
#include "MJavaEnv.h"

#if !M_NO_FILESYSTEM

   #if !M_NO_REFLECTION

      /// Default constructor that creates an uninitialized file object.
      ///
      /// The result file object is not open, no streaming operations are possible until \ref Open is called
      ///
      /// \see Open
      ///
      static MObject* DoNew0()
      {
         return M_NEW MStreamFile;
      }

      /// Create object, and open a file stream by name for reading.
      ///
      /// The file with such name shall exist, and be a simple file as no header is processed by the call.
      ///
      /// \par fileName
      ///    File name, either relative or absolute.
      ///     - When relative path is specified, current directory of the process is taken into consideration.
      ///     - Absolute path will not depend on the current path of the process.
      ///     - UNIX-style forward slashes can be used for directory separation on any operating system, including Windows.
      ///     - On windows only, the Windows specific back slashes will work as usual.
      ///
      /// Many types of errors can be reported by this constructor to indicate impossibility of the operation.
      /// In general, the file should exist, and the user who runs the process shall have proper permissions.
      ///
      static MObject* DoNew1(const MStdString& str)
      {
         return M_NEW MStreamFile(str);
      }

      /// Create object, and open a file stream by name.
      ///
      /// Depending on the flags given, this can create a file if it is not present, or truncate it if necessary.
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
      ///      - See \ref MStream::OpenFlags for details of generic stream flags.
      ///      - See \ref MStreamFile::OpenFlags for details of file stream flags.
      ///    \endif
      ///
      /// Many types of errors can be reported by this constructor to indicate impossibility of the operation.
      ///
      static MObject* DoNew2(const MStdString& str, unsigned flags)
      {
         return M_NEW MStreamFile(str, flags);
      }

      /// Create object, and open a file stream by name.
      ///
      /// Depending on the flags given, this can create a file if it is not present, or truncate it if necessary.
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
      ///
      /// \par sharing
      ///    This has effect only on Windows operating system, file sharing mode tells
      ///    if the same file can be manipulated at the same time by this or other process or user.
      ///    This mode is a set of enumeration values, not a bit mask.
      ///
      /// Many types of errors can be reported by this constructor to indicate impossibility of the operation.
      ///
      static MObject* DoNew3(const MStdString& str, unsigned flags, unsigned sharing)
      {
         return M_NEW MStreamFile(str, flags, sharing);
      }

   #endif

M_START_PROPERTIES(StreamFile)
   M_CLASS_ENUMERATION_UINT         (StreamFile, FlagCreate)
   M_CLASS_ENUMERATION_UINT         (StreamFile, FlagNoReplace)
   M_CLASS_ENUMERATION_UINT         (StreamFile, FlagTruncate)
   M_CLASS_ENUMERATION_UINT         (StreamFile, FlagAppend)
   M_CLASS_ENUMERATION_UINT         (StreamFile, SharingAllowNone)
   M_CLASS_ENUMERATION_UINT         (StreamFile, SharingAllowRead)
   M_CLASS_ENUMERATION_UINT         (StreamFile, SharingAllowWrite)
   M_CLASS_ENUMERATION_UINT         (StreamFile, SharingAllowAll)
#if !M_NO_CONSOLE
   M_CLASS_PROPERTY_READONLY_OBJECT (StreamFile, StdIn)
   M_CLASS_PROPERTY_READONLY_OBJECT (StreamFile, StdOut)
   M_CLASS_PROPERTY_READONLY_OBJECT (StreamFile, StdErr)
#endif
M_START_METHODS(StreamFile)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(StreamFile, New,    DoNew3,  3, ST_MObjectP_S_constMStdStringA_unsigned_unsigned)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(StreamFile, New,    DoNew2,  2, ST_MObjectP_S_constMStdStringA_unsigned)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(StreamFile, New,    DoNew1,  1, ST_MObjectP_S_constMStdStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(StreamFile, New,    DoNew0,  0, ST_MObjectP_S)
   M_OBJECT_SERVICE_OVERLOADED      (StreamFile, Open,   Open,    3, ST_X_constMStdStringA_unsigned_unsigned)
   M_OBJECT_SERVICE_OVERLOADED      (StreamFile, Open,   DoOpen2, 2, ST_X_constMStdStringA_unsigned)  // SWIG_HIDE
   M_OBJECT_SERVICE_OVERLOADED      (StreamFile, Open,   DoOpen1, 1, ST_X_constMStdStringA)           // SWIG_HIDE
   M_CLASS_SERVICE                  (StreamFile, StaticReadAll,      ST_MByteString_S_constMStdStringA)
   M_CLASS_SERVICE                  (StreamFile, StaticReadAllLines, ST_MStdStringVector_S_constMStdStringA)
#if !M_NO_TIME
   M_CLASS_SERVICE                  (StreamFile, GetModifyTime,      ST_MObjectByValue_S_constMStdStringA)
#endif
M_END_CLASS(StreamFile, Stream)

#if (M_OS & M_OS_WINDOWS) != 0 && !defined(INVALID_SET_FILE_POINTER)
   #define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

MStreamFile::MStreamFile()
:
   MStream(),
   #if (M_OS & M_OS_POSIX) != 0
      m_handle(-1),
   #else
      m_handle(INVALID_HANDLE_VALUE),
   #endif
   #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
      m_asset(NULL),
   #endif
   m_handleOwned(false),
   m_handleStandardConsole(false),
   m_fileName()
{
}

MStreamFile::MStreamFile(const MStdString& filename, unsigned flags, unsigned sharing)
:
   MStream(),
   #if (M_OS & M_OS_POSIX) != 0
      m_handle(-1),
   #else
      m_handle(INVALID_HANDLE_VALUE),
   #endif
   #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
      m_asset(NULL),
   #endif
   m_handleOwned(false),
   m_handleStandardConsole(false),
   m_fileName()
{
   Open(filename, flags, sharing);
}

MStreamFile::MStreamFile(StreamFileHandle handle, bool handleOwned, unsigned flags, MConstChars name, bool handleStandardConsole)
:
   MStream(),
   #if (M_OS & M_OS_POSIX) != 0
      m_handle(-1),
   #else
      m_handle(INVALID_HANDLE_VALUE),
   #endif
   #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
      m_asset(NULL),
   #endif
   m_handleOwned(false),
   m_handleStandardConsole(false),
   m_fileName()
{
   DoStartOpen(flags);
   m_handle = handle; // only then change handle
   m_handleOwned = handleOwned;
   m_handleStandardConsole = handleStandardConsole;
   if ( name != NULL )
      m_fileName = name;
   #if (M_OS & M_OS_POSIX) != 0
      M_ASSERT(m_handle != -1);
   #else
      M_ASSERT(m_handle != INVALID_HANDLE_VALUE);
   #endif
   DoFinishOpen();
}

MStreamFile::~MStreamFile() M_NO_THROW
{
   try
   {
      Close();
   }
#if M_DEBUG
   catch ( MException& ex )
   {
      M_USED_VARIABLE(ex); // debug helper, set the breakpoint here
      M_ASSERT(0);
   }
#else
   catch ( ... )
   {
   }
#endif
}

void MStreamFile::DoOpen1(const MStdString& fileName)
{
   Open(fileName);
}

void MStreamFile::DoOpen2(const MStdString& fileName, unsigned flags)
{
   Open(fileName, flags);
}

void MStreamFile::Open(const MStdString& fileName, unsigned flags, unsigned sharing)
{
   Close();

   m_handleOwned = true;
   m_handleStandardConsole = false;
   m_fileName = fileName;

   {
      if ( (flags & (MStreamFile::FlagNoReplace | MStreamFile::FlagCreate)) == MStreamFile::FlagNoReplace )
      {
         DoThrowStreamSoftwareError(M_ERR_BAD_STREAM_FLAG, M_OPT_STR("FlagNoReplace for '%s' is only valid together with FlagCreate"));
         M_ENSURED_ASSERT(0);
      }
      DoStartOpen(flags);

      #if (M_OS & M_OS_POSIX) != 0

         #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
            if ( !m_fileName.empty() && m_fileName[0] == ':' ) // asset within APK
            {
               AAssetManager* assetManager = MJavaEnv::GetJniAssetManager();
               const char* name = m_fileName.c_str() + 1;
               m_asset = AAssetManager_open(assetManager, name, AASSET_MODE_STREAMING);
               if ( m_asset == NULL )
               {
                  MESystemError::ThrowFileNotOpen(m_fileName);
                  M_ENSURED_ASSERT(0);
               }
            }
            else
         #endif
            {
               int flags = 0;
               if ( (m_flags & MStreamFile::FlagReadWrite) == MStreamFile::FlagReadWrite )
                  flags = O_RDWR;
               else if ( (m_flags & MStreamFile::FlagReadOnly) != 0 )
                  flags = O_RDONLY;
               else if ( (m_flags & MStreamFile::FlagWriteOnly) != 0 )
                  flags = O_WRONLY;

               if ( (m_flags & MStreamFile::FlagAppend) != 0 )
                  flags |= O_APPEND;
               if ( (m_flags & MStreamFile::FlagCreate) != 0 )
                  flags |= O_CREAT;
               if ( (m_flags & MStreamFile::FlagTruncate) != 0 )
                  flags |= O_TRUNC;
               if ( (m_flags & MStreamFile::FlagNoReplace) != 0 )
                  flags |= O_EXCL;

               m_handle = open(m_fileName.c_str(), flags, 0666);
               if ( m_handle == -1 )
               {
                  MESystemError::ThrowFileNotOpen(m_fileName);
                  M_ENSURED_ASSERT(0);
               }
            }

      #elif (M_OS & M_OS_WINDOWS) != 0

         M_COMPILED_ASSERT(MStreamFile::FlagReadWrite == (MStreamFile::FlagReadOnly | MStreamFile::FlagWriteOnly));

         DWORD desiredAccess = 0;
         if ( (m_flags & MStreamFile::FlagReadOnly) != 0 )
            desiredAccess |= GENERIC_READ;
         if ( (m_flags & MStreamFile::FlagWriteOnly) != 0 )
            desiredAccess |= GENERIC_WRITE;
         M_ASSERT(desiredAccess != 0); // there is a check in DoStartOpen to make sure either FlagReadOnly or FlagWriteOnly or both are supplied

         DWORD disposition;
         switch ( m_flags & (MStreamFile::FlagCreate | MStreamFile::FlagTruncate | MStreamFile::FlagNoReplace) )
         {
         case MStreamFile::FlagCreate:
            disposition = OPEN_ALWAYS;
            break;
         case MStreamFile::FlagCreate | MStreamFile::FlagTruncate:
            disposition = CREATE_ALWAYS;
            break;
         case MStreamFile::FlagCreate | MStreamFile::FlagNoReplace:
         case MStreamFile::FlagCreate | MStreamFile::FlagNoReplace | MStreamFile::FlagTruncate:
            disposition = CREATE_NEW;
            break;
         case MStreamFile::FlagTruncate:
         case MStreamFile::FlagTruncate | MStreamFile::FlagNoReplace:
            disposition = TRUNCATE_EXISTING;
            break;
         default: // notice, FlagNoReplace without FlagCreate has no effect
            disposition = OPEN_EXISTING;
            break;
         }

         // Note: Sharing flags are the same as in Windows API, so no conversion is needed.
         #if M_UNICODE
            m_handle = ::CreateFile(MToWideString(fileName).c_str(), desiredAccess, (DWORD)sharing, 0, disposition, FILE_FLAG_RANDOM_ACCESS, 0);
         #else
            m_handle = ::CreateFile(fileName.c_str(), desiredAccess, (DWORD)sharing, 0, disposition, FILE_FLAG_RANDOM_ACCESS, 0);
         #endif
         if ( m_handle == INVALID_HANDLE_VALUE )
         {
            MESystemError::ThrowFileNotOpen(fileName);
            M_ENSURED_ASSERT(0);
         }

         if ( (m_flags & MStreamFile::FlagAppend) != 0 )
         {
            DWORD ptr = ::SetFilePointer(m_handle, 0, 0, SEEK_END);
            MESystemError::CheckLastSystemError(ptr == INVALID_SET_FILE_POINTER);
         }

      #else
         #error "Implement file open for this OS"
      #endif
      DoFinishOpen();
   }
}

MByteString MStreamFile::StaticReadAll(const MStdString& fileName)
{
   MStreamFile file(fileName, FlagReadOnly);
   return file.ReadAll();
}

MStdStringVector MStreamFile::StaticReadAllLines(const MStdString& fileName)
{
   MStreamFile file(fileName, FlagReadOnly | FlagBuffered);
   return file.ReadAllLines();
}

#if !M_NO_TIME
MTime MStreamFile::GetModifyTime(const MStdString& fileName)
{

#if M_OS & M_OS_WINDOWS
#if M_UNICODE
   HANDLE hFile = ::CreateFile(MToWideString(fileName).c_str(), 0,
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                               OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0 );
#else
   HANDLE hFile = ::CreateFile(fileName.c_str(), 0,
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                               OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0 );
#endif
   if ( hFile == NULL )
   {
      MESystemError::ThrowFileNotOpen(fileName);
      M_ENSURED_ASSERT(0);
   }
   FILETIME fmt;
   SYSTEMTIME smt;
   if ( GetFileTime(hFile, 0, 0, &fmt) == 0 )
   {
      MESystemError::ThrowLastSystemError();
      M_ENSURED_ASSERT(0);
   }
   FileTimeToSystemTime (&fmt, &smt);
   MTime modifyTime = MTime (smt.wYear, smt.wMonth, smt.wDay, smt.wHour, smt.wMinute, smt.wSecond);
   CloseHandle(hFile);
   return modifyTime;
#else
   struct stat _buf;
   if ( stat(fileName.c_str(), &_buf) != 0 )
   {
      MESystemError::ThrowLastSystemError();
      M_ENSURED_ASSERT(0);
   }
   MTime modifyTime ( _buf.st_mtime);
   return modifyTime;
#endif
}
#endif

unsigned MStreamFile::DoGetSize() const
{
   #if (M_OS & M_OS_POSIX) != 0
      #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
         if ( m_asset != NULL ) // asset within APK
            return AAsset_getLength(m_asset);
      #endif
      struct stat st;
      int result = fstat(m_handle, &st);
      MESystemError::CheckLastSystemError(result == -1);
      return unsigned(st.st_size);
   #elif (M_OS & M_OS_WINDOWS) != 0
      DWORD result = ::GetFileSize(m_handle, 0);
      MESystemError::CheckLastSystemError(result == INVALID_FILE_SIZE);
      return unsigned(result);
   #else
      #error "Implement GetSize for this OS"
   #endif
}

void MStreamFile::DoSetSize(unsigned size)
{
   #if M_OS & M_OS_POSIX
      #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
         if ( m_asset != NULL ) // asset within APK
         {
            MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_INVALID_OPERATION_ON_APK_ASSET, "Cannot change an asset within apk"));
            M_ENSURED_ASSERT(0);
         }
      #endif
      int result = ftruncate(m_handle, size);
      MESystemError::CheckLastSystemError(result == -1);
   #elif (M_OS & M_OS_WINDOWS) != 0
      SetPosition(size);
      BOOL result = ::SetEndOfFile(m_handle);
      MESystemError::CheckLastSystemError(!result);
   #else
      #error "Implement SetSize for this OS"
   #endif
}

unsigned MStreamFile::DoGetPosition() const
{
   #if (M_OS & M_OS_POSIX) != 0
      off_t result;
      #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
         if ( m_asset != NULL ) // asset within APK
            result = AAsset_seek(m_asset, 0, SEEK_CUR);
         else
      #endif
            result = lseek(m_handle, 0, SEEK_CUR);
      MESystemError::CheckLastSystemError(result == -1);
      return static_cast<unsigned>(result);
   #elif (M_OS & M_OS_WINDOWS) != 0
      DWORD result = ::SetFilePointer(m_handle, 0, 0, FILE_CURRENT);
      MESystemError::CheckLastSystemError(result == INVALID_SET_FILE_POINTER);
      return static_cast<unsigned>(result);
   #else
      #error "Implement GetPosition for this OS"
   #endif
}

void MStreamFile::DoSetPosition(unsigned position)
{
      // Note: Origin values are fit to SetFilePointer function
   #if (M_OS & M_OS_POSIX) != 0
      off_t result;
      #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
         if ( m_asset != NULL ) // asset within APK
            result = AAsset_seek(m_asset, position, SEEK_SET);
         else
      #endif
            result = lseek(m_handle, position, SEEK_SET);
      MESystemError::CheckLastSystemError(result == -1);
   #elif (M_OS & M_OS_WINDOWS) != 0
      DWORD result = ::SetFilePointer(m_handle, position, 0, FILE_BEGIN);
      MESystemError::CheckLastSystemError(result == INVALID_SET_FILE_POINTER);
   #else
      #error "Implement SetPosition for this OS"
   #endif
}

unsigned MStreamFile::DoReadAvailableBytesImpl(char* buffer, unsigned count)
{
   #if (M_OS & M_OS_POSIX) != 0
      ssize_t bytesRead;
      #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
         if ( m_asset != NULL ) // asset within APK
            bytesRead = AAsset_read(m_asset, buffer, count);
         else
      #endif
            bytesRead = read(m_handle, buffer, count);
      MESystemError::CheckLastSystemError(bytesRead == -1);
   #elif (M_OS & M_OS_WINDOWS) != 0
      DWORD bytesRead = 0;
      BOOL result = ::ReadFile(m_handle, buffer, count, &bytesRead, 0);
      MESystemError::CheckLastSystemError(!result);
   #else
      #error "Implement DoReadAvailableBytesImpl for this OS"
   #endif
   return static_cast<unsigned>(bytesRead);
}

void MStreamFile::DoWriteBytesImpl(const char* buffer, unsigned count)
{
   #if (M_OS & M_OS_POSIX) != 0
      #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
         if ( m_asset != NULL ) // asset within APK
         {
            MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_INVALID_OPERATION_ON_APK_ASSET, "Cannot change an asset within apk"));
            M_ENSURED_ASSERT(0);
         }
      #endif
      ssize_t bytesWritten = write(m_handle, buffer, count);
      MESystemError::CheckLastSystemError(bytesWritten == -1);
      M_ASSERT(bytesWritten == ssize_t(count));
   #elif (M_OS & M_OS_WINDOWS) != 0
      DWORD bytesWritten = 0;
      BOOL result = ::WriteFile(m_handle, buffer, count, &bytesWritten, 0);
      MESystemError::CheckLastSystemError(!result);
   #else
      #error "Implement DoWriteBytesImpl for this OS"
   #endif
}

void MStreamFile::DoCloseImpl()
{
   if ( m_handleOwned )
   {
      #if (M_OS & M_OS_POSIX) != 0
         #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
            if ( m_asset != NULL ) // asset within APK
            {
               M_ASSERT(m_handle == -1); // either one or another
               AAsset_close(m_asset);
               m_asset = NULL;
               return;
            }
         #endif
         if ( m_handle != -1 )
         {
            close(m_handle);
            m_handle = -1;
         }
      #elif (M_OS & M_OS_WINDOWS) != 0
         if ( m_handle != INVALID_HANDLE_VALUE )
         {
            ::CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
         }
      #else
         #error "Implement DoCloseImpl for this OS"
      #endif
   }
}

bool MStreamFile::DoIsOpenImpl() const
{
   #if (M_OS & M_OS_POSIX) != 0
      #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
         if ( m_asset != NULL ) // asset within APK
         {
            M_ASSERT(m_handle == -1); // either one or another
            return true;
         }
      #endif
      return m_handle != -1;
   #elif (M_OS & M_OS_WINDOWS) != 0
      return m_handle != INVALID_HANDLE_VALUE;
   #else
      #error "Implement DoIsOpenImpl for this OS"
   #endif
}

void MStreamFile::DoFlushImpl(bool softFlush)
{
   if ( !softFlush )
   {
      // As it appears, flush buffer operation is extremely slow.
      // This is the reason of introducing softFlush parameter.

      #if (M_OS & M_OS_POSIX) != 0
         #if (M_OS & M_OS_ANDROID) != 0 && !M_NO_JNI
            if ( m_asset != NULL ) // asset within APK
               return; // do nothing for assets
         #endif
         const int status = fsync(m_handle);
         MESystemError::CheckLastSystemError(status);
      #elif (M_OS & M_OS_WINDOWS) != 0

         // According to MSDN ::FlushFileBuffers:
         //    The function fails if hFile is a handle to the console output. That is because the console output is not buffered.
         // Therefore, consider this to be okay.
         if ( !m_handleStandardConsole )
         {
            BOOL result = ::FlushFileBuffers(m_handle);
            MESystemError::CheckLastSystemError(!result);
         }

      #else
         #error "Implement DoFlushImpl for this OS"
      #endif
   }
}

MStdString MStreamFile::GetName() const
{
   return m_fileName;
}

#if !M_NO_CONSOLE

   #if (M_OS & M_OS_POSIX) != 0
      #define STD_INPUT_HANDLE   (unsigned)(-10)
      #define STD_OUTPUT_HANDLE  (unsigned)(-11)
      #define STD_ERROR_HANDLE   (unsigned)(-12)
   #endif

   static MStreamFile* s_stdIn = NULL;
   static MStreamFile* s_stdOut = NULL;
   static MStreamFile* s_stdErr = NULL;

   static struct MStdStreamFileDeleter
   {
      // Silent destroyer is necessary because at the program exit there might be cases when
      // i/o streams are no longer open, handle is invalid.
      //
      static void DestroySilently(MStreamFile*& s) M_NO_THROW;

      ~MStdStreamFileDeleter()
      {
         DestroySilently(s_stdIn);
         DestroySilently(s_stdOut);
         DestroySilently(s_stdErr);
      }
   } s_stdStreamFileDeleter;

   void MStdStreamFileDeleter::DestroySilently(MStreamFile*& s) M_NO_THROW
   {
      if ( s != NULL )
      {
         try
         {
            s->Close();
         }
         catch ( ... )
         {
         }
         try
         {
            delete s;
         }
         catch ( ... )
         {
         }
         s = NULL;  // nullify pointers for an unlikely but possible case something gets printed after pointer is deleted
      }
   }

   #if M_OS & M_OS_POSIX

      #define _M_STDIN   stdin
      #define _M_STDOUT  stdout
      #define _M_STDERR  stderr

      static MStreamFile* DoCreateStandardStream(FILE* file, unsigned flags, MConstChars name)
      {
         MStreamFile::StreamFileHandle handle = fileno(file);
         return M_NEW MStreamFile(handle, false, flags, name);
      }

   #else

      #define _M_STDIN   STD_INPUT_HANDLE
      #define _M_STDOUT  STD_OUTPUT_HANDLE
      #define _M_STDERR  STD_ERROR_HANDLE

      static MStreamFile* DoCreateStandardStream(DWORD handleType, unsigned flags, MConstChars name)
      {
         const MStreamFile::StreamFileHandle handle = ::GetStdHandle(handleType);
         return M_NEW MStreamFile(handle, false, flags, name, true);
      }

   #endif

MStreamFile* MStreamFile::GetStdIn()
{
   if ( s_stdIn == NULL )
      s_stdIn = DoCreateStandardStream(_M_STDIN, MStreamFile::FlagReadOnly, "stdin");
   return s_stdIn;
}

MStreamFile* MStreamFile::GetStdOut()
{
   if ( s_stdOut == NULL )
      s_stdOut = DoCreateStandardStream(_M_STDOUT, MStreamFile::FlagWriteOnly, "stdout");
   return s_stdOut;
}

MStreamFile* MStreamFile::GetStdErr()
{
   if ( s_stdErr == NULL )
      s_stdErr = DoCreateStandardStream(_M_STDERR, MStreamFile::FlagWriteOnly, "stderr");
   return s_stdErr;
}

#endif // !M_NO_CONSOLE
#endif // !M_NO_FILESYSTEM
