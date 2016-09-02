#ifndef MCORE_MSERVICEDEFINITION_H
#define MCORE_MSERVICEDEFINITION_H
/// \file MCORE/MServiceDefinition.h

#ifndef MCORE_MOBJECT_H
   #error "This header file has hard interdepencency from reflection headers. Please include MCORE/MObject.h or MCORE/MCORE.h instead of this file."
#endif

#if !M_NO_REFLECTION

/// Service definition of the object.
///
/// There are the following types of services: constructors, object methods, and class methods.
/// Using this definition, the service marshalling is implemented.
/// The user can gain access to this structure to acquire the
/// attributes of the service.
///
/// The service definitions are stored in an array with the special last service definition
/// having an empty name.
///
class M_CLASS MServiceDefinition
{
public: // Constants:

   enum
   {
      MAXIMUM_SERVICE_NAME_LENGTH = 40 // maximum size of service name in characters including trailing zero
   };

public: // Fields:

   /// Name of the service.
   ///
   /// Internally, if the name is a string containing only binary zeros, 
   /// it means this structure is the last in the service list.
   ///
   MChar m_name [ MAXIMUM_SERVICE_NAME_LENGTH ];

   /// If the procedure has overloaded parameters, this is the number of them.
   /// If this is not an overloaded procedure, this value is -1.
   ///
   int m_overloadedNumberOfParameters;

   /// Service type
   ///
   MClass::ServiceType m_type;

   /// Object method, valid only if the service type denotes the object method, otherwise NULL.
   ///
   MObject::Method m_objectMethod;

   /// Class method, valid only if the service type denotes the class method, otherwise NULL.
   ///
   MClass::Method m_classMethod;

public: // Services:

   /// True if this is a class service rather than an object service.
   /// Class service is called static service in C++.
   ///
   /// \pre The parameter is in range of ServiceType enumeration. There is a debug check.
   ///
   bool IsClassService() const
   {
      M_ASSERT((m_objectMethod != NULL && m_type <= MClass::ST__OBJECT_METHOD_LAST) !=
               (m_classMethod != NULL && (m_type > MClass::ST__OBJECT_METHOD_LAST &&
                                          m_type <= MClass::ST__CLASS_METHOD_LAST)));
      return m_classMethod != NULL;
   }

#if !M_NO_FULL_REFLECTION // if we offer extensive information on service parameters

   /// Return an array of parameter types.
   ///
   /// \pre The object is built correctly. There are some debug asserts.
   ///
   const MVariant::Type* GetParameterTypes() const;

   /// Return the number of parameters of this service.
   ///
   /// \pre The object is built correctly. There are some debug asserts.
   ///
   unsigned GetNumberOfParameters() const;

   /// Return service type, or MVariant::VAR_EMPTY if this is a procedure.
   ///
   /// \pre The object is built correctly. There are some debug asserts.
   ///
   MVariant::Type GetReturnType() const;

   /// Whether this service is procedure or function.
   ///
   /// \pre The object is built correctly. There are some debug asserts.
   ///
   bool IsProcedure() const
   {
      return GetReturnType() == MVariant::VAR_EMPTY;
   }

#endif
};

#endif
#endif


