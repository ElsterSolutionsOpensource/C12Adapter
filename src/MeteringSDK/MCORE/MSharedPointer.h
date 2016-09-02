#ifndef MCORE_MSHAREDPOINTER_H
#define MCORE_MSHAREDPOINTER_H
/// \file MCORE/MSharedPointer.h
/// \addtogroup MCORE
///@{

#include <MCORE/MInterlocked.h>

/// Generic intrusive shared pointer.
///
/// Shared pointer should be used for those classes, where ownership
/// is shared among objects that can have different and generally unpredictable life spans.
/// An important restriction is that there should be no circular dependency in shared ownership
/// of such objects, as in this case the objects will hold references to themselves forever.
/// Also, shared pointer cannot hold an array of elements.
/// These restrictions shall be watched at design and development time.
///
/// The reason why the intrusive shared pointer is implemented as a set of macros
/// is to avoid multiple inheritance for types that have to be parts of the existing
/// implementation hierarchy. While macros are somewhat uglier, they do what we want.
///
/// The class to which a shared pointer can point should include a macro
/// \ref M_SHARED_POINTER_CLASS, which will define a special \c typedef \c SharedPointer,
/// and a set of private definitions to handle sharing. Here is an example:
/// \code
///    // Definition:
///    class MSomeClass
///    {
///    public:
///       MSomeClass()
///       :
///          M_SHARED_POINTER_CLASS_INIT
///       {
///       }
///
///       MSomeClass(const MStdString& name)
///       :
///          M_SHARED_POINTER_CLASS_INIT,
///          m_name(name)
///       {
///       }
///  
///       ....
///       M_SHARED_POINTER_CLASS(SomeClass)
///    };
/// \endcode
/// Usage:
/// \code
///    {
///       MSomeClass::SharedPointer ptr = new MSomeClass()
///       MSomeClass::SharedPointer otherPtr = ptr;
///       ptr = NULL;
///       otherPtr = NULL; // delete will be called on MSomeClass object here.
///    }
/// \endcode
template
   <class C>
class MSharedPointer
{
public: // Types:

   /// Static type of the client class or its parent as used during template definition.
   ///
   typedef C ClientType;

public: // Constructors, destructor:

   /// Default constructor to initialize the pointer to \c NULL value.
   ///
   MSharedPointer() 
   :
      m_pointer(NULL)
   {
   }

   /// Constructor that takes a pointer to client as parameter.
   ///
   /// The shared pointer will be initialized and the usage count will be increased in client.
   ///
   /// \param client
   ///     Client object defined with \ref M_SHARED_POINTER_CLASS macro
   ///
   MSharedPointer(C* client) 
   :
      m_pointer(client)
   {
      DoAddRef(); // if client is NULL, it will take care inside.
   }

   /// Copy constructor that takes another shared pointer.
   ///
   /// \param other
   ///     Other object of the same type or its child
   ///
   /// \pre The shared pointer given is a valid shared object.
   /// No explicit checks are done, but in many cases there will be a compile error otherwise.
   ///
   MSharedPointer(const MSharedPointer& other)
   {
      m_pointer = other.m_pointer;
      DoAddRef();
   }

   /// Shared pointer destructor.
   ///
   /// If the client object is not \c NULL, decrease the usage count,
   /// and when the usage count reaches zero, delete the client object.
   ///
   ~MSharedPointer()
   {
      DoReleaseRef();
   }

public: // Methods and operators:

   /// Get the current number of usage references in the object.
   ///
   /// If the shared pointer is \c NULL, return zero by convention.
   /// If the shared pointer is not \c NULL, zero count is impossible due to implementation.
   ///
   int GetNumberOfReferences() const
   {
      return (m_pointer == NULL) ? 0 : (int)m_pointer->m_numRefs;
   }

   /// Assignment operator that takes another shared pointer.
   ///
   /// \param other
   ///     Other object of the same type or its child
   ///
   /// \pre The shared pointer given is a valid shared object, not a \c NULL.
   /// No explicit checks are done, but in many cases there will be a compile error otherwise.
   ///
   MSharedPointer& operator=(const MSharedPointer& other)
   {
      if ( m_pointer != other.m_pointer )    // different pointers
      {
         DoReleaseRef();
         m_pointer = other.m_pointer;
         DoAddRef();
      }

      return *this;
   }

   /// Assignment operator that takes a pointer.
   ///
   /// \pre The shared pointer given is a valid shared object, not a \c NULL.
   /// No explicit checks are done, but in many cases there will be a compile error otherwise.
   ///
   MSharedPointer& operator=(C* ptr)
   {
      if ( m_pointer != ptr )    // different pointers
      {
         DoReleaseRef();
         m_pointer = ptr;
         DoAddRef();
      }

      return *this;
   }

