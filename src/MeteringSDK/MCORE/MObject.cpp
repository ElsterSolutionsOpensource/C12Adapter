// File MCORE/MObject.cpp

#include "MCOREExtern.h"

#include "MObject.h"
#include "MDictionary.h"
#include "MException.h"

   using namespace std;

const MClass* MObject::GetClass() const
{
   return &s_class;
}

#if !M_NO_VARIANT
unsigned MObject::GetEmbeddedSizeof() const
{
   return 0u;
}
#endif

#if M_NO_REFLECTION

   const MClass MObject::s_class = {NULL};

#else // reflection is on

M_START_PROPERTIES(Object)
   M_OBJECT_PROPERTY_PERSISTENT_STRING         (Object, Type,                   "", ST_MConstChars_X, ST_X_constMStdStringA)
   M_OBJECT_PROPERTY_READONLY_STRING_COLLECTION(Object, AllPropertyNames,           ST_MStdStringVector_X)
   M_OBJECT_PROPERTY_READONLY_STRING_COLLECTION(Object, AllPersistentPropertyNames, ST_MStdStringVector_X)
M_START_METHODS(Object)
   M_OBJECT_SERVICE      (Object, GetProperty,                         ST_MVariant_X_constMStdStringA)
   M_OBJECT_SERVICE      (Object, SetProperty,                         ST_X_constMStdStringA_constMVariantA)
   M_OBJECT_SERVICE      (Object, GetPersistentPropertyDefaultValue,   ST_MVariant_X_constMStdStringA)
   M_OBJECT_SERVICE      (Object, Call,                                ST_MVariant_X_constMStdStringA_constMVariantA)                // would be better to be a class service...
   M_OBJECT_SERVICE      (Object, Call0,                               ST_MVariant_X_constMStdStringA)                               // would be better to be a class service...
   M_OBJECT_SERVICE      (Object, Call1,                               ST_MVariant_X_constMStdStringA_constMVariantA)                // would be better to be a class service...
   M_OBJECT_SERVICE      (Object, Call2,                               ST_MVariant_X_constMStdStringA_constMVariantA_constMVariantA) // would be better to be a class service...
   M_OBJECT_SERVICE      (Object, Validate,                            ST_X)
   M_OBJECT_SERVICE      (Object, IsPropertyPresent,                   ST_bool_X_constMStdStringA)     // would be better to be a class service...
   M_OBJECT_SERVICE      (Object, IsServicePresent,                    ST_bool_X_constMStdStringA)     // would be better to be a class service...
   M_CLASS_SERVICE       (Object, IsClassPresent,                      ST_bool_S_constMStdStringA)
{ "\0\0\0" } };  // Cannot use a macro because there is no parent

   const MClass MObject::s_class = {"Object", "Object", s__propertiesObject, s__servicesObject, NULL};
   static const MClass::RegisterClassHelper s__registerObject(&MObject::s_class);

bool MObject::IsPropertyPresent(const MStdString& name) const
{
   return GetClass()->IsPropertyPresent(name);
}

bool MObject::IsServicePresent(const MStdString& name) const
{
   return GetClass()->IsServicePresent(name);
}

