// File MCORE/MStream.cpp

#include "MCOREExtern.h"
#include "MStreamFile.h"
#include "MStreamProcessor.h"
#include "MException.h"
#include "MUtilities.h"
#include "MAesEax.h"

M_START_PROPERTIES(Stream)
   M_CLASS_ENUMERATION                  (Stream, FlagReadOnly)
   M_CLASS_ENUMERATION                  (Stream, FlagWriteOnly)
   M_CLASS_ENUMERATION                  (Stream, FlagReadWrite)
   M_CLASS_ENUMERATION                  (Stream, FlagText)
   M_CLASS_ENUMERATION                  (Stream, FlagBuffered)
   M_OBJECT_PROPERTY_READONLY_STRING    (Stream, Name,           ST_MStdString_X)
   M_OBJECT_PROPERTY_UINT               (Stream, Position)
   M_OBJECT_PROPERTY_UINT               (Stream, Size)
   M_OBJECT_PROPERTY_READONLY_UINT      (Stream, Flags)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT(Stream, IsOpen)
   M_OBJECT_PROPERTY_STRING             (Stream, Key, ST_MStdString_X, ST_X_constMStdStringA)
M_START_METHODS(Stream)
   M_OBJECT_SERVICE(Stream, ReadByte,           ST_MVariant_X)
   M_OBJECT_SERVICE(Stream, Read,               ST_MByteString_X_unsigned)
   M_OBJECT_SERVICE(Stream, ReadLine,           ST_MVariant_X)
   M_OBJECT_SERVICE(Stream, ReadAll,            ST_MByteString_X)
   M_OBJECT_SERVICE(Stream, ReadAvailable,      ST_MByteString_X_unsigned)
   M_OBJECT_SERVICE(Stream, ReadAllLines,       ST_MStdStringVector_X)
   M_OBJECT_SERVICE(Stream, WriteByte,          ST_X_byte)
   M_OBJECT_SERVICE(Stream, Write,              ST_X_constMByteStringA)
   M_OBJECT_SERVICE(Stream, WriteLine,          ST_X_constMStdStringA)
   M_OBJECT_SERVICE(Stream, WriteAllLines,      ST_X_constMStdStringVectorA)
   M_OBJECT_SERVICE(Stream, Skip,               ST_X_unsigned)
   M_OBJECT_SERVICE(Stream, Flush,              ST_X)
   M_OBJECT_SERVICE(Stream, Close,              ST_X)
M_END_CLASS(Stream, Object)

   using namespace std;


MStream::MStream()
:
   m_flags(0),
   m_lastOp(STREAMOP_NONE),
   m_processor(NULL),
   m_bytesSavedCount(0),
   m_key()
{
   m_bytesSaved[0] = '\0';
   m_bytesSaved[1] = '\0'; // pacify lint
}

MStream::~MStream() M_NO_THROW
{
   MAes::DestroySecureData(m_key);
}

MStdString MStream::GetKey() const
{
   return MUtilities::BytesToHex(m_key, false);
}

void MStream::SetKey(const MStdString& key)
{
   M_ASSERT(this != m_processor);
   MByteString tmpKey = MUtilities::HexStringToBytes(key);
   if ( !tmpKey.empty() )
      MAesEax::CheckKeySizeValid(tmpKey);
   if ( m_processor != NULL )
      m_processor->DoSetKeyImpl(tmpKey);
   MAes::MoveSecureData(m_key, tmpKey);
}

unsigned MStream::GetPosition() const
{
   if ( m_processor != NULL )
      return m_processor->DoGetPosition();
   else
      return DoGetPosition();
}

unsigned MStream::DoGetPosition() const
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_OPERATION_NOT_SUPPORTED_FOR_THIS_TYPE, "Cannot get position from this stream type"));
   M_ENSURED_ASSERT(0);
   return 0;
}

void MStream::SetPosition(unsigned position)
{
   if ( m_processor != NULL )
      m_processor->DoSetPosition(position);
   else
      DoSetPosition(position);
}

void MStream::DoSetPosition(unsigned)
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_OPERATION_NOT_SUPPORTED_FOR_THIS_TYPE, "Cannot set position for this stream type"));
   M_ENSURED_ASSERT(0);
}

