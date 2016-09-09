#ifndef MCORE_MFINDFILE_H
#define MCORE_MFINDFILE_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MFindFile.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_FILESYSTEM

/// Operating system and compiler independent way to search for file or directory
///
class M_CLASS MFindFile
{
public: // Constructor, destructor and services.

   /// Default constructor, no search is initialized.
   ///
   MFindFile();

   /// Constructor, which initializes the search.
   ///
   /// It is not an error if the directory does not exist.
   /// In this case, \ref FindNext returns NULL.
   ///
   /// \param directory
   ///     Path to a directory where to search for files or subdirectories.
   ///     It can be full path or path relative to current directory.
   ///
   /// \param fileMask
   ///     The file mask cannot contain a subdirectory.
   ///     Regular file mask syntax applies, * and ? can be used for a sequence of characters and for a single character.
   ///
   /// \param searchForDirectories
   ///     When true, the search is done for directories only.
   ///     Otherwise, when false, the search is done only for ordinary files.
   ///
   /// \param reserved
   ///     Reserved parameter that has no effect.
   ///
   /// \param excludeDotFiles
   ///     Special directories '.' and '..' are never listed,
   ///     but when this parameter is true, any file that starts with period, such as ".svn",
   ///     will not appear in the result search.
   ///
   /// \see Init - initialize the search after the object has been constructed
   ///
   MFindFile(MConstChars directory, MConstChars fileMask, bool searchForDirectories = false, bool reserved = false, bool excludeDotFiles = false);

   /// Object destructor.
   ///
   ~MFindFile() M_NO_THROW;

   /// Close the result of the previous search.
   /// It is always safe to call Close, any number of times.
   ///
   void Close() M_NO_THROW;

   /// Initialize the search
   ///
   /// It is not an error if the directory does not exist.
   /// In this case, \ref FindNext returns NULL.
   ///
   /// \param directory
   ///     Path to a directory where to search for files or subdirectories.
   ///     It can be full path or path relative to current directory.
   ///
   /// \param fileMask
   ///     The file mask cannot contain a subdirectory.
   ///     Regular file mask syntax applies, * and ? can be used for a sequence of characters and for a single character.
   ///
   /// \param searchForDirectories
   ///     When true, the search is done for directories only.
   ///     Otherwise, when false, the search is done only for ordinary files.
   ///
   /// \param reserved
   ///     Reserved parameter that has no effect.
   ///
   /// \param excludeDotFiles
   ///     Special directories '.' and '..' are never listed,
   ///     but when this parameter is true, any file that starts with period, such as ".svn",
   ///     will not appear in the result search.
   ///
   void Init(MConstChars directory, MConstChars fileMask, bool searchForDirectories = false, bool reserved = false, bool excludeDotFiles = false);

   /// Find the next file that corresponds to the initialization parameters of the object.
   ///
   /// Note that the storage for the file name is allocated inside the class.
   /// It should not be attempted to be deleted.
   ///
   /// If the search for directories was initiated, then the file returned
   /// is a directory name, otherwise this is non-directory name.
   ///
   /// \param returnFullPath
   ///     If true, return the full path. Otherwise, return only file and extension.
   ///
   MConstChars FindNext(bool returnFullPath = true);

   /// Populate the result vector with files from the given directory. 
   ///
   /// This method combines the search initializer with FindNext.
   ///
   /// \param result
   ///     String vector, shall be empty prior to this call, will be populated with file names.
   ///
   /// \param directory
   ///     Path to a directory where to search for files or subdirectories.
   ///     It can be full path or path relative to current directory.
   ///
   /// \param fileMask
   ///     The file mask cannot contain a subdirectory.
   ///     Regular file mask syntax applies, * and ? can be used for a sequence of characters and for a single character.
   ///
   /// \param searchForDirectories
   ///     When true, the search is done for directories only.
   ///     Otherwise, when false, the search is done only for ordinary files.
   ///
   /// \param reserved
   ///     Reserved parameter that has no effect.
   ///
   /// \param excludeDotFiles
   ///     Special directories '.' and '..' are never listed,
   ///     but when this parameter is true, any file that starts with period, such as ".svn",
   ///     will not appear in the result search.
   ///
   static void Populate(MStdStringVector& result, const MStdString& directory, const MStdString& fileMask, bool searchForDirectories = false, bool reserved = false, bool excludeDotFiles = false)
   {
      M_USED_VARIABLE(reserved);
      DoPopulate(result, directory, fileMask, searchForDirectories, excludeDotFiles);
   }

private: // Method:

   static void DoPopulate(MStdStringVector& result, const MStdString& directory, const MStdString& fileMask, bool searchForDirectories, bool excludeDotFiles);
  
   static M_NORETURN_FUNC void DoThrowFindArgumentBad();

private: // Attributes:

   // Current item index in results
   //
   unsigned m_index;

   // Results of find operation.
   //
   MStdStringVector m_results;

   // Directory of find operation.
   //
   MStdString m_directory;

   // Results of find operation.
   //
   MStdString m_returned;
};

#endif // !M_NO_FILESYSTEM

///@}
#endif
