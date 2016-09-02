#ifndef MCORE_MINTERLOCKED_H
#define MCORE_MINTERLOCKED_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MInterlocked.h

#ifdef ewarm
   #include <intrinsics.h>
#endif

#include <MCORE/MCOREDefs.h>

/// System independent lightweight synchronization object that
/// synchronizes the access to one variable across multiple threads.
///
/// At present, the only atomic operations supported are increment and decrement.
///
/// Implementation note: On the majority of architectures, assignment
/// to and from a properly aligned unsigned variable is an atomic operation.
///
class MInterlocked
{
public: // Types:

   /// Type used by interlocked class by default
   ///
   typedef int ValueType;

public:

   /// Constructor of the interlocked variable.
   ///
   /// \param value
   ///     Initial value of interlocked variable
   ///
   MInterlocked(int value = 0)
   :
      m_value(value)
   {
   }

   /// Copy constructor that copies value from parameter.
   ///
   /// If during this operation the value of the other object keeps
   /// changing, the result value will be consistent, correspond
   /// to some value that was held by other object at an unknown 
   /// time of the copy operation.
   ///
   /// \param other
   ///     Interlocked object from which to copy value
   ///
   MInterlocked(const MInterlocked& other)
   :
      m_value(other)
   {
   }

   /// Assignment operator that takes integer value.
   ///
   /// When many threads attempt to change interlocked value
   /// at the same time, any one can be the last, and
   /// the result value will be atomically changed.
   ///
   /// \param value
   ///     Value of interlocked variable
   ///
   MInterlocked& operator=(int value)
   {
      m_value = value;
      return *this;
   }

   /// Object assignment operator.
   ///
   /// If during this operation the value of the other object keeps
   /// changing, the result value will be consistent, correspond
   /// to some value that was held by the other object at an unknown time of the copy operation.
   /// If during this operation the value of this object keeps changing with other assignment operators,
   /// the result of the copy can be discarded, however if increment or decrement operations are used,
   /// the increments will start to work on the new value.
   ///
   /// \param other
   ///     Interlocked object from which to copy value
   ///
   MInterlocked& operator=(const MInterlocked& other)
   {
      if ( this != &other )
         m_value = other;
      return *this;
   }

   /// Access the value of the interlocked class.
   ///
   /// If the object keeps changing from other threads,
   /// the result will be a consistent value at some definite but unknown period of time.
   ///
   operator int() const
   {
      return static_cast<const volatile int&>(m_value);
   }

   /// Atomically increment this object value and return result.
   ///
   /// This operation should be used in multithreaded environments in order to
   /// prevent situations when multiple threads would fail to change the same value
   /// in a consistent way.
   ///
   /// \return integer value that is the result of the operation.
   ///
   int operator++()
   {
      return IncrementAndFetch(&m_value);
   }

   /// Atomically increment this object value and return previous value.
   ///
   /// This operation should be used in multithreaded environments in order to
   /// prevent situations when multiple threads would fail to change the same value
   /// in a consistent way.
   ///
   /// \return integer value that had previously been in memory.
   ///
   int operator++(int)
   {
      return FetchAndIncrement(&m_value);
   }

   /// Atomically decrement this object value and return result.
   ///
   /// This operation should be used in multithreaded environments in order to
   /// prevent situations when multiple threads would fail to change the same value
   /// in a consistent way.
   ///
   /// \return integer value that is the result of the operation.
   ///
   int operator--()
   {
      return DecrementAndFetch(&m_value);
   }

   /// Atomically decrement this object value and return previous value.
   ///
   /// This operation should be used in multithreaded environments in order to
   /// prevent situations when multiple threads would fail to change the same value
   /// in a consistent way.
   ///
   /// \return integer value that had previously been in memory.
   ///
   int operator--(int)
   {
      return FetchAndDecrement(&m_value);
   }

public: // Static methods:

   /// Static function that atomically increments an integer in the given pointer 
   /// and returns the new value.
   ///
   /// This operation should be used in multithreaded environments in order to
   /// prevent situations when multiple threads would fail to change the same value
   /// in a consistent way.
   ///
   /// \param v
   ///     Volatile pointer to the integer that should be incremented
   ///
   /// \return integer value that is the result of the operation.
   ///
   static int IncrementAndFetch(volatile int* v)
   {
      #if (M_OS & M_OS_QNXNTO) != 0
         return atomic_add_value(reinterpret_cast<volatile unsigned*>(v), 1) + 1; // returns previous value
      #elif (M_OS & M_OS_WIN32_CE) != 0
         return ::InterlockedIncrement(const_cast<LONG*>(reinterpret_cast<volatile LONG*>(v)));
      #elif (M_OS & M_OS_WINDOWS) != 0
         return ::InterlockedIncrement(reinterpret_cast<volatile LONG*>(v));
      #elif (M_OS & M_OS_CMX) && defined(ewarm)
         {
            int o, n;
            do
            {
               o = __LDREX((unsigned long*)v);
               n = o + 1;
            } while(__STREX(n, (unsigned long*)v));
            return n;
         }
      #else // Otherwise assume GCC or compatibles
         return __sync_add_and_fetch(v, 1);
      #endif
   }