unsigned MStream::GetSize() const
{
   if ( m_processor != NULL )
      return m_processor->DoGetSize();
   else
      return DoGetSize();
}

unsigned MStream::DoGetSize() const
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_OPERATION_NOT_SUPPORTED_FOR_THIS_TYPE, "Cannot get size from this stream type"));
   M_ENSURED_ASSERT(0);
   return 0;
}

void MStream::SetSize(unsigned length)
{
   DoPrepareForOp(STREAMOP_WRITE);
   if ( m_processor != NULL )
      m_processor->DoSetSize(length);
   else
      DoSetSize(length);
}

void MStream::DoSetSize(unsigned)
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_OPERATION_NOT_SUPPORTED_FOR_THIS_TYPE, "Cannot set size for this stream type"));
   M_ENSURED_ASSERT(0);
}

#if !M_NO_VARIANT
MVariant MStream::ReadByte()
{
   M_COMPILED_ASSERT(sizeof(char) == sizeof(Muint8));
   char b;
   unsigned len = ReadAvailableBytes(&b, 1);
   if ( len == 0 )
      return -1;
   return (Muint8)b;
}
#else
int MStream::ReadByte()
{
   M_COMPILED_ASSERT(sizeof(char) == sizeof(Muint8));
   char b;
   unsigned len = ReadAvailableBytes(&b, 1);
   if ( len == 0 )
      return -1;
   return (int)(Muint8)b; // positive, 0 .. 255
}
#endif

MByteString MStream::Read(unsigned count)
{
   MByteString buffer;
   if ( count > 0 )
   {
#ifdef M_USE_USTL
      buffer.append(count, '\0');
#else
      buffer.assign(count, '\0');
#endif
      ReadBytes(&(buffer[0]), count);
   }
   else
      ReadBytes(NULL, count); // do initiate standard read procedure as it processes flags, etc.
   return buffer;
}

unsigned MStream::DoReadAllAvailableBytesImpl(char* buffer, unsigned count)
{
   return ReadAvailableBytes(buffer, count);
}

MByteString MStream::ReadAll()
{
   MByteString result;
   unsigned len;
   char buffer [ 0x1000 ]; // 4k
   do
   {
      len = DoReadAllAvailableBytesImpl(buffer, sizeof(buffer));
      result.append(buffer, len); // len can be zero, which is okay
   } while ( len == sizeof(buffer) );
   return result;
}

MByteString MStream::ReadAvailable(unsigned count)
{
   MByteString buffer;
   if ( count > 0 )
   {
#ifdef M_USE_USTL
      buffer.append(count, '\0');
#else
      buffer.assign(count, '\0');
#endif
      unsigned size = ReadAvailableBytes(&(buffer[0]), count);
      if ( size != count )
         buffer.erase(buffer.begin() + size, buffer.end());
   }
   return buffer;
}

void MStream::ReadBytes(char* buffer, unsigned count)
{
   unsigned size = ReadAvailableBytes(buffer, count);
   if ( size != count )
   {
      DoThrowEndOfStream();
      M_ENSURED_ASSERT(0);
   }
}

   // Whether there is a need to remove the given char
   //
   inline bool DoNeedToCopy(unsigned flags, char c)
   {
      #if (M_OS & M_OS_WINDOWS) != 0
         return c != '\r' || (flags & MStream::FlagText) == 0;
      #else
         return true;
      #endif
   }

unsigned MStream::ReadAvailableBytes(char* buffer, unsigned count)
{
   DoPrepareForOp(STREAMOP_READ);

   if ( count == 0 )
      return 0; // done, by convention after the check that read can be done.

   unsigned readCount = 0;
   if ( m_bytesSavedCount > 0 )
   {
      *buffer++ = m_bytesSaved[0];
      --m_bytesSavedCount;
      --readCount;
      --count;
      if ( m_bytesSavedCount != 0 && count != 0 ) // count equal to or bigger than 2
      {
         *buffer++ = m_bytesSaved[1];
         --m_bytesSavedCount;
         --readCount;
         --count;
      }
   }

   MStream* stream = (m_processor == NULL) ? this : m_processor;
   readCount += stream->DoReadAvailableBytesImpl(buffer, count);
   return readCount;
}

