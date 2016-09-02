// File MCORE/MClass.cpp

#include "MCOREExtern.h"
#include "MClass.h"
#include "MObject.h"
#include "MException.h"
#include "MAlgorithm.h"
#include "MCriticalSection.h"

bool MClass::IsKindOf(const MClass* cls) const
{
   const MClass* c = this;
   M_ASSERT(c != NULL);
   do
   {
      if ( c == cls )
         return true;
      c = c->m_parent;
   } while ( c != NULL );
   return false;
}

bool MClass::StaticIsKindOf(const MObject* obj, const MClass* cls)
{
   return obj != NULL && obj->GetClass()->IsKindOf(cls);
}

#if M_NO_REFLECTION

M_NORETURN_FUNC void MClass::DoThrowCannotConvert(const MObject* from, const MClass* cls)
{
   MException::Throw(MException::ErrorSoftware, M_CODE_STR(M_ERR_COULD_NOT_CAST_OBJECT_OF_TYPE_S1_TO_S2, "Could not cast object"));
   M_ENSURED_ASSERT(0);
}

#else // M_NO_REFLECTION

   using namespace std;

   // Maximum number of classes.
   // Increase this number if not enough...
   //
   const int MAXIMUM_NUMBER_OF_CLASSES = 256;

   // Global Class Singleton is here.
   // We have to allow Classes watch for the ownership of it due to
   // order of initialization problems.
   //
   static const MClass* s_applicationClasses [ MAXIMUM_NUMBER_OF_CLASSES ];
   static const MClass** s_applicationClassesLast = s_applicationClasses;

#if !M_NO_FULL_REFLECTION // if we offer extensive information on service parameters

   static const MVariant::Type s_type_null[1] = {(MVariant::Type)0xFF};
   #define _M(st, type, call, par, ret, num)  static const MVariant::Type s_type_ ## st []  = par;
   #include "MObjectMethods.inc"
   #define _M(st, type, call, par, ret, num)  static const MVariant::Type s_type_ ## st []  = par;
   #include "MClassMethods.inc"

   // Array of parameter types
   //
   const MVariant::Type* s_parameterTypes[] =
      {
         s_type_null,
         #define _M(st, type, call, par, ret, num)  s_type_ ## st,
         #include "MObjectMethods.inc"
         #define _M(st, type, call, par, ret, num)  s_type_ ## st,
         #include "MClassMethods.inc"
      };

   // Array of numbers of parameters.
   // Unsigned char is used to save space in the array, but the real data are of unsigned type
   //
   const unsigned char s_numberOfParameters[] =
      {
         (const unsigned char)0xFF,
         #define _M(st, type, call, par, ret, num)  (const unsigned char)(num),
         #include "MObjectMethods.inc"
         #define _M(st, type, call, par, ret, num)  (const unsigned char)(num),
         #include "MClassMethods.inc"
      };

   // Array of service return values. If there is no return value, this is VAR_EMPTY.
   // Unsigned char is used to save space in the array, but the real data are of MVariant::Type type.
   //
   const unsigned char s_returnTypes[] =
      {
         (const unsigned char)0xFF,
         #define _M(st, type, call, par, ret, num)  (unsigned char)(ret),
         #include "MObjectMethods.inc"
         #define _M(st, type, call, par, ret, num)  (unsigned char)(ret),
         #include "MClassMethods.inc"
      };

const MVariant::Type* MServiceDefinition::GetParameterTypes() const
{
   M_ASSERT(m_type > 0 && m_type <= MClass::ST__CLASS_METHOD_LAST);
   M_ASSERT(m_type != MClass::ST__NULL);
   return s_parameterTypes[m_type];
}

unsigned MServiceDefinition::GetNumberOfParameters() const
{
   M_ASSERT(m_type > 0 && m_type <= MClass::ST__CLASS_METHOD_LAST);
   M_ASSERT(m_type != MClass::ST__NULL);
   M_ASSERT(m_overloadedNumberOfParameters < 0 || m_overloadedNumberOfParameters == s_numberOfParameters[m_type]);
   return (unsigned)s_numberOfParameters[m_type];
}

