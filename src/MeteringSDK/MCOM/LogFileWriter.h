#ifndef MCOM_LOGFILEWRITER_H
#define MCOM_LOGFILEWRITER_H
/// \addtogroup MCOM
///@{
/// \file MCOM/LogFileWriter.h

#include <MCOM/LogFile.h>

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING && !M_NO_FILESYSTEM

/// Log file writer, one that writes information to a file.
/// It can be used separately from MMonitorFile to read and produce the log with the possible 
/// circular buffer behavior.
///
class MCOM_CLASS MLogFileWriter : public MLogFile
{
public: // Constructor and destructor:

   /// Constructor that creates an uninitialized log file object.
   ///
   MLogFileWriter()
   :
      MLogFile(),
      m_maxFileSizeKB(0),
      m_maxNumberOfPages(0)
   {
   }

   /// Constructor that creates a writable log file with the given file name.
   /// The value of maxFileSizeKB is taken into account to possibly limit the file size.
   ///
   /// \pre File, if given, shall be a valid file name, or an exception is thrown.
   /// Write access to a file is attempted. If anything fails there is an exception.
   ///
   explicit MLogFileWriter(const MStdString& fileName, unsigned maxFileSizeKB = 0)
   :
      MLogFile(),
      m_maxFileSizeKB(0),
      m_maxNumberOfPages(0)
   {
      Open(fileName, maxFileSizeKB);
   }

   /// Object destructor, close the log file.
   ///
   virtual ~MLogFileWriter() M_NO_THROW;

public: // Services (property getters and setters):

   /// Get the maximum file size in kilobytes, or zero if the maximum file size is not set.
   /// Zero, there is no imposed restriction on the file size, is the default.
   /// Once the maximum size is achieved, the old data will start to disappear from the file as new data is added.
   ///
   unsigned GetMaxFileSizeKB() const
   {
      return m_maxFileSizeKB;
   }

   /// Set the maximum file size in kilobytes, or zero if the maximum file size shall not be watched.
   /// Zero, there is no imposed restriction on the file size, is the default.
   /// Once the maximum size is achieved, the old data will start to disappear from the file as new data is added.
   ///
   /// If a file exists prior the call, but its size is bigger than the imposing restriction, the
   /// file will be truncated (which can possibly take a significant amount of time).
   ///
   /// \pre The size shall either be zero, no size restriction, or it shall be in range 
   /// 16 to approximately 32,000 kilobytes (which corresponds to 32 megabytes), or a range expression takes place.
   ///
   void SetMaxFileSizeKB(unsigned size);

public: // Services:

   /// Open the file for writing, or create a new one with the given name.
   /// The value of maxFileSizeKB is taken into account to possibly limit the file size.
   ///
   /// \pre File, if given, shall be a valid file name, the open procedure 
   /// has to be achievable by the operating system, or an exception is thrown.
   ///
   void Open(const MStdString& fileName, unsigned maxFileSizeKB = 0);

   /// Close the file, if it was open.
   ///
   virtual void Close() M_NO_THROW;

   /// Clear the contents of the file, if open, and start the log over from the beginning of the file.
   /// It is not an error to clear the file which is not open.
   ///
   /// \pre The file, if open, has to be open for writing, and the necessary write operations
   /// shall be successful, or the error is thrown.
   ///
   void Clear();

   /// Write the whole message to the log file.
   ///
   void WriteMessage(const char* data, size_t dataSize);

   /// Send the message with the prepared header.
   ///
   /// \pre Header shall be valid, length corresponds to the given data pointer.
   /// Also, any file write-related exception can be thrown.
   ///
   void WriteMessage(const MLogFile::PacketHeader& header, const char* data);

   /// Send the message with the specified code.
   ///
   /// \pre Code is valid and message has a valid format.
   /// The check is done in the debugging version that the code fits in range 0 to 0xFFFF.
   /// Also, any file write-related exception can be thrown.
   ///
   void WriteMessage(unsigned code, const char* message, unsigned length);

   /// Send several messages formatted as a buffer containing packets.
   ///
   /// \pre Messages have a valid format, no part of a message can be written
   /// with this service, only a number of complete messages. There is a debug check
   /// for a format error. Also, any file write-related exception can be thrown.
   ///
   void WriteMultipleMessages(const MByteString& messages);

private: // Services:

   // Initialize the data in the page so it appears cleared.
   //
   void DoInitNewPage() M_NO_THROW;

   // Write the chunk of data to the pages.
   //
   // \pre Any file write-related exception can be thrown.
   //
   void DoWriteBytes(const char* body, unsigned length);

   // Write m_page into the given index.
   //
   // \pre The read operation has to be successful, or a file exception takes place.
   // The index has to be within range, there is a debug check.
   //
   void DoWritePage(unsigned index, bool doNotNullifyUnusedSpace = false);

   // Called internally before writing the message to set the offset of message
   // which appears first in the page.
   //
   // \pre Called at the time the message is about to be put on the page.
   // No check is done.
   //
   void DoSetFirstMessageOffset()
   {
      if ( m_page.m_firstMessageOffset == (Muint32)-1 ) // if the page does not have a first message yet
         m_page.m_firstMessageOffset = Muint32(m_pageBodyPtr - m_page.m_body); // initialize it
   }

private: // Data members:

   // Maximum file size in kilobytes, or zero if the file size is not restrained, in which case
   // it will grow indefinitely.
   //
   unsigned m_maxFileSizeKB;

   // Maximum number of pages that correspond to the maximum file size
   //
   unsigned m_maxNumberOfPages;
};

#endif // !M_NO_MCOM_MONITOR

///@}
#endif
