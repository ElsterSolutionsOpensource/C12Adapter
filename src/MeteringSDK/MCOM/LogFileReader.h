#ifndef MCOM_LOGFILEREADER_H
#define MCOM_LOGFILEREADER_H
/// \addtogroup MCOM
///@{
/// \file MCOM/LogFileReader.h

#include <MCOM/LogFile.h>

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING && !M_NO_FILESYSTEM

/// Log file utility class to handle the log from the monitor.
/// The concrete instances of this class will provide facilities 
/// for reading and writing the log files.
///
/// Reader can walk through the file sequentially. Also, it is possible to save and restore
/// the current position within a file. Position is bound to a concrete file and it will not be
/// valid if the file changed. For that, it is recommended to use positions only for a currently 
/// opened file.
///
/// Typical way how the file is open and read is this:
///
/// \code
///     MLogFileReader::PositionType ptr0 = NULL;
///     // Traversing through the file
///     MLogFileReader reader;
///     for ( ; !reader.EndOfFile(), reader.Next() )
///     {
///         const MLogFile::PacketHeader& header = reader.GetPacketHeader(); // getting header
///         char* buffer = new char [ reader.GetPacketBodyLength() ];
///         reader.GetPacketBody(buffer);
///         ... using length and body here ...
///         if ( ptr0 == NULL )
///            ptr0 = reader.GetPosition(); // getting the pointer (example)
///     }
///     // Using pointer to set the current position for the reader:
///     reader.SetPosition(ptr0);
///     const MLogFile::PacketHeader& header = reader.GetPacketHeader(); // getting header
///     char* buffer = new char [ header.m_length ];
///     reader.GetPacketBody(buffer);
/// \endcode
///
class MCOM_CLASS MLogFileReader : public MLogFile
{
public: // Types:

   /// Type used to denote the reader position.
   /// It allows storing and restoring the position within the file.
   /// Position pointer of NULL will point to nowhere.
   ///
   typedef Muint32
      PositionType;

private: // Constants:

   enum
   {
      POSITION_PAGE_MASK    = 0xFFFF, ///< Page mask within the position.
      POSITION_OFFSET_SHIFT = 16      ///< Shift for getting the offset within the page.
   };

public: // Constructor and destructor:

   /// Constructor that creates an uninitialized log file object.
   ///
   MLogFileReader()
   :
      MLogFile()
      // no need to initialize m_position or m_header
   {
   }

   /// Constructor that creates an existing log file with the given file name.
   ///
   /// \pre File, if given, shall be a valid file name, or an exception is thrown.
   /// Write access to a file is attempted. If anything fails there is an exception.
   ///
   explicit MLogFileReader(const MStdString& fileName)
   :
      MLogFile()
      // no need to initialize m_position or m_header
   {
      Open(fileName);
   }

   /// Object destructor, close the log file.
   ///
   virtual ~MLogFileReader() M_NO_THROW;

public: // Services:

   /// Open an existing file to read.
   ///
   /// \pre The file shall be a valid file name, the open procedure 
   /// has to be achievable by the operating system, or an exception is thrown.
   ///
   void Open(const MStdString& fileName);

   /// Reset the current pointer, one that walks through messages.
   /// Position points to the first item in a file.
   ///
   /// \pre The file has to be successfully open, there is an assertion.
   /// Also, file read error can be raised.
   ///
   void Reset()
   {
      SetPosition(m_firstPosition);
      m_header.m_length = Muint32(-1); // this is to prevent end of file condition
   }

   /// Tells if the current position is the End of the File.
   /// End of the file position pointer will be a NULL pointer.
   ///
   /// \pre The file has to be open, there is an assertion.
   ///
   bool EndOfFile() const
   {
      return m_header.m_length == 0; // length is always no less than 10
   }

   /// Read the packet header of the current packet.
   ///
   /// \pre It shall not be an end of file condition, or there is an assertion.
   ///
   const MLogFile::PacketHeader& ReadPacketHeader();

   /// Get the length of the packet body after the reader was read successfully.
   ///
   /// \pre It shall not be an end of file condition, or there is an assertion.
   ///
   unsigned GetPacketBodyLength() const
   {
      M_ASSERT(!EndOfFile());
      M_ASSERT(m_header.m_length >= PACKET_HEADER_SIZE);
      return m_header.m_length - PACKET_HEADER_SIZE;
   }

   /// Fill the given buffer with the body characters of the packet and advance
   /// the file pointer to the next packet.
   ///
   /// \pre The buffer shall be at least of the size denoted by 
   /// GetPacketBodyLength(), or the behavior is undefined.
   /// It shall not be an end of file condition, or there is an assertion.
   ///
   void ReadPacketBody(char* buffer);

   /// Skip the body of the packet and advance to the next packet.
   ///
   /// \pre It shall not be an end of file condition, 
   /// or there is an assertion. Also, any file-related exception can be thrown.
   ///
   void SkipPacketBody();

   /// Get the position of the current packet within an opened file.
   /// Position is like pointer, except that it points to a place within an opened log file.
   /// It is guaranteed that the position will be of a size the same as the size 
   /// of a pointer to any type (like void). Also, NULL pointer will point to no object, 
   /// and referencing it its position will lead to EndOfFile condition.
   ///
   PositionType GetPosition() const
   {
      return m_position;
   }
   
   /// Get the position of the current packet within an opened file.
   /// Position is like pointer, except that it points to a place within an opened log file.
   /// It is guaranteed that the position will be of a size the same as the size 
   /// of a pointer to any type (like void). Also, NULL pointer will point to no object, 
   /// and referencing it its position will lead to EndOfFile condition.
   ///
   void SetPosition(PositionType ptr);

private: // Services:

   // Read the bytes from the current position within the page into the buffer.
   // Advance to the next page if necessary. In case buffer equals NULL,
   // the bytes are skipped without copying.
   //
   // \pre Called internally at the proper time, when the buffer is reset
   // and there was no end of pages condition.
   //
   void DoReadBytes(char* buffer, unsigned length);

   // Get the exact current position within the file.
   // This is private, as it can be a position in the middle of the message.
   //
   PositionType DoGetPosition() const
   {
      M_ASSERT(m_currentPageIndex <= NUMBER_OF_PAGES_LIMIT); // this is the precondition in the writer
      Muint32 posOnPage = static_cast<Muint32>((const char*)m_pageBodyPtr - (const char*)&m_page);
      return (posOnPage << POSITION_OFFSET_SHIFT) | Muint32(m_currentPageIndex);
   }

private: // Services:

   // Current packet header
   //
   PacketHeader m_header;

   // Position of the above current header
   //
   mutable PositionType m_position;

   // First position within the file, used to reset the contents
   //
   PositionType m_firstPosition;
};

#endif // !M_NO_MCOM_MONITOR
///@}
#endif