#if !M_NO_VARIANT
MVariant MStream::ReadLine()
{
   MVariant result;
   MStdString str;
   if ( ReadOneLine(str) )
      result = str;
   return result;
}
#endif // !M_NO_VARIANT

bool MStream::ReadOneLine(MStdString& str)
{
   str.erase();
   char c [ 1 ];
   unsigned len;
   for ( ; ; )
   {
      len = ReadAvailableBytes(c, 1);
      if ( len == 0 )
      {
         if ( str.empty() )
            return false; // end of file
         else
            break;
      }
      if ( c[0] == '\n' )
         break; // got the line
      if ( c[0] != '\r' ) // by convention, ignore \r
         str += c[0];
   }
   return true;
}

MStdStringVector MStream::ReadAllLines()
{
   MStdStringVector result;
   MStdString str;
   for ( ;; )
   {
      if ( !ReadOneLine(str) )
         break;
      result.push_back(str); // sure, the type is string
   }
   return result;
}

void MStream::WriteAllLines(const MStdStringVector& lines)
{
   MStdStringVector::const_iterator it = lines.begin();
   MStdStringVector::const_iterator itEnd = lines.end();
   for ( ; it != itEnd; ++it )
      WriteLine(*it);
}

void MStream::WriteFormat(MConstChars format, ...)
{
   va_list va;
   va_start(va, format);
   WriteFormatVA(format, va);
   va_end(va);
}

void MStream::WriteFormatVA(MConstChars format, va_list args)
{
   Write(MGetStdStringVA(format, args));
}

void MStream::Skip(unsigned count)
{
   char buff [ 256 ];
   while ( count > 0 )
   {
      unsigned chunk = unsigned(sizeof(buff));
      if ( chunk > count )
         chunk = count;
      ReadBytes(buff, chunk);
      count -= chunk;
   }
}

void MStream::WriteByte(Muint8 byte)
{
   WriteBytes((const char*)&byte, 1);
}

void MStream::Write(const MByteString& str)
{
#ifdef M_USE_USTL
   WriteBytes(const_cast<MByteString&>(str).data(), M_64_CAST(unsigned, str.size()));
#else
   WriteBytes(str.data(), M_64_CAST(unsigned, str.size()));
#endif
}

void MStream::WriteBytes(const char* buffer, unsigned count)
{
   DoPrepareForOp(STREAMOP_WRITE);
   if ( m_processor != NULL )
      m_processor->DoWriteBytesImpl(buffer, count);
   else
      DoWriteBytesImpl(buffer, count);
}

void MStream::WriteChars(const char* chars)
{
   WriteBytes(chars, M_64_CAST(unsigned, strlen(chars)));
}

void MStream::WriteLine(const MStdString& str)
{
   if ( !str.empty() )
   {
      Write(str);
      if ( *str.rbegin() == '\n' )
         return; // do not write \n in this case
   }
   WriteByte('\n');
}

void MStream::Flush()
{
   DoPrepareForOp(STREAMOP_WRITE);
    if ( m_processor != NULL )
      m_processor->DoFlushImpl(false); // hard flush
   else
      DoFlushImpl(false);
}

bool MStream::IsOpen() const
{
    if ( m_processor != NULL )
      return m_processor->DoIsOpenImpl();
   else
      return DoIsOpenImpl();
}

void MStream::DoCloseWithNoFlush()
{
   try
   {
      if ( m_processor != NULL )
         m_processor->DoCloseImpl();
      else
         DoCloseImpl();
      m_flags = 0u;
      m_lastOp = STREAMOP_NONE;
      DoDeleteProcessors();
   }
   catch ( ... )
   {
      m_flags = 0u;
      m_lastOp = STREAMOP_NONE;
      DoDeleteProcessors();
      throw;
   }
}

