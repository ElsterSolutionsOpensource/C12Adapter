// File MCOM/MLogFile.cpp

#include "MCOMExtern.h"
#include "MCOMExceptions.h"
#include "LogFile.h"
#include "MonitorFile.h"

#if !M_NO_MCOM_MONITOR && !M_NO_MULTITHREADING && !M_NO_FILESYSTEM

   const Muint32 NIL = (Muint32)-1;

   M_COMPILED_ASSERT(sizeof(MLogFile::LogFilePage) == MLogFile::PAGE_TOTAL_SIZE); // check the alignment, constant

void MLogFile::LogFilePage::OnceBeforeWrite()
{
   Muint32 checksum = m_signature;
   Muint32* body = &m_lastPageIndex; // start from the first byte
   Muint32* bodyEnd = &m_checksum; // checksum is not included
   if ( m_signature == (Muint32)MLogFile::PAGE_HEADER_SIGNATURE )
   {
      for ( ; body != bodyEnd; ++body )
         checksum += *body;
   }
   else
   {
      M_ASSERT(m_signature == (Muint32)MLogFile::PAGE_OBFUSCATED_HEADER_SIGNATURE);
      for ( ; body != bodyEnd; ++body )
      {
         *body ^= MLogFile::PAGE_OBFUSCATED_HEADER_SIGNATURE;
         checksum += *body;
      }
   }
   m_checksum = checksum;
}

bool MLogFile::LogFilePage::OnceAfterRead()
{
   Muint32 checksum = m_signature;
   Muint32* body = &m_lastPageIndex; // start from the first byte
   Muint32* bodyEnd = &m_checksum; // checksum is not included
   if ( m_signature == (Muint32)MLogFile::PAGE_HEADER_SIGNATURE )
   {
      for ( ; body != bodyEnd; ++body )
         checksum += *body;
   }
   else if ( m_signature == (Muint32)MLogFile::PAGE_OBFUSCATED_HEADER_SIGNATURE )
   {
      for ( ; body != bodyEnd; ++body )
      {
         checksum += *body;
         *body ^= MLogFile::PAGE_OBFUSCATED_HEADER_SIGNATURE;
      }
   }
   else
      return false;

   if ( !(m_firstMessageOffset == NIL || m_firstMessageOffset < PAGE_BODY_SIZE) )
      return false;
   return m_checksum == checksum;
}

MLogFile::MLogFile()
:
   m_file(),
   m_fileName(),
   m_openWarnings(),
   m_pageCounter(),
   m_numberOfPages(0),
   m_lastPageIndex(0),
   m_currentPageIndex(0),
   m_pageBodyPtr(NULL),
   m_page(),
   m_listener(NULL),
   m_obfuscate(false)
{
}

MLogFile::~MLogFile() M_NO_THROW
{
   Close(); // It is always safe to call Close. This particular call will close the file of the reader
}

   inline bool DoCheckIfText(const char* buff, unsigned size)
   {
      const char* buffEnd = buff + size;
      for ( ; buff != buffEnd; ++buff )
      {
         char c = *buff;
         if ( iscntrl(c) && c != '\n' && c != '\r' && c != '\t' )
            return false;
      }
      return true;
   }

bool MLogFile::DoOpen(const MStdString& fileName, bool readonly)
{
   M_ASSERT(!IsOpen());
   M_ASSERT(!fileName.empty());

   m_openWarnings.clear();
   m_pageCounter = 0;
   m_numberOfPages = 0;
   m_currentPageIndex = 0;
   m_lastPageIndex = 0;
   m_pageBodyPtr = NULL;

   m_fileName = MUtilities::GetFullPath(fileName); // this will be an empty string on failure
   if ( m_fileName.empty() )
      m_fileName = fileName; // recover the file name, if the attempt to get the full path failed

   int flags = readonly ? MStreamFile::FlagReadOnly : (MStreamFile::FlagCreate | MStreamFile::FlagReadWrite);
   m_file.Open(m_fileName, flags, MStreamFile::SharingAllowRead);

   long fileSize = m_file.GetSize();
   m_numberOfPages = static_cast<unsigned>(fileSize / PAGE_TOTAL_SIZE); // can be zero, if the file is new
   if ( fileSize == 0 ) // if the file is empty
   {
      m_lastPageIndex = 0u;
      return true;
   }

   // Otherwise the file is not empty
   //
   try
   {
      DoReadPage(0u);
   }
   catch ( MException& ex )
   {
      if ( ex.GetCode() == MErrorEnum::BadFileFormat )
      {
         unsigned readSize = m_file.GetPosition(); // this is how many bytes were read above in DoReadPage
         if ( readSize > 0 )
         {
            if ( readSize > PAGE_TOTAL_SIZE )
            {
               M_ASSERT(0); // something is going on really unexpected
               readSize = PAGE_TOTAL_SIZE;
            }
            if ( DoCheckIfText(reinterpret_cast<const char*>(&m_page), readSize) )
               ex.SetMessageString(MGetStdString(M_I("File '%s' is likely a text file, cannot open it as binary log"), fileName.c_str()));
         }
      }
      throw;
   }

   if ( m_page.m_lastPageIndex != NIL ) // if the file was closed normally when written
   {
      m_lastPageIndex = m_page.m_lastPageIndex;
      return true;
   }

   // The file was not closed normally when written, search for the place when the page number stops to grow.
   //
   m_openWarnings = "File was not properly closed when last written"; // this text shall not be localized
   m_pageCounter = m_page.m_pageCounter;
   for ( unsigned i = 1; i < m_numberOfPages; ++i )
   {
      DoReadPage(i);
      ++m_pageCounter;
      if ( m_pageCounter != m_page.m_pageCounter ) // if the proper ascending sequence of pages is interrupted...
      {
         --m_pageCounter; // then the previous page was what we are searching for (it is never the zero page)
         m_lastPageIndex = i - 1;
         return false;
      }
   }

   // Either the file was written with all pages ascending, or simpler, there is only one page in the file
   //
   m_lastPageIndex = m_numberOfPages - 1; // in case the file is contiguously written this will work
   return false;
}

void MLogFile::Close() M_NO_THROW
{
   m_file.Close();
}

void MLogFile::DoReadPage(unsigned index)
{
   M_ASSERT(IsOpen());
   M_ASSERT(index <= m_numberOfPages); // index can be equal to m_numberOfPages if the file is of zero size, in which case we throw an IO exception

   m_file.SetPosition(long(index * PAGE_TOTAL_SIZE));
   unsigned readSize = m_file.ReadAvailableBytes((char*)&m_page, PAGE_TOTAL_SIZE);
   if ( readSize != PAGE_TOTAL_SIZE || !m_page.OnceAfterRead() )
   {
      MException::ThrowBadFileFormat(m_fileName);
      M_ENSURED_ASSERT(0);
   }

   m_obfuscate = (m_page.m_signature == (unsigned)PAGE_OBFUSCATED_HEADER_SIGNATURE);
   m_currentPageIndex = index;
   m_pageBodyPtr = (char*)m_page.m_body;
}

#endif // !M_NO_MCOM_MONITOR
