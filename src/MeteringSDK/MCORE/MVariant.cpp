// File MCORE/MVariant.cpp

#include "MCOREExtern.h"
#include "MVariant.h"
#include "MObject.h"
#include "MException.h"
#include "MUtilities.h"
#include "MAlgorithm.h"
#include "MMath.h"
#include "MStr.h"

   const MStdString MVariant::s_emptyString;

#if !M_NO_VARIANT

   // Because below we use numeric_limits::min() and numeric_limits::max()
   #ifdef min
      #undef min
   #endif
   #ifdef max
      #undef max
   #endif

   using namespace std;

   #if !M_NO_REFLECTION

      // Reflected variation of enumerations defined in MVariant.
      // This type has no use outside reflection.
      //
      class MVariantEnum : public MObject
      {
      public: // Type:

         enum Type
         {
            VarEmpty             = MVariant::VAR_EMPTY,
            VarBool              = MVariant::VAR_BOOL,
            VarByte              = MVariant::VAR_BYTE,
            VarChar              = MVariant::VAR_CHAR,
            VarUInt              = MVariant::VAR_UINT,
            VarInt               = MVariant::VAR_INT,
            VarDouble            = MVariant::VAR_DOUBLE,
            VarByteString        = MVariant::VAR_BYTE_STRING,
            VarString            = MVariant::VAR_STRING,
            VarStringCollection  = MVariant::VAR_STRING_COLLECTION,
            VarObject            = MVariant::VAR_OBJECT,
            VarObjectEmbedded    = MVariant::VAR_OBJECT_EMBEDDED,
            VarVariantCollection = MVariant::VAR_VARIANT_COLLECTION,
            VarMap               = MVariant::VAR_MAP,
            VarVariant           = MVariant::VAR_VARIANT
         };
         M_DECLARE_CLASS(VariantEnum)
      };

      M_START_PROPERTIES(VariantEnum)
         M_CLASS_ENUMERATION(VariantEnum, VarEmpty)
         M_CLASS_ENUMERATION(VariantEnum, VarBool)
         M_CLASS_ENUMERATION(VariantEnum, VarByte)
         M_CLASS_ENUMERATION(VariantEnum, VarChar)
         M_CLASS_ENUMERATION(VariantEnum, VarUInt)
         M_CLASS_ENUMERATION(VariantEnum, VarInt)
         M_CLASS_ENUMERATION(VariantEnum, VarDouble)
         M_CLASS_ENUMERATION(VariantEnum, VarByteString)
         M_CLASS_ENUMERATION(VariantEnum, VarString)
         M_CLASS_ENUMERATION(VariantEnum, VarStringCollection)
         M_CLASS_ENUMERATION(VariantEnum, VarObject)
         M_CLASS_ENUMERATION(VariantEnum, VarObjectEmbedded)
         M_CLASS_ENUMERATION(VariantEnum, VarVariantCollection)
         M_CLASS_ENUMERATION(VariantEnum, VarMap)
         M_CLASS_ENUMERATION(VariantEnum, VarVariant)
      M_START_METHODS(VariantEnum)
      M_END_CLASS(VariantEnum, Object)

      // Reflection-specific constants
      //
      static const MStdString s_asString  = "AsString";
      static const MStdString s_compare   = "Compare";
      static const MStdString s_add       = "Add";
      static const MStdString s_subtract  = "Subtract";
      static const MStdString s_multiply  = "Multiply";
      static const MStdString s_divide    = "Divide";
      static const MStdString s_item      = "Item";
      static const MStdString s_setItem   = "SetItem";
      static const MStdString s_setToNull = "SetToNull";
      static const MStdString s_value     = "Value";

   #endif

   const int s_MCHAR_MIN = SCHAR_MIN; // cover both signed and unsigned char, -127 ..
   const int s_MCHAR_MAX = UCHAR_MAX; // cover both signed and unsigned char, .. 255

M_COMPILED_ASSERT(sizeof(MVariant) == 16);

const MVariant MVariant::s_null;

MVariant::MVariant(MObject* o)
{
   if ( o != NULL )
   {
      unsigned size = o->GetEmbeddedSizeof();
      if ( size != 0 )
      {
         m_type = VAR_OBJECT_EMBEDDED;
         m_bufferType = BUFFERTYPE_REFCOUNT;
         DoCreateSharedBuffer((const char*)o, size);
         return;
      }
   }
   m_type = VAR_OBJECT;
   m_bufferType = BUFFERTYPE_COPY;
   m_object = o;
}

void MVariant::DoCleanup() M_NO_THROW
{
   if ( m_bufferType == BUFFERTYPE_REFCOUNT )
   {
      if ( IsCollection() && m_int32 > 0 )
      {
         MSharedString tmp(DoAccessSharedString());
         DoAccessSharedString()._get_buffer()->_ref_decrement();
         if ( !tmp.is_shared() ) // need to delete the buffer
         {
            MVariant* it = reinterpret_cast<MVariant*>(&tmp[0]);
            MVariant* last = it + m_int32;
            for ( ; it != last; ++it )
               it->MVariant::~MVariant();
         }
      }
      else
         DoAccessSharedString()._get_buffer()->_ref_decrement();
   }
#if M_DEBUG
   m_type = VAR_EMPTY; // so the checks of AssignToEmpty work
#endif
}

void MVariant::DoCreateSharedBuffer(const char* buff, unsigned size)
{
   new((void*)&m_pointerBytes) MSharedString(buff, size);
}

void MVariant::DoCreateSharedBuffer(unsigned size)
{
   new((void*)&m_pointerBytes) MSharedString(size, '\0'); // this guarantees creation of empty variants in the buffer
}

void MVariant::DoConvertInternalToSharedBuffer(unsigned reserve) const
{
   M_ASSERT(m_type == VAR_STRING || m_type == VAR_BYTE_STRING);
   M_ASSERT(m_bufferType == BUFFERTYPE_COPY);
   M_ASSERT(reserve >= m_uint32);

   PointerBytesType tmp; // use temporary to allow AsConstChars() work on this
   new((void*)&tmp) MSharedString();
   MSharedString* tmpString = reinterpret_cast<MSharedString*>(&tmp);
   tmpString->reserve(reserve);
   tmpString->assign(AsConstChars(), m_uint32);
   m_bufferType = BUFFERTYPE_REFCOUNT;
   m_pointerBytes = tmp; // only then assign
}

void MVariant::SetEmpty() M_NO_THROW
{
   DoSetType(VAR_EMPTY, BUFFERTYPE_NONE);
}

void MVariant::SetToNull(Type type)
{
   DoCleanup();
   m_type = type;
   switch ( m_type )
   {
   case VAR_STRING:
   case VAR_BYTE_STRING: // string and byte string are created as copy-buffer in-place string
      m_bufferType = BUFFERTYPE_COPY;
      m_uint32 = 0;
      // do not bother to clear the placeholder
      break;
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
   case VAR_MAP:
      m_bufferType = BUFFERTYPE_REFCOUNT;
      m_uint32 = 0; // count
      new((void*)&m_pointerBytes) MSharedString();
      break;
   case VAR_DOUBLE:
      m_bufferType = BUFFERTYPE_COPY;
      m_double = 0.0;
      break;
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      m_bufferType = BUFFERTYPE_COPY;
      m_object = NULL;
      break;
   default:
      m_bufferType = BUFFERTYPE_NONE;
      m_uint32 = 0;
      break; // no need to copy anything
   }
}

