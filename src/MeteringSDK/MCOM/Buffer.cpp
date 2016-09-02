// File MCOM/Buffer.cpp

#include "MCOMExtern.h"
#include "Buffer.h"
#include <MCORE/MIso8825.h>
#include "MCOMExceptions.h"

void MBuffer::AppendIsoLength(unsigned len)
{
   char buff [ 8 ];
   unsigned size = MIso8825::EncodeLengthIntoBuffer(len, buff);
   m_bytes.append(buff, size);
}

void MBuffer::AppendUidIfPresent(char tag, const MByteString& uid)
{
   if ( !uid.empty() )
   {
      char buff [ 64 ];
      unsigned size = MIso8825::EncodeTaggedUidIntoBuffer(tag, uid, buff);
      Append(buff, size);
   }
}

void MBuffer::AppendUnsigned(char tag, unsigned val)
{
   char buff [ 8 ];
   unsigned size = MIso8825::EncodeTaggedUnsignedIntoBuffer(tag, val, buff);
   Append(buff, size);
}

void MBufferReader::ReadBytes(unsigned size, MByteString& result)
{
   if ( size > 0 )
   {
      result.clear();
      result.resize(size);
      ReadBuffer(&result[0], size);
   }
}

void MBufferReader::ReadRemainingBytes(MByteString& result)
{
   ReadBytes(GetRemainingReadSize(), result);
}

void MBufferReader::ReadBuffer(Muint8* data, unsigned size)
{
   M_ASSERT(m_buff != NULL);
   if ( size > 0 )
   {
      unsigned remainingSize = GetRemainingReadSize();
      if ( remainingSize < size )
      {
         MCOMException::CheckIfExpectedDataSizeDifferent(remainingSize, size); // always throws, remainingSize is less than length
         M_ENSURED_ASSERT(0);
      }
      memcpy(data, GetReadPtr(), size);
      m_readPosition += size;
   }
}

void MBufferReader::SetEndPosition(unsigned pos)
{
   M_ASSERT(m_buff != NULL && m_readPosition <= pos);
   unsigned totalSize = m_buff->GetTotalSize();
   if ( pos > totalSize )
   {
      MCOMException::CheckIfExpectedDataSizeDifferent(totalSize, pos);
      M_ENSURED_ASSERT(0);
   }
   m_readEnd = pos;
}

Muint8 MBufferReader::ReadByte()
{
   Muint8 c;
   ReadBuffer(&c, sizeof(c));
   return c;
}

unsigned MBufferReader::ReadIsoLength()
{
   return MIso8825::DecodeLengthFromBuffer(GetTotalPtr(), m_readEnd, &m_readPosition);
}
