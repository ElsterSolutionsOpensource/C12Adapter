// File MCORE/MStreamProcessor.cpp

#include "MCOREExtern.h"
#include "MStreamProcessor.h"
#include "MDes.h"

MStdString MStreamProcessor::GetName() const
{
   M_ASSERT(m_processor != NULL);
   return m_processor->GetName();
}

void MStreamProcessor::DoSetKeyImpl(const MByteString& key)
{
   M_ASSERT(m_processor != NULL);
   m_processor->DoSetKeyImpl(key);
}

void MStreamProcessor::DoCloseImpl()
{
   M_ASSERT(m_processor != NULL);
   m_processor->DoCloseImpl();
}

bool MStreamProcessor::DoIsOpenImpl() const
{
   M_ASSERT(m_processor != NULL);
   return m_processor->DoIsOpenImpl();
}

void MStreamProcessor::DoFlushImpl(bool softFlush)
{
   M_ASSERT(m_processor != NULL);
   m_processor->DoFlushImpl(softFlush);
}

unsigned MStreamProcessor::DoGetPosition() const
{
   M_ASSERT(m_processor != NULL);
   return m_processor->DoGetPosition();
}

void MStreamProcessor::DoSetPosition(unsigned pos)
{
   M_ASSERT(m_processor != NULL);
   m_processor->DoSetPosition(pos);
}

unsigned MStreamProcessor::DoGetSize() const
{
   M_ASSERT(m_processor != NULL);
   return m_processor->DoGetSize();
}

void MStreamProcessor::DoSetSize(unsigned length)
{
   M_ASSERT(m_processor != NULL);
   m_processor->DoSetSize(length);
}


#if (M_OS & M_OS_WINDOWS) != 0

MStreamProcessorText::~MStreamProcessorText() M_NO_THROW
{
}

   // Remove \r from given buffer, return the new count of buffer
   //
   inline unsigned DoProcessTextBuffer(char* buffer, unsigned count)
   {
      char* it = buffer;
      char* itOut = it;
      char* itEnd = it + count;
      for ( ; it < itEnd; ++it )
      {
         char c = *it;
         if ( c == '\r' )
            --count;
         else
            *itOut++ = c;
      }
      return count;
   }

unsigned MStreamProcessorText::DoReadAvailableBytesImpl(char* buffer, unsigned count)
{
   M_ASSERT(m_processor != NULL);

   char* buffPtr = buffer;
   unsigned outCount = 0u;
   for ( ;; )
   {
      unsigned bytesToRead = count - outCount;
      if ( bytesToRead == 0 )
         break;
      unsigned readLenImpl = m_processor->DoReadAvailableBytesImpl(buffPtr, bytesToRead);
      outCount += DoProcessTextBuffer(buffPtr, readLenImpl);
      if ( readLenImpl < bytesToRead )  // no more bytes are available in buffer
         break; // outCount will have a correct value - how many bytes were read
      buffPtr = buffer + outCount;
   }
   return outCount;
}

void MStreamProcessorText::DoWriteBytesImpl(const char* buffer, unsigned count)
{
   MByteString buff;

   M_ASSERT(m_processor != NULL);

   if ( count > 0 )
   {
      buff.reserve(count);
      const char* it = buffer;
      const char* itEnd = buffer + count;
      char c = *it;
      if ( c == '\n' ) // help the compiler optimize by handling the first element that will always require \r if it is \n
         buff += '\r';
      buff += c;
      for ( ; ; )
      {
         ++it;
         if ( it == itEnd )
            break;
         c = *it;
         if ( c == '\n' && *(it - 1) != '\r' ) // no need to check here if it - 1 is within buffer
            buff += '\r';
         buff += c;
      }
   }
   m_processor->DoWriteBytesImpl(buff.data(), M_64_CAST(unsigned, buff.size()));
}
#endif

unsigned MStreamProcessorBuffered::DoReadAvailableBytesImpl(char* buffer, unsigned count)
{
   M_ASSERT((m_flags & FlagReadOnly) != 0);
   m_lastOp = STREAMOP_READ;

   if ( !m_buffPresent )
      DoReadPage(m_pageInBuffer);
   M_ASSERT(m_buffPresent);

   char* buffPtr = buffer;
   unsigned outCount = 0u;
   unsigned remainingCount = count;
   do
   {
      unsigned bufferCount = m_buffEnd - m_buffCurr;
      if ( bufferCount > 0 ) // read from buffer first
      {
         if ( bufferCount > remainingCount ) // everything fits within already read buffer
            bufferCount = remainingCount;
         memcpy(buffPtr, m_buff.m_bytes + m_buffCurr, bufferCount);
         m_buffCurr += bufferCount;
         outCount += bufferCount;
         if ( outCount == count )  // done reading from the buffer entirely
            break;
         if ( m_buffEnd != m_pageDataSize ) // stream already ended
         {
            M_ASSERT(m_buffCurr == m_buffEnd);
            break;
         }
         buffPtr += bufferCount;
         remainingCount -= bufferCount;
      }
      if ( m_buffEnd != m_pageDataSize )
         break;
      // otherwise the next page might have more data
      DoReadPage(m_pageInBuffer + 1);
      m_buffCurr = 0;
   } while( m_buffEnd != 0 );
   return outCount;
}

