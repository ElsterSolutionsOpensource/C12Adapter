#ifndef MCORE_MPROPERTYDEFINITION_H
#define MCORE_MPROPERTYDEFINITION_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MPropertyDefinition.h

#ifndef MCORE_MOBJECT_H
   #error "This header file has hard interdepencency from reflection headers. Please include MCORE/MObject.h or MCORE/MCORE.h instead of this file."
#endif

#if !M_NO_REFLECTION

/// Property definition of the object.
///
/// Using this definition, the property marshalling is implemented.
/// The user can gain access to this structure to acquire the
/// attributes of the property.
///
/// The property definitions are stored in an array with the special last property definition
/// having an empty name.
///
class M_CLASS MPropertyDefinition
{
public: // Constants:

   enum
   {
      MAXIMUM_PROPERTY_NAME_LENGTH = 44 // maximum size of property name in characters including trailing zero
   };

public: // Services:

   /// Whether the property is actually a read-only integer enumeration value.
   ///
   bool IsEnumeration() const
   {
      M_ENSURED_ASSERT(m_name[0] != '\0'); // check if the service is valid (not the end-list tag)
      return m_type == MVariant::VAR_EMPTY;
   }

   /// True if this is a class service rather than an object service.
   /// Class service is called static service in C++.
   ///
   /// \pre The parameter is in range of ServiceType enumeration. There is a debug check.
   ///
   bool IsClassProperty() const
   {
      M_ASSERT((m_getObjectMethod != NULL && m_getServiceType < MClass::ST__OBJECT_METHOD_LAST) ^
               (m_getClassMethod != NULL && (m_getServiceType > MClass::ST__OBJECT_METHOD_LAST &&
                                             m_getServiceType < MClass::ST__CLASS_METHOD_LAST)));
      M_ASSERT((m_setObjectMethod == NULL || m_setServiceType < MClass::ST__OBJECT_METHOD_LAST) &&
               (m_setClassMethod == NULL || (m_setServiceType > MClass::ST__OBJECT_METHOD_LAST &&
                                             m_setServiceType < MClass::ST__CLASS_METHOD_LAST)));
      return m_getClassMethod != NULL;
   }

   /// Whether this is a read-only property, or enumeration.
   ///
   bool IsReadOnly() const
   {
      return m_setObjectMethod == 0 && m_setClassMethod == 0; // case for enumeration too
   }

   /// Tells if the default value of the property is present.
   /// The properties with default values are those which are stored persistently,
   /// and only persistent properties can have default values.
   /// By convention, enumerations do not have default values. If it is
   /// an enumeration, false is returned.
   ///
   bool IsDefaultValuePresent() const
   {
      M_ASSERT(m_name[0] != '\0'); // check if the service is valid (not the end-list tag)
      return m_valuePtr != NULL;
   }

   /// Get the default value of the property.
   /// One can look at the convenience service IsDefaultValuePresent to check
   /// if the property has a default value.
   ///
   /// \pre This has to be a persistent property.
   ///
   MVariant GetDefaultValue() const
   {
      M_ENSURED_ASSERT(m_name[0] != '\0'); // check if the service is valid (not the end-list tag)
      M_ENSURED_ASSERT(m_type != MVariant::VAR_EMPTY); // check if we are not an enumeration
      M_ENSURED_ASSERT(m_valuePtr != NULL);
      switch ( m_type )
      {
      case MVariant::VAR_BOOL:
         return m_valueInt != 0;
      case MVariant::VAR_BYTE:
         return static_cast<Muint8>(m_valueInt);
      case MVariant::VAR_CHAR:
         return static_cast<char>(m_valueInt);
      case MVariant::VAR_UINT:
      case MVariant::VAR_VARIANT: // by convention, variant default is an unsigned (works so far)
         return static_cast<unsigned>(m_valueInt);
      case MVariant::VAR_INT:
         return m_valueInt;
      case MVariant::VAR_DOUBLE:
         return *reinterpret_cast<double*>(m_valuePtr);
      case MVariant::VAR_BYTE_STRING:
         return MVariant(reinterpret_cast<const Muint8*>(m_valuePtr), static_cast<unsigned>(m_valueInt));
      case MVariant::VAR_STRING:
         break; // make it return at the end of the function to pacify the compiler
      default:
         M_ENSURED_ASSERT(0);
      }
      return reinterpret_cast<const char*>(m_valuePtr);
   }

   /// Get the enumeration value, always an unsigned.
   ///
   /// \pre This has to be enumeration property or an assertion is hit in debug mode.
   ///
   unsigned GetEnumerationValue() const
   {
      M_ENSURED_ASSERT(m_name[0] != '\0'); // check if the service is valid (not the end-list tag)
      M_ASSERT(m_getObjectMethod == NULL && m_getClassMethod == NULL);
      M_ASSERT(m_setObjectMethod == NULL && m_setClassMethod == NULL);
      M_ENSURED_ASSERT(m_type == MVariant::VAR_EMPTY); // check if we are not an enumeration
      return static_cast<unsigned>(m_valueInt);
   }

public: // Semi-private attributes:

   /// Name of the property.
   ///
   /// Internally, if the name is all zeros,
   /// it means this structure is the last in the property list.
   ///
   MChar m_name [ MAXIMUM_PROPERTY_NAME_LENGTH ];

   /// Type of the property.
   ///
   MVariant::Type m_type;

   /// Get method type.
   ///
   MClass::ServiceType m_getServiceType;

   /// Set method type.
   ///
   MClass::ServiceType m_setServiceType;

   /// Pointer to the object method to call in order to get the property.
   /// Note that the actual type is determined by m_getServiceType, and it should correspond
   /// to m_type. Few assertions done in the source code.
   ///
   /// Valid only if name is not empty, and service type is an object method.
   /// If this is equal to NULL, it means the property is Set-only.
   ///
   MObject::Method m_getObjectMethod;

   /// Pointer to the class method to call in order to get the property.
   /// Note that the actual type is determined by m_getServiceType, and it should correspond
   /// to m_type. Few assertions done in the source code.
   ///
   /// Valid only if name is not empty, and service type is a class method.
   /// If this is equal to NULL, it means the property is Set-only.
   ///
   MClass::Method m_getClassMethod;

   /// Pointer to the object method to call in order to set the property.
   /// Note that the actual type is determined by m_setServiceType, and it should correspond to m_type.
   ///
   /// Valid only if name is not empty, and service type is an object method
   /// Because there are no set-only properties, the object method is never equal to NULL.
   ///
   MObject::Method m_setObjectMethod;

   /// Pointer to the class method to call in order to set the property.
   /// Note that the actual type is determined by m_setServiceType, and it should correspond to m_type.
   ///
   /// Valid only if name is not empty, and service type is a class method.
   /// Because there are no set-only properties, the class method is never equal to NULL.
   ///
   MClass::Method m_setClassMethod;

   /// First value placeholder, if used. This is a pointer value.
   ///
   void* m_valuePtr;

   /// Second value placeholder, if used. This is an integer value.
   ///
   int m_valueInt;
};

#endif
///@}
#endif
