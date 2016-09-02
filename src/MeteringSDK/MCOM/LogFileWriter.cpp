// File MCOM/MLogFile.cpp

#include "MCOMExtern.h"
#include "MCOMExceptions.h"
#include "LogFileWriter.h"
#include "MonitorFile.h"

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING && !M_NO_FILESYSTEM

   const Muint32 NIL = (Muint32)-1;

MLogFileWriter::~MLogFileWriter() M_NO_THROW
{
   Close(); // this one never throws anything
}

void MLogFileWriter::Open(const MStdString& fileName, unsigned maxFileSizeKB)
{
   m_openWarnings.clear();
   Close();
   if ( fileName.empty() )
      return; // nothing more to do, by convention, this call closes the existing file, if was open

   SetMaxFileSizeKB(maxFileSizeKB);
   bool wasFinished = DoOpen(fileName, false);
   if ( m_numberOfPages != 0 )
   {
      if ( wasFinished ) // otherwise there is no need to write the page 0
      {
         DoReadPage(0u);
         m_page.m_lastPageIndex = NIL; // by this tell the file is in process of being written
         DoWritePage(0u, true); // nullify the last page field
      }
      DoReadPage(m_lastPageIndex);        // go to the end of the log
      m_pageCounter = m_page.m_pageCounter;
      m_pageBodyPtr = m_page.m_body + m_page.m_firstMessageOffset;
      char* bodyEnd = m_page.m_body + sizeof(m_page.m_body) - PACKET_HEADER_SIZE; // last packet on the page

      // Now find the last message on this page.
      PacketHeader header;
      for ( ;; )
      {
         memcpy(&header, m_pageBodyPtr, PACKET_HEADER_SIZE);
         if ( header.m_length == 0 ) // end of the file
            break;
         char* nextPageBodyPtr = m_pageBodyPtr + header.m_length;
         if ( nextPageBodyPtr > bodyEnd ) // possibly, the file was corrupt, not finished; or simply the end packet is on the next page
            break;
         m_pageBodyPtr = nextPageBodyPtr;
      }
   }
   else // zero size, new file
   {
      M_ASSERT(m_numberOfPages == 0);
      M_ASSERT(m_lastPageIndex == 0);
      m_currentPageIndex = 0;
      m_pageCounter = 0;
      DoInitNewPage();
   }
}

void MLogFileWriter::Close() M_NO_THROW
{
   if ( IsOpen() )
   {
      try  // These actions constitute a successful close of a file
      {
         // Write end-of-file actually, end-of-packets
         //
         static const PacketHeader s_zeroPacketHeader;
         unsigned last = m_currentPageIndex; // save current index in a temporary, the tail can be on another page
         DoSetFirstMessageOffset();
         DoWriteBytes((const char*)&s_zeroPacketHeader, PACKET_HEADER_SIZE);
         DoWritePage(m_currentPageIndex);
         DoReadPage(0u);
         m_page.m_lastPageIndex = last;
         DoWritePage(0u, true);
      }
      catch ( ... ) // avoid exception flown from Close
      {
      }
      MLogFile::Close(); // this actually closes the file
   }
}

void MLogFileWriter::SetMaxFileSizeKB(unsigned size)
{
   if ( size == 0 )
      m_maxNumberOfPages = NUMBER_OF_PAGES_LIMIT;
   else
   {
      MENumberOutOfRange::CheckInteger(64, 0x7FFF, int(size), M_OPT_STR("MaxFileSizeKB"));
      m_maxNumberOfPages = size * 1024 / PAGE_TOTAL_SIZE - 1;
   }
   m_maxFileSizeKB = size; // the range check is done already above, do the assignment and that's all
}

void MLogFileWriter::WriteMessage(const char* data, size_t dataSize)
{
   DoSetFirstMessageOffset();
   DoWriteBytes(data, static_cast<unsigned>(dataSize));
}

void MLogFileWriter::WriteMessage(const MLogFile::PacketHeader& header, const char* data)
{
   DoSetFirstMessageOffset();
   DoWriteBytes((const char*)&header, PACKET_HEADER_SIZE);
   unsigned dataSize = header.GetPacketBodyLength();
   if ( dataSize != 0 )
      DoWriteBytes(data, dataSize);
}

void MLogFileWriter::WriteMessage(unsigned code, const char* data, unsigned length)
{
   DoSetFirstMessageOffset();
   PacketHeader header(Muint32(length), code);
   DoWriteBytes((const char*)&header, PACKET_HEADER_SIZE);
   if ( length != 0 )
      DoWriteBytes(data, length);
}

