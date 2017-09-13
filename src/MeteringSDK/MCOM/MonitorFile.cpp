// File MCOM/MonitorFile.cpp
#include "MCOMExtern.h"
#include "MCOMExceptions.h"
#include "MonitorFile.h"
#include "MonitorFilePrivateThread.h"
#include <MCORE/MTime.h>

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING && !M_NO_FILESYSTEM

   #if !M_NO_REFLECTION
      static MMonitorFile* DoNew1(const MStdString& clientAddress)
      {
         return M_NEW MMonitorFile(clientAddress);
      }

      /// Constructor that creates a file monitor without giving a file name.
      ///
      /// No monitoring will be done until the file name is assigned.
      ///
      /// \pre The object should be created on a heap with operator new,
      /// and handled with a shared pointer, otherwise the behavior is undefined.
      /// File, if given, shall be a valid directory or a file name,
      /// but the check is deferred to the attaching time.
      ///
      static MMonitorFile* DoNew0()
      {
         return DoNew1(MVariant::s_emptyString);
      }
   #endif

M_START_PROPERTIES(MonitorFile)
   M_OBJECT_PROPERTY_UINT           (MonitorFile, MaxFileSizeKB)
   M_OBJECT_PROPERTY_BOOL           (MonitorFile, Obfuscate)
   M_OBJECT_PROPERTY_STRING         (MonitorFile, FileName, ST_constMStdStringA_X, ST_X_constMStdStringA)
