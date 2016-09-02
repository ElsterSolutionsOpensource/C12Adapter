#ifndef MCORE_MSCOPESAVIORS_H
#define MCORE_MSCOPESAVIORS_H

#include <MCORE/MVariant.h>

/// Generic pointer that cannot be copied
///
/// Abstract class
///
template
   <typename Type>
class MGenericNoncopyablePtr
{
public: // Types:

   /// Type of the unique pointer.
   ///
   typedef Type element_type;

protected: // Constructor and destructor:

   /// Protected explicit initialization constructor.
   ///
   explicit MGenericNoncopyablePtr(Type* ptr = NULL) M_NO_THROW
   :
      m_pointer(ptr)
   {
   }

   /// Protected destructor.
   ///
   /// Generic pointer is a fully hidden.
   ///
   ~MGenericNoncopyablePtr()
   {
   }

public: // Operators:

   /// Field dereference operator.
   ///
   Type* operator->() const M_NO_THROW
   {
      return get();
   }

   /// Pointer dereference operator.
   ///
   Type& operator*() const M_NO_THROW
   {
      return *get();
   }

public: // Methods:

   /// Get the underlying pointer.
   ///
   Type* get() const M_NO_THROW
   {
      return m_pointer;
   }

   /// Return the the underlying pointer while nullifying the unique pointer object.
   ///
   Type* release() M_NO_THROW
   {
      Type* result = m_pointer;
      m_pointer = NULL;
      return result;
   }

private: // Copying operations are prohibited:

   MGenericNoncopyablePtr(MGenericNoncopyablePtr<Type>& other);
   MGenericNoncopyablePtr<Type>& operator=(MGenericNoncopyablePtr<Type>& other);

protected: // Data:

   /// Pointer to object.
   ///
   Type* m_pointer;
};

/// Compiler version independent unique pointer to a variable that is not an array.
///
/// This one avoids a problem with older compilers that do not support unique_ptr,
/// while at the same time silences deprecation warnings of newer compilers such as GNU GCC 6.1.1 or later.
///
/// The class interface is a proper subset of classic auto_ptr,
/// however it does not have a copy constructor at all, making it safe in the way unique_ptr is safe.
/// The whole codebase can be moved to using unique_ptr once all the clients of the library
/// upgrade their compilers from C++Builder 2007 and Visual C++ 2010.
///
/// One cannot give an array to unique pointer, and there is a static check for explicit array type.
/// Notice, it is still possible to use the class improperly if a given pointer is not an array,
/// but it points to an array.
///
/// \see MUniqueArrayPtr - unique pointer to an array variable
///
template
   <typename Type>
class MUniquePtr : public MGenericNoncopyablePtr<Type>
{
public: // Constructors:

   /// Explicit initialization constructor
   ///
   /// Usage is compatible with both std::auto_ptr and std::unique_ptr
   ///
   explicit MUniquePtr(Type* ptr = NULL) M_NO_THROW
   :
      MGenericNoncopyablePtr<Type>(ptr)
   {
   }

   /// Destructor
   ///
   /// Same as std::auto_ptr, and different from std::unique_ptr as it cannot handle arrays correctly.
   /// There is a compile time check that verifies the pointer is not declared as array.
   ///
   ~MUniquePtr()
   {
      #if  (__cplusplus > 199711) || (defined(_MSC_VER) && _MSC_VER >= 1600) // if this is C++11 or later or this is Visual C++ 2010 or later
         M_COMPILED_ASSERT(!std::is_array<Type>::value);
      #endif
      delete MGenericNoncopyablePtr<Type>::m_pointer;
   }

public: // Methods:

   /// Destroy the current object and assign a given pointer.
   ///
   /// Usage is compatible with both std::auto_ptr and std::unique_ptr
   ///
   void reset(Type* ptr = NULL)
   {
      if ( MGenericNoncopyablePtr<Type>::m_pointer != ptr )
         delete MGenericNoncopyablePtr<Type>::m_pointer;
      MGenericNoncopyablePtr<Type>::m_pointer = ptr;
   }
};

/// Compiler version independent unique pointer to an array variable.
///
/// This is similar to unique_ptr with an array deleter, while it works with the older
/// compilers. Otherwise, the class interface is a proper subset of classic auto_ptr,
/// however it does not have a copy constructor at all, making it safe in the way unique_ptr is safe.
/// The whole codebase can be moved to using unique_ptr with an array deleter
/// once all the clients of the library upgrade their compilers from C++Builder 2007 and Visual C++ 2010.
///
/// One cannot give a non-array to unique array pointer, and there is a static check for explicit array type.
///
/// \see MUniquePtr - unique pointer to a variable that is not an array
///
template
   <typename Type>