void MStreamProcessorBuffered::DoReadPage(unsigned num)
{
   if ( m_buffChanged )
      DoWriteCurrentPage();

   m_buffEnd = 0; // Initialize to no data before reading
   if ( m_pageOfFile != num )
      MStreamProcessor::DoSetPosition(num * STREAM_BUFFER_SIZE + m_fileHeaderSize);

   unsigned size = DoReadPageAtCurrentFilePosition();
   m_buffEnd = size;
   if ( size == m_pageDataSize )
      m_pageOfFile = num + 1;
   else
      m_pageOfFile = UINT_MAX; // invalidate page info
   if ( m_buffCurr > size )
      m_buffCurr = size;

   m_pageInBuffer = num;
   m_buffPresent = true;
}

unsigned MStreamProcessorBuffered::DoReadPageAtCurrentFilePosition()
{
   return m_processor->DoReadAvailableBytesImpl(m_buff.m_bytes, STREAM_BUFFER_SIZE); // fill in the whole buffer
}

void MStreamProcessorBuffered::DoWriteBytesImpl(const char* buffer, unsigned count)
{
   M_ASSERT((m_flags & FlagWriteOnly) != 0);
   m_lastOp = STREAMOP_WRITE;

   if ( !m_buffPresent ) // buffer was never initialized
   {
      unsigned fileSize = DoGetSize();
      unsigned currPos = DoGetPosition();
      if ( (currPos % m_pageDataSize) == 0 )  // simple case of being on a page boundary (most often, this will be the beginning of empty file)
      {
         M_ASSERT(m_buffCurr == 0);
         if ( count < m_pageDataSize && currPos + count < fileSize ) // we only have to read page if we are writing less than the whole page
            DoReadPage(m_pageInBuffer);
         else
            m_buffPresent = true;
      }
      else
         DoReadPage(m_pageInBuffer);
      M_ASSERT(m_buffPresent);
   }

   const char* buffPtr = buffer;
   for ( ; ; )
   {
      unsigned bufferCount = m_pageDataSize - m_buffCurr;
      if ( bufferCount > 0 ) // write into buffer first
      {
         if ( bufferCount > count ) // everything fits within already read buffer
            bufferCount = count;
         m_buffChanged = true;
         memcpy(m_buff.m_bytes + m_buffCurr, buffPtr, bufferCount);
         m_buffCurr += bufferCount; // current moves along
         if ( m_buffEnd < m_buffCurr )
            m_buffEnd = m_buffCurr;
         count -= bufferCount;
         if ( count == 0 )  // done writing, buffer used
            return;
         buffPtr += bufferCount;
      }
      if ( m_buffChanged )
         DoWriteCurrentPage();

      unsigned fileSize = DoGetSize();

      ++ m_pageInBuffer;
      unsigned nextPageOffset = m_pageInBuffer * m_pageDataSize;
      if ( nextPageOffset + count < fileSize ) // have to read the page
         DoReadPage(m_pageInBuffer);
      else
      {
         // Silently initialize the next page, no read necessary
         m_buffEnd = 0;         // prepare new page
         m_buffChanged = false; // prepare new page
      }
      m_buffCurr = 0;           // prepare new page
   }
}

void MStreamProcessorBuffered::DoWriteCurrentPage()
{
   M_ASSERT((m_flags & FlagWriteOnly) != 0);
   if ( m_buffEnd != 0 )
   {
      if ( m_pageInBuffer != m_pageOfFile )
         MStreamProcessor::DoSetPosition(m_pageInBuffer * STREAM_BUFFER_SIZE + m_fileHeaderSize);
      DoWritePageAtCurrentFilePosition();
      m_buffChanged = false;
      if ( m_buffEnd == m_pageDataSize )
         m_pageOfFile = m_pageInBuffer + 1;
      else
         m_pageOfFile = UINT_MAX;
      DoGetSize(); // update cache of m_fileSize, if this is the last page
   }
}

void MStreamProcessorBuffered::DoWritePageAtCurrentFilePosition()
{
   M_ASSERT(m_buffEnd != 0); // we always have something to write in this method
   m_processor->DoWriteBytesImpl(m_buff.m_bytes, m_buffEnd);
}

