#ifndef MCORE_MSTREAMPROCESSOR_H
#define MCORE_MSTREAMPROCESSOR_H
// File MCORE/MStreamProcessor.h

#include <MCORE/MStream.h>
#include <MCORE/MAesEax.h>
#include <MCORE/MRandomGenerator.h>

#if !defined(MCORE_PROJECT_COMPILING) || MCORE_PROJECT_COMPILING == 0
   #error "This header shall not be included into client code!"
#endif

/// \cond SHOW_INTERNAL

class MStreamProcessor : public MStream
{
   friend class MStream;

public: // Type:

   enum
   {
      MANGLE_BYTE = 0xAC,

      #if M_DEBUG
         STREAM_BUFFER_SIZE = 64 // increase rate of page changes
      #else
         STREAM_BUFFER_SIZE = 4096
      #endif
   };

public: // Overloads:

   MStreamProcessor()
   :
      MStream()
   {
   }

   virtual ~MStreamProcessor() M_NO_THROW
   {
   }

   virtual MStdString GetName() const;

   virtual void DoSetKeyImpl(const MByteString&);
   virtual void DoCloseImpl();
   virtual void DoFlushImpl(bool softFlush);
   virtual bool DoIsOpenImpl() const;

   virtual unsigned DoGetSize() const;
   virtual void DoSetSize(unsigned length);
   virtual unsigned DoGetPosition() const;
   virtual void DoSetPosition(unsigned length);

};

#if (M_OS & M_OS_WINDOWS) != 0
class MStreamProcessorText : public MStreamProcessor
{
public: // Overloads:

   MStreamProcessorText()
   :
      MStreamProcessor()
   {
   }

   virtual ~MStreamProcessorText() M_NO_THROW;

   virtual unsigned DoReadAvailableBytesImpl(char* buffer, unsigned count);

   virtual void DoWriteBytesImpl(const char* buffer, unsigned count);
};
#endif

class MStreamProcessorBuffered : public MStreamProcessor
{
protected:

   union Page
   {
      char    m_bytes  [ STREAM_BUFFER_SIZE ];
      Muint64 m_qwords [ STREAM_BUFFER_SIZE / sizeof(Muint64) ];
   };

public: // Data:

   const unsigned   m_fileHeaderSize; // Size of file header
   const unsigned   m_pageDataSize;   // Data size in the page size (equal or smaller than STREAM_BUFFER_SIZE)
   unsigned         m_pageInBuffer;   // Currently loaded page number, starting from zero. Each page consists of a buffer
   mutable unsigned m_fileSize;       // Cached real file size (excluding possibly unflushed bufer 
   unsigned         m_pageOfFile;     // Page number at which file pointer points (way of caching SetPosition calls)
   unsigned         m_buffCurr;       // Current offset in the buffer
   unsigned         m_buffEnd;        // End of the buffer
   bool             m_buffPresent;    // Whether the buffer was read from the stream, and so it is available
   bool             m_buffChanged;    // Whether the buffer was changed by write operation
   Page             m_buff;

public: // Methods:

   MStreamProcessorBuffered(unsigned flags, unsigned pageDataSize = STREAM_BUFFER_SIZE, unsigned fileHeaderSize = 0)
   :
      MStreamProcessor(),
      m_fileHeaderSize(fileHeaderSize),
      m_pageDataSize(pageDataSize),
      m_pageInBuffer(0),      // is always good
      m_fileSize(UINT_MAX),   // UINT_MAX means not cached
      m_pageOfFile(pageDataSize == 0 ? 0 : UINT_MAX),        // UINT_MAX means not good
      m_buffCurr(0),          // is always good
      m_buffEnd(0),           // is always good
      m_buffPresent(false),   // properties are good, but the buffer is not read
      m_buffChanged(false)
   {
      m_flags = flags;
      M_ASSERT((m_pageDataSize % sizeof(Muint64)) == 0);
   }

   virtual ~MStreamProcessorBuffered() M_NO_THROW
   {
   }

   virtual unsigned DoReadAvailableBytesImpl(char* buffer, unsigned count);
   void DoReadPage(unsigned page);
   virtual unsigned DoReadPageAtCurrentFilePosition();

   virtual void DoWriteBytesImpl(const char* buffer, unsigned count);
   virtual void DoWriteCurrentPage();
   virtual void DoWritePageAtCurrentFilePosition();

   virtual void DoFlushImpl(bool softFlush);

   virtual unsigned DoGetSize() const;
   virtual void DoSetSize(unsigned length);
   virtual unsigned DoGetPosition() const;
   virtual void DoSetPosition(unsigned length);

   void DoCheckEncryptedStreamFormat(bool trueCondition) const;
};


/// \endcond SHOW_INTERNAL

#endif