void MVariant::ReserveElements(int count)
{
   MEIndexOutOfRange::CheckIndex(0, INT_MAX / 4, count); // restrict index with a reasonably big value of about 500 megs
   unsigned size = static_cast<unsigned>(count); // help compiler optimize multiplication below
   switch ( m_type )
   {
   case VAR_MAP:
      size *= (2 * sizeof(MVariant));
      break;
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      size *= sizeof(MVariant);
      break;
   case VAR_BYTE_STRING:
   case VAR_STRING:
      if ( m_bufferType == BUFFERTYPE_COPY ) // and not SharedString
      {
         if ( size >= EMBEDDED_BUFFER_SIZE ) // otherwise nothing to do
            DoConvertInternalToSharedBuffer(size);
         return; // done
      }
      break;
   default:
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
   DoAccessSharedString().reserve(size);
}

void MVariant::SetEmptyWithObjectDelete()
{
   if ( m_type == VAR_OBJECT )
   {
      delete m_object;
      m_type = VAR_EMPTY;
      m_bufferType = BUFFERTYPE_NONE;
   }
   else if ( m_type == VAR_VARIANT_COLLECTION || m_type == VAR_MAP )
   {
      const int num = m_int32;
      for ( int i = 0; i < num; ++i )
         DoAccessVariantItem(i).SetEmptyWithObjectDelete();
      m_type = VAR_EMPTY;
      m_bufferType = BUFFERTYPE_NONE;
   }
   else
      SetEmpty();
}

int MVariant::GetCount() const
{
   if ( m_type == VAR_MAP )
   {
      M_ASSERT((m_uint32 & 0x0001) == 0); // has to be an even number
      return static_cast<int>(m_uint32 >> 1); // real count is twice smaller
   }
   else if ( !IsIndexed() )
   {
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
   return m_int32;
}

void MVariant::SetCount(int count)
{
   MEIndexOutOfRange::CheckIndex(0, INT_MAX / 4, count); // restrict index with a reasonably big value of about 500 megs
   if ( m_type == VAR_MAP )
   {
      count <<= 1; // real count is twice bigger
      if ( m_int32 < count ) // cannot increase size of collection without specifying any new items
      {
         MException::Throw(MException::ErrorSoftware, M_CODE_STR(MErrorEnum::CannotIndexItem, "Cannot grow collection count"));
         M_ENSURED_ASSERT(0);
      }
   }
   else if ( !IsIndexed() )
   {
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }

   if ( m_int32 != count )
   {
      if ( m_type == VAR_STRING || m_type == VAR_BYTE_STRING )
      {
         if ( m_bufferType == BUFFERTYPE_COPY ) // and not SharedString
         {
            if ( count < EMBEDDED_BUFFER_SIZE )
            {
               int diff = count - m_int32;
               if ( diff > 0 )
               {
                  M_ASSERT(diff < EMBEDDED_BUFFER_SIZE);
                  memcpy(m_placeholder + m_int32, "\0\0\0\0\0\0\0", diff);
               }
               m_uint32 = count;
               return;
            }

            DoConvertInternalToSharedBuffer(count);
            DoAccessSharedString().resize(count);
            m_uint32 = count;     // and modify count
            return; // done
         }
         DoAccessSharedString().resize(count);
      }
      else
      {
         M_ASSERT(IsCollection());
         int prevCount = m_int32;
         if ( prevCount > count ) // shrink
         {
            DoMakeCollectionUnique();
            do
            {
               DoAccessVariantItem(--prevCount).MVariant::~MVariant();
            } while ( prevCount != count );
         }
         else if ( !DoAccessSharedString().is_shared() ) // this check has to be performed prior to resize
            DoAccessSharedString().resize(count * sizeof(MVariant));
         else
         {
            DoAccessSharedString().resize(count * sizeof(MVariant));
            for ( int i = prevCount - 1; i >= 0; --i )
            {
               MVariant& var = DoAccessVariantItem(i);
               if ( var.m_bufferType == BUFFERTYPE_REFCOUNT )
                  var.DoAccessSharedString()._get_buffer()->_ref_increment();
            }
         }
      }
      m_int32 = count;
   }
}

MVariant& MVariant::operator=(Mint64 v)
{
   DoCleanup();
   DoAssignToEmpty(v);
   return *this;
}

MVariant& MVariant::operator=(Muint64 v)
{
   DoCleanup();
   DoAssignToEmpty(v);
   return *this;
}

MVariant& MVariant::operator=(double v)
{
   DoCleanup();
   DoAssignToEmpty(v);
   return *this;
}

MVariant& MVariant::operator=(MConstChars s)
{
   DoCleanup();
   DoAssignToEmpty(s);
   return *this;
}

MVariant& MVariant::operator=(const MStdString& s)
{
   DoCleanup();
   DoAssignToEmpty(s);
   return *this;
}

MVariant& MVariant::operator=(const MStdStringVector& v)
{
   DoCleanup();
   DoAssignToEmpty(v);
   return *this;
}

MVariant& MVariant::operator=(const VariantVector& c)
{
   DoSetType(VAR_STRING_COLLECTION, BUFFERTYPE_REFCOUNT);
   int size = static_cast<int>(c.size());
   m_int32 = size;
   DoCreateSharedBuffer(size * sizeof(MVariant)); // initializes with empty variants
   for ( int i = 0; i < size; ++i )
      DoAccessVariantItem(i).DoAssignToEmpty(c[i]);
   return *this;
}

MVariant& MVariant::operator=(const MObject* o)
{
   if ( o != NULL )
   {
      unsigned size = o->GetEmbeddedSizeof();
      if ( size != 0u )
      {
         DoSetType(VAR_OBJECT_EMBEDDED, BUFFERTYPE_REFCOUNT);
         unsigned size = o->GetEmbeddedSizeof();
         DoCreateSharedBuffer((const char*)o, size);
         return *this;
      }
   }
   DoSetType(VAR_OBJECT, BUFFERTYPE_COPY);
   m_object = const_cast<MObject*>(o);
   return *this;
}

MVariant& MVariant::operator=(const MVariant& v)
{
   DoCleanup();
   DoAssignToEmpty(v);
   return *this;
}

void MVariant::AssignByteString(const MByteString& v)
{
   DoCleanup();
   DoAssignByteStringToEmpty(v);
}

void MVariant::AssignByteStringCollection(const MByteStringVector& v)
{
   DoCleanup();
   DoAssignByteStringCollectionToEmpty(v);
}

void MVariant::Assign(const Muint8* p, unsigned size)
{
   DoCleanup();
   DoAssignByteStringToEmpty(reinterpret_cast<const char*>(p), size);
}

void MVariant::AssignString(MConstChars p, unsigned size)
{
   DoCleanup();
   DoAssignToEmpty(p, size);
}

const char* MVariant::AsConstChars() const
{
   switch ( m_type )
   {
   case VAR_CHAR:
   case VAR_BYTE:
      // Use placeholder temporarily to return the result
      m_placeholder[0] = static_cast<char>(m_int32);
      m_placeholder[1] = '\0';
      break;
   case VAR_BYTE_STRING:
   case VAR_STRING:
      if ( m_bufferType == BUFFERTYPE_REFCOUNT )
         return DoAccessSharedString().c_str();

      // Otherwise this is an in-place string
      M_ASSERT(m_int32 < EMBEDDED_BUFFER_SIZE); // size is no bigger than 7
      m_placeholder[m_uint32] = '\0'; // zero-terminate the string
      break;
   default:
      MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert this type to a string"));
      M_ENSURED_ASSERT(0);
   }
   return m_placeholder;
}

bool MVariant::AsBool() const
{
   switch ( m_type )
   {
   case VAR_CHAR:
      return m_int32 != 0 && m_int32 != '0'; // either '0' or '\0'
   case VAR_BOOL:
   case VAR_BYTE:
   case VAR_INT:
   case VAR_UINT:
      return m_int32 != 0;
   case VAR_DOUBLE:
      return m_double != 0.0;
   case VAR_BYTE_STRING:
   case VAR_STRING:
      {  // Perl-like conventions apply
         if ( m_int32 == 0 )
            return false;
         const char* chars = AsConstChars();
         if ( m_int32 == 1 )
            return chars[0] != '\0' && chars[0] != '0';
         if ( m_int32 == 5 ) // convention
            return memcmp(chars, "FALSE", m_int32) != 0;
         return true;
      }
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
   case VAR_MAP:
      return m_int32 == 0;  // convention
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      return m_object != NULL;  // convention
   default:
      ;
   }
   M_ENSURED_ASSERT(m_type == VAR_EMPTY);
   return false; // by convention, VAR_EMPTY.AsBool is false
}

   static MVariant DoGetClientValueIfPresent(const MVariant* self, MConstLocalChars str)
   {
      #if !M_NO_REFLECTION

         if ( !self->IsObject() )
         {
            MException::Throw(MErrorEnum::BadConversion, str);
            M_ENSURED_ASSERT(0);
         }

         MObject* obj = self->AsObject();
         if ( obj == NULL || !obj->IsPropertyPresent(s_value) )
         {
            MException::Throw(MErrorEnum::BadConversion, str);
            M_ENSURED_ASSERT(0);
         }
         return obj->GetProperty(s_value);

      #else

         MException::ThrowNotSupportedForThisType();
         M_ENSURED_ASSERT(0);
         return MVariant(); // pacify compiler warnings

      #endif
   }

MChar MVariant::AsChar() const
{
   MChar str [ 32 ];

   switch ( m_type )
   {
   case VAR_BYTE:
   case VAR_BOOL: // by convention...
   case VAR_CHAR:
      return static_cast<MChar>(m_uint32);
   case VAR_INT:
   case VAR_UINT:
      {
         Mint32 val = m_int32;
         if ( val < s_MCHAR_MIN || val > s_MCHAR_MAX ) // cover both signed and unsigned
         {
            MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert '%s' to a single character"), MToChars(val, str));
            M_ENSURED_ASSERT(0);
         }
         return (MChar)val;
      }
   case VAR_DOUBLE:
      {
         double val = m_double;
         if ( val < double(s_MCHAR_MIN) || val > double(s_MCHAR_MAX) ) // cover both signed and unsigned
         {
            MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert '%s' to a single character"), MToChars(val, str));
            M_ENSURED_ASSERT(0);
         }
         return M_ROUND_TO(MChar, val);
      }
   case VAR_BYTE_STRING:
   case VAR_STRING:
      if ( m_int32 != 1 )
      {
         MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert a string of size %d to a single character"), m_int32);
         M_ENSURED_ASSERT(0);
      }
      return AsConstChars()[0];
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
   case VAR_MAP:
      return DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a character")).AsChar();
   default:
      ;
   }
   M_ASSERT(m_type == VAR_EMPTY);
   MException::ThrowNoValue();
   M_ENSURED_ASSERT(0);
   return '\0'; // we are never here, pacify compilers in debug mode
}

Muint8 MVariant::AsByte() const
{
   switch ( m_type )
   {
   case VAR_CHAR:
   case VAR_BOOL: // by convention...
   case VAR_BYTE:
      M_ASSERT(m_uint32 < 256);
      return static_cast<Muint8>(m_uint32);
   case VAR_INT:
   case VAR_UINT:
      {
         Mint32 val = m_int32;
         MENumberOutOfRange::CheckInteger(0, 255, (int)val);
         return (Muint8)val;
      }
   case VAR_DOUBLE:
      MENumberOutOfRange::Check(0.0, 255.0, m_double);
      return M_ROUND_TO(Muint8, m_double);
   case VAR_BYTE_STRING:
   case VAR_STRING:
      if ( m_int32 != 1 )
      {
         MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert a string of size %d to a single byte"), m_int32);
         M_ENSURED_ASSERT(0);
      }
      return AsConstChars()[0];
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
   case VAR_MAP:
      return DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a character")).AsByte();
   default:
      ;
   }
   M_ASSERT(m_type == VAR_EMPTY);
   MException::ThrowNoValue();
   M_ENSURED_ASSERT(0);
   return (Muint8)'\0'; // we are never here, pacify compilers in debug mode
}

Muint32 MVariant::AsDWord() const
{
   if ( m_type >= VAR_BOOL && m_type <= VAR_INT ) // most probable case is first
      return m_uint32;
   if ( m_type == VAR_DOUBLE ) // next probable case
   {
      double val = MMath::Round(m_double);
      MENumberOutOfRange::Check(static_cast<double>(INT_MIN), static_cast<double>(UINT_MAX), val);   // see, INT_MIN .. UINT_MAX
      if ( val < 0.0 )
         return static_cast<Muint32>(static_cast<Mint32>(val)); // not (unsigned long)!
      return static_cast<Muint32>(val);
   }
   if ( m_type == VAR_STRING || m_type == VAR_BYTE_STRING )
   {
      const char* chars = AsConstChars();
      if ( m_int32 != 0 && chars[0] == '-' )
         return (Muint32)MToInt(chars);
      return (Muint32)MToUnsigned(chars);
   }
   if ( m_type == VAR_EMPTY )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
   return DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a numeric value")).AsDWord();
}

int MVariant::AsInt() const
{
   if ( m_type == VAR_UINT ) // special case, quite probable
   {
      if ( m_int32 < 0 ) // unsigned integer bigger than maximum long
      {
         MChar str [ 64 ];
         MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert '%s' to integer"), MToChars(m_uint32, str));
         M_ENSURED_ASSERT(0);
      }
      return m_int32;
   }
   if ( m_type >= VAR_BOOL && m_type <= VAR_INT ) // most probable case, exclude UINT
      return m_int32;
   if ( m_type == VAR_DOUBLE ) // next probable case
   {
      double val = MMath::Round(m_double);
      MENumberOutOfRange::Check(static_cast<double>(INT_MIN), static_cast<double>(INT_MAX), val);
      return static_cast<int>(val);
   }
   if ( m_type == VAR_STRING || m_type == VAR_BYTE_STRING )
      return MToInt(AsConstChars());
   return DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a numeric value")).AsInt();
}

unsigned MVariant::AsUInt() const
{
   if ( m_type == VAR_INT ) // special case first
   {
      if ( m_int32 < 0 )
      {
         MChar str [ 64 ];
         MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert '%s' to unsigned integer"), MToChars(m_int32, str));
         M_ENSURED_ASSERT(0);
      }
      return static_cast<unsigned>(m_int32);
   }
   if ( m_type >= VAR_BOOL && m_type <= VAR_UINT ) // quite probable case
      return m_uint32;
   if ( m_type == VAR_DOUBLE )
   {
      double val = MMath::Round(m_double);
      if ( val < 0.0 || val > (double)UINT_MAX )
      {
         MChar str [ 64 ];
         MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert '%s' to unsigned integer"), MToChars(val, str));
         M_ENSURED_ASSERT(0);
      }
      return static_cast<unsigned>(val);
   }
   if ( m_type == VAR_STRING || m_type == VAR_BYTE_STRING )
      return MToUnsigned(AsConstChars());
   if ( m_type == VAR_EMPTY )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
   return DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a numeric value")).AsUInt();
}

Mint64 MVariant::AsInt64() const
{
   if ( m_type == VAR_UINT )
      return static_cast<Mint64>(m_uint32);
   if ( m_type >= VAR_BOOL && m_type <= VAR_INT ) // most probable case is first
      return static_cast<Mint64>(m_int32);
   if ( m_type == VAR_DOUBLE )
   {
      double val = MMath::Round(m_double);
      MENumberOutOfRange::Check(static_cast<double>(std::numeric_limits<Mint64>::min()), static_cast<double>(std::numeric_limits<Mint64>::max()), val);
      return static_cast<Mint64>(val);
   }
   if ( m_type == VAR_STRING || m_type == VAR_BYTE_STRING )
      return MToInt64(AsConstChars());
   if ( m_type == VAR_EMPTY )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
   return DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a numeric value")).AsInt64();
}

Muint64 MVariant::AsUInt64() const
{
   if ( m_type == VAR_UINT ) // most probable case is first
      return m_uint32;
   if ( m_type >= VAR_BOOL && m_type <= VAR_INT ) // most probable case is first
   {
      if ( m_int32 < 0 )
      {
         MChar str [ 64 ];
         MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert '%s' to unsigned integer"), MToChars(m_int32, str));
         M_ENSURED_ASSERT(0);
      }
      return static_cast<Muint64>(m_uint32);
   }
   if ( m_type == VAR_DOUBLE )
   {
      double val = MMath::Round(m_double);
      MENumberOutOfRange::Check(static_cast<double>(std::numeric_limits<Muint64>::min()), static_cast<double>(std::numeric_limits<Muint64>::max()), val);
      return static_cast<Muint64>(val);
   }
   if ( m_type == VAR_STRING || m_type == VAR_BYTE_STRING )
      return MToUInt64(AsConstChars());
   if ( m_type == VAR_EMPTY )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
   return DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a numeric value")).AsUInt64();
}

double MVariant::AsDouble() const
{
   if ( m_type == VAR_DOUBLE ) // most likely case
      return m_double;
   if ( m_type == VAR_INT )
      return static_cast<double>(m_int32);
   if ( m_type >= VAR_BOOL && m_type <= VAR_UINT )
      return static_cast<double>(m_uint32);
   if ( m_type == VAR_STRING || m_type == VAR_BYTE_STRING )
      return MToDouble(AsConstChars());
   if ( m_type == VAR_EMPTY )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
   return DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a numeric value")).AsDouble();
}

MByteString MVariant::AsByteString() const
{
   M_COMPILED_ASSERT(sizeof(long) == sizeof(unsigned long));

   MByteString result;
   switch ( m_type )
   {
   case VAR_EMPTY:
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   case VAR_BOOL:
      result.append((size_t)1, (m_int32 != 0) ? '\1' : '\0');
      break;
   case VAR_INT:
   case VAR_UINT: // we assume int and uint are of the same size
      result.assign((const char*)&m_int32, sizeof(Mint32));
      break;
   case VAR_DOUBLE:
      result.assign((const char*)&m_double, sizeof(double));
      break;
   case VAR_BYTE:
   case VAR_CHAR:
      result.append((size_t)1, static_cast<char>(m_uint32));
      break;
   case VAR_BYTE_STRING:
   case VAR_STRING:
      result.assign(AsConstChars(), m_int32);
      break;
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
   case VAR_MAP:
      result = DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a string value")).AsByteString();
      break;
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION: // special case, possibly an array of bytes
      {
         const int num = m_int32;
         for ( int i = 0; i < num; ++i )
            result += (MByteString::value_type)DoAccessVariantItem(i).AsByte();
         break;
      }
   default:
      M_ENSURED_ASSERT(0);
   }
   return result;
}

MStdString MVariant::AsString() const
{
   MStdString result;
   switch ( m_type )
   {
   case VAR_EMPTY:
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   case VAR_INT:
   case VAR_BOOL:
      result = MToStdString(m_int32);
      break;
   case VAR_UINT:
      result = MToStdString(m_uint32);
      break;
   case VAR_DOUBLE:
      result = MToStdString(m_double);
      break;
   case VAR_BYTE:
   case VAR_CHAR:
      result.append(1, static_cast<char>(m_uint32));
      break;
   case VAR_BYTE_STRING:
   case VAR_STRING:
      result.assign(AsConstChars(), m_int32);
      break;
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      #if M_NO_REFLECTION
         MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert variant containing object reference to a string value"));
         M_ENSURED_ASSERT(0);
      #else
         {
            const MObject* obj = AsExistingObject();
            if ( obj->IsPropertyPresent(s_asString) )
               return obj->GetProperty(s_asString).DoInterpretAsString();
            result = DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a string value")).AsString();
            break;
         }
      #endif
   case VAR_VARIANT_COLLECTION: // special case, possibly an array of chars
   case VAR_STRING_COLLECTION:  // allow this for collection of single characters
      {
         const int num = m_int32;
         for ( int i = 0; i < num; ++i )
            result += DoAccessVariantItem(i).AsChar();
         break;
      }
   case VAR_MAP:
      MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert map to a string value"));
   default:
      M_ENSURED_ASSERT(0);
   }
   return result; // we are never here, pacify compilers in debug mode
}

MSharedString MVariant::AsSharedString() const
{
   MSharedString result;
   char buffer [ 128 ];
   switch ( m_type )
   {
   case VAR_EMPTY:
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   case VAR_INT:
   case VAR_BOOL:
      result = MToChars(m_int32, buffer);
      break;
   case VAR_UINT:
      result = MToChars(m_uint32, buffer);
      break;
   case VAR_DOUBLE:
      result = MToChars(m_double, buffer, true);
      break;
   case VAR_BYTE:
   case VAR_CHAR:
      result.assign(1, static_cast<MChar>(m_uint32));
      break;
   case VAR_BYTE_STRING:
   case VAR_STRING:
      if ( m_bufferType == BUFFERTYPE_COPY )
         DoConvertInternalToSharedBuffer(m_uint32); // self-convert since we return the string
      result = DoAccessSharedString();
      break;
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      #if M_NO_REFLECTION
         MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert variant containing object reference to a string value"));
         M_ENSURED_ASSERT(0);
      #else
         {
            const MObject* obj = AsExistingObject();
            if ( obj->IsPropertyPresent(s_asString) )
               return obj->GetProperty(s_asString).AsSharedString();
            result = DoGetClientValueIfPresent(this, M_I("Could not convert variant containing object reference to a string value")).AsSharedString();
            break;
         }
      #endif
   case VAR_VARIANT_COLLECTION: // special case, possibly an array of chars
   case VAR_STRING_COLLECTION:  // allow this for collection of single characters
      {
         const int num = m_int32;
         for ( int i = 0; i < num; ++i )
            result += DoAccessVariantItem(i).AsChar();
         break;
      }
   case VAR_MAP:
      MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert map to a string value"));
   default:
      M_ENSURED_ASSERT(0);
   }
   return result; // we are never here, pacify compilers in debug mode
}

MStdString MVariant::AsString(unsigned mask) const
{
   MStdString result;
   switch ( m_type )
   {
   case VAR_CHAR:
      result = MStr::CharToEscapedString(static_cast<MChar>(m_uint32));
      break;
   default:
      result = MStr::ToString(AsString(), mask);
      break;
   }
   return result;
}

MStdString MVariant::AsEscapedString() const
{
   return AsString(MStr::StrNone);
}

MStdStringVector MVariant::AsStringCollection() const
{
   MStdStringVector result;
   if ( IsCollection() )
   {
      int count = m_int32;
      result.reserve(count);
      for ( int i = 0; i < count; ++i )
      {
         const MVariant& item = DoAccessVariantItem(i);
         if ( item.IsEmpty() )
            result.push_back(MStdString()); // by convention
         else
            result.push_back(item.AsString());
      }
   }
   else
      result.push_back(AsString()); // this will throw an error if variant is empty
   return result;
}

MVariant::VariantVector MVariant::AsVariantCollection() const
{
   VariantVector result;
   if ( IsCollection() )
   {
      int count = m_int32;
      result.reserve(count);
      for ( int i = 0; i < count; ++i )
      {
         const MVariant& item = DoAccessVariantItem(i);
         result.push_back(item);
      }
   }
   else if ( IsEmpty() )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
   else
      result.push_back(*this); // this will throw an error if variant is empty
   return result;
}

MByteStringVector MVariant::AsByteStringCollection() const
{
   MByteStringVector result;
   if ( IsCollection() )
   {
      int count = m_int32;
      result.reserve(count);
      for ( int i = 0; i < count; ++i )
      {
         const MVariant& item = DoAccessVariantItem(i);
         if ( item.IsEmpty() )
            result.push_back(MByteString()); // by convention
         else
            result.push_back(item.AsByteString());
      }
   }
   else
      result.push_back(AsByteString()); // this will throw an error if variant is empty
   return result;
}

MObject* MVariant::AsObject()
{
   if ( m_type == VAR_OBJECT )
      return m_object;
   if ( m_type == VAR_OBJECT_EMBEDDED ) // most probable case first
   {
      DoAccessSharedString().unshare();
      return m_object;
   }
   if ( m_type == VAR_EMPTY )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
   if ( (m_type != VAR_INT && m_type != VAR_UINT) || m_int32 != 0 )
   {
      MException::Throw(MException::ErrorSoftware, MErrorEnum::BadConversion, M_I("Could not convert variant to object reference"));
      M_ENSURED_ASSERT(0);
   }
   return NULL;
}

MObject* MVariant::AsExistingObject()
{
   MObject* obj = AsObject();
   if ( obj == NULL )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
   return obj;
}

void MVariant::Swap(MVariant& other) M_NO_THROW
{
   // We know that Variant is always movable in memory as a whole
   //
   Muint64 tmp = *(Muint64*)this;
   *(Muint64*)this = *(Muint64*)&other;
   *(Muint64*)&other = tmp;
   tmp = m_uint64;
   m_uint64 = other.m_uint64;
   other.m_uint64 = tmp;
}

void MVariant::MoveFrom(MVariant& other) M_NO_THROW
{
   DoCleanup();

   *(Muint64*)this = *(Muint64*)&other;
   m_uint64 = other.m_uint64;
   other.m_type = VAR_EMPTY;
   other.m_bufferType = BUFFERTYPE_NONE;
}

MVariant MVariant::Pow(const MVariant& a) const
{
   return MMath::Pow(AsDouble(), a.AsDouble());
}

void MVariant::AdjustIndex(int& index, unsigned count)
{
   M_ASSERT((int)count >= 0); // check that we are not too big (16-bit architecture possibility)
   int signedCount = int(count);
   MEIndexOutOfRange::Check(-signedCount, signedCount - 1, index);
   if ( index < 0 )
      index += signedCount;
}

MVariant MVariant::GetItem(const MVariant& idx) const
{
   MVariant result;
   if ( m_type == VAR_OBJECT || m_type == VAR_OBJECT_EMBEDDED )
   {
      #if M_NO_REFLECTION
         MException::ThrowCannotIndexItem();
         M_ENSURED_ASSERT(0);
      #else
         MObject* obj = const_cast<MVariant*>(this)->AsExistingObject();
         result.DoAssignToEmpty(obj->Call1(s_item, idx));
      #endif
   }
   else if ( m_type == VAR_MAP )
      result = AccessItem(idx);
   else
      result = GetItem(idx.AsInt());
   return result;
}

MVariant MVariant::GetItem(int index) const
{
   MVariant result;
   switch ( m_type )
   {
   case VAR_BYTE_STRING:
      {
         AdjustIndex(index, m_int32);
         result.DoAssignToEmpty((Muint8)AsConstChars()[index]);
         break;
      }
   case VAR_STRING:
      {
         AdjustIndex(index, m_int32);
         result.DoAssignToEmpty(AsConstChars()[index]);
         break;
      }
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      {
         AdjustIndex(index, m_int32);
         result.DoAssignToEmpty(DoAccessVariantItem(index));
         break;
      }
   case VAR_MAP:
      result = GetItem(MVariant(index)); // someone called GetItem for collection with int parameter from C++
      break;
   default:
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
   return result;
}

void MVariant::SetItem(const MVariant& index, const MVariant& value)
{
   switch ( m_type )
   {
#if !M_NO_REFLECTION
   case VAR_OBJECT:
      {
         MObject* obj = const_cast<MVariant*>(this)->AsExistingObject();
         obj->Call2(s_setItem, index, value);
      }
      break;
   case VAR_OBJECT_EMBEDDED:
      {
         DoAccessSharedString().unshare();
         MObject* obj = const_cast<MVariant*>(this)->AsExistingObject();
         obj->Call2(s_setItem, index, value);
      }
      break;
#endif
   case VAR_MAP:
      if ( m_int32 == 0 ) // easy case, optimize by not doing an auxiliary DoMakeCollectionUnique
      {
         m_int32 = 2;
         DoAccessSharedString().resize(2 * sizeof(MVariant));
         DoAccessVariantItem(0).DoAssignToEmpty(index);
         DoAccessVariantItem(1).DoAssignToEmpty(value);
      }
      else
      {
         DoMakeCollectionUnique();
         for ( int i = m_int32 - 2; ; i -= 2 )
         {
            if ( i < 0 )
            {
               m_int32 += 2;
               DoAccessSharedString().resize(m_int32 * sizeof(MVariant));
               DoAccessVariantItem(m_int32 - 2).DoAssignToEmpty(index);
               DoAccessVariantItem(m_int32 - 1).DoAssignToEmpty(value);
               break;
            }
            if ( DoAccessVariantItem(i) == index )
            {
               DoAccessVariantItem(i + 1) = value;
               break;
            }
         }
      }
      break;
   default:
      SetItem(index.AsInt(), value);
   }
}

void MVariant::SetItem(int index, const MVariant& value)
{
   switch ( m_type )
   {
   case VAR_BYTE_STRING:
   case VAR_STRING:
      {
         AdjustIndex(index, m_int32);
         char c = value.AsChar();
         if ( m_bufferType == BUFFERTYPE_COPY )
         {
            M_ASSERT(index < EMBEDDED_BUFFER_SIZE);
            m_placeholder[index] = c;
         }
         else
         {
            DoAccessSharedString().unshare();
            DoAccessSharedString()[index] = c;
         }
      }
      break;
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      AdjustIndex(index, m_int32);
      DoMakeCollectionUnique();
      DoAccessVariantItem(index) = value;
      break;
   case VAR_MAP:
      SetItem(MVariant(index), value); // if someone called this exact method from C++, bypassing SetItem(any, any)
      break;
   default:
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
}

const MVariant& MVariant::AccessItem(const MVariant& idx) const
{
   if ( m_type == VAR_MAP )
   {
      for ( int i = m_int32 - 2; ; i -= 2 )
      {
         if ( i < 0 )
         {
            char buff [ MException::MaximumVisibleParameterLength ];
            MException::Throw(MException::ErrorSoftware, M_CODE_STR_P1(M_ERR_ENTRY_NOT_FOUND, "Entry '%s' not found in the map", MException::VisualizeVariantParameter(buff, idx)));
            M_ENSURED_ASSERT(0);
         }
         if ( DoAccessVariantItem(i) == idx )
            return DoAccessVariantItem(i + 1);
      }
   }
   return AccessItem(idx.AsInt());
}

const MVariant& MVariant::AccessItem(int index) const
{
   if ( m_type == VAR_MAP )
      return AccessItem(MVariant(index)); // someone called GetItem for collection with int parameter from C++
   if ( m_type != VAR_VARIANT_COLLECTION && m_type != VAR_STRING_COLLECTION )
   {
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
   AdjustIndex(index, m_int32);
   return DoAccessVariantItem(index);
}

MVariant& MVariant::AccessItem(const MVariant& index)
{
   if ( m_type == VAR_MAP )
   {
      DoMakeCollectionUnique();
      for ( int i = m_int32 - 2; ; i -= 2 )
      {
         if ( i < 0 )
         {
            m_int32 += 2;
            DoAccessSharedString().resize(m_int32 * sizeof(MVariant));
            DoAccessVariantItem(m_int32 - 2).DoAssignToEmpty(index);
            return DoAccessVariantItem(m_int32 - 1);
         }
         if ( DoAccessVariantItem(i) == index )
            return DoAccessVariantItem(i + 1);
      }
   }
   return AccessItem(index.AsInt());
}

MVariant& MVariant::AccessItem(int index)
{
   if ( m_type == VAR_MAP )
      return AccessItem(MVariant(index)); // someone called GetItem for collection with int parameter from C++
   if ( m_type != VAR_VARIANT_COLLECTION && m_type != VAR_STRING_COLLECTION )
   {
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
   AdjustIndex(index, m_int32);
   DoMakeCollectionUnique();
   return DoAccessVariantItem(index);
}

void MVariant::SwapItems(int index1, int index2)
{
   switch ( m_type )
   {
   case VAR_BYTE_STRING:
   case VAR_STRING:
      {
         AdjustIndex(index1, m_int32);
         AdjustIndex(index2, m_int32);
         if ( m_bufferType == BUFFERTYPE_COPY )
         {
            M_ASSERT(index1 >= 0 && index1 < EMBEDDED_BUFFER_SIZE); // otherwise it would not be in-place
            char c = m_placeholder[index1];
            m_placeholder[index1] = m_placeholder[index2];
            m_placeholder[index2] = c;
         }
         else
         {
            MSharedString& str = DoAccessSharedString();
            str.unshare();
            char c = str[index1];
            str[index1] = str[index2];
            str[index2] = c;
         }
      }
      break;
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      AdjustIndex(index1, m_int32);
      AdjustIndex(index2, m_int32);
      if ( index1 != index2 )
      {
         DoMakeCollectionUnique();
         DoAccessVariantItem(index1).Swap(DoAccessVariantItem(index2));
      }
      break;
   default:
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
}

int MVariant::AdjustSlice(int& from, int& to, unsigned count)
{
   const int length = static_cast<int>(count);

   if ( from < 0 )
       from += length;
   if ( from < 0 )
      from = 0;
   if ( from >= length )
      from = length;

   if ( to < 0 )
      to += length;
   if ( to < 0 )
      to = -1; // here is an asymmetry in the way from and to are handled
   if ( to > length )
      to = length;

   int size = to - from;
   if ( size < 0 )
   {
      to = from;
      return 0;
   }
   return size;
}

MVariant MVariant::GetSlice(int from, int to) const
{
   MVariant result;
   switch ( m_type )
   {
   default:
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   case VAR_BYTE_STRING:
      {
         int size = AdjustSlice(from, to, m_int32);
         result.DoAssignByteStringToEmpty(AsConstChars() + from, size);
         break;
      }
   case VAR_STRING:
      {
         int size = AdjustSlice(from, to, m_int32);
         result.DoAssignToEmpty(AsConstChars() + from, size);
         break;
      }
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      {
         int size = AdjustSlice(from, to, m_int32);
         result.m_type = m_type;
         result.m_bufferType = BUFFERTYPE_REFCOUNT;
         result.m_int32 = size;
         result.DoCreateSharedBuffer(size * sizeof(MVariant));
         for ( int i = 0; i < size; ++i )
            result.DoAccessVariantItem(i).DoAssignToEmpty(DoAccessVariantItem(i + from));
         break;
      }
   }
   return result;
}

void MVariant::SetSlice(int from, int to, const MVariant& values)
{
   int size = AdjustSlice(from, to, m_int32);
   switch ( m_type )
   {
   case VAR_STRING:
   case VAR_BYTE_STRING:
      if ( m_bufferType == BUFFERTYPE_COPY )
         DoConvertInternalToSharedBuffer(m_int32); // since this is a rare operation, always do a shared buffer
      if ( values.IsEmpty() )
         DoAccessSharedString().erase(from, size);
      else if ( values.IsIndexed() )
      {
         const MByteString& s = values.AsByteString();
         DoAccessSharedString().replace(static_cast<MSharedString::size_type>(from), static_cast<MSharedString::size_type>(size), s.data(), static_cast<MSharedString::size_type>(s.size()));
      }
      else
         DoAccessSharedString().replace(from, size, 1, static_cast<char>(values.AsByte()));
      m_int32 = DoAccessSharedString().size();
      break;
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      for ( int i = from; i < to; ++i )     // first, discard previous variants
         DoAccessVariantItem(i).MVariant::~MVariant();
      if ( values.IsEmpty() )
         DoAccessSharedString().erase(from * sizeof(MVariant), size * sizeof(MVariant));
      else if ( values.IsCollection() )
      {
         int count = values.GetCount();
         char* buff = DoAccessSharedString()._replace_uninitialized(from * sizeof(MVariant), size * sizeof(MVariant), count * sizeof(MVariant));
         MVariant* vars = reinterpret_cast<MVariant*>(buff);
         for ( int i = 0; i < count; ++i )
            new((void*)(vars + i)) MVariant(values.GetItem(i));
      }
      else
      {
         char* buff = DoAccessSharedString()._replace_uninitialized(from * sizeof(MVariant), size * sizeof(MVariant), sizeof(MVariant));
         MVariant* var = reinterpret_cast<MVariant*>(buff);
         new((void*)var) MVariant(values);
      }
      m_int32 = DoAccessSharedString().size() / sizeof(MVariant);
      break;
   default:
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
}

#if !M_NO_REFLECTION
   static int DoCompareObjects(const MVariant& v1, const MVariant& v2, bool equality = false)
   {
      M_ASSERT(v1.IsObject() || v2.IsObject());
      if ( v1.IsObject() )
      {
         MObject* o1 = v1.AsObject();
         if ( o1 != NULL )
         {
            if ( v2.IsObject() && o1 == v2.AsObject() )
               return 0;
            if ( equality && !o1->IsServicePresent(s_compare) )
               return 1; // not equal
            return o1->Call1(s_compare, v2).AsInt();
         }
         // o1 == NULL
         if ( v2.IsEmpty() )
            return 0; // NULL object is equal to empty variant
         if ( !v2.IsObject() )
            return -1; // NULL object is less than any other value
      }
      if ( v2.IsObject() )
      {
         MObject* o2 = v2.AsObject();
         if ( o2 != NULL )
         {
            if ( equality && !o2->IsServicePresent(s_compare) )
               return 1; // not equal
            return -(o2->Call1(s_compare, v1).AsInt()); // we changed the order of the arguments => change the sign of the result
         }
         // o2 == NULL
         if ( v1.IsEmpty() )
            return 0; // empty variant is equal to NULL object
         if ( !v1.IsObject() )
            return 1; // non-object is more than NULL object
      }
      // v1.IsObject() && v2.IsObject()
      // both are NULL objects
      return 0;
   }
#endif

bool MVariant::operator==(const MVariant& v) const
{
   switch ( (m_type > v.m_type) ? m_type : v.m_type )
   {
   case VAR_EMPTY: // both variables are empty, comparison gives true
      return true;
   case VAR_BOOL:
      return AsBool() == v.AsBool();
   case VAR_BYTE:
   case VAR_CHAR:
   case VAR_UINT:
   case VAR_INT:
   case VAR_DOUBLE:
      return AsDouble() == v.AsDouble(); // works very well even for CHAR, UINT and INT, precision is not decreased
   case VAR_MAP:
      if ( m_type != v.m_type )
         return false;
      // otherwise fall through
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      {
         if ( !IsCollection() || !v.IsCollection() || m_uint32 != v.m_uint32 )
            return false;
         for ( int i = 0; i < m_int32; ++i ) // compare all elements (map has total index)
            if ( DoAccessVariantItem(i) != v.DoAccessVariantItem(i) )
               return false;
      }
      return true;
#if !M_NO_REFLECTION // otherwise fall into default branch
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      return DoCompareObjects(*this, v, true) == 0;
#endif
   default: // case VAR_STRING, also will throw No Value for VAR_EMPTY...
      if ( m_type == v.m_type )
         return m_int32 == v.m_int32 && memcmp(AsConstChars(), v.AsConstChars(), m_int32) == 0;
      else
         return AsSharedString() == v.AsSharedString();
   }
}

   template
      <class Vect>
   static bool DoVectorLess(const Vect& left, const Vect& right)
   {
      for ( unsigned i = 0 ; i < right.size() && i < left.size() ; ++i )
      {
         if ( left[i] < right[i] )
            return true;
         if ( left[i] > right[i] )
            return false;
      }
      return left.size() < right.size();
   }

bool MVariant::operator<(const MVariant& v) const
{
   switch ( (m_type > v.m_type) ? m_type : v.m_type )
   {
   case VAR_BOOL:
      return !AsBool() && v.AsBool();
   case VAR_BYTE:
   case VAR_CHAR:
   case VAR_UINT:
   case VAR_INT:
   case VAR_DOUBLE:
      return AsDouble() < v.AsDouble(); // works very well even for CHAR, UINT and INT, precision is not decreased
#if !M_NO_REFLECTION // otherwise fall into default branch
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      return DoCompareObjects(*this, v) < 0;
#endif
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      return DoVectorLess(AsVariantCollection(), v.AsVariantCollection());
   default: // case VAR_STRING, also will throw No Value for VAR_EMPTY...
      return AsString() < v.AsString();
   }
}

bool MVariant::operator>(const MVariant& v) const
{
   switch ( (m_type > v.m_type) ? m_type : v.m_type )
   {
   case VAR_BOOL:
      return AsBool() && !v.AsBool();
   case VAR_BYTE:
   case VAR_CHAR:
   case VAR_UINT:
   case VAR_INT:
   case VAR_DOUBLE:
      return AsDouble() > v.AsDouble(); // works very well even for CHAR, UINT and INT, precision is not decreased
#if !M_NO_REFLECTION // otherwise fall into default branch
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      return DoCompareObjects(*this, v) > 0;
#endif
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      return DoVectorLess(v.AsVariantCollection(), AsVariantCollection());
   default: // case VAR_STRING, also will throw No Value for VAR_EMPTY...
      return AsString() > v.AsString();
   }
}

   enum DoAndOrXorEnum
   {
      DoAnd = -1,
      DoOr  = 0,
      DoXor = 1
   };

   static void DoAndOrXorOnVariants(MVariant& result, const MVariant& v1, const MVariant& v2, DoAndOrXorEnum andOrXor)
   {
      const MByteString& self = v1.AsByteString();
      const MByteString& other = v2.AsByteString();
      if ( self.size() != other.size() )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_SIZES_OF_ITEMS_ARE_DIFFERENT_D1_AND_D2, M_I("Sizes of items are different, %d and %d"), self.size(), other.size());
         M_ENSURED_ASSERT(0);
      }
      MByteString resultByteString;
      MByteString::const_iterator it = self.begin();
      MByteString::const_iterator itEnd = self.end();
      MByteString::const_iterator otherIt = other.begin();
      if ( andOrXor < 0 )
      {
         M_ENSURED_ASSERT(andOrXor == DoAnd);
         for ( ; it != itEnd; ++it, ++otherIt )
            resultByteString += (MByteString::value_type)((unsigned char)*it & (unsigned char)*otherIt);
      }
      else if ( andOrXor > 0 )
      {
         M_ENSURED_ASSERT(andOrXor == DoXor);
         for ( ; it != itEnd; ++it, ++otherIt )
            resultByteString += (MByteString::value_type)((unsigned char)*it ^ (unsigned char)*otherIt);
      }
      else
      {
         M_ENSURED_ASSERT(andOrXor == DoOr);
         for ( ; it != itEnd; ++it, ++otherIt )
            resultByteString += (MByteString::value_type)((unsigned char)*it | (unsigned char)*otherIt);
      }
      result.DoAssignByteStringToEmpty(resultByteString);
   }

MVariant MVariant::operator|(const MVariant& v) const
{
   MVariant result;
   switch ( (m_type > v.m_type) ? m_type : v.m_type )
   {
   case VAR_BOOL:
   case VAR_OBJECT: // assume object's zero check
      result.DoAssignToEmpty((bool)(AsBool() || v.AsBool()));
      break;
   case VAR_BYTE:
      result.DoAssignToEmpty((Muint8)(AsByte() | v.AsByte()));
      break;
   case VAR_CHAR:
      result.DoAssignToEmpty((MChar)(AsChar() | v.AsChar()));
      break;
   case VAR_BYTE_STRING:
      DoAndOrXorOnVariants(result, *this, v, DoOr);
      break;
   case VAR_INT:
      result.DoAssignToEmpty(static_cast<int>(AsDWord() | v.AsDWord()));
      break;
   default: // also will throw No Value for VAR_EMPTY...
      result.DoAssignToEmpty(static_cast<unsigned>(AsDWord() | v.AsDWord()));
      break;
   }
   return result;
}

MVariant MVariant::operator&(const MVariant& v) const
{
   MVariant result;
   switch ( (m_type > v.m_type) ? m_type : v.m_type )
   {
   case VAR_BOOL:
   case VAR_OBJECT: // assume object's zero check
      result.DoAssignToEmpty((bool)(AsBool() && v.AsBool()));
      break;
   case VAR_BYTE:
      result.DoAssignToEmpty((Muint8)(AsByte() & v.AsByte()));
      break;
   case VAR_CHAR:
      result.DoAssignToEmpty((MChar)(AsChar() & v.AsChar()));
      break;
   case VAR_BYTE_STRING:
      DoAndOrXorOnVariants(result, *this, v, DoAnd);
      break;
   case VAR_INT:
      result.DoAssignToEmpty(static_cast<int>(AsDWord() & v.AsDWord())); // and make it back signed
      break;
   default: // will throw No Value for VAR_EMPTY...
      result.DoAssignToEmpty(static_cast<unsigned>(AsDWord() & v.AsDWord()));
      break;
   }
   return result;
}

MVariant MVariant::operator^(const MVariant& v) const
{
   MVariant result;
   switch ( (m_type > v.m_type) ? m_type : v.m_type )
   {
   case VAR_BOOL:
   case VAR_OBJECT: // assume object's zero check
      result.DoAssignToEmpty((bool)(AsBool() != v.AsBool()));
      break;
   case VAR_BYTE:
      result.DoAssignToEmpty((Muint8)(AsByte() ^ v.AsByte()));
      break;
   case VAR_CHAR:
      result.DoAssignToEmpty((MChar)(AsChar() ^ v.AsChar()));
      break;
   case VAR_BYTE_STRING:
      DoAndOrXorOnVariants(result, *this, v, DoXor);
      break;
   case VAR_INT:
      result.DoAssignToEmpty(static_cast<int>(AsDWord() ^ v.AsDWord())); // and make it a signed type
      break;
   default: // also will throw No Value for VAR_EMPTY...
      result.DoAssignToEmpty(static_cast<unsigned>(AsDWord() ^ v.AsDWord()));
      break;
   }
   return result;
}

MVariant MVariant::operator!() const
{
   MVariant result;
   switch ( m_type )
   {
   case VAR_EMPTY:
      result.DoAssignToEmpty(true); // by convention
      break;
   case VAR_BOOL:
   case VAR_OBJECT: // assume object's zero check
      result.DoAssignToEmpty((bool)!AsBool());
      break;
   case VAR_INT:
      result.DoAssignToEmpty(static_cast<int>(~AsDWord()));
      break;
   case VAR_BYTE:
      result.DoAssignToEmpty((Muint8)~AsByte());
      break;
   case VAR_CHAR:
      result.DoAssignToEmpty((MChar)~AsChar());
      break;
   default: // VAR_UINT, all the others....
      result.DoAssignToEmpty(static_cast<unsigned>(~AsDWord()));
      break;
   }
   return result;
}

MVariant MVariant::operator-() const
{
   switch ( m_type )
   {
   case VAR_UINT:
      return (m_int32) < 0
             ? MVariant(-static_cast<double>(m_uint32))
             : MVariant(-static_cast<Mint32>(m_uint32));
   case VAR_INT:
      return MVariant(-m_int32);
   case VAR_DOUBLE:
      return MVariant(-m_double);
   default:
      try
      {
         return MVariant(-AsInt());
      }
      catch ( ... )
      {
      }
   };

   return MVariant(-AsDouble());
}

   static MVariant DoReturnTyped(double result, MVariant::Type type1, MVariant::Type type2)
   {
      if ( type1 == MVariant::VAR_BOOL || type2 == MVariant::VAR_BOOL )
      {
         MException::ThrowNotSupportedForThisType();
         M_ENSURED_ASSERT(0);
      }
      switch ( (type1 > type2) ? type1 : type2 )
      {
      case MVariant::VAR_BYTE:
         if ( result >= 0.0 && result <= 256.0 ) // hard-code range here, do not use defines
            return (Muint8)result;
         break;
      case MVariant::VAR_CHAR:
         if ( result >= -128.0 && result <= 256.0 ) // hard-code range here, do not use defines
            return (MChar)(char)result;
         break;
      case MVariant::VAR_UINT:
         if ( result >= 0.0 && result <= (double)UINT_MAX )
            return (unsigned)result;
         break;
      case MVariant::VAR_INT:
         if ( result >= (double)INT_MIN && result <= (double)INT_MAX )
            return (int)result;
         break;
      default:
         break;
      }
      return result;
   }

MVariant MVariant::operator+(const MVariant& v) const
{
   MVariant result;
   switch ( (m_type > v.m_type) ? m_type : v.m_type )
   {
   case VAR_BYTE_STRING:
      result.DoAssignByteStringToEmpty(AsByteString() + v.AsByteString());
      break;
   case VAR_STRING:
      result.DoAssignToEmpty(AsString() + v.AsString());
      break;
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      if ( m_type == VAR_VARIANT_COLLECTION || m_type == VAR_STRING_COLLECTION )
      {
         result.DoAssignToEmpty(*this);
         result += v;
      }
      else
      {
         M_ASSERT(v.m_type == VAR_STRING_COLLECTION);
         result.DoAssignToEmpty(v);
         result += *this;
      }
      break;
   case VAR_MAP:
      if ( m_type == VAR_MAP )
      {
         result.DoAssignToEmpty(*this);
         result += v;
      }
      else
      {
         M_ASSERT(v.m_type == VAR_MAP);
         result.DoAssignToEmpty(v);
         result += *this;
      }
      break;
#if !M_NO_REFLECTION // otherwise fall into default branch
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      if ( IsObject() )
         result.DoAssignToEmpty(const_cast<MVariant*>(this)->AsExistingObject()->Call1(s_add, v));
      else
         result.DoAssignToEmpty(v.AsExistingObject()->Call1(s_add, *this));
      break;
#endif
   default:
      {
         double res = AsDouble() + v.AsDouble();
         result.DoAssignToEmpty(DoReturnTyped(res, GetType(), v.GetType()));
      }
      break;
   }
   return result;
}

MVariant& MVariant::operator+=(const MVariant& v)
{
   switch ( m_type )
   {
   case VAR_STRING:
   case VAR_BYTE_STRING:
      {
         if ( v.IsNumeric() )
         {
            char c = (char)v.AsByte();
            if ( m_bufferType == BUFFERTYPE_COPY )
            {
               if ( m_int32 < EMBEDDED_BUFFER_SIZE - 1 )
               {
                  m_placeholder[m_int32] = c;
                  m_int32 += 1;
                  return *this;
               }
               DoConvertInternalToSharedBuffer(m_int32 + 1);
            }
            DoAccessSharedString() += c;
            m_int32 += 1;
         }
         else
         {
            const MByteString& s = v.AsByteString();
            int newSize = static_cast<int>(m_int32 + s.size());
            if ( m_bufferType == BUFFERTYPE_COPY )
            {
               if ( newSize < EMBEDDED_BUFFER_SIZE - 1 )
               {
                  memcpy(m_placeholder + m_uint32, s.data(), s.size());
                  m_uint32 += static_cast<unsigned>(s.size());
                  return *this;
               }
               DoConvertInternalToSharedBuffer(newSize);
            }
            DoAccessSharedString().append(s.data(), static_cast<MSharedString::size_type>(s.size()));
            m_int32 += static_cast<int>(s.size());
         }
      }
      break;
   case VAR_STRING_COLLECTION:
   case VAR_VARIANT_COLLECTION:
      if ( v.IsCollection() )
      {
         int vCount = v.GetCount();
         int newCount = m_int32 + vCount;
         SetCount(newCount);
         for ( ; ; )
         {
            --vCount;
            if ( vCount < 0 )
               break;
            --newCount;
            DoAccessVariantItem(newCount).DoAssignToEmpty(v.DoAccessVariantItem(vCount));
         }
      }
      else
         AddToVariantCollection(v);
      break;
   case VAR_MAP:
      {
         Type type = v.GetType();
         if ( type == VAR_MAP ) // adding two maps together
         {
            const int count = v.m_int32;
            M_ASSERT((count & 0x01) == 0); // has to be an even number
            for ( int i = 0; i < count; i += 2 )
               SetItem(v.DoAccessVariantItem(i), v.DoAccessVariantItem(i + 1));
         }
         else if ( (type == VAR_STRING_COLLECTION || type == VAR_VARIANT_COLLECTION) && v.m_int32 == 2 ) // the only case when we can do +=
            SetItem(v.DoAccessVariantItem(0), v.DoAccessVariantItem(1));
         else
         {
            MException::Throw(MException::ErrorSoftware, M_CODE_STR(MErrorEnum::CannotIndexItem, "Cannot add item of this type to the map"));
            M_ENSURED_ASSERT(0);
         }
      }
      break;
   default: // Also handles NoValue condition:
      *this = *this + v;
      break;
   }
   return *this;
}

MVariant MVariant::operator-(const MVariant& v) const
{
   MVariant result;
#if !M_NO_REFLECTION
   if ( m_type == VAR_OBJECT || m_type == VAR_OBJECT_EMBEDDED )
      result = const_cast<MVariant*>(this)->AsExistingObject()->Call1(s_subtract, v);
   else // proceed with the other if choices
#endif

   if ( m_type == VAR_MAP )
   {
      result = *this;
      result -= v;
   }
   else if ( v.m_type == VAR_MAP )
   {
      result = v;
      result -= *this;
   }
   else
   {
      double res = AsDouble() - v.AsDouble();
      result = DoReturnTyped(res, GetType(), v.GetType());
   }
   return result;
}

MVariant& MVariant::operator-=(const MVariant& v)
{
   if ( m_type == VAR_STRING_COLLECTION || m_type == VAR_VARIANT_COLLECTION )
   {
      for ( int i = m_int32 - 1; i >= 0; --i )
      {
         MVariant& item = DoAccessVariantItem(i);
         if ( v == item )
         {
             item.MVariant::~MVariant();
             DoAccessSharedString().erase(i * sizeof(MVariant), sizeof(MVariant));
             --m_int32;
         }
      }
   }
   else if ( m_type == VAR_MAP )
   {
      if ( v.m_type == VAR_MAP )
      {
         for ( int i = v.m_int32 - 2; i >= 0; i -= 2 )
            operator-=(DoAccessVariantItem(i)); // recurse by every item
      }
      else
      {
         for ( int i = m_int32 - 2; i >= 0; i -= 2 )
         {
            MVariant& key = DoAccessVariantItem(i);
            if ( key == v )
            {
               DoMakeCollectionUnique();
               DoAccessVariantItem(i).MVariant::~MVariant();
               DoAccessVariantItem(i + 1).MVariant::~MVariant();
               DoAccessSharedString().erase(i *  sizeof(MVariant), 2 * sizeof(MVariant));
               m_int32 -= 2;
               break;
            }
         }
      }
   }
   else // Also handles NoValue condition:
      *this = *this - v;
   return *this;
}

MVariant MVariant::operator*(const MVariant& v) const
{
   MVariant result;
   switch ( (m_type > v.m_type) ? m_type : v.m_type )
   {
   case VAR_STRING:
   case VAR_BYTE_STRING:
      {
         const char* str;
         unsigned strSize;
         unsigned num;
         if ( m_type == VAR_STRING || m_type == VAR_BYTE_STRING )
         {
            str = AsConstChars();
            strSize = m_uint32;
            num = v.AsUInt();
         }
         else
         {
            M_ASSERT(v.m_type == VAR_BYTE_STRING || v.m_type == VAR_STRING);
            str = v.AsConstChars();
            strSize = v.m_uint32;
            num = AsUInt();
         }

         result.m_uint32 = strSize * num;
         result.m_type = m_type;
         char* p;
         if ( result.m_int32 < EMBEDDED_BUFFER_SIZE )
         {
            result.m_bufferType = BUFFERTYPE_COPY;
            p = result.m_placeholder;
         }
         else
         {
            result.m_bufferType = BUFFERTYPE_REFCOUNT;
            result.DoCreateSharedBuffer(result.m_uint32);
            p = result.m_bytes;
         }
         for ( unsigned i = num; i > 0; --i, p += strSize )
            memcpy(p, str, strSize);
      }
      break;
#if !M_NO_REFLECTION // otherwise fall into default branch
   case VAR_OBJECT:
   case VAR_OBJECT_EMBEDDED:
      if ( IsObject() )
         result = const_cast<MVariant*>(this)->AsExistingObject()->Call1(s_multiply, v);
      else
         result = v.AsExistingObject()->Call1(s_multiply, *this);
      break;
#endif
   default:
      {
         double res = AsDouble() * v.AsDouble();
         result = DoReturnTyped(res, GetType(), v.GetType());
      }
      break; // break to a double multiplication
   }
   return result;
}

MVariant MVariant::operator/(const MVariant& v) const
{
   #if !M_NO_REFLECTION // otherwise fall into default branch
      if ( m_type == VAR_OBJECT || m_type == VAR_OBJECT_EMBEDDED )
         return const_cast<MVariant*>(this)->AsExistingObject()->Call1(s_divide, v);
   #endif

   double divisor = v.AsDouble();
   if ( divisor == 0.0 )
   {
      MException::ThrowDivisionByZero();
      M_ENSURED_ASSERT(0);
   }
   double result = AsDouble() / divisor;
   return DoReturnTyped(result, GetType(), v.GetType());
}

MVariant MVariant::operator%(const MVariant& v) const
{
   if ( m_type == MVariant::VAR_BOOL || v.m_type == MVariant::VAR_BOOL )
   {
      MException::ThrowNotSupportedForThisType();
      M_ENSURED_ASSERT(0);
   }

   if ( m_type == VAR_DOUBLE || v.m_type == VAR_DOUBLE )
   {
      double val = v.AsDouble();
      if ( val == 0.0 )
      {
         MException::ThrowDivisionByZero();
         M_ENSURED_ASSERT(0);
      }
      return fmod(AsDouble(), val);
   }
   else
   {
      // this operator is assuming the positive second argument,
      // so the behavior depends only on the first argument

      int val = v.AsInt();
      if ( val == 0 )
      {
         MException::ThrowDivisionByZero();
         M_ENSURED_ASSERT(0);
      }

      if ( m_type == VAR_UINT || m_type == VAR_BYTE )
         return AsUInt() % val;
      else
         return AsInt() % val;
   }
}

MVariant& MVariant::operator*=(const MVariant& v)
{
   return *this = *this * v;
}

MVariant& MVariant::operator/=(const MVariant& v)
{
   return *this = *this / v;
}

MVariant& MVariant::operator%=(const MVariant& v)
{
   return *this = *this % v;
}

MVariant& MVariant::operator>>=(const MVariant& v)
{
   return *this = *this >> v;
}

MVariant& MVariant::operator<<=(const MVariant& v)
{
   return *this = *this << v;
}

MVariant& MVariant::operator|=(const MVariant& v)
{
   return *this = *this | v;
}

MVariant& MVariant::operator&=(const MVariant& v)
{
   return *this = *this & v;
}

MVariant& MVariant::operator^=(const MVariant& v)
{
   return *this = *this ^ v;
}

MVariant MVariant::operator<<(const MVariant& v) const
{
   // this operator is assuming the positive second argument,
   // so the behavior depends only on the first argument

   int val = v.AsInt();
   if ( m_type == VAR_UINT || m_type == VAR_BYTE )
      return AsUInt() << val;
   else
      return AsInt() << val;
}

MVariant MVariant::operator>>(const MVariant& v) const
{
   // this operator is assuming the positive second argument,
   // so the behavior depends only on the first argument

   int val = v.AsInt();
   if ( m_type == VAR_UINT || m_type == VAR_BYTE )
      return AsUInt() >> val;
   else
      return AsInt() >> val;
}

   const MChar OPERATOR_AUTOINCREMENT_STRING[] = "++";

MVariant& MVariant::operator++()
{
   switch ( m_type )
   {
   case VAR_EMPTY:
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
      // no need to break here
   case VAR_BOOL:
      M_ASSERT(m_uint32 <= 1);
      if ( m_uint32 != 0 )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_OVERFLOW_IN_OPERATION_S1, M_I("Overflow in operation '%s'"), OPERATOR_AUTOINCREMENT_STRING);
         M_ENSURED_ASSERT(0);
      }
      ++m_uint32;
      break;
   case VAR_BYTE:
      if ( m_uint32 >= UCHAR_MAX )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_OVERFLOW_IN_OPERATION_S1, M_I("Overflow in operation '%s'"), OPERATOR_AUTOINCREMENT_STRING);
         M_ENSURED_ASSERT(0);
      }
      ++m_uint32;
      break;
   case VAR_CHAR:
      if ( m_int32 >= s_MCHAR_MAX )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_OVERFLOW_IN_OPERATION_S1, M_I("Overflow in operation '%s'"), OPERATOR_AUTOINCREMENT_STRING);
         M_ENSURED_ASSERT(0);
      }
      ++m_int32;
      break;
   case VAR_INT:
      if ( m_int32 == INT_MAX )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_OVERFLOW_IN_OPERATION_S1, M_I("Overflow in operation '%s'"), OPERATOR_AUTOINCREMENT_STRING);
         M_ENSURED_ASSERT(0);
      }
      ++m_int32;
      break;
   case VAR_UINT:
      if ( m_uint32 == UINT_MAX )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_OVERFLOW_IN_OPERATION_S1, M_I("Overflow in operation '%s'"), OPERATOR_AUTOINCREMENT_STRING);
         M_ENSURED_ASSERT(0);
      }
      ++m_uint32;
      break;
   case VAR_DOUBLE:
      ++ m_double;
      break;
   default: // all the others....
      MException::ThrowNotSupportedForThisType();
      M_ENSURED_ASSERT(0);
   }
   return *this;
}

   const MChar OPERATOR_AUTODECREMENT_STRING[] = "--";