void MStream::Close()
{
   if ( m_flags != 0u ) // if we are open
   {
      try
      {
         if ( m_lastOp == STREAMOP_WRITE )
         {
            if ( m_processor != NULL )
               m_processor->DoFlushImpl(true); // soft flush
            else
               DoFlushImpl(true); // soft flush
         }
         DoCloseWithNoFlush();
      }
      catch ( ... )
      {
         DoCloseWithNoFlush();
         throw;
      }
   }
   else
      DoDeleteProcessors(); // this is necessary to do with some unsuccessfully opened streams
}

#if !M_NO_VARIANT

   template
      <class T>
   inline T DoReadRaw(MStream& stream)
   {
      T result;
      stream.ReadBytes(reinterpret_cast<char*>(&result), sizeof(T));
      return result;
   }

   template
      <class T>
   inline void DoWriteRaw(MStream& stream, T value)
   {
      stream.WriteBytes(reinterpret_cast<const char*>(&value), sizeof(T));
   }

int MStream::ReadRawInt()
{
   return DoReadRaw<int>(*this);
}

MChar MStream::ReadRawChar()
{
   return DoReadRaw<MChar>(*this);
}

bool MStream::ReadRawBool()
{
   return DoReadRaw<char>(*this) != 0;
}

double MStream::ReadRawDouble()
{
   return DoReadRaw<double>(*this) != 0;
}

Muint8 MStream::ReadRawByte()
{
   return DoReadRaw<Muint8>(*this);
}

MByteString MStream::ReadRawByteString()
{
   MByteString result;
   unsigned length = static_cast<unsigned>(ReadRawInt());
   if ( length != 0 ) // check if string is empty
      result = Read(length);
   return result;
}

MStdString MStream::ReadRawString()
{
   return ReadRawByteString();
}

MVariant MStream::ReadRawVariant()
{
   int typeId = ReadRawInt();
   switch ( typeId )
   {
   case MVariant::VAR_EMPTY:
      return MVariant();
   case MVariant::VAR_BOOL:
      return MVariant(ReadRawBool());
   case MVariant::VAR_BYTE:
      return MVariant((Muint8)ReadRawByte());
   case MVariant::VAR_CHAR:
      return MVariant(ReadRawChar());
   case MVariant::VAR_INT:
      return MVariant(ReadRawInt());
   case MVariant::VAR_UINT:
      return MVariant(unsigned(ReadRawInt()));
   case MVariant::VAR_DOUBLE:
      return MVariant(unsigned(ReadRawDouble()));
   case MVariant::VAR_BYTE_STRING:
      return MVariant(ReadRawByteString(), MVariant::ACCEPT_BYTE_STRING);
   case MVariant::VAR_STRING:
      return MVariant(ReadRawString());
   case MVariant::VAR_STRING_COLLECTION:
      {
         int count = ReadRawInt();
         MENumberOutOfRange::CheckNamedIntegerRange(0, 0xFFFFFF, count, "StringCollectionCount");
         MStdStringVector coll;
         for ( int i = 0; i < count; ++i )
            coll.push_back(ReadRawString());
         return MVariant(coll);
      }
   case MVariant::VAR_VARIANT_COLLECTION:
      {
         int count = ReadRawInt();
         MENumberOutOfRange::CheckNamedIntegerRange(0, 0xFFFFFF, count, "VariantCollectionCount");
         MVariant::VariantVector coll;
         for ( int i = 0; i < count; ++i )
            coll.push_back(ReadRawVariant());
         return MVariant(coll);
      }
   default:
      MException::ThrowUnsupportedType(typeId);
      M_ENSURED_ASSERT(0);
   }
   return MVariant();
}

void MStream::WriteRawInt(int value)
{
   DoWriteRaw<int>(*this, value);
}

void MStream::WriteRawChar(MChar value)
{
   DoWriteRaw<MChar>(*this, value);
}

void MStream::WriteRawBool(bool value)
{
   DoWriteRaw<char>(*this, value ? '\x01' : '\0');
}

void MStream::WriteRawDouble(double value)
{
   DoWriteRaw<double>(*this, value);
}

void MStream::WriteRawByteString(const MByteString& value)
{
   WriteRawInt(M_64_CAST(int, value.size()));
#ifdef M_USE_USTL
   WriteBytes(const_cast<MByteString&>(value).data(), M_64_CAST(unsigned, value.size()));
#else
   WriteBytes(value.data(), M_64_CAST(unsigned, value.size()));
#endif
}

