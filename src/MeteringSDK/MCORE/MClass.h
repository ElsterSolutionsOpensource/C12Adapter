#ifndef MCORE_MCLASS_H
#define MCORE_MCLASS_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MClass.h

#include <MCORE/MCOREDefs.h>
#include <MCORE/MVariant.h>

/// Notion of a class that serves as the base for the reflection API.
///
/// Every class derived from MObject and exposed through Reflection has an associated class object.
/// Class gives the way for an object to dynamically present the information about itself:
///    - Parent class
///    - Properties
///    - Services (methods) and their parameters
/// Current implementation supports only single inheritance.
///
/// Somewhat more popular name for what is called "service" is "method."
///
/// Properties can be read-only, in which case their values cannot be set by direct assignment to property.
/// Read-write properties can be persistent, which means there is a default value
/// available for such property, and that the value of such property is suitable for persisting in configuration.
///
/// \see MObject
///
class M_CLASS MClass
{
   friend class M_CLASS MObject;

public: // Constants:

   enum
   {
      MAXIMUM_NUMBER_OF_SERVICE_PARAMETERS = 6, ///< Maximum number of parameters supported in a service
      MAXIMUM_CLASS_NAME_LENGTH = 40            ///< Maximum size of class name in characters including trailing zero (divisible by 4)
   };

public: // Services:

   /// Get parent of a class, or NULL if the class has no parent.
   ///
   /// The only class that has no parent is MObject. 
   ///
   const MClass* GetParent() const
   {
      return m_parent;
   }

   /// Tell about the relationship between this class and a given class.
   ///
   /// \return True will mean the current object is kind of a given class.
   ///
   /// \see StaticIsKindOf - static version that also checks if the object is not null
   ///
   bool IsKindOf(const MClass*) const;

   ///@{
   /// Static version that tells if a given object is of a given class, or it is its child.
   ///
   /// Different from \ref IsKindOf, this version checks of the given object is null.
   ///
   /// \return True will mean the given object is not null, and is kind of a given class.
   ///
   static bool StaticIsKindOf(const MObject* obj, const MClass* cls); // SWIG_HIDE
   static bool StaticIsKindOf(const MObject& ref, const MClass* cls)  // SWIG_HIDE
   {
      return StaticIsKindOf(&ref, cls);
   }
   ///@}

/// \cond SHOW_INTERNAL

   // Throw Cannot Convert type x to y exception, treating parameters accordingly.
   // From parameter can be zero, in which case it is replaced by NULL.
   // The function never returns.
   //
   static M_NORETURN_FUNC void DoThrowCannotConvert(const MObject* from, const MClass* cls);

#if !M_NO_REFLECTION

   // Internal generic type definition for the method of the class.
   // (the static method of an object).
   //
   typedef void (*Method)();

   // Internal types for the class methods that accept parameters of different kind
   // Please see the insides of file MCORE/MObjectMethods.inc, second
   // parameters in the macro definitions, for particular type definitions.
   //
   #define _M(st, type, call, par, ret, num)  typedef type;
   #include "MClassMethods.inc"

   // Enumeration of signatures for service types.
   //
   // The signature denotes the prototype of the service.
   // Return value, if present, goes first, then goes the type letter, then the parameter signature.
   // There are constructors, object methods and class methods (static object methods).
   // Constructors are denoted by N letter, object methods with X, and static methods with S.
   // This letter is placed in the position of a function name, in between the return value
   // and the parameter signature.
   //
   enum ServiceType
   {
      ST__NULL,                         // no method at all

      // Object methods
      //
      // ST enumerations that relate to object methods.
      // Please see the insides of file MCORE/MObjectMethods.inc, first
      // parameters in the macro definitions, for particular enumeration.
      //
      #define _M(st, type, call, par, ret, num)  st,
      #include "MObjectMethods.inc"
      ST__OBJECT_METHOD_LAST = ST_MByteString_X_constMVariantA_int_int_int_int_int,

      // Class methods (object static methods)
      //
      // ST enumerations that relate to class methods.
      // Please see the insides of file MCORE/MObjectMethods.inc, first
      // parameters in the macro definitions, for particular enumeration.
      //
      #define _M(st, type, call, par, ret, num)  st,
      #include "MClassMethods.inc"
      ST__CLASS_METHOD_LAST = ST_MVariant_S_int_int_int_int_int_int
   };