class MUniqueArrayPtr : public MGenericNoncopyablePtr<Type>
{
public: // Constructors:

   /// Explicit initialization constructor
   ///
   /// Usage is compatible with both std::auto_ptr and std::unique_ptr
   ///
   explicit MUniqueArrayPtr(Type* ptr = NULL) M_NO_THROW
   :
      MGenericNoncopyablePtr<Type>(ptr)
   {
   }

   /// Destructor
   ///
   /// Same as std::auto_ptr, and different from std::unique_ptr as it cannot handle arrays correctly.
   /// There is a compile time check that verifies the pointer is not declared as array.
   ///
   ~MUniqueArrayPtr()
   {
      delete [] MGenericNoncopyablePtr<Type>::m_pointer;
   }

public: // Methods:

   /// Destroy the current object and assign a given pointer.
   ///
   /// Usage is compatible with both std::auto_ptr and std::unique_ptr
   ///
   void reset(Type* ptr = NULL)
   {
      if ( MGenericNoncopyablePtr<Type>::m_pointer != ptr )
         delete [] MGenericNoncopyablePtr<Type>::m_pointer;
      MGenericNoncopyablePtr<Type>::m_pointer = ptr;
   }
};

/// Class that helps preserve a certain value of a variable outside a local C++ scope.
///
/// The constructor saves the given value, and the destructor restores it.
/// The type of the variable to manipulate shall be assignable, or a compile error will result from the usage attempt.
/// Typical use case:
/// \code
///     // assume count is nonzero here, say 10
///     {
///          MValueSavior<int> countSavior(count, 0); // here the previous count is saved, and initialized to zero
///          ... at this point the count can be manipulated freely
///     }
///     // here the value of count will be restored to 10 no matter how the above scope was exited
/// \endcode
///
template
   <typename Type>
class MValueSavior : public MGenericNoncopyablePtr<Type>
{
public:

   /// Constructor of the savior that accepts the pointer to the variable which value has to be saved.
   ///
   /// The constructor will store the address of the variable and its value, so it can be restored in the destructor.
   ///
   /// \param var
   ///     Pointer to the value that has to be saved through the time of existence of the value savior class.
   ///
   /// \pre
   ///     The type of variable shall be assignable with the assignment operator, or the code will not compile.
   ///
   MValueSavior(Type* var)
   :
      MGenericNoncopyablePtr<Type>(var),
      m_value(*var)
   {
   }

   /// Constructor of the savior that accepts the pointer to the value that has to be saved, and a new value.
   ///
   /// The constructor will store the address of the value and the value itself, so it can be restored in the destructor.
   /// Then it will assign a new value given as parameter. This constructor is a convenient shortcut of the following:
   /// \code
   ///      MValueSavior<MStdString> savior(myString);
   ///      myString = "new value";
   /// \endcode
   /// which can be written in a new line:
   /// \code
   ///      MValueSavior<MStdString> savior(myString, "new value");
   /// \endcode
   ///
   /// \param var
   ///     Pointer to the value that has to be saved through the time of existence of the value savior class.
   /// \param scopeValue
   ///     New value with which the given variable will be initialized within this constructor.
   ///
   /// \pre
   ///     The type of variable shall be assignable with the assignment operator, or the code will not compile.
   ///
   MValueSavior(Type* var, const Type& scopeValue)
   :
      MGenericNoncopyablePtr<Type>(var),
      m_value(*var)
   {
      *var = scopeValue;
   }

   /// Destructor that restores the variable given in constructor to its original value.
   ///
   /// As value savior is often created on the stack, there is no necessity to call
   /// its destructor directly or by means of \c operator \c delete.
   ///
   ~MValueSavior()
   {
      if ( MGenericNoncopyablePtr<Type>::m_pointer != NULL )
         *MGenericNoncopyablePtr<Type>::m_pointer = m_value;
   }

private: // Data:

   // Saved value to be restored at the end of the scope
   //
   Type  m_value;
};

/// Class that helps set a certain value when a certain scope exits.
///
/// This is basically to delay the assignment of a given variable to the time of destruction of end scope setter.
/// The type of the variable to manipulate shall be assignable, or a compile error will result from the usage attempt.
/// Typical use case:
/// \code
///     doneAnyhow = false; // assume this is a global variable
///     {
///          ...
///          MValueEndScopeSetter<int> endScopeSetter(doneAnyhow, true);
///          ...
///          // here the value of doneAnyhow can be freely manipulated
///          ...
///     }
///     // No matter how this scope is exited, with an exception or not, doneAnyhow will be true here
/// \endcode
///
template
   <typename Type>
class MValueEndScopeSetter : public MGenericNoncopyablePtr<Type>
{
public:

