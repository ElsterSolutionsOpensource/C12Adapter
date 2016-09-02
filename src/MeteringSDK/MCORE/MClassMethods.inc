// File MCORE/MClassMethods.inc

// If service parameters matter, we define individual macros,
// Otherwise just _M(st, method, call, par, ret) is enough.
//

#define _M_NULL {(MVariant::Type)0xFF}

#ifdef _M
   #define _M_PROCEDURE0(st, type, call)          _M(st, type, call, _M_NULL,MVariant::VAR_EMPTY, 0)
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

_M_PROCEDURE0(ST_S,
   void (*Method_S)(),
   ((Method_S)def->m_classMethod)())

_M_FUNCTION0(ST_bool_S,
   bool (*Method_bool_S)(),
   result.DoAssignToEmpty(((Method_bool_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_BOOL)

_M_FUNCTION0(ST_byte_S,
   Muint8 (*Method_byte_S)(),
   result.DoAssignToEmpty(((Method_byte_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_BYTE)

_M_FUNCTION0(ST_MChar_S,
   MChar (*Method_MChar_S)(),
   result.DoAssignToEmpty(((Method_MChar_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_CHAR)

_M_FUNCTION0(ST_int_S,
   int (*Method_int_S)(),
   result.DoAssignToEmpty(((Method_int_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_INT)

_M_FUNCTION0(ST_unsigned_S,
   unsigned (*Method_unsigned_S)(),
   result.DoAssignToEmpty(((Method_unsigned_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_UINT)

_M_FUNCTION0(ST_double_S,
   double (*Method_double_S)(),
   result.DoAssignToEmpty(((Method_double_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_DOUBLE)

_M_FUNCTION0(ST_constMVariantA_S,
   const MVariant& (*Method_constMVariantA_S)(),
   result.DoAssignToEmpty(((Method_constMVariantA_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_VARIANT)

_M_FUNCTION0(ST_MVariant_S,
   MVariant (*Method_MVariant_S)(),
   result.DoAssignToEmpty(((Method_MVariant_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_VARIANT)

_M_FUNCTION0(ST_constMByteStringA_S,
   const MByteString& (*Method_constMByteStringA_S)(),
   result.DoAssignByteStringToEmpty(((Method_constMByteStringA_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_BYTE_STRING)

_M_FUNCTION0(ST_MByteString_S,
   MByteString (*Method_MByteString_S)(),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_BYTE_STRING)

_M_FUNCTION0(ST_constMStdStringA_S,
   const MStdString& (*Method_constMStdStringA_S)(),
   result.DoAssignToEmpty(((Method_constMStdStringA_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_STRING)

_M_FUNCTION0(ST_MStdString_S,
   MStdString (*Method_MStdString_S)(),
   result.DoAssignToEmpty(((Method_MStdString_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_STRING)

_M_FUNCTION0(ST_MConstChars_S,
   MConstChars (*Method_MConstChars_S)(),
   result.DoAssignToEmpty(((Method_MConstChars_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_STRING)

_M_FUNCTION0(ST_constMStdStringVectorA_S,
   const MStdStringVector& (*Method_constMStdStringVectorA_S)(),
   result.DoAssignToEmpty(((Method_constMStdStringVectorA_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION0(ST_MStdStringVector_S,
   MStdStringVector (*Method_MStdStringVector_S)(),
   result.DoAssignToEmpty(((Method_MStdStringVector_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION0(ST_MObjectP_S,
   MObject* (*Method_MObjectP_S)(),
   result.DoAssignToEmpty(((Method_MObjectP_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_OBJECT)

_M_FUNCTION0(ST_MObjectByValue_S,
   MVariant::ObjectByValue (*Method_MObjectByValue_S)(),
   result.DoAssignToEmpty(((Method_MObjectByValue_S)def->m_classMethod)()),
   _M_NULL, MVariant::VAR_OBJECT)


   // One-parameter procedures and functions

_M_PROCEDURE1(ST_S_bool,
   void (*Method_S_bool)(bool),
   ((Method_S_bool)def->m_classMethod)(p[0].AsBool()),
   {MVariant::VAR_BOOL})

_M_PROCEDURE1(ST_S_byte,
   void (*Method_S_byte)(Muint8),
   ((Method_S_byte)def->m_classMethod)(p[0].AsByte()),
   {MVariant::VAR_BYTE})

_M_PROCEDURE1(ST_S_MChar,
   void (*Method_S_MChar)(MChar),
   ((Method_S_MChar)def->m_classMethod)(p[0].AsChar()),
   {MVariant::VAR_CHAR})

_M_PROCEDURE1(ST_S_int,
   void (*Method_S_int)(int),
   ((Method_S_int)def->m_classMethod)(p[0].AsInt()),
   {MVariant::VAR_INT})

_M_PROCEDURE1(ST_S_unsigned,
   void (*Method_S_unsigned)(unsigned),
   ((Method_S_unsigned)def->m_classMethod)(p[0].AsUInt()),
   {MVariant::VAR_UINT})

_M_PROCEDURE1(ST_S_double,
   void (*Method_S_double)(double),
   ((Method_S_double)def->m_classMethod)(p[0].AsDouble()),
   {MVariant::VAR_DOUBLE})

_M_PROCEDURE1(ST_S_constMVariantA,
   void (*Method_S_constMVariantA)(const MVariant&),
   ((Method_S_constMVariantA)def->m_classMethod)(p[0]),
   {MVariant::VAR_VARIANT})

_M_PROCEDURE1(ST_S_constMByteStringA,
   void (*Method_S_constMByteStringA)(const MByteString&),
   ((Method_S_constMByteStringA)def->m_classMethod)(p[0].AsByteString()),
   {MVariant::VAR_BYTE_STRING})

_M_PROCEDURE1(ST_S_constMStdStringA,
   void (*Method_S_constMStdStringA)(const MStdString&),
   ((Method_S_constMStdStringA)def->m_classMethod)(p[0].AsString()),
   {MVariant::VAR_STRING})

_M_PROCEDURE1(ST_S_MConstChars,
   void (*Method_S_MConstChars)(MConstChars),
   ((Method_S_MConstChars)def->m_classMethod)(p[0].AsString().c_str()),
   {MVariant::VAR_STRING})

_M_PROCEDURE1(ST_S_constMStdStringVectorA,
   void (*Method_S_constMStdStringVectorA)(const MStdStringVector&),
   ((Method_S_constMStdStringVectorA)def->m_classMethod)(p[0].AsStringCollection()),
   {MVariant::VAR_STRING_COLLECTION})

_M_PROCEDURE1(ST_S_MObjectP,
   void (*Method_S_MObjectP)(MObject*),
   ((Method_S_MObjectP)def->m_classMethod)(p[0].AsObject()),
   {MVariant::VAR_OBJECT})

_M_FUNCTION1(ST_bool_S_byte,
   bool (*Method_bool_S_byte)(Muint8),
   result.DoAssignToEmpty(((Method_bool_S_byte)def->m_classMethod)(p[0].AsByte())),
   {MVariant::VAR_BYTE}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_bool_S_int,
   bool (*Method_bool_S_int)(int),
   result.DoAssignToEmpty(((Method_bool_S_int)def->m_classMethod)(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_bool_S_unsigned,
   bool (*Method_bool_S_unsigned)(unsigned),
   result.DoAssignToEmpty(((Method_bool_S_unsigned)def->m_classMethod)(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_bool_S_constMByteStringA,
   bool (*Method_bool_S_constMByteStringA)(const MByteString&),
   result.DoAssignToEmpty(((Method_bool_S_constMByteStringA)def->m_classMethod)(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_int_S_int,
   int (*Method_int_S_int)(int),
   result.DoAssignToEmpty(((Method_int_S_int)def->m_classMethod)(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_INT)

_M_FUNCTION1(ST_int_S_double,
   int (*Method_int_S_double)(double),
   result.DoAssignToEmpty(((Method_int_S_double)def->m_classMethod)(p[0].AsDouble())),
   {MVariant::VAR_DOUBLE}, MVariant::VAR_INT)

_M_FUNCTION1(ST_double_S_int,
   double (*Method_double_S_int)(int),
   result.DoAssignToEmpty(((Method_double_S_int)def->m_classMethod)(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_DOUBLE)

_M_FUNCTION1(ST_double_S_double,
   double (*Method_double_S_double)(double),
   result.DoAssignToEmpty(((Method_double_S_double)def->m_classMethod)(p[0].AsDouble())),
   {MVariant::VAR_DOUBLE}, MVariant::VAR_DOUBLE)

_M_FUNCTION1(ST_double_S_constMByteStringA,
   double (*Method_double_S_constMByteStringA)(const MByteString&),
   result.DoAssignToEmpty(((Method_double_S_constMByteStringA)def->m_classMethod)(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_DOUBLE)

_M_FUNCTION1(ST_unsigned_S_constMByteStringA,
   unsigned (*Method_unsigned_S_constMByteStringA)(const MByteString&),
   result.DoAssignToEmpty(((Method_unsigned_S_constMByteStringA)def->m_classMethod)(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_UINT)

_M_FUNCTION1(ST_bool_S_constMStdStringA,
   bool (*Method_bool_S_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty(((Method_bool_S_constMStdStringA)def->m_classMethod)(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_MStdString_S_int,
   MStdString (*Method_MStdString_S_int)(int),
   result.DoAssignToEmpty(((Method_MStdString_S_int)def->m_classMethod)(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdString_S_constMStdStringA,
   MStdString (*Method_MStdString_S_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty(((Method_MStdString_S_constMStdStringA)def->m_classMethod)(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdString_S_constMByteStringA,
   MStdString (*Method_MStdString_S_constMByteStringA)(const MByteString&),
   result.DoAssignToEmpty(((Method_MStdString_S_constMByteStringA)def->m_classMethod)(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MByteString_S_constMByteStringA,
   MByteString (*Method_MByteString_S_constMByteStringA)(const MByteString&),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMByteStringA)def->m_classMethod)(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MByteString_S_constMStdStringA,
   MByteString (*Method_MByteString_S_constMStdStringA)(const MStdString&),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMStdStringA)def->m_classMethod)(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MByteString_S_unsigned,
   MByteString (*Method_MByteString_S_unsigned)(unsigned),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_unsigned)def->m_classMethod)(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MByteString_S_double,
   MByteString (*Method_MByteString_S_double)(double),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_double)def->m_classMethod)(p[0].AsDouble())),
   {MVariant::VAR_DOUBLE}, MVariant::VAR_BYTE_STRING)

_M_FUNCTION1(ST_MStdString_S_constMVariantA,
   MStdString (*Method_MStdString_S_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty(((Method_MStdString_S_constMVariantA)def->m_classMethod)(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdString_S_MObjectP,
   MStdString (*Method_MStdString_S_MObjectP)(MObject*),
   result.DoAssignToEmpty(((Method_MStdString_S_MObjectP)def->m_classMethod)(p[0].AsObject())),
   {MVariant::VAR_OBJECT}, MVariant::VAR_STRING)

_M_FUNCTION1(ST_MStdStringVector_S_bool,
   MStdStringVector (*Method_MStdStringVector_S_bool)(bool),
   result.DoAssignToEmpty(((Method_MStdStringVector_S_bool)def->m_classMethod)(p[0].AsBool())),
   {MVariant::VAR_BOOL}, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION1(ST_MStdStringVector_S_constMStdStringA,
   MStdStringVector (*Method_MStdStringVector_S_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty(((Method_MStdStringVector_S_constMStdStringA)def->m_classMethod)(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION1(ST_byte_S_unsigned,
   Muint8 (*Method_byte_S_unsigned)(unsigned),
   result.DoAssignToEmpty(((Method_byte_S_unsigned)def->m_classMethod)(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_BYTE)

_M_FUNCTION1(ST_MChar_S_unsigned,
   MChar (*Method_MChar_S_unsigned)(unsigned),
   result.DoAssignToEmpty(((Method_MChar_S_unsigned)def->m_classMethod)(p[0].AsUInt())),
   {MVariant::VAR_UINT}, MVariant::VAR_CHAR)

_M_FUNCTION1(ST_unsigned_S_byte,
   unsigned (*Method_unsigned_S_byte)(Muint8),
   result.DoAssignToEmpty(((Method_unsigned_S_byte)def->m_classMethod)(p[0].AsByte())),
   {MVariant::VAR_BYTE}, MVariant::VAR_UINT)

_M_FUNCTION1(ST_unsigned_S_MChar,
   unsigned (*Method_unsigned_S_MChar)(MChar),
   result.DoAssignToEmpty(((Method_unsigned_S_MChar)def->m_classMethod)(p[0].AsChar())),
   {MVariant::VAR_CHAR}, MVariant::VAR_UINT)

_M_FUNCTION1(ST_bool_S_MObjectP,
   bool (*Method_bool_S_MObjectP)(MObject*),
   result.DoAssignToEmpty(((Method_bool_S_MObjectP)def->m_classMethod)(p[0].AsObject())),
   {MVariant::VAR_OBJECT}, MVariant::VAR_BOOL)

_M_FUNCTION1(ST_MVariant_S_int,
   MVariant (*Method_MVariant_S_int)(int),
   result.DoAssignToEmpty(((Method_MVariant_S_int)def->m_classMethod)(p[0].AsInt())),
   {MVariant::VAR_INT}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MVariant_S_constMVariantA,
   MVariant (*Method_MVariant_S_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty(((Method_MVariant_S_constMVariantA)def->m_classMethod)(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MVariant_S_MObjectP,
   MVariant (*Method_MVariant_S_MObjectP)(MObject*),
   result.DoAssignToEmpty(((Method_MVariant_S_MObjectP)def->m_classMethod)(p[0].AsObject())),
   {MVariant::VAR_OBJECT}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MObjectP_S_MObjectP,
   MObject* (*Method_MObjectP_S_MObjectP)(MObject*),
   result.DoAssignToEmpty(((Method_MObjectP_S_MObjectP)def->m_classMethod)(p[0].AsObject())),
   {MVariant::VAR_OBJECT}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_MObjectP_S_constMStdStringA,
   MObject* (*Method_MObjectP_S_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMStdStringA)def->m_classMethod)(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_MVariant_S_constMStdStringA,
   MVariant (*Method_MVariant_S_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty(((Method_MVariant_S_constMStdStringA)def->m_classMethod)(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_VARIANT)

_M_FUNCTION1(ST_MObjectP_S_constMByteStringA,
   MObject* (*Method_MObjectP_S_constMByteStringA)(const MByteString&),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMByteStringA)def->m_classMethod)(p[0].AsByteString())),
   {MVariant::VAR_BYTE_STRING}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_MObjectP_S_constMVariantA,
   MObject* (*Method_MObjectP_S_constMVariantA)(const MVariant&),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMVariantA)def->m_classMethod)(p[0])),
   {MVariant::VAR_VARIANT}, MVariant::VAR_OBJECT)

_M_FUNCTION1(ST_MObjectByValue_S_constMStdStringA,
   MVariant::ObjectByValue (*Method_MObjectByValue_S_constMStdStringA)(const MStdString&),
   result.DoAssignToEmpty(((Method_MObjectByValue_S_constMStdStringA)def->m_classMethod)(p[0].AsString())),
   {MVariant::VAR_STRING}, MVariant::VAR_OBJECT)

   // Two-parameter procedures and functions
   
_M_PROCEDURE2(ST_S_byte_byte,
   void (*Method_S_byte_byte)(Muint8, Muint8),
   ((Method_S_byte_byte)def->m_classMethod)(p[0].AsByte(), p[1].AsByte()),
   (MVariant::VAR_BYTE, MVariant::VAR_BYTE))

_M_PROCEDURE2(ST_S_byte_constMByteStringA,
   void (*Method_S_byte_constMByteStringA)(Muint8, const MByteString&),
   ((Method_S_byte_constMByteStringA)def->m_classMethod)(p[0].AsByte(), p[1].AsByteString()),
   (MVariant::VAR_BYTE, MVariant::VAR_BYTE_STRING))

_M_PROCEDURE2(ST_S_constMStdStringA_constMStdStringA,
   void (*Method_S_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   ((Method_S_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString()),
   (MVariant::VAR_STRING, MVariant::VAR_STRING))

_M_PROCEDURE2(ST_S_int_constMStdStringA,
   void (*Method_S_int_constMStdStringA)(int, const MStdString&),
   ((Method_S_int_constMStdStringA)def->m_classMethod)(p[0].AsInt(), p[1].AsString()),
   (MVariant::VAR_INT, MVariant::VAR_STRING))

_M_PROCEDURE2(ST_S_int_int,
   void (*Method_S_int_int)(int, int),
   ((Method_S_int_int)def->m_classMethod)(p[0].AsInt(), p[1].AsInt()),
   (MVariant::VAR_INT, MVariant::VAR_INT))

_M_PROCEDURE2(ST_S_double_MConstChars,
   void (*Method_S_double_MConstChars)(double, MConstChars),
   ((Method_S_double_MConstChars)def->m_classMethod)(p[0].AsDouble(), p[1].AsString().c_str()),
   (MVariant::VAR_DOUBLE, MVariant::VAR_STRING))

_M_FUNCTION2(ST_double_S_double_int,
   double (*Method_double_S_double_int)(double, int),
   result.DoAssignToEmpty(((Method_double_S_double_int)def->m_classMethod)(p[0].AsDouble(), p[1].AsInt())),
   (MVariant::VAR_DOUBLE, MVariant::VAR_INT), MVariant::VAR_DOUBLE)

_M_FUNCTION2(ST_double_S_double_double,
   double (*Method_double_S_double_double)(double, double),
   result.DoAssignToEmpty(((Method_double_S_double_double)def->m_classMethod)(p[0].AsDouble(), p[1].AsDouble())),
   (MVariant::VAR_DOUBLE, MVariant::VAR_DOUBLE), MVariant::VAR_DOUBLE)

_M_FUNCTION2(ST_double_S_constMByteStringA_bool,
   double (*Method_double_S_constMByteStringA_bool)(const MByteString&, bool),
   result.DoAssignToEmpty(((Method_double_S_constMByteStringA_bool)def->m_classMethod)(p[0].AsByteString(), p[1].AsBool())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BOOL), MVariant::VAR_DOUBLE)

_M_FUNCTION2(ST_int_S_int_int,
   int (*Method_int_S_int_int)(int, int),
   result.DoAssignToEmpty(((Method_int_S_int_int)def->m_classMethod)(p[0].AsInt(), p[1].AsInt())),
   (MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_INT)

_M_FUNCTION2(ST_unsigned_S_unsigned_unsigned,
   unsigned (*Method_unsigned_S_unsigned_unsigned)(unsigned, unsigned),
   result.DoAssignToEmpty(((Method_unsigned_S_unsigned_unsigned)def->m_classMethod)(p[0].AsUInt(), p[1].AsUInt())),
   (MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_UINT)

_M_FUNCTION2(ST_int_S_constMStdStringA_constMStdStringA,
   int (*Method_int_S_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   result.DoAssignToEmpty(((Method_int_S_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_INT)

_M_FUNCTION2(ST_bool_S_constMStdStringA_constMStdStringA,
   bool (*Method_bool_S_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   result.DoAssignToEmpty(((Method_bool_S_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_BOOL)

_M_FUNCTION2(ST_MObjectP_S_MObjectP_int,
   MObject* (*Method_MObjectP_S_MObjectP_int)(MObject*, int),
   result.DoAssignToEmpty(((Method_MObjectP_S_MObjectP_int)def->m_classMethod)(p[0].AsObject(), p[1].AsInt())),
   (MVariant::VAR_OBJECT, MVariant::VAR_INT), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MByteString_S_double_unsigned,
   MByteString (*Method_MByteString_S_double_unsigned)(double, unsigned),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_double_unsigned)def->m_classMethod)(p[0].AsDouble(), p[1].AsUInt())),
   (MVariant::VAR_DOUBLE, MVariant::VAR_UINT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MByteString_S_constMByteStringA_bool,
   MByteString (*Method_MByteString_S_constMByteStringA_bool)(const MByteString&, bool),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMByteStringA_bool)def->m_classMethod)(p[0].AsByteString(), p[1].AsBool())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BOOL), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MByteString_S_constMByteStringA_constMByteStringA,
   MByteString (*Method_MByteString_S_constMByteStringA_constMByteStringA)(const MByteString&, const MByteString&),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMByteStringA_constMByteStringA)def->m_classMethod)(p[0].AsByteString(), p[1].AsByteString())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE_STRING), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MByteString_S_constMByteStringA_constMVariantA,
   MByteString (*Method_MByteString_S_constMByteStringA_constMVariantA)(const MByteString&, const MVariant&),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMByteStringA_constMVariantA)def->m_classMethod)(p[0].AsByteString(), p[1])),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_VARIANT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MStdString_S_constMByteStringA_bool,
   MStdString (*Method_MStdString_S_constMByteStringA_bool)(const MByteString&, bool),
   result.DoAssignToEmpty(((Method_MStdString_S_constMByteStringA_bool)def->m_classMethod)(p[0].AsByteString(), p[1].AsBool())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MByteString_S_constMStdStringA_bool,
   MByteString (*Method_MByteString_S_constMStdStringA_bool)(const MStdString&, bool),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMStdStringA_bool)def->m_classMethod)(p[0].AsString(), p[1].AsBool())),
   (MVariant::VAR_STRING, MVariant::VAR_BYTE), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MStdString_S_constMByteStringA_constMStdStringA,
   MStdString (*Method_MStdString_S_constMByteStringA_constMStdStringA)(const MByteString&, const MStdString&),
   result.DoAssignToEmpty((((Method_MStdString_S_constMByteStringA_constMStdStringA)def->m_classMethod)(p[0].AsByteString(), p[1].AsString()))),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_STRING), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MStdString_S_constMByteStringA_constMVariantA,
   MStdString (*Method_MStdString_S_constMByteStringA_constMVariantA)(const MByteString&, const MVariant&),
   result.DoAssignToEmpty((((Method_MStdString_S_constMByteStringA_constMVariantA)def->m_classMethod)(p[0].AsByteString(), p[1]))),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_VARIANT), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MByteString_S_constMStdStringA_unsigned,
   MByteString (*Method_MByteString_S_constMStdStringA_unsigned)(const MStdString&, unsigned),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMStdStringA_unsigned)def->m_classMethod)(p[0].AsString(), p[1].AsUInt())),
   (MVariant::VAR_STRING, MVariant::VAR_UINT), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MStdString_S_int_constMVariantA,
   MStdString (*Method_MStdString_S_int_constMVariantA)(int, const MVariant&),
   result.DoAssignToEmpty(((Method_MStdString_S_int_constMVariantA)def->m_classMethod)(p[0].AsInt(), p[1])),
   (MVariant::VAR_INT, MVariant::VAR_VARIANT), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MStdString_S_MObjectP_bool,
   MStdString (*Method_MStdString_S_MObjectP_bool)(MObject*, bool),
   result.DoAssignToEmpty(((Method_MStdString_S_MObjectP_bool)def->m_classMethod)(p[0].AsObject(), p[1].AsBool())),
   (MVariant::VAR_OBJECT, MVariant::VAR_BOOL), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MVariant_S_constMByteStringA_bool,
   MVariant (*Method_MVariant_S_constMByteStringA_bool)(const MByteString&, bool),
   result.DoAssignToEmpty(((Method_MVariant_S_constMByteStringA_bool)def->m_classMethod)(p[0].AsByteString(), p[1].AsBool())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BOOL), MVariant::VAR_VARIANT)

_M_FUNCTION2(ST_MVariant_S_constMVariantA_unsigned,
   MVariant (*Method_MVariant_S_constMVariantA_unsigned)(const MVariant&, unsigned),
   result.DoAssignToEmpty(((Method_MVariant_S_constMVariantA_unsigned)def->m_classMethod)(p[0], p[1].AsUInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_UINT), MVariant::VAR_VARIANT)

_M_FUNCTION2(ST_MStdString_S_constMStdStringA_constMStdStringA,
   MStdString (*Method_MStdString_S_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   result.DoAssignToEmpty(((Method_MStdString_S_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_STRING)

_M_FUNCTION2(ST_MByteString_S_constMStdStringA_constMStdStringA,
   MByteString (*Method_MByteString_S_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_BYTE_STRING)

_M_FUNCTION2(ST_MStdString_S_constMStdStringA_constMByteStringA,
   MByteString (*Method_MStdString_S_constMStdStringA_constMByteStringA)(const MStdString&, const MByteString&),
   result.DoAssignToEmpty(((Method_MStdString_S_constMStdStringA_constMByteStringA)def->m_classMethod)(p[0].AsString(), p[1].AsByteString())),
   (MVariant::VAR_STRING, MVariant::VAR_BYTE_STRING), MVariant::VAR_STRING)

_M_FUNCTION2(ST_int_S_constMVariantA_constMVariantA,
   int (*Method_int_S_constMVariantA_constMVariantA)(const MVariant&, const MVariant&),
   result.DoAssignToEmpty(((Method_int_S_constMVariantA_constMVariantA)def->m_classMethod)(p[0], p[1])),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT), MVariant::VAR_INT)

_M_FUNCTION2(ST_MObjectP_S_int_constMStdStringA,
   MObject* (*Method_MObjectP_S_int_constMStdStringA)(int, const MStdString&),
   result.DoAssignToEmpty(((Method_MObjectP_S_int_constMStdStringA)def->m_classMethod)(p[0].AsInt(), p[1].AsString())),
   (MVariant::VAR_INT, MVariant::VAR_STRING), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MObjectP_S_constMStdStringA_bool,
   MObject* (*Method_MObjectP_S_constMStdStringA_bool)(const MStdString&, bool),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMStdStringA_bool)def->m_classMethod)(p[0].AsString(), p[1].AsBool())),
   (MVariant::VAR_STRING, MVariant::VAR_BOOL), MVariant::VAR_OBJECT)
   
_M_FUNCTION2(ST_MObjectP_S_constMByteStringA_unsigned,
   MObject* (*Method_MObjectP_S_constMByteStringA_unsigned)(const MByteString&, unsigned),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMByteStringA_unsigned)def->m_classMethod)(p[0].AsByteString(), p[1].AsUInt())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_UINT), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MObjectP_S_constMStdStringA_unsigned,
   MObject* (*Method_MObjectP_S_constMStdStringA_unsigned)(const MStdString&, unsigned),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMStdStringA_unsigned)def->m_classMethod)(p[0].AsString(), p[1].AsUInt())),
   (MVariant::VAR_STRING, MVariant::VAR_UINT), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MObjectP_S_constMVariantA_unsigned,
   MObject* (*Method_MObjectP_S_constMVariantA_unsigned)(const MVariant&, unsigned),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMVariantA_unsigned)def->m_classMethod)(p[0], p[1].AsUInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_UINT), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MObjectP_S_constMVariantA_constMVariantA,
   MObject* (*Method_MObjectP_S_constMVariantA_constMVariantA)(const MVariant&, const MVariant&),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMVariantA_constMVariantA)def->m_classMethod)(p[0], p[1])),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_unsigned_S_constMByteStringA_constMByteStringA,
   unsigned (*Method_unsigned_S_constMByteStringA_constMByteStringA)(const MByteString&, const MByteString&),
   result.DoAssignToEmpty(((Method_unsigned_S_constMByteStringA_constMByteStringA)def->m_classMethod)(p[0].AsByteString(), p[1].AsByteString())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE_STRING), MVariant::VAR_UINT)

_M_FUNCTION2(ST_MObjectP_S_constMVariantA_constMStdStringA,
   MObject* (*Method_MObjectP_S_constMVariantA_constMStdStringA)(const MVariant&, const MStdString&),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMVariantA_constMStdStringA)def->m_classMethod)(p[0], p[1].AsString())),
   (MVariant::VAR_VARIANT, MVariant::VAR_STRING), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MVariant_S_int_constMStdStringA,
   MVariant (*Method_MVariant_S_int_constMStdStringA)(int, const MStdString&),
   result.DoAssignToEmpty(((Method_MVariant_S_int_constMStdStringA)def->m_classMethod)(p[0].AsInt(), p[1].AsString())),
   (MVariant::VAR_INT, MVariant::VAR_STRING), MVariant::VAR_VARIANT)

_M_FUNCTION2(ST_MVariant_S_constMVariantA_constMVariantA,
   MVariant (*Method_MVariant_S_constMVariantA_constMVariantA)(const MVariant&, const MVariant&),
   result.DoAssignToEmpty(((Method_MVariant_S_constMVariantA_constMVariantA)def->m_classMethod)(p[0], p[1])),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT), MVariant::VAR_VARIANT)

_M_FUNCTION2(ST_MVariant_S_constMVariantA_bool,
   MVariant (*Method_MVariant_S_constMVariantA_bool)(const MVariant&, bool),
   result.DoAssignToEmpty(((Method_MVariant_S_constMVariantA_bool)def->m_classMethod)(p[0], p[1].AsBool())),
   (MVariant::VAR_VARIANT, MVariant::VAR_BOOL), MVariant::VAR_VARIANT)

_M_FUNCTION2(ST_MObjectP_S_MObjectP_MObjectP,
   MObject* (*Method_MObjectP_S_MObjectP_MObjectP)(MObject*, MObject*),
   result.DoAssignToEmpty(((Method_MObjectP_S_MObjectP_MObjectP)def->m_classMethod)(p[0].AsObject(), p[1].AsObject())),
   (MVariant::VAR_OBJECT, MVariant::VAR_OBJECT), MVariant::VAR_OBJECT)

_M_FUNCTION2(ST_MStdStringVector_S_constMStdStringA_constMStdStringA,
   MStdStringVector (*Method_MStdStringVector_S_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&),
   result.DoAssignToEmpty(((Method_MStdStringVector_S_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_STRING_COLLECTION)

_M_FUNCTION2(ST_MObjectP_S_MObjectP_bool,
   MObject* (*Method_MObjectP_S_MObjectP_bool)(MObject*, bool),
   result.DoAssignToEmpty(((Method_MObjectP_S_MObjectP_bool)def->m_classMethod)(p[0].AsObject(), p[1].AsBool())),
   (MVariant::VAR_OBJECT, MVariant::VAR_BOOL), MVariant::VAR_OBJECT)

   // Three-parameter procedures and functions

_M_PROCEDURE3(ST_S_int_constMStdStringA_constMVariantA,
   void (*Method_S_int_constMStdStringA_constMVariantA)(int, const MStdString&, const MVariant&),
   ((Method_S_int_constMStdStringA_constMVariantA)def->m_classMethod)(p[0].AsInt(), p[1].AsString(), p[2]),
   (MVariant::VAR_INT, MVariant::VAR_STRING, MVariant::VAR_VARIANT))

_M_PROCEDURE3(ST_S_int_int_int,
   void (*Method_S_int_int_int)(int, int, int),
   ((Method_S_int_int_int)def->m_classMethod)(p[0].AsInt(), p[1].AsInt(), p[2].AsInt()),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT))

_M_PROCEDURE3(ST_S_double_double_double,
   void (*Method_S_double_double_double)(double, double, double),
   ((Method_S_double_double_double)def->m_classMethod)(p[0].AsDouble(), p[1].AsDouble(), p[2].AsDouble()),
   (MVariant::VAR_DOUBLE, MVariant::VAR_DOUBLE, MVariant::VAR_DOUBLE))

_M_FUNCTION3(ST_bool_S_MConstChars_constMStdStringA_bool,
   bool (*Method_bool_S_MConstChars_constMStdStringA_bool)(MConstChars, const MStdString&, bool),
   result.DoAssignToEmpty(((Method_bool_S_MConstChars_constMStdStringA_bool)def->m_classMethod)(p[0].AsString().c_str(), p[1].AsString(), p[2].AsBool())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_BOOL), MVariant::VAR_BOOL)

_M_FUNCTION3(ST_int_S_constMVariantA_constMVariantA_bool,
   int (*Method_int_S_constMVariantA_constMVariantA_bool)(const MVariant&, const MVariant&, bool),
   result.DoAssignToEmpty(((Method_int_S_constMVariantA_constMVariantA_bool)def->m_classMethod)(p[0], p[1], p[2].AsBool())),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT, MVariant::VAR_BOOL), MVariant::VAR_INT)

_M_FUNCTION3(ST_MByteString_S_constMByteStringA_constMByteStringA_bool,
   MByteString (*Method_MByteString_S_constMByteStringA_constMByteStringA_bool)(const MByteString&, const MByteString&, bool),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMByteStringA_constMByteStringA_bool)def->m_classMethod)(p[0].AsByteString(), p[1].AsByteString(), p[2].AsBool())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE_STRING, MVariant::VAR_BOOL), MVariant::VAR_BYTE_STRING)

_M_FUNCTION3(ST_MStdString_S_constMStdStringA_constMStdStringA_constMStdStringA,
   MStdString (*Method_MStdString_S_constMStdStringA_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&, const MStdString&),
   result.DoAssignToEmpty(((Method_MStdString_S_constMStdStringA_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString(), p[2].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_STRING)

_M_FUNCTION3(ST_MByteString_S_double_unsigned_bool,
   MByteString (*Method_MByteString_S_double_unsigned_bool)(double, unsigned, bool),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_double_unsigned_bool)def->m_classMethod)(p[0].AsDouble(), p[1].AsUInt(), p[2].AsBool())),
   (MVariant::VAR_DOUBLE, MVariant::VAR_UINT, MVariant::VAR_BOOL), MVariant::VAR_BYTE_STRING)

_M_FUNCTION3(ST_MByteString_S_constMVariantA_unsigned_bool,
   MByteString (*Method_MByteString_S_constMVariantA_unsigned_bool)(const MVariant&, unsigned, bool),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMVariantA_unsigned_bool)def->m_classMethod)(p[0], p[1].AsUInt(), p[2].AsBool())),
   (MVariant::VAR_VARIANT, MVariant::VAR_UINT, MVariant::VAR_BOOL), MVariant::VAR_BYTE_STRING)

_M_FUNCTION3(ST_int_S_constMStdStringA_constMStdStringA_constMStdStringA,
   int (*Method_int_S_constMStdStringA_constMStdStringA_constMStdStringA)(const MStdString&, const MStdString&, const MStdString&),
   result.DoAssignToEmpty(((Method_int_S_constMStdStringA_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString(), p[2].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_INT)

_M_FUNCTION3(ST_MVariant_S_int_int_int,
   MVariant (*Method_MVariant_S_int_int_int)(int, int, int),
   result.DoAssignToEmpty(((Method_MVariant_S_int_int_int)def->m_classMethod)(p[0].AsInt(), p[1].AsInt(), p[2].AsInt())),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_VARIANT)

_M_FUNCTION3(ST_MVariant_S_constMStdStringA_int_int,
   MVariant (*Method_MVariant_S_constMStdStringA_int_int)(const MStdString&, int, int),
   result.DoAssignToEmpty(((Method_MVariant_S_constMStdStringA_int_int)def->m_classMethod)(p[0].AsString(), p[1].AsInt(), p[2].AsInt())),
   (MVariant::VAR_STRING, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_VARIANT)

_M_FUNCTION3(ST_MObjectP_S_constMStdStringA_unsigned_bool,
   MObject* (*Method_MObjectP_S_constMStdStringA_unsigned_bool)(const MStdString&, unsigned, bool),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMStdStringA_unsigned_bool)def->m_classMethod)(p[0].AsString(), p[1].AsUInt(), p[2].AsBool())),
   (MVariant::VAR_STRING, MVariant::VAR_UINT, MVariant::VAR_BOOL), MVariant::VAR_OBJECT)

_M_FUNCTION3(ST_MObjectP_S_constMStdStringA_unsigned_unsigned,
   MObject* (*Method_MObjectP_S_constMStdStringA_unsigned_unsigned)(const MStdString&, unsigned, unsigned),
   result.DoAssignToEmpty(((Method_MObjectP_S_constMStdStringA_unsigned_unsigned)def->m_classMethod)(p[0].AsString(), p[1].AsUInt(), p[2].AsUInt())),
   (MVariant::VAR_STRING, MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_OBJECT)

_M_FUNCTION3(ST_MObjectP_S_int_constMStdStringA_bool,
   MObject* (*Method_MObjectP_S_int_constMStdStringA_bool)(int, const MStdString&, bool),
   result.DoAssignToEmpty(((Method_MObjectP_S_int_constMStdStringA_bool)def->m_classMethod)(p[0].AsInt(), p[1].AsString(), p[2].AsBool())),
   (MVariant::VAR_INT, MVariant::VAR_STRING, MVariant::VAR_BOOL), MVariant::VAR_OBJECT)

_M_FUNCTION3(ST_MByteString_S_constMByteStringA_constMByteStringA_constMByteStringA,
   MByteString (*Method_MByteString_S_constMByteStringA_constMByteStringA_constMByteStringA)(const MByteString&, const MByteString&, const MByteString&),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMByteStringA_constMByteStringA_constMByteStringA)def->m_classMethod)(p[0].AsByteString(), p[1].AsByteString(), p[2].AsByteString())),
   (MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE_STRING, MVariant::VAR_BYTE_STRING), MVariant::VAR_BYTE_STRING)

_M_FUNCTION3(ST_MByteString_S_constMStdStringA_constMStdStringA_constMByteStringA,
   MByteString (*Method_MByteString_S_constMStdStringA_constMStdStringA_constMByteStringA)(const MStdString&, const MStdString&, const MByteString&),
   result.DoAssignByteStringToEmpty(((Method_MByteString_S_constMStdStringA_constMStdStringA_constMByteStringA)def->m_classMethod)(p[0].AsString(), p[1].AsString(), p[2].AsByteString())),
   (MVariant::VAR_STRING, MVariant::VAR_STRING, MVariant::VAR_BYTE_STRING), MVariant::VAR_BYTE_STRING)

_M_FUNCTION3(ST_MStdString_S_constMStdStringA_constMByteStringA_constMStdStringA,
   MStdString (*Method_MStdString_S_constMStdStringA_constMByteStringA_constMStdStringA)(const MStdString&, const MByteString&, const MStdString&),
   result.DoAssignToEmpty(((Method_MStdString_S_constMStdStringA_constMByteStringA_constMStdStringA)def->m_classMethod)(p[0].AsString(), p[1].AsByteString(), p[2].AsString())),
   (MVariant::VAR_STRING, MVariant::VAR_BYTE_STRING, MVariant::VAR_STRING), MVariant::VAR_STRING)

_M_FUNCTION3(ST_MVariant_S_int_constMStdStringA_constMVariantA,
   MVariant (*Method_MVariant_S_int_constMStdStringA_constMVariantA)(int, const MStdString&, const MStdString&),
   result.DoAssignToEmpty(((Method_MVariant_S_int_constMStdStringA_constMVariantA)def->m_classMethod)(p[0].AsInt(), p[1].AsString(), p[2].AsString())),
   (MVariant::VAR_INT, MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_VARIANT)

_M_FUNCTION3(ST_MVariant_S_constMVariantA_constMVariantA_bool,
   MVariant (*Method_MVariant_S_constMVariantA_constMVariantA_bool)(const MVariant&, const MVariant&, bool),
   result.DoAssignToEmpty(((Method_MVariant_S_constMVariantA_constMVariantA_bool)def->m_classMethod)(p[0], p[1], p[2].AsBool())),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT, MVariant::VAR_BOOL), MVariant::VAR_VARIANT)

_M_FUNCTION3(ST_MVariant_S_constMVariantA_unsigned_unsigned,
   MVariant (*Method_MVariant_S_constMVariantA_unsigned_unsigned)(const MVariant&, unsigned, unsigned),
   result.DoAssignToEmpty(((Method_MVariant_S_constMVariantA_unsigned_unsigned)def->m_classMethod)(p[0], p[1].AsUInt(), p[2].AsUInt())),
   (MVariant::VAR_VARIANT, MVariant::VAR_UINT, MVariant::VAR_UINT), MVariant::VAR_VARIANT)

_M_FUNCTION3(ST_MVariant_S_constMVariantA_constMVariantA_constMVariantA,
   MVariant (*Method_MVariant_S_constMVariantA_constMVariantA_constMVariantA)(const MVariant&, const MVariant&, const MVariant&),
   result.DoAssignToEmpty(((Method_MVariant_S_constMVariantA_constMVariantA_constMVariantA)def->m_classMethod)(p[0], p[1], p[2])),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT, MVariant::VAR_VARIANT), MVariant::VAR_VARIANT)


   // Four-parameter procedures and functions

_M_PROCEDURE4(ST_S_int_int_int_MConstChars,
   void (*Method_S_int_int_int_MConstChars)(int, int, int, MConstChars),
   ((Method_S_int_int_int_MConstChars)def->m_classMethod)(p[0].AsInt(), p[1].AsInt(), p[2].AsInt(), p[3].AsString().c_str()),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_STRING))

_M_PROCEDURE4(ST_S_double_double_double_MConstChars,
   void (*Method_S_double_double_double_MConstChars)(double, double, double, MConstChars),
   ((Method_S_double_double_double_MConstChars)def->m_classMethod)(p[0].AsDouble(), p[1].AsDouble(), p[2].AsDouble(), p[3].AsString().c_str()),
   (MVariant::VAR_DOUBLE, MVariant::VAR_DOUBLE, MVariant::VAR_DOUBLE, MVariant::VAR_STRING))

_M_FUNCTION4(ST_MObjectP_S_int_int_MObjectP_MObjectP,
   MObject* (*Method_MObjectP_S_int_int_MObjectP_MObjectP)(int, int, MObject*, MObject*),
   result.DoAssignToEmpty(((Method_MObjectP_S_int_int_MObjectP_MObjectP)def->m_classMethod)(p[0].AsInt(), p[1].AsInt(), p[2].AsObject(), p[3].AsObject())),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_OBJECT, MVariant::VAR_OBJECT), MVariant::VAR_OBJECT)

_M_FUNCTION4(ST_MVariant_S_int_int_int_int,
   MVariant (*Method_MVariant_S_int_int_int_int)(int, int, int, int),
   result.DoAssignToEmpty(((Method_MVariant_S_int_int_int_int)def->m_classMethod)(p[0].AsInt(), p[1].AsInt(), p[2].AsInt(), p[3].AsInt())),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_VARIANT)

_M_FUNCTION4(ST_MVariant_S_constMVariantA_constMVariantA_bool_bool,
   MVariant (*Method_MVariant_S_constMVariantA_constMVariantA_bool_bool)(const MVariant&, const MVariant&, bool, bool),
   result.DoAssignToEmpty(((Method_MVariant_S_constMVariantA_constMVariantA_bool_bool)def->m_classMethod)(p[0], p[1], p[2].AsBool(), p[3].AsBool())),
   (MVariant::VAR_VARIANT, MVariant::VAR_VARIANT, MVariant::VAR_BOOL, MVariant::VAR_BOOL), MVariant::VAR_VARIANT)


   // Five-parameter procedures and functions

_M_FUNCTION5(ST_MVariant_S_int_int_int_int_int,
   MVariant (*Method_MVariant_S_int_int_int_int_int)(int, int, int, int, int),
   result.DoAssignToEmpty(((Method_MVariant_S_int_int_int_int_int)def->m_classMethod)(p[0].AsInt(), p[1].AsInt(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt())),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_VARIANT)


   // Six-parameter procedures and functions

_M_FUNCTION6(ST_MObjectP_S_int_int_MObjectP_MObjectP_constMStdStringA_constMStdStringA,
   MObject* (*Method_MObjectP_S_int_int_MObjectP_MObjectP_constMStdStringA_constMStdStringA)(int, int, MObject*, MObject*, const MStdString&, const MStdString&),
   result.DoAssignToEmpty(((Method_MObjectP_S_int_int_MObjectP_MObjectP_constMStdStringA_constMStdStringA)def->m_classMethod)(p[0].AsInt(), p[1].AsInt(), p[2].AsObject(), p[3].AsObject(), p[4].AsString(), p[5].AsString())),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_OBJECT, MVariant::VAR_OBJECT, MVariant::VAR_STRING, MVariant::VAR_STRING), MVariant::VAR_OBJECT)


   // THE FOLLOWING SHOULD BE THE LAST DEFINITION!
   // PLEASE INSERT NEW METHODS ABOVE THIS LINE!

_M_FUNCTION6(ST_MVariant_S_int_int_int_int_int_int,
   MVariant (*Method_MVariant_S_int_int_int_int_int_int)(int, int, int, int, int, int),
   result.DoAssignToEmpty(((Method_MVariant_S_int_int_int_int_int_int)def->m_classMethod)(p[0].AsInt(), p[1].AsInt(), p[2].AsInt(), p[3].AsInt(), p[4].AsInt(), p[5].AsInt())),
   (MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT, MVariant::VAR_INT), MVariant::VAR_VARIANT)


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
