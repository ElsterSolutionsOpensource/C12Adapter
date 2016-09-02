// File MCORE/MStreamExternalMemory.cpp

#include "MCOREExtern.h"
#include "MStreamExternalMemory.h"
#include "MException.h"

MStreamExternalMemory::MStreamExternalMemory()
:
   MStream(),
   m_buffer(NULL),
   m_bufferSize(0),
   m_fileSize(0),
   m_position(0)
{
}

MStreamExternalMemory::MStreamExternalMemory(char* buffer, unsigned bufferSize, unsigned fileSize, unsigned flags)
:
   MStream(),
   m_buffer(NULL),
   m_bufferSize(0),
   m_fileSize(0),
   m_position(0)
{
   Open(buffer, bufferSize, fileSize, flags);
}

MStreamExternalMemory::MStreamExternalMemory(const char* buffer, unsigned size, unsigned flags)
:
   MStream(),
   m_buffer(NULL),
   m_bufferSize(0),
   m_fileSize(0),
   m_position(0)
{
   OpenReadOnly(buffer, size, flags);
}

MStreamExternalMemory::~MStreamExternalMemory() M_NO_THROW
{
   try
   {
      Close();
   }
   catch ( ... )
   {
      M_ASSERT(0);
   }
}

void MStreamExternalMemory::CloseAndClear()
{
   Close();
   m_buffer = NULL;
   m_bufferSize = 0;
   m_fileSize = 0;
   m_position = 0;
}

unsigned MStreamExternalMemory::DoGetPosition() const
{
   return m_position;
}

void MStreamExternalMemory::DoSetPosition(unsigned position)
{
   if ( position > m_fileSize )
   {
      DoThrowEndOfStream();
      M_ENSURED_ASSERT(0);
   }
   m_position = position;
}

unsigned MStreamExternalMemory::DoGetSize() const
{
   return m_fileSize;
}

void MStreamExternalMemory::DoSetSize(unsigned length)
{
   if ( size_t(length) > m_fileSize )
   {
      DoThrowEndOfStream();
      M_ENSURED_ASSERT(0);
   }
   m_bufferSize = length;
}

void MStreamExternalMemory::Open(char* buffer, unsigned bufferSize, unsigned fileSize, unsigned flags)
{
   CloseAndClear();
   MENumberOutOfRange::CheckNamedUnsignedRange(0, bufferSize, fileSize, "FileSize");
   DoStartOpen(flags);
   m_buffer = buffer;
   m_bufferSize = bufferSize;
   m_fileSize = fileSize;
   m_position = 0;
   DoFinishOpen();
}

void MStreamExternalMemory::OpenReadOnly(const char* buffer, unsigned length, unsigned flags)
{
   CloseAndClear();
   if ( (flags & FlagWriteOnly) != 0 )
   {
      MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_BAD_STREAM_FLAG, "Specify ReadOnly flag for read-only stream"));
      M_ENSURED_ASSERT(0);
   }
   DoStartOpen(flags);
   m_buffer = const_cast<char*>(buffer);
   m_bufferSize = length;
   m_fileSize = length;
   m_position = 0;
   DoFinishOpen();
}

unsigned MStreamExternalMemory::DoReadAvailableBytesImpl(char* buffer, unsigned count)
{
   if ( m_position == m_fileSize )
      return 0u;
   M_ASSERT(m_position < m_fileSize);
   unsigned len = m_fileSize - m_position;
   if ( len > count )
      len = count;
   memcpy(buffer, m_buffer + m_position, len);
   m_position += len;
   return len;
}

void MStreamExternalMemory::DoWriteBytesImpl(const char* buffer, unsigned count)
{
   M_ASSERT(m_fileSize >= m_position);
   if ( count != 0 )
   {
      if ( m_position + count > m_bufferSize )
      {
         DoThrowEndOfStream();
         M_ENSURED_ASSERT(0);
      }
      memcpy(m_buffer + m_position, buffer, count);
      m_position += count;
      if ( m_position > m_fileSize )
         m_fileSize = m_position;
      M_ASSERT(m_position <= m_bufferSize);
      M_ASSERT(m_fileSize <= m_bufferSize);
   }
}

void MStreamExternalMemory::DoCloseImpl()
{
   m_position = 0;
   // Intentionally do nothing else here
}

MStdString MStreamExternalMemory::GetName() const
{
   return MStdString("<mem>", 5);
}

bool MStreamExternalMemory::DoIsOpenImpl() const
{
   return true; // memory stream is always open. Even closed stream is open (but it might have zero size)
}