MVariant::Type MServiceDefinition::GetReturnType() const
{
   M_ASSERT(m_type > 0 && m_type <= MClass::ST__CLASS_METHOD_LAST);
   M_ASSERT(m_type != MClass::ST__NULL);
   return (MVariant::Type)s_returnTypes[m_type];
}

#endif

M_NORETURN_FUNC void MClass::DoThrowCannotConvert(const MObject* from, const MClass* cls)
{
   MConstChars fromName = (from == NULL) ? "NULL" : from->GetClass()->GetName();
   MException::Throw(MException::ErrorSoftware, M_ERR_COULD_NOT_CAST_OBJECT_OF_TYPE_S1_TO_S2, "Could not cast object of type '%s' to '%s'", fromName, cls->GetName());
   M_ENSURED_ASSERT(0);
}

MClass::RegisterClassHelper::RegisterClassHelper(const MClass* self)
{
   M_ASSERT(s_applicationClassesLast >= s_applicationClasses);
   M_ASSERT(s_applicationClassesLast < (s_applicationClasses + MAXIMUM_NUMBER_OF_CLASSES));
   *s_applicationClassesLast++ = self;
}

const MClass* MClass::GetParentClass(const MStdString& className) const
{
   for ( const MClass* pretender = m_parent; pretender != NULL; pretender = pretender->m_parent )
      if ( pretender->m_name == className )
         return pretender;
   return NULL;
}

const MClass* MClass::GetClass(const MStdString& name)
{
   for ( const MClass** it = s_applicationClasses; it != s_applicationClassesLast; ++it )
   {
      const MClass* cls = *it;
      if ( name == cls->m_name )
         return cls;
   }
   return NULL;
}

const MClass* MClass::GetExistingClass(const MStdString& className)
{
   const MClass* cls = GetClass(className);
   if ( cls == NULL )
   {
      MException::Throw(MErrorEnum::ClassNotFound, "Class %s not found", className.c_str());
      M_ENSURED_ASSERT(0);
   }
   return cls;
}

const MServiceDefinition* MClass::GetServiceDefinition(const MStdString& name, int expectedNumberOfParameters) const
{
   const MServiceDefinition* def = GetServiceDefinitionOrNull(name, expectedNumberOfParameters);
   if ( def == NULL )
   {
      if ( expectedNumberOfParameters != -1 && GetServiceDefinitionOrNull(name, -1) != NULL )
         DoThrowServiceDoesNotHaveNParameters(name, expectedNumberOfParameters);
      else
         DoThrowUnknownServiceException(m_typeName, name);
      M_ENSURED_ASSERT(0);
   }
   return def;
}

