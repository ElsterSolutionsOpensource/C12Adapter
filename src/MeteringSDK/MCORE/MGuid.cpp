// File MCORE/MGuid.cpp

#include "MCOREExtern.h"
#include "MRandomGenerator.h"
#include "MUtilities.h"
#include "MGuid.h"
#include "MException.h"

   #if !M_NO_REFLECTION

      static MVariant DoNew0()
      {
         MGuid guid;
         return MVariant(&guid, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      static MVariant DoNew1(const MVariant& v)
      {
         MGuid guid;
         if ( v.IsObject() )
            guid = *M_DYNAMIC_CONST_CAST_WITH_THROW(MGuid, v.AsObject());
         else
            guid.SetAsString(v.AsString());
         return MVariant(&guid, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

   #endif

#if !M_NO_VARIANT
   M_COMPILED_ASSERT(sizeof(MGuid) <= sizeof(MVariant::ObjectByValue)); // we use this fact in implementation (shall be checked/changed for 64-bit environments)
#endif

M_START_PROPERTIES(Guid)
   M_OBJECT_PROPERTY_READONLY_BOOL_EXACT(Guid, IsNull)
   M_OBJECT_PROPERTY_STRING_EXACT       (Guid, AsString, ST_MStdString_X, ST_X_constMStdStringA)
M_START_METHODS(Guid)
   M_CLASS_SERVICE                      (Guid, Generate,                 ST_MObjectByValue_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED    (Guid, New,     DoNew0,       0, ST_MVariant_S)
   M_CLASS_FRIEND_SERVICE_OVERLOADED    (Guid, New,     DoNew1,       1, ST_MVariant_S_constMVariantA)
   M_OBJECT_SERVICE                     (Guid, Assign,                   ST_X_MObjectP)
   M_OBJECT_SERVICE                     (Guid, SetToNull,                ST_X)
   M_OBJECT_SERVICE                     (Guid, CheckIfNotNull,           ST_X)
   M_OBJECT_SERVICE                     (Guid, Compare,                  ST_int_X_MObjectP)
M_END_CLASS(Guid, Object)

void MGuid::SetToNull()
{
   memset(&m_value, 0, sizeof(m_value));
}

bool MGuid::IsNull() const
{
   for ( unsigned i = 0; i < M_NUMBER_OF_ARRAY_ELEMENTS(m_value.m_dwords); ++i )
      if ( m_value.m_dwords[i] != 0 )
         return false;
   return true;
}

void MGuid::CheckIfNotNull() const
{
   if ( IsNull() )
   {
      MException::ThrowNoValue();
      M_ENSURED_ASSERT(0);
   }
}

MGuid MGuid::Generate()
{
   MGuid result;
   MRandomGenerator randomGenerator;

   randomGenerator.GenerateBuffer((char*)result.m_value.m_bytes, sizeof(result.m_value));

   // According to rfc 4122,
   // First 4 bits of the 7-th octet should be 0100
   // (this means that we use a random algorithm)
   // and the first 2 bits of 9-th octet should be 10
   result.m_value.m_bytes[6] = (result.m_value.m_bytes[6] & 0x0F) | 0x40;
   result.m_value.m_bytes[8] = (result.m_value.m_bytes[8] & 0x3F) | 0x80;

   return result;
}

MStdString MGuid::AsString() const
{
   MStdString result;   // By convention, null GUID is representable by an empty string
   char buffer [ 64 ];
   MFormat(buffer, sizeof(buffer), "{%8.8X-%4.4X-%4.4X-%2.2X%2.2X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X}",
           static_cast<Muint32>(m_value.m_guid.Data1),
           static_cast<unsigned>(static_cast<Muint16>(m_value.m_guid.Data2)),
           static_cast<unsigned>(static_cast<Muint16>(m_value.m_guid.Data3)),
           static_cast<unsigned>(static_cast<Muint8>(m_value.m_guid.Data4[0])),
           static_cast<unsigned>(static_cast<Muint8>(m_value.m_guid.Data4[1])), 
           static_cast<unsigned>(static_cast<Muint8>(m_value.m_guid.Data4[2])), 
           static_cast<unsigned>(static_cast<Muint8>(m_value.m_guid.Data4[3])),
           static_cast<unsigned>(static_cast<Muint8>(m_value.m_guid.Data4[4])),
           static_cast<unsigned>(static_cast<Muint8>(m_value.m_guid.Data4[5])),
           static_cast<unsigned>(static_cast<Muint8>(m_value.m_guid.Data4[6])),
           static_cast<unsigned>(static_cast<Muint8>(m_value.m_guid.Data4[7])));
   result = buffer;
   return result;
}

void MGuid::SetAsString(const MStdString& str)
{
   if ( str.empty() )
      SetToNull();
   else if ( str.size() == 38 && str[0] == '{' && str[str.size() - 1] == '}' ) //checking the right format of str (length and the right position of '-' symbols
   {
       Muint32 p0;
       int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;
       int err = sscanf(str.c_str() + 1, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);
       if ( err != 11 )
       {
           MException::Throw(M_CODE_STR(M_ERR_BAD_GUID_FORMAT, M_I("Bad guid format")));
           M_ENSURED_ASSERT(0);
       }

       m_value.m_guid.Data1 = static_cast<Muint32>(p0);
       m_value.m_guid.Data2 = static_cast<Muint16>(p1);
       m_value.m_guid.Data3 = static_cast<Muint16>(p2);
       m_value.m_guid.Data4[0] = static_cast<char>(p3);
       m_value.m_guid.Data4[1] = static_cast<char>(p4);
       m_value.m_guid.Data4[2] = static_cast<char>(p5);
       m_value.m_guid.Data4[3] = static_cast<char>(p6);
       m_value.m_guid.Data4[4] = static_cast<char>(p7);
       m_value.m_guid.Data4[5] = static_cast<char>(p8);
       m_value.m_guid.Data4[6] = static_cast<char>(p9);
       m_value.m_guid.Data4[7] = static_cast<char>(p10);
   }
   else
   {
      MException::Throw(M_CODE_STR(M_ERR_BAD_GUID_FORMAT, M_I("Bad guid format")));
      M_ENSURED_ASSERT(0);
   }
}

void MGuid::Assign(const MGuid& other)
{
   M_DYNAMIC_CAST_WITH_THROW(MGuid, &other); // make sure the reflected call operates on Guid
   if ( this != &other )
      memcpy(m_value.m_bytes, other.m_value.m_bytes, sizeof(m_value));
}

MGuid& MGuid::operator=(const unsigned char* other)
{
   if ( m_value.m_bytes != other )
      memcpy(m_value.m_bytes, other, sizeof(m_value));
   return *this;
}

int MGuid::Compare(const MGuid& other)
{
   return memcmp(m_value.m_bytes, other.m_value.m_bytes, sizeof(m_value));
}

#if !M_NO_VARIANT
unsigned MGuid::GetEmbeddedSizeof() const
{
   M_COMPILED_ASSERT(sizeof(MGuid) <= sizeof(MVariant::ObjectByValue));
   return sizeof(MGuid);
}
#endif
