#ifndef MCOM_BUFFER_H
#define MCOM_BUFFER_H
/// \addtogroup MCOM
///@{
/// \file MCOM/Buffer.h

#include <MCOM/MCOMDefs.h>

/// Buffer, a byte string with extra manipulation facilities.
///
/// The internal implementation is subject to change.
/// The base class provides lightweight manipulation with an array of bytes
/// similar to standard vector, except it can be 'read' sequentially like a stream with \ref MBufferReader.
///
/// The real big reason for existence of this simple class is its child
/// \ref MBufferBidirectional, which is like std::deque, but more efficient
/// for prepending the buffer with a 'header'.
///
/// There is also unrelated class \ref MBufferCircular, variable capacity circular buffer.
///
/// Care must be taken as for efficiency reasons the destructor of this class is not virtual.
/// No virtual functions exist either. Therefore, one should never hold this class
/// with an owning generic pointer.
///
class MCOM_CLASS MBuffer
{
public: // methods:

   /// Constructor of an empty unallocated buffer.
   ///
   /// \see \ref Clear - will restore buffer into empty state, however keeping all preallocated space.
   ///
   MBuffer()
   :
      m_bytes()
   {
   }

   /// Attention, the destructor is not virtual.
   ///
   ~MBuffer()
   {
   }

public: // accessing whole buffer, header might be wrong

   /// Access all bytes in the buffer.
   ///
   MByteString& AccessAllBytes()
   {
      return m_bytes;
   }

   /// Constant access all bytes in the buffer.
   ///
   const MByteString& AccessAllBytes() const
   {
      return m_bytes;
   }

public: // Clearing, resetting, resizing:

   /// Resize buffer.
   ///
   /// The buffer will have a new size after the call.
   ///
   /// \param size New buffer size.
   ///
   /// \see \ref Reserve - change the internal buffer capacity, but not the size of the buffer.
   ///
   void Resize(unsigned size)
   {
      m_bytes.resize(size);
   }

   /// Reserve the given number of bytes in the whole buffer.
   ///
   /// Keep the buffer size the same.
   ///
   /// \param capacity New buffer capacity, how many bytes can be added efficiently.
   ///
   /// \see \ref Resize - change both the internal buffer capacity and buffer size.
   ///
   void Reserve(unsigned capacity)
   {
      m_bytes.reserve(capacity);
   }

   /// Make buffer size equal to zero.
   ///
   void Clear()
   {
      m_bytes.clear();
   }

public: // Adding packet chunks

   /// Assign the whole buffer with the given data.
   ///
   /// \param data New data for the buffer.
   ///
   /// \see \ref Append(const MByteString&) - append a chunk to the existing buffer.
   ///
   void Assign(const MByteString& data)
   {
      m_bytes = data;
   }

   /// Assign the whole buffer with the given data.
   ///
   /// \param buff Pointer to the new data buffer.
   /// \param size How many bytes to copy from the buffer.
   ///
   /// \see \ref Append(const char*, unsigned) - append a chunk to the existing buffer.
   ///
   void Assign(const char* buff, unsigned size)
   {
      m_bytes.assign(buff, size);
   }

   /// Append a character or byte to the buffer.
   ///
   /// \param c Character.
   ///
   void Append(char c)
   {
      m_bytes += c;
   }

   /// Append a chunk to the existing buffer.
   ///
   /// \param data Bytes to be appended.
   ///
   /// \see \ref Assign(const MByteString&) - Assign the whole buffer with the given data.
   ///
   void Append(const MByteString& data)
   {
      m_bytes += data;
   }

   /// Append a chunk determined by the pointer and size to the existing buffer.
   ///
   /// \param buff Pointer to the new data buffer.
   /// \param size How many bytes to copy from the buffer.
   ///
   /// \see \ref Assign(const char*, unsigned) - Assign the whole buffer with the given data.
   ///
   void Append(const char* buff, unsigned size)
   {
      m_bytes.append(buff, size);
   }

   /// Append ISO 8825 length BER representation to this buffer.
   ///
   /// \param len ISO 8825 length value.
   ///
   void AppendIsoLength(unsigned len);

   /// Append ISO 8825 UID to this buffer, if the uid is given.
   ///
   /// If the given uid is an empty string, nothing is done.
   /// Otherwise, added are:
   ///   - Tag, one byte.
   ///   - Data length and OID type, relative or absolute, all according to ISO 8825 encoding.
   ///   - Uid Raw byte data, added as is (no conversion of any type is done).
   ///
   /// \param tag BER object tag of the UID.
   /// \param uid Chunk of raw bytes. If not empty, added together with the properly formed header.
   ///
   void AppendUidIfPresent(char tag, const MByteString& uid);