void MStream::WriteRawString(const MStdString& value)
{
   // always write as MWideString
   WriteRawInt(M_64_CAST(int, value.size()));
   if ( value.size() > 0 )
#ifdef M_USE_USTL
      WriteBytes(reinterpret_cast<const char*>(const_cast<MStdString&>(value).data()),
                 M_64_CAST(unsigned, value.size() * sizeof(MChar)));
#else
      WriteBytes(reinterpret_cast<const char*>(value.data()), M_64_CAST(unsigned, value.size() * sizeof(MChar)));
#endif
}

void MStream::WriteRawVariant(const MVariant& value)
{
   int type = value.GetType();
   if ( type >= MVariant::VAR_VARIANT ) // all the others can be handled
   {
      MException::ThrowUnsupportedType(type);
      M_ENSURED_ASSERT(0);
   }

   // Writing type id in a compatibility manner
   //
   int typeToWrite = type;
   WriteRawInt(typeToWrite);
   switch ( type )
   {
      case MVariant::VAR_EMPTY:
         break;
      case MVariant::VAR_BYTE:
         WriteByte(value.AsByte());
         break;
      case MVariant::VAR_CHAR:
         WriteRawChar(value.AsChar());
         break;
      case MVariant::VAR_BOOL:
         WriteRawBool(value.AsBool());
         break;
      case MVariant::VAR_INT:
      case MVariant::VAR_UINT:
         WriteRawInt(int(value.AsDWord()));
         break;
      case MVariant::VAR_DOUBLE:
         WriteRawDouble(int(value.AsDouble()));
         break;
      case MVariant::VAR_BYTE_STRING:
         WriteRawByteString(value.DoInterpretAsByteString());
         break;
      case MVariant::VAR_STRING:
         WriteRawString(value.DoInterpretAsString());
         break;
      case MVariant::VAR_STRING_COLLECTION:
         {
            const MStdStringVector coll = value.AsStringCollection();
            // write to stream only non-empty collection
            WriteRawInt(M_64_CAST(int, coll.size()));
            for ( MStdStringVector::const_iterator it = coll.begin(); it != coll.end(); ++it )
               WriteRawString(*it);
         }
         break;
      case MVariant::VAR_VARIANT_COLLECTION:
         {
            const MVariant::VariantVector coll = value.AsVariantCollection();
            WriteRawInt(M_64_CAST(int, coll.size()));
            for ( MVariant::VariantVector::const_iterator it = coll.begin(); it != coll.end(); ++it )
               WriteRawVariant(*it);
         }
         break;
      default:
         // never here
         M_ENSURED_ASSERT(0);
   }
}

#endif // !M_NO_VARIANT

void MStream::DoSetKeyImpl(const MByteString&)
{
}

void MStream::DoCloseImpl()
{
}

void MStream::DoStartOpen(unsigned flags)
{
   M_ASSERT(m_flags == 0u);
   M_ASSERT(m_lastOp == STREAMOP_NONE);

   if ( (flags & FlagReadWrite) == 0 )
      flags |= FlagReadOnly; // by convention


   m_flags = flags;
   m_bytesSavedCount = 0;
}

void MStream::DoFinishOpen()
{
   DoDeleteProcessors();

   if ( (m_flags & FlagBuffered) != 0 )
         DoInsertProcessor(M_NEW MStreamProcessorBuffered(m_flags));

   #if (M_OS & M_OS_WINDOWS) != 0
      if ( (m_flags & FlagText) != 0 )
         DoInsertProcessor(M_NEW MStreamProcessorText);
   #endif
}

void MStream::DoInsertProcessor(MStreamProcessor* processor)
{
   processor->m_processor = (m_processor == NULL) ? this : m_processor;
   m_processor = processor;
}

void MStream::DoDeleteProcessors()
{
   M_ASSERT(this != m_processor);
   for ( MStream* p = m_processor; p != NULL ; )
   {
      MStream* tmp = p;
      p = p->m_processor;
      delete tmp;
      if ( p == this )
         break;
   }
   m_processor = NULL;
}