   /// Return the pointer that is associated with this shared pointer class.
   ///
   /// This makes possible the pointer-like behavior of this class.
   ///
   operator C*() const
   {
      return m_pointer;
   }

   /// Return the reference to client C object associated with this shared pointer class.
   ///
   /// This makes possible the pointer-like behavior of this class.
   ///
   /// \pre On debug build, the assertion is done whether the pointer is not \c NULL.
   ///
   C& operator*()
   {
      M_ASSERT(m_pointer != NULL);
      return *m_pointer;
   }

   /// Return the constant reference to client C object associated with this shared pointer class.
   /// This makes possible the pointer-like behavior of this class.
   ///
   /// \pre On debug build, the assertion is done whether the pointer is not \c NULL.
   ///
   const C& operator*() const
   {
      M_ASSERT(m_pointer != NULL);
      return *m_pointer;
   }

   /// Dereference the pointer that is associated with this shared pointer class.
   /// This makes possible the pointer-like behavior of this class.
   ///
   /// \pre The C object has to be associated with this object. Otherwise there
   /// is an assertion hit in the debugging version.
   ///
   C* operator->()
   {
      M_ASSERT(m_pointer != NULL);
      return m_pointer;
   }

   /// Dereference the constant pointer that is associated with this shared pointer class.
   /// This makes possible the pointer-like behavior of this class.
   ///
   /// \pre The C object has to be associated with this object. Otherwise there
   /// is an assertion hit in the debugging version.
   ///
   const C* operator->() const
   {
      M_ASSERT(m_pointer != NULL);
      return m_pointer;
   }

   /// Equality comparison operator that takes two shared pointers.
   ///
   friend bool operator==(const MSharedPointer<C>& p1, const MSharedPointer<C>& p2)
   {
      return p1.m_pointer == p2.m_pointer;
   }

   /// Equality comparison operator that takes a shared pointer and a pointer.
   ///
   friend bool operator==(const MSharedPointer<C>& p1, const C* p2)
   {
      return p1.m_pointer == p2;
   }

   /// Equality comparison operator that takes a pointer and a shared pointer.
   ///
   friend bool operator==(const C* p1, const MSharedPointer<C>& p2)
   {
      return p1 == p2.m_pointer;
   }

   /// Inequality comparison operator that takes two shared pointers.
   ///
   friend bool operator!=(const MSharedPointer<C>& p1, const MSharedPointer<C>& p2)
   {
      return p1.m_pointer != p2.m_pointer;
   }

   /// Inequality comparison operator that takes a shared pointer and a pointer.
   ///
   friend bool operator!=(const MSharedPointer<C>& p1, const C* p2)
   {
      return p1.m_pointer != p2;
   }

   /// Inequality comparison operator that takes a pointer and a shared pointer.
   ///
   friend bool operator!=(const C* p1, const MSharedPointer<C>& p2)
   {
      return p1 != p2.m_pointer;
   }

   /// Add the reference to the client object.
   ///
   /// If the client is \c NULL, nothing is done by this call.
   /// This service is public, while direct manipulation of reference count should be very rare.
   ///
   void DoAddRef()
   {
      if ( m_pointer != NULL )
         ++ m_pointer->m_numRefs;
   }

   /// Release the reference to the client object.
   ///
   /// If the client is \c NULL, nothing is done by this call.
   /// If the count reaches zero, delete client object and nullify pointer.
   /// This service is public, while direct manipulation of reference count should be very rare.
   ///
   void DoReleaseRef()
   {
      if ( m_pointer != NULL )
      {
         if ( -- m_pointer->m_numRefs == 0 )
         {
            delete m_pointer;
            m_pointer = NULL; // nullify, so we know that we point to nothing
         }
      }
   }

private: // Attributes:

   // Pointer to the client object, which is shared among several shared pointers
   //
   C* m_pointer;
};

/// Macro that should be defined at the end of the class, which is the client of MSharedPointer.
///
/// The macro will define type SharedPointer, and a set of private definitions
/// needed to implement the functionality.
///
/// \param C - the parameter is the class name without the \c M prefix.
///
/// \see MSharedPointer for more information and an example.
///
#define M_SHARED_POINTER_CLASS(C) \
   public: typedef MSharedPointer<M ## C> SharedPointer; \
   private: MInterlocked m_numRefs; \
   friend class MSharedPointer<M ## C>;

/// Macro that should be defined at the initialization section of constructor for the shared class.
///
/// If there is more data to follow, just use comma after the macro.
///
/// \see MSharedPointer for more information and an example.
///
#define M_SHARED_POINTER_CLASS_INIT \
   m_numRefs(0)

///@}
#endif