const MServiceDefinition* MClass::GetServiceDefinitionOrNull(const MStdString& name, int expectedNumberOfParameters) const
{
   // The below algorithm is optimized for fast searches of a string in array
   // It tries to compare integers, and also does a search for the first couple of bytes horizontally
   // The algorithm uses the restriction that all our services are at least 3 characters long

   M_COMPILED_ASSERT(sizeof(unsigned) <= 4); // otherwise would not work

   int dataSizeMinusUnsigned = (int)(name.size() + 1) * sizeof(MChar) - (int)sizeof(unsigned); // +1 to watch for the trailing zero (otherwise Read and ReadPartial would match)

   const char* data = (const char*)name.data();
   unsigned nameFirstU;
#if M_LITTLE_ENDIAN
   nameFirstU  = (unsigned)*data++;         // Data is not aligned for the majority of basic_string implementations...
   nameFirstU |= (unsigned)*data++ << 8;    // This way we can not use casting, should assemble the value with shifts
   nameFirstU |= (unsigned)*data++ << 16;   // to avoid alignment problem.
   nameFirstU |= (unsigned)*data++ << 24;   // The alternative: const unsigned nameFirstU = *(const unsigned*)data;
#else
   nameFirstU  = (unsigned)*data++ << 24;   // Data is not aligned for the majority of basic_string implementations...
   nameFirstU |= (unsigned)*data++ << 16;   // This way we can not use casting, should assemble the value with shifts
   nameFirstU |= (unsigned)*data++ << 8;    // to avoid alignment problem.
   nameFirstU |= (unsigned)*data++;         // The alternative: const unsigned nameFirstU = *(const unsigned*)data;
#endif

   for ( const MClass* cl = this; cl != NULL; cl = cl->m_parent )
   {
      if ( cl->m_services == NULL )
         continue;
      for ( const MServiceDefinition* def = cl->m_services; ; ++def )
      {
         const unsigned* candidate = (const unsigned*)def->m_name;
         M_ASSERT((reinterpret_cast<size_t>(candidate) & 0x3) == 0); // the algorithm assumes the data are aligned by doubleword
         const unsigned candidateFirstU = *candidate;
         if ( candidateFirstU == 0u ) // end of list
            break;
         if ( candidateFirstU == nameFirstU ) // matched first couple characters
         {                                    // try the rest
            if ( dataSizeMinusUnsigned <= 0 ) // service 3 or 4 characters long
            {
               if ( def->m_overloadedNumberOfParameters < 0 ||                           // if this is not an overloaded procedure
                    def->m_overloadedNumberOfParameters == expectedNumberOfParameters || // or if the number of parameters is the same
                    expectedNumberOfParameters < 0 )                                     // or if we do not care about number of parameters
               {
                  return def;
               }
            }
            else
            {
               ++candidate; // as we looked at first couple of bytes already...
               if ( memcmp(data, candidate, dataSizeMinusUnsigned) == 0 &&
                    (def->m_overloadedNumberOfParameters < 0 ||                           // if this is not an overloaded procedure
                     def->m_overloadedNumberOfParameters == expectedNumberOfParameters || // or if the number of parameters is the same
                     expectedNumberOfParameters < 0) )                                    // or if we do not care about number of parameters
               {
                  return def;
               }
            }
         }
      }
   }
   return NULL; // we are never here...
}


   // Tell if the given property name matches the candidate given.
   // This particular implementation does comparison with respect to MCOM syntax of
   // properties, which are all uppercased with word separation done with underscores.
   //
   // PRECONDITION: None
   //
   // POSTCONDITION: true is returned if two strings are the same names of the property.
   //
   inline bool DoComparePropertyNames(MConstChars name, MConstChars candidate)
   {
      M_ASSERT(name != NULL && *name != '\0' && candidate != NULL);

      // first proceed with the exact match until it works
      MConstChars c = name;
      for ( ; ;  ++c, ++candidate )
      {
         if ( *c == '\0' )
            return *candidate == '\0'; // possible exact match (if candidate also ends)
         if ( *c != *candidate )
            break; // exact mismatch failed
      }

      // Now see whether the mismatch is in the second char, and it is only in the case of the letter.
      // If it is, we need to proceed with MCOM syntax for properties
      //
      if ( c - 1 != name || m_toupper(*c) != *candidate )
         return false; // no match

      // Otherwise we have MCOM property name (which is case sensitive, but the matching is funny)
      //
      ++c;
      ++candidate;
      for ( ; *c != '\0'; ++c, ++candidate )
      {
         if ( m_isupper(*c) )
         {
            if ( *candidate++ != '_' || *candidate != *c ) // cures the case when *candidate == '\0'
               return false;
         }
         else if ( m_toupper(*c) != *candidate )
            return false;
      }
      return *candidate == '\0';
   }

