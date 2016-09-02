#ifndef MCOM_LOGFILE_H
#define MCOM_LOGFILE_H
/// \addtogroup MCOM
///@{
/// \file MCOM/LogFile.h

#include <MCOM/MCOMDefs.h>

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING && !M_NO_FILESYSTEM

/// Abstract log file utility class to handle the log from the monitor.
/// The concrete instances of this class will provide facilities
/// for reading and writing the log files.
///
class MCOM_ABSTRACT_CLASS MLogFile
{
public: // Constants:

   enum
   {
      PAGE_HEADER_SIGNATURE         = 0xA2EBBAED,    ///< Monitor file header and page header signature (also tells about version).
      PAGE_OBFUSCATED_HEADER_SIGNATURE = 0xA2EBBAEC, ///< Monitor file header and page header signature that tells the page contents are obfuscated (also tells about version).
      PAGE_TOTAL_SIZE               = 0x1000,  ///< Total page size, shall be efficient for most architectures.
      PAGE_HEADER_SIZE              = 16,      ///< Page header size.
      PAGE_FOOTER_SIZE              = 4,       ///< Page footer size.
      PAGE_BODY_SIZE                = PAGE_TOTAL_SIZE - PAGE_HEADER_SIZE - PAGE_FOOTER_SIZE, ///< Body size of the page.
      PACKET_HEADER_SIZE            = 10,      ///< Size of the packet header.
      NUMBER_OF_PAGES_LIMIT         = 0xFFFF   ///< Still limit the number of pages in the file with no size limit, the file size is about 100 megabytes.
   };

public: // Types:

   /// Page format of the monitor file.
   ///
   /// Unused message body is filled with zeros.
   ///
   struct LogFilePage
   {
   public: // Data:

      /// Page header signature, equal to constant FILE_HEADER_SIGNATURE.
      /// This is part of the page header.
      ///
      Muint32 m_signature;

      /// Last finished page index, if the file is released.
      /// If this is not the first page, or if the file was not closed normally, it will be 0xFFFFFFFF.
      /// This is part of the page header.
      ///
      Muint32 m_lastPageIndex;

      /// Page counter. Starts at zero, for every successfully written page this counter is incremented.
      /// A proper file will have a range of page counters.
      ///
      Muint32 m_pageCounter;

      /// First consistent message offset on this page.
      /// If the page has a middle of a message, or a tail of the last one, this property will equal to 0xFFFFFFFF.
      /// Correspondingly, if the page starts with the message, this is zero.
      /// This is part of the page header.
      ///
      Muint32 m_firstMessageOffset;

      /// Body, stream of messages.
      /// Messages follow one after another, byte aligned.
      /// Each message starts with the message code, then the timestamp, then the size, and then the body.
      /// Unused message body is filled with zeros (there is no message code zero).
      ///
      char m_body [ PAGE_BODY_SIZE ];

      /// Checksum, sum of all quadruples of bytes in the page excluding the checksum itself.
      /// Checksum is used to determine that the page has been written fully and successfully.
      /// This is part of the page footer.
      ///
      Muint32 m_checksum;

   public: // Services:

      /// Update packet so it is ready to be written.
      /// Possibly obfuscate and calculate and update checksum.
      ///
     void OnceBeforeWrite();

     /// Verify packet integrity and prepare it for handling.
     ///
     bool OnceAfterRead();
   };

   /// Packet header structure.
   /// The body of the packet has dummy size, and the body follows the header.
   ///
   struct PacketHeader
   {
   public: // Data:

      /// Total number of bytes in the packet, header and message included.
      ///
      Muint32 m_length;

      /// Time stamp of the message in milliseconds.
      ///
      Muint32 m_timeStamp;

      /// Message code, as defined by MMonitor::MessageType, but stored in two bytes.
      ///
      Muint16 m_code;

   public: // Constructors:

      /// Default constructor that initializes fields to zeros.
      ///
      PacketHeader()
      :
         m_length(0u),
         m_timeStamp(0u),
         m_code(0u)
      {
      }

      /// Constructor that initializes the header with the length, code, and timestamp gotten from the
      /// system tick clock.
      ///
      /// \pre The code fits into a short word, there is a debug check.
      ///
      PacketHeader(unsigned length, unsigned code)
      :
         m_length(Muint32(length + PACKET_HEADER_SIZE)),
         m_timeStamp(Muint32(MUtilities::GetTickCount())),
         m_code(Muint16(code))
      {
         M_ASSERT(code <= 0xFFFF);
      }

