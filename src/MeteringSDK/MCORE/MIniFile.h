#ifndef MCORE_MINIFILE_H
#define MCORE_MINIFILE_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MIniFile.h

#include <MCORE/MStreamFile.h>
#include <MCORE/MFileNameAndLineNumber.h>

#if !M_NO_FILESYSTEM

/// Class that allows reading and writing the Windows-like INI files sequentially.
///
/// Note that there is no searching through the file
/// for certain key and value implemented,
/// the file has to be read until a desired data is found.
///
/// The syntax of the INI file is as follows:
/// \code
///          [Key1]
///          Name1=1
///            Name2  =   2 ; blanks are stripped from both name and value
///
///          ; comment can be here
///          [Key2]       ; or here
///          Name4=Value4
///          Name5=Value5  ; comment
///          Name5="Value that can; have; semicolons" ; comment
///          ; Comment
///          SomeFileName = c:\example\of\a\string\that\is\taken\as.is ; comment can be here
///
///          V1= TRUE   ; Boolean TRUE, FALSE 
///          V2 = 1u    ; Unsigned number, C syntax
///          V3 = 0xFF  ; hex
///          V4 = {1, 2, 3}                 ; array of three variants
///          V5 = {"key1" : 1, "key2" : 2}  ; map of two key-value pairs
///          V6 = {"key2" : 'a', "key3" : {1, 2, 3}}  ; can be of any complexity
/// \endcode
/// The characters after the semicolon to the end of the line are ignored,
/// except in the case when semicolons are inside strings.
/// Obviously, if the strings are not enclosed in quotes, such string cannot have semicolons.
///
/// Blanks around the equal sign are not significant (unlike in many Windows implementations of ini file).
/// Leading and trailing blanks are always stripped down, including ones that precede the semicolon comment.
///
/// It is allowed to have multiple keys with the same name.
///
/// One cannot use the same object for both reading and writing simultaneously.
/// Whether the reading or writing is to be performed is specified in the
/// parameter to the Init service.
///
/// During the reading, the blank lines are skipped.
/// During writing one single blank line is added before each key.
/// The comments are not stored in the INI file, and because of it
/// the output file will not have them.
///
/// The errors that can happen during handling of INI file are of MException type.
///
class M_CLASS MIniFile : public MObject
{
public: // Types:

   /// Type of the current line that was read from the INI file
   ///
   enum LineType
   {
       LineEof        = 0, ///< End of file read, no more lines available
       LineKey        = 1, ///< The "[Key]" is read
       LineNameValue  = 2  ///< Name = value pair is read
   };

   /// Maximum input line length
   ///
   enum { MAX_LINE_LENGTH = 1024 };

public:

   /// Create a new INI file object, whether for reading from it, or for
   /// writing. The constructor does not specify the mode, and the object
   /// itself is useless until Init is called.
   ///
   MIniFile();

   /// Create and initialize a new INI file object.
   ///
   /// \pre There is only one reason for this service to fail, and that is the
   /// file fails to open for the access rights requested. Therefore,
   /// the file should exist and the user has to have appropriate rights.
   ///
   /// \param fileName
   ///     File name to open for either reading or writing
   ///     File can be absolute or relative path, file extension has to be present, no defaults are assumed.
   /// \param modeWrite
   ///     Whether this is for writing the ini file, otherwise read-only.
   ///
   /// \see Init - to open or reopen an ini file using the same object
   ///
   MIniFile(const MStdString& fileName, bool modeWrite = false);

   /// Object destructor.
   ///
   virtual ~MIniFile();

public: // properties:

   /// Initialize the INI file object to work on the file name specified.
   ///
   /// \pre There is only one reason for this service to fail, and that is the
   /// file fails to open for the access rights requested. Therefore,
   /// the file should exist and the user has to have appropriate rights.
   ///
   /// \param fileName
   ///     File name to open for either reading or writing
   ///     File can be absolute or relative path, file extension has to be present, no defaults are assumed.
   /// \param modeWrite
   ///     Whether this is for writing the ini file, otherwise read-only.
   ///
   void Init(const MStdString& fileName, bool modeWrite = false);

   /// Reinitialize the INI file for a repeat operation.
   ///
   /// When reading the ini file start reading from the beginning of the file,
   /// and when writing truncate the file to zero size.
   ///
   void ReInit();

   /// Frees the resources not needed after the object is being used.
   ///
   void Done();

