// File MCORE/MObjectMethods.inc

// If service parameters matter, we define individual macros,
// Otherwise just _M(st, method, call, par) is enough.
//

#define _M_NULL {(MVariant::Type)0xFF}

#ifdef _M
   #define _M_PROCEDURE0(st, type, call)          _M(st, type, call, _M_NULL, MVariant::VAR_EMPTY, 0)
   #define _M_PROCEDURE1(st, type, call, par)     _M(st, type, call, par, MVariant::VAR_EMPTY, 1)
   #define _M_PROCEDURE2(st, type, call, par)     _M(st, type, call, _B2 par, MVariant::VAR_EMPTY, 2)
   #define _M_PROCEDURE3(st, type, call, par)     _M(st, type, call, _B3 par, MVariant::VAR_EMPTY, 3)
   #define _M_PROCEDURE4(st, type, call, par)     _M(st, type, call, _B4 par, MVariant::VAR_EMPTY, 4)
   #define _M_PROCEDURE5(st, type, call, par)     _M(st, type, call, _B5 par, MVariant::VAR_EMPTY, 5)
   #define _M_PROCEDURE6(st, type, call, par)     _M(st, type, call, _B6 par, MVariant::VAR_EMPTY, 6)
   #define _M_FUNCTION0(st, type, call, par, ret) _M(st, type, call, par, ret, 0)
   #define _M_FUNCTION1(st, type, call, par, ret) _M(st, type, call, par, ret, 1)
   #define _M_FUNCTION2(st, type, call, par, ret) _M(st, type, call, _B2 par, ret, 2)
   #define _M_FUNCTION3(st, type, call, par, ret) _M(st, type, call, _B3 par, ret, 3)
   #define _M_FUNCTION4(st, type, call, par, ret) _M(st, type, call, _B4 par, ret, 4)
   #define _M_FUNCTION5(st, type, call, par, ret) _M(st, type, call, _B5 par, ret, 5)
   #define _M_FUNCTION6(st, type, call, par, ret) _M(st, type, call, _B6 par, ret, 6)
#else
   #ifndef _M_PROCEDURE0
      #define _M_PROCEDURE0(st, type, call)
   #endif
   #ifndef _M_PROCEDURE1
      #define _M_PROCEDURE1(st, type, call, par)
   #endif
   #ifndef _M_PROCEDURE2
      #define _M_PROCEDURE2(st, type, call, par)
   #endif
   #ifndef _M_PROCEDURE3
      #define _M_PROCEDURE3(st, type, call, par)
   #endif
   #ifndef _M_PROCEDURE4
      #define _M_PROCEDURE4(st, type, call, par)
   #endif
   #ifndef _M_PROCEDURE5
      #define _M_PROCEDURE5(st, type, call, par)
   #endif
   #ifndef _M_PROCEDURE6
      #define _M_PROCEDURE6(st, type, call, par)
   #endif
   #ifndef _M_FUNCTION0
      #define _M_FUNCTION0(st, type, call, par, ret)
   #endif
   #ifndef _M_FUNCTION1
      #define _M_FUNCTION1(st, type, call, par, ret)
   #endif
   #ifndef _M_FUNCTION2
      #define _M_FUNCTION2(st, type, call, par, ret)
   #endif
   #ifndef _M_FUNCTION3
      #define _M_FUNCTION3(st, type, call, par, ret)
   #endif
   #ifndef _M_FUNCTION4
      #define _M_FUNCTION4(st, type, call, par, ret)
   #endif
   #ifndef _M_FUNCTION5
      #define _M_FUNCTION5(st, type, call, par, ret)
   #endif
   #ifndef _M_FUNCTION6
      #define _M_FUNCTION6(st, type, call, par, ret)
   #endif
#endif
#if !M_NO_FULL_REFLECTION
   #define _B2(a, b)              {a, b}
   #define _B3(a, b, c)           {a, b, c}
   #define _B4(a, b, c, d)        {a, b, c, d}
   #define _B5(a, b, c, d, e)     {a, b, c, d, e}
   #define _B6(a, b, c, d, e, f)  {a, b, c, d, e, f}
#else
   #define _B2(a, b)               0
   #define _B3(a, b, c)            0
   #define _B4(a, b, c, d)         0
   #define _B5(a, b, c, d, e)      0
   #define _B6(a, b, c, d, e, f)   0
#endif

   // Zero-parameter procedures and functions

_M_PROCEDURE0(ST_X,
   void (MObject::*Method_X)(),
   (this->*((Method_X)def->m_objectMethod))())