M_START_METHODS(MonitorFile)
   M_OBJECT_SERVICE                 (MonitorFile, DeleteFile,     ST_X)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(MonitorFile, New, DoNew1, 1, ST_MObjectP_S_constMStdStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED(MonitorFile, New, DoNew0, 0, ST_MObjectP_S)
M_END_CLASS(MonitorFile, Monitor)

MMonitorFile::MMonitorFile(const MStdString& fileName)
:
   m_foregroundThreadBufferLock(),
   m_fileLock(),
   m_foregroundThreadBuffer(),
   m_maxFileSizeKB(0u),
   m_fileName(),
   m_logFile(NULL),
   m_syncMessagePosted(false),
   m_isFinished(false),
   m_obfuscate(false),
   m_fileWasDeleted(false)
{
   SetFileName(fileName);
   MMonitorFilePrivateThread::AttachMonitor(this); // this can attempt to attach the monitor again, no problem
}

MMonitorFile::~MMonitorFile()
{
   DoFinish();
   DoFileDetach();
   delete m_logFile;
}

void MMonitorFile::SetMaxFileSizeKB(unsigned size)
{
   if ( size != 0 )
      MENumberOutOfRange::CheckInteger(64, 0x7FFF, int(size), M_OPT_STR("MaxFileSizeKB"));

   MCriticalSection::Locker lock(m_fileLock);
   m_maxFileSizeKB = size; // set the local value
   if ( m_logFile != NULL )
      m_logFile->SetMaxFileSizeKB(size);
}

void MMonitorFile::SetFileName(const MStdString& name)
{
   MCriticalSection::Locker lock(m_fileLock);
   if ( name != m_fileName || m_fileWasDeleted )
   {
      if ( !m_fileName.empty() )
         OnIdle(); // do this once "by hand" on the foreground thread

      m_fileWasDeleted = false;
      m_fileName = name; // and set the local value too
      if ( name.empty() )
         DoFileDetach();
      else
      {
         if ( m_logFile == NULL )
            m_logFile = M_NEW MLogFileWriter;
         m_logFile->SetObfuscate(m_obfuscate);
         m_logFile->Open(name, m_maxFileSizeKB); // it is okay if the object is created, but the exception is thrown
         m_listening = -1;
      }
   }
}

void MMonitorFile::SetObfuscate(bool yes)
{
   m_obfuscate = yes;
   if ( m_logFile != NULL )
      m_logFile->SetObfuscate(m_obfuscate);
}

void MMonitorFile::DoFileDetach()
{
   MCriticalSection::Locker lock(m_fileLock);
   m_fileWasDeleted = false;
   if ( m_logFile != NULL )
      m_logFile->Close();
}

void MMonitorFile::DeleteFile()
{
   MCriticalSection::Locker lock(m_fileLock);
   if ( !m_fileName.empty() && MUtilities::IsPathExisting(m_fileName) )
   {
      DoFileDetach();
      MUtilities::DeleteFile(m_fileName); // this will report a good error if the file was not deleted for some reason
      m_fileWasDeleted = true;
   }
}

void MMonitorFile::Attach(const MStdString& mediaIdentification)
{
   m_listening = -1; // initiate sending of the information
   m_syncMessagePosted = 0;
   MMonitor::Attach(mediaIdentification);
}

void MMonitorFile::Detach()
{
   MMonitor::Detach();
   DoFileDetach();
}

void MMonitorFile::OnMessage(MMonitor::MessageType code, const char* data, int length)
{
   MMonitor::OnMessage(code, data, length);

   if ( m_fileWasDeleted && !m_fileName.empty() )
   {
      try
      {
         SetFileName(m_fileName);
      }
#if M_DEBUG
      catch ( MException& ex )
      {
         M_USED_VARIABLE(ex);
         M_ASSERT(0);
      }
#endif
      catch ( ... )
      {
         M_ASSERT(0);  // never consider an error in release, we are possibly communicating
      }
   }

   unsigned currentTimestamp = MUtilities::GetTickCount();
   MLogFile::PacketHeader header;
   header.m_length = Muint32(length + MLogFile::PACKET_HEADER_SIZE);
   header.m_timeStamp = Muint32(currentTimestamp);
   header.m_code = Muint16(code);

   {
      MCriticalSection::Locker lock(m_foregroundThreadBufferLock);
      m_foregroundThreadBuffer.append((const char*)&header,  MLogFile::PACKET_HEADER_SIZE);
      m_foregroundThreadBuffer.append(data, length);
   }

   // send synchronize message
   if ( !m_syncMessagePosted && code != MessageProtocolSynchronize )
      PostSyncMessage();
}

void MMonitorFile::OnPageBoundHit()
{
   m_syncMessagePosted = false;
}

void MMonitorFile::DoFinish()
{
   if ( !m_isFinished ) // this could be finished earlier
   {
      m_isFinished = true;
      MMonitorFilePrivateThread::DetachMonitor(this);
      OnIdle(); // do this once "by hand" on the foreground thread
   }
}

void MMonitorFile::OnIdle() M_NO_THROW
{
   try
   {
      MByteString backgroundThreadBuffer;

      {
         // Do this operation in a separate scope to minimize locking
         MCriticalSection::Locker lock(m_foregroundThreadBufferLock);
         m_foregroundThreadBuffer.swap(backgroundThreadBuffer);
         m_foregroundThreadBuffer.clear();
      }

      if ( m_listening != 0 && !backgroundThreadBuffer.empty() )
      {
         unsigned result = DoSendBackgroundBuffer(backgroundThreadBuffer);
         if ( result == 0 && m_client == NULL ) // only in this case reset `m_listening`
            m_listening = 0;
      }
   }
   catch ( ... )
   {
      // ignore errors in release version
      M_ASSERT(0);
   }
}

unsigned MMonitorFile::DoSendBackgroundBuffer(const MByteString& backgroundThreadBuffer)
{
   MCriticalSection::Locker lock(m_fileLock);
   if ( m_logFile != NULL && m_logFile->IsOpen() )
   {
      try
      {
         // set listener
         m_logFile->SetListener(this);
         m_logFile->WriteMultipleMessages(backgroundThreadBuffer);
         // unset listener (helpful for debug)
         m_logFile->SetListener(NULL);
         return (unsigned)-1; // success, we are interested in data
      }
      catch ( ... )
      {
      }
   }
   return 0; // we are not interested in further data for whatever reason
}

void MMonitorFile::PostSyncMessage()
{
   char buffer [ 64 ];
#if M_OS & M_OS_POSIX
   struct timeval tv;
   struct timezone tz;
   gettimeofday(&tv, &tz);
   MTime t(tv.tv_sec + tz.tz_minuteswest * 60);
   // format is YYYY.MM.DD hh:mm:ss.ms
   size_t buflen = MFormat(buffer, sizeof(buffer), "Timestamp %04u.%02u.%02u %02u:%02u:%02u.%03u",
                    t.GetYear(), t.GetMonth(), t.GetDayOfMonth(),
                    t.GetHours(), t.GetMinutes(), t.GetSeconds(), tv.tv_usec / 1000);
#else
   SYSTEMTIME st;
   GetSystemTime(&st);
   // format is YYYY.MM.DD hh:mm:ss.ms
   size_t buflen = MFormat(buffer, sizeof(buffer), "Timestamp %04u.%02u.%02u %02u:%02u:%02u.%03u",
                    st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#endif
   M_ASSERT(buflen > 0 && buflen < sizeof(buffer)); // check if the supplied buffer fits (due to the supplied format it does)
   OnMessage(MMonitor::MessageProtocolSynchronize, buffer, static_cast<int>(buflen));
   m_syncMessagePosted = true;
}

#endif // !M_NO_MCOM_MONITOR
