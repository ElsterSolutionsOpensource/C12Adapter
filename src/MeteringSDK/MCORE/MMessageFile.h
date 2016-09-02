#ifndef MCORE_MMESSAGEFILE_H
#define MCORE_MMESSAGEFILE_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MMessageFile.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_MESSAGE_CATALOG

/// Message file, internal implementation of file with international strings.
///
class M_CLASS MMessageFile
{
private: // Constants:

   enum
   {
      MaximumMessageFileSize = 0x0FFFFFFF, ///< Maximum supported message file size
      MaximumCountOfStrings  = 0x00FFFFFF  ///< Maximum supported number of strings in file
   };

private: // Types:

   // Message file header
   //
   struct MessageFileHeader
   {
      // Message file tag, a magic number
      //
      Muint32 m_tag;

      // Version of the message file format. Lowest possible version is supported.
      //
      Muint32 m_version;

      // Count of strings in the message file
      //
      Muint32 m_stringsCount;

      // Offset of the table with the original strings
      //
      Muint32 m_originalsTableOffset;

      // Offset of the table with the translated strings
      //
      Muint32 m_translationsTableOffset;

      // Size of hash table (ignored by current implementation)
      //
      Muint32 m_hashTableSize;

      // Offset of hash table (ignored by current implementation)
      //
      Muint32 m_hashTableOffset;
   };

   // Entry in the message table
   //
   struct MessageTableEntry
   {
      // String length
      //
      Muint32 m_length;

      // Offset of the string within file
      //
      Muint32 m_offset;
   };

public: // Constructor and destructor:

   /// Constructor that builds message file
   ///
   MMessageFile(const MStdString& fileName, const MStdString& domain);

   /// Destructor that destroys message file.
   ///
   ~MMessageFile();

   /// Access domain of this message source.
   ///
   const MStdString& GetDomain() const
   {
      return m_domain;
   }

   /// Assign domain of this message source.
   ///
   void SetDomain(const MStdString& domain)
   {
      m_domain = domain;
   }

   /// Access file name where this object is stored.
   ///
   const MStdString& GetFileName() const
   {
      return m_fileName;
   }

   /// Translate the given English string using this message file.
   /// The string size given shall be correct.
   ///
   const char* Translate(MConstLocalChars str, unsigned& strSize) const M_NO_THROW;

private: // Services:

   // Get pointer stored in file at a given offset, make sure it can accommodate at least length bytes.
   //
   // \pre The given offset and length shall be in range of a file size.
   //
   const void* DoGetPointerAtOffset(Muint32 offset, Muint32 size) const;

   // Implementation method that fetches string using message entry.
   //
   const char* DoGetString(const MessageTableEntry* ent) const
   {
      M_ASSERT(ent != NULL);
      const char* result = (const char*)(m_contents + ent->m_offset);
      M_ASSERT(result[ent->m_length] == '\0');                               // verify the result is zero-terminated (as verified at load time)
      return result; // all checks are done already at file load, assertions made, return string
   }

   // Local verification procedure used at load time.
   //
   // \pre If expression is false, bad file format is reported.
   //
   void DoVerify(bool expression) const;

   // Used at load time, this private procedure swaps all values within table entry array, given as pointer.
   // The length of the table entry array is m_stringsCount.
   //
   void DoSwapTableEntries(MessageTableEntry* entries);

   // Used at load time, this private procedure verifies all values within table entry array, given as pointer.
   // The length of the table entry array is m_stringsCount.
   //
   // \pre If any table entry is bad, an exception is thrown.
   //
   void DoVerifyTableEntries(MessageTableEntry* entries);

private: // Data:

   // The whole message file is stored here
   //
   char* m_contents;

   // Size of the above contents, used for verification
   //
   Muint32 m_size;

   // Count of strings, as in the file header
   //
   Muint32 m_stringsCount;

   // First message entry for original English strings
   //
   MessageTableEntry* m_originals;

   // First message entry for translated strings
   //
   MessageTableEntry* m_translations;

   // Message file text domain, as defined by POSIX.
   //
   MStdString m_domain;

   // Message file name
   //
   MStdString m_fileName;
};

#endif // !M_NO_MESSAGE_CATALOG
///@}
#endif
