#ifndef MCORE_MTYPECASTING_H
#define MCORE_MTYPECASTING_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MTypeCasting.h
///
/// Type casting helpers.
///

#ifndef MCORE_MOBJECT_H
   #error "This header file has hard interdepencency from reflection headers. Please include MCORE/MObject.h or MCORE/MCORE.h instead of this file."
#endif

/// Macro M_RTTI_SUPPORTED will tell the best guess if RTTI is supported by the current compiler
/// and its options. It is used primarily internally.
///
#if defined(_MSC_VER)
   #if defined(_CPPRTTI)
      #define M_RTTI_SUPPORTED 1
   #else
      #define M_RTTI_SUPPORTED 0
   #endif
#else
   #define M_RTTI_SUPPORTED 1
#endif

/// Casting nonzero pointers statically, with check on debug time.
/// If the target object is the one expected, it is returned. Otherwise an assertion is fired.
/// Simply speaking, dynamic_cast with assert is used during debugging, and static_cast on release.
///
#if defined(M_DEBUG) && M_DEBUG && M_RTTI_SUPPORTED
   #define M_CHECKED_CAST(t,p)           (MCheckPointer(static_cast<t>(p)))
#else
   #define M_CHECKED_CAST(t,p)           (static_cast<t>(p))
#endif
   
/// Casting possibly zero pointers statically, with check on debug time.
/// If the target object is the one expected, or it is zero, it is returned. Otherwise an assertion is fired.
///
/// \see M_CHECKED_CAST
///
#if defined(M_DEBUG) && M_DEBUG && M_RTTI_SUPPORTED
   #define M_CHECKED_CAST_OR_NULL(t,p)   (MCheckPointerOrNull(static_cast<t>(p)))
#else
   #define M_CHECKED_CAST_OR_NULL(t,p)   (static_cast<t>(p))
#endif

/// \cond SHOW_INTERNAL

#if defined(M_DEBUG) && M_DEBUG && M_RTTI_SUPPORTED
   // Helper function that actually does asserted checking of the pointer
   //
   template
      <class T>
   inline T MCheckPointer(T p)
   {
      M_ASSERT(dynamic_cast<T>(p) != NULL);
      return p;
   }

   template
      <class T>
   inline T MCheckPointerOrNull(T p)
   {
      if ( p == NULL )
         return NULL;
      return MCheckPointer(p);
   }
#endif

// Inline helper that does the casting job for a nonconst object. The syntax of its usage is close to the standard dynamic_cast.
// We do not use it directly in the source code due to compatibility problems with different compilers.
//
template
   <class To>
inline To* MDoObjectDynamicCast(MObject* from)
{
   M_ENSURED_ASSERT(from != NULL); // this is what is different from dynamic_cast.
   if ( from->GetClass()->IsKindOf(To::GetStaticClass()) )
   {
      To* ret = M_CHECKED_CAST(To*, from);
      M_ENSURED_ASSERT(ret != NULL);   // This helps optimizing ifs that check the object type.
      return ret;
   }
   else
      return NULL;
}

// Inline helper that does the casting job for a const object. The syntax of its usage is close to the standard dynamic_cast.
// We do not use it directly in the source code due to compatibility problems with different compilers.
//
template
   <class To>
inline To* MDoObjectDynamicCast(const MObject* from)
{
   M_ENSURED_ASSERT(from != NULL); // this is what is different from dynamic_cast.
   if ( from->GetClass()->IsKindOf(To::GetStaticClass()) )
   {
      const To* ret = M_CHECKED_CAST(const To*, from);
      M_ENSURED_ASSERT(ret != NULL);  // This helps optimizing ifs that check the object type.
      return ret;
   }
   else
      return NULL;
}

// Inline helper that does the casting job for a nonconst object. The syntax of its usage is close to the standard dynamic_cast.
// We do not use it directly in the source code due to compatibility problems with different compilers.
//
template
   <class To>