void MStream::DoPrepareForOp(MStream::StreamOpType op)
{
   M_ASSERT(op != STREAMOP_NONE);
   if ( m_flags == 0 )
   {
      DoThrowStreamError(M_ERR_BAD_STREAM_FLAG, M_OPT_STR(M_I("Stream '%s' not open")));
      M_ENSURED_ASSERT(0);
   }
   if ( op != m_lastOp )
   {
      switch ( op )
      {
      case STREAMOP_WRITE:
         if ( (m_flags & FlagWriteOnly) == 0 )
         {
            DoThrowStreamError(MErrorEnum::CannotWriteToReadonlyStream, M_OPT_STR(M_I("Cannot write to readonly stream '%s'")));
            M_ENSURED_ASSERT(0);
         }
         break;
      case STREAMOP_READ:
         if ( (m_flags & FlagReadOnly) == 0 )
         {
            DoThrowStreamError(MErrorEnum::CannotReadFromWriteonlyStream, M_OPT_STR(M_I("Cannot read from writeonly stream '%s'")));
            M_ENSURED_ASSERT(0);
         }
         break;
      default:
         M_ENSURED_ASSERT(op == STREAMOP_NONE);
      }
      m_lastOp = op;
   }
}

void MStream::DoFlushImpl(bool)
{
}

M_NORETURN_FUNC void MStream::DoThrowStreamSoftwareError(MErrorEnum::Type err, const char* msg)
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(err, msg, GetName().c_str()));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MStream::DoThrowStreamError(MErrorEnum::Type err, MConstLocalChars msg)
{
   MException::Throw(M_CODE_STR_P1(err, msg, GetName().c_str()));
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MStream::DoThrowEndOfStream()
{
   MException::Throw(M_CODE_STR_P1(MErrorEnum::EndOfStream, M_I("End of stream '%s'"), GetName().c_str()));
   M_ENSURED_ASSERT(0);
}

void MStream::DoSwap(MStream& stream)
{
   std::swap(m_flags, stream.m_flags);
   std::swap(m_lastOp, stream.m_lastOp);
   std::swap(m_processor, stream.m_processor);

   std::swap(m_bytesSavedCount, stream.m_bytesSavedCount);
   std::swap(m_bytesSaved[0], stream.m_bytesSaved[0]);
   std::swap(m_bytesSaved[1], stream.m_bytesSaved[1]);

   m_key.swap(stream.m_key);
}

M_FUNC MStream& operator<<(MStream& stream, const MStdString& str)
{
   stream.WriteBytes(str.c_str(), M_64_CAST(unsigned, str.size()));
   return stream;
}

#if !M_NO_WCHAR_T
M_FUNC MStream& operator<<(MStream& stream, const MWideString& str)
{
   return operator<<(stream, MToStdString(str));
}
#endif

M_FUNC MStream& operator<<(MStream& stream, const char* str)
{
   unsigned len = M_64_CAST(unsigned, strlen(str));
   stream.WriteBytes(str, len);
   return stream;
}

#if !M_NO_WCHAR_T
M_FUNC MStream& operator<<(MStream& stream, const wchar_t* str)
{
   MStdString ansi = MToStdString(str);
   stream.WriteBytes(ansi.c_str(), M_64_CAST(unsigned, ansi.size()));
   return stream;
}
#endif

M_FUNC MStream& operator<<(MStream& stream, char c)
{
   stream.WriteByte(c);
   return stream;
}

#if !M_NO_WCHAR_T
M_FUNC MStream& operator<<(MStream& stream, wchar_t c)
{
   char buffer [ 6 ];
   size_t i = MFormat(buffer, 6, "%lc", c);
   M_ASSERT(i > 0);
   stream.WriteBytes(buffer, static_cast<unsigned>(i));
   return stream;
}
#endif

M_FUNC MStream& operator<<(MStream& stream, int val)
{
   return operator<<(stream, MToStdString(val));
}

M_FUNC MStream& operator<<(MStream& stream, unsigned val)
{
   return operator<<(stream, MToStdString(val));
}

M_FUNC MStream& operator<<(MStream& stream, double val)
{
   return operator<<(stream, MToStdString(val));
}