   // Class register helper, one which registers the class with the reflection support facility.
   // There is no need to export this class.
   //
   class M_CLASS RegisterClassHelper
   {
   public: // Services:

      // Constructor, that initializes the application class
      //
      // \pre The class with such name should not exist in the collection
      //
      RegisterClassHelper(const MClass* self);
   };

/// \endcond SHOW_INTERNAL

public:

   /// Access the name of the MClass.
   ///
   /// This will be a string representation of a particular class,
   /// like "Table" would be the one to represent class MTable.
   ///
   MConstChars GetName() const
   {
      return m_name;
   }

   /// Access the type name of the class.
   ///
   /// Type of the class is in majority of cases the same as the class name.
   /// The differences exist only in MCOM classes, those derived from MCOMObject,
   /// such as MProtocol and MChannel.
   ///
   MConstChars GetTypeName() const
   {
      return m_typeName;
   }

   /// Get parent class by the name specified.
   ///
   /// \param name
   ///    Class name, parent name.
   ///
   /// \return
   ///    MClass object to represent the parent class,
   ///    or null, if parent with such name does not exist.
   ///
   const MClass* GetParentClass(const MStdString& name) const;

   /// Find an MClass of a class with the name given.
   ///
   /// All reflected classes existing in the application are enumerated to find a class.
   ///
   /// \param name
   ///    Class name to search for
   ///
   /// \return
   ///    MClass object to represent the existing reflected class,
   ///    or null, if parent with such name does not exist.
   ///
   /// \see GetExistingClass - throws an exception if class is not present
   ///
   static const MClass* GetClass(const MStdString& name);

   /// Find an existing MClass of a class with the name given.
   ///
   /// All reflected classes existing in the application are enumerated to find a class.
   ///
   /// \param name
   ///    Class name to search for. Such class should exist, or an exception is thrown.
   ///
   /// \return
   ///    MClass object to represent the existing reflected class.
   ///
   /// \see GetClass - returns null if class is not present
   ///
   static const MClass* GetExistingClass(const MStdString& name);

   /// Get the constant definition of service (method) with the given name, or return NULL if such service does not exist.
   ///
   /// This is C++ only. Service, as used in the identifier, is another name for method.
   ///
   /// \param name
   ///    Service name to get definition for. If service does not exist, NULL will be returned.
   ///
   /// \param expectedNumberOfParameters
   ///    Expected number of parameters. If specified, and not -1, shall denote the valid expected number of parameters.
   ///    This parameter will need to be specified only for services that allow overloading.
   ///
   /// \return
   ///    Service definition, if such service exists in the class, or NULL, if the specified service does not exist.
   ///
   /// \see GetServiceDefinition - throws an exception
   ///
   const MServiceDefinition* GetServiceDefinitionOrNull(const MStdString& name, int expectedNumberOfParameters = -1) const;

   /// Get the constant definition of service (method) with the given name, or return NULL if such service does not exist.
   ///
   /// This is C++ only. Service, as used in the identifier, is another name for method.
   ///
   /// \param name
   ///    Service name to get definition for. If service does not exist, an exception is thrown.
   ///
   /// \param expectedNumberOfParameters
   ///    Expected number of parameters. If specified, and not -1, shall denote the valid expected number of parameters.
   ///    This parameter will need to be specified only for services that allow overloading.
   ///
   /// \return
   ///    Service definition is returned, never a null.
   ///
   /// \see GetServiceDefinitionOrNull - throws an exception
   ///
   const MServiceDefinition* GetServiceDefinition(const MStdString& name, int expectedNumberOfParameters = -1) const;

   /// Tell if the service (method) with the given name exists in the class.
   ///
   /// \param name
   ///    Name of the service to check.
   ///
   /// \return
   ///    True returned if such service/method is present in either this class, or its parent
   ///
   /// \see MObject::IsServicePresent - essentially the same call, but it works on objects, not classes
   ///
   bool IsServicePresent(const MStdString& name) const
   {
      return GetServiceDefinitionOrNull(name, -1) != NULL;
   }

   /// The collection of all publicly available reflection enabled services.
   ///
   /// The list of services includes services of parent class.
   /// If the service has copies with different number of parameters, still only one copy of the service name will be present.
   ///
   /// \see GetOwnServiceNames - return class own service names, excluding parents
   ///
   MStdStringVector GetAllServiceNames() const;

