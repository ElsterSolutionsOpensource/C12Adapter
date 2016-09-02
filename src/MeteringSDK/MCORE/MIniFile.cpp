// File MCORE/MIniFile.h

#include "MCOREExtern.h"
#include "MCORE.h"
#include "MIniFile.h"

#if !M_NO_FILESYSTEM

   #if !M_NO_REFLECTION
      static MIniFile* DoNew2(const MStdString& fileName, bool modeWrite)
      {
         return M_NEW MIniFile(fileName, modeWrite);
      }
   #endif

M_START_PROPERTIES(IniFile)
   M_CLASS_ENUMERATION               (IniFile, LineEof)
   M_CLASS_ENUMERATION               (IniFile, LineKey)
   M_CLASS_ENUMERATION               (IniFile, LineNameValue)
   M_OBJECT_PROPERTY_READONLY_STRING (IniFile, Key,                ST_constMStdStringA_X)
   M_OBJECT_PROPERTY_READONLY_STRING (IniFile, Name,               ST_constMStdStringA_X)
   M_OBJECT_PROPERTY_READONLY_VARIANT(IniFile, Value,              ST_constMVariantA_X)
   M_OBJECT_PROPERTY_READONLY_STRING (IniFile, FileName,           ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_INT    (IniFile, FileLineNumber)
   M_OBJECT_PROPERTY_BOOL            (IniFile, RespectValueType)
M_START_METHODS(IniFile)
   M_OBJECT_SERVICE                  (IniFile, ReadLine,           ST_int_X)
   M_OBJECT_SERVICE                  (IniFile, WriteKey,           ST_X_constMStdStringA)
   M_OBJECT_SERVICE                  (IniFile, WriteNameValue,     ST_X_constMStdStringA_constMVariantA)
   M_CLASS_FRIEND_SERVICE            (IniFile, New, DoNew2,        ST_MObjectP_S_constMStdStringA_bool)
   M_OBJECT_SERVICE                  (IniFile, Done,               ST_X)
   M_OBJECT_SERVICE                  (IniFile, ThrowError,         ST_X_constMStdStringA)
M_END_CLASS(IniFile, Object)

MIniFile::MIniFile()
:
   m_file(),
   m_fileNameAndLineNumber(),
   m_modeWrite(false),
   m_respectValueType(false)
{
}

MIniFile::MIniFile(const MStdString& file, bool modeWrite)
:
   m_file(),
   m_respectValueType(false)
{
   Init(file, modeWrite);
}

MIniFile::~MIniFile()
{
   Done();
}

void MIniFile::Init(const MStdString& file, bool modeWrite)
{
   Done();
   m_modeWrite = modeWrite;
   MStdString fileName = MUtilities::GetFullPath(file);
   if ( fileName.empty() )
      fileName = file;
   m_fileNameAndLineNumber.Set(fileName, 0);
   unsigned flags = modeWrite
                  ? (MStreamFile::FlagText | MStreamFile::FlagBuffered | MStream::FlagWriteOnly | MStreamFile::FlagCreate | MStreamFile::FlagTruncate)
                  : (MStreamFile::FlagText | MStreamFile::FlagBuffered | MStreamFile::FlagReadOnly);
   m_file.Open(fileName, flags);
}

void MIniFile::ReInit()
{
   // All these operations throw errors when the file is not open
   if ( m_modeWrite )
      m_file.SetSize(0);
   else
      m_file.SetPosition(0);
   m_fileNameAndLineNumber.Set(m_file.GetName(), 0);
}

void MIniFile::Done()
{
   m_file.Close();
   m_value.SetEmpty();
   m_key.clear();
   m_name.clear();
   m_fileNameAndLineNumber.Clear();
}

MIniFile::LineType MIniFile::ReadLine()
{
   try
   {
      MStdString line;
      do
      {
         ++ m_fileNameAndLineNumber;
         MVariant result = m_file.ReadLine();
         if ( result.IsEmpty() )
            return LineEof;
         line = result.AsString();

         // Scan the line for comment respecting the string boundaries
         char stringBoundary = '\0'; // no string boundary at the beginning of the scan
         MStdString::const_iterator it = line.begin();
         MStdString::const_iterator itEnd = line.end();
         for ( ; it != itEnd; ++it )
         {
            char c = *it;
            if ( c == '\"' || c == '\'' )
            {
               if ( stringBoundary == '\0' ) // string starts
                  stringBoundary = c;
               else if ( stringBoundary == c ) // string ends
                  stringBoundary = '\0';
            }
            else if ( c == '\\' )
            {
               ++it; // skip one, presumably '\"' or '\''
               if ( it == itEnd ) // we do not support adding next line, instead just bail out
                  break;
            }
            else if ( c == ';' && stringBoundary == '\0' ) // semicolon outside string boundary
            {
               line.resize(it - line.begin());
               break;
            }
         }
         MAlgorithm::InplaceTrim(line);
      }
      while ( line.empty() );

      if ( *line.begin() == '[' ) // key is read
      {
         if ( *line.rbegin() != ']' )
         {
            Throw(M_I("']' not found"));
            M_ENSURED_ASSERT(0);
         }
         m_key.assign(line.begin() + 1, line.end() - 1); // skip []
         MAlgorithm::InplaceTrim(m_key);
         return LineKey;
      }

      // otherwise this is name=value
      size_t equalSign = line.find('=');
      if ( equalSign == line.npos )
      {
         Throw(M_I("'=' not found"));
         M_ENSURED_ASSERT(0);
      }
      m_name.assign(line.begin(), line.begin() + equalSign); // make name
      MAlgorithm::InplaceTrimRight(m_name);
      line.erase(0, equalSign + 1); // now line is the data portion
      MAlgorithm::InplaceTrim(line);
      if ( line.empty() )
         m_value.SetToNull(MVariant::VAR_STRING); // by convention (see below, use EMPTY for empty variant)
      else if ( m_respectValueType )
         m_value = MUtilities::FromMDLConstant(line);
      else
      {
         char c = *line.begin();
         if ( c == '\"' || c == '\'' || c == '[' || c == '{' )
            m_value = MUtilities::FromMDLConstant(line);
         else if ( line == "EMPTY" ) // this one has to be handled differently, for others the line assignment will work
            m_value.SetEmpty();
         else
            m_value = line;
      }
   }
   catch ( MException& ex )
   {
      ex.UpdateFileNameAndLineNumber(m_fileNameAndLineNumber);
      throw;
   }
   return LineNameValue;
}

MStdString MIniFile::GetStringValue() const
{
   MStdString result;
   if ( !m_value.IsEmpty() )
      result = m_value.AsString();
   return result;
}

void MIniFile::WriteKey(const MStdString& key)
{
   if ( key.find_first_of(";]") != MStdString::npos )
   {
      Throw(M_I("Key cannot have ';' or ']'"));
      M_ENSURED_ASSERT(0);
   }
   if ( m_fileNameAndLineNumber.GetFileLineNumber() == 0 ) // no writes to a file
      m_file.WriteChar('[');
   else // otherwise
      m_file.WriteBytes("\n[", 2);
   m_file.Write(MAlgorithm::TrimString(key));
   m_file.WriteBytes("]\n", 2);
   m_key = key;
   m_fileNameAndLineNumber += 2; // we've added two lines
}

void MIniFile::WriteNameValue(const MStdString& name, const MVariant& value)
{
   if ( name.find_first_of(";=") != MStdString::npos )
   {
      Throw(M_I("Name cannot have ';' or '='"));
      M_ENSURED_ASSERT(0);
   }
   m_file.Write(MAlgorithm::TrimString(name));
   m_file.WriteChar('=');
   m_file.Write(MUtilities::ToRelaxedMDLConstant(value));
   m_file.WriteChar('\n');
   m_name = name;
   m_value = value;
   ++m_fileNameAndLineNumber;
}

void MIniFile::ThrowError(const MStdString& errorMessage)
{
   MException ex;
   ex.SetFileNameAndLineNumber(m_fileNameAndLineNumber);
   ex.InitAll(MException::ErrorConfiguration, MErrorEnum::BadFileFormat, errorMessage);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

void MIniFile::Throw(MConstLocalChars errorMessage)
{
   MException ex;
   ex.SetFileNameAndLineNumber(m_fileNameAndLineNumber);
   ex.Init(MException::ErrorConfiguration, MErrorEnum::BadFileFormat, errorMessage);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

#endif // !M_NO_FILESYSTEM