   /// Append ISO 8825 unsigned value to this buffer.
   ///
   /// Added are:
   ///   - Tag, one byte.
   ///   - Data length and type, ISO 8825 encoding.
   ///   - Value Raw byte data, added as is (no conversion of any type is done).
   ///
   /// \param tag BER object tag of the UID.
   /// \param val Value to add.
   ///
   void AppendUnsigned(char tag, unsigned val);

public: // Reading information from buffer:

   /// Raw pointer to the whole buffer data.
   ///
   /// Care must be taken as some children overwrite this method nonvirtually.
   /// It is always assumed that this class is manipulated through its nongeneric instance.
   ///
   /// \see \ref GetTotalSize Size of this pointer data.
   ///
   char* GetTotalPtr()
   {
      return &m_bytes[0];
   }

   /// Constant raw pointer to the whole buffer data.
   ///
   /// Care must be taken as some children overwrite this method nonvirtually.
   /// It is always assumed that this class is manipulated through its nongeneric instance.
   ///
   /// \see \ref GetTotalSize Size of this pointer data.
   ///
   const char* GetTotalPtr() const
   {
      return m_bytes.data();
   }

   /// Size of the whole buffer.
   ///
   /// Care must be taken as some children overwrite this method nonvirtually.
   /// It is always assumed that this class is manipulated through its nongeneric instance.
   ///
   /// \see \ref GetTotalPtr for the pointer to data
   ///
   unsigned GetTotalSize() const
   {
      return static_cast<unsigned>(m_bytes.size());
   }

protected: // data:

   /// Buffer internal holder.
   ///
   MByteString m_bytes;
};

/// Buffer traverser or reader.
///
/// Uses buffer object to walk through it sequentially.
/// The buffer reader has
///   - Buffer object pointer, the one from which it reads bytes
///   - Read position, current offset for the next bytes to be read.
///     Read position should fit within the buffer object.
///   - End position, offset where the read should stop.
///     End position should not be smaller than read position,
///     but can be smaller than the buffer size.
///
/// One buffer can be used by multiple readers, each having its own
/// possibly overlapping read position at the end.
///
class MCOM_CLASS MBufferReader
{
public: // methods:

   /// Create an empty reader.
   ///
   /// Before use, the buffer has to be assigned to this object with \ref AssignBuffer.
   ///
   MBufferReader()
   :
      m_buff(NULL),
      m_readPosition(0),
      m_readEnd(0)
   {
   }

   /// Create a reader for a given buffer.
   ///
   /// The read position is zero, the start of the given buffer.
   /// The end position is the end of the buffer, buffer size.
   ///
   /// \param buffer Buffer object that will be read from the start.
   ///
   MBufferReader(MBuffer* buffer)
   {
      AssignBuffer(buffer);
   }

   /// Create a reader for a given buffer at a given position and size.
   ///
   /// \param buffer Buffer object that will be read from the given position.
   /// \param readPosition Start position, should be within the span of the buffer,
   ///               as checked with the debug level assert.
   /// \param readEnd End position, should be within the span of the buffer,
   ///               as checked with the debug level assert.
   ///
   MBufferReader(MBuffer* buffer, unsigned readPosition, unsigned readEnd)
   {
      AssignBuffer(buffer, readPosition, readEnd);
   }

   /// Create a reader from a given copy.
   ///
   /// The buffer object, as well as the read position and end, are copied
   /// and the new object can read the buffer independently.
   ///
   /// \param other Buffer reader from which this buffer is copied.
   ///
   MBufferReader(const MBufferReader& other)
   :
      m_buff(other.m_buff),
      m_readPosition(other.m_readPosition),
      m_readEnd(other.m_readEnd)
   {
   }

   /// Destructor.
   ///
   ~MBufferReader()
   {
   }

public: // Manipulating with position:

   ///@{
   /// Current read position of the buffer reader.
   ///
   unsigned GetReadPosition() const
   {
      M_ASSERT(m_buff != NULL);
      return m_readPosition;
   }
   void SetReadPosition(unsigned pos)
   {
      M_ASSERT(pos <= GetTotalSize());
      m_readPosition = pos;
   }
   ///@}

   ///@{
   /// Current end position of the buffer reader.
   ///
   unsigned GetEndPosition() const
   {
      M_ASSERT(m_buff != NULL);
      return m_readEnd;
   }
   void SetEndPosition(unsigned pos);
   ///@}

public: // Adding packet chunks

   /// Assign the buffer that is to be read.
   ///
   /// The read position is zero, the start of the given buffer.
   /// The end position is the end of the buffer, buffer size.
   /// It is okay if this reader was pointing to some other buffer prior to this call.
   ///
   /// \param buffer Buffer object that will be read from the start.
   ///
   void AssignBuffer(MBuffer* buffer)
   {
      M_ASSERT(buffer != NULL);
      m_buff = buffer;
      m_readPosition = 0;
      m_readEnd = buffer->GetTotalSize();
   }