   /// Static function that atomically increments an integer in the given pointer
   /// and returns the previous value.
   ///
   /// This operation should be used in multithreaded environments in order to
   /// prevent situations when multiple threads would fail to change the same value
   /// in a consistent way.
   ///
   /// \param v
   ///     Volatile pointer to the integer that should be incremented
   ///
   /// \return integer value that had previously been in memory.
   ///
   static int FetchAndIncrement(volatile int* v)
   {
      #if (M_OS & M_OS_QNXNTO) != 0
         return atomic_add_value(reinterpret_cast<volatile unsigned*>(v), 1);
      #elif (M_OS & M_OS_WIN32_CE) != 0
         return ::InterlockedIncrement(const_cast<LONG*>(reinterpret_cast<volatile LONG*>(v))) - 1;
      #elif (M_OS & M_OS_WINDOWS) != 0
         return ::InterlockedIncrement(reinterpret_cast<volatile LONG*>(v)) - 1;
      #elif (M_OS & M_OS_CMX) != 0 && defined(ewarm)
         {
            int o, n;
            do
            {
               o = __LDREX((unsigned long*)v);
               n = o + 1;
            } while(__STREX(n, (unsigned long*)v));
            return o;
         }
      #else // Otherwise assume GCC or compatibles
         return __sync_fetch_and_add(v, 1);
      #endif
   }

   /// Static function that atomically decrements an integer in the given pointer
   /// and returns the new value.
   ///
   /// This operation should be used in multithreaded environments in order to
   /// prevent situations when multiple threads would fail to change the same value
   /// in a consistent way.
   ///
   /// \param v
   ///     Volatile pointer to the integer that should be decremented
   ///
   /// \return integer value that is the result of the operation.
   ///
   static int DecrementAndFetch(volatile int* v)
   {
      #if (M_OS & M_OS_QNXNTO) != 0
         return atomic_sub_value(reinterpret_cast<volatile unsigned*>(v), 1) - 1;
      #elif (M_OS & M_OS_WIN32_CE) != 0
         return ::InterlockedDecrement(const_cast<LONG*>(reinterpret_cast<volatile LONG*>(v)));
      #elif (M_OS & M_OS_WINDOWS) != 0
         return ::InterlockedDecrement(reinterpret_cast<volatile LONG*>(v)); // returns new value, not previous
      #elif (M_OS & M_OS_CMX) != 0 && defined(ewarm)
         {
            int o, n;
            do
            {
               o = __LDREX((unsigned long*)v);
               n = o - 1;
            } while(__STREX(n, (unsigned long*)v));
            return n;
         }
      #else // Otherwise assume GCC or compatibles
         return __sync_sub_and_fetch(v, 1);
      #endif
   }

   /// Static function that atomically decrements an integer in the given pointer
   /// and returns the previous value.
   ///
   /// This operation should be used in multithreaded environments in order to
   /// prevent situations when multiple threads would fail to change the same value
   /// in a consistent way.
   ///
   /// \param v
   ///     Volatile pointer to the integer that should be decremented
   ///
   /// \return integer value that had previously been in memory.
   ///
   static int FetchAndDecrement(volatile int* v)
   {
      #if (M_OS & M_OS_QNXNTO) != 0
         return atomic_sub_value(reinterpret_cast<volatile unsigned*>(v), 1);
      #elif (M_OS & M_OS_WIN32_CE) != 0
         return ::InterlockedDecrement(const_cast<LONG*>(reinterpret_cast<volatile LONG*>(v))) + 1;
      #elif (M_OS & M_OS_WINDOWS) != 0
         return ::InterlockedDecrement(reinterpret_cast<volatile LONG*>(v)) + 1; // returns new value, not previous
      #elif (M_OS & M_OS_CMX) != 0 && defined(ewarm)
         {
            int o, n;
            do
            {
               o = __LDREX((unsigned long*)v);
               n = o - 1;
            } while(__STREX(n, (unsigned long*)v));
            return o;
         }
      #else // Otherwise assume GCC or compatibles
         return __sync_fetch_and_sub(v, 1);
      #endif
   }

private: // Data:

   // Value itself
   //
   volatile int m_value;
};

///@}
#endif // MCORE_MINTERLOCKED_H
