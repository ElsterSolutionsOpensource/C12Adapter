#ifndef MCORE_MOBJECT_H
#define MCORE_MOBJECT_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MObject.h

#include <MCORE/MClass.h>

/// Root object that establishes dynamic class information,
/// dynamic property and method handling and marshaling.
/// The class is an abstract base.
///
/// The derived objects have type names available from the
/// abstract service GetType.
///
/// This class allows for having a number of properties and methods
/// to be publicly defined, and available through their names.
/// This is done by implementation of the the dynamic dispatching mechanism
/// alternative to the semi-static virtual C++ dispatching.
///
/// The children of this class should have a macro M_DECLARE_*_CLASS at the end
/// of their declaration, and a set of M_PROPERTY_ macros in the implementation
/// CPP file. They should define the properties publicly available.
///
/// Here is an example of some MObject abstract child:
/// \code
///   // File SomeObject.h
///   class MSomeObject : public MObject
///   {
///   public:
///      MSomeObject();
///      virtual ~MSomeObject();
///
///      int GetSpeed() const;         // Get integer property method
///      void SetSpeed(int);           // Set integer property method
///      unsigned GetDeviceID() const; // Get device ID, Get-only
///
///   private:
///      M_DECLARE_CLASS(SomeObject)
///   };
/// \endcode
/// \code
///   // File SomeObject.cpp
///   #include <MCORE/MCOREExtern.h>
///   #include "SomeObject.h"
///   ....
///   M_START_PROPERTIES(SomeObject)
///      M_OBJECT_PROPERTY_PERSISTENT_INT(SomeObject, Speed, 9600)
///      M_OBJECT_PROPERTY_READONLY_UINT (SomeObject, DeviceId, 0) // Get-only property
///   M_START_METHODS(SomeObject)
///   M_END_CLASS(SomeObject, Object)
/// \endcode
/// Here is an example of some MObject child:
/// \code
///   // File MSomeProtocol.h
///   class MSomeProtocol : public MObject
///   {
///   public:
///      MSomeProtocol();
///      virtual ~MSomeProtocol();
///
///      int GetSpeed() const;         // Get integer property method
///      void SetSpeed(int);           // Set integer property method
///      unsigned GetDeviceID() const; // Get device ID, Get-only
///
///   private:
///      M_DECLARE_CLASS(SomeProtocol)
///   };
/// \endcode
/// \code
///   // File SomeProtocol.cpp
///   #include <MCORE/MCOREExtern.h>
///   #include "SomeProtocol.h"
///   ....
///   M_START_PROPERTIES(SomeProtocol)
///      M_OBJECT_PROPERTY_PERSISTENT_INT(SomeProtocol, Speed, 9600)
///      M_OBJECT_PROPERTY_READONLY_UINT (SomeProtocol, DeviceId) // Get-only property
///   M_START_METHODS(SomeProtocol)
///   M_END_CLASS(SomeProtocol, Object)
/// \endcode
/// The _EXACT class declarations do not add M prefix to their names given.
/// The _TYPED class declarations make type and class names different for the class definition.
///
/// There is one property defined for the class. This is TYPE, which gives
/// the standard string, representation of the type (class).
///
class M_ABSTRACT_CLASS MObject
{
   friend class MVariant;

protected: // Constructor:

   /// Object constructor, protected as the class is abstract.
   ///
   MObject()
   {
   }

public: // Destructor and services:

   /// Object destructor.
   ///
   virtual ~MObject()
   {
   }

   /// Get the final class of the object.
   /// Do not overload this service explicitly. It is done automatically
   /// within the appropriate implementation macros.
   ///
   /// \pre This particular service is pure virtual, and should not
   /// be called explicitly.
   ///
   virtual const MClass* GetClass() const = 0;

   /// Get the declared class of this particular object.
   /// This service is defined explicitly with the reflection support macros.
   ///
   static const MClass* GetStaticClass()
   {
      return &s_class;
   }

#if !M_NO_VARIANT
   /// For embedded object types, return the size of the class
   ///
   /// For regular, not embedded types, this is zero, which is the default implementation.
   ///
   /// \return size of embedded object in bytes, or zero if the object is not embedded
   ///
   virtual unsigned GetEmbeddedSizeof() const;

