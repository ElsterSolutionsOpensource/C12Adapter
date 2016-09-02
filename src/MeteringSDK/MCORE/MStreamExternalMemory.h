#ifndef MCORE_MSTREAMEXTERNALMEMORY_H
#define MCORE_MSTREAMEXTERNALMEMORY_H
/// \file MCORE/MStreamExternalMemory.h
/// \addtogroup MCORE
///@{

#include <MCORE/MCOREDefs.h>
#include <MCORE/MStream.h>

/// The random access stream, arranged on the memory chunk, that this class does not own.
///
/// This is sometimes more convenient or faster than MStreamMemory as it avoids extra data copying.
/// The class is not reflected.
///
class M_CLASS MStreamExternalMemory : public MStream
{
public: // Constructors and destructor:

   /// Create object without associating it with any buffer.
   ///
   /// \see Open for opening the chunk of memory for reading or writing.
   /// \see OpenReadOnly for opening the chunk of memory only for reading.
   ///
   MStreamExternalMemory();

   /// Creates the stream on the given read/write memory buffer.
   ///
   /// \param buffer
   ///     Buffer to use for reading and writing.
   ///
   /// \param bufferSize
   ///     The size of the buffer.
   ///
   /// \param fileSize
   ///     Initial size of the file arranged in the buffer.
   ///     The value would be zero if the buffer is to be filled with data.
   ///     The file size shall not be bigger than bufferSize.
   ///
   /// \param flags
   ///     Flags that shall be used by the stream, default is FlagReadWrite.
   ///     It would be a logic error to not have write access in flags.
   ///
   MStreamExternalMemory(char* buffer, unsigned bufferSize, unsigned fileSize, unsigned flags = FlagReadWrite);

   /// Creates the stream on the given read-only memory buffer.
   ///
   /// \param buffer
   ///     Buffer to use for reading.
   ///
   /// \param bufferAndFileSize
   ///     The size of the buffer, equal to size of the file.
   ///
   /// \param flags
   ///     Flags that shall be used by the stream, default is FlagReadOnly.
   ///     It would be a logic error to give write access in flags, as the buffer is not supposed to be changed.
   ///
   MStreamExternalMemory(const char* buffer, unsigned bufferAndFileSize, unsigned flags = FlagReadOnly);

   /// Closes the stream and destroys the object.
   ///
   virtual ~MStreamExternalMemory() M_NO_THROW;

public: // Properties:

   /// Return a representative name of a stream.
   ///
   /// This method overwrites the parent implementation.
   /// The name always returned by this type of stream is "<mem>"
   ///
   virtual MStdString GetName() const;

   ///@{
   /// Access the buffer associated with the stream.
   ///
   char* GetBuffer()
   {
      return m_buffer;
   }
   const char* GetBuffer() const
   {
      return m_buffer;
   }
   ///@}

   /// Return the buffer size of the stream.
   ///
   /// For read-only streams, this is also the size of the stream,
   /// while for read-write streams, this value can be bigger than stream size.
   ///
   unsigned GetBufferSize() const
   {
      return m_bufferSize;
   }

   /// Opens the stream on the given read-write memory buffer.
   ///
   /// \param buffer
   ///     Buffer to use for reading and writing.
   ///
   /// \param bufferSize
   ///     The size of the buffer.
   ///
   /// \param fileSize
   ///     Initial size of the file arranged in the buffer.
   ///     The value would be zero if the buffer is to be filled with data.
   ///     The file size shall not be bigger than bufferSize.
   ///
   /// \param flags
   ///     Flags that shall be used by the stream, default is FlagReadWrite.
   ///     It would be a logic error to not have write access in flags.
   ///
   void Open(char* buffer, unsigned bufferSize, unsigned fileSize, unsigned flags = FlagReadWrite);

   /// Opens the stream on the given read-only memory buffer.
   ///
   /// \param buffer
   ///     Buffer to use for reading.
   ///
   /// \param bufferAndFileSize
   ///     The size of the buffer, equal to size of the file.
   ///
   /// \param flags
   ///     Flags that shall be used by the stream, default is FlagReadOnly.
   ///     It would be a logic error to give write access in flags, as the buffer is not supposed to be changed.
   ///
   void OpenReadOnly(const char* buffer, unsigned bufferAndFileSize, unsigned flags = FlagReadOnly);

   /// Close the stream and clear any buffer pointers associated with the stream.
   ///
   /// After the standard method Close, the buffer and size methods can be used
   /// to access the buffer, but after CloseAndClear they all get nullified.
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

/// \endcond SHOW_INTERNAL
private: // Data:

   // Working buffer
   //
   char* m_buffer;

   // Size of the buffer
   //
   unsigned m_bufferSize;

   // Size of the buffer
   //
   unsigned m_fileSize;

   // Stream pointer
   //
   unsigned m_position;
};

///@}
#endif
