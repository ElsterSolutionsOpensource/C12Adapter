// File MCORE/MException.cpp

#include "MCOREExtern.h"
#include "MException.h"
#include "MRegexp.h"
#include "MStr.h"
#include "MUtilities.h"
#include "MAlgorithm.h"

#if (M_OS & M_OS_POSIX) != 0
   #include <fenv.h>
#endif

M_START_PROPERTIES(Exception)
   M_OBJECT_PROPERTY_UINT                      (Exception, Code)
   M_OBJECT_PROPERTY_STRING                    (Exception, MessageString,      ST_MStdString_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_READONLY_STRING_EXACT     (Exception, AsString,           ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_STRING_EXACT     (Exception, AsSimplifiedString, ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_STRING           (Exception, FileName,           ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_UINT             (Exception, FileLineNumber)
   M_OBJECT_PROPERTY_UINT                      (Exception, Kind)
   M_OBJECT_PROPERTY_READONLY_STRING           (Exception, KindAsString,       ST_MStdString_X)
   M_OBJECT_PROPERTY_READONLY_STRING_COLLECTION(Exception, CallStack,          ST_MStdStringVector_X)

// Enumerations:
   M_CLASS_ENUMERATION                         (Exception, ErrorNone)
   M_CLASS_ENUMERATION                         (Exception, ErrorInformation)
   M_CLASS_ENUMERATION                         (Exception, ErrorWarning)
   M_CLASS_ENUMERATION                         (Exception, Error)
   M_CLASS_ENUMERATION                         (Exception, ErrorFatal)
   M_CLASS_ENUMERATION                         (Exception, ErrorCommunication)
   M_CLASS_ENUMERATION                         (Exception, ErrorSystem)
   M_CLASS_ENUMERATION                         (Exception, ErrorSocket)
   M_CLASS_ENUMERATION                         (Exception, ErrorConfiguration)
   M_CLASS_ENUMERATION                         (Exception, ErrorSecurity)
   M_CLASS_ENUMERATION                         (Exception, ErrorMeter)
   M_CLASS_ENUMERATION                         (Exception, ErrorSoftware)
M_START_METHODS(Exception)
   M_CLASS_SERVICE                             (Exception, New,                   ST_MObjectP_S)
   M_OBJECT_SERVICE                            (Exception, Append,                ST_X_constMStdStringA)
   M_OBJECT_SERVICE                            (Exception, Prepend,               ST_X_constMStdStringA)
   M_OBJECT_SERVICE                            (Exception, NewClone,              ST_MObjectP_X)
   M_OBJECT_SERVICE                            (Exception, Rethrow,               ST_X)
   M_OBJECT_SERVICE                            (Exception, UpdateFileAndLine,     ST_X_constMStdStringA_unsigned)
   M_OBJECT_SERVICE                            (Exception, AddFileAndLineToStack, ST_X_constMStdStringA_unsigned)
   M_CLASS_SERVICE                             (Exception, GetKindAsStringStatic, ST_MStdString_S_int)
   M_CLASS_SERVICE                             (Exception, VisualizeParameter,    ST_MStdString_S_constMVariantA)
   M_CLASS_SERVICE                             (Exception, SimplifyMessageString, ST_MStdString_S_constMStdStringA)
M_END_CLASS(Exception, Object)

#if !M_NO_VERBOSE_ERROR_INFORMATION
   const char MException::s_itemIsUnknownErrorString[] = "Item '%s' is unknown";
#endif

MException::MException()
:
   m_code(MErrorEnum::Unknown),
   m_kind(Error)
#if !M_NO_VERBOSE_ERROR_INFORMATION
   , m_message()
   , m_fileNameAndLineNumber()
   , m_stack()
#endif
{
}

#if !M_NO_VERBOSE_ERROR_INFORMATION
MException::MException(const MStdString& message, MErrorEnum::Type code, KindType kind)
:
   m_code(code),
   m_kind(kind),
   m_message(message),
   m_fileNameAndLineNumber(),
   m_stack()
{
}
#endif

MException::MException(const MException& ex)
:
   m_code(ex.m_code),
   m_kind(ex.m_kind)
#if !M_NO_VERBOSE_ERROR_INFORMATION
   , m_message(ex.m_message)
   , m_fileNameAndLineNumber(ex.m_fileNameAndLineNumber)
   , m_stack(ex.m_stack)
#endif
{
}

MException::~MException() M_NO_THROW
{
}

MException& MException::operator=(const MException& ex)
{
   if ( this != &ex )
   {
      m_code = ex.m_code;
      m_kind = ex.m_kind;
#if !M_NO_VERBOSE_ERROR_INFORMATION
      m_message = ex.m_message;
      m_fileNameAndLineNumber = ex.m_fileNameAndLineNumber;
      m_stack = ex.m_stack;
#endif
   }
   return *this;
}

MException* MException::New()
{
   return M_NEW MException;
}

#if !M_NO_VERBOSE_ERROR_INFORMATION

void MException::InitAll(MException::KindType kind, MErrorEnum::Type code, const MStdString& message) M_NO_THROW
{
   m_kind = kind;
   m_code = code;
   m_message = message;
   m_stack.clear();
}

void MException::InitVA(MException::KindType kind, MErrorEnum::Type code, MConstLocalChars str, va_list va) M_NO_THROW
{
   InitAll(kind, code, MGetStdStringVA(str, va));
}

void MException::InitVA(MException::KindType kind, MErrorEnum::Type code, const char* str, va_list va) M_NO_THROW
{
   InitAll(kind, code, MGetStdStringVA(str, va));
}

void MException::InitVA(MErrorEnum::Type code, MConstLocalChars str, va_list va) M_NO_THROW
{
   InitVA(MException::Error, code, str, va);
}

void MException::InitVA(MErrorEnum::Type code, const char* str, va_list va) M_NO_THROW
{
   InitVA(MException::Error, code, str, va);
}

void MException::Init(KindType kind, MErrorEnum::Type code, MConstLocalChars str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   InitVA(kind, code, str, va);
   va_end(va);
}

void MException::Init(KindType kind, MErrorEnum::Type code, const char* str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   InitVA(kind, code, str, va);
   va_end(va);
}

void MException::Init(MErrorEnum::Type code, MConstLocalChars str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   InitVA(code, str, va);
   va_end(va);
}

void MException::Init(MErrorEnum::Type code, const char* str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   InitVA(code, str, va);
   va_end(va);
}

M_NORETURN_FUNC void MException::Throw(MErrorEnum::Type code, MConstLocalChars str, ...)
{
   va_list va;
   va_start(va, str);
   MException ex;
   ex.InitVA(code, str, va);
   va_end(va);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::Throw(MErrorEnum::Type code, const char* str, ...)
{
   va_list va;
   va_start(va, str);
   MException ex;
   ex.InitVA(code, str, va);
   va_end(va);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::Throw(KindType kind, MErrorEnum::Type code, MConstLocalChars str, ...)
{
   va_list va;
   va_start(va, str);
   MException ex;
   ex.InitVA(kind, code, str, va);
   va_end(va);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::Throw(KindType kind, MErrorEnum::Type code, const char* str, ...)
{
   va_list va;
   va_start(va, str);
   MException ex;
   ex.InitVA(kind, code, str, va);
   va_end(va);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::Throw(const MStdString& message, MErrorEnum::Type code)
{
   MException(message, code).Rethrow();
   M_ENSURED_ASSERT(0);
}

#else // !M_NO_VERBOSE_ERROR_INFORMATION

M_NORETURN_FUNC void MException::Throw(MErrorEnum::Type code)
{
   MException ex;
   ex.Init(code);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::Throw(KindType kind, MErrorEnum::Type code)
{
   MException ex;
   ex.InitAll(kind, code);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

M_NORETURN_FUNC void MException::ThrowUnknownError()
{
   MException::Throw(M_CODE_STR(MErrorEnum::Unknown, M_I("Unknown error")));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowUnknownItem(MException::KindType kind, MConstChars name)
{
   MException::Throw(kind, M_CODE_STR_P1(MErrorEnum::UnknownItem, s_itemIsUnknownErrorString, name));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowUnknownItem(MConstChars name)
{
   ThrowUnknownItem(ErrorSoftware, name);
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowUnknownItem(MException::KindType kind, const MStdString& name)
{
   MException::Throw(kind, M_CODE_STR_P1(MErrorEnum::UnknownItem, s_itemIsUnknownErrorString, name.c_str()));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowUnknownItem(const MStdString& name)
{
   ThrowUnknownItem(ErrorSoftware, name);
   M_ENSURED_ASSERT(0);
}

#if !M_NO_VARIANT
M_NORETURN_FUNC void MException::ThrowUnsupportedType(int typetag)
{
   MException::Throw(M_CODE_STR_P1(MErrorEnum::UnsupportedType, "Unsupported type %d", typetag));
}
#endif

M_NORETURN_FUNC void MException::ThrowNotSupportedForThisType()
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_OPERATION_NOT_SUPPORTED_FOR_THIS_TYPE, M_I("Operation not supported for this type")));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowCannotIndexItem(MConstChars itemName)
{
   Throw(ErrorSoftware, M_CODE_STR_P1(MErrorEnum::CannotIndexItem, M_I("Item '%s' is not an array or set, and cannot be indexed"), (itemName != NULL) ? itemName : "?"));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowDivisionByZero()
{
   Throw(ErrorSoftware, M_CODE_STR(M_ERR_DIVISION_BY_ZERO, M_I("Division by zero")));
}

M_NORETURN_FUNC void MException::ThrowCallOutOfSequence()
{
   Throw(ErrorSoftware, M_CODE_STR(M_ERR_OUT_OF_SEQUENCE, "Call is made out of sequence"));
}

M_NORETURN_FUNC void MException::ThrowNoValue()
{
   MException ex;
   ex.Init(ErrorSoftware, M_CODE_STR(MErrorEnum::NoValue, M_I("No value exists")));
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowBadTimeValue(MConstChars str)
{
#if !M_NO_VERBOSE_ERROR_INFORMATION
   if ( str == NULL || str[0] == '\0' )
      Throw(M_ERR_BAD_TIME_VALUE, M_I("Time value is bad"));
   else
      Throw(M_ERR_BAD_TIME_VALUE, M_I("Cannot cteare time from '%s'"), str);
#else
   M_ASSERT(str == NULL);
   Throw(M_ERR_BAD_TIME_VALUE);
#endif
}

#if !M_NO_VERBOSE_ERROR_INFORMATION
M_NORETURN_FUNC void MException::ThrowNoValue(MFileNameAndLineNumber fl)
{
   MException ex;
   ex.Init(ErrorSoftware, MErrorEnum::NoValue, M_I("No value exists"));
   ex.SetFileNameAndLineNumber(fl);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}
#endif

#if !M_NO_FILESYSTEM
M_NORETURN_FUNC void MException::ThrowBadFileFormat(const MStdString& fileName)
{
   Throw(MErrorEnum::BadFileFormat, M_I("File '%s' has bad format or it is corrupt"), fileName.c_str());
   M_ENSURED_ASSERT(0);
}
#endif

#if !M_NO_VARIANT
M_NORETURN_FUNC void MException::ThrowSyntaxError(MConstChars offendingString)
{
   ThrowSyntaxError(offendingString, strlen(offendingString));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowSyntaxError(MConstChars offendingString, size_t len)
{
   char buff [ MException::MaximumVisibleParameterLength ];
   Throw(MException::ErrorSoftware, M_ERR_SYNTAX_ERROR_IN_S1, M_I("Syntax error in '%s'"), VisualizeCharsParameter(buff, offendingString, static_cast<unsigned>(len)));
   M_ENSURED_ASSERT(0);
}
#endif // !M_NO_VARIANT

M_NORETURN_FUNC void MException::ThrowUnexpectedChar(int ch)
{
   Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_UNEXPECTED_CHARACTER_C1,
                                                  ((ch > ' ' && ch <= '~')
                                                  ? "Unexpected character '%c'"
                                                  : "Unexpected character with code 0x%X"), ch));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MException::ThrowStringTooLong(int stringLength, int maximumPossibleLength)
{
   Throw(M_CODE_STR_P2(MErrorEnum::StringTooLong, M_I("String of %d characters is too long to fit within %d characters"), stringLength, maximumPossibleLength));
   M_ENSURED_ASSERT(0);
}

#if !M_NO_VERBOSE_ERROR_INFORMATION
void MException::AddFileNameAndLineNumberToStack(MFileNameAndLineNumber fl) M_NO_THROW
{
   if ( !fl.IsEmpty() )
   {
      try
      {
         if ( m_fileNameAndLineNumber.IsEmpty() )
            m_fileNameAndLineNumber = fl;
         else if ( m_fileNameAndLineNumber != fl && std::find(m_stack.rbegin(), m_stack.rend(), fl) == m_stack.rend() )
            m_stack.push_back(fl);
      }
      catch ( ... )
      {
         M_ASSERT(0);
      }
   }
}

void MException::UpdateFileAndLine(const MStdString& file, unsigned line) M_NO_THROW
{
   try
   {
      UpdateFileNameAndLineNumber(MFileNameAndLineNumber(file, line));
   }
   catch ( ... )
   {
      M_ASSERT(0);
   }
}

void MException::AddFileAndLineToStack(const MStdString& file, unsigned line) M_NO_THROW
{
   try
   {
      AddFileNameAndLineNumberToStack(MFileNameAndLineNumber(file, line));
   }
   catch ( ... )
   {
      M_ASSERT(0);
   }
}

MStdStringVector MException::GetCallStack() const
{
   MStdStringVector result;
   try
   {
      MFileNameAndLineNumber::VectorType::const_iterator it = m_stack.begin();
      MFileNameAndLineNumber::VectorType::const_iterator itEnd = m_stack.end();
      for ( ; it != itEnd; ++it )
      {
         MStdString str = (*it).GetFileName();
         str += '(';
         str += MToStdString((*it).GetFileLineNumber());
         str += ')';
         result.push_back(str);
      }
   }
   catch ( ... )
   {
      M_ASSERT(0);
   }
   return result;
}

MStdString MException::AsString() const
{
   return m_message;
}

MStdString MException::SimplifyMessageString(const MStdString& message)
{
   MStdString result = message;
   try
   {
      // Remove possible upper component prefix
      //
      MStdString regexpString = M_I_STR(M_I("^(While doing [^:]+: )")); // build international string, do not make it static
      MRegexp re(regexpString);
      while ( re.Match(result) ) // the string can take place many times
      {  // Item 1 is the first substring (zero item stands for the whole string)
         M_ASSERT(re.GetItemStart(1) == 0);
         result.erase((size_t)0, re.GetItemLength(1));
      }

      // Remove possible MCOM postfix
      //
      regexpString = MGetStdString(M_I("( in %s)$"), "[A-Za-z0-9_]+(\\([^)]+\\))?");  // build international string, do not make it static
      re.Compile(regexpString);
      while ( re.Match(result) ) // There could be multiple addendums to string
      {  // Item 1 is the first substring
         unsigned start = re.GetItemStart(1);
         result.erase(start, result.size() - start);
      }
   }
   catch ( MException& )
   {
      M_ASSERT(0); // notify on debug version only
   }
   return result;
}


MStdString MException::AsSimplifiedString() const
{
   return SimplifyMessageString(m_message);
}

MStdString MException::GetKindAsString() const
{
   return GetKindAsStringStatic(GetKind());
}

MStdString MException::GetKindAsStringStatic(KindType kind)
{
   MConstLocalChars str;
   switch ( kind )
   {
   case ErrorNone:            str = M_I("OK");                   break;
   case ErrorInformation:     str = M_I("Information");          break;
   case ErrorWarning:         str = M_I("Warning");              break;
   case Error:                str = M_I("Error");                break;
   case ErrorFatal:           str = M_I("Fatal error");          break;
   case ErrorCommunication:   str = M_I("Communication error");  break;
   case ErrorSystem:          str = M_I("System error");         break;
   case ErrorSocket:          str = M_I("Socket error");         break;
   case ErrorConfiguration:   str = M_I("Configuration error");  break;
   case ErrorSecurity:        str = M_I("Security error");       break;
   case ErrorMeter:           str = M_I("Error in the meter");   break;
   case ErrorSoftware:        str = M_I("Software error");       break;
   default:                   str = M_I("Application error %d"); break;
   }
   return MGetStdString(str, kind);
}

void MException::Append(const MStdString& str) M_NO_THROW
{
   m_message += str;
}

void MException::AppendToString(MConstLocalChars str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   m_message += MGetStdStringVA(str, va);
   va_end(va);
}

void MException::AppendToString(const char* str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   m_message += MGetStdStringVA(str, va);
   va_end(va);
}

void MException::Prepend(const MStdString& str) M_NO_THROW
{
   m_message = str + m_message;
}

void MException::PrependBeforeString(MConstLocalChars str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   m_message = MGetStdStringVA(str, va) + m_message;
   va_end(va);
}

void MException::PrependBeforeString(const char* str, ...) M_NO_THROW
{
   va_list va;
   va_start(va, str);
   m_message = MGetStdStringVA(str, va) + m_message;
   va_end(va);
}

MStdString MException::VisualizeParameter(const MVariant& v) M_NO_THROW
{
   MStdString result;
   char buff [ MException::MaximumVisibleParameterLength ];
   result = VisualizeVariantParameter(buff, v);
   return result;
}

   // Make this one invisible to the client so one does not mistakely use it in applications for buffer size
   //
   static const size_t MAXIMUM_PARAM_LENGTH_BEFORE_TRUNCATION = MException::MaximumVisibleParameterLength - 8;

char* MException::VisualizeVariantParameter(char* buff, const MVariant& v) M_NO_THROW
{
   try
   {
      MStdString result = MUtilities::ToMDLConstant(v);
      M_ASSERT(!result.empty()); // this would be impossible

      size_t len = result.size();
      const char* start = result.data();
      if ( (*start == '\'' || *start == '"') ) // truncate quotes if present
      {
         M_ASSERT(len >= 2); // this is how ToMDLConstant will always work - syntactically it is impossible to have just one quote
         M_ASSERT(*start == *(start + len - 1));
         ++start;
         len -= 2;
      }

      if ( len <= MAXIMUM_PARAM_LENGTH_BEFORE_TRUNCATION ) // truncate for better error message
         memcpy(buff, start, len);
      else
      {
         len = MAXIMUM_PARAM_LENGTH_BEFORE_TRUNCATION;
         memcpy(buff, start, len);
         memcpy(buff + len, " ...", 4);
         len += 4;
      }
      buff[len] = '\0';
      M_ASSERT(len < MaximumVisibleParameterLength);
   }
   catch ( ... )
   {
      M_ASSERT(0); // in reality no exceptions are possible in the code above
      buff[0] = '\0'; // recoved from hopefully impossible case of a bad conversion
   }
   return buff;
}

char* MException::VisualizeStringParameter(char* buff, const MStdString& str) M_NO_THROW
{
   try
   {
      VisualizeVariantParameter(buff, MVariant(str));
   }
   catch ( ... )
   {
      M_ASSERT(0); // in reality no exceptions are possible in the code above
      buff[0] = '\0'; // recoved from hopefully impossible case of a bad conversion
   }
   return buff;
}

char* MException::VisualizeCharsParameter(char* buff, const char* str) M_NO_THROW
{
   try
   {
      VisualizeVariantParameter(buff, MVariant(str));
   }
   catch ( ... )
   {
      M_ASSERT(0); // in reality no exceptions are possible in the code above
      buff[0] = '\0'; // recoved from hopefully impossible case of a bad conversion
   }
   return buff;
}

char* MException::VisualizeCharsParameter(char* buff, const char* str, unsigned len) M_NO_THROW
{
   try
   {
      VisualizeVariantParameter(buff, MVariant(str, len));
   }
   catch ( ... )
   {
      M_ASSERT(0); // in reality no exceptions are possible in the code above
      buff[0] = '\0'; // recoved from hopefully impossible case of a bad conversion
   }
   return buff;
}

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

MException* MException::NewClone() const
{
   return M_NEW MException(*this);
}

M_NORETURN_FUNC void MException::Rethrow()
{
   throw *this;
}

M_START_PROPERTIES(ESystemError)
   M_OBJECT_PROPERTY_READONLY_UINT        (ESystemError, SystemErrorCode)
M_START_METHODS(ESystemError)
   M_CLASS_SERVICE                        (ESystemError, New,                ST_MObjectP_S)
M_END_CLASS(ESystemError, Exception)

MESystemError::MESystemError()
:
   MException(),
   m_systemErrorCode()
#if (M_OS & M_OS_WINDOWS) != 0
   , m_isErrno(false)
#endif 
{
}

#if (M_OS & M_OS_WINDOWS) != 0
MESystemError::MESystemError(unsigned error, bool isErrno)
:
   MException(),
   m_systemErrorCode(error),
   m_isErrno(isErrno)
{
   MException::KindType kind = MException::ErrorSystem;
#if !M_NO_VERBOSE_ERROR_INFORMATION
   MStdString message;
   if ( error != 0 )
   {
      if ( error == ERROR_ACCESS_DENIED )
         kind = MException::ErrorSecurity;
      
      // make sure the below "error |= 0x8004000" does the same job as MAKE_HRESULT
      M_ASSERT((error | 0x80000000) == (unsigned)(MAKE_HRESULT(1, 0, error)));
      message = MessageFromSystemError(error, isErrno);
      if ( message.empty() )
         message = MGetStdString(M_I("System error %X"), error);
      error |= 0x80000000;
   }
   else
   {
      message = M_I_STR(M_I("Unknown system error"));
      error = (unsigned)MErrorEnum::UnknownSystemError;
   }
   InitAll(kind, (MErrorEnum::Type)error, message);
#else
   if ( error != 0 )
      error |= 0x80000000;
   else
      error = (unsigned)MErrorEnum::UnknownSystemError;
   Init(kind, (MErrorEnum::Type)error);
#endif
   ClearGlobalSystemError();
}
#else
MESystemError::MESystemError(unsigned error)
:
   MException(),
   m_systemErrorCode(error)
{
   MException::KindType kind = MException::ErrorSystem;
#if !M_NO_VERBOSE_ERROR_INFORMATION
   MStdString message;
   if ( error != 0 )
   {
      message = MessageFromSystemError(error);
      if ( message.empty() )
         message = MGetStdString(M_I("System error %X"), error);
      error |= 0x80000000;
   }
   else
   {
      message = M_I_STR(M_I("Unknown system error"));
      error = (unsigned)MErrorEnum::UnknownSystemError;
   }
   InitAll(kind, (MErrorEnum::Type)error, message);
#else
   if ( error != 0 )
      error |= 0x80000000;
   else
      error = (unsigned)MErrorEnum::UnknownSystemError;
   Init(kind, (MErrorEnum::Type)error);
#endif
   ClearGlobalSystemError();
}
#endif

MESystemError::MESystemError(const MESystemError& ex)
:
   MException(ex),
   m_systemErrorCode(ex.m_systemErrorCode)
#if (M_OS & M_OS_WINDOWS) != 0
   , m_isErrno(ex.m_isErrno)
#endif
{
}

MESystemError::~MESystemError() M_NO_THROW
{
}

#if !M_NO_VERBOSE_ERROR_INFORMATION

   // Cut the trailing carriage return, line feed, and period
   inline void DoStreamlineMessage(MStdString& message)
   {
      if ( !message.empty() )
      {
         MStdString::iterator itEnd = message.begin();
         MStdString::iterator itLast = message.end() - 1;
         MStdString::iterator it = itLast;
         for ( ; it > itEnd; --it )
            if ( *it != '\r' && *it != '\n' && *it != '.')
               break;
         if ( it < itLast )
            message.erase(it + 1, message.end());
      }
   }
      
#if M_OS & M_OS_WINDOWS
MStdString MESystemError::MessageFromSystemError(unsigned error, bool isErrno)
{
   MStdString message;
   if ( !isErrno )
   {
      LPTSTR messageBuffer;
      DWORD ret = ::FormatMessage((FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS),
                                  NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                                  (LPTSTR)&messageBuffer,
                                  0, NULL );
      if ( ret != 0 ) // if the information is present, append.
      {               // System messages can be stripped from kernel in a CE device, in which case we have nothing to add...
         M_ASSERT(messageBuffer);
         #if M_UNICODE
            message = MToStdString(messageBuffer);
         #else
            message = messageBuffer;
         #endif
         ::LocalFree(messageBuffer);
      }
   }
   else
   {
      #ifdef __BORLANDC__
         message = strerror(error);
         MAlgorithm::InplaceTrim(message);
      #else
         char buf [1024];
         message = strerror_s(buf, sizeof(buf), error);
      #endif
   }
   DoStreamlineMessage(message);
   return message;
}
#else
MStdString MESystemError::MessageFromSystemError(unsigned error)
{
   MStdString message;
   #if (M_OS & M_OS_NUTTX) != 0
      message = strerror(error);
   #elif (M_OS & M_OS_CMX) != 0
      message = "System error";
   #else
      char buf [ 1024 ];
      buf[0] = '\0';
      strerror_r(error, buf, sizeof(buf));
      if ( buf[0] == '\0' )
         message = strerror(error); // fallback
      else
         message = buf;
   #endif
   DoStreamlineMessage(message);
   return message;
}
#endif

M_NORETURN_FUNC void MESystemError::Throw(unsigned error, MConstChars prependMessage)
{
   MESystemError ex(error);
   if ( prependMessage != NULL )
   {
      MStdString msg = prependMessage;
      if ( error == 0 ) // unknown system error
         ex.SetMessageString(msg);
      else
      {
         msg += ". ";
         ex.Prepend(msg);
      }
   }
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

#else

M_NORETURN_FUNC void MESystemError::Throw(unsigned error)
{
   MESystemError ex(error);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}

#endif // !M_NO_VERBOSE_ERROR_INFORMATION

M_NORETURN_FUNC void MESystemError::ThrowLastSystemError()
{
#if M_OS & M_OS_WINDOWS
   unsigned code = GetLastGlobalSystemError();
   bool isErrno = false;
   if ( code == 0 )
   {
      code = errno;
      if ( code != 0 )
      {
         errno = 0; // clear errno so it does not stay on after reporting
         isErrno = true;
      }
   }
   MESystemError ex(code, isErrno);
   ex.Rethrow();
#else
   int e = MESystemError::GetLastGlobalSystemError();
   errno = 0;  // clear errno so it does not stay on after reporting
   Throw(e);
#endif
   M_ENSURED_ASSERT(0);
}

void MESystemError::CheckLastSystemError(bool isError)
{
   if ( isError )
   {
      ThrowLastSystemError();
      M_ENSURED_ASSERT(0);
   }
}

#if !M_NO_VERBOSE_ERROR_INFORMATION
M_NORETURN_FUNC void MESystemError::ThrowLastSystemError(const MStdString& extraMessage)
{
   Throw(MESystemError::GetLastGlobalSystemError(), extraMessage.c_str());
   M_ENSURED_ASSERT(0);
}
#endif

#if !M_NO_FILESYSTEM
M_NORETURN_FUNC void MESystemError::ThrowFileNotOpen(const MStdString& fileName)
{
   ThrowLastSystemError(MGetStdString(M_I("File '%s' not open"), fileName.c_str()));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MESystemError::ThrowInputOutputError(const MStdString& fileName)
{
   ThrowLastSystemError(MGetStdString(M_I("Input/Output operation error for file '%s'"), fileName.c_str()));
   M_ENSURED_ASSERT(0);
}
#endif // !M_NO_FILESYSTEM

void MESystemError::VerifySystemError(unsigned error)
{
   #if M_OS & M_OS_WINDOWS
      if ( error != ERROR_SUCCESS )
      {
         Throw(error);
         M_ENSURED_ASSERT(0);
      }
   #else
      if ( error != 0 )
      {
         Throw(error);
         M_ENSURED_ASSERT(0);
      }
   #endif
}

MException* MESystemError::NewClone() const
{
   return M_NEW MESystemError(*this);
}

M_NORETURN_FUNC void MESystemError::Rethrow()
{
   throw *this;
}

MESystemError* MESystemError::New()
{
   return M_NEW MESystemError;
}

#if !M_NO_SOCKETS

   #if (M_OS & M_OS_WINDOWS) != 0
      static const int OS_ETIMEDOUT = WSAETIMEDOUT;
   #else
      static const int OS_ETIMEDOUT = ETIMEDOUT;
   #endif

M_START_PROPERTIES(ESocketError)
   M_OBJECT_PROPERTY_READONLY_UINT        (ESocketError, SocketErrorCode)
M_START_METHODS(ESocketError)
   M_CLASS_SERVICE                        (ESocketError, New,                ST_MObjectP_S)
M_END_CLASS(ESocketError, Exception)

MESocketError::MESocketError()
:
   MException(),
   m_socketErrorCode(0)
{
   m_kind = ErrorSocket;
}

MESocketError::MESocketError(const MESocketError& ex)
:
   MException(ex),
   m_socketErrorCode(ex.m_socketErrorCode)
{
   M_ASSERT(m_kind == ErrorSocket);
}

MESocketError::~MESocketError() M_NO_THROW
{
}

M_NORETURN_FUNC void MESocketError::ThrowLastSocketError()
{
   const unsigned socketErrorCode = GetLastGlobalSocketError();
   ThrowSocketError(socketErrorCode);
}

M_NORETURN_FUNC void MESocketError::ThrowSocketError(unsigned socketErrorCode)
{
   unsigned code = socketErrorCode;
#if !M_NO_VERBOSE_ERROR_INFORMATION
   if ( code != 0 ) // if the information is available
   {
      MStdString message = MESystemError::MessageFromSystemError(socketErrorCode);
      if ( message.empty() )
         message = MGetStdString(M_I("Socket error %X"), socketErrorCode);
      #if M_OS & M_OS_WINDOWS
         // make sure the below "error |= 0x8004000" does the same job as MAKE_HRESULT
         M_ASSERT((code | 0x80000000) == (unsigned)(MAKE_HRESULT(1, 0, code)));
      #endif
      code |= 0x80000000;
      ThrowSocketError(socketErrorCode, (MErrorEnum::Type)code, message.c_str());
   }
   else
   {
      ThrowSocketError(socketErrorCode, (MErrorEnum::Type)MErrorEnum::UnknownSocketError, M_I("Unknown socket error"));
   }
#else
   if ( code != 0 ) // if the information is available
      code |= 0x80000000;
   else
      code = (unsigned)MErrorEnum::UnknownSocketError;
   ThrowSocketError(socketErrorCode, (MErrorEnum::Type)code);
#endif
   M_ENSURED_ASSERT(0);
}

unsigned MESocketError::GetLastGlobalSocketError()
{
#if (M_OS & M_OS_POSIX)
   return (unsigned)errno;
#else
   return (unsigned)::WSAGetLastError(); // call immediately
#endif
}

M_NORETURN_FUNC void MESocketError::ThrowSocketReadTimeout()
{
   ThrowSocketError(OS_ETIMEDOUT, M_CODE_STR(MErrorEnum::SocketReadTimeout, M_I("Socket read timeout")));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MESocketError::ThrowSocketWriteTimeout()
{
   ThrowSocketError(OS_ETIMEDOUT, M_CODE_STR(MErrorEnum::SocketWriteTimeout, M_I("Socket write timeout")));
   M_ENSURED_ASSERT(0);
}

#if !M_NO_VERBOSE_ERROR_INFORMATION
M_NORETURN_FUNC void MESocketError::ThrowSocketError(unsigned socketError, MErrorEnum::Type code, MConstLocalChars str)
{
   MESocketError ex;
   ex.m_socketErrorCode = socketError;
   ex.InitAll(ErrorSocket, (MErrorEnum::Type)code, MGetStdString(str));
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}
M_NORETURN_FUNC void MESocketError::ThrowSocketError(unsigned socketError, MErrorEnum::Type code, MConstChars str)
{
   MESocketError ex;
   ex.m_socketErrorCode = socketError;
   ex.InitAll(ErrorSocket, (MErrorEnum::Type)code, str);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}
#else
M_NORETURN_FUNC void MESocketError::ThrowSocketError(unsigned socketError, MErrorEnum::Type code)
{
   MESocketError ex;
   ex.m_socketErrorCode = socketError;
   ex.InitAll(ErrorSocket,  (MErrorEnum::Type)code);
   ex.Rethrow();
   M_ENSURED_ASSERT(0);
}
#endif

M_NORETURN_FUNC void MESocketError::ThrowSocketErrorFromReturnValue(int returnValue)
{
#if (M_OS & M_OS_WINDOWS)
   M_USED_VARIABLE(returnValue);
   MESocketError::ThrowLastSocketError();
#else
   if ( returnValue == EAI_SYSTEM )
      ThrowLastSocketError();
   else
      ThrowSocketError(returnValue, M_CODE_STR((MErrorEnum::Type)returnValue, gai_strerror(returnValue)));
#endif
   M_ENSURED_ASSERT(0);
}

MException* MESocketError::NewClone() const
{
   return M_NEW MESocketError(*this);
}

void MESocketError::Rethrow()
{
   throw *this;
}

MESocketError* MESocketError::New()
{
   return M_NEW MESocketError;
}

#endif // !M_NO_SOCKETS

M_START_PROPERTIES(EOperationCancelled)
M_START_METHODS(EOperationCancelled)
   M_CLASS_SERVICE(EOperationCancelled, New,   ST_MObjectP_S)
   M_CLASS_SERVICE(EOperationCancelled, Throw, ST_S)
M_END_CLASS(EOperationCancelled, Exception)

MEOperationCancelled::MEOperationCancelled()
:
   MException()
{
   Init(ErrorInformation, M_CODE_STR(MErrorEnum::OperationCancelled, M_I("Operation cancelled")));
}

MEOperationCancelled::MEOperationCancelled(const MEOperationCancelled& ex)
:
   MException(ex)
{
}

MEOperationCancelled::~MEOperationCancelled() M_NO_THROW
{
}

M_NORETURN_FUNC void MEOperationCancelled::Throw()
{
   MEOperationCancelled().Rethrow();
   M_ENSURED_ASSERT(0);
}

MException* MEOperationCancelled::NewClone() const
{
   return M_NEW MEOperationCancelled(*this);
}

M_NORETURN_FUNC void MEOperationCancelled::Rethrow()
{
   throw *this;
}

MEOperationCancelled* MEOperationCancelled::New()
{
   return M_NEW MEOperationCancelled;
}

M_START_PROPERTIES(ENumberOutOfRange)
   M_OBJECT_PROPERTY_DOUBLE               (ENumberOutOfRange, RangeMin)
   M_OBJECT_PROPERTY_DOUBLE               (ENumberOutOfRange, RangeMax)
   M_OBJECT_PROPERTY_DOUBLE               (ENumberOutOfRange, Value)
   M_OBJECT_PROPERTY_STRING               (ENumberOutOfRange, Name, ST_constMStdStringA_X, ST_X_constMStdStringA)
M_START_METHODS(ENumberOutOfRange)
   M_CLASS_SERVICE                        (ENumberOutOfRange, New,                                     ST_MObjectP_S)
   M_CLASS_SERVICE_OVERLOADED             (ENumberOutOfRange, Throw,        ThrowValue,             1, ST_S_double)
   M_CLASS_SERVICE_OVERLOADED             (ENumberOutOfRange, Throw,        ThrowNamedValue,        2, ST_S_double_MConstChars)
   M_CLASS_SERVICE_OVERLOADED             (ENumberOutOfRange, Throw,        ThrowRange,             3, ST_S_double_double_double)
   M_CLASS_SERVICE_OVERLOADED             (ENumberOutOfRange, Throw,        ThrowNamedRange,        4, ST_S_double_double_double_MConstChars)
   M_CLASS_SERVICE_OVERLOADED             (ENumberOutOfRange, Check,        CheckRange,             3, ST_S_double_double_double)
   M_CLASS_SERVICE_OVERLOADED             (ENumberOutOfRange, Check,        CheckNamedRange,        4, ST_S_double_double_double_MConstChars)
   M_CLASS_SERVICE_OVERLOADED             (ENumberOutOfRange, CheckInteger, CheckIntegerRange,      3, ST_S_int_int_int)
   M_CLASS_SERVICE_OVERLOADED             (ENumberOutOfRange, CheckInteger, CheckNamedIntegerRange, 4, ST_S_int_int_int_MConstChars)
M_END_CLASS(ENumberOutOfRange, Exception)

MENumberOutOfRange::MENumberOutOfRange(double lo, double hi, double val, MConstChars itemName)
:
   MException(),
   m_lo(lo),
   m_hi(hi),
   m_value(val)
#if !M_NO_VERBOSE_ERROR_INFORMATION
   , m_name()
#endif
{
#if !M_NO_VERBOSE_ERROR_INFORMATION
   if ( itemName != NULL )
      m_name = itemName;
   MChar valStr [ 64 ];
   MToChars(val, valStr, true);

   // Check if the range was proper, and the value is in that range.
   // Recover cases when hi or lo were unknown, hence the range was invalid and should not be reported.
   // This is not an assertion condition - such cases exist.
   //
   if ( (val > lo && val < hi) || lo > hi )
   {
      if ( itemName == NULL )
         Init(MErrorEnum::NumberOutOfRange, M_I("Value %s is out of range"), valStr);
      else
         Init(MErrorEnum::NumberOutOfRange, M_I("Value %s for '%s' is out of range"), valStr, itemName);
   }
   else
   {
      MChar loStr [ 64 ];
      MChar hiStr [ 64 ];
      MToChars(lo, loStr, true);
      MToChars(hi, hiStr, true);

      if ( itemName == NULL )
         Init(MErrorEnum::NumberOutOfRange, M_I("Value %s is out of range %s .. %s"), valStr, loStr, hiStr);
      else
         Init(MErrorEnum::NumberOutOfRange, M_I("Value %s for '%s' is out of range %s .. %s"), valStr, itemName, loStr, hiStr);
   }
#else
   Init(MErrorEnum::NumberOutOfRange);
#endif
}

MENumberOutOfRange::MENumberOutOfRange(const MENumberOutOfRange& ex)
:
   MException(ex),
   m_lo(ex.m_lo),
   m_hi(ex.m_hi),
   m_value(ex.m_value)
#if !M_NO_VERBOSE_ERROR_INFORMATION
   , m_name(ex.m_name)
#endif
{
}

MENumberOutOfRange::~MENumberOutOfRange() M_NO_THROW
{
}

MException* MENumberOutOfRange::NewClone() const
{
   return M_NEW MENumberOutOfRange(*this);
}

M_NORETURN_FUNC void MENumberOutOfRange::Rethrow()
{
   throw *this;
}

M_NORETURN_FUNC void MENumberOutOfRange::ThrowValue(double val)
{
   ThrowRange(1.0, 0.0, val);
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MENumberOutOfRange::ThrowNamedValue(double val, MConstChars itemName)
{
   ThrowNamedRange(1.0, 0.0, val, itemName);
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MENumberOutOfRange::ThrowRange(double lo, double hi, double val)
{
   ThrowNamedRange(lo, hi, val, NULL);
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MENumberOutOfRange::ThrowNamedRange(double lo, double hi, double val, MConstChars itemName)
{
   MENumberOutOfRange(lo, hi, val, itemName).Rethrow();
   M_ENSURED_ASSERT(0);
}

void MENumberOutOfRange::CheckRange(double lo, double hi, double val)
{
   CheckNamedRange(lo, hi, val, NULL);
}

void MENumberOutOfRange::CheckNamedRange(double lo, double hi, double val, MConstChars itemName)
{
   M_ASSERT(lo <= hi);
   if ( val < lo || val > hi )
   {
      ThrowNamedRange(lo, hi, val, itemName);
      M_ENSURED_ASSERT(0);
   }
}

void MENumberOutOfRange::CheckIntegerRange(int lo, int hi, int val)
{
   CheckNamedIntegerRange(lo, hi, val, NULL);
}

void MENumberOutOfRange::CheckNamedIntegerRange(int lo, int hi, int val, MConstChars itemName)
{
   M_ASSERT(lo <= hi || (lo == 0 && hi == -1) || (lo == 1 && hi == 0)); // take care of special case, array count check
   if ( val < lo || val > hi )
   {
      ThrowNamedRange(double(lo), double(hi), double(val), itemName);
      M_ENSURED_ASSERT(0);
   }
}

void MENumberOutOfRange::CheckUnsignedRange(unsigned lo, unsigned hi, unsigned val)
{
   CheckNamedUnsignedRange(lo, hi, val, NULL);
}

void MENumberOutOfRange::CheckNamedUnsignedRange(unsigned lo, unsigned hi, unsigned val, MConstChars itemName)
{
   M_ASSERT(lo <= hi && hi != UINT_MAX); // obviously, bad use. Possibly, the result of (unsigned)size - 1
   if ( val < lo || val > hi )
   {
      ThrowNamedRange(double(lo), double(hi), double(val), itemName);
      M_ENSURED_ASSERT(0);
   }
}

void MENumberOutOfRange::CheckNamedUInt64Range(Muint64 lo, Muint64 hi, Muint64 val, MConstChars itemName)
{
   if ( val < lo || val > hi )
   {
      ThrowNamedRange(double(lo), double(hi), double(val), itemName);
      M_ENSURED_ASSERT(0);
   }
}

MENumberOutOfRange* MENumberOutOfRange::New()
{
   return M_NEW MENumberOutOfRange(0, 0, 0);
}

M_START_PROPERTIES(EIndexOutOfRange)
   M_OBJECT_PROPERTY_INT            (EIndexOutOfRange, IndexMin)
   M_OBJECT_PROPERTY_INT            (EIndexOutOfRange, IndexMax)
   M_OBJECT_PROPERTY_INT            (EIndexOutOfRange, Value)
M_START_METHODS(EIndexOutOfRange)
   M_CLASS_SERVICE                  (EIndexOutOfRange, New,                       ST_MObjectP_S)
   M_CLASS_SERVICE_OVERLOADED       (EIndexOutOfRange, Throw, ThrowIndex,      3, ST_S_int_int_int)
   M_CLASS_SERVICE_OVERLOADED       (EIndexOutOfRange, Throw, ThrowNamedIndex, 4, ST_S_int_int_int_MConstChars)
   M_CLASS_SERVICE_OVERLOADED       (EIndexOutOfRange, Check, CheckIndex,      3, ST_S_int_int_int)
   M_CLASS_SERVICE_OVERLOADED       (EIndexOutOfRange, Check, CheckNamedIndex, 4, ST_S_int_int_int_MConstChars)
M_END_CLASS(EIndexOutOfRange, Exception)

MEIndexOutOfRange::MEIndexOutOfRange(int lo, int hi, int val, MConstChars itemName)
:
   MException(),
   m_lo(lo),
   m_hi(hi),
   m_value(val)
{
#if !M_NO_VERBOSE_ERROR_INFORMATION
   if ( itemName != NULL )
      Init(MErrorEnum::IndexOutOfRange, M_I("Index %d for '%s' is out of range %d .. %d"), val, itemName, lo, hi);
   else
      Init(MErrorEnum::IndexOutOfRange, M_I("Index %d is out of range %d .. %d"), val, lo, hi);
#else
   Init(MErrorEnum::IndexOutOfRange);
#endif
}

MEIndexOutOfRange::MEIndexOutOfRange(const MEIndexOutOfRange& ex)
:
   MException(ex),
   m_lo(ex.m_lo),
   m_hi(ex.m_hi),
   m_value(ex.m_value)
{
}

MEIndexOutOfRange::~MEIndexOutOfRange() M_NO_THROW
{
}

M_NORETURN_FUNC void MEIndexOutOfRange::ThrowIndex(int lo, int hi, int val)
{
   ThrowNamedIndex(lo, hi, val, NULL);
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MEIndexOutOfRange::ThrowNamedIndex(int lo, int hi, int val, MConstChars itemName)
{
   MEIndexOutOfRange(lo, hi, val, itemName).Rethrow();
   M_ENSURED_ASSERT(0);
}

void MEIndexOutOfRange::CheckIndex(int lo, int hi, int val)
{
   CheckNamedIndex(lo, hi, val, NULL);
}

void MEIndexOutOfRange::CheckNamedIndex(int lo, int hi, int val, MConstChars itemName)
{
   M_ASSERT(lo <= hi || (lo == 0 && hi == -1)); // take care of special case when the array has no elements
   if ( val < lo || val > hi )
   {
      ThrowNamedIndex(lo, hi, val, itemName);
      M_ENSURED_ASSERT(0);
   }
}

MException* MEIndexOutOfRange::NewClone() const
{
   return M_NEW MEIndexOutOfRange(*this);
}

M_NORETURN_FUNC void MEIndexOutOfRange::Rethrow()
{
   throw *this;
}

MEIndexOutOfRange* MEIndexOutOfRange::New()
{
   return M_NEW MEIndexOutOfRange(0, 0, 0);
}


M_START_PROPERTIES(EMath)
M_START_METHODS(EMath)
   M_CLASS_SERVICE                  (EMath, New,   ST_MObjectP_S)
   M_CLASS_SERVICE                  (EMath, Throw, ST_S)
M_END_CLASS(EMath, Exception)

MEMath::MEMath()
:
   MException()
{
}

MEMath::MEMath(const MEMath& other)
:
   MException(other)
{
}

MEMath::~MEMath() M_NO_THROW
{
}

void MEMath::BeforeDoingMath() M_NO_THROW
{
   errno = 0;
   #if (M_OS & M_OS_POSIX) != 0
     feclearexcept(FE_ALL_EXCEPT);
   #endif
}

   #if defined(_MSC_VER) || defined(__BORLANDC__)
      #define isnan(x) _isnan(x)
   #endif

using namespace std;

void MEMath::AfterDoingMath(double result, const char* operationName)
{
   int e = errno;
   if ( e != 0 || isnan(result) )
   {
       errno = 0; // clear it before throwing

       MEMath ex;
       if ( e == 0 ) // result == NAN but errno is not set, workaround for environments that do not set errno
          e = ERANGE; // ERANGE = 34  the best guess for the error 
       ex.SetCode(static_cast<MErrorEnum::Type>(e | 0x80000000));
       #if (M_OS & M_OS_WINDOWS) != 0
          ex.SetMessageString(MESystemError::MessageFromSystemError(e, true));
       #else
          ex.SetMessageString(MESystemError::MessageFromSystemError(e));
       #endif
       if ( operationName != NULL && operationName[0] != '\0' )
          ex.AppendToString(M_I(" in '%s'"), operationName);
       ex.Rethrow();
       M_ENSURED_ASSERT(0);
   }
}

MException* MEMath::NewClone() const
{
   return M_NEW MEMath(*this);
}

M_NORETURN_FUNC void MEMath::Rethrow()
{
   throw *this;
}

M_NORETURN_FUNC void MEMath::Throw()
{
   MEMath().Rethrow();
   M_ENSURED_ASSERT(0);
}

MEMath* MEMath::New()
{
   return M_NEW MEMath;
}

#if defined(_MSC_VER) && (M_OS & M_OS_WIN32_CE) == 0

   static void __cdecl LocalSeTranslator(unsigned code, PEXCEPTION_POINTERS)
   {
      throw MEProgramError(code);
   }

MEProgramError::Guard::Guard()
:
   m_savedTranslator(_set_se_translator(LocalSeTranslator)),
   m_programErrorCode(0),
   m_fileNameAndLineNumber()
{
}

MEProgramError::Guard::~Guard()
{
   _set_se_translator(m_savedTranslator);

   if ( m_programErrorCode == EXCEPTION_STACK_OVERFLOW ) // only here we restore stack
      _resetstkoflw();
}

void MEProgramError::Guard::RethrowIfError()
{
   if ( m_programErrorCode != 0 )
   {
      MException ex;
      InitializeException(&ex);
      ex.Rethrow();
      M_ENSURED_ASSERT(0);
   }
}

void MEProgramError::Guard::InitializeException(MException* ex) M_NO_THROW
{
   M_ASSERT(m_programErrorCode != 0);
   M_ASSERT(ex != NULL);

   const char* err;
   char buff [ 64 ];
   switch ( m_programErrorCode )
   {
   case EXCEPTION_ACCESS_VIOLATION:
      err = "Access violation";
      break;
   case EXCEPTION_FLT_DIVIDE_BY_ZERO:
   case EXCEPTION_INT_DIVIDE_BY_ZERO:
      err = "Division by zero";
      break;
   case EXCEPTION_STACK_OVERFLOW:
      // one shall not call _resetstkoflw() here! It shall be called in destructor instead.
      err = "Stack overflow. Infinite recursion?";
      break;
   default:
      MFormat(buff, 64, "Application error %d", m_programErrorCode);
      err = buff;
      break;
   }
   ex->Init(MException::ErrorFatal, MErrorEnum::Unknown, err);
   ex->UpdateFileNameAndLineNumber(m_fileNameAndLineNumber);
}

void MEProgramError::StackOverflowTester()
{
   // This code assumes the operating system page is no less than 4 kilobytes
   //
   const unsigned count = 2048;
   volatile Mint32 dummy[ count ]; // 8 kilobytes of nonoptimizable stack
   dummy[count / 2 - 1] = 0; // test for stack overflow
   dummy[count - 1] = 0;
}

#endif