   /// Tell if the object is of embedded kind.
   ///
   /// Embedded objects are value types, those that can be copied and compared
   /// by simple memory copy and comparison.
   ///
   /// \return True means the object is of an embedded kind, and it can be manipulated by value
   ///
   bool IsEmbeddedObject() const
   {
      return GetEmbeddedSizeof() != 0u;
   }
#endif

public: // Properties:

   /// Class of MObject.
   ///
   static const MClass s_class;

#if !M_NO_REFLECTION // Services pertinent only to reflection follow

   friend class MClass;
   friend class MPropertyDefinition;
   friend class MServiceDefinition;

public: // Types:

/// \cond SHOW_INTERNAL

   // IMPLEMENTATION NOTE:
   // The types below would probably be more naturally seen in the MClass declaration,
   // but unfortunately there is a bug in Visual C++ which prevents this.
   // It seems like Visual C++ does not want incomplete class type used for method typedef.

   /// Internal generic type definition for the method of object.
   ///
   typedef void (MObject::*Method)();

   /// Internal types for the methods that accept parameters of different kind
   /// Please see the insides of file MCORE/MObjectMethods.inc, second
   /// parameters in the macro definitions, for particular type definitions.
   ///
   #define _M(st, type, call, par, ret, num) typedef type;
   #include <MCORE/MObjectMethods.inc>
   #undef _M

/// \endcond SHOW_INTERNAL

   /// Call the object service with parameters, given as variant.
   /// Parameters can be Empty variant, which will mean no parameters;
   /// a variant vector, which will be the vector of parameters;
   /// or it can be one single parameter.
   ///
   /// \pre The service with such name should exist,
   /// and the number of parameters should be equal to one supported by the call,
   /// otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see CallV which unconditionally takes the vector of parameters.
   ///
   MVariant Call(const MStdString& name, const MVariant& params);

   /// Call the object service with no parameters.
   ///
   /// \pre The service with such name should exist,
   /// and shall have no parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see CallV which unconditionally takes the variant as parameters
   /// \see Call that takes a variant
   /// \see Call1 that takes a variant with 1 parameter
   /// \see Call2 that takes a variant with 2 parameters
   /// \see Call3 that takes a variant with 3 parameters
   /// \see Call4 that takes a variant with 4 parameters
   /// \see Call5 that takes a variant with 5 parameters
   /// \see Call6 that takes a variant with 6 parameters
   ///
   MVariant Call0(const MStdString& name);

   /// Call the object service with one parameter.
   /// The clients have to provide CallV as implementation.
   ///
   /// \pre The service with such name should exist,
   /// and shall have no parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see CallV which unconditionally takes the variant as parameters
   /// \see Call that takes a variant
   /// \see Call0 that takes a variant with no parameters
   /// \see Call2 that takes a variant with 2 parameters
   /// \see Call3 that takes a variant with 3 parameters
   /// \see Call4 that takes a variant with 4 parameters
   /// \see Call5 that takes a variant with 5 parameters
   /// \see Call6 that takes a variant with 6 parameters
   ///
   MVariant Call1(const MStdString& name, const MVariant& p1);

   /// Call the object service with two parameter.
   /// The clients have to provide CallV as implementation.
   ///
   /// \pre The service with such name should exist,
   /// and shall have no parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see CallV which unconditionally takes the variant as parameters
   /// \see Call that takes a variant
   /// \see Call0 that takes a variant with no parameters
   /// \see Call1 that takes a variant with 1 parameter
   /// \see Call3 that takes a variant with 3 parameters
   /// \see Call4 that takes a variant with 4 parameters
   /// \see Call5 that takes a variant with 5 parameters
   /// \see Call6 that takes a variant with 6 parameters
   ///
   MVariant Call2(const MStdString& name, const MVariant& p1, const MVariant& p2);