void MLogFileWriter::WriteMultipleMessages(const MByteString& messagesBuffer)
{
   const char* data = messagesBuffer.data();
   unsigned size = M_64_CAST(unsigned, messagesBuffer.size());
   unsigned remainingLen = unsigned(m_page.m_body + sizeof(m_page.m_body) - m_pageBodyPtr);
   if ( remainingLen >= size ) // a lot faster way is if the whole message fits within the remaining page
   {
      #if M_DEBUG   // check for validity of the buffer, it has to contain complete messages only
         {
            const PacketHeader* packet = (PacketHeader*)data;
            const PacketHeader* lastPacket = (PacketHeader*)(data + size);
            while ( packet < lastPacket )
            {
               PacketHeader hdr;
               memcpy(&hdr, packet, PACKET_HEADER_SIZE); // fix memory alignment on ARM
               packet = (PacketHeader*)((const char*)packet + hdr.m_length);
            }

            // This is the whole contract -- the packet shall match, not just be bigger
            // If this assert fails, the user attempted to place incomplete packets with WriteMultipleMessages
            //
            M_ASSERT(packet == lastPacket);
         }
      #endif

      DoSetFirstMessageOffset();
      memcpy(m_pageBodyPtr, data, size);
      m_pageBodyPtr += size;
   }
   else // we have to walk this way due to the necessity to set the start for the message
   {
      const PacketHeader* packet = (PacketHeader*)data;
      const PacketHeader* lastPacket = (PacketHeader*)(data + size);
      while ( packet < lastPacket )
      {
         DoSetFirstMessageOffset();
         PacketHeader hdr;
         memcpy(&hdr, packet, PACKET_HEADER_SIZE); // fix memory alignment on ARM
         DoWriteBytes((const char*)packet, hdr.m_length);
         packet = (PacketHeader*)((const char*)packet + hdr.m_length);
      }
   }
}

void MLogFileWriter::DoWriteBytes(const char* buff, unsigned length)
{
   M_ASSERT(length > 0);
   const char* const pageEnd = m_page.m_body + sizeof(m_page.m_body);
   M_ASSERT(m_pageBodyPtr >= m_page.m_body && m_pageBodyPtr <= pageEnd);
   for ( ;; )
   {
      unsigned remainingLen = unsigned(pageEnd - m_pageBodyPtr);
      if ( remainingLen >= length ) // if the remaining buffer will fit in the current page
         break;
      if ( remainingLen > 0 )
      {
         memcpy(m_pageBodyPtr, buff, remainingLen);
         m_pageBodyPtr += remainingLen;
         buff += remainingLen;
         length -= remainingLen;
      }
      DoWritePage(m_currentPageIndex);
      if ( m_currentPageIndex >= m_maxNumberOfPages ) // we have to start from the zero page
         m_currentPageIndex = 0;
      else
         ++m_currentPageIndex;
      m_lastPageIndex = m_currentPageIndex;
      DoInitNewPage();
      // notify listener
      if ( m_listener != NULL )
         m_listener->OnPageBoundHit();
      M_ASSERT(m_pageBodyPtr == m_page.m_body);
   }
   memcpy(m_pageBodyPtr, buff, length);
   m_pageBodyPtr += length;
}

void MLogFileWriter::DoInitNewPage() M_NO_THROW
{
   M_ASSERT(IsOpen());
   ++m_numberOfPages;
   m_pageBodyPtr = (char*)m_page.m_body;
   m_page.m_signature = m_obfuscate ? PAGE_OBFUSCATED_HEADER_SIGNATURE : PAGE_HEADER_SIGNATURE;
   m_page.m_lastPageIndex = NIL;
   m_page.m_firstMessageOffset = NIL; // by default, no valid messages exist in the page
  // m_page.m_pageCounter will be initialized on page write
}

void MLogFileWriter::DoWritePage(unsigned index, bool doNotNullifyUnusedSpace)
{
   M_ASSERT(IsOpen());
   M_ASSERT(m_page.m_signature == (unsigned)PAGE_HEADER_SIGNATURE || m_page.m_signature == (unsigned)PAGE_OBFUSCATED_HEADER_SIGNATURE);
   M_ASSERT(index <= m_numberOfPages);

   if ( !doNotNullifyUnusedSpace )
   {
      int diff = int((const char*)m_page.m_body + sizeof(m_page.m_body) - m_pageBodyPtr);
      if ( diff > 0 )
         memset(m_pageBodyPtr, 0, diff); // nullify the rest of the page
   }

   m_page.m_pageCounter = ++m_pageCounter;
   m_page.OnceBeforeWrite();

   m_file.SetPosition(long(index * PAGE_TOTAL_SIZE));
   m_file.WriteBytes((char*)&m_page, PAGE_TOTAL_SIZE);
}

#endif // !M_NO_MCOM_MONITOR