const MPropertyDefinition* MClass::GetPropertyDefinitionOrNull(const MStdString& name) const
{
   // The below algorithm is optimized for fast searches of a string in array
   // It tries to compare integers, and also does a search for the first couple of bytes horizontally
   // The algorithm uses the restriction that all our services are at least 3 characters long

   M_COMPILED_ASSERT(sizeof(unsigned) == 4); // otherwise would not work, needs change

   const char* chars = name.c_str();
   unsigned dataSize = unsigned(name.size()); // valid property names are not 64-bit length
   if ( dataSize > 2 )
   {
      int dataSizeMinusUnsigned = (int)(dataSize + 1) * sizeof(MChar) - (int)sizeof(unsigned);   // +1 to watch for trailing zero (otherwise a User and UserId would match)
 
      const char* data = chars;
      unsigned nameFirstU = (unsigned)*data++; // Data is not aligned for the majority of basic_string implementations...
      nameFirstU |= (unsigned)*data++ << 8;    // This way we cannot use casting, should assemble the value with shifts
      nameFirstU |= (unsigned)*data++ << 16;   // to avoid alignment problem.
      nameFirstU |= (unsigned)*data++ << 24;   // The alternative: const unsigned nameFirstU = *(const unsigned*)data;

      if ( (nameFirstU & 0x20202020u) != 0 )   // This could be machine-dependent in a rare case if this is not ASCII...
      {                                        // This checks if there is any lowercase letter among first four characters.
                                               // In this case we can apply a much faster search algorithm of exact match search.
                                               // The speed improvement is four times, worth doing.
         for ( const MClass* cl = this; cl != NULL; cl = cl->m_parent )
         {
            if ( cl->m_properties == NULL )
               continue;
            for ( const MPropertyDefinition* def = cl->m_properties; ; ++def )
            {
               const unsigned* candidate = (const unsigned*)def->m_name;
               M_ASSERT((reinterpret_cast<size_t>(candidate) & 0x3) == 0); // the algorithm assumes the data are aligned by doubleword
               const unsigned candidateFirstU = *candidate;
               if ( candidateFirstU == 0u ) // end of list
                  break;
               if ( candidateFirstU == nameFirstU ) // matched first couple characters
               {                                    // try the rest
                  if ( dataSizeMinusUnsigned <= 0 ) // service 3 or 4 characters long
                     return def;
                  ++candidate; // as we looked at first couple of bytes already...
                  if ( memcmp(data, candidate, dataSizeMinusUnsigned) == 0 )
                     return def;
               }
            }
         }
      }
   }

   // otherwise compare by the algorithm, which assumes that "SomeProperty" is the same as "SAME_PROPERTY"
   //
   for ( const MClass* cl = this; cl != NULL; cl = cl->m_parent )
      for ( const MPropertyDefinition* def = cl->m_properties; def->m_name[0] != '\0'; ++def )
         if ( DoComparePropertyNames(def->m_name, chars) )
            return def;
   return NULL;
}

MStdStringVector MClass::GetAllClassNames()
{
   MStdStringVector result;
   for ( const MClass** it = s_applicationClasses; it != s_applicationClassesLast; ++it )
   {
      const MClass* cls = *it;
      result.push_back(cls->m_name);
   }
   return result;
}

MStdStringVector MClass::GetAllServiceNames() const
{
   MStdStringVector result;
   for ( const MClass* cl = this; cl != NULL; cl = cl->m_parent )
      for ( const MServiceDefinition* def = cl->m_services; def->m_name[0] != '\0'; ++def )
         result.push_back(def->m_name);
   MAlgorithm::InplaceSort(result, true);
   return result;
}

MStdStringVector MClass::GetOwnServiceNames() const
{
   MStdStringVector result;
   const MClass* cl = this;
   for ( const MServiceDefinition* def = cl->m_services; def->m_name[0] != '\0'; ++def )
      result.push_back(def->m_name);
   MAlgorithm::InplaceSort(result, true);
   return result;
}

MStdStringVector MClass::GetAllPropertyNames() const
{
   static const MChar s_propertyNamesStr[] = "AllPropertyNames";
   static const MChar s_persistentPropertyNamesStr[] = "AllPersistentPropertyNames";

   MStdStringVector result;
   for ( const MClass* cl = this; cl != NULL; cl = cl->m_parent )
      for ( const MPropertyDefinition* def = cl->m_properties; ; ++def )
      {  // attention, efficient code! Make use of knowledge of sizes. It is also unicode-safe.
         MConstChars name = def->m_name;
         if ( name[0] == '\0' )
            break; // break to outer loop
         if ( memcmp(name, s_propertyNamesStr, sizeof(s_propertyNamesStr)) != 0 &&
              memcmp(name, s_persistentPropertyNamesStr, sizeof(s_persistentPropertyNamesStr)) != 0 )
         {
            result.push_back(name);
         }
      }
   MAlgorithm::InplaceSort(result, true);
   return result;
}