   /// Call the object service with three parameter.
   /// The clients have to provide CallV as implementation.
   ///
   /// \pre The service with such name should exist,
   /// and shall have no parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see CallV which unconditionally takes the variant as parameters
   /// \see Call that takes a variant
   /// \see Call0 that takes a variant with no parameters
   /// \see Call1 that takes a variant with 1 parameter
   /// \see Call2 that takes a variant with 2 parameters
   /// \see Call4 that takes a variant with 4 parameters
   /// \see Call5 that takes a variant with 5 parameters
   /// \see Call6 that takes a variant with 6 parameters
   ///
   MVariant Call3(const MStdString& name, const MVariant& p1, const MVariant& p2, const MVariant& p3);

   /// Call the object service with four parameter.
   /// The clients have to provide CallV as implementation.
   ///
   /// \pre The service with such name should exist,
   /// and shall have no parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see CallV which unconditionally takes the variant as parameters
   /// \see Call that takes a variant
   /// \see Call0 that takes a variant with no parameters
   /// \see Call1 that takes a variant with 1 parameter
   /// \see Call2 that takes a variant with 2 parameters
   /// \see Call3 that takes a variant with 3 parameters
   /// \see Call5 that takes a variant with 5 parameters
   /// \see Call6 that takes a variant with 6 parameters
   ///
   MVariant Call4(const MStdString& name, const MVariant& p1, const MVariant& p2, const MVariant& p3, const MVariant& p4);

   /// Call the object service with five parameter.
   /// The clients have to provide CallV as implementation.
   ///
   /// \pre The service with such name should exist,
   /// and shall have no parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see CallV which unconditionally takes the variant as parameters
   /// \see Call that takes a variant
   /// \see Call0 that takes a variant with no parameters
   /// \see Call1 that takes a variant with 1 parameter
   /// \see Call2 that takes a variant with 2 parameters
   /// \see Call3 that takes a variant with 3 parameters
   /// \see Call4 that takes a variant with 4 parameters
   /// \see Call6 that takes a variant with 6 parameters
   ///
   MVariant Call5(const MStdString& name, const MVariant& p1, const MVariant& p2, const MVariant& p3, const MVariant& p4, const MVariant& p5);

   /// Call the object service with six parameter.
   /// The clients have to provide CallV as implementation.
   ///
   /// \pre The service with such name should exist,
   /// and shall have no parameters, otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see CallV which unconditionally takes the variant as parameters
   /// \see Call that takes a variant
   /// \see Call0 that takes a variant with no parameters
   /// \see Call1 that takes a variant with 1 parameter
   /// \see Call2 that takes a variant with 2 parameters
   /// \see Call3 that takes a variant with 3 parameters
   /// \see Call4 that takes a variant with 4 parameters
   /// \see Call5 that takes a variant with 5 parameters
   ///
   MVariant Call6(const MStdString& name, const MVariant& p1, const MVariant& p2, const MVariant& p3, const MVariant& p4, const MVariant& p5, const MVariant& p6);

   /// Call the object service with parameters, given as variant vector.
   ///
   /// \pre The service with such name should exist,
   /// and the number of parameters should be equal to one supported by the call,
   /// otherwise an exception is thrown.
   /// The preconditions of the particular service apply.
   ///
   /// \see Call that takes a variant
   /// \see Call0 that takes a variant with no parameters
   /// \see Call1 that takes a variant with 1 parameter
   /// \see Call2 that takes a variant with 2 parameters
   /// \see Call3 that takes a variant with 3 parameters
   /// \see Call4 that takes a variant with 4 parameters
   /// \see Call5 that takes a variant with 5 parameters
   /// \see Call6 that takes a variant with 6 parameters
   ///
   virtual MVariant CallV(const MStdString& name, const MVariant::VariantVector& params);

   /// Tell if the property with the given name exists.
   ///
   virtual bool IsPropertyPresent(const MStdString& name) const;

   /// Tell if the service with the given name exists.
   ///
   virtual bool IsServicePresent(const MStdString& name) const;

