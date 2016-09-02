#ifndef MCORE_MREFLECTEDMACROS_H
#define MCORE_MREFLECTEDMACROS_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MReflectedMacros.h
///
/// Helper macros used in reflection.
///

#ifndef MCORE_MOBJECT_H
   #error "This header file has hard interdependency from reflection headers. Please include MCORE/MObject.h or MCORE/MCORE.h instead of this file."
#endif

/// This macro has to be defined in every MObject child at the end
/// of the class definitions (because after it all the definitions become protected).
/// See MObject class definition for details.
///
#define M_DECLARE_CLASS(Class) \
   public: virtual const MClass* GetClass() const; \
            static const MClass* GetStaticClass() { return &s_class; } \
            static const MClass s_class;

/// This macro has to be defined in every MObject child at the end
/// of the class definitions (because after it all the definitions become protected).
/// The name of the class is exact, without M prefix.
/// See MObject class definition for details.
///
#define M_DECLARE_CLASS_EXACT(Class) \
   public: virtual const MClass* GetClass() const; \
            static const MClass* GetStaticClass() { return &s_class; } \
            static const MClass s_class;

#if !M_NO_REFLECTION || defined(M_DOXYGEN)

   /// This macro has to be defined in every MObject child at the beginning
   /// of the class macro definitions. See MObject class definition for details.
   ///
   #define M_START_PROPERTIES(Class) \
      const MPropertyDefinition s__properties ## Class[] = {

   /// Define the enumeration value within a class.
   ///
   #define M_CLASS_ENUMERATION(Class, Enumeration) \
      {# Enumeration, MVariant::VAR_EMPTY, MClass::ST__NULL, MClass::ST__NULL, NULL, NULL, NULL, NULL, NULL, (int)(M ## Class::Enumeration)},

   /// Define the object Boolean persistent property with default value. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_BOOL(Class, Property, defaultValue) \
      {# Property, MVariant::VAR_BOOL, MClass::ST_bool_X, MClass::ST_X_bool, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)-1, (int)(defaultValue)},

   /// Define the object Boolean property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_BOOL(Class, Property) \
      {# Property, MVariant::VAR_BOOL, MClass::ST_bool_X, MClass::ST_X_bool, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object Boolean property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_BOOL_EXACT(Class, Property) \
      {# Property, MVariant::VAR_BOOL, MClass::ST_bool_X, MClass::ST_X_bool, \
       (MObject::Method)&M ## Class::Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object read-only Boolean property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_BOOL(Class, Property) \
      {# Property, MVariant::VAR_BOOL, MClass::ST_bool_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object read-only Boolean property with the exact name. See details in MObject class header comment.
   /// This one is suitable for properties like IsEmpty, IsReadOnly and so on.
   ///
   #define M_OBJECT_PROPERTY_READONLY_BOOL_EXACT(Class, Property) \
      {# Property, MVariant::VAR_BOOL, MClass::ST_bool_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Property, NULL, NULL, NULL, NULL},

   /// Define the class Boolean property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_BOOL(Class, Property) \
      {# Property, MVariant::VAR_BOOL, MClass::ST_bool_S, MClass::ST_S_bool, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, (MClass::Method)&M ## Class::Set ## Property, NULL},

   /// Define the class read-only Boolean property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_BOOL(Class, Property) \
      {# Property, MVariant::VAR_BOOL, MClass::ST_bool_S, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, NULL, NULL},

   /// Define the class read-only Boolean property with the exact name. See details in MObject class header comment.
   /// This one is suitable for properties like IsEmpty, IsReadOnly and so on.
   ///
   #define M_CLASS_PROPERTY_READONLY_BOOL_EXACT(Class, Property) \
      {# Property, MVariant::VAR_BOOL, MClass::ST_bool_S, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Property, NULL, NULL, NULL},

   /// Define the object Byte persistent property with default value. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_BYTE(Class, Property, defaultValue) \
      {# Property, MVariant::VAR_BYTE, MClass::ST_byte_X, MClass::ST_X_byte, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)-1, (int)(defaultValue)},

   /// Define the object Char persistent property with default value. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_CHAR(Class, Property, defaultValue) \
      {# Property, MVariant::VAR_CHAR, MClass::ST_MChar_X, MClass::ST_X_MChar, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)-1, (int)(defaultValue)},

   /// Define the object Byte property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_BYTE(Class, Property) \
      {# Property, MVariant::VAR_BYTE, MClass::ST_byte_X, MClass::ST_X_byte, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object Char property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_CHAR(Class, Property) \
      {# Property, MVariant::VAR_CHAR, MClass::ST_MChar_X, MClass::ST_X_MChar, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object Byte property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_BYTE(Class, Property) \
      {# Property, MVariant::VAR_BYTE, MClass::ST_byte_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object Char property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_CHAR(Class, Property) \
      {# Property, MVariant::VAR_CHAR, MClass::ST_MChar_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object Integer persistent property with default value. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_INT(Class, Property, defaultValue) \
      {# Property, MVariant::VAR_INT, MClass::ST_int_X, MClass::ST_X_int, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)-1, (int)(defaultValue)},

   /// Define the object Integer property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_INT(Class, Property) \
      {# Property, MVariant::VAR_INT, MClass::ST_int_X, MClass::ST_X_int, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object read-only Integer property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_INT(Class, Property) \
      {# Property, MVariant::VAR_INT, MClass::ST_int_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object read-only Integer property with exact name. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_INT_EXACT(Class, Property) \
      {# Property, MVariant::VAR_INT, MClass::ST_int_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Property, NULL, NULL, NULL, NULL},

   /// Define the object Unsigned Integer persistent property with default value. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_UINT(Class, Property, defaultValue) \
      {# Property, MVariant::VAR_UINT, MClass::ST_unsigned_X, MClass::ST_X_unsigned, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)-1, (int)(defaultValue)},

   /// Define the object Unsigned Integer property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_UINT(Class, Property) \
      {# Property, MVariant::VAR_UINT, MClass::ST_unsigned_X, MClass::ST_X_unsigned, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object read-only Unsigned Integer property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_UINT(Class, Property) \
      {# Property, MVariant::VAR_UINT, MClass::ST_unsigned_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the class Unsigned Integer property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_INT(Class, Property) \
      {# Property, MVariant::VAR_INT, MClass::ST_int_S, MClass::ST_S_int, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, (MClass::Method)&M ## Class::Set ## Property, NULL},

   /// Define the class read-only Unsigned Integer property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_INT(Class, Property) \
      {# Property, MVariant::VAR_INT, MClass::ST_int_S, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, NULL, NULL},

   /// Define the class Unsigned Integer property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_UINT(Class, Property) \
      {# Property, MVariant::VAR_UINT, MClass::ST_unsigned_S, MClass::ST_S_unsigned, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, (MClass::Method)&M ## Class::Set ## Property, NULL},

   /// Define the class read-only Unsigned Integer property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_UINT(Class, Property) \
      {# Property, MVariant::VAR_UINT, MClass::ST_unsigned_S, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, NULL, NULL},

   /// Define the object Double persistent property with default value. See details in MObject class header comment.
   ///
   /// Different from the other persistent properties, the default value for this one has to be a pointer to a static constant.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_DOUBLE(Class, Property, defaultValue) \
      {# Property, MVariant::VAR_DOUBLE, MClass::ST_double_X, MClass::ST_X_double, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)&defaultValue, 0},

   /// Define the object double precision property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_DOUBLE(Class, Property) \
      {# Property, MVariant::VAR_DOUBLE, MClass::ST_double_X, MClass::ST_X_double, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object read-only double precision property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_DOUBLE(Class, Property) \
      {# Property, MVariant::VAR_DOUBLE, MClass::ST_double_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object read-only double precision property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_DOUBLE(Class, Property) \
      {# Property, MVariant::VAR_DOUBLE, MClass::ST_double_S, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, NULL, NULL},

   /// Define the object byte string persistent property with default value. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(Class, Property, defaultValue, defaultValueLen, getMethodSignature, putMethodSignature) \
      {# Property, MVariant::VAR_BYTE_STRING, MClass::getMethodSignature, MClass::putMethodSignature, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)defaultValue, defaultValueLen},

   /// Define the object byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_BYTE_STRING(Class, Property, getMethodSignature, putMethodSignature) \
      {# Property, MVariant::VAR_BYTE_STRING, MClass::getMethodSignature, MClass::putMethodSignature, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object read-only byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_BYTE_STRING(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_BYTE_STRING, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object string persistent property with default value. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_STRING(Class, Property, defaultValue, getMethodSignature, putMethodSignature) \
      {# Property, MVariant::VAR_STRING, MClass::getMethodSignature, MClass::putMethodSignature, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)(MConstChars)(defaultValue)},

   /// Define the object byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_STRING(Class, Property, getMethodSignature, putMethodSignature) \
      {# Property, MVariant::VAR_STRING, MClass::getMethodSignature, MClass::putMethodSignature, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_STRING_EXACT(Class, Property, getMethodSignature, putMethodSignature) \
      {# Property, MVariant::VAR_STRING, MClass::getMethodSignature, MClass::putMethodSignature, \
       (MObject::Method)&M ## Class::Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object read-only byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_STRING(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_STRING, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object read-only byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_STRING_EXACT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_STRING, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Property, NULL, NULL, NULL, NULL},

   /// Define the class String property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_STRING(Class, Property, getMethodSignature, putMethodSignature) \
      {# Property, MVariant::VAR_STRING, MClass::getMethodSignature, MClass::putMethodSignature, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, (MClass::Method)&M ## Class::Set ## Property, NULL},

   /// Define the class read-only String property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_STRING(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_STRING, MClass::getMethodSignature, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, NULL, NULL},

   /// Define the object byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_STRING_COLLECTION(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_STRING_COLLECTION, MClass::getMethodSignature, MClass::ST_X_constMStdStringVectorA, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_BYTE_STRING_COLLECTION(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_VARIANT_COLLECTION, MClass::getMethodSignature, MClass::ST_X_constMByteStringVectorA, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object read-only byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_STRING_COLLECTION(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_STRING_COLLECTION, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object read-only byte string property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_VARIANT_COLLECTION(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_VARIANT_COLLECTION, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object property of type MVariant. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_VARIANT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_VARIANT, MClass::getMethodSignature, MClass::ST_X_constMVariantA, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object variant persistent property with default value. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_PERSISTENT_VARIANT(Class, Property, defaultValue, getMethodSignature, putMethodSignature) \
      {# Property, MVariant::VAR_VARIANT, MClass::getMethodSignature, MClass::putMethodSignature, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, (void*)-1, (int)(defaultValue)},

   /// Define the object property of type MVariant. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_VARIANT_EXACT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_VARIANT, MClass::getMethodSignature, MClass::ST_X_constMVariantA, \
       (MObject::Method)&M ## Class::Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object read-only variant property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_VARIANT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_VARIANT, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object read-only variant property, exact name. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_VARIANT_EXACT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_VARIANT, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Property, NULL, NULL, NULL, NULL},

   /// Define the embedded read-only class property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_VARIANT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_VARIANT, MClass::getMethodSignature, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, NULL, NULL},

   /// Define the embedded read-only class property with the exact name. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_VARIANT_EXACT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_VARIANT, MClass::getMethodSignature, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Property, NULL, NULL, NULL},

   /// Define the object property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_OBJECT(Class, Property) \
      {# Property, MVariant::VAR_OBJECT, MClass::ST_MObjectP_X, MClass::ST_X_MObjectP, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the object property with the explicit name. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_OBJECT_OVERLOADED(Class, Property, GetPropertyImpl, SetPropertyImpl) \
      {# Property, MVariant::VAR_OBJECT, MClass::ST_MObjectP_X, MClass::ST_X_MObjectP, \
       (MObject::Method)&M ## Class::GetPropertyImpl, NULL, (MObject::Method)&M ## Class::SetPropertyImpl, NULL, NULL},

   /// Define the object read-only object property. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_OBJECT(Class, Property) \
      {# Property, MVariant::VAR_OBJECT, MClass::ST_MObjectP_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the object read-only object property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_OBJECT(Class, Property) \
      {# Property, MVariant::VAR_OBJECT, MClass::ST_MObjectP_S, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, NULL, NULL},

   /// Define the object read-only object property with the exact name. See details in MObject class header comment.
   ///
   #define M_OBJECT_PROPERTY_READONLY_OBJECT_EXACT(Class, Property) \
      {# Property, MVariant::VAR_OBJECT, MClass::ST_MObjectP_X, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Property, NULL, NULL, NULL, NULL},

   /// Define the embedded object property. See details in MObject class header comment.
   /// Get method signature is most often ST_MObject_X, but it can be ST_MObjectByValue_X.
   ///
   #define M_OBJECT_PROPERTY_OBJECT_EMBEDDED(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_OBJECT_EMBEDDED, MClass::getMethodSignature, MClass::ST_X_MObjectP, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, (MObject::Method)&M ## Class::Set ## Property, NULL, NULL},

   /// Define the embedded read-only object property. See details in MObject class header comment.
   /// Get method signature is most often ST_MObject_X, but it can be ST_MObjectByValue_X.
   ///
   #define M_OBJECT_PROPERTY_READONLY_OBJECT_EMBEDDED(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_OBJECT_EMBEDDED, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Get ## Property, NULL, NULL, NULL, NULL},

   /// Define the embedded read-only object property with the exact name. See details in MObject class header comment.
   /// Get method signature is most often ST_MObject_X, but it can be ST_MObjectByValue_X.
   ///
   #define M_OBJECT_PROPERTY_READONLY_OBJECT_EMBEDDED_EXACT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_OBJECT_EMBEDDED, MClass::getMethodSignature, MClass::ST__NULL, \
       (MObject::Method)&M ## Class::Property, NULL, NULL, NULL, NULL},

   /// Define the embedded read-only object property with the exact name. See details in MObject class header comment.
   /// Get method signature is most often ST_MObject_S, but it can be ST_MObjectByValue_S.
   ///
   #define M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_OBJECT_EMBEDDED, MClass::getMethodSignature, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Get ## Property, NULL, NULL, NULL},

   /// Define the embedded read-only object property with the exact name. See details in MObject class header comment.
   /// Get method signature is most often ST_MObject_S, but it can be ST_MObjectByValue_S.
   ///
   #define M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED_EXACT(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_OBJECT_EMBEDDED, MClass::getMethodSignature, MClass::ST__NULL, \
       NULL, (MClass::Method)&M ## Class::Property, NULL, NULL, NULL},

   /// Define the object read-only byte string property. See details in MObject class header comment.
   ///
   #define M_CLASS_PROPERTY_READONLY_STRING_COLLECTION(Class, Property, getMethodSignature) \
      {# Property, MVariant::VAR_STRING_COLLECTION, MClass::getMethodSignature, MClass::ST__NULL, \
       NULL, (MClass::Method)M ## Class::Get ## Property, NULL, NULL, NULL},

   /// This macro has to be defined in every MObject child in the class macro definitions. 
   /// See MObject class definition for details.
   ///
   #define M_START_METHODS(Class) \
      {""} }; \
      const MServiceDefinition s__services ## Class[] = {

   /// Define the object service. See details in MObject class header comment.
   ///
   #define M_OBJECT_SERVICE(Class, Service, serviceType) \
      {# Service, -1, MClass::serviceType, (MObject::Method)&M ## Class::Service, NULL},

   /// Define the object service with a name different than the implementation.
   /// See details in MObject class header comment.
   ///
   #define M_OBJECT_SERVICE_NAMED(Class, Service, ServiceImpl, serviceType) \
      {# Service, -1, MClass::serviceType, (MObject::Method)&M ## Class::ServiceImpl, NULL},

   /// Define the object service with the explicit name and number of parameters.
   /// This is to help implement services with default parameters.
   /// See details in MObject class header comment.
   ///
   #define M_OBJECT_SERVICE_OVERLOADED(Class, Service, ServiceImpl, numParameters, serviceType) \
      {# Service, numParameters, MClass::serviceType, (MObject::Method)&M ## Class::ServiceImpl, NULL},

   /// Define the class service. See details in MObject class header comment.
   ///
   #define M_CLASS_SERVICE(Class, Service, serviceType) \
      {# Service, -1, MClass::serviceType, NULL, (MClass::Method)&M ## Class::Service},

   /// Define the class service with the explicit name and number of parameters.
   /// This is to help implement services with default parameters.
   /// See details in MObject class header comment.
   ///
   #define M_CLASS_SERVICE_OVERLOADED(Class, Service, ServiceImpl, numParameters, serviceType) \
      {# Service, numParameters, MClass::serviceType, NULL, (MClass::Method)&M ## Class::ServiceImpl},

   /// Define the friend class service. See details in MObject class header comment.
   ///
   #define M_CLASS_FRIEND_SERVICE(Class, Service, FriendService, serviceType) \
      {# Service, -1, MClass::serviceType, NULL, (MClass::Method)&FriendService},

   /// Define the class service with the explicit name and number of parameters.
   /// This is to help implement services with default parameters.
   /// See details in MObject class header comment.
   ///
   #define M_CLASS_FRIEND_SERVICE_OVERLOADED(Class, Service, FriendService, numParameters, serviceType) \
      {# Service, numParameters, MClass::serviceType, NULL, (MClass::Method)&FriendService},

   /// Complete the definitions for the class. This macro appears in the implementation file.
   ///
   #define M_END_CLASS(Class, Parent) \
      {"\0\0\0"} }; \
      const MClass M ## Class::s_class = {# Class, # Class, s__properties ## Class, s__services ## Class, &(M ## Parent::s_class)}; \
      static const MClass::RegisterClassHelper s__register ## Class(&M ## Class::s_class); \
      const MClass* M ## Class::GetClass() const { return &s_class; }

   /// Complete the definitions for the class with the exact given name.
   /// Note that the parent has to have an exact name too.
   /// This macro appears in the implementation file.
   ///
   #define M_END_CLASS_EXACT(Class, Parent) \
      {"\0\0\0"} }; \
      const MClass Class::s_class = {# Class, # Class, s__properties ## Class, s__services ## Class, &(Parent::s_class)}; \
      static const MClass::RegisterClassHelper s__register ## Class(&Class::s_class); \
      const MClass* Class::GetClass() const { return &s_class; }

   /// Complete the definitions for the typed class (one which type name is different from class name.
   /// This macro appears in the implementation file.
   ///
   #define M_END_CLASS_TYPED(Class, Parent, Type) \
      {"\0\0\0"} }; \
      const MClass M ## Class::s_class = {# Class, Type, s__properties ## Class, s__services ## Class, &(M ## Parent::s_class)}; \
      static const MClass::RegisterClassHelper s__register ## Class(&M ## Class::s_class); \
      const MClass* M ## Class::GetClass() const { return &s_class; }

   /// Set all persistent properties of this object and all its parents to default values.
   ///
   /// This macro is typically called from constructors of classes that have persistent properties.
   ///
   #define M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(Class)  DoSetPersistentPropertiesToDefault(&s_class)

   /// This macro ensures the reflected class gets linked even though the linker would place it away.
   ///
   #define M_LINK_THE_CLASS_IN(Class)    const MClass* _linkWith ## Class = Class::GetStaticClass();

#else // No-Reflection definitions 

   #define M_START_PROPERTIES(Class) \
      inline void MStaticDoSetPersistentPropertiesToDefault(M ## Class* obj) {

   #define M_CLASS_ENUMERATION(Class, Enumeration)

   #define M_OBJECT_PROPERTY_PERSISTENT_BOOL(Class, Property, defaultValue) \
      obj->Set ## Property(defaultValue);

   #define M_OBJECT_PROPERTY_BOOL(Class, Property)

   #define M_OBJECT_PROPERTY_BOOL_EXACT(Class, Property)

   #define M_OBJECT_PROPERTY_READONLY_BOOL(Class, Property)

   #define M_OBJECT_PROPERTY_READONLY_BOOL_EXACT(Class, Property)

   #define M_CLASS_PROPERTY_BOOL(Class, Property)

   #define M_CLASS_PROPERTY_READONLY_BOOL(Class, Property)

   #define M_CLASS_PROPERTY_READONLY_BOOL_EXACT(Class, Property)

   #define M_OBJECT_PROPERTY_PERSISTENT_CHAR(Class, Property, defaultValue)  \
      obj->Set ## Property(defaultValue);

   #define M_OBJECT_PROPERTY_CHAR(Class, Property)

   #define M_OBJECT_PROPERTY_READONLY_CHAR(Class, Property)

   #define M_OBJECT_PROPERTY_BYTE(Class, Property)

   #define M_OBJECT_PROPERTY_PERSISTENT_INT(Class, Property, defaultValue) \
      obj->Set ## Property(defaultValue);

   #define M_OBJECT_PROPERTY_INT(Class, Property)

   #define M_OBJECT_PROPERTY_READONLY_INT(Class, Property)

   #define M_OBJECT_PROPERTY_READONLY_INT_EXACT(Class, Property)

   #define M_OBJECT_PROPERTY_PERSISTENT_UINT(Class, Property, defaultValue) \
      obj->Set ## Property(defaultValue);

   #define M_OBJECT_PROPERTY_UINT(Class, Property)

   #define M_OBJECT_PROPERTY_READONLY_UINT(Class, Property)

   #define M_CLASS_PROPERTY_INT(Class, Property)

   #define M_CLASS_PROPERTY_READONLY_INT(Class, Property)

   #define M_CLASS_PROPERTY_UINT(Class, Property)

   #define M_CLASS_PROPERTY_READONLY_UINT(Class, Property)

   #define M_OBJECT_PROPERTY_PERSISTENT_DOUBLE(Class, Property, defaultValue) \
      obj->Set ## Property(defaultValue);

   #define M_OBJECT_PROPERTY_DOUBLE(Class, Property)

   #define M_OBJECT_PROPERTY_READONLY_DOUBLE(Class, Property)

   #define M_CLASS_PROPERTY_READONLY_DOUBLE(Class, Property)

   #define M_OBJECT_PROPERTY_PERSISTENT_BYTE_STRING(Class, Property, defaultValue, defaultValueLen, getMethodSignature, putMethodSignature) \
      obj->Set ## Property(MByteString((char*)(defaultValue), (unsigned)(defaultValueLen))); 

   #define M_OBJECT_PROPERTY_BYTE_STRING(Class, Property, getMethodSignature, putMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_BYTE_STRING(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_PERSISTENT_STRING(Class, Property, defaultValue, getMethodSignature, putMethodSignature) \
      obj->Set ## Property(defaultValue);

   #define M_OBJECT_PROPERTY_STRING(Class, Property, getMethodSignature, putMethodSignature)

   #define M_OBJECT_PROPERTY_STRING_EXACT(Class, Property, getMethodSignature, putMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_STRING(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_STRING_EXACT(Class, Property, getMethodSignature)

   #define M_CLASS_PROPERTY_STRING(Class, Property, getMethodSignature, putMethodSignature)

   #define M_CLASS_PROPERTY_READONLY_STRING(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_BYTE_STRING_COLLECTION(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_STRING_COLLECTION(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_STRING_COLLECTION(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_VARIANT_COLLECTION(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_PERSISTENT_VARIANT(Class, Property, defaultValue, getMethodSignature, putMethodSignature) \
      obj->Set ## Property((int)defaultValue)

   #define M_OBJECT_PROPERTY_VARIANT(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_VARIANT_EXACT(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_VARIANT(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_VARIANT_EXACT(Class, Property, getMethodSignature)

   #define M_CLASS_PROPERTY_READONLY_VARIANT(Class, Property, getMethodSignature)

   #define M_CLASS_PROPERTY_READONLY_VARIANT_EXACT(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_OBJECT(Class, Property)
   
   #define M_OBJECT_PROPERTY_OBJECT_OVERLOADED(Class, Property, GetPropertyImpl, SetPropertyImpl)

   #define M_OBJECT_PROPERTY_READONLY_OBJECT(Class, Property)

   #define M_OBJECT_PROPERTY_READONLY_OBJECT_EXACT(Class, Property)

   #define M_OBJECT_PROPERTY_OBJECT_EMBEDDED(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_OBJECT_EMBEDDED(Class, Property, getMethodSignature)

   #define M_OBJECT_PROPERTY_READONLY_OBJECT_EMBEDDED_EXACT(Class, Property, getMethodSignature)

   #define M_CLASS_PROPERTY_READONLY_OBJECT(Class, Property)

   #define M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED(Class, Property, getMethodSignature)

   #define M_CLASS_PROPERTY_READONLY_OBJECT_EMBEDDED_EXACT(Class, Property, getMethodSignature)

   #define M_CLASS_PROPERTY_READONLY_STRING_COLLECTION(Class, Property, getMethodSignature)

   #define M_START_METHODS(Class) \
       } // end inline void MStaticDoSetPersistentPropertiesToDefault(Class* obj)

   #define M_OBJECT_SERVICE(Class, Service, serviceType)

   #define M_OBJECT_SERVICE_NAMED(Class, Service, ServiceImpl, serviceType)

   #define M_OBJECT_SERVICE_OVERLOADED(Class, Service, ServiceImpl, numParameters, serviceType)

   #define M_CLASS_SERVICE(Class, Service, serviceType)

   #define M_CLASS_SERVICE_OVERLOADED(Class, Service, ServiceImpl, numParameters, serviceType)

   #define M_CLASS_FRIEND_SERVICE(Class, Service, FriendService, serviceType)

   #define M_CLASS_FRIEND_SERVICE_OVERLOADED(Class, Service, FriendService, numParameters, serviceType)

   #define M_END_CLASS(Class, Parent) \
      const MClass M ## Class::s_class = {&(M ## Parent::s_class)}; \
      const MClass* M ## Class::GetClass() const { return &s_class; }

   #define M_END_CLASS_EXACT(Class, Parent) \
      const MClass Class::s_class = {&(Parent::s_class)}; \
      const MClass* Class::GetClass() const { return &s_class; }

   #define M_END_CLASS_TYPED(Class, Parent, Type) \
      const MClass M ## Class::s_class = {&(M ## Parent::s_class)}; \
      const MClass* M ## Class::GetClass() const { return &s_class; }

   #define M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT(Class)   MStaticDoSetPersistentPropertiesToDefault(this)

   #define M_LINK_THE_CLASS_IN(Class)

#endif

///@}
#endif