MVariant MObject::GetProperty(const MStdString& name) const
{
   MVariant result;
   const MPropertyDefinition* def = GetClass()->GetPropertyDefinition(name);
   M_ASSERT(def != 0);
   if ( def->m_getObjectMethod != 0 )
   {
      switch ( def->m_type )
      {
      case MVariant::VAR_BOOL:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_bool_X);
         result.DoAssignToEmpty((this->*((Method_bool_X)def->m_getObjectMethod))());
         break;
      case MVariant::VAR_BYTE:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_byte_X);
         result.DoAssignToEmpty((this->*((Method_byte_X)def->m_getObjectMethod))());
         break;
      case MVariant::VAR_CHAR:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MChar_X);
         result.DoAssignToEmpty((this->*((Method_MChar_X)def->m_getObjectMethod))());
         break;
      case MVariant::VAR_INT:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_int_X);
         result.DoAssignToEmpty((this->*((Method_int_X)def->m_getObjectMethod))());
         break;
      case MVariant::VAR_UINT:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_unsigned_X);
         result.DoAssignToEmpty((this->*((Method_unsigned_X)def->m_getObjectMethod))());
         break;
      case MVariant::VAR_DOUBLE:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_double_X);
         result.DoAssignToEmpty((this->*((Method_double_X)def->m_getObjectMethod))());
         break;
      case MVariant::VAR_BYTE_STRING:
         if ( def->m_getServiceType == MClass::ST_constMByteStringA_X )
            result.DoAssignByteStringToEmpty((this->*((Method_constMByteStringA_X)def->m_getObjectMethod))());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MByteString_X);
            result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X)def->m_getObjectMethod))());
         }
         break;
      case MVariant::VAR_STRING:
         if ( def->m_getServiceType == MClass::ST_constMStdStringA_X )
            result.DoAssignToEmpty((this->*((Method_constMStdStringA_X)def->m_getObjectMethod))());
         else if ( def->m_getServiceType == MClass::ST_MStdString_X )
            result.DoAssignToEmpty((this->*((Method_MStdString_X)def->m_getObjectMethod))());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MConstChars_X);
            result.DoAssignToEmpty((this->*((Method_MConstChars_X)def->m_getObjectMethod))());
         }
         break;
      case MVariant::VAR_STRING_COLLECTION:
         if ( def->m_getServiceType == MClass::ST_constMStdStringVectorA_X )
            result.DoAssignToEmpty((this->*((Method_constMStdStringVectorA_X)def->m_getObjectMethod))());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MStdStringVector_X);
            result.DoAssignToEmpty((this->*((Method_MStdStringVector_X)def->m_getObjectMethod))());
         }
         break;
      case MVariant::VAR_OBJECT:
         M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MObjectP_X);
         result.DoAssignToEmpty((this->*((Method_MObjectP_X)def->m_getObjectMethod))());
         break;
      case MVariant::VAR_OBJECT_EMBEDDED:
         if ( def->m_getServiceType == MClass::ST_MObjectP_X )
            result.DoAssignObjectEmbeddedToEmpty((this->*((Method_MObjectP_X)def->m_getObjectMethod))());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MObjectByValue_X);
            result.DoAssignToEmpty((this->*((Method_MObjectByValue_X)def->m_getObjectMethod))());
         }
         break;
      case MVariant::VAR_VARIANT:
         if ( def->m_getServiceType == MClass::ST_constMVariantA_X )
            result.DoAssignToEmpty((this->*((Method_constMVariantA_X)def->m_getObjectMethod))());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_MVariant_X);
            result.DoAssignToEmpty((this->*((Method_MVariant_X)def->m_getObjectMethod))());
         }
         break;
      case MVariant::VAR_VARIANT_COLLECTION:
         if ( def->m_getServiceType == MClass::ST_MVariantVector_X )
            result.DoAssignToEmpty((this->*((Method_MVariantVector_X)def->m_getObjectMethod))());
         else
         {
            M_ENSURED_ASSERT(def->m_getServiceType == MClass::ST_constMByteStringVectorA_X);
            result.DoAssignByteStringCollectionToEmpty((this->*((Method_constMByteStringVectorA_X)def->m_getObjectMethod))());
         }
         break;
      default:
         M_ENSURED_ASSERT(0);
      }
   }
   else      // Otherwise marshal the class property or enumeration to the class
      result.DoAssignToEmpty(GetClass()->GetProperty(name));
   return result;
}