inline To* MDoObjectDynamicCastWithNullCheck(MObject* from)
{
   if ( from == NULL )
      return NULL;
   return MDoObjectDynamicCast<To>(from);
}

// Inline helper that does the casting job for a const object. The syntax of its usage is close to the standard dynamic_cast.
// We do not use it directly in the source code due to compatibility problems with different compilers.
//
template
   <class To>
inline To* MDoObjectDynamicCastWithNullCheck(const MObject* from)
{
   if ( from == NULL )
      return NULL;
   return MDoObjectDynamicCast<const To>(from);
}

// Inline helper that does the casting job for a nonconst object. The syntax of its usage is close to the standard dynamic_cast.
// We do not use it directly in the source code due to compatibility problems with different compilers.
//
template
   <class To>
inline To* MDoObjectDynamicCastWithThrow(MObject* from)
{
   To* o = MDoObjectDynamicCastWithNullCheck<To>(from);
   if ( o == NULL )
   {
      MClass::DoThrowCannotConvert(from, To::GetStaticClass());
      M_ENSURED_ASSERT(0);
   }
   return o;
}

// Inline helper that does the casting job for a const object. The syntax of its usage is close to the standard dynamic_cast.
// We do not use it directly in the source code due to compatibility problems with different compilers.
//
template
   <class To>
inline const To* MDoObjectDynamicCastWithThrow(const MObject* from)
{
   const To* o = MDoObjectDynamicCastWithNullCheck<const To>(from);
   if ( o == NULL )
   {
      MClass::DoThrowCannotConvert(from, To::GetStaticClass());
      M_ENSURED_ASSERT(0);
   }
   return o;
}

/// \endcond SHOW_INTERNAL

/// Pointer-based variation of the standard dynamic cast for objects, not NULL, usable for MObject children.
/// Because there is no multiple inheritance, this macro call is many times faster than the standard dynamic cast,
/// and it also resolves the problem on some CE platforms where dynamic_cast is not properly implemented.
/// Different from \ref M_CHECKED_CAST, this macro takes non const-volatile-qualified name of the target class, 
/// and there is no star to denote a pointer.
///
/// The difference in behavior with the standard dynamic_cast is that they do not accept NULL as a valid argument.
///
/// \param c Must be a symbolic name of a class derived from MObject, 
///          and this class shall be reflected with appropriate macros.
///          Otherwise compiler errors will appear, or a cast will not work as expected.
/// \param o Object to be casted into class c.
///          It must be a nonzero object, a debug time assertion will be hit. 
/// \return 
///   - If o is of requested type, pointer casted to a requested object is returned.
///   - Otherwise NULL is returned.
///
/// Example:
/// \code
///     MDataObject* dataObj = M_DYNAMIC_CAST(MDataObject, obj);
/// \endcode
///
/// \see M_DYNAMIC_CONST_CAST
/// \see M_DYNAMIC_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CONST_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CAST_WITH_THROW
/// \see M_DYNAMIC_CONST_CAST_WITH_THROW
///
#define M_DYNAMIC_CAST(c,o)   MDoObjectDynamicCast<c>(o)

/// Constant version of pointer-based variation of the standard dynamic cast for objects, not NULL, usable for MObject children.
/// Because there is no multiple inheritance, this macro call is many times faster than the standard dynamic cast,
/// and it also resolves the problem on some CE platforms where dynamic_cast is not properly implemented.
/// Different from =\ref M_CHECKED_CAST, this macro takes non const-volatile-qualified name of the target class, 
/// and there is no star to denote a pointer.
///
/// The difference in behavior with the standard dynamic_cast is that they do not accept NULL as a valid argument.
///
/// \param c Must be a symbolic name of a class derived from MObject, 
///          and this class shall be reflected with appropriate macros.
///          Qualifier const shall not be mentioned.
///          Otherwise compiler errors will appear, or a cast will not work as expected.
/// \param o Object to be casted into class c.
///          It must be a non-NULL object, or a debug time assertion will be hit.
/// \return 
///   - If o is of requested constant type, constant pointer casted to a requested object is returned.
///   - Otherwise NULL is returned.
///
/// Example:
/// \code
///     const MDataObject* dataObj = M_DYNAMIC_CONST_CAST(MDataObject, obj);
/// \endcode
///
/// \see M_DYNAMIC_CAST
/// \see M_DYNAMIC_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CONST_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CAST_WITH_THROW
/// \see M_DYNAMIC_CONST_CAST_WITH_THROW
///
#define M_DYNAMIC_CONST_CAST(c,o)   MDoObjectDynamicCast<const c>(o)