MVariant& MVariant::operator--()
{
   switch ( m_type )
   {
   case VAR_EMPTY:
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
      break;
   case VAR_BOOL:
   case VAR_BYTE:
   case VAR_CHAR:
   case VAR_UINT:
      if ( m_uint32 == 0 )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_UNDERFLOW_IN_OPERATION_S1, M_I("Underflow in operation '%s'"), OPERATOR_AUTODECREMENT_STRING);
         M_ENSURED_ASSERT(0);
      }
      --m_uint32;
      break;
   case VAR_INT:
      if ( m_int32 == INT_MIN )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_UNDERFLOW_IN_OPERATION_S1, M_I("Underflow in operation '%s'"), OPERATOR_AUTODECREMENT_STRING);
         M_ENSURED_ASSERT(0);
      }
      --m_int32;
      break;
   case VAR_DOUBLE:
      -- m_double;
      break;
   default: // all the others....
      MException::ThrowNotSupportedForThisType();
      M_ENSURED_ASSERT(0);
   }
   return *this;
}

MVariant& MVariant::DoSetInt(int value, Type type) M_NO_THROW
{
   M_ASSERT(type >= VAR_BOOL && type <= VAR_INT);
   DoSetType(type, BUFFERTYPE_NONE);
   m_int32 = value;
   return *this;
}