   /// Read the next record from the file.
   ///
   /// Skip empty lines.
   /// Note that the keys are also read through this service.
   ///
   /// \pre The file is open for reading successfully, the syntax is correct,
   /// and no IO error happened. Otherwise an exception is raised.
   ///
   /// \return an indication of what happened:
   ///   - LineEof = 0 - End of file read, no more lines available
   ///   - LineKey = 1 - The "[Key]" is read
   ///   - LineNameValue = 2 - Name = value pair is read
   ///
   LineType ReadLine();

   /// Returns the current key, as read.
   ///
   /// Note that if the current line is the name-value line, the key returned
   /// is the nearest key above, or an empty string if there was no key read.
   ///
   /// \return String representation of the key, can be empty
   ///
   const MStdString& GetKey() const
   {
       return m_key;
   }

   /// Returns the current name, as read.
   ///
   /// Note that if the current line is the key line, the name returned
   /// relates to the previous name=value pair.
   ///
   /// \return String representation of the name in the name=value pair, can be empty
   ///
   const MStdString& GetName() const
   {
       return m_name;
   }

   /// Returns the current value, as read.
   ///
   /// Note that if the current line is the key line, the value returned
   /// relates to the previous name=value pair.
   ///
   /// \return Variant of any type, whatever is given in the name=value pair, can be an empty variant or an empty string
   ///
   const MVariant& GetValue() const
   {
       return m_value;
   }

   /// Returns the current value as string.
   ///
   /// Note that if the current line is the key line, the value returned
   /// relates to the previous name=value pair.
   ///
   /// \return String representation of the value
   ///
   MStdString GetStringValue() const;

   /// Get file name of the INI file. Full path is returned.
   ///
   /// \pre The file is successfully open. Otherwise an empty string is returned.
   ///
   MStdString GetFileName() const
   {
      return m_fileNameAndLineNumber.GetFileName();
   }

   /// Get the current line number.
   ///
   /// \pre The file is open successfully, and at least one line is read.
   /// Otherwise the value returned is zero.
   ///
   int GetFileLineNumber() const
   {
      return m_fileNameAndLineNumber.GetFileLineNumber();
   }

   ///@{
   /// When reading ini, this option controls expected value types
   ///
   /// This option allows handling some special ini file cases such a having 
   /// values that are file names, possibly with back slashes, that are not 
   /// enclosed in quotes.
   ///
   /// \default_value False
   ///
   /// \possible_values
   ///   - False - Prefer string data types for values not enclosed by quotes
   ///   - True - Attempt to interpret values as constants
   ///
   bool GetRespectValueType() const
   {
      return m_respectValueType;
   }
   void SetRespectValueType(bool yes)
   {
      m_respectValueType = yes;
   }
   ///@}

   /// Write the key string into the file.
   ///
   /// The file is added with [key] where key is the value of the given string, without quotes.
   ///
   /// \pre This operation is only valid if the file is open for writing.
   /// If the file was open for reading, an IO error is raised through MException.
   ///
   /// \param key The key string, shall have no ']' in it or an exception is thrown
   ///
   void WriteKey(const MStdString& key);

   /// Write the name-equals-value string into the file.
   ///
   /// \pre This operation is only valid if the file is open for writing.
   /// If the file was open for reading, an IO error is raised through MException.
   ///
   void WriteNameValue(const MStdString& name, const MVariant& value);

   /// Throws an exception and use current file name and line number associated with the file
   ///
   /// \param errorMessage Error message to throw
   ///
   void ThrowError(const MStdString& errorMessage);

   /// Throws an internationalized exception and uses the current file name and 
   /// line number associated with the file.
   ///
   /// \param errorMessage Error message to throw
   ///
   void Throw(MConstLocalChars errorMessage);

private: // Attributes:

   // File associated with the INI file object
   //
   MStreamFile m_file;

   // Current key
   //
   MStdString m_key;

   // Current name
   //
   MStdString m_name;

   // Current value, if present for such name
   //
   MVariant m_value;

   // File name of this INI file object
   //
   MStdString m_fileName;

   // Current line number
   //
   MFileNameAndLineNumber m_fileNameAndLineNumber;

   // Whether the INI file is used for writing the data
   //
   bool m_modeWrite;

   // Whether the values in the INI are strict constants
   //
   bool m_respectValueType;

   M_DECLARE_CLASS(IniFile)
};

#endif // !M_NO_FILESYSTEM

///@}
#endif