/// Pointer-based variation of the standard dynamic cast for objects or NULL, usable for MObject children.
/// Because there is no multiple inheritance, this macro call is many times faster than the standard dynamic cast,
/// and it also resolves the problem on some CE platforms where dynamic_cast is not properly implemented.
/// Different from \ref M_CHECKED_CAST, this macro takes non const-volatile-qualified name of the target class, 
/// and there is no star to denote a pointer.
///
/// When NULL value is given as object, the macro behaves like standard pointer-based dynamic_cast, returns NULL.
///
/// \param c Must be a symbolic name of a class derived from MObject, 
///          and this class shall be reflected with appropriate macros.
///          Otherwise compiler errors will appear, or a cast will not work as expected.
/// \param o Object to be casted into class c.
///          NULL pointer value can be given, in which case NULL is returned.
/// \return 
///   - If o is of requested type, pointer casted to a requested object is returned.
///   - Otherwise NULL is returned.
///
/// \see M_DYNAMIC_CAST
/// \see M_DYNAMIC_CONST_CAST
/// \see M_DYNAMIC_CONST_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CAST_WITH_THROW
/// \see M_DYNAMIC_CONST_CAST_WITH_THROW
///
#define M_DYNAMIC_CAST_WITH_NULL_CHECK(c,o)       MDoObjectDynamicCastWithNullCheck<c>(o)

/// Constant version of pointer-based variation of the standard dynamic cast for objects or NULL, usable for MObject children.
/// Because there is no multiple inheritance, this macro call is many times faster than the standard dynamic cast,
/// and it also resolves the problem on some CE platforms where dynamic_cast is not properly implemented.
/// Different from =\ref M_CHECKED_CAST, this macro takes non const-volatile-qualified name of the target class, 
/// and there is no star to denote a pointer.
///
/// When NULL value is given as object, the macro behaves like standard pointer-based dynamic_cast, returns NULL.
///
/// \param c Must be a symbolic name of a class derived from MObject, 
///          and this class shall be reflected with appropriate macros.
///          Qualifier const shall not be mentioned.
///          Otherwise compiler errors will appear, or a cast will not work as expected.
/// \param o Object to be casted into class c.
///          NULL pointer value can be given, in which case NULL is returned.
/// \return 
///   - If o is of requested constant type, constant pointer casted to a requested object is returned.
///   - Otherwise NULL is returned.
///
/// \see M_DYNAMIC_CAST
/// \see M_DYNAMIC_CONST_CAST
/// \see M_DYNAMIC_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CAST_WITH_THROW
/// \see M_DYNAMIC_CONST_CAST_WITH_THROW
///
#define M_DYNAMIC_CONST_CAST_WITH_NULL_CHECK(c,o)   MDoObjectDynamicCastWithNullCheck<const c>(o)

/// Pointer-based variation of the standard dynamic cast for objects, not NULL, usable for MObject children.
/// Because there is no multiple inheritance, this macro call is many times faster than the standard dynamic cast,
/// and it also resolves the problem on some CE platforms where dynamic_cast is not properly implemented.
/// Different from \ref M_CHECKED_CAST, this macro takes non const-volatile-qualified name of the target class, 
/// and there is no star to denote a pointer.
///
/// Much like reference-based dynamic_cast, this call will throw an exception if the given object is not of the type requested.
///
/// \param c Must be a symbolic name of a class derived from MObject, 
///          and this class shall be reflected with appropriate macros.
///          Otherwise compiler errors will appear, or a cast will not work as expected.
/// \param o Object to be casted into class c.
///          NULL pointer value can be given, in which case NULL is returned.
/// \return Pointer casted to a requested object is returned. NULL is never returned from this call.
///
/// \see M_DYNAMIC_CAST
/// \see M_DYNAMIC_CONST_CAST
/// \see M_DYNAMIC_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CONST_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CONST_CAST_WITH_THROW
///
#define M_DYNAMIC_CAST_WITH_THROW(c,o)            MDoObjectDynamicCastWithThrow<c>(o)

