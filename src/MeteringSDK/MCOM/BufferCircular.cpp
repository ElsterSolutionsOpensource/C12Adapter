// File MCOM/BufferCircular.cpp

#include "MCOMExtern.h"
#include "BufferCircular.h"

MBufferCircular::MBufferCircular(unsigned initialCapacity)
:
   m_bufferSize(initialCapacity),
   m_getPosition(0u),
   m_putPosition(0u)
{
   M_ASSERT(initialCapacity > 1); // otherwise we cannot put even a single byte in
   m_buffer = M_NEW char [ initialCapacity ];
}

MBufferCircular::~MBufferCircular() M_NO_THROW
{
   delete [] m_buffer;
}

void MBufferCircular::Resize(unsigned newCapacity)
{
   unsigned size = GetSize(); // save this in a temp variable since positions will be changing
   M_ASSERT(newCapacity > 0 && newCapacity > size);

   char* newBuffer = M_NEW char [ newCapacity ];
   int diff = static_cast<int>(m_putPosition - m_getPosition);
   if ( diff >= 0 ) // one chunk or empty
      memcpy(newBuffer, m_buffer + m_getPosition, diff);
   else  // two separate chunks
   {
      unsigned sizeToTheEnd = m_bufferSize - m_getPosition;
      memcpy(newBuffer, m_buffer + m_getPosition, sizeToTheEnd);
      memcpy(newBuffer + sizeToTheEnd, m_buffer, m_putPosition);
   }
   m_getPosition = 0;
   m_putPosition = size;
   delete [] m_buffer;
   m_buffer = newBuffer;
   m_bufferSize = newCapacity;
   M_ASSERT(size == GetSize());         // invariant stays
   M_ASSERT(CanPutWithoutResize() > 0); // otherwise the precondition assert would fail
}

void MBufferCircular::Put(const char* buff, unsigned size)
{
   unsigned remainingBytes = CanPutWithoutResize();
   if ( remainingBytes < size )
   {
      if ( m_bufferSize < size ) // an unexpectedly huge buffer is requested, be savvy
         Resize(m_bufferSize + size);
      else
         Resize(m_bufferSize * 2);
      M_ASSERT(CanPutWithoutResize() >= size); // should fit now
   }

   // Now the buffer will fit, guaranteed
   if ( m_putPosition < m_getPosition ) // one chunk, easy go
   {
      M_ASSERT(m_getPosition - m_putPosition > size); // We know that there is enough room allocated above
      memcpy(m_buffer + m_putPosition, buff, size);
      m_putPosition += size;
   }
   else  // two chunks, one at the tail, one at the head
   {
      unsigned sizeToTheEnd = m_bufferSize - m_putPosition;
      unsigned sizeFromTheStart = size - sizeToTheEnd;
      if ( static_cast<int>(sizeFromTheStart) <= 0 ) // fit at the tail
      {
          memcpy(m_buffer + m_putPosition, buff, size);
          m_putPosition += size;
      }
      else
      {
          memcpy(m_buffer + m_putPosition, buff, sizeToTheEnd);
          memcpy(m_buffer, buff + sizeToTheEnd, sizeFromTheStart);
          m_putPosition = sizeFromTheStart;
      }
   }
   if ( m_putPosition == m_bufferSize )
      m_putPosition = 0;
}

unsigned MBufferCircular::Get(char* buff, unsigned size)
{
   unsigned remainingBytes = GetSize();
   if ( remainingBytes < size )
      size = remainingBytes;

   if ( m_getPosition <= m_putPosition ) // one chunk, easy go
   {
      M_ASSERT(m_putPosition - m_getPosition >= size); // and we can be sure the size fits
      memcpy(buff, m_buffer + m_getPosition, size);
      m_getPosition += size;
   }
   else // two chunks, tail and head
   {
      unsigned sizeToTheEnd = m_bufferSize - m_getPosition;
      unsigned sizeFromTheStart = size - sizeToTheEnd;
      if ( static_cast<int>(sizeFromTheStart) <= 0 ) // fit at the tail
      {
         memcpy(buff, m_buffer + m_getPosition, size);
         m_getPosition += size;
      }
      else
      {
         memcpy(buff, m_buffer + m_getPosition, sizeToTheEnd);
         memcpy(buff + sizeToTheEnd, m_buffer, sizeFromTheStart);
         m_getPosition = sizeFromTheStart;
      }
   }
   if ( m_getPosition == m_bufferSize )
      m_getPosition = 0;
   return size; // can be smaller than requested
}