_M_FUNCTION0(ST_bool_X,
   bool (MObject::*Method_bool_X)() const,
    result.DoAssignToEmpty((this->*((Method_bool_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_BOOL)

_M_FUNCTION0(ST_byte_X,
   Muint8 (MObject::*Method_byte_X)() const,
    result.DoAssignToEmpty((this->*((Method_byte_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_BYTE)

_M_FUNCTION0(ST_char_X,
   char (MObject::*Method_char_X)() const,
    result.DoAssignToEmpty((this->*((Method_char_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_CHAR)

_M_FUNCTION0(ST_int_X,
   int (MObject::*Method_int_X)() const,
   result.DoAssignToEmpty((this->*((Method_int_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_INT)

_M_FUNCTION0(ST_unsigned_X,
   unsigned (MObject::*Method_unsigned_X)() const,
   result.DoAssignToEmpty((this->*((Method_unsigned_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_UINT)

_M_FUNCTION0(ST_double_X,
   double (MObject::*Method_double_X)() const,
   result.DoAssignToEmpty((this->*((Method_double_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_DOUBLE)

_M_FUNCTION0(ST_constMVariantA_X,
   const MVariant& (MObject::*Method_constMVariantA_X)() const,
   result.DoAssignToEmpty((this->*((Method_constMVariantA_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_VARIANT)

_M_FUNCTION0(ST_MVariant_X,
   MVariant (MObject::*Method_MVariant_X)() const,
   result.DoAssignToEmpty((this->*((Method_MVariant_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_VARIANT)

_M_FUNCTION0(ST_constMByteStringA_X,
   const MByteString& (MObject::*Method_constMByteStringA_X)() const,
   result.DoAssignByteStringToEmpty((this->*((Method_constMByteStringA_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_BYTE_STRING)

_M_FUNCTION0(ST_MByteString_X,
   MByteString (MObject::*Method_MByteString_X)() const,
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_BYTE_STRING)

_M_FUNCTION0(ST_constMStdStringA_X,
   const MStdString& (MObject::*Method_constMStdStringA_X)() const,
   result.DoAssignToEmpty((this->*((Method_constMStdStringA_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_STRING)

_M_FUNCTION0(ST_MStdString_X,
   MStdString (MObject::*Method_MStdString_X)() const,
   result.DoAssignToEmpty((this->*((Method_MStdString_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_STRING)

_M_FUNCTION0(ST_MConstChars_X,
   MConstChars (MObject::*Method_MConstChars_X)() const,
   result.DoAssignToEmpty((this->*((Method_MConstChars_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_STRING)

_M_FUNCTION0(ST_constMStdStringVectorA_X,
   const MStdStringVector& (MObject::*Method_constMStdStringVectorA_X)() const,
   result.DoAssignToEmpty((this->*((Method_constMStdStringVectorA_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION0(ST_MStdStringVector_X,
   MStdStringVector (MObject::*Method_MStdStringVector_X)() const,
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION0(ST_constMByteStringVectorA_X,
   const MByteStringVector& (MObject::*Method_constMByteStringVectorA_X)() const,
   result.DoAssignByteStringCollectionToEmpty((this->*((Method_constMByteStringVectorA_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_VARIANT_COLLECTION)

_M_FUNCTION0(ST_MObjectP_X,
   MObject* (MObject::*Method_MObjectP_X)() const,
   result.DoAssignToEmpty((this->*((Method_MObjectP_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_OBJECT)

_M_FUNCTION0(ST_MObjectByValue_X,
   MVariant::ObjectByValue (MObject::*Method_MObjectByValue_X)() const,
   result.DoAssignToEmpty((this->*((Method_MObjectByValue_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_OBJECT)

_M_FUNCTION0(ST_MVariantVector_X,
   MVariant::VariantVector (MObject::*Method_MVariantVector_X)() const,
   result.DoAssignToEmpty((this->*((Method_MVariantVector_X)def->m_objectMethod))()),
   _M_NULL, MVariant::VAR_VARIANT_COLLECTION)


   // One-parameter procedures and functions

_M_PROCEDURE1(ST_X_bool,
   void (MObject::*Method_X_bool)(bool),
   (this->*((Method_X_bool)def->m_objectMethod))(p[0].AsBool()),
   {MVariant::VAR_BOOL})

_M_PROCEDURE1(ST_X_byte,
   void (MObject::*Method_X_byte)(Muint8),
   (this->*((Method_X_byte)def->m_objectMethod))(p[0].AsByte()),
   {MVariant::VAR_BYTE})

_M_PROCEDURE1(ST_X_char,
   void (MObject::*Method_X_char)(char),
   (this->*((Method_X_char)def->m_objectMethod))(p[0].AsChar()),
   {MVariant::VAR_CHAR})

_M_PROCEDURE1(ST_X_int,
   void (MObject::*Method_X_int)(int),
   (this->*((Method_X_int)def->m_objectMethod))(p[0].AsInt()),
   {MVariant::VAR_INT})

_M_PROCEDURE1(ST_X_unsigned,
   void (MObject::*Method_X_unsigned)(unsigned),
   (this->*((Method_X_unsigned)def->m_objectMethod))(p[0].AsUInt()),
   {MVariant::VAR_UINT})

_M_PROCEDURE1(ST_X_double,
   void (MObject::*Method_X_double)(double),
   (this->*((Method_X_double)def->m_objectMethod))(p[0].AsDouble()),
   {MVariant::VAR_DOUBLE})

_M_PROCEDURE1(ST_X_constMVariantA,
   void (MObject::*Method_X_constMVariantA)(const MVariant&),
   (this->*((Method_X_constMVariantA)def->m_objectMethod))(p[0]),
   {MVariant::VAR_VARIANT})

_M_PROCEDURE1(ST_X_constMByteStringA,
   void (MObject::*Method_X_constMByteStringA)(const MByteString&),
   (this->*((Method_X_constMByteStringA)def->m_objectMethod))(p[0].AsByteString()),
   {MVariant::VAR_BYTE_STRING})

_M_PROCEDURE1(ST_X_constMStdStringA,
   void (MObject::*Method_X_constMStdStringA)(const MStdString&),
   (this->*((Method_X_constMStdStringA)def->m_objectMethod))(p[0].AsString()),
   {MVariant::VAR_STRING})

_M_PROCEDURE1(ST_X_MConstChars,
   void (MObject::*Method_X_MConstChars)(MConstChars),
   (this->*((Method_X_MConstChars)def->m_objectMethod))(p[0].AsString().c_str()),
   {MVariant::VAR_STRING})

_M_PROCEDURE1(ST_X_constMStdStringVectorA,
   void (MObject::*Method_X_constMStdStringVectorA)(const MStdStringVector&),
   (this->*((Method_X_constMStdStringVectorA)def->m_objectMethod))(p[0].AsStringCollection()),
   {MVariant::VAR_STRING_COLLECTION})

_M_PROCEDURE1(ST_X_constMByteStringVectorA,
   void (MObject::*Method_X_constMByteStringVectorA)(const MByteStringVector&),
   (this->*((Method_X_constMByteStringVectorA)def->m_objectMethod))(p[0].AsByteStringCollection()),
   {MVariant::VAR_VARIANT_COLLECTION})

_M_PROCEDURE1(ST_X_MObjectP,
   void (MObject::*Method_X_MObjectP)(MObject*),
   (this->*((Method_X_MObjectP)def->m_objectMethod))(p[0].AsObject()),
   {MVariant::VAR_OBJECT})

_M_FUNCTION1(ST_MObjectByValue_X_int,
   MVariant::ObjectByValue (MObject::*Method_MObjectByValue_X_int)(int),
   result.DoAssignToEmpty((this->*((Method_MObjectByValue_X_int)def->m_objectMethod))(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_MObjectByValue_X_MObjectP,
   MVariant::ObjectByValue (MObject::*Method_MObjectByValue_X_MObjectP)(MObject*),
   result.DoAssignToEmpty((this->*((Method_MObjectByValue_X_MObjectP)def->m_objectMethod))(p[0].AsObject())),
   {MVariant::VAR_OBJECT}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_int_X_bool,
   int (MObject::*Method_int_X_bool)(bool),
   result.DoAssignToEmpty((this->*((Method_int_X_bool)def->m_objectMethod))(p[0].AsBool())),
   {MVariant::VAR_BOOL}, MVariant::VAR_INT)

_M_FUNCTION1(ST_int_X_MObjectP,
   int (MObject::*Method_int_X_MObjectP)(MObject*),
   result.DoAssignToEmpty((this->*((Method_int_X_MObjectP)def->m_objectMethod))(p[0].AsObject())),
   {MVariant::VAR_OBJECT}, MVariant::VAR_INT)

_M_FUNCTION1(ST_bool_X_MObjectP,
   bool (MObject::*Method_bool_X_MObjectP)(MObject*),
   result.DoAssignToEmpty((this->*((Method_bool_X_MObjectP)def->m_objectMethod))(p[0].AsObject())),
   {MVariant::VAR_OBJECT}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_bool_X_constMVariantA,
   bool (MObject::*Method_bool_X_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty((this->*((Method_bool_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_char_X_unsigned,
   char (MObject::*Method_char_X_unsigned)(unsigned),
   result.DoAssignToEmpty((this->*((Method_char_X_unsigned)def->m_objectMethod))(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_CHAR)

_M_FUNCTION1(ST_bool_X_unsigned,
   bool (MObject::*Method_bool_X_unsigned)(unsigned),
   result.DoAssignToEmpty((this->*((Method_bool_X_unsigned)def->m_objectMethod))(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_unsigned_X_constMByteStringA,
   unsigned (MObject::*Method_unsigned_X_constMByteStringA)(const MByteString&),
   result.DoAssignToEmpty((this->*((Method_unsigned_X_constMByteStringA)def->m_objectMethod))(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_UINT)

_M_FUNCTION1(ST_unsigned_X_constMStdStringA, // Metercat use
   unsigned (MObject::*Method_unsigned_X_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_unsigned_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_UINT)

_M_FUNCTION1(ST_int_X_constMVariantA,
   int (MObject::*Method_int_X_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty((this->*((Method_int_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_INT)

_M_FUNCTION1(ST_bool_X_constMStdStringA,
   bool (MObject::*Method_bool_X_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_bool_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_MObjectP_X_int,
   MObject* (MObject::*Method_MObjectP_X_int)(int),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_int)def->m_objectMethod))(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_MObjectP_X_double,
   MObject* (MObject::*Method_MObjectP_X_double)(double),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_double)def->m_objectMethod))(p[0].AsDouble())),
   {MVariant::VAR_DOUBLE}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_MVariant_X_MConstChars,
   MObject* (MObject::*Method_MVariant_X_MConstChars)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MVariant_X_MConstChars)def->m_objectMethod))(p[0].AsString().c_str())),
   {MVariant::VAR_STRING}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MObjectP_X_constMStdStringA,
   MObject* (MObject::*Method_MObjectP_X_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_MObjectP_X_constMVariantA,
   MObject* (MObject::*Method_MObjectP_X_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_int_X_constMStdStringA,
   int (MObject::*Method_int_X_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_int_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_INT)

_M_FUNCTION1(ST_int_X_int,
   int (MObject::*Method_int_X_int)(int),
   result.DoAssignToEmpty((this->*((Method_int_X_int)def->m_objectMethod))(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_INT)

_M_FUNCTION1(ST_unsigned_X_unsigned,
   unsigned (MObject::*Method_unsigned_X_unsigned)(unsigned),
   result.DoAssignToEmpty((this->*((Method_unsigned_X_unsigned)def->m_objectMethod))(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_UINT)

_M_FUNCTION1(ST_unsigned_X_int,
   unsigned (MObject::*Method_unsigned_X_int)(int),
   result.DoAssignToEmpty((this->*((Method_unsigned_X_int)def->m_objectMethod))(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_UINT)

_M_FUNCTION1(ST_MByteString_X_byte,
   MByteString (MObject::*Method_MByteString_X_byte)(Muint8),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_byte)def->m_objectMethod))(p[0].AsByte())),
   {MVariant::VAR_BYTE}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_constMByteStringA_X_unsigned,
   const MByteString& (MObject::*Method_constMByteStringA_X_unsigned)(unsigned),
   result.DoAssignByteStringToEmpty((this->*((Method_constMByteStringA_X_unsigned)def->m_objectMethod))(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MByteString_X_int,
   MByteString (MObject::*Method_MByteString_X_int)(int),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_int)def->m_objectMethod))(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MByteString_X_unsigned,
   MByteString (MObject::*Method_MByteString_X_unsigned)(unsigned),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_unsigned)def->m_objectMethod))(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MByteString_X_constMByteStringA,
   MByteString (MObject::*Method_MByteString_X_constMByteStringA)(const MByteString&),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMByteStringA)def->m_objectMethod))(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MByteString_X_constMVariantA,
   MByteString (MObject::*Method_MByteString_X_constMVariantA)(const MVariant&),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MByteString_X_constMStdStringA,
   MByteString (MObject::*Method_MByteString_X_constMStdStringA)(const MStdString&),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_unsigned_X_constMVariantA,
   unsigned (MObject::*Method_unsigned_X_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty((this->*((Method_unsigned_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_UINT)

_M_FUNCTION1(ST_MStdString_X_constMVariantA,
   MStdString (MObject::*Method_MStdString_X_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MVariant_X_constMVariantA,
   MVariant (MObject::*Method_MVariant_X_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MVariant_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MVariant_X_int,
   MVariant (MObject::*Method_MVariant_X_int)(int),
   result.DoAssignToEmpty((this->*((Method_MVariant_X_int)def->m_objectMethod))(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MStdString_X_int,
   MStdString (MObject::*Method_MStdString_X_int)(int),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_int)def->m_objectMethod))(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdString_X_MConstChars,
   MStdString (MObject::*Method_MStdString_X_MConstChars)(MConstChars),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_MConstChars)def->m_objectMethod))(p[0].AsString().c_str())),
   {MVariant::VAR_STRING}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdString_X_constMByteStringA,
   MStdString (MObject::*Method_MStdString_X_constMByteStringA)(const MByteString&),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_constMByteStringA)def->m_objectMethod))(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdString_X_constMStdStringA,
   MStdString (MObject::*Method_MStdString_X_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdString_X_MObjectP,
   MStdString (MObject::*Method_MStdString_X_MObjectP)(MObject*),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_MObjectP)def->m_objectMethod))(p[0].AsObject())),
   {MVariant::VAR_OBJECT}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MVariant_X_constMByteStringA,
   MVariant (MObject::*Method_Variant_X_constMByteStringA)(const MByteString&),
   result.DoAssignToEmpty((this->*((Method_Variant_X_constMByteStringA)def->m_objectMethod))(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MVariant_X_constMStdStringA,
   MVariant (MObject::*Method_Variant_X_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_Variant_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MStdStringVector_X_constMStdStringA,
   MStdStringVector (MObject::*Method_MStdStringVector_X_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION1(ST_MStdStringVector_X_constMVariantA,
   MStdStringVector (MObject::*Method_MStdStringVector_X_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION1(ST_MStdStringVector_X_bool,
   MStdStringVector (MObject::*Method_MStdStringVector_X_bool)(bool),
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X_bool)def->m_objectMethod))(p[0].AsBool())),
   {MVariant::VAR_BOOL}, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION1(ST_MStdStringVector_X_unsigned,
   MStdStringVector (MObject::*Method_MStdStringVector_X_unsigned)(unsigned),
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X_unsigned)def->m_objectMethod))(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION1(ST_MStdString_X_unsigned,
   MStdString (MObject::*Method_MStdString_X_unsigned)(unsigned),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_unsigned)def->m_objectMethod))(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdString_X_bool,
   MStdString (MObject::*Method_MStdString_X_bool)(bool),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_bool)def->m_objectMethod))(p[0].AsBool())),
   {MVariant::VAR_BOOL}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_double_X_constMStdStringA,
   double (MObject::*Method_double_X_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty((this->*((Method_double_X_constMStdStringA)def->m_objectMethod))(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_DOUBLE)

_M_FUNCTION1(ST_constMVariantA_X_constMVariantA,
   const MVariant& (MObject::*Method_constMVariantA_X_constMVariantA)(const MVariant&) const,
   result.DoAssignToEmpty((this->*((Method_constMVariantA_X_constMVariantA)def->m_objectMethod))(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_VARIANT)


   // Two-parameter procedures and functions

_M_PROCEDURE2(ST_X_char_constMByteStringA,
   void (MObject::*Method_X_char_constMByteStringA)(char, const MByteString&),
   (this->*((Method_X_char_constMByteStringA)def->m_objectMethod))(p[0].AsChar(), p[1].AsByteString()),
   (MVariant::VAR_CHAR, MVariant::VAR_BYTE_STRING))

_M_PROCEDURE2(ST_X_bool_int,
   void (MObject::*Method_X_bool_int)(bool, int),
   (this->*((Method_X_bool_int)def->m_objectMethod))(p[0].AsBool(), p[1].AsInt()),
   (MVariant::VAR_BOOL, MVariant::VAR_INT))

_M_PROCEDURE2(ST_X_int_int,
   void (MObject::*Method_X_int_int)(int, int),
   (this->*((Method_X_int_int)def->m_objectMethod))(p[0].AsInt(), p[1].AsInt()),
   (MVariant::VAR_INT, MVariant::VAR_INT))

_M_PROCEDURE2(ST_X_unsigned_char,
   void (MObject::*Method_X_unsigned_char)(unsigned, char),
   (this->*((Method_X_unsigned_char)def->m_objectMethod))(p[0].AsUInt(), p[1].AsChar()),
   (MVariant::VAR_UINT, MVariant::VAR_CHAR))

_M_PROCEDURE2(ST_X_unsigned_unsigned,
   void (MObject::*Method_X_unsigned_unsigned)(unsigned, unsigned),
   (this->*((Method_X_unsigned_unsigned)def->m_objectMethod))(p[0].AsUInt(), p[1].AsUInt()),
   (MVariant::VAR_UINT, MVariant::VAR_UINT))

_M_PROCEDURE2(ST_X_unsigned_constMStdStringA,
   void (MObject::*Method_X_unsigned_constMStdStringA)(unsigned, const MStdString&),
   (this->*((Method_X_unsigned_constMStdStringA)def->m_objectMethod))(p[0].AsUInt(), p[1].AsString()),
   (MVariant::VAR_UINT, MVariant::VAR_STRING))

_M_PROCEDURE2(ST_X_constMByteStringA_bool,
   void (MObject::*Method_X_constMByteStringA_bool)(const MByteString&, bool),
   (this->*((Method_X_constMByteStringA_bool)def->m_objectMethod))(p[0].AsByteString(), p[1].AsBool()),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BOOL))

_M_PROCEDURE2(ST_X_constMStdStringA_bool,
   void (MObject::*Method_X_constMStdStringA_bool)(const MStdString&, bool),
   (this->*((Method_X_constMStdStringA_bool)def->m_objectMethod))(p[0].AsString(), p[1].AsBool()),
   (MVariant::VAR_STRING, MVariant::VAR_BOOL))

_M_PROCEDURE2(ST_X_constMStdStringA_unsigned,
   void (MObject::*Method_X_constMStdStringA_unsigned)(const MStdString&, unsigned),
   (this->*((Method_X_constMStdStringA_unsigned)def->m_objectMethod))(p[0].AsString(), p[1].AsUInt()),
   (MVariant::VAR_STRING, MVariant::VAR_UINT))

_M_PROCEDURE2(ST_X_constMByteStringA_int,
   void (MObject::*Method_X_constMByteStringA_int)(const MByteString&, int),
   (this->*((Method_X_constMByteStringA_int)def->m_objectMethod))(p[0].AsByteString(), p[1].AsInt()),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_INT))

_M_PROCEDURE2(ST_X_unsigned_constMByteStringA,
   void (MObject::*Method_X_unsigned_constMByteStringA)(unsigned, const MByteString&),
   (this->*((Method_X_unsigned_constMByteStringA)def->m_objectMethod))(p[0].AsUInt(), p[1].AsByteString()),
   (MVariant::VAR_UINT, MVariant::VAR_BYTE_STRING))

_M_PROCEDURE2(ST_X_constMVariantA_int,
   void (MObject::*Method_X_constMVariantA_int)(const MVariant&, int),
   (this->*((Method_X_constMVariantA_int)def->m_objectMethod))(p[0], p[1].AsInt()),
   (MVariant::VAR_VARIANT, MVariant::VAR_INT))

_M_PROCEDURE2(ST_X_constMVariantA_constMByteStringA,
   void (MObject::*Method_X_constMVariantA_constMByteStringA)(const MVariant&, const MByteString&),
   (this->*((Method_X_constMVariantA_constMByteStringA)def->m_objectMethod))(p[0], p[1].AsByteString()),
   (MVariant::VAR_VARIANT, MVariant::VAR_BYTE_STRING))

_M_PROCEDURE2(ST_X_MObjectP_constMStdStringA,
   void (MObject::*Method_X_MObjectP_constMStdStringA)(MObject*, const MStdString&),
   (this->*((Method_X_MObjectP_constMStdStringA)def->m_objectMethod))(p[0].AsObject(), p[1].AsString()),
   (MVariant::VAR_OBJECT, MVariant::VAR_STRING))

_M_PROCEDURE2(ST_X_MObjectP_constMVariantA,
   void (MObject::*Method_X_MObjectP_constMVariantA)(MObject*, const MVariant&),
   (this->*((Method_X_MObjectP_constMVariantA)def->m_objectMethod))(p[0].AsObject(), p[1]),
   (MVariant::VAR_OBJECT, MVariant::VAR_VARIANT))

_M_PROCEDURE2(ST_X_constMStdStringA_constMVariantA,
   void (MObject::*Method_X_constMStdStringA_constMVariantA)(const MStdString&, const MVariant&),
   (this->*((Method_X_constMStdStringA_constMVariantA)def->m_objectMethod))(p[0].AsString(), p[1]),
   (MVariant::VAR_STRING, MVariant::VAR_VARIANT))

_M_PROCEDURE2(ST_X_constMStdStringA_constMByteStringA,
   void (MObject::*Method_X_constMStdStringA_constMByteStringA)(const MStdString&, const MByteString&),
   (this->*((Method_X_constMStdStringA_constMByteStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsByteString()),
   (MVariant::VAR_STRING, MVariant::VAR_BYTE_STRING))

_M_PROCEDURE2(ST_X_constMByteStringA_unsigned,
   void (MObject::*Method_X_constMByteStringA_unsigned)(const MByteString&, unsigned),
   (this->*((Method_X_constMByteStringA_unsigned)def->m_objectMethod))(p[0].AsByteString(), p[1].AsUInt()),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_UINT))

_M_PROCEDURE2(ST_X_constMStdStringA_MObjectP,
   void (MObject::*Method_X_constMStdStringA_MObjectP)(const MStdString&, MObject*),
   (this->*((Method_X_constMStdStringA_MObjectP)def->m_objectMethod))(p[0].AsString(), p[1].AsObject()),
   (MVariant::VAR_STRING, MVariant::VAR_OBJECT))

_M_PROCEDURE2(ST_X_constMStdStringA_int,
   void (MObject::*Method_X_constMStdStringA_int)(const MStdString&, int),
   (this->*((Method_X_constMStdStringA_int)def->m_objectMethod))(p[0].AsString(), p[1].AsInt()),
   (MVariant::VAR_STRING, MVariant::VAR_INT))

_M_PROCEDURE2(ST_X_constMStdStringA_constMStdStringA,
   void (MObject::*Method_X_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   (this->*((Method_X_constMStdStringA_constMStdStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsString()),
   (MVariant::VAR_STRING, MVariant::VAR_STRING))

_M_PROCEDURE2(ST_X_double_constMStdStringA,
   void (MObject::*Method_X_double_constMStdStringA)(double, const MStdString&),
   (this->*((Method_X_double_constMStdStringA)def->m_objectMethod))(p[0].AsDouble(), p[1].AsString()),
   (MVariant::VAR_DOUBLE, MVariant::VAR_STRING))

_M_PROCEDURE2(ST_X_constMVariantA_MObjectP,
   void (MObject::*Method_X_constMVariantA_MObjectP)(const MVariant&, MObject*),
   (this->*((Method_X_constMVariantA_MObjectP)def->m_objectMethod))(p[0], p[1].AsObject()),
   (MVariant::VAR_VARIANT, MVariant::VAR_OBJECT))

_M_PROCEDURE2(ST_X_constMVariantA_constMVariantA,
   void (MObject::*Method_constMVariantA_constMVariantA)(const MVariant&, const MVariant&),
   (this->*((Method_constMVariantA_constMVariantA)def->m_objectMethod))(p[0], p[1]),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT))

_M_PROCEDURE2(ST_X_MObjectP_constMStdStringVectorA,
   void (MObject::*Method_X_MObjectP_constMStdStringVectorA)(MObject*, const MStdStringVector&),
   (this->*((Method_X_MObjectP_constMStdStringVectorA)def->m_objectMethod))(p[0].AsObject(), p[1].AsStringCollection()),
   (MVariant::VAR_OBJECT, MVariant::VAR_STRING_COLLECTION))

_M_PROCEDURE2(ST_X_constMByteStringA_constMByteStringA,
   void (MObject::*Method_X_constMByteStringA_constMByteStringA)(const MByteString&, const MByteString&),
   (this->*((Method_X_constMByteStringA_constMByteStringA)def->m_objectMethod))(p[0].AsByteString(), p[1].AsByteString()),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE_STRING))

_M_FUNCTION2(ST_MStdString_X_bool_bool,
   MStdString (MObject::*Method_MStdString_X_bool_bool)(bool, bool),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_bool_bool)def->m_objectMethod))(p[0].AsBool(), p[1].AsBool())),
   (MVariant::VAR_BOOL, MVariant::VAR_BOOL), MVariant::VAR_STRING)

_M_FUNCTION2(ST_unsigned_X_bool_unsigned,
   unsigned (MObject::*Method_unsigned_X_bool_unsigned)(bool, unsigned),
   result.DoAssignToEmpty((this->*((Method_unsigned_X_bool_unsigned)def->m_objectMethod))(p[0].AsBool(), p[1].AsUInt())),
   (MVariant::VAR_BOOL, MVariant::VAR_UINT), MVariant::VAR_UINT)

_M_FUNCTION2(ST_unsigned_X_int_bool,
   unsigned (MObject::*Method_unsigned_X_int_bool)(int, bool),
   result.DoAssignToEmpty((this->*((Method_unsigned_X_int_bool)def->m_objectMethod))(p[0].AsInt(), p[1].AsBool())),
   (MVariant::VAR_INT, MVariant::VAR_BOOL), MVariant::VAR_UINT)

_M_FUNCTION2(ST_int_X_unsigned_unsigned,
   int (MObject::*Method_int_X_unsigned_unsigned)(unsigned, unsigned),
   result.DoAssignToEmpty((this->*((Method_int_X_unsigned_unsigned)def->m_objectMethod))(p[0].AsUInt(), p[1].AsUInt())),
   (MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_INT)

_M_FUNCTION2(ST_unsigned_X_unsigned_unsigned,
   unsigned (MObject::*Method_unsigned_X_unsigned_unsigned)(unsigned, unsigned),
   result.DoAssignToEmpty((this->*((Method_unsigned_X_unsigned_unsigned)def->m_objectMethod))(p[0].AsUInt(), p[1].AsUInt())),
   (MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_UINT)

_M_FUNCTION2(ST_int_X_int_int,
   int (MObject::*Method_int_X_int_int)(int, int),
   result.DoAssignToEmpty((this->*((Method_int_X_int_int)def->m_objectMethod))(p[0].AsInt(), p[1].AsInt())),
   (MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_INT)

_M_FUNCTION2(ST_bool_X_unsigned_constMStdStringA,
   bool (MObject::*Method_bool_X_unsigned_constMStdStringA)(unsigned, const MStdString&),
   result.DoAssignToEmpty((this->*((Method_bool_X_unsigned_constMStdStringA)def->m_objectMethod))(p[0].AsUInt(), p[1].AsString())),
   (MVariant::VAR_UINT, MVariant::VAR_STRING), MVariant::VAR_BOOL)

_M_FUNCTION2(ST_bool_X_MObjectP_bool,
   bool (MObject::*Method_bool_X_MObjectP_bool)(MObject*, bool),
   result.DoAssignToEmpty((this->*((Method_bool_X_MObjectP_bool)def->m_objectMethod))(p[0].AsObject(), p[1].AsBool())),
   (MVariant::VAR_OBJECT, MVariant::VAR_BOOL), MVariant::VAR_BOOL)

_M_FUNCTION2(ST_bool_X_MObjectP_unsigned,
   bool (MObject::*Method_bool_X_MObjectP_unsigned)(MObject*, unsigned),
   result.DoAssignToEmpty((this->*((Method_bool_X_MObjectP_unsigned)def->m_objectMethod))(p[0].AsObject(), p[1].AsUInt())),
   (MVariant::VAR_OBJECT, MVariant::VAR_UINT), MVariant::VAR_BOOL)

_M_FUNCTION2(ST_bool_X_constMStdStringA_constMVariantA,
   bool (MObject::*Method_bool_X_constMStdStringA_constMVariantA)(const MStdString&, const MVariant&),
   result.DoAssignToEmpty((this->*((Method_bool_X_constMStdStringA_constMVariantA)def->m_objectMethod))(p[0].AsString(), p[1])),
   (MVariant::VAR_STRING, MVariant::VAR_VARIANT), MVariant::VAR_BOOL)

_M_FUNCTION2(ST_int_X_constMVariantA_int,
   int (MObject::*Method_int_X_constMVariantA_int)(const MVariant&, int),
   result.DoAssignToEmpty((this->*((Method_int_X_constMVariantA_int)def->m_objectMethod))(p[0], p[1].AsInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_INT), MVariant::VAR_INT)

_M_FUNCTION2(ST_int_X_constMStdStringA_int,
   int (MObject::*Method_int_X_constMStdStringA_int)(const MStdString&, int),
   result.DoAssignToEmpty((this->*((Method_int_X_constMStdStringA_int)def->m_objectMethod))(p[0].AsString(), p[1].AsInt())),
   (MVariant::VAR_STRING, MVariant::VAR_INT), MVariant::VAR_INT)

_M_FUNCTION2(ST_int_X_MObjectP_bool,
   int (MObject::*Method_int_X_MObjectP_bool)(MObject*, bool),
   result.DoAssignToEmpty((this->*((Method_int_X_MObjectP_bool)def->m_objectMethod))(p[0].AsObject(), p[1].AsBool())),
   (MVariant::VAR_OBJECT, MVariant::VAR_BOOL), MVariant::VAR_INT)

_M_FUNCTION2(ST_int_X_MObjectP_unsigned,
   int (MObject::*Method_int_X_MObjectP_unsigned)(MObject*, unsigned),
   result.DoAssignToEmpty((this->*((Method_int_X_MObjectP_unsigned)def->m_objectMethod))(p[0].AsObject(), p[1].AsUInt())),
   (MVariant::VAR_OBJECT, MVariant::VAR_UINT), MVariant::VAR_INT)

_M_FUNCTION2(ST_int_X_double_double,
   int (MObject::*Method_int_X_double_double)(double, double),
   (this->*((Method_int_X_double_double)def->m_objectMethod))(p[0].AsDouble(), p[1].AsDouble()),
   (MVariant::VAR_DOUBLE, MVariant::VAR_DOUBLE), MVariant::VAR_INT)

_M_FUNCTION2(ST_byte_X_constMByteStringA_unsigned,
   Muint8 (MObject::*Method_byte_X_constMByteStringA_unsigned)(const MByteString&, unsigned),
   result.DoAssignToEmpty((this->*((Method_byte_X_constMByteStringA_unsigned)def->m_objectMethod))(p[0].AsByteString(), p[1].AsUInt())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_UINT), MVariant::VAR_BYTE)

_M_FUNCTION2(ST_MByteString_X_unsigned_unsigned,
   MByteString (MObject::*Method_MByteString_X_unsigned_unsigned)(unsigned, unsigned),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_unsigned_unsigned)def->m_objectMethod))(p[0].AsUInt(), p[1].AsUInt())),
   (MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MByteString_X_char_constMByteStringA,
   MByteString (MObject::*Method_MByteString_X_char_constMByteStringA)(char, const MByteString&),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_char_constMByteStringA)def->m_objectMethod))(p[0].AsChar(), p[1].AsByteString())),
   (MVariant::VAR_CHAR, MVariant::VAR_BYTE_STRING), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MByteString_X_constMStdStringA_int,
   MByteString (MObject::*Method_MByteString_X_constMStdStringA_int)(const MStdString&, int),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMStdStringA_int)def->m_objectMethod))(p[0].AsString(), p[1].AsInt())),
   (MVariant::VAR_STRING, MVariant::VAR_INT), MVariant::VAR_BYTE_STRING)
   
_M_FUNCTION2(ST_MByteString_X_constMVariantA_constMByteStringA,
   MByteString (MObject::*Method_MByteString_X_constMVariantA_constMByteStringA)(const MVariant&, const MByteString&),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMVariantA_constMByteStringA)def->m_objectMethod))(p[0], p[1].AsByteString())),
   (MVariant::VAR_VARIANT, MVariant::VAR_BYTE_STRING), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MByteString_X_constMVariantA_int,
   MByteString (MObject::*Method_MByteString_X_constMVariantA_int)(const MVariant&, int),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMVariantA_int)def->m_objectMethod))(p[0], p[1].AsInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_INT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MByteString_X_constMVariantA_unsigned,
   MByteString (MObject::*Method_MByteString_X_constMVariantA_unsigned)(const MVariant&, unsigned),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMVariantA_unsigned)def->m_objectMethod))(p[0], p[1].AsUInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_UINT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MStdString_X_unsigned_constMStdStringA,
   MStdString (MObject::*Method_MStdString_X_unsigned_constMStdStringA)(unsigned, const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_unsigned_constMStdStringA)def->m_objectMethod))(p[0].AsUInt(), p[1].AsString())),
   (MVariant::VAR_UINT, MVariant::VAR_STRING), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MStdString_X_constMStdStringA_constMByteStringA,
   MStdString (MObject::*Method_MStdString_X_constMStdStringA_constMByteStringA)(const MStdString&, const MByteString&),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_constMStdStringA_constMByteStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsByteString())),
   (MVariant::VAR_STRING, MVariant::VAR_BYTE_STRING), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MStdString_X_constMStdStringA_constMStdStringA,
   MStdString (MObject::*Method_MStdString_X_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_constMStdStringA_constMStdStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MObjectP_X_constMStdStringA_constMStdStringA,
   MObject* (MObject::*Method_MObjectP_X_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMStdStringA_constMStdStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MByteString_X_constMByteStringA_constMByteStringA,
   MByteString (MObject::*Method_MByteString_X_constMByteStringA_constMByteStringA)(const MByteString&, const MByteString&),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMByteStringA_constMByteStringA)def->m_objectMethod))(p[0].AsByteString(), p[1].AsByteString())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE_STRING), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MByteString_X_constMStdStringA_constMByteStringA,
   MByteString (MObject::*Method_MByteString_X_constMStdStringA_constMByteStringA)(const MStdString&, const MByteString&),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMStdStringA_constMByteStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsByteString())),
   (MVariant::VAR_STRING, MVariant::VAR_BYTE_STRING), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MStdStringVector_X_constMStdStringA_bool,
   MStdStringVector (MObject::*Method_MStdStringVector_X_constMStdStringA_bool)(const MStdString&, bool),
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X_constMStdStringA_bool)def->m_objectMethod))(p[0].AsString(), p[1].AsBool())),
   (MVariant::VAR_STRING, MVariant::VAR_BOOL), MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION2(ST_MStdStringVector_X_constMStdStringA_constMStdStringA,
   MStdStringVector (MObject::*Method_MStdStringVector_X_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X_constMStdStringA_constMStdStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION2(ST_MStdStringVector_X_MObjectP_constMStdStringA,
   MStdStringVector (MObject::*Method_MStdStringVector_X_MObjectP_constMStdStringA)(MObject*, const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X_MObjectP_constMStdStringA)def->m_objectMethod))(p[0].AsObject(), p[1].AsString())),
   (MVariant::VAR_OBJECT, MVariant::VAR_STRING), MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION2(ST_MObjectP_X_constMStdStringA_bool,
   MObject* (MObject::*Method_MObjectP_X_constMStdStringA_bool)(const MStdString&, bool),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMStdStringA_bool)def->m_objectMethod))(p[0].AsString(), p[1].AsBool())),
   (MVariant::VAR_STRING, MVariant::VAR_BOOL), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MObjectP_X_constMStdStringA_constMVariantA,
   MObject* (MObject::*Method_MObjectP_X_constMStdStringA_constMVariantA)(const MStdString&, const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMStdStringA_constMVariantA)def->m_objectMethod))(p[0].AsString(), p[1])),
   (MVariant::VAR_STRING, MVariant::VAR_VARIANT), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MObjectP_X_constMVariantA_constMStdStringA,
   MObject* (MObject::*Method_MObjectP_X_constMVariantA_constMStdStringA)(const MVariant&, const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMVariantA_constMStdStringA)def->m_objectMethod))(p[0], p[1].AsString())),
   (MVariant::VAR_VARIANT, MVariant::VAR_STRING), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MObjectP_X_constMVariantA_constMVariantA,
   MObject* (MObject::*Method_MObjectP_X_constMVariantA_constMVariantA)(const MVariant&, const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMVariantA_constMVariantA)def->m_objectMethod))(p[0], p[1])),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MObjectByValue_X_MObjectP_bool,
   MVariant::ObjectByValue (MObject::*Method_MObjectByValue_X_MObjectP_bool)(MObject*, bool),
   result.DoAssignToEmpty((this->*((Method_MObjectByValue_X_MObjectP_bool)def->m_objectMethod))(p[0].AsObject(), p[1].AsBool())),
   (MVariant::VAR_OBJECT, MVariant::VAR_BOOL), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MStdString_X_unsigned_unsigned,
   MStdString (MObject::*Method_MStdString_X_unsigned_unsigned)(unsigned, unsigned),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_unsigned_unsigned)def->m_objectMethod))(p[0].AsUInt(), p[1].AsUInt())),
   (MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MVariant_X_constMStdStringA_constMVariantA,
   MVariant (MObject::*Method_MVariant_X_constMStdStringA_constMVariantA)(const MStdString&, const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MVariant_X_constMStdStringA_constMVariantA)def->m_objectMethod))(p[0].AsString(), p[1])),
   (MVariant::VAR_STRING, MVariant::VAR_VARIANT), MVariant::VAR_VARIANT)

   // Three-parameter procedures and functions

_M_PROCEDURE3(ST_X_int_constMVariantA_constMVariantA,
   void (MObject::*Method_X_int_constMVariantA_constMVariantA)(int, const MVariant&, const MVariant&),
   (this->*((Method_X_int_constMVariantA_constMVariantA)def->m_objectMethod))(p[0].AsInt(), p[1], p[2]),
   (MVariant::VAR_INT, MVariant::VAR_VARIANT, MVariant::VAR_VARIANT))

_M_PROCEDURE3(ST_X_constMVariantA_constMByteStringA_int,
   void (MObject::*Method_X_constMVariantA_constMByteStringA_int)(const MVariant&, const MByteString&, int),
   (this->*((Method_X_constMVariantA_constMByteStringA_int)def->m_objectMethod))(p[0], p[1].AsByteString(), p[2].AsInt()),
   (MVariant::VAR_VARIANT, MVariant::VAR_BYTE_STRING, MVariant::VAR_INT))

_M_PROCEDURE3(ST_X_constMVariantA_unsigned_int,
   void (MObject::*Method_X_constMVariantA_unsigned_int)(const MVariant&, unsigned, int),
   (this->*((Method_X_constMVariantA_unsigned_int)def->m_objectMethod))(p[0], p[1].AsUInt(), p[2].AsInt()),
   (MVariant::VAR_VARIANT, MVariant::VAR_UINT, MVariant::VAR_INT))

_M_PROCEDURE3(ST_X_constMStdStringA_unsigned_bool,
   void (MObject::*Method_X_constMStdStringA_unsigned_bool)(const MStdString&, unsigned, bool),
   (this->*((Method_X_constMStdStringA_unsigned_bool)def->m_objectMethod))(p[0].AsString(), p[1].AsUInt(), p[2].AsBool()),
   (MVariant::VAR_STRING, MVariant::VAR_UINT, MVariant::VAR_BOOL))

_M_PROCEDURE3(ST_X_constMStdStringA_unsigned_unsigned,
   void (MObject::*Method_X_constMStdStringA_unsigned_unsigned)(const MStdString&, unsigned, unsigned),
   (this->*((Method_X_constMStdStringA_unsigned_unsigned)def->m_objectMethod))(p[0].AsString(), p[1].AsUInt(), p[2].AsUInt()),
   (MVariant::VAR_STRING, MVariant::VAR_UINT, MVariant::VAR_UINT))

_M_PROCEDURE3(ST_X_constMStdStringA_unsigned_constMStdStringA,
   void (MObject::*Method_X_constMStdStringA_unsigned_constMStdStringA)(const MStdString&, int, const MStdString&),
   (this->*((Method_X_constMStdStringA_unsigned_constMStdStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsUInt(), p[2].AsString()),
   (MVariant::VAR_STRING, MVariant::VAR_UINT, MVariant::VAR_STRING))

_M_PROCEDURE3(ST_X_constMStdStringA_constMStdStringA_constMByteStringA,
   void (MObject::*Method_X_constMStdStringA_constMStdStringA_constMByteStringA)(const MStdString&, const MStdString&, const MByteString&),
   (this->*((Method_X_constMStdStringA_constMStdStringA_constMByteStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsString(), p[2].AsByteString()),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_BYTE_STRING))

_M_PROCEDURE3(ST_X_unsigned_constMStdStringA_constMStdStringA,
   void (MObject::*Method_X_unsigned_constMStdStringA_constMStdStringA)(unsigned, const MStdString&, const MStdString&),
   (this->*((Method_X_unsigned_constMStdStringA_constMStdStringA)def->m_objectMethod))(p[0].AsUInt(), p[1].AsString(), p[2].AsString()),
   (MVariant::VAR_UINT, MVariant::VAR_STRING, MVariant::VAR_STRING))

_M_PROCEDURE3(ST_X_constMStdStringA_constMStdStringA_unsigned,
   void (MObject::*Method_X_constMStdStringA_constMStdStringA_unsigned)(const MStdString&, const MStdString&, unsigned),
   (this->*((Method_X_constMStdStringA_constMStdStringA_unsigned)def->m_objectMethod))(p[0].AsString(), p[1].AsString(), p[2].AsUInt()),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_UINT))

_M_PROCEDURE3(ST_X_int_int_int,
   void (MObject::*Method_X_int_int_int)(int, int, int),
   (this->*((Method_X_int_int_int)def->m_objectMethod))(p[0].AsInt(), p[1].AsInt(), p[2].AsInt()),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT))

_M_PROCEDURE3(ST_X_int_unsigned_unsigned,
   void (MObject::*Method_X_int_unsigned_unsigned)(int, unsigned, unsigned),
   (this->*((Method_X_int_unsigned_unsigned)def->m_objectMethod))(p[0].AsInt(), p[1].AsUInt(), p[2].AsUInt()),
   (MVariant::VAR_INT, MVariant::VAR_UINT, MVariant::VAR_UINT))

_M_PROCEDURE3(ST_X_int_int_constMVariantA,
   void (MObject::*Method_X_int_int_constMVariantA)(int, int, const MVariant&),
   (this->*((Method_X_int_int_constMVariantA)def->m_objectMethod))(p[0].AsInt(), p[1].AsInt(), p[2]),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_VARIANT))

_M_PROCEDURE3(ST_X_double_double_int,
   void (MObject::*Method_X_double_double_int)(double, double, int),
   (this->*((Method_X_double_double_int)def->m_objectMethod))(p[0].AsDouble(), p[1].AsDouble(), p[2].AsInt()),
   (MVariant::VAR_DOUBLE, MVariant::VAR_DOUBLE, MVariant::VAR_INT))

_M_PROCEDURE3(ST_X_MObjectP_unsigned_unsigned,
   void (MObject::*Method_X_MObjectP_unsigned_unsigned)(MObject*, unsigned, unsigned),
   (this->*((Method_X_MObjectP_unsigned_unsigned)def->m_objectMethod))(p[0].AsObject(), p[1].AsUInt(), p[2].AsUInt()),
   (MVariant::VAR_OBJECT, MVariant::VAR_UINT, MVariant::VAR_UINT))

_M_PROCEDURE3(ST_X_MObjectP_constMStdStringA_bool,
   void (MObject::*Method_X_MObjectP_constMStdStringA_bool)(MObject*, const MStdString&, bool),
   (this->*((Method_X_MObjectP_constMStdStringA_bool)def->m_objectMethod))(p[0].AsObject(), p[1].AsString(), p[2].AsBool()),
   (MVariant::VAR_OBJECT, MVariant::VAR_STRING, MVariant::VAR_BOOL))

_M_PROCEDURE3(ST_X_MObjectP_constMVariantA_constMStdStringVectorA,
   void (MObject::*Method_X_MObjectP_constMVariantA_constMStdStringVectorA)(MObject*, const MVariant&, const MStdStringVector&),
   (this->*((Method_X_MObjectP_constMVariantA_constMStdStringVectorA)def->m_objectMethod))(p[0].AsObject(), p[1], p[2].AsStringCollection()),
   (MVariant::VAR_OBJECT, MVariant::VAR_VARIANT, MVariant::VAR_STRING_COLLECTION))

_M_PROCEDURE3(ST_X_byte_constMByteStringA_bool,
   void (MObject::*Method_X_byte_constMByteStringA_bool)(Muint8, const MByteString&, bool),
   (this->*((Method_X_byte_constMByteStringA_bool)def->m_objectMethod))(p[0].AsByte(), p[1].AsByteString(), p[2].AsBool()),
   (MVariant::VAR_BYTE, MVariant::VAR_BYTE_STRING, MVariant::VAR_BOOL))

_M_PROCEDURE3(ST_X_int_constMStdStringA_bool,
   void (MObject::*Method_X_int_constMStdStringA_bool)(int, const MStdString&, bool),
   (this->*((Method_X_int_constMStdStringA_bool)def->m_objectMethod))(p[0].AsInt(), p[1].AsString(), p[2].AsBool()),
   (MVariant::VAR_INT, MVariant::VAR_STRING, MVariant::VAR_BOOL))

_M_PROCEDURE3(ST_X_constMVariantA_int_unsigned,
   void (MObject::*Method_X_constMVariantA_int_unsigned)(const MVariant&, int, unsigned),
   (this->*((Method_X_constMVariantA_int_unsigned)def->m_objectMethod))(p[0], p[1].AsInt(), p[2].AsUInt()),
   (MVariant::VAR_VARIANT, MVariant::VAR_INT, MVariant::VAR_UINT))

_M_FUNCTION3(ST_bool_X_bool_bool_bool,
    bool (MObject::*Method_bool_X_bool_bool_bool)(bool, bool, bool),
    result.DoAssignToEmpty((this->*((Method_bool_X_bool_bool_bool)def->m_objectMethod))(p[0].AsBool(), p[1].AsBool(), p[2].AsBool())),
    (MVariant::VAR_BOOL, MVariant::VAR_BOOL, MVariant::VAR_BOOL), MVariant::VAR_BOOL)

_M_FUNCTION3(ST_MObjectP_X_bool_bool_bool,
   MObject* (MObject::*Method_MObjectP_X_bool_bool_bool)(bool, bool, bool),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_bool_bool_bool)def->m_objectMethod))(p[0].AsBool(), p[1].AsBool(), p[2].AsBool())),
   (MVariant::VAR_BOOL, MVariant::VAR_BOOL, MVariant::VAR_BOOL), MVariant::VAR_OBJECT)

_M_FUNCTION3(ST_MByteString_X_char_unsigned_unsigned,
   MByteString (MObject::*Method_MByteString_X_char_unsigned_unsigned)(char, unsigned, unsigned),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_char_unsigned_unsigned)def->m_objectMethod))(p[0].AsChar(), p[1].AsUInt(), p[2].AsUInt())),
   (MVariant::VAR_CHAR, MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION3(ST_MByteString_X_constMVariantA_int_int,
   MByteString (MObject::*Method_MByteString_X_constMVariantA_int_int)(const MVariant&, int, int),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMVariantA_int_int)def->m_objectMethod))(p[0], p[1].AsInt(), p[2].AsInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION3(ST_MByteString_X_int_int_unsigned,
   MByteString (MObject::*Method_MByteString_X_int_int_unsigned)(int, int, unsigned),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_int_int_unsigned)def->m_objectMethod))(p[0].AsInt(), p[1].AsInt(), p[2].AsUInt())),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_UINT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION3(ST_MStdString_X_bool_bool_bool,
   MStdString (MObject::*Method_MStdString_X_bool_bool_bool)(bool, bool, bool),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_bool_bool_bool)def->m_objectMethod))(p[0].AsBool(), p[1].AsBool(), p[2].AsBool())),
   (MVariant::VAR_BOOL, MVariant::VAR_BOOL, MVariant::VAR_BOOL), MVariant::VAR_STRING)

_M_FUNCTION3(ST_MStdString_X_unsigned_unsigned_unsigned,
   MStdString (MObject::*Method_MStdString_X_unsigned_unsigned_unsigned)(unsigned, unsigned, unsigned),
   result.DoAssignToEmpty((this->*((Method_MStdString_X_unsigned_unsigned_unsigned)def->m_objectMethod))(p[0].AsUInt(), p[1].AsUInt(), p[2].AsUInt())),
   (MVariant::VAR_UINT, MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_STRING)

_M_FUNCTION3(ST_MStdStringVector_X_bool_bool_bool,
   MStdStringVector (MObject::*Method_MStdStringVector_X_bool_bool_bool)(bool, bool, bool),
   result.DoAssignToEmpty((this->*((Method_MStdStringVector_X_bool_bool_bool)def->m_objectMethod))(p[0].AsBool(), p[1].AsBool(), p[2].AsBool())),
   (MVariant::VAR_BOOL, MVariant::VAR_BOOL, MVariant::VAR_BOOL), MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION3(ST_MObjectP_X_constMStdStringA_constMStdStringA_unsigned,
   MObject* (MObject::*Method_MObjectP_X_constMStdStringA_constMStdStringA_unsigned)(const MStdString&, const MStdString&, unsigned),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMStdStringA_constMStdStringA_unsigned)def->m_objectMethod))(p[0].AsString(), p[1].AsString(), p[2].AsUInt())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_UINT), MVariant::VAR_OBJECT)

_M_FUNCTION3(ST_MVariant_X_constMStdStringA_constMVariantA_constMVariantA,
   MVariant (MObject::*Method_MVariant_X_constMStdStringA_constMVariantA_constMVariantA)(const MStdString&, const MVariant&, const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MVariant_X_constMStdStringA_constMVariantA_constMVariantA)def->m_objectMethod))(p[0].AsString(), p[1], p[2])),
   (MVariant::VAR_STRING, MVariant::VAR_VARIANT, MVariant::VAR_VARIANT), MVariant::VAR_VARIANT)

_M_FUNCTION3(ST_MObjectP_X_constMStdStringA_constMStdStringA_constMStdStringA,
   MObject* (MObject::*Method_MObjectP_X_constMStdStringA_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&, const MStdString&),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMStdStringA_constMStdStringA_constMStdStringA)def->m_objectMethod))(p[0].AsString(), p[1].AsString(), p[2].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_OBJECT)


   // Four-parameter procedures and functions

_M_PROCEDURE4(ST_X_int_int_int_int,
   void (MObject::*Method_X_int_int_int_int)(int, int, int, int),
   (this->*((Method_X_int_int_int_int)def->m_objectMethod))(p[0].AsInt(), p[1].AsInt(), p[2].AsInt(), p[3].AsInt()),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT))

_M_PROCEDURE4(ST_X_constMStdStringVectorA_constMStdStringA_unsigned_bool,
   void (MObject::*Method_X_constMStdStringVectorA_constMStdStringA_unsigned_bool)(const MStdStringVector&, const MStdString&, unsigned, bool),
   (this->*((Method_X_constMStdStringVectorA_constMStdStringA_unsigned_bool)def->m_objectMethod))(p[0].AsStringCollection(), p[1].AsString(), p[2].AsUInt(), p[3].AsBool()),
   (MVariant::VAR_STRING_COLLECTION, MVariant::VAR_STRING, MVariant::VAR_UINT, MVariant::VAR_BOOL))

_M_PROCEDURE4(ST_X_constMVariantA_int_int_int,
   void (MObject::*Method_X_constMVariantA_int_int_int)(const MVariant&, int, int, int),
   (this->*((Method_X_constMVariantA_int_int_int)def->m_objectMethod))(p[0], p[1].AsInt(), p[2].AsInt(), p[3].AsInt()),
   (MVariant::VAR_VARIANT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT))

_M_PROCEDURE4(ST_X_MObjectP_constMStdStringA_unsigned_bool,
   void (MObject::*Method_X_MObjectP_constMStdStringA_unsigned_bool)(MObject*, const MStdString&, unsigned, bool),
   (this->*((Method_X_MObjectP_constMStdStringA_unsigned_bool)def->m_objectMethod))(p[0].AsObject(), p[1].AsString(), p[2].AsUInt(), p[3].AsBool()),
   (MVariant::VAR_OBJECT, MVariant::VAR_STRING, MVariant::VAR_UINT, MVariant::VAR_BOOL))

_M_FUNCTION4(ST_MObjectP_X_constMStdStringA_constMStdStringA_constMStdStringA_constMVariantA,
   MObject* (MObject::*Method_MObjectP_X_constMStdStringA_constMStdStringA_constMStdStringA_constMVariantA)(const MStdString&, const MStdString&, const MStdString&, const MVariant&),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_constMStdStringA_constMStdStringA_constMStdStringA_constMVariantA)def->m_objectMethod))(p[0].AsString(), p[1].AsString(), p[2].AsString(), p[3])),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_VARIANT), MVariant::VAR_OBJECT)