/// Constant version of pointer-based variation of the standard dynamic cast for objects, not NULL, usable for MObject children.
/// Because there is no multiple inheritance, this macro call is many times faster than the standard dynamic cast,
/// and it also resolves the problem on some CE platforms where dynamic_cast is not properly implemented.
/// Different from =\ref M_CHECKED_CAST, this macro takes non const-volatile-qualified name of the target class, 
/// and there is no star to denote a pointer.
///
/// \param c Must be a symbolic name of a class derived from MObject, 
///          and this class shall be reflected with appropriate macros.
///          Qualifier const shall not be mentioned.
///          Otherwise compiler errors will appear, or a cast will not work as expected.
/// \param o Object to be casted into class c.
///          NULL pointer value can be given, in which case NULL is returned.
/// \return Pointer casted to a requested object is returned. NULL is never returned from this call.
///
/// \see M_DYNAMIC_CAST
/// \see M_DYNAMIC_CONST_CAST
/// \see M_DYNAMIC_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CONST_CAST_WITH_NULL_CHECK
/// \see M_DYNAMIC_CAST_WITH_THROW
///
#define M_DYNAMIC_CONST_CAST_WITH_THROW(c,o)      MDoObjectDynamicCastWithThrow<const c>(o)


#if (M_OS & M_OS_WIN64) == M_OS_WIN64
/// \cond SHOW_INTERNAL

   // Helper functions to avoid using M_I macro in this header
   M_FUNC M_NORETURN_FUNC void MDoThrowBadConversionUint64(const char* typeName);
   M_FUNC M_NORETURN_FUNC void MDoThrowBadConversionInt64(const char* typeName);

   // Inline helper that does the casting job for an unsigned 64-bit value. The syntax of its usage is close to the standard dynamic_cast.
   // We do not use it directly in the source code due to compatibility problems with different compilers.
   //
   template
      <typename To>
   inline To MDo64Cast(Muint64 from)
   {
      To toValue = static_cast<To>(from); // possible loss of data
      if ( from != toValue ) // have we lost any bits ?
      {
         MDoThrowBadConversionUint64(M_MACRO_STRING(To));
         M_ENSURED_ASSERT(0);
      }
      return toValue;
   }

   // Inline helper that does the casting job for an signed 64-bit value. The syntax of its usage is close to the standard dynamic_cast.
   // We do not use it directly in the source code due to compatibility problems with different compilers.
   //
   template
      <typename To>
   inline To MDo64Cast(Mint64 from)
   {
      To toValue = static_cast<To>(from);
      if ( from != toValue )
      {
         MDoThrowBadConversionInt64(M_MACRO_STRING(To));
         M_ENSURED_ASSERT(0);
      }
      return toValue;
   }

/// \endcond SHOW_INTERNAL

   /// 32-bit OS only: Cast 64-bit integral type into 32-bit integral type with check whether the conversion lost any bits.
   /// This is typically used for size_t conversion.
   ///
   /// \param type is either int or unsigned, literal
   /// \param value is of type Muint64 or Mint64.
   ///              If value does not fit within 32-bits a conversion exception is thrown.
   /// \return value cast to given type
   ///
   #define M_64_CAST(type, value)   MDo64Cast<type>(value)
#else
   #define M_64_CAST(type, value)   (value) // do not perform any casting on non-64 bit platforms
#endif

///@}
#endif