   /// Get the property value using name of the property.
   ///
   /// The service allows extensions in children objects.
   ///
   /// \pre The property with such name should exist, otherwise
   /// an exception will be thrown. The property value should be available
   /// at the time the service is called, or the value-related exception
   /// can be thrown.
   ///
   /// \see MClass::GetPropertyDefinition returns the constant pointer to the 
   ///       definition of the property with a name given.
   ///
   virtual MVariant GetProperty(const MStdString& name) const;

   /// Set the property using name of the property, and value.
   ///
   /// The service allows extensions in children objects.
   ///
   /// \pre The property with such name should exist, otherwise
   /// an exception will be thrown. Any value-related or object-related
   /// exception can be thrown in case the given value has wrong type, or
   /// it is invalid, or the value cannot be set at this moment.
   ///
   virtual void SetProperty(const MStdString& name, const MVariant& value);

   /// Return the list of publicly available properties, persistent or not.
   ///
   /// \see GetAllPersistentPropertyNames for persistent properties only
   ///
   virtual MStdStringVector GetAllPropertyNames() const;

   /// Return the list of persistent properties.
   /// 
   /// \see GetAllPropertyNames for the list of all properties
   ///
   virtual MStdStringVector GetAllPersistentPropertyNames() const;

   /// Set the persistent properties of the object to their default values.
   /// Also look at DoSetPersistentPropertiesToDefault that is typically used in constructors
   /// of MObject parents that have persistent properties.
   ///
   /// \pre The object is built completely, will not work in constructor
   /// or destructor due to incomplete virtual table.
   ///
   virtual void SetPersistentPropertiesToDefault();

   /// Get the default value of persistent property with the name given.
   ///
   /// \pre The object should have the property with such name, and this
   /// property should have a default value (be persistent). Otherwise an exception
   /// is thrown.
   ///
   virtual MVariant GetPersistentPropertyDefaultValue(const MStdString& name) const;

   /// Set the persistent property with the name given to default value.
   /// If the persistent property does exist, the error never results from setting
   /// the value to this property, as it is the duty of the application developer
   /// to ensure that the value is correct (the assertion is in place for debug version).
   ///
   /// \pre The object should have the property with such name, and this
   /// property should have a default value (be persistent). Otherwise an exception
   /// is thrown.
   ///
   virtual void SetPersistentPropertyToDefault(const MStdString& name);

   /// Get the name of the type for the object (could be the same as class name).
   ///
   virtual const char* GetType() const;

   /// Intentionally, it will set the name of the type for the object,
   /// but the service will not allow setting the name to anything other
   /// than the current name.
   ///
   /// \pre The name supplied should match exactly the one which is got
   /// by GetType, otherwise the exception takes place.
   ///
   virtual void SetType(const MStdString&);

   /// Validate internal structures of the object.
   ///
   /// \pre The object is properly initialized.
   ///
   virtual void Validate();

   /// Tells if the given class name is available
   ///
   /// The call uses the reflection mechanism to tell if the class name exists
   ///
   /// \param name Class name to test
   /// \return True is returned if the given class is available, False otherwise
   ///
   static bool IsClassPresent(const MStdString& name)
   {
      return MClass::GetClass(name) != NULL;
   }

protected: // Methods:

   /// Set the persistent properties to their default values for one object provided the class for that object.
   ///
   /// This service is for calling it from constructors.
   /// A typical parameter is the static class of the object.
   /// The parameters for the parent properties are not touched.
   /// This is not a virtual service!
   ///
   /// \param staticClass Pointer to a class of this object, typically m_class
   ///
   /// \see M_SET_PERSISTENT_PROPERTIES_TO_DEFAULT Easiest way to call this method
   ///
   void DoSetPersistentPropertiesToDefault(const MClass* staticClass);

#endif // !M_NO_REFLECTION
};

// These includes have to appear after the MObject declaration.
// This is due to their hard interdependency.
//
#include <MCORE/MReflectedMacros.h>
#include <MCORE/MTypeCasting.h>
#include <MCORE/MPropertyDefinition.h>
#include <MCORE/MServiceDefinition.h>
#include <MCORE/MScopeSaviors.h>

///@}
#endif
