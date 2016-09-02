// File MCORE/MFileNameAndLineNumber.cpp

#include "MCOREExtern.h"
#include "MFileNameAndLineNumber.h"
#include "MCriticalSection.h"
#include "MMath.h"

#if !M_NO_VERBOSE_ERROR_INFORMATION

   static MCriticalSection s_fileNamesGuard;
   static MStdStringVector s_fileNames;

void MFileNameAndLineNumber::Set(const MStdString& fileName, unsigned lineNumber) M_NO_THROW
{
   MCriticalSection::Locker lock(s_fileNamesGuard);
   try
   {
      MStdStringVector::const_iterator itBegin = s_fileNames.begin();
      MStdStringVector::const_iterator itEnd = s_fileNames.end();
      MStdStringVector::const_iterator it = std::find(itBegin, itEnd, fileName);
      MStdStringVector::difference_type pos = it - itBegin + 1;
      if ( it == itEnd )
      {
         if ( pos < FILENAMEANDLINENUMBER_FILE_MAX_INDEX )
            s_fileNames.push_back(fileName);
         else
         {
            M_ASSERT(pos == FILENAMEANDLINENUMBER_FILE_MAX_INDEX);

            const int barrier = FILENAMEANDLINENUMBER_FILE_MAX_INDEX / 2;
            MStdStringVector::difference_type start = MMath::RandomInRange(0, barrier - 1);
            MStdStringVector::difference_type it = start;
            MStdStringVector::difference_type end = it + barrier;
            for ( ; ; ++it )
            {
               if ( it == end ) // Worst case: no more room and all extensions are two-letter.
               {
                  pos = start;
                  break;
               }
               const MStdString& name = s_fileNames[it];
               const size_t nameSize = name.size();
               if ( nameSize < 4 || name[nameSize - 3] != '.' ) // prefer not to remove two-letter file names. Stop on the first. 
               {
                  pos = it;
                  break;
               }
            }
            s_fileNames[pos] = fileName;
         }
      }
      m_data = (lineNumber & (Muint32)FILENAMEANDLINENUMBER_LINE_MASK) 
             | (Muint32)(pos << FILENAMEANDLINENUMBER_NAME_SHIFT);
   }
   catch ( ... )
   {
      m_data = 0;
      M_ASSERT(0);
   }
}

MStdString MFileNameAndLineNumber::GetFileName() const M_NO_THROW
{
   unsigned index = m_data >> FILENAMEANDLINENUMBER_NAME_SHIFT;
   if ( index == 0 )
      return MStdString();

   MCriticalSection::Locker lock(s_fileNamesGuard);
   M_ASSERT(index <= s_fileNames.size());
   return s_fileNames[index - 1];   
}

void MFileNameAndLineNumber::Uninitialize() M_NO_THROW
{
   MCriticalSection::Locker lock(s_fileNamesGuard);
   s_fileNames.clear();
}

bool MFileNameAndLineNumber::IsSameFileDifferentLine(const MFileNameAndLineNumber& other) const
{
   return GetFileNameIndex() == other.GetFileNameIndex() && GetFileLineNumber() != other.GetFileLineNumber();
}

#endif // !M_NO_VERBOSE_ERROR_INFORMATION