void MObject::SetProperty(const MStdString& name, const MVariant& value)
{
   const MPropertyDefinition* def = GetClass()->GetPropertyDefinition(name);
   M_ASSERT(def != 0);

   if ( def->m_setObjectMethod != 0 )
   {
      switch ( def->m_type )
      {
      case MVariant::VAR_BOOL:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_bool);
         (this->*((Method_X_bool)def->m_setObjectMethod))(value.AsBool());
         return;
      case MVariant::VAR_BYTE:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_byte);
         (this->*((Method_X_byte)def->m_setObjectMethod))(value.AsByte());
         return;
      case MVariant::VAR_CHAR:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_MChar);
         (this->*((Method_X_MChar)def->m_setObjectMethod))(value.AsChar());
         return;
      case MVariant::VAR_INT:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_int);
         (this->*((Method_X_int)def->m_setObjectMethod))(value.AsInt());
         return;
      case MVariant::VAR_UINT:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_unsigned);
         (this->*((Method_X_unsigned)def->m_setObjectMethod))(value.AsUInt());
         return;
      case MVariant::VAR_DOUBLE:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_double);
         (this->*((Method_X_double)def->m_setObjectMethod))(value.AsDouble());
         return;
      case MVariant::VAR_BYTE_STRING:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_constMByteStringA);
         (this->*((Method_X_constMByteStringA)def->m_setObjectMethod))(value.AsByteString());
         return;
      case MVariant::VAR_STRING:
         if ( def->m_setServiceType == MClass::ST_X_constMStdStringA )
            (this->*((Method_X_constMStdStringA)def->m_setObjectMethod))(value.AsString());
         else
         {
            M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_MConstChars);
            (this->*((Method_X_MConstChars)def->m_setObjectMethod))(value.AsString().c_str());
         }
         return;
      case MVariant::VAR_STRING_COLLECTION:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_constMStdStringVectorA);
         (this->*((Method_X_constMStdStringVectorA)def->m_setObjectMethod))(value.AsStringCollection());
         return;
      case MVariant::VAR_OBJECT:
      case MVariant::VAR_OBJECT_EMBEDDED:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_MObjectP);
         (this->*((Method_X_MObjectP)def->m_setObjectMethod))(const_cast<MVariant&>(value).AsObject());
         return;
      case MVariant::VAR_VARIANT_COLLECTION:
         if ( def->m_setServiceType == MClass::ST_X_constMVariantA )
            (this->*((Method_X_constMVariantA)def->m_setObjectMethod))(value);
         else
         {
            M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_constMByteStringVectorA);
            (this->*((Method_X_constMByteStringVectorA)def->m_setObjectMethod))(value.AsByteStringCollection());
         }
         return;
      case MVariant::VAR_VARIANT:
         M_ENSURED_ASSERT(def->m_setServiceType == MClass::ST_X_constMVariantA);
         (this->*((Method_X_constMVariantA)def->m_setObjectMethod))(value);
         return;
      default:
         M_ENSURED_ASSERT(0);
         return;
      }
   }
   if ( def->m_getObjectMethod == 0 ) // if we are dealing with the object property...
      GetClass()->SetProperty(name, value); // marshal to the class property
   else
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_CANNOT_SET_READONLY_PROPERTY_S1, "Cannot set readonly property '%s'", name.c_str());
      M_ENSURED_ASSERT(0);
   }
}

void MObject::SetType(const MStdString& name)
{
   MConstChars realType = GetType();
   if ( m_strcmp(name.c_str(), realType) != 0 && m_strcmp(name.c_str(), GetClass()->GetName()) != 0 )
   {
      MException::Throw(MException::ErrorSoftware, M_ERR_ATTEMPT_TO_CHANGE_OBJECT_TYPE_FROM_S1_TO_S2, M_I("Attempt to change object type from '%s' to '%s'"), realType, name.c_str());
      M_ENSURED_ASSERT(0);
   }
}

const char* MObject::GetType() const
{
   return GetClass()->GetTypeName();
}

void MObject::Validate()
{
}

void MObject::SetPersistentPropertyToDefault(const MStdString& name)
{
   SetProperty(name, GetPersistentPropertyDefaultValue(name));
}


MStdStringVector MObject::GetAllPropertyNames() const
{
   return GetClass()->GetAllPropertyNames();
}

MStdStringVector MObject::GetAllPersistentPropertyNames() const
{
   return GetClass()->GetAllPersistentPropertyNames();
}

MVariant MObject::GetPersistentPropertyDefaultValue(const MStdString& name) const
{
   return GetClass()->GetPersistentPropertyDefaultValue(name);
}

void MObject::SetPersistentPropertiesToDefault()
{
   MStdStringVector properties = GetAllPersistentPropertyNames();
   MStdStringVector::const_iterator it = properties.begin();
   MStdStringVector::const_iterator itEnd = properties.end();
   for ( ; it != itEnd; ++it )
   {
      MConstChars str = (*it).c_str();
      if ( m_stricmp(str, "Type") != 0 ) // avoid setting TYPE or Type to default
         SetPersistentPropertyToDefault(str);
   }
}

void MObject::DoSetPersistentPropertiesToDefault(const MClass* cl)
{
   for ( const MPropertyDefinition* def = cl->m_properties; def->m_name[0] != '\0'; ++def )
      if ( def->IsDefaultValuePresent() )
         SetPersistentPropertyToDefault(def->m_name);
}

MVariant MObject::Call0(const MStdString& name)
{
   return CallV(name, MVariant::VariantVector());
}

MVariant MObject::Call1(const MStdString& name, const MVariant& p1)
{
   return CallV(name, MVariant::VariantVector(1, p1));
}

MVariant MObject::Call2(const MStdString& name, const MVariant& p1, const MVariant& p2)
{
   MVariant::VariantVector vect(1, p1);
   vect.push_back(p2);
   return CallV(name, vect);
}