   /// The collection of publicly available reflection enabled services owned by the class.
   ///
   /// The list of services does not include services of parent class.
   /// If the service has copies with different number of parameters, still only one copy of the service name will be present.
   ///
   /// \see GetAllServiceNames - return all service names available, including those in parents
   ///
   MStdStringVector GetOwnServiceNames() const;

   /// Return the pointer to the array of objects that represent publicly available services.
   ///
   /// This is a C++ only call.
   ///
   const MServiceDefinition* GetOwnServiceDefinitions() const
   {
      return m_services;
   }

   /// Call the static service of the class with parameters, given as variant.
   ///
   /// Service of the class is the same as static method in C++.
   ///
   /// \pre The service with such name should exist, shall be static (service of class),
   /// and the number of parameters should be equal to one supported by the call,
   /// otherwise an exception is thrown.
   /// The preconditions of the particular service that gets called also apply.
   ///
   /// \param name
   ///    Name of the static service (method) to call.
   ///
   /// \param params
   ///    Parameters to pass to the static service.
   ///      - To pass no parameters, supply an empty MVariant as parameter, or more conveniently, use \ref Call0.
   ///      - To pass a single non-empty non-collection MVariant using \ref Call, give it directly as param, or use \ref Call1.
   ///      - To pass a single empty MVariant as parameter, enclose it into a collection, or use \ref Call1.
   ///      - To pass a couple of parameters, supply them in a collection of MVariant values.
   ///      - To pass a single parameter, which is a collection of MVariant value, enclose it into another collection.
   ///
   /// \return
   ///    Returns whatever the reflected service/method with the given name returns.
   ///    If the called service has no return value, the returned MVariant is empty.
   ///
   /// \see Call0 - calling service without parameters
   /// \see Call1 - calling service with one parameter
   /// \see Call2 - calling service with two parameters
   /// \see CallV - calling service with a vector of parameters
   /// \see MObject::Call - calling non-static service of an object
   ///
   MVariant Call(const MStdString& name, const MVariant& params) const;

   /// Call the static service of the object with no parameters.
   ///
   /// Service of the class is the same as static method in C++.
   ///
   /// \pre The service with such name should exist, shall be static (service of class),
   /// and should have no parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service that gets called also apply.
   ///
   /// \param name
   ///    Name of the static service (method) to call.
   ///
   /// \return
   ///    Returns whatever the reflected service/method with the given name returns.
   ///    If the called service has no return value, the returned MVariant is empty.
   ///
   /// \see Call  - generic call of the service with parameters given as generic MVariant
   /// \see Call1 - calling service with one parameter
   /// \see Call2 - calling service with two parameters
   /// \see CallV - calling service with a vector of parameters
   /// \see MObject::Call0 - calling non-static service of an object with no parameters
   ///
   MVariant Call0(const MStdString& name) const;

   /// Call the static service of the object with one parameter.
   ///
   /// Service of the class is the same as static method in C++.
   ///
   /// \pre The service with such name should exist, shall be static (service of class),
   /// and should have one parameter, otherwise an exception is thrown.
   /// The preconditions of the particular service that gets called also apply.
   ///
   /// \param name
   ///    Name of the static service (method) to call.
   ///
   /// \param p1
   ///    The parameter to pass to static service.
   ///
   /// \return
   ///    Returns whatever the reflected service/method with the given name returns.
   ///    If the called service has no return value, the returned MVariant is empty.
   ///
   /// \see Call  - generic call of the service with parameters given as generic MVariant
   /// \see Call0 - calling service with no parameters
   /// \see Call2 - calling service with two parameters
   /// \see CallV - calling service with a vector of parameters
   /// \see MObject::Call1 - calling non-static service of an object with one parameter
   ///
   MVariant Call1(const MStdString& name, const MVariant& p1) const;

