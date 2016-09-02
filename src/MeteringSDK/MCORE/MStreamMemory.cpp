// File MCORE/MStreamMemory.cpp

#include "MCOREExtern.h"
#include "MStreamMemory.h"
#include "MException.h"

   #if !M_NO_REFLECTION

      /// Creates the memory stream filled with specified data using flags given.
      ///
      /// \param bytes
      ///     Buffer with data that shall be used for the stream.
      ///
      /// \param flags
      ///     Flags with which the stream shall be open.
      ///
      static MObject* DoNew2(const MByteString& bytes, unsigned flags)
      {
         return M_NEW MStreamMemory(bytes, flags);
      }

      /// Creates the memory stream filled by specified data.
      ///
      /// The stream will be opened with FlagReadWrite.
      ///
      /// \param bytes
      ///     Buffer with data that shall be used for the stream.
      ///
      static MObject* DoNew1(const MByteString& bytes)
      {
         return DoNew2(bytes, MStream::FlagReadWrite);
      }

      /// Default constructor creates the memory stream with an empty buffer for reading and writing.
      ///
      /// The stream will be empty, opened with FlagReadWrite.
      ///
      static MObject* DoNew0()
      {
         return M_NEW MStreamMemory();
      }

   #endif

M_START_PROPERTIES(StreamMemory)
   M_OBJECT_PROPERTY_READONLY_BYTE_STRING(StreamMemory, Buffer, ST_constMByteStringA_X)
M_START_METHODS(StreamMemory)
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (StreamMemory, New,  DoNew2,  2, ST_MObjectP_S_constMByteStringA_unsigned)
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (StreamMemory, New,  DoNew1,  1, ST_MObjectP_S_constMByteStringA)
   M_CLASS_FRIEND_SERVICE_OVERLOADED     (StreamMemory, New,  DoNew0,  0, ST_MObjectP_S)
   M_OBJECT_SERVICE_OVERLOADED           (StreamMemory, Open, Open,    2, ST_X_constMByteStringA_unsigned)
   M_OBJECT_SERVICE_OVERLOADED           (StreamMemory, Open, DoOpen1, 1, ST_X_constMByteStringA) // SWIG_HIDE
   M_OBJECT_SERVICE                      (StreamMemory, CloseAndClear,    ST_X)
M_END_CLASS(StreamMemory, Stream)

MStreamMemory::MStreamMemory(unsigned flags)
:
   MStream(),
   m_buffer(),
   m_position(0)
{
   OpenBuffer("", 0, flags);
}

MStreamMemory::MStreamMemory(const char* buffer, unsigned size, unsigned flags)
:
   MStream(),
   m_buffer(),
   m_position(0)
{
   OpenBuffer(buffer, size, flags);
}

MStreamMemory::MStreamMemory(const MByteString& bytes, unsigned flags)
:
   MStream(),
   m_buffer(),
   m_position(0)
{
   Open(bytes, flags);
}

MStreamMemory::~MStreamMemory() M_NO_THROW
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

void MStreamMemory::CloseAndClear()
{
   Close();
   m_buffer.clear();
   M_ASSERT(m_position == 0);
}

unsigned MStreamMemory::DoGetPosition() const
{
   return m_position;
}

void MStreamMemory::DoSetPosition(unsigned position)
{
   if ( position > m_buffer.size() )
   {
      DoThrowEndOfStream();
      M_ENSURED_ASSERT(0);
   }
   m_position = position;
}

unsigned MStreamMemory::DoGetSize() const
{
   return M_64_CAST(unsigned, m_buffer.size());
}

void MStreamMemory::DoSetSize(unsigned length)
{
   if ( size_t(length) > m_buffer.size() )
   {
      DoThrowEndOfStream();
      M_ENSURED_ASSERT(0);
   }
   if ( size_t(length) < m_buffer.size() )
   {
      m_buffer.erase(m_buffer.begin() + length, m_buffer.end());
      m_position = M_64_CAST(unsigned, m_buffer.size());
   }
}

void MStreamMemory::DoOpen1(const MByteString& bytes)
{
   Open(bytes);
}

void MStreamMemory::Open(const MByteString& bytes, unsigned flags)
{
#ifdef M_USE_USTL
   OpenBuffer(const_cast<MByteString&>(bytes).data(), M_64_CAST(unsigned, bytes.size()), flags);
#else
   OpenBuffer(bytes.data(), M_64_CAST(unsigned, bytes.size()), flags);
#endif
}

void MStreamMemory::OpenBuffer(const char* buffer, unsigned length, unsigned flags)
{
   CloseAndClear();
   DoStartOpen(flags);
   m_buffer.assign(buffer, buffer + length);
   DoFinishOpen();
}

unsigned MStreamMemory::DoReadAvailableBytesImpl(char* buffer, unsigned count)
{
   unsigned size = M_64_CAST(unsigned, m_buffer.size());
   if ( m_position == size )
      return 0u;
   M_ASSERT(m_position < size);
   unsigned len = size - m_position;
   if ( len > count )
      len = count;
   memcpy(buffer, m_buffer.data() + m_position, len);
   m_position += len;
   return len;
}

void MStreamMemory::DoWriteBytesImpl(const char* buffer, unsigned count)
{
   unsigned size = M_64_CAST(unsigned, m_buffer.size());
   M_ASSERT(size >= m_position);
   unsigned toWrite = size - m_position;
   if ( toWrite > 0 )
   {
      if ( toWrite > count ) // if we are fit within buffer
         toWrite = count;
      memcpy(&m_buffer[m_position], buffer, toWrite);
      m_position += toWrite;
      count -= toWrite;
      if ( count == 0 )
         return;
      buffer += toWrite;
   }
   m_buffer.append(buffer, count);
   m_position += count;
   M_ASSERT(m_position <= m_buffer.size());
}

void MStreamMemory::DoCloseImpl()
{
   m_position = 0;
   // Intentionally do nothing else here
}

MStdString MStreamMemory::GetName() const
{
   return MStdString("<memory>", 8);
}

bool MStreamMemory::DoIsOpenImpl() const
{
   return true; // Memory stream is always open. Even closed stream is open.
}