void MStreamProcessorBuffered::DoFlushImpl(bool softFlush)
{
   m_lastOp = STREAMOP_WRITE;

   if ( m_buffChanged )
      DoWriteCurrentPage();
   m_processor->DoFlushImpl(softFlush);
}

unsigned MStreamProcessorBuffered::DoGetSize() const
{
   M_ASSERT(m_processor != NULL);

   if ( m_fileSize == UINT_MAX )
   {
      m_fileSize = MStreamProcessor::DoGetSize();
      if ( m_fileHeaderSize != 0 && m_fileSize >= m_fileHeaderSize )
         m_fileSize -= m_fileHeaderSize;
      if ( m_fileSize != 0 && m_pageDataSize != STREAM_BUFFER_SIZE ) // if we have to deal with page header
      {
         M_ENSURED_ASSERT(m_pageDataSize < STREAM_BUFFER_SIZE);
         const unsigned headerSize = STREAM_BUFFER_SIZE - m_pageDataSize;
         unsigned numPages = m_fileSize / STREAM_BUFFER_SIZE;
         unsigned lastPageSize = m_fileSize - numPages * STREAM_BUFFER_SIZE;
         if ( lastPageSize != 0 )
         {
            DoCheckEncryptedStreamFormat(lastPageSize > headerSize); // if there is a header, data cannot be zero
            ++numPages; // one extra page with the data remainder
         }
         m_fileSize -= numPages * headerSize; // extract headers
      }
   }
   unsigned currPageEndPosition = m_pageInBuffer * m_pageDataSize + m_buffEnd;
   if ( currPageEndPosition > m_fileSize )
      m_fileSize = currPageEndPosition; //case when there is a buffer not yet flushed
   return m_fileSize;
}

void MStreamProcessorBuffered::DoSetSize(unsigned size)
{
   M_ASSERT(m_pageDataSize == STREAM_BUFFER_SIZE); // otherwise it shall not be called
   M_ASSERT((m_flags & FlagWriteOnly) != 0);

   unsigned curentSize = DoGetSize();
   if ( size == curentSize )
      return; // done
   if ( size > curentSize )
   {
      MStream::DoThrowEndOfStream();
      M_ENSURED_ASSERT(0);
   }

   unsigned currentPosition = DoGetPosition();
   if ( m_buffPresent )
   {
      unsigned pageLast = size / m_pageDataSize;
      if ( pageLast == m_pageOfFile ) // special case when the very last page is loaded
      {
         unsigned newBuffEnd = size % m_pageDataSize;
         M_ASSERT(m_buffEnd > newBuffEnd); // if all the above checks are correct
         m_buffEnd = newBuffEnd;
         if ( m_buffCurr > m_buffEnd )
            m_buffCurr = m_buffEnd;
         m_buffChanged = true;
      }
      else
      {
         MStreamProcessor::DoSetSize(m_fileHeaderSize + size);
         if ( pageLast < m_pageOfFile ) // next special case - when we have to invalidate the buffer
         {
            m_buffPresent = false;
            DoSetPosition(size);
         }
      }
   }
   else
   {
      MStreamProcessor::DoSetSize(m_fileHeaderSize + size);
      if ( currentPosition > size )
         DoSetPosition(size);
   }
   m_fileSize = size;
}

unsigned MStreamProcessorBuffered::DoGetPosition() const
{
   return m_pageInBuffer * m_pageDataSize + m_buffCurr;
}

void MStreamProcessorBuffered::DoSetPosition(unsigned pos)
{
   unsigned size = DoGetSize();
   if ( size < pos )
   {
      MStream::DoThrowEndOfStream();
      M_ENSURED_ASSERT(0);
   }

   const unsigned newPageInBuffer = pos / m_pageDataSize; // have these two lines together
   const unsigned posInPage       = pos % m_pageDataSize; // so the compiler does modulo and divisor in one instruction
   if ( newPageInBuffer != m_pageInBuffer )
   {
      if ( m_buffChanged )
         DoWriteCurrentPage();
      m_pageInBuffer = newPageInBuffer;
      m_buffPresent = false;
   }
   m_buffCurr = posInPage;

#if M_DEBUG
   unsigned c = DoGetPosition();
   M_ASSERT(c == pos);
#endif
}

void MStreamProcessorBuffered::DoCheckEncryptedStreamFormat(bool trueCondition) const
{
   if ( !trueCondition )
   {
      MException::Throw(M_CODE_STR_P1(MErrorEnum::BadStreamFormat, M_I("Encrypted stream '%s' has bad format"), GetName().c_str()));
      M_ENSURED_ASSERT(0);
   }
}

