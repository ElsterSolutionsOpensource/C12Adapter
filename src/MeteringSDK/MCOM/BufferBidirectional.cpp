// File MCOM/BufferBidirectional.cpp

#include "MCOMExtern.h"
#include "BufferBidirectional.h"
#include <MCORE/MIso8825.h>

void MBufferBidirectional::Assign(const MByteString& data)
{
   Assign(data.data(), static_cast<unsigned>(data.size()));
}

void MBufferBidirectional::Assign(const char* buff, unsigned size)
{
   ClearWithReserve(8, size + 8); // 8 bytes for possible serial C12.22 header
   Append(buff, size);
}

void MBufferBidirectional::ClearWithReserve(unsigned headerSize, unsigned totalCapacity)
{
   M_ASSERT(headerSize < totalCapacity);

   if ( m_bytes.capacity() < totalCapacity )  // according to standard, reserve() can reallocate. Prevent it.
      m_bytes.reserve(totalCapacity);

   m_indexHeaderStart = headerSize;
   m_indexHeaderEnd = headerSize;

   m_bytes.resize(m_indexHeaderEnd);
}

void MBufferBidirectional::Prepend(char c)
{
   Prepend(&c, sizeof(c));
}

void MBufferBidirectional::Prepend(const char* buff, unsigned size)
{
   int index = static_cast<int>(m_indexHeaderStart - size);
   if ( index < 0 )
   {  // reserve room for header
      const unsigned diff = -index;
      m_bytes.insert(0, diff, '\0');
      m_indexHeaderStart += diff;
      m_indexHeaderEnd += diff;
      index = 0;
   }
   memcpy(&m_bytes[index], buff, size);
   m_indexHeaderStart = index;
}

void MBufferBidirectional::Prepend(const MByteString& data)
{
   Prepend(data.data(), static_cast<unsigned>(data.size()));
}

void MBufferBidirectional::PrependIsoLength(unsigned len)
{
   char buff [ 8 ];
   unsigned size = MIso8825::EncodeLengthIntoBuffer(len, buff);
   Prepend(buff, size);
}

void MBufferBidirectional::PrependUidIfPresent(char tag, const MByteString& uid)
{
   if ( !uid.empty() )
   {
      char buff [ 64 ];
      unsigned size = MIso8825::EncodeTaggedUidIntoBuffer(tag, uid, buff);
      Prepend(buff, size);
   }
}

void MBufferBidirectional::PrependUnsigned(char tag, unsigned val)
{
   char buff [ 8 ];
   unsigned size = MIso8825::EncodeTaggedUnsignedIntoBuffer(tag, val, buff);
   Prepend(buff, size);
}