   /// Call the static service of the object with two parameter.
   ///
   /// Service of the class is the same as static method in C++.
   ///
   /// \pre The service with such name should exist, shall be static (service of class),
   /// and should have two parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service that gets called also apply.
   ///
   /// \param name
   ///    Name of the service (method) to call.
   ///
   /// \param p1
   ///    The first parameter to pass to service.
   ///
   /// \param p2
   ///    The second parameter to pass to service.
   ///
   /// \return
   ///    Returns whatever the reflected service/method with the given name returns.
   ///    If the called service has no return value, the returned MVariant is empty.
   ///
   /// \see Call  - generic call of the service with parameters given as generic MVariant
   /// \see Call0 - calling service with no parameters
   /// \see Call1 - calling service with two parameters
   /// \see CallV - calling service with a vector of parameters
   /// \see MObject::Call2 - calling non-static service of an object with two parameters
   ///
   MVariant Call2(const MStdString& name, const MVariant& p1, const MVariant& p2) const;

   /// Call the static service of the object with vector of parameter.
   ///
   /// Service of the class is the same as static method in C++.
   ///
   /// \pre The service with such name should exist, shall be static (service of class),
   /// and should have the same number of parameters as the vector count given, otherwise an exception is thrown.
   /// The preconditions of the particular service that gets called also apply.
   ///
   /// \param name
   ///    Name of the service (method) to call.
   ///
   /// \param params
   ///    The vector of parameters to pass to service.
   ///
   /// \return
   ///    Returns whatever the reflected service/method with the given name returns.
   ///    If the called service has no return value, the returned MVariant is empty.
   ///
   /// \see Call  - generic call of the service with parameters given as generic MVariant
   /// \see Call0 - calling service with no parameters
   /// \see Call1 - calling service with one parameter
   /// \see Call2 - calling service with two parameters
   /// \see MObject::CallV - calling non-static service of an object with vector of parameters
   ///
   MVariant CallV(const MStdString& name, const MVariant::VariantVector& params) const;

   /// Get the existing constant definition of the property with the name specified.
   ///
   /// \param name
   ///    The name of the property. The property with the name given should exist,
   ///    otherwise the service throws an exception.
   ///
   /// \return
   ///    Property definition to represent the property of a given name will be returned.
   ///
   /// \see GetPropertyDefinitionOrNull - will return NULL if the property with the given name does not exist.
   ///
   const MPropertyDefinition* GetPropertyDefinition(const MStdString& name) const
   {
      const MPropertyDefinition* def = GetPropertyDefinitionOrNull(name);
      if ( def == NULL )
      {
         DoThrowUnknownPropertyException(m_typeName, name); // this helper service allows us having an inline
         M_ENSURED_ASSERT(0);                               // part in the efficient way
      }
      return def;
   }

   /// Get the constant definition of the property with the name specified, or NULL if such property does not exist.
   ///
   /// \param name
   ///    The name of the property.
   ///
   /// \return
   ///    Property definition to represent the property of a given name will be returned, or NULL, if there is no such property.
   ///
   /// \see GetPropertyDefinition - will never return NULL but rather throw an exception if the property does not exist.
   ///
   const MPropertyDefinition* GetPropertyDefinitionOrNull(const MStdString& name) const;

   /// Tell if the property with the given name exists.
   ///
   /// \param name
   ///    The property name for which to check the existence
   ///
   /// \return
   ///    True returned if such property is present in either this class, or its parent
   ///
   /// \see MObject::IsPropertyPresent - essentially the same call, but it works on objects, not classes
   ///
   bool IsPropertyPresent(const MStdString& name) const
   {
      return GetPropertyDefinitionOrNull(name) != NULL;
   }

   /// Get the static class property value using name of the property.
   ///
   /// \pre
   ///    Particular property's preconditions apply to this call.
   ///
   /// \param name
   ///    Name of the property of interest.
   ///    The property with such name should exist, and it should be
   ///    the property of the class. Otherwise an exception will be thrown.
   ///
   /// \return
   ///    Value of property is returned.
   ///
   /// \see SetProperty - Set class property value.
   /// \see GetPropertyDefinition - returns the constant pointer to the definition of the property with a name given.
   /// \see MObject::GetProperty - Get property of an object. In case the static class property is accessed,
   ///                             the call MObject::GetProperty translates into this call, MClass::GetProperty
   ///
   MVariant GetProperty(const MStdString& name) const;

   /// Set the static class property value using name of the property.
   ///
   /// \pre
   ///    Particular property's preconditions apply to this call.
   ///
   /// \param name
   ///    Name of the property of interest.
   ///    The property with such name should exist, it should not be read-only,
   ///    and it should be the property of the class. Otherwise an exception will be thrown.
   ///
   /// \param value
   ///    Value to assign to the property.
   ///
   /// \see GetProperty - Get class property value.
   /// \see MObject::SetProperty - Set property of an object. In case the static class property is accessed,
   ///                             the call MObject::SetProperty translates into this call, MClass::SetProperty
   ///
   void SetProperty(const MStdString& name, const MVariant& value) const;

