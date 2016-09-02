// File MCORE/MMessageFile.cpp

#include "MCOREExtern.h"
#include "MMessageFile.h"
#include "MException.h"
#include "MStreamFile.h"

#if !M_NO_MESSAGE_CATALOG

   const Muint32 MESSAGE_FILE_TAG         = 0x950412DE;  // File header tag, by which we recognize the file is correct
   const Muint32 MESSAGE_FILE_TAG_SWAPPED = 0xDE120495;  // File header tag, made at big endian computer

MMessageFile::MMessageFile(const MStdString& name, const MStdString& domain)
:
   m_contents(NULL),
   m_domain(domain),
   m_fileName(name)
{
   M_ASSERT(name.size() > 3 && m_stricmp(name.substr(name.size() - 3, 3).c_str(), ".mo") == 0); // make sure the file ends with .mo

   MStreamFile file(m_fileName, MStreamFile::FlagReadOnly);
   m_size = file.GetSize();
   DoVerify(m_size > sizeof(MessageFileHeader) && m_size <= MaximumMessageFileSize);

   m_contents = M_NEW char [ m_size ];
   file.ReadBytes(m_contents, m_size);

   bool swap;
   unsigned originalsTableOffset;
   unsigned translationsTableOffset;
   MessageFileHeader* header = (MessageFileHeader*)m_contents;
   if ( header->m_tag == MESSAGE_FILE_TAG_SWAPPED ) // swapped
   {
      swap = true;
      m_stringsCount = MSwapUINT32(header->m_stringsCount);
      originalsTableOffset = MSwapUINT32(header->m_originalsTableOffset);
      translationsTableOffset = MSwapUINT32(header->m_translationsTableOffset);
   }
   else 
   {
      DoVerify(header->m_tag == MESSAGE_FILE_TAG); // direct header
      swap = false;
      m_stringsCount = header->m_stringsCount;
      originalsTableOffset = header->m_originalsTableOffset;
      translationsTableOffset = header->m_translationsTableOffset;
   }
   DoVerify(m_stringsCount <= MaximumCountOfStrings);    // ensure we don't have too much

   const unsigned tableSize = m_stringsCount * sizeof(MessageTableEntry); 
   m_originals    = (MessageTableEntry*)DoGetPointerAtOffset(originalsTableOffset, tableSize);
   m_translations = (MessageTableEntry*)DoGetPointerAtOffset(translationsTableOffset, tableSize);

   DoVerify((reinterpret_cast<size_t>(m_originals) & 0x3) == 0 &&      // ensure the originals are properly aligned
            (reinterpret_cast<size_t>(m_translations) & 0x3) == 0);    // ensure the translations are properly aligned
   if ( swap )
   {
      DoSwapTableEntries(m_originals);
      DoSwapTableEntries(m_translations);
   }
   DoVerifyTableEntries(m_originals);
   DoVerifyTableEntries(m_translations);
}

MMessageFile::~MMessageFile()
{
   delete[] m_contents;
}

void MMessageFile::DoVerify(bool expression) const
{
   M_ASSERT(expression); // debug mode handling
   if ( !expression )
   {
      MException::ThrowBadFileFormat(m_fileName);
      M_ENSURED_ASSERT(0);
   }
}

void MMessageFile::DoSwapTableEntries(MessageTableEntry* entries)
{
   MessageTableEntry* it = entries;
   MessageTableEntry* last = it + m_stringsCount;
   for ( ; it != last; ++it )
   {
      it->m_offset = MSwapUINT32(it->m_offset);
      it->m_length = MSwapUINT32(it->m_length);
   }
}

void MMessageFile::DoVerifyTableEntries(MessageTableEntry* entries)
{
   MessageTableEntry* it = entries;
   MessageTableEntry* last = it + m_stringsCount;
   for ( ; it != last; ++it )
   {
      DoVerify(it->m_offset < MaximumMessageFileSize && // make sure none of them are too big so the below comparison works due to overflow
               it->m_length < MaximumMessageFileSize && // make sure none of them are too big so the below comparison works due to overflow
               it->m_offset + it->m_length < m_size &&  // make sure the string fits into range
               ((const char*)(m_contents + it->m_offset))[it->m_length] == '\0'); // and it is zero terminated
   }
}

const void* MMessageFile::DoGetPointerAtOffset(Muint32 offset, Muint32 size) const
{
   M_ASSERT(m_size >= sizeof(MessageFileHeader));
   if ( (int)offset >= (int)m_size - (int)size )
   {
      MException::ThrowBadFileFormat(m_fileName);
      M_ENSURED_ASSERT(0);
   }
   return m_contents + offset;
}

const char* MMessageFile::Translate(MConstLocalChars str, unsigned& strSize) const M_NO_THROW
{
   // Will implement binary search or hash in the future...
   //
   const unsigned constOriginalStrSize = strSize; // use a temporary, tell the compiler strSize is invariant in the loop unless the string is found
   for ( unsigned i = 0; i < m_stringsCount; ++i )
   {
      const MessageTableEntry* orig = &m_originals[i];
      if ( constOriginalStrSize == orig->m_length ) // here is the performance trick - it is a lot faster to traverse through strings incrementally and compare length first
      {                                            // and only when the length is the same do a comparison with the fastest possible memcmp call
         const char* candidate = DoGetString(orig); 
         if ( memcmp(str, candidate, constOriginalStrSize) == 0 )
         {
            const MessageTableEntry* trans = &m_translations[i];
            strSize = trans->m_length;
            return DoGetString(trans);
         }
      }
   }
   return NULL;
}

#endif // !M_NO_MESSAGE_CATALOG