bool MVariant::IsPresent(const MVariant& v) const
{
   if ( v.IsCollection() ) // case when the parameter is collection
   {
      int i = v.GetCount() - 1;
      if ( v.m_type == VAR_MAP )
      {
         for ( ; i >= 0; --i )
         {
            const MVariant& key = v.GetMapKeyByIndex(i);
            if ( !IsPresent(key) ) // recursive call for each item in the parameter
               return false;
         }
      }
      else
      {
         for ( ; i >= 0; --i )
            if ( !IsPresent(v.GetItem(i)) ) // recursive call for each item in the parameter
               return false;
      }
      return true;
   }

   if ( IsIndexed() ) // case such as 'a' IN "ABCD" (but neither are collections)
      return FindIndexOf(v) >= 0;

   // otherwise both are not indexed
   return v == *this;
}

int MVariant::FindIndexOf(const MVariant& v, bool reverse) const
{
   if ( !IsIndexed() )
   {
      MException::ThrowCannotIndexItem();
      M_ENSURED_ASSERT(0);
   }
   if ( IsCollection() ) // if this is a collection but parameter is not
   {
      if ( m_type == VAR_MAP )
      {
         // For map, reverse does not apply, always do a reverse
         M_ASSERT((m_uint32 & 0x01) == 0);
         for ( int i = static_cast<int>(m_uint32 >> 1) - 1; i >= 0; --i )
            if ( GetMapKeyByIndex(i) == v )
               return i;
      }
      else if ( reverse )
      {
         for ( int i = m_int32 - 1; i >= 0; --i )
            if ( AccessItem(i) == v )
               return i;
      }
      else
      {
         const int count = m_int32;
         for ( int i = 0; i < count; ++i )
            if ( AccessItem(i) == v )
               return i;
      }
      return -1;
   }

   M_ASSERT(m_type == MVariant::VAR_STRING || m_type == MVariant::VAR_BYTE_STRING);

   const char* self = AsConstChars();
   const char* selfEnd = self + m_uint32;
   const MSharedString& what = v.AsSharedString();
   const char* result;
   if ( reverse )
      result = std::find_end(self, selfEnd, what.begin(), what.end());
   else
      result = std::search(self, selfEnd, what.begin(), what.end());
   if ( result == selfEnd )
   {
      if ( !what.empty() )
         return -1;

      // By convention, empty string is always present
      if ( reverse )
         return static_cast<int>(selfEnd - self);
      else
         return 0;
   }
   M_ASSERT(result >= self && result < selfEnd);
   return static_cast<int>(result - self);
}

