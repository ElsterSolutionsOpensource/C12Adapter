#ifndef MCOM_BUFFERBIDIRECTIONAL_H
#define MCOM_BUFFERBIDIRECTIONAL_H
/// \addtogroup MCOM
///@{
/// \file MCOM/BufferBidirectional.h

#include <MCOM/Buffer.h>

/// Buffer where the data can be added to both the beginning and the end of the buffer.
///
/// The left part of the bidirectional buffer is the header, it is prepended.
/// The right part of the bidirectional buffer is the body, it is appended.
///
/// The internal implementation of this class is subject to change.
///
class MCOM_CLASS MBufferBidirectional : public MBuffer
{
public: // methods:

   /// Construct an empty bidirectional buffer.
   ///
   /// No preallocation is made.
   ///
   MBufferBidirectional()
   :
      MBuffer(),
      m_indexHeaderStart(0),
      m_indexHeaderEnd(0)
   {
   }

   /// Attention, the destructor is not virtual!
   ///
   ~MBufferBidirectional()
   {
   }

public: // Reading information from buffer:

   /// Raw pointer to the whole buffer data.
   ///
   /// Care must be taken as the base class has nonvirtual method with the same name.
   /// It is always assumed that this class is manipulated through its nongeneric instance.
   ///
   /// \see \ref GetTotalSize Size of this pointer data.
   ///
   const char* GetTotalPtr() const
   {
      return m_bytes.data() + m_indexHeaderStart;
   }

   /// Size of the whole buffer data.
   ///
   /// Care must be taken as the base class has nonvirtual method with the same name.
   /// It is always assumed that this class is manipulated through its nongeneric instance.
   ///
   /// \see \ref GetTotalPtr for the pointer to data.
   /// \see \ref GetHeaderSize Size of the header part of the total buffer.
   ///
   unsigned GetTotalSize() const
   {
      return static_cast<unsigned>(m_bytes.size() - m_indexHeaderStart);
   }

   /// Size of the header part of the buffer.
   ///
   /// Header is at the beginning of the total data.
   ///
   /// \see \ref GetTotalPtr for the pointer to header data, same as total data pointer.
   ///
   unsigned GetHeaderSize() const
   {
      return m_indexHeaderEnd - m_indexHeaderStart;
   }

   /// Raw pointer to the whole body part of the data.
   ///
   /// \see \ref GetBodySize Size of this pointer data.
   ///
   char* GetBodyPtr()
   {
      return &m_bytes[m_indexHeaderEnd];
   }

   /// Raw constant pointer to the whole body part of the data.
   ///
   /// \see \ref GetBodySize Size of this pointer data.
   ///
   const char* GetBodyPtr() const
   {
      return m_bytes.data() + m_indexHeaderEnd;
   }

   /// Size of the body part of the buffer.
   ///
   /// Header is at the beginning of the total data.
   ///
   /// \see \ref GetBodyPtr for the pointer to body data.
   ///
   unsigned GetBodySize() const
   {
      return static_cast<unsigned>(m_bytes.size() - m_indexHeaderEnd);
   }

   /// Clear the data, but keep reserved the byte space.
   ///
   /// \param headerSize How many bytes to reserve for the header.
   /// \param totalCapacity Total capacity, should not be smaller than the header size.
   ///
   void ClearWithReserve(unsigned headerSize, unsigned totalCapacity);

public: // assigning body:

   /// Assign the given data to the body, and clear the header.
   ///
   /// The data will be assigned to the right of the bidirectional buffer.
   ///
   /// \param data Bytes that will comprise the body of the buffer.
   ///
   void Assign(const MByteString& data);

   /// Assign the given pointer and size to the body, and clear the header.
   ///
   /// The data will be assigned to the right of the bidirectional buffer.
   ///
   /// \param buff Byte pointer that will comprise the body of the buffer.
   /// \param size Size of the data pointed by buff.
   ///
   void Assign(const char* buff, unsigned size);

public: // Prepending header:

   /// Prepend a character at the header of the bidirectional buffer.
   ///
   /// \param c Character or byte to prepend.
   ///
   /// \see \ref MBuffer::Append(char) Append a byte or character to the body, parent call.
   ///
   void Prepend(char c);

   /// Prepend bytes at the header of the bidirectional buffer.
   ///
   /// \param data Bytes to be prepended to the left of the buffer.
   ///
   /// \see \ref MBuffer::Append(const MByteString&) Append data to the body, parent call.
   ///
   void Prepend(const MByteString& data);

   /// Prepend bytes at the header of the bidirectional buffer.
   ///
   /// \param buff Pointer to bytes that will be prepended to the left of the buffer.
   /// \param size Size of the data at the pointer.
   ///
   /// \see \ref MBuffer::Append(const char*, unsigned) Append data to the body, parent call.
   ///
   void Prepend(const char* buff, unsigned size);

   /// Prepend ISO 8825 length to the header of the bidirectional buffer.
   ///
   /// \param len Value of length.
   ///
   /// \see \ref MBuffer::AppendIsoLength Append ISO length to the body, parent call.
   ///
   void PrependIsoLength(unsigned len);

   /// Prepend ISO 8825 UID to the header of this buffer, if the uid is given.
   ///
   /// If the given uid is an empty string nothing is done.
   /// Otherwise, added are:
   ///   - Tag, one byte.
   ///   - Data length and OID type, relative or absolute, all according to ISO 8825 encoding.
   ///   - Uid Raw byte data, added as is (no conversion of any type is done).
   ///
   /// \param tag BER object tag of the UID.
   /// \param uid Chunk of raw bytes. If not empty, added together with the properly formed header.
   ///
   /// \see \ref MBuffer::AppendUidIfPresent Append uid to the body, parent call.
   ///
   void PrependUidIfPresent(char tag, const MByteString& uid);

   /// Prepend ISO 8825 unsigned value to the header of this buffer.
   ///
   /// Added are:
   ///   - Tag, one byte.
   ///   - Data length and type, ISO 8825 encoding.
   ///   - Value Raw byte data, added as is (no conversion of any type is done).
   ///
   /// \param tag BER object tag of the UID.
   /// \param val Value to add.
   ///
   /// \see \ref MBuffer::AppendUnsigned Append unsigned value to the body, parent call.
   ///
   void PrependUnsigned(char tag, unsigned val);

private:

   void Clear();          // hide child's call
   void Resize(unsigned); // hide child's call

private:

   unsigned m_indexHeaderStart;
   unsigned m_indexHeaderEnd;
};

///@}
#endif