   /// Assign a reader for a given buffer at a given position and end.
   ///
   /// It is okay if this reader was pointing to some other buffer prior to this call.
   ///
   /// \param buffer Buffer object that will be read from the given position.
   /// \param readPosition Start position, should be within the span of the buffer,
   ///               as checked with the debug level assert.
   /// \param readEnd End position, should be within the span of the buffer,
   ///               as checked with the debug level assert.
   ///
   void AssignBuffer(MBuffer* buffer, unsigned readPosition, unsigned readEnd)
   {
      M_ASSERT(buffer != NULL);
      M_ASSERT(readEnd <= buffer->GetTotalSize());
      m_buff = buffer;
      m_readEnd = readEnd;
      SetReadPosition(readPosition);
   }

public: // Reading information from buffer:

   /// Access constant pointer of the whole buffer, regardless of the reader position.
   ///
   const char* GetTotalPtr() const
   {
      M_ASSERT(m_buff != NULL);
      return m_buff->GetTotalPtr();
   }

   /// Access the size of the whole buffer, regardless of the reader position.
   ///
   unsigned GetTotalSize() const
   {
      M_ASSERT(m_buff != NULL);
      return m_readEnd;
   }

   /// Access the pointer to data to which the current position of the reader points.
   ///
   char* GetReadPtr()
   {
      M_ASSERT(m_buff != NULL);
      return m_buff->GetTotalPtr() + m_readPosition;
   }

   /// Access the constant pointer to data to which the current position of the reader points.
   ///
   const char* GetReadPtr() const
   {
      M_ASSERT(m_buff != NULL);
      return m_buff->GetTotalPtr() + m_readPosition;
   }

   /// The number of bytes left to read to reach the end of the reader.
   ///
   unsigned GetRemainingReadSize() const
   {
      M_ASSERT(m_buff != NULL);
      return m_readEnd - m_readPosition;
   }

public: // Reading methods:

   /// Traverse the reader current position by ignoring the given number of bytes.
   ///
   /// Only debug level checks are present.
   ///
   void IgnoreBytes(unsigned pos)
   {
      M_ASSERT(m_readPosition + pos <= m_readEnd);
      m_readPosition += pos;
   }

   /// Read characters from the buffer into the given pointer and size.
   ///
   /// \param data Pointer into which to read bytes.
   /// \param size How many bytes to read, should not be less than what is available in the data pointer.
   ///             If the requested size is smaller than what is available to reader,
   ///             an exception is thrown.
   ///
   void ReadBuffer(Muint8* data, unsigned size);

   /// Read bytes from the buffer into the given pointer and size.
   ///
   /// \param data Pointer into which to read bytes
   /// \param size How many bytes to read, should not be less than what is available in the data pointer.
   ///             If the requested size is smaller than what is available to reader,
   ///             an exception is thrown.
   ///
   void ReadBuffer(char* data, unsigned size)
   {
      ReadBuffer((Muint8*)data, size);
   }

   /// Read a byte from the buffer.
   ///
   /// If the read position is at the end already, an exception is thrown.
   ///
   /// \return Byte as read from the current position.
   ///
   Muint8 ReadByte();

   /// Read bytes from the buffer into the given result.
   ///
   /// The reason this method takes an 'out' variable is that
   /// it is typically kept with some preallocated memory for efficiency of multiple calls.
   ///
   /// \param size How many bytes to read, should not be less than what is available in the data pointer.
   ///             If the requested size is smaller than what is available to reader,
   ///             an exception is thrown.
   /// \param result Write-only parameter into which the data will be read.
   ///
   void ReadBytes(unsigned size, MByteString& result);

   /// Read all remaining bytes from the buffer into the given result.
   ///
   /// The reason this method takes an 'out' variable is that
   /// it is typically kept with some preallocated memory for efficiency of multiple calls.
   /// After this method is called, the read position will be equal to the end position.
   ///
   /// \param result Write-only parameter into which the data will be read.
   ///
   void ReadRemainingBytes(MByteString& result);

   /// Read an ISO 8825 length BER representation from this buffer.
   ///
   /// Exception will be thrown if there is not enough bytes in the buffer reader,
   /// or if the bytes do not comprise a proper ISO length.
   ///
   unsigned ReadIsoLength();

protected: // Data:

   /// Client buffer object, not owned by this class.
   ///
   MBuffer* m_buff;

   /// Current read position within the buffer.
   ///
   unsigned m_readPosition;

   /// End position within the buffer, possibly smaller than the buffer size.
   ///
   unsigned m_readEnd;
};

///@}

#endif