MStdStringVector MClass::GetOwnPropertyNames() const
{
   static const MChar s_propertyNamesStr[] = "AllPropertyNames";
   static const MChar s_persistentPropertyNamesStr[] = "AllPersistentPropertyNames";

   MStdStringVector result;

   const MClass* cl = this;
   for ( const MPropertyDefinition* def = cl->m_properties; ; ++def )
   {  // attention, efficient code! Make use of knowledge of sizes. It is also unicode-safe.
      MConstChars name = def->m_name;
      if ( name[0] == '\0' )
         break; // break to outer loop
      if ( memcmp(name, s_propertyNamesStr, sizeof(s_propertyNamesStr)) != 0 &&
           memcmp(name, s_persistentPropertyNamesStr, sizeof(s_persistentPropertyNamesStr)) != 0 )
      {
         result.push_back(name);
      }
   }
   MAlgorithm::InplaceSort(result, true);
   return result;
}

MStdStringVector MClass::GetAllPersistentPropertyNames() const
{
   MStdStringVector result;
   for ( const MClass* cl = this; cl != 0; cl = cl->m_parent )
      for ( const MPropertyDefinition* def = cl->m_properties; def->m_name[0] != '\0'; ++def )
         if ( def->m_getObjectMethod != 0 && def->IsDefaultValuePresent() ) // persistent object property
            result.push_back(def->m_name); // note "AllPropertyNames" and "AllPersistentPropertyNames" are not persistent, no need to check
   MAlgorithm::InplaceSort(result, true);
   return result;
}

MVariant MClass::GetPersistentPropertyDefaultValue(const MStdString& name) const
{
   const MPropertyDefinition* def = GetPropertyDefinition(name);
   if ( !def->IsDefaultValuePresent() )
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_PROPERTY_S1_IS_NOT_PERSISTENT_AND_HAS_NO_DEFAULT_VALUE, "Property '%s' is not persistent and it has no default value", name.c_str());
      M_ENSURED_ASSERT(0);
   }
   return def->GetDefaultValue();
}