      /// Get the length of the packet body.
      ///
      unsigned GetPacketBodyLength() const
      {
         M_ASSERT(m_length >= PACKET_HEADER_SIZE);
         return m_length - PACKET_HEADER_SIZE;
      }
   };

protected: // Constructors:

   /// Constructor that creates an uninitialized log file object.
   ///
   MLogFile();

public: // Destructor:

   /// Object destructor, close the log file.
   ///
   virtual ~MLogFile() M_NO_THROW;

public: // Services (property getters and setters):

   /// Tells if the file is open. Normally the error is thrown at the constructor if there is a problem
   /// with the file at opening, however if the problem appears later on use, the object will exist,
   /// throw exceptions on attempts to operate the file, and IsOpen will become false.
   ///
   bool IsOpen() const
   {
      return m_file.IsOpen();
   }

   /// Get the file name, as set for logging.
   /// If the file exists, the full file name will be returned,
   /// made from the user file name by the operating system.
   ///
   const MStdString& GetFileName() const
   {
      return m_fileName;
   }

   /// Get the warning message which might arise during opening a file and checking its contents.
   /// It will return an empty string if there were no warnings during opening.
   /// The warnings are kept until a new open is called.
   ///
   const MStdString& GetOpenWarnings() const
   {
      return m_openWarnings;
   }

   /// Sets the listener object to start handling events. Note that new pointer replaces previous one without
   /// destroying it.
   ///
   void SetListener(MMonitorFile* listener)
   {
      m_listener = listener;
   }

   /// @{
   /// Whether to obfuscate the monitor file so it does not clearly present text chunks of data.
   /// This option is not any sort of secure encryption.
   ///
   bool GetObfuscate() const
   {
      return m_obfuscate;
   }
   void SetObfuscate(bool yes)
   {
      m_obfuscate = yes;
   }
   /// @}

public: // Services:

   /// Close the file, if it was open.
   ///
   virtual void Close() M_NO_THROW;

protected: // Services:
/// \cond SHOW_INTERNAL

   // Open a file, protected service.
   // If readonly is true, a new file will not be opened if it does not exist.
   // If readonly is false, a new file is created if there was no such file.
   // Service returns true if the file was closed successfully at the previous
   // write sequence, or if it is false, it means the file was not closed,
   // and the application which was writing it was most likely crashing or hanging.
   //
   // \pre File, if given, shall be a valid file name, the open procedure
   // has to be achievable by the operating system, or an exception is thrown.
   //
   bool DoOpen(const MStdString& fileName, bool readonly);

   // Read m_page with the index into the current page.
   //
   // \pre The read operation has to be successful, or a file exception takes place.
   // The index has to be within range, there is a debug check.
   //
   void DoReadPage(unsigned index);

protected: // Data members:

   // File handle.
   //
   MStreamFile m_file;

   // File or directory name where the logging needs to be done.
   // This parameter is specified by the user.
   // If the directory is specified, the file name will be constructed from the attach string.
   //
   MStdString m_fileName;

   // String that has a collection of warnings at opening of the file.
   //
   MStdString m_openWarnings;

   // Counter which is kept unique for every page in the file.
   // It will only grow for every new page written to a file, much like autoincrement database key.
   // In case of a file read, this is kept as information only, basically how many pages have
   // ever been written to a file.
   //
   unsigned m_pageCounter;

   // Current number of pages in an open file.
   //
   unsigned m_numberOfPages;

   // Index of the page written to the file last.
   // Next to this page in the circular buffer will be the one written to the file earliest.
   //
   unsigned m_lastPageIndex;

   // Current page index.
   //
   unsigned m_currentPageIndex;

   // Current pointer to the body in the current page.
   // If we are writing, this is the current packet is the next place to write into.
   //
   char* m_pageBodyPtr;

   // Current page in the file, one that is used during processing
   //
   LogFilePage m_page;

   MMonitorFile* m_listener;

   bool m_obfuscate;

/// \endcond SHOW_INTERNAL
};

#endif // !M_NO_MCOM_MONITOR
///@}
#endif