void MVariant::AddToVariantCollection(const MVariant& v)
{
   if ( m_type != VAR_VARIANT_COLLECTION && m_type != VAR_STRING_COLLECTION )
   {
      MException::ThrowNotSupportedForThisType();
      M_ENSURED_ASSERT(0);
   }
   int oldCount = m_int32;
   SetCount(oldCount + 1);
   DoAccessVariantItem(oldCount).DoAssignToEmpty(v);
}

void MVariant::DoAssignToEmpty(MConstChars s)
{
   DoAssignToEmpty(s, static_cast<unsigned>(m_strlen(s)));
}

void MVariant::DoAssignToEmpty(MConstChars s, unsigned len)
{
   M_ASSERT(static_cast<int>(len) >= 0);
   M_ASSERT(m_type == VAR_EMPTY);
   m_type = VAR_STRING;
   m_int32 = static_cast<int>(len);
   if ( len < EMBEDDED_BUFFER_SIZE )
   {
      m_bufferType = BUFFERTYPE_COPY;
      memcpy(m_placeholder, s, len);
   }
   else
   {
      m_bufferType = BUFFERTYPE_REFCOUNT;
      DoCreateSharedBuffer(s, len);
   }
}

void MVariant::DoAssignToEmpty(const MStdString& s)
{
   DoAssignToEmpty(s.data(), static_cast<unsigned>(s.size()));
}