MVariant MClass::GetProperty(const MStdString& name) const
{
   MVariant result;
   const MPropertyDefinition* def = GetPropertyDefinition(name);
   M_ENSURED_ASSERT(def != 0);
   if ( def->m_getObjectMethod != 0 )
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_OBJECT_PROPERTY_S1_CANNOT_BE_GOT_FROM_A_CLASS_WITHOUT_OBJECT, "Object property '%s' cannot be got from a class, without object", name.c_str());
      M_ENSURED_ASSERT(0);
   }
   if ( def->m_getClassMethod != 0 )
   {
      switch ( def->m_type )
      {
      case MVariant::VAR_BOOL:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_bool_S);
         result.DoAssignToEmpty(((Method_bool_S)def->m_getClassMethod)());
         break;
      case MVariant::VAR_BYTE:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_byte_S);
         result.DoAssignToEmpty(((Method_byte_S)def->m_getClassMethod)());
         break;
      case MVariant::VAR_CHAR:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MChar_S);
         result.DoAssignToEmpty(((Method_MChar_S)def->m_getClassMethod)());
         break;
      case MVariant::VAR_INT:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_int_S);
         result.DoAssignToEmpty(((Method_int_S)def->m_getClassMethod)());
         break;
      case MVariant::VAR_UINT:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_unsigned_S);
         result.DoAssignToEmpty(((Method_unsigned_S)def->m_getClassMethod)());
         break;
      case MVariant::VAR_DOUBLE:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_double_S);
         result.DoAssignToEmpty(((Method_double_S)def->m_getClassMethod)());
         break;
      case MVariant::VAR_BYTE_STRING:
         if ( def->m_getServiceType == MClass::ST_constMByteStringA_S )
            result.DoAssignByteStringToEmpty(((Method_constMByteStringA_S)def->m_getClassMethod)());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MByteString_S);
            result.DoAssignByteStringToEmpty(((Method_MByteString_S)def->m_getClassMethod)());
         }
         break;
      case MVariant::VAR_STRING:
         if ( def->m_getServiceType == MClass::ST_constMStdStringA_S )
            result.DoAssignToEmpty(((Method_constMStdStringA_S)def->m_getClassMethod)());
         else if ( def->m_getServiceType == MClass::ST_MStdString_S )
            result.DoAssignToEmpty(((Method_MStdString_S)def->m_getClassMethod)());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MConstChars_S);
            result.DoAssignToEmpty(((Method_MConstChars_S)def->m_getClassMethod)());
         }
         break;
      case MVariant::VAR_STRING_COLLECTION:
         if ( def->m_getServiceType == MClass::ST_constMStdStringVectorA_S )
            result.DoAssignToEmpty(((Method_constMStdStringVectorA_S)def->m_getClassMethod)());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MStdStringVector_S);
            result.DoAssignToEmpty(((Method_MStdStringVector_S)def->m_getClassMethod)());
         }
         break;
      case MVariant::VAR_OBJECT:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MObjectP_S);
         result.DoAssignToEmpty(((Method_MObjectP_S)def->m_getClassMethod)());
         break;
      case MVariant::VAR_OBJECT_EMBEDDED:
         if ( def->m_getServiceType == MClass::ST_MObjectP_S )
            result.DoAssignObjectEmbeddedToEmpty(((Method_MObjectP_S)def->m_getClassMethod)());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MObjectByValue_S);
            result.DoAssignToEmpty(((Method_MObjectByValue_S)def->m_getClassMethod)());
         }
         break;
      case MVariant::VAR_VARIANT_COLLECTION:
      case MVariant::VAR_VARIANT:
         if ( def->m_getServiceType == MClass::ST_constMVariantA_S )
            result.DoAssignToEmpty(((Method_constMVariantA_S)def->m_getClassMethod)());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MVariant_S);
            result.DoAssignToEmpty(((Method_MVariant_S)def->m_getClassMethod)());
         }
         break;
      default:
         M_ENSURED_ASSERT(0);
      }
   }
   else
   {
      // Enumeration constant otherwise...
      //
      M_ENSURED_ASSERT(def->m_type == MVariant::VAR_EMPTY); // enumeration value has this precondition
      result.DoAssignToEmpty(static_cast<unsigned>(def->m_valueInt)); // enumerations are always unsigned
   }
   M_ASSERT(!result.IsEmpty() || def->m_type == MVariant::VAR_VARIANT); // the only way for a property to return an empty variant is to have it of Variant type
   return result;
}