   /// Return the list of all publicly available classes.
   ///
   /// While this property is declared in MClass, it really belongs to global environment of the application.
   /// The list composes all classes from all shared libraries or executables available at the time of the call.
   /// If a library is loaded dynamically at run time, and it has reflected classes, this list will grow.
   ///
   static MStdStringVector GetAllClassNames();

   /// Return the whole list of publicly available properties of this class and parents.
   ///
   /// Persistent and not persistent properties of this class and all parents of this class are returned.
   ///
   /// \see GetOwnPropertyNames - fetch only this class properties, excluding properties of parents
   /// \see GetAllPersistentPropertyNames - fetch only persistent properties.
   ///
   MStdStringVector GetAllPropertyNames() const;

   /// Return the list of publicly available properties owned by this class.
   ///
   /// Persistent and not persistent properties of this class are returned.
   ///
   /// \see GetAllPropertyNames - fetch properties of this class and all parents of this class
   /// \see GetAllPersistentPropertyNames - fetch only persistent properties.
   ///
   MStdStringVector GetOwnPropertyNames() const;

   /// Return the list of publicly available persistent properties of this class and all parents of this class.
   ///
   /// Persistent properties of this class and all parents of this class are returned.
   ///
   /// \see GetAllPropertyNames - fetch properties of this class and all parents of this class
   /// \see GetOwnPropertyNames - fetch properties of this class, excluding parents
   ///
   MStdStringVector GetAllPersistentPropertyNames() const;

   /// Get the default value of persistent property with the name given.
   ///
   /// \param name
   ///    Name of the persistent property to fetch default value from.
   ///    The object should have the property with such name, and this
   ///    property should have a default value (be persistent).
   ///    Otherwise an exception is thrown.
   ///
   /// \return
   ///    Default value of property of a given name is returned.
   ///
   MVariant GetPersistentPropertyDefaultValue(const MStdString& name) const;

   /// Whether the given string is either class name or type name
   ///
   /// This method is a convenience when a string has to match either class or type of this Class.
   ///
   /// \param name
   ///    Name of the presumed class or type.
   ///
   /// \return
   ///    True will be returned if the parameter matches either class name or type name.
   ///
   bool MatchesClassOrTypeName(const MStdString& name) const;
   
public: ///  Semi-private services:
/// \cond SHOW_INTERNAL

   // Unconditionally throws an exception of unknown property.
   // This has to be within the CPP file due to interdependency between MClass and MException.
   //
   static M_NORETURN_FUNC void DoThrowUnknownPropertyException(MConstChars className, const MStdString& name);

   // Unconditionally throws an exception of unknown service.
   // This has to be within the CPP file due to interdependency between MClass and MException.
   //
   static M_NORETURN_FUNC void DoThrowUnknownServiceException(MConstChars className, const MStdString& name);

   static M_NORETURN_FUNC void DoThrowServiceDoesNotHaveNParameters(const MStdString& name, int parametersCount);

public: // Attributes:

   /// Name of the class as string.
   ///
   /// The name of a class MTime will be "Time", as the common prefix letter will be discarded.
   ///
   MChar m_name [ MAXIMUM_CLASS_NAME_LENGTH ];

   /// Type name of the class, typically the same as the class name.
   /// The difference is due to historic names in MCOM component.
   ///
   MChar m_typeName [ MAXIMUM_CLASS_NAME_LENGTH ];

   /// The pointer to the list of properties.
   /// The list is terminated with a special property with no name.
   /// See MPropertyDefinition for details.
   ///
   const MPropertyDefinition* m_properties;

   /// The pointer to the list of services.
   /// The list is terminated with a special service with no name.
   /// See MServiceDefinition for details.
   ///
   const MServiceDefinition* m_services;

#endif // Reflection-related definitions end here

public: // Attributes:

   /// Parent class, if present, otherwise NULL (for MObject oneself).
   /// If reflection is switched off, parent is the only field present.
   ///
   const MClass* m_parent;
   
/// \endcond SHOW_INTERNAL
};

///@}
#endif