_M_FUNCTION4(ST_MObjectP_X_MObjectP_MObjectP_MObjectP_MObjectP,
   MObject* (MObject::*Method_MObjectP_X_MObjectP_MObjectP_MObjectP_MObjectP)(MObject*, MObject*, MObject*, MObject*),
   result.DoAssignToEmpty((this->*((Method_MObjectP_X_MObjectP_MObjectP_MObjectP_MObjectP)def->m_objectMethod))(p[0].AsObject(), p[1].AsObject(), p[2].AsObject(), p[3].AsObject())),
   (MVariant::VAR_OBJECT, MVariant::VAR_OBJECT, MVariant::VAR_OBJECT, MVariant::VAR_OBJECT), MVariant::VAR_OBJECT)

_M_PROCEDURE4(ST_X_constMStdStringA_constMStdStringA_unsigned_bool,
   void (MObject::*Method_X_constMStdStringA_constMStdStringA_unsigned_bool)(const MStdString&, const MStdString&, unsigned, bool),
   (this->*((Method_X_constMStdStringA_constMStdStringA_unsigned_bool)def->m_objectMethod))(p[0].AsString(), p[1].AsString(), p[2].AsUInt(), p[3].AsBool()),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_UINT, MVariant::VAR_BOOL))

_M_PROCEDURE4(ST_X_byte_constMByteStringA_constMByteStringA_bool,
   void (MObject::*Method_X_byte_constMByteStringA_constMByteStringA_bool)(Muint8, const MByteString&, const MByteString&, bool),
   (this->*((Method_X_byte_constMByteStringA_constMByteStringA_bool)def->m_objectMethod))(p[0].AsByte(), p[1].AsByteString(), p[2].AsByteString(), p[3].AsBool()),
   (MVariant::VAR_BYTE, MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE_STRING, MVariant::VAR_BOOL))