void MClass::SetProperty(const MStdString& name, const MVariant& value) const
{
   const MPropertyDefinition* def = GetPropertyDefinition(name);
   M_ENSURED_ASSERT(def != 0);

   if ( def->m_setClassMethod != 0 )
   {
      switch ( def->m_type )
      {
      case MVariant::VAR_BOOL:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_bool);
         ((Method_S_bool)def->m_setClassMethod)(value.AsBool());
         break;
      case MVariant::VAR_BYTE:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_byte);
         ((Method_S_byte)def->m_setClassMethod)(value.AsByte());
         break;
      case MVariant::VAR_CHAR:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_MChar);
         ((Method_S_MChar)def->m_setClassMethod)(value.AsChar());
         break;
      case MVariant::VAR_INT:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_int);
         ((Method_S_int)def->m_setClassMethod)(value.AsInt());
         break;
      case MVariant::VAR_UINT:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_unsigned);
         ((Method_S_unsigned)def->m_setClassMethod)(value.AsUInt());
         break;
      case MVariant::VAR_DOUBLE:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_double);
         ((Method_S_double)def->m_setClassMethod)(value.AsDouble());
         break;
      case MVariant::VAR_BYTE_STRING:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_constMByteStringA);
         ((Method_S_constMByteStringA)def->m_setClassMethod)(value.AsByteString());
         break;
      case MVariant::VAR_STRING:
         if ( def->m_setServiceType == MClass::ST_S_constMStdStringA )
            ((Method_S_constMStdStringA)def->m_setClassMethod)(value.AsString());
         else
         {
            M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_MConstChars);
            ((Method_S_MConstChars)def->m_setClassMethod)(value.AsString().c_str());
         }
         break;
      case MVariant::VAR_STRING_COLLECTION:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_constMStdStringVectorA);
         ((Method_S_constMStdStringVectorA)def->m_setClassMethod)(value.AsStringCollection());
         break;
      case MVariant::VAR_OBJECT:
      case MVariant::VAR_OBJECT_EMBEDDED:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_MObjectP);
         ((Method_S_MObjectP)def->m_setClassMethod)(const_cast<MVariant&>(value).AsObject());
         break;
      case MVariant::VAR_VARIANT_COLLECTION:
      case MVariant::VAR_VARIANT:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_S_constMVariantA);
         ((Method_S_constMVariantA)def->m_setClassMethod)(value);
         break;
      default:
         M_ENSURED_ASSERT(0);
      }
   }
   else
   {
      if ( def->m_getClassMethod != 0 )
         MException::Throw(MException::ErrorSoftware, M_ERR_CANNOT_SET_READONLY_PROPERTY_S1, "Cannot set readonly property '%s'", name.c_str());
      else if ( def->m_getObjectMethod != 0 )
         MException::Throw(MException::ErrorSoftware, M_ERR_OBJECT_PROPERTY_S1_CANNOT_BE_SET_TO_A_CLASS_WITHOUT_OBJECT, "Object property '%s' cannot be set to a class, without object", name.c_str());
      else
         MException::Throw(MException::ErrorSoftware, M_ERR_ENUMERATION_S1_CANNOT_BE_ASSIGNED_TO, "Enumeration value '%s' cannot be assigned to", name.c_str());
      M_ENSURED_ASSERT(0);
   }
}

MVariant MClass::Call0(const MStdString& name) const
{
   return CallV(name, MVariant::VariantVector());
}

MVariant MClass::Call1(const MStdString& name, const MVariant& p1) const
{
   return CallV(name, MVariant::VariantVector(1, p1));
}

MVariant MClass::Call2(const MStdString& name, const MVariant& p1, const MVariant& p2) const
{
   MVariant::VariantVector vect(1, p1);
   vect.push_back(p2);
   return CallV(name, vect);
}

MVariant MClass::Call(const MStdString& name, const MVariant& params) const
{
   switch ( params.GetType() )
   {
   case MVariant::VAR_VARIANT_COLLECTION:
      return CallV(name, params.DoInterpretAsVariantCollection());
   case MVariant::VAR_EMPTY:
      return CallV(name, MVariant::VariantVector());
   default:
      return CallV(name, MVariant::VariantVector(1, params));
   }
}

   static M_NORETURN_FUNC void DoThrowCallException(const MServiceDefinition* def, const MStdString& name, int parametersCount)
   {
      if ( def->m_classMethod == 0 )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_SERVICE_S1_CANNOT_BE_CALLED_WITHOUT_OBJECT, "Service '%s' cannot be called without object", name.c_str());
         M_ENSURED_ASSERT(0);
      }
      else
      {
         MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
         M_ENSURED_ASSERT(0);
      }
   }

