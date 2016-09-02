#ifndef MCORE_MSTREAMMEMORY_H
#define MCORE_MSTREAMMEMORY_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MStreamMemory.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MStream.h>

/// The random access stream in memory that is always open.
///
/// This class internally owns the buffer that it uses for reading and writing.
///
class M_CLASS MStreamMemory : public MStream
{
public: // Constructors and destructor:

   /// Default constructor creates the memory stream with an empty buffer.
   ///
   /// This is a handy way for creating empty memory streams that can be written.
   ///
   /// \param flags
   ///     Flags with which the stream shall be opened.
   ///     It is a logic error not to give write flag while creating the stream in such a way.
   ///
   MStreamMemory(unsigned flags = FlagReadWrite);

   /// Creates the memory stream filled by specified data.
   ///
   /// This is a C++ only method. The given buffer is going to be copied into the stream's internal storage.
   ///
   /// \param buffer
   ///     Pointer to buffer with data that shall be used for the stream.
   ///
   /// \param length
   ///     Length of the buffer.
   ///
   /// \param flags
   ///     Flags with which the stream shall be open.
   ///
   MStreamMemory(const char* buffer, unsigned length, unsigned flags = FlagReadWrite);

   /// Creates the memory stream filled by specified data.
   ///
   /// The given buffer is going to be copied into the stream's internal storage.
   ///
   /// \param bytes
   ///     Data that shall be used for the stream.
   ///
   /// \param flags
   ///     Flags with which the stream shall be open.
   ///
   MStreamMemory(const MByteString& bytes, unsigned flags = FlagReadWrite);

   /// Closes the stream and reclaims memory.
   ///
   virtual ~MStreamMemory() M_NO_THROW;

public: // Properties:

   /// Return a representative name of a stream.
   ///
   /// This method overwrites the parent implementation.
   /// The name always returned by this type of stream is "<memory>"
   ///
   virtual MStdString GetName() const;

   /// Access the whole stream data.
   ///
   const MByteString& GetBuffer() const
   {
      return m_buffer;
   }

   /// Opens the stream and initializes it with values from the given bytes.
   ///
   /// Later on, during write operations, the stream size can grow beyond the given number of bytes.
   /// This is possible because the given buffer is copied, and can grow.
   ///
   /// \param bytes
   ///     Initial bytes to use for reading and writing.
   ///
   /// \param flags
   ///     Flags that shall be used by the stream, default is FlagReadWrite.
   ///
   void Open(const MByteString& bytes, unsigned flags = FlagReadWrite);

   /// Opens the stream and initializes it with values from the given buffer.
   ///
   /// Later on, during write operations, the stream size can grow beyond the given number of bytes.
   /// This is possible because the given buffer is copied, and can grow.
   ///
   /// \param buffer
   ///     Buffer to use for reading and writing.
   ///
   /// \param length
   ///     The size of the buffer.
   ///
   /// \param flags
   ///     Flags that shall be used by the stream, default is FlagReadWrite.
   ///
   void OpenBuffer(const char* buffer, unsigned length, unsigned flags = FlagReadWrite);

   /// Close the stream and clear the buffer associated with the stream.
   ///
   /// After the standard method Close, the buffer can be accessed,
   /// but after CloseAndClear it will be cleared.
   ///
   /// If there is a necessity to clear the buffer without closing the stream
   /// one can assign zero to \refprop{MStream.SetSize,Stream.Size} property.
   ///
   void CloseAndClear();

protected: // Parent overloads:
/// \cond SHOW_INTERNAL

   virtual unsigned DoReadAvailableBytesImpl(char* buffer, unsigned count);

   virtual void DoWriteBytesImpl(const char* buffer, unsigned count);

   virtual bool DoIsOpenImpl() const;
   virtual void DoCloseImpl();

   virtual unsigned DoGetPosition() const;
   virtual void DoSetPosition(unsigned position);
   virtual unsigned DoGetSize() const;
   virtual void DoSetSize(unsigned length);

public: // Semi-public calls:

   /// Opens the stream and initializes it with values from the given bytes.
   ///
   /// The stream will be opened with FlagReadWrite.
   ///
   /// \param bytes
   ///     Initial bytes to use for reading and writing.
   ///
   void DoOpen1(const MByteString& bytes);

/// \endcond SHOW_INTERNAL
private: // Data:

   // Working buffer
   //
   MByteString m_buffer;

   // Stream pointer
   //
   unsigned m_position;

   M_DECLARE_CLASS(StreamMemory)
};

///@}
#endif