void MVariant::DoAssignToEmpty(const MStdStringVector& s)
{
   M_ASSERT(m_type == VAR_EMPTY);
   m_type = VAR_STRING_COLLECTION;
   m_bufferType = BUFFERTYPE_REFCOUNT;
   int size = static_cast<int>(s.size());
   m_int32 = size;
   DoCreateSharedBuffer(size * sizeof(MVariant)); // initializes with empty variants
   for ( int i = 0; i < size; ++i )
      DoAccessVariantItem(i).DoAssignToEmpty(s[i]);
}

void MVariant::DoAssignToEmpty(const VariantVector& v)
{
   M_ASSERT(m_type == VAR_EMPTY);
   m_type = VAR_VARIANT_COLLECTION;
   m_bufferType = BUFFERTYPE_REFCOUNT;
   int size = static_cast<int>(v.size());
   m_int32 = size;
   DoCreateSharedBuffer(size * sizeof(MVariant)); // initializes with empty variants
   for ( int i = 0; i < size; ++i )
      DoAccessVariantItem(i).DoAssignToEmpty(v[i]);
}

void MVariant::DoAssignToEmpty(const MVariant& other)
{
   M_ASSERT(m_type == VAR_EMPTY);
   *(Muint64*)this = *(Muint64*)&other; // copy first quadword
   if ( m_bufferType != BUFFERTYPE_NONE )
   {
      if ( m_bufferType == BUFFERTYPE_COPY )
         m_double = other.m_double; // copy the second quadword
      else // share the whole buffer
      {
         M_ASSERT(m_bufferType == BUFFERTYPE_REFCOUNT);
         new((void*)&m_pointerBytes) MSharedString(*(const MSharedString*)&other.m_pointerBytes);
      }
   }
}