_M_PROCEDURE4(ST_X_constMVariantA_constMByteStringA_int_unsigned,
   void (MObject::*Method_X_constMVariantA_constMByteStringA_int_unsigned)(const MVariant&, const MByteString&, int, unsigned),
   (this->*((Method_X_constMVariantA_constMByteStringA_int_unsigned)def->m_objectMethod))(p[0], p[1].AsByteString(), p[2].AsInt(), p[3].AsUInt()),
   (MVariant::VAR_VARIANT, MVariant::VAR_BYTE_STRING, MVariant::VAR_INT, MVariant::VAR_UINT))


   // Five-parameter procedures and functions

_M_PROCEDURE5(ST_X_int_int_int_int_int,
   void (MObject::*Method_X_int_int_int_int_int)(int, int, int, int, int),
   (this->*((Method_X_int_int_int_int_int)def->m_objectMethod))(p[0].AsInt(), p[1].AsInt(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt()),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT))

_M_PROCEDURE5(ST_X_constMVariantA_constMByteStringA_int_int_int,
   void (MObject::*Method_X_constMVariantA_constMByteStringA_int_int_int)(const MVariant&, const MByteString&, int, int, int),
   (this->*((Method_X_constMVariantA_constMByteStringA_int_int_int)def->m_objectMethod))(p[0], p[1].AsByteString(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt()),
   (MVariant::VAR_VARIANT, MVariant::VAR_BYTE_STRING, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT))

_M_FUNCTION5(ST_MByteString_X_constMVariantA_int_int_int_int,
   MByteString (MObject::*Method_MByteString_X_constMVariantA_int_int_int_int)(const MVariant&, int, int, int, int),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMVariantA_int_int_int_int)def->m_objectMethod))(p[0], p[1].AsInt(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_BYTE_STRING)


   // Six-parameter procedures and functions

_M_PROCEDURE6(ST_X_constMVariantA_constMByteStringA_int_int_int_int,
   void (MObject::*Method_X_constMVariantA_constMByteStringA_int_int_int_int)(const MVariant&, const MByteString&, int, int, int, int),
   (this->*((Method_X_constMVariantA_constMByteStringA_int_int_int_int)def->m_objectMethod))(p[0], p[1].AsByteString(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt(), p[5].AsInt()),
   (MVariant::VAR_VARIANT, MVariant::VAR_BYTE_STRING, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT))

_M_PROCEDURE6(ST_X_int_int_int_int_int_int,
   void (MObject::*Method_X_int_int_int_int_int_int)(int, int, int, int, int, int),
   (this->*((Method_X_int_int_int_int_int_int)def->m_objectMethod))(p[0].AsInt(), p[1].AsInt(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt(), p[5].AsInt()),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT))

_M_FUNCTION6(ST_MByteString_X_constMVariantA_constMByteStringA_int_int_int_int,
   MByteString (MObject::*Method_MByteString_X_constMVariantA_constMByteStringA_int_int_int_int)(const MVariant&, const MByteString&, int, int, int, int),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMVariantA_constMByteStringA_int_int_int_int)def->m_objectMethod))(p[0], p[1].AsByteString(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt(), p[5].AsInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_BYTE_STRING, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_BYTE_STRING)

   // THE FOLLOWING SHOULD BE THE LAST DEFINITION!
   // PLEASE INSERT NEW METHODS ABOVE THIS LINE!

_M_FUNCTION6(ST_MByteString_X_constMVariantA_int_int_int_int_int,
   MByteString (MObject::*Method_MByteString_X_constMVariantA_int_int_int_int_int)(const MVariant&, int, int, int, int, int),
   result.DoAssignByteStringToEmpty((this->*((Method_MByteString_X_constMVariantA_int_int_int_int_int)def->m_objectMethod))(p[0], p[1].AsInt(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt(), p[5].AsInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_BYTE_STRING)

#ifdef _M
   #undef _M
#endif
#undef _M_PROCEDURE0
#undef _M_PROCEDURE1
#undef _M_PROCEDURE2
#undef _M_PROCEDURE3
#undef _M_PROCEDURE4
#undef _M_PROCEDURE5
#undef _M_PROCEDURE6
#undef _M_FUNCTION0
#undef _M_FUNCTION1
#undef _M_FUNCTION2
#undef _M_FUNCTION3
#undef _M_FUNCTION4
#undef _M_FUNCTION5
#undef _M_FUNCTION6
#undef _B2
#undef _B3
#undef _B4
#undef _B5
#undef _B6
#undef _M_NULL
