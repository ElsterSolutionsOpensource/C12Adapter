#ifndef MCORE_MFILENAMEANDLINENUMBER_H
#define MCORE_MFILENAMEANDLINENUMBER_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MFileNameAndLineNumber.h

#include <MCORE/MCOREDefs.h>

#if !M_NO_VERBOSE_ERROR_INFORMATION

/// Memory saving fast copied storage for file name and file line.
/// Since the number of file names with which the program works is limited,
/// Internally, a table is used to store them in one place.
///
class M_CLASS MFileNameAndLineNumber
{
private: // Constants:

   enum
   {
      FILENAMEANDLINENUMBER_NAME_SHIFT      = 20,         // Shift to use in a mask to store the file name index
      FILENAMEANDLINENUMBER_LINE_MASK       = 0x000FFFFF, // File line mask
      FILENAMEANDLINENUMBER_FILE_MAX_INDEX  = 0x00000FFF  // File name maximum index
   };

public: // Types:

   /// Type that represents an array of files and lines, or a stack.
   ///
   typedef std::vector<MFileNameAndLineNumber>
      VectorType;

public: // Constructors:

   /// Class constructor that initializes the object with zero file name and line number
   ///
   MFileNameAndLineNumber()
   :
      m_data(0)
   {
   }

   /// Class constructor that initializes the object from a copy object.
   ///
   MFileNameAndLineNumber(const MFileNameAndLineNumber& other)
   :
      m_data(other.m_data)
   {
   }

   /// Class constructor that initializes the object from the internal data, as returned by GetData.
   ///
   explicit MFileNameAndLineNumber(Muint32 data)
   :
      m_data(data)
   {
   }

   /// Most useful constructor that creates file name and line number information.
   ///
   MFileNameAndLineNumber(const MStdString& fileName, unsigned lineNumber)
   {
      Set(fileName, lineNumber);
   }

   /// Assignment operator that makes this file and line as of the given argument.
   ///
   MFileNameAndLineNumber& operator=(const MFileNameAndLineNumber& other)
   {
      if ( this != &other )
         m_data = other.m_data;
      return *this;
   }
   
   /// Equality operator that tests whether the two files and line data are the same.
   ///
   bool operator==(const MFileNameAndLineNumber& other) const
   {
      return m_data == other.m_data;
   }
   
   /// Inequality operator that tests whether the two files and line data are the same.
   ///
   bool operator!=(const MFileNameAndLineNumber& other) const
   {
      return m_data != other.m_data;
   }
   
   /// Setter, that initializes data for the whole class.
   ///
   /// \pre The line value shall be smaller than (1 << FILENAMEANDLINENUMBER_NAME_SHIFT)
   /// Or it will not be stored correctly.
   ///
   void Set(const MStdString& fileName, unsigned lineNumber) M_NO_THROW;

   /// Checks whether there is no file and line information in an object
   ///
   bool IsEmpty() const
   {
      return m_data == 0u;
   }

   /// Clear file name and line number information.
   ///
   void Clear()
   {
      m_data = 0u;
   }

   /// Get file name associated with the object.
   ///
   MStdString GetFileName() const M_NO_THROW;

   /// Internal service that returns file name index in the internal array.
   /// This can be convenient for comparison of files for equality.
   ///
   unsigned GetFileNameIndex() const
   {
      return m_data >> FILENAMEANDLINENUMBER_NAME_SHIFT;
   }

   /// Return the internal representaion of a class.
   ///
   Muint32 GetData() const
   {
      return m_data;
   }

   /// Get file line, or zero if there is no information.
   ///
   unsigned GetFileLineNumber() const
   {
      unsigned result = m_data & (Muint32)FILENAMEANDLINENUMBER_LINE_MASK;
      return result == (Muint32)FILENAMEANDLINENUMBER_LINE_MASK ? 0 : result;
   }

   /// Increment line information by one line.
   ///
   void operator++()
   {
      if ( (FILENAMEANDLINENUMBER_LINE_MASK & m_data) != FILENAMEANDLINENUMBER_LINE_MASK ) // if we are still having room for numbers, increment
         ++m_data;
   }

   /// Increase line number by a given count
   ///
   /// \param i increase
   ///
   void operator+=(unsigned i)
   {
      unsigned value = (FILENAMEANDLINENUMBER_LINE_MASK & m_data) + i; // if we are still having room for numbers, increment
      if ( value > FILENAMEANDLINENUMBER_LINE_MASK )
         value = FILENAMEANDLINENUMBER_LINE_MASK;
      m_data += value;
   }

   /// True if the given file and line information refers to a different line within the same file.
   ///
   /// This is a handy way of detecting if there is a duplicate definition of the same entry.
   /// Notice if the entity is defined in the same line this check will be a false positive.
   ///
   /// \param other Another file and name information to check.
   /// \return True if the given entity is in the same file, but the line is different.
   ///
   bool IsSameFileDifferentLine(const MFileNameAndLineNumber& other) const;

public: // Global services:

   /// Clear all file and line information within a class.
   ///
   /// All memory is freed, No file and line information will be available after
   ///
   static void Uninitialize() M_NO_THROW;

private: // Private data:

   // Mask that holds the file name index and line number
   //
   Muint32 m_data;
};

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

///@}
#endif