   /// Constructor of the end scope setter that accepts the pointer to the variable which shall be assigned at the end of the scope.
   ///
   /// The constructor will store the address of the variable and the value that shall be assigned at the end,
   /// and delay the assignment to the destructor.
   ///
   /// \param var
   ///     Pointer to the value that has to be assigned at destructor.
   /// \param endScopeValue
   ///     The value to which to assign \c *var at the time of destruction of end scope setter.
   ///
   /// \pre
   ///     The type of variable shall be assignable with the assignment operator, or the code will not compile.
   ///
   /// \see SetEndScopeValue - a way to overwrite endScopeValue parameter before destruction of end scope setter.
   ///
   MValueEndScopeSetter(Type* var, const Type& endScopeValue)
   :
      MGenericNoncopyablePtr<Type>(var),
      m_value(endScopeValue)
   {
   }

   /// Destructor, a place of delayed assignment of the variable given in constructor.
   ///
   ~MValueEndScopeSetter()
   {
      if ( MGenericNoncopyablePtr<Type>::m_pointer != NULL )
         *MGenericNoncopyablePtr<Type>::m_pointer = m_value;
   }

   /// A way of overwriting of the value that has to be assigned at destruction of end scope setter.
   ///
   void SetEndScopeValue(const Type& endScopeValue)
   {
      m_value = endScopeValue;
   }

private: // Data:

   // Value to be set at the end of the scope
   //
   Type m_value;
};

#if !M_NO_REFLECTION 

/// Class that helps preserve a property value outside a local C++ scope.
///
/// The constructor saves the given property by name, and the destructor restores it.
/// The type of the variable to manipulate shall be assignable, or a compile error will result from the usage attempt.
/// Typical use case:
/// \code
///     // assume Baud here is equal to 19200
///     {
///          MObjectPropertySavior baudSavior(channel, "Baud", 9600); // here the previous Baud is saved, and set to a new value 9600
///          ... at this point the baud can be manipulated freely, if necessary
///     }
///     // here the value of Baud will be restored to the previous value 19200 no matter how the above scope was exited
/// \endcode
///
/// Exception behavior has these specifics:
///   - If an exception takes place in the constructor at getting the property value, no attempt will be made at restoration of such
///     value in the destructor, as obviously, it is assumed the value need not be restored.
///   - If an exception takes place in the constructor later, at setting of property to the new value, the restoration
///     attempt will be attempted at destructor, as it is assumed it is not known whether the property is consistent.
///   - All exceptions in destructor are silenced, this is when the restoration attempt is made. 
///     Therefore, strictly speaking, the preservation of the value is not guaranteed.
///
class M_CLASS MObjectPropertySavior
{
public:

   /// Constructor that accepts the object and the name of the property that has to be saved
   ///
   /// The constructor will fetch the property value by name and store it for later restoration in destructor.
   ///
   /// \param obj
   ///     Object for which the property has to be preserve.
   /// \param propertyName
   ///     Name of the property to preserve. Such property should exist in object, and be read/write.
   ///
   MObjectPropertySavior(MObject* obj, const MStdString& propertyName);

   /// Constructor that accepts the object, the property name, and the new value of the property.
   ///
   /// The constructor will store the address of the value and the value itself, so it can be restored in the destructor.
   /// Then it will assign a new value given as parameter. This constructor is a convenient shortcut of the following:
   /// \code
   ///      MValueSavior<MStdString> savior(myString);
   ///      myString = "new value";
   /// \endcode
   /// which can be written in a new line:
   /// \code
   ///      MValueSavior<MStdString> savior(myString, "new value");
   /// \endcode
   ///
   /// The constructor will fetch the property value by name and store it for later restoration in destructor.
   ///
   /// \param obj
   ///     Object for which the property has to be preserve.
   /// \param propertyName
   ///     Name of the property to preserve. Such property should exist in object, and be read/write.
   /// \param value
   ///     New value to be assigned to object property, should satisfy the requirements for such property.
   ///
   MObjectPropertySavior(MObject* obj, const MStdString& propertyName, const MVariant& value);

   /// Destructor that restores the value of the property to its original value.
   ///
   /// As value savior is often created on the stack there is no necessity to call
   /// its destructor directly or by means of \c operator \c delete.
   ///
   ~MObjectPropertySavior() M_NO_THROW;

private: // Data:

   // Object pointer
   //
   MObject* m_object;

   // Property name within the object
   //
   MStdString m_propertyName;
   
   // Saved property value, the one to be restored in the destructor
   //
   MVariant m_propertyValue;
};

#endif // !M_NO_REFLECTION

#endif // MCORE_MSCOPESAVIORS_H
