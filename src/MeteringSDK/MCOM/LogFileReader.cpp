// File MCOM/MLogFile.cpp

#include "MCOMExtern.h"
#include "MCOMExceptions.h"
#include "LogFileReader.h"

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING && !M_NO_FILESYSTEM

   const Muint32 NIL = (Muint32)-1;

MLogFileReader::~MLogFileReader() M_NO_THROW
{
}

void MLogFileReader::Open(const MStdString& fileName)
{
   m_firstPosition = 0;
   Close();
   DoOpen(fileName, true); // return value is ignored, not used: as the file is read-only we cannot repair it
   for ( m_currentPageIndex = m_lastPageIndex + 1; ; ++m_currentPageIndex )
   {
      if ( m_currentPageIndex >= m_numberOfPages )
         m_currentPageIndex = 0u;
      DoReadPage(m_currentPageIndex);
      if ( m_page.m_firstMessageOffset != NIL ) // if there was a message on this page
         break;
      if ( m_currentPageIndex == m_lastPageIndex ) // we have looped through all pages and no message start...
      {
         MException::ThrowBadFileFormat(m_fileName);
         M_ENSURED_ASSERT(0);
      }
   }
   m_pageBodyPtr = m_page.m_body + m_page.m_firstMessageOffset;
   m_position = m_firstPosition = DoGetPosition();
}

const MLogFile::PacketHeader& MLogFileReader::ReadPacketHeader()
{
   m_position = DoGetPosition();
   DoReadBytes((char*)&m_header, PACKET_HEADER_SIZE); 
   return m_header;
}

void MLogFileReader::ReadPacketBody(char* buffer)
{
   M_ASSERT(m_header.m_length > 0);
   unsigned len = GetPacketBodyLength();
   if ( len > 0 )
      DoReadBytes(buffer, GetPacketBodyLength()); // read the body, okay if the body is zero bytes
}

void MLogFileReader::SkipPacketBody()
{
   M_ASSERT(m_header.m_length > 0);
   unsigned len = GetPacketBodyLength();
   if ( len > 0 )
      DoReadBytes(NULL, GetPacketBodyLength()); // read the header
}

void MLogFileReader::SetPosition(PositionType ptr)
{
   unsigned pageIndex = (ptr & POSITION_PAGE_MASK);
   if ( pageIndex != m_currentPageIndex )
   {
      DoReadPage(pageIndex);
      M_ASSERT(pageIndex == m_currentPageIndex);
   }
   m_pageBodyPtr = (char*)&m_page + unsigned(ptr >> POSITION_OFFSET_SHIFT);
}

void MLogFileReader::DoReadBytes(char* buff, unsigned length)
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
         if ( buff != NULL ) // if it is not skipping the bytes
         {
            memcpy(buff, m_pageBodyPtr, remainingLen);
            buff += remainingLen;
         }
         m_pageBodyPtr += remainingLen;
         length -= remainingLen;
      }
      unsigned nextIndex = m_currentPageIndex + 1;
      if ( nextIndex == m_numberOfPages ) // we have to start from the zero page
         nextIndex = 0;
      DoReadPage(nextIndex);
      M_ASSERT(m_pageBodyPtr == m_page.m_body);
   }
   if ( buff != NULL ) // if it is not skipping the bytes
      memcpy(buff, m_pageBodyPtr, length);
   m_pageBodyPtr += length;
}

#endif // !M_NO_MCOM_MONITOR
