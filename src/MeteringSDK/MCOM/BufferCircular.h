#ifndef MCOM_BUFFERCIRCULAR_H
#define MCOM_BUFFERCIRCULAR_H
/// \addtogroup MCOM
///@{
/// \file MCOM/BufferCircular.h

#include <MCOM/MCOMDefs.h>

/// Byte buffer with variable capacity that allows efficient buffering by having one reader/getter and one writer/putter.
///
/// This is a slightly more efficient std::deque, which lacks methods to pop chunks of bytes.
/// Grows as necessary to accommodate putting of any number of bytes.
/// However, of course, one cannot get more bytes than available in the buffer.
/// The class is low level and it does not throw exceptions as any would be a program error.
/// Synchronization has to be provided outside.
///
class MCOM_CLASS MBufferCircular
{
public: // Constants:

   enum
   {
      DEFAULT_INITIAL_CAPACITY = 1024 ///< Default initial capacity of the buffer
   };

public:

   /// Create buffer of a given initial capacity.
   ///
   /// \param initialCapacity Byte size of the initial buffer, will grow if necessary.
   ///               Should be more than 2, typical is a power of 2 like 512.
   ///               The default initial capacity is 1024.
   ///
   MBufferCircular(unsigned initialCapacity = DEFAULT_INITIAL_CAPACITY);

   /// Circular buffer destructor.
   ///
   ~MBufferCircular() M_NO_THROW;

public: // Properties:

   /// Number of bytes buffered, available for getting.
   ///
   unsigned GetSize() const
   {
      unsigned diff = m_putPosition - m_getPosition;
      if ( static_cast<int>(diff) < 0 )  // chunk rolled over
         diff += m_bufferSize;
      M_ASSERT(static_cast<int>(diff) >= 0);
      return diff;
   }

   /// How many bytes can be put into circular buffer without necessity to reallocate buffer.
   ///
   /// This method is rarely needed as the buffer is reallocated at necessity.
   ///
   unsigned CanPutWithoutResize() const
   {
      unsigned result = m_bufferSize - GetSize() - 1u; // -1 to distinguish empty buffer from full subtract one
      M_ASSERT(static_cast<int>(result) >= 0);
      return static_cast<unsigned>(result);
   }

public: // Methods:

   /// Clear the contents of the buffer so the size becomes zero.
   ///
   void Clear()
   {
      m_getPosition = 0u;
      m_putPosition = 0u;
   }

   /// Resize the buffer to given capacity.
   ///
   /// Note that the real amount of bytes that can be put into the buffer is one less than its capacity.
   /// In most cases there is no need for this method to be called as the growth happens
   /// transparently when necessary.
   ///
   /// \param newCapacity New capacity of the buffer.
   ///
   void Resize(unsigned newCapacity);

   /// Put the whole given buffer, grow object capacity if necessary.
   ///
   /// \param buff Buffer where the chunk is located.
   /// \param size Size of the chunk.
   ///
   void Put(const char* buff, unsigned size);

   /// Get the data given chunk from the circular buffer.
   ///
   /// \param buff Buffer where to get the data.
   /// \param size Size of the chunk. Size and offset should fit within the buffer length.
   /// \return Actual number of bytes got, could be zero if the buffer is empty.
   ///
   unsigned Get(char* buff, unsigned size);

private: // Data:

   char*    m_buffer;
   unsigned m_bufferSize;
   unsigned m_getPosition;
   unsigned m_putPosition;
};

///@}

#endif
