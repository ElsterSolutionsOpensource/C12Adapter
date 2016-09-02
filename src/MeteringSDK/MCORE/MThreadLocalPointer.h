#ifndef MCORE_MTHREADLOCALPOINTER_H
#define MCORE_MTHREADLOCALPOINTER_H
/// \addtogroup MCORE
/// @{
/// \file MCORE/MThreadLocalPointer.h

#if !M_NO_MULTITHREADING

///@{
/// Compiler-native thread storage specifier for a variable, efficient implementation.
///
/// With this approach, a thread local variable is declared like this:
/// \code
///      M_THREAD_LOCAL_STORAGE_VARIABLE MyType myThreadLocalVariable;
/// \endcode
/// List of known platforms where this is supported (Notice that Windows XP has a buggy implementation!):
///  - Windows since Windows Vista using Visual C++ and Borland C++Builder
///  - Linux with GCC version >= 45000
///  - Very likely, many other GCC based platforms
///
/// \see MThreadLocalPointer for more portable but less efficient API based implementation.
/// \see M_THREAD_LOCAL_POINTER for a conditional macro that attempts to determine
///         the most efficient while portable/workable implementation.
///
#if defined(_MSC_VER) || defined(__BORLANDC__)
   #define M_THREAD_LOCAL_STORAGE_VARIABLE __declspec(thread)
#elif defined(__GNUC__) || defined(M_DOXYGEN)
   #define M_THREAD_LOCAL_STORAGE_VARIABLE __thread
#else
    #error "M_THREAD_LOCAL_POINTER in not supported by compiler"
#endif
///@}

///@{
/// Whether to use compiler-native \ref M_THREAD_LOCAL_STORAGE_VARIABLE in \ref M_THREAD_LOCAL_POINTER.
///
/// Determine whether to use local storage natively supported by the compiler and development environment,
/// or rather use a more portable API-based class \ref MThreadLocalPointer.
///
/// When the user does not supply this value, an attempt to determine it from the environment is made.
/// Currently, an assumption is made that we have to support Windows XP,
/// the one with buggy native __declspec(thread).
///
/// \see M_THREAD_LOCAL_POINTER the user level macro to declare the most portable and efficient thread local pointer.
///
#ifndef M__USE_THREAD_LOCAL_POINTER
   #define M__USE_THREAD_LOCAL_POINTER ((defined(__GNUC__) && M_GCC_VERSION >= 44300) || defined(__clang__))
#endif
///@}

///@{
/// API based thread storage specifier for a variable, most portable implementation.
///
/// With this approach, a thread local variable is declared like this:
/// \code
///      // Notice that myThreadLocalVariable will be a pointer of type MyType*
///      //
///      M_THREAD_LOCAL_POINTER(MyType) myThreadLocalVariable;
/// \endcode
/// To ensure portability, using this define requires compiling code with either 0 or 1 value
/// of \ref M__USE_THREAD_LOCAL_POINTER.
///
/// \see MThreadLocalStorage class that implements API based variant of the pointer.
/// \see M_THREAD_LOCAL_STORAGE_VARIABLE less portable but more efficient thread local storage.
/// \see M__USE_THREAD_LOCAL_POINTER determines which method is used for M_THREAD_LOCAL_POINTER.
///
#if M__USE_THREAD_LOCAL_POINTER
   #define M_THREAD_LOCAL_POINTER(T) M_THREAD_LOCAL_STORAGE_VARIABLE T*
#else
   #define M_THREAD_LOCAL_POINTER(T) MThreadLocalPointer<T>
#endif
///@}

/// Thread-local pointer, the one that will be different in every thread, API based portable implementation.
///
/// There are three opportunities for the user to use Thread local storage:
///    1. Use this thread local pointer class, which is the most portable way.
///    2. Use native compiler declaration by utilizing the macro \ref M_THREAD_LOCAL_STORAGE_VARIABLE.
///    3. Use macro \ref M_THREAD_LOCAL_POINTER that attempts to be efficient on platforms that support
///       approach 2, while rolling back to approach 1 on less efficient platforms.
///
/// \see M_THREAD_LOCAL_STORAGE_VARIABLE Efficient approach, less portable.
/// \see M_THREAD_LOCAL_POINTER Efficient on platforms that support thread local variable, portable otherwise.
/// \see M__USE_THREAD_LOCAL_POINTER Macro that determines which approach is used by \ref M_THREAD_LOCAL_POINTER.
///
template
   <class T>
class MThreadLocalPointer
{
public:
   // constructor, destructor

   /// Constructor that does not initialize the value of the pointer.
   ///
   /// The value of the pointer will remain undefined until the assignment operator is called.
   ///
   MThreadLocalPointer()
   {
      DoInitialize();
   }

   /// Constructor to initialize the pointer with the given value.
   ///
   MThreadLocalPointer(const T* p)
   {
      DoInitialize();
      Set(p);
   }

   /// Copy constructor that takes another thread local pointer.
   ///
   /// \param other
   ///     Other object of the same type or its child.
   ///
   MThreadLocalPointer(const MThreadLocalPointer& other)
   {
      DoInitialize();
      Set(other.Get());
   }