void MVariant::DoAssignByteStringToEmpty(const MByteString& v)
{
   DoAssignByteStringToEmpty(v.data(), static_cast<unsigned>(v.size()));
}

void MVariant::DoAssignByteStringToEmpty(const char* bytes, unsigned size)
{
   M_ASSERT(static_cast<int>(size) >= 0);
   M_ASSERT(m_type == VAR_EMPTY);
   m_type = VAR_BYTE_STRING;
   m_int32 = static_cast<int>(size);
   if ( size < unsigned(EMBEDDED_BUFFER_SIZE) )
   {
      m_bufferType = BUFFERTYPE_COPY;
      memcpy(m_placeholder, bytes, size);
   }
   else
   {
      m_bufferType = BUFFERTYPE_REFCOUNT;
      DoCreateSharedBuffer(bytes, size);
   }
}

void MVariant::DoAssignByteStringCollectionToEmpty(const MByteStringVector& v)
{
   M_ASSERT(m_type == VAR_EMPTY);
   int size = static_cast<int>(v.size());
   m_type = VAR_VARIANT_COLLECTION;
   m_bufferType = BUFFERTYPE_REFCOUNT;
   m_int32 = size;
   DoCreateSharedBuffer(size * sizeof(MVariant)); // initializes with empty variants
   for ( int i = 0; i < size; ++i )
      DoAccessVariantItem(i).DoAssignByteStringToEmpty(v[i]);
}