MVariant MObject::Call3(const MStdString& name, const MVariant& p1, const MVariant& p2, const MVariant& p3)
{
   MVariant::VariantVector vect(1, p1);
   vect.push_back(p2);
   vect.push_back(p3);
   return CallV(name, vect);
}

MVariant MObject::Call4(const MStdString& name, const MVariant& p1, const MVariant& p2, const MVariant& p3, const MVariant& p4)
{
   MVariant::VariantVector vect(1, p1);
   vect.push_back(p2);
   vect.push_back(p3);
   vect.push_back(p4);
   return CallV(name, vect);
}

MVariant MObject::Call5(const MStdString& name, const MVariant& p1, const MVariant& p2, const MVariant& p3, const MVariant& p4, const MVariant& p5)
{
   MVariant::VariantVector vect(1, p1);
   vect.push_back(p2);
   vect.push_back(p3);
   vect.push_back(p4);
   vect.push_back(p5);
   return CallV(name, vect);
}

MVariant MObject::Call6(const MStdString& name, const MVariant& p1, const MVariant& p2, const MVariant& p3, const MVariant& p4, const MVariant& p5, const MVariant& p6)
{
   MVariant::VariantVector vect(1, p1);
   vect.push_back(p2);
   vect.push_back(p3);
   vect.push_back(p4);
   vect.push_back(p5);
   vect.push_back(p6);
   return CallV(name, vect);
}

MVariant MObject::Call(const MStdString& name, const MVariant& params)
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

MVariant MObject::CallV(const MStdString& name, const MVariant::VariantVector& p)
{
   MVariant result;
   const MClass* cl = GetClass();
   int parametersCount = int(p.size());
   const MServiceDefinition* def = cl->GetServiceDefinition(name, parametersCount);
   M_ENSURED_ASSERT(def != 0);
   if ( def->m_objectMethod == 0 )
      result = cl->CallV(name, p);
   else
   {
      switch ( parametersCount )
      {
      case 0:
         switch ( def->m_type )
         {
            #define _M_PROCEDURE0(st, type, call)          case MClass::st: call; break;
            #define _M_FUNCTION0(st, type, call, par, ret) case MClass::st: call; break;
            #include "MObjectMethods.inc"
         default:
            MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
            M_ENSURED_ASSERT(0);
         }
         break;
      case 1:
         switch ( def->m_type )
         {
            #define _M_PROCEDURE1(st, type, call, par)     case MClass::st: call; break;
            #define _M_FUNCTION1(st, type, call, par, ret) case MClass::st: call; break;
            #include "MObjectMethods.inc"
         default:
            MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
            M_ENSURED_ASSERT(0);
         }
         break;
      case 2:
         switch ( def->m_type )
         {
            #define _M_PROCEDURE2(st, type, call, par)     case MClass::st: call; break;
            #define _M_FUNCTION2(st, type, call, par, ret) case MClass::st: call; break;
            #include "MObjectMethods.inc"
         default:
            MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
            M_ENSURED_ASSERT(0);
         }
         break;
      case 3:
         switch ( def->m_type )
         {
            #define _M_PROCEDURE3(st, type, call, par)     case MClass::st: call; break;
            #define _M_FUNCTION3(st, type, call, par, ret) case MClass::st: call; break;
            #include "MObjectMethods.inc"
         default:
            MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
            M_ENSURED_ASSERT(0);
         }
         break;
      case 4:
         switch ( def->m_type )
         {
            #define _M_PROCEDURE4(st, type, call, par)     case MClass::st: call; break;
            #define _M_FUNCTION4(st, type, call, par, ret) case MClass::st: call; break;
            #include "MObjectMethods.inc"
         default:
            MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
            M_ENSURED_ASSERT(0);
         }
         break;
      case 5:
         switch ( def->m_type )
         {
            #define _M_PROCEDURE5(st, type, call, par)     case MClass::st: call; break;
            #define _M_FUNCTION5(st, type, call, par, ret) case MClass::st: call; break;
            #include "MObjectMethods.inc"
         default:
            MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
            M_ENSURED_ASSERT(0);
         }
         break;
      case 6:
         switch ( def->m_type )
         {
            #define _M_PROCEDURE6(st, type, call, par)     case MClass::st: call; break;
            #define _M_FUNCTION6(st, type, call, par, ret) case MClass::st: call; break;
            #include "MObjectMethods.inc"
         default:
            MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
            M_ENSURED_ASSERT(0);
         }
         break;
      default:
         MClass::DoThrowServiceDoesNotHaveNParameters(name, parametersCount);
         M_ENSURED_ASSERT(0);
      }
   }
   return result;
}

#endif