   /// Destructor, clear system resources.
   ///
   ~MThreadLocalPointer()
   {
      #if (M_OS & M_OS_WINDOWS) != 0
         BOOL success = TlsFree(m_index);
         M_USED_VARIABLE(success);
         M_ASSERT(success);
      #else
         int error = pthread_key_delete(m_index);
         M_USED_VARIABLE(error);
         M_ASSERT(!error);
      #endif
   }

private: // Members:

   /// Dereference the pointer.
   ///
   T* Get()
   {
      #if (M_OS & M_OS_WINDOWS) != 0
         void* result = TlsGetValue(m_index);
         if ( result == NULL && GetLastError() != NO_ERROR ) // NULL can be a valid value
         {
            MESystemError::ThrowLastSystemError();
            M_ENSURED_ASSERT(0);
         }
         return reinterpret_cast<T*>(result);
      #else
         // NULL is a normal return, no error can be detected
         return reinterpret_cast<T*>(pthread_getspecific(m_index));
      #endif
   }

   /// Assign the pointer.
   ///
   void Set(T* value)
   {
      #if (M_OS & M_OS_WINDOWS) != 0
         if ( !TlsSetValue(m_index, reinterpret_cast<void*>(value)) )
         {
            MESystemError::ThrowLastSystemError();
            M_ENSURED_ASSERT(0);
         }
      #else
         int error = pthread_setspecific(m_index, reinterpret_cast<void*>(value));
         if ( error )
         {
            MESystemError::Throw(error);
            M_ENSURED_ASSERT(0);
         }
      #endif
   }

public: // Operators:

   /// Assignment operator that takes another thread local pointer.
   /// The operator returns void, and the chaining of a few of them is impossible.
   ///
   /// \param other
   ///     Other object of the same type or its child.
   ///
   void operator=(const MThreadLocalPointer& other)
   {
      if ( this != &other )
      {
         Set((T*)other);
      }
   }

   /// Assignment operator that takes a pointer.
   ///
   /// The operator returns void, and the chaining of a few of them is impossible.
   ///
   void operator=(T* ptr)
   {
      Set(ptr);
   }

   /// Return the pointer that is associated with this thread local pointer class.
   /// This makes possible the pointer-like behavior of this class.
   ///
   operator T*()
   {
      return Get();
   }

   /// Return the reference to client object associated with this pointer class.
   /// This makes possible the pointer-like behavior of this class.
   ///
   T& operator*()
   {
      T* result = Get();
      M_ASSERT(result != NULL);
      return *result;
   }

   /// Return the constant reference to client object associated with this shared pointer class.
   /// This makes possible the pointer-like behavior of this class.
   ///
   const T& operator*() const
   {
      T* result = Get();
      M_ASSERT(result != NULL);
      return *result;
   }

   /// Dereference the pointer that is associated with this class.
   /// This makes possible the pointer-like behavior of this class.
   ///
   T* operator->()
   {
      T* result = Get();
      M_ASSERT(result != NULL);
      return result;
   }

   /// Dereference the constant pointer that is associated with this shared pointer class.
   /// This makes possible the pointer-like behavior of this class.
   ///
   const T* operator->() const
   {
      T* result = Get();
      M_ASSERT(result != NULL);
      return result;
   }

   /// Equality comparison operator that takes two thread local pointers.
   ///
   friend bool operator==(const MThreadLocalPointer<T>& p1, const MThreadLocalPointer<T>& p2)
   {
      return p1.Get() == p2.Get();
   }

   /// Equality comparison operator that takes a shared pointer and a pointer.
   ///
   friend bool operator==(const MThreadLocalPointer<T>& p1, const T* p2)
   {
      return p1.Get() == p2;
   }

   /// Equality comparison operator that takes a pointer and a shared pointer.
   ///
   friend bool operator==(const T* p1, const MThreadLocalPointer<T>& p2)
   {
      return p1 == p2.Get();
   }

   /// Inequality comparison operator that takes two shared pointers.
   ///
   friend bool operator!=(const MThreadLocalPointer<T>& p1, const MThreadLocalPointer<T>& p2)
   {
      return p1.Get() != p2.Get();
   }

   /// Inequality comparison operator that takes a shared pointer and a pointer.
   ///
   friend bool operator!=(const MThreadLocalPointer<T>& p1, const T* p2)
   {
      return p1.Get() != p2;
   }

   /// Inequality comparison operator that takes a pointer and a shared pointer.
   ///
   friend bool operator!=(const T* p1, const MThreadLocalPointer<T>& p2)
   {
      return p1 != p2.Get();
   }

private: // Methods:

   void DoInitialize()
   {
      #if (M_OS & M_OS_WINDOWS) != 0
         m_index = TlsAlloc();
         if ( m_index == TLS_OUT_OF_INDEXES )
         {
            MESystemError::ThrowLastSystemError();
            M_ENSURED_ASSERT(0);
         }
      #else
         int error = pthread_key_create(&m_index, NULL);
         if ( error )
         {
            MESystemError::Throw(error);
            M_ENSURED_ASSERT(0);
         }
      #endif
   }

private: // Data:

#if (M_OS & M_OS_WINDOWS) != 0
   DWORD m_index;
#else
   pthread_key_t m_index;
#endif
};

#endif // !M_NO_MULTITHREADING
///@}
#endif