void MVariant::DoAssignObjectEmbeddedToEmpty(const MObject* o)
{
   M_ASSERT(m_type == VAR_EMPTY);
   M_ASSERT(o != NULL);

   unsigned size = o->GetEmbeddedSizeof();
   M_ASSERT(size > 0); // true for embedded object
   DoCreateSharedBuffer((const char*)o, size); // initializes with empty variants
   m_type = VAR_OBJECT_EMBEDDED;
   m_bufferType = BUFFERTYPE_REFCOUNT;
}

void MVariant::DoMakeCollectionUnique()
{
   M_ASSERT(IsCollection());
   char* buff = DoAccessSharedString().unshare();
   if ( buff != NULL )
   {
      MVariant* it = reinterpret_cast<MVariant*>(buff);
      MVariant* last = it + m_int32;
      for ( ; it != last; ++it )
         if ( it->m_bufferType == BUFFERTYPE_REFCOUNT )
            it->DoAccessSharedString()._get_buffer()->_ref_increment();
   }
}

   static void DoCheckIfMap(MVariant::Type type)
   {
      if ( type != MVariant::VAR_MAP )
      {
         MException::Throw(MException::ErrorSoftware, M_CODE_STR(MErrorEnum::BadConversion, "Variant type is not a map"));
         M_ENSURED_ASSERT(0);
      }
   }

MVariant MVariant::DoGetAllMapItems(bool returnValues) const
{
   MVariant result;
   DoCheckIfMap(m_type);
   result.SetToNull(VAR_VARIANT_COLLECTION);
   const int count = m_int32;
   for ( int i = returnValues ? 1 : 0; i < count; i += 2 )
      result.AddToVariantCollection(DoAccessVariantItem(i));
   return result;
}

const MVariant& MVariant::DoGetMapItemByIndex(bool returnValues, int i) const
{
   DoCheckIfMap(m_type);
   M_ASSERT((m_uint32 & 0x01) == 0); // value is odd
   MEIndexOutOfRange::Check(0, m_uint32 >> 1, i); // adjust index here so it shows correct error when it fails
   i <<= 1; // adjust to map
   if ( returnValues )
      ++i;
   return DoAccessVariantItem(i);
}

bool MVariant::StaticIsObject(const MVariant* var)
{
   return var != NULL && var->IsObject();
}

#endif // !M_NO_VARIANT