MVariant MClass::CallV(const MStdString& name, const MVariant::VariantVector& p) const
{
   MVariant result;
   const MServiceDefinition* def = GetServiceDefinition(name, int(p.size())); // p.size is the count of parameters
   M_ENSURED_ASSERT(def != 0);
   const int parametersCount = int(p.size());
   switch ( parametersCount )
   {
   case 0:
      switch ( def->m_type )
      {
         #define _M_PROCEDURE0(st, type, call)          case st: call; break;
         #define _M_FUNCTION0(st, type, call, par, ret) case st: call; break;
         #include "MClassMethods.inc"
      default:
         DoThrowCallException(def, name, parametersCount);
      }
      break;
   case 1:
      switch ( def->m_type )
      {
         #define _M_PROCEDURE1(st, type, call, par)     case st: call; break;
         #define _M_FUNCTION1(st, type, call, par, ret) case st: call; break;
         #include "MClassMethods.inc"
      default:
         {
            MConstChars nameStr = name.c_str();
            if ( m_strcmp(nameStr, "GetProperty") == 0 ) // special support for GetProperty call for a class directly
               result.DoAssignToEmpty(GetProperty(p[0].AsString()));
            else if ( m_strcmp(nameStr, "IsPropertyPresent") == 0 ) // special support for IsPropertyPresent call for a class directly
               result.DoAssignToEmpty(IsPropertyPresent(p[0].AsString()));
            else if ( m_strcmp(nameStr, "IsServicePresent") == 0 ) // special support for IsServicePresent call for a class directly
               result.DoAssignToEmpty(IsServicePresent(p[0].AsString()));
            else
               DoThrowCallException(def, name, parametersCount);
         }
      }
      break;
   case 2:
      switch ( def->m_type )
      {
         #define _M_PROCEDURE2(st, type, call, par)     case st: call; break;
         #define _M_FUNCTION2(st, type, call, par, ret) case st: call; break;
         #include "MClassMethods.inc"
      default:
         if ( m_strcmp(name.c_str(), "SetProperty") == 0 ) // special support for SetProperty call for a class directly
            SetProperty(p[0].AsString(), p[1]);
         else
            DoThrowCallException(def, name, parametersCount);
      }
      break;
   case 3:
      switch ( def->m_type )
      {
         #define _M_PROCEDURE3(st, type, call, par)     case st: call; break;
         #define _M_FUNCTION3(st, type, call, par, ret) case st: call; break;
         #include "MClassMethods.inc"
      default:
         DoThrowCallException(def, name, parametersCount);
      }
      break;
   case 4:
      switch ( def->m_type )
      {
         #define _M_PROCEDURE4(st, type, call, par)     case st: call; break;
         #define _M_FUNCTION4(st, type, call, par, ret) case st: call; break;
         #include "MClassMethods.inc"
      default:
         DoThrowCallException(def, name, parametersCount);
      }
      break;
   case 5:
      switch ( def->m_type )
      {
         #define _M_PROCEDURE5(st, type, call, par)     case st: call; break;
         #define _M_FUNCTION5(st, type, call, par, ret) case st: call; break;
         #include "MClassMethods.inc"
      default:
         DoThrowCallException(def, name, parametersCount);
      }
      break;
   case 6:
      switch ( def->m_type )
      {
         #define _M_PROCEDURE6(st, type, call, par)     case st: call; break;
         #define _M_FUNCTION6(st, type, call, par, ret) case st: call; break;
         #include "MClassMethods.inc"
      default:
         DoThrowCallException(def, name, parametersCount);
      }
      break;
   default:
      DoThrowCallException(def, name, parametersCount);
   }
   return result;
}

bool MClass::MatchesClassOrTypeName(const MStdString& name) const
{
   return name == m_name || name == m_typeName;
}

M_NORETURN_FUNC void MClass::DoThrowUnknownPropertyException(MConstChars className, const MStdString& name)
{
   MException::Throw(MException::ErrorSoftware, MErrorEnum::NoSuchProperty, "'%s' does not have property '%s'", className, name.c_str());
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MClass::DoThrowUnknownServiceException(MConstChars className, const MStdString& name)
{
   MException::Throw(MException::ErrorSoftware, MErrorEnum::NoSuchService, "'%s' does not have service '%s'", className, name.c_str());
   M_ENSURED_ASSERT(0);
}

M_NORETURN_FUNC void MClass::DoThrowServiceDoesNotHaveNParameters(const MStdString& name, int parametersCount)
{
   MException::Throw(MException::ErrorSoftware, M_ERR_SERVICE_S1_DOES_NOT_HAVE_D2_PARAMETERS, "Service '%s' does not have %d parameters", name.c_str(), parametersCount);
   M_ENSURED_ASSERT(0);
}

#endif
