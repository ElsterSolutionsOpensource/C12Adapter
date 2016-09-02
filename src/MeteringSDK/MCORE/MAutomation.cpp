// File MCORE/MAutomation.cpp

#include "MCOREExtern.h"
#include "MCOREDefs.h"
#include "MTimeSpan.h"
#include "MException.h"
#include "MAutomation.h"

#if !M_NO_AUTOMATION

M_START_PROPERTIES(Automation)
M_START_METHODS(Automation)
   M_CLASS_SERVICE(Automation, CreateObject, ST_MObjectP_S_constMStdStringA)
   M_OBJECT_SERVICE(Automation, GetAutomationProperty, ST_MVariant_X_constMStdStringA)
   M_OBJECT_SERVICE(Automation, SetAutomationProperty, ST_X_constMStdStringA_constMVariantA)
M_END_CLASS(Automation, Object)

   using namespace std;

   // Global category definition for metering objects
   //
   #define CATID_MeterObjectsDefs {0xF0EC64E3,0x50CB,0x11D4,{0x97,0x58,0x00,0x50,0x04,0x05,0x83,0x22}}

   CATID MAutomation::CATID_MeterObjects = CATID_MeterObjectsDefs;

   MAutomation::COMInitializer::COMInitializer(bool delayInitialization) M_NO_THROW
   :
      m_initialized(false),
      m_shouldUninitialize(false)
   {
      if ( !delayInitialization )
         EnsureInitialized();
   }

   void MAutomation::COMInitializer::EnsureInitialized() M_NO_THROW
   {
      if ( !m_initialized )
      {
         HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
         if ( SUCCEEDED(hr) )
            m_shouldUninitialize = true;
         m_initialized = true;
      }
   }

   MAutomation::COMInitializer::~COMInitializer() M_NO_THROW
   {
      if ( m_shouldUninitialize )
         ::CoUninitialize();
   }

   static MAutomation::COMInitializer s_initializer(true);

MAutomation::~MAutomation()
{
   m_dispatch->Release();
}

void MAutomation::RegisterMeterObjectsComponentCategory()
{
   s_initializer.EnsureInitialized();

   static CATEGORYINFO s_categoriesInfo[] =
   {
      {
         CATID_MeterObjectsDefs,
         MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),
         L"Elster Meter Objects"
      }
   };

   // Register component categories
   //
   CComPtr<ICatRegister> catReg;
   HRESULT hResult = catReg.CoCreateInstance(CLSID_StdComponentCategoriesMgr);
   M_ASSERT(!FAILED(hResult));
   hResult = catReg->RegisterCategories(M_NUMBER_OF_ARRAY_ELEMENTS(s_categoriesInfo), s_categoriesInfo);
   M_ASSERT(!FAILED(hResult));
}

SAFEARRAY* MAutomation::ToSafeArray(const char* data, unsigned size)
{
   SAFEARRAYBOUND bound;
   bound.lLbound = 0;
   bound.cElements = size;
   SAFEARRAY* value = SafeArrayCreate(VT_UI1, 1, &bound);
   if ( value == NULL )
     throw bad_alloc();

   if ( size > 0 )
   {
      // The following will always be successful, as we've just created the array by ourselves
      char* buffer;
      SafeArrayAccessData(value, (void**)&buffer);
      memcpy(buffer, data, size);
      SafeArrayUnaccessData(value);
   }
   return value;
}

   static void DoCheckSingleDimensionalArray(bool ok)
   {
      if ( !ok )
      {
         MException::Throw(MException::ErrorSoftware, M_ERR_SINGLE_DIMENSION_ARRAY_IS_REQUIRED, "Single dimension array of bytes is required");
         M_ENSURED_ASSERT(0);
      }
   }

MByteString MAutomation::ToByteString(const SAFEARRAY* sa)
{
   MByteString result;

   SAFEARRAY* safeArray = const_cast<SAFEARRAY*>(sa);
   DoCheckSingleDimensionalArray(SafeArrayGetDim(safeArray) == 1);
   DoCheckSingleDimensionalArray(SafeArrayGetElemsize(safeArray) == 1);

   long lBound;
   long uBound;
   char* data;
   MESystemError::VerifySystemError(SafeArrayGetLBound(safeArray, 1, &lBound));
   MESystemError::VerifySystemError(SafeArrayGetUBound(safeArray, 1, &uBound));
   MESystemError::VerifySystemError(SafeArrayAccessData(safeArray, (void**)&data));
   int size = uBound - lBound + 1;
   result.assign(data, size);
   MESystemError::VerifySystemError(SafeArrayUnaccessData(safeArray));
   return result;
}

MByteString MAutomation::ToByteString(const BSTR vbString)
{
   MStdString result;
   if ( vbString != NULL )
   {
      unsigned len = ::SysStringLen(vbString);
      if ( len > 0 )
      {
         result.reserve(len);
         const wchar_t* vbStringEnd = vbString + len;
         for ( const wchar_t* p = reinterpret_cast<const wchar_t*>(vbString); p != vbStringEnd; ++p )
         {
            unsigned c = static_cast<unsigned>(static_cast<Muint16>(*p));
            MENumberOutOfRange::CheckNamedUnsignedRange(0, 255, c, "OLE string");
            result += static_cast<char>(c);
         }
      }
   }
   return result;
}

MByteString MAutomation::ToByteString(const VARIANT* Value)
{
   MStdString result;
   if ( Value != NULL )
   {
      VARTYPE vt = Value->vt;
      if ( (vt & VT_ARRAY) != 0 )
      {
         SAFEARRAY* arr = (vt & VT_BYREF) != 0 ? *(Value->pparray) : Value->parray;
         if ( (vt & VT_TYPEMASK) == VT_UI1 )
            result = ToByteString(arr);
         else if ( (vt & VT_TYPEMASK) == VT_VARIANT )
         {
            DoCheckSingleDimensionalArray(SafeArrayGetDim(arr) == 1);

            long lBound, uBound;
            MESystemError::VerifySystemError(SafeArrayGetLBound(arr, 1, &lBound));
            MESystemError::VerifySystemError(SafeArrayGetUBound(arr, 1, &uBound));

            for ( long i = lBound; i <= uBound; ++i )
            {
               VARIANT v;
               MESystemError::VerifySystemError(SafeArrayGetElement(arr, &i, reinterpret_cast<void*>(&v)));
               MESystemError::VerifySystemError(VariantChangeType(&v, &v, 0, VT_UI1));
               result.push_back(static_cast<char>(v.bVal));
            }
         }
         else
            DoCheckSingleDimensionalArray(false);
      }
      else if ( vt == VT_BSTR )
      {
         result = MAutomation::ToByteString(Value->bstrVal);
      }
      else if ( vt == (VT_BSTR | VT_BYREF) )
      {
         result = MAutomation::ToByteString(*Value->pbstrVal);
      }
      else
      {
         VARIANT v;
         MESystemError::VerifySystemError(VariantChangeType(&v, Value, 0, VT_UI1));
         result.push_back(v.bVal);
      }
   }
   return result;
}

BSTR MAutomation::ToBSTR(const MStdString& str)
{
   MWideString wideStr = MToWideString(str);
   unsigned size = M_64_CAST(unsigned, wideStr.size());
   BSTR bstr = SysAllocStringLen(wideStr.c_str(), size);
   if ( bstr == NULL )
      throw bad_alloc();
   return bstr;
}

BSTR MAutomation::ToByteStringBSTR(const MByteString& str)
{
   MWideString result;
   result.reserve(str.size());
   MByteString::const_iterator it = str.begin();
   MByteString::const_iterator itEnd = str.end();
   for ( ; it != itEnd; ++it )
      result += static_cast<wchar_t>(static_cast<Muint8>(*it));
   BSTR bstr = SysAllocStringLen(result.data(), static_cast<UINT>(result.size()));
   if ( bstr == NULL )
      throw bad_alloc();
   return bstr;
}

MStdString MAutomation::ToStdString(const BSTR vbString)
{
   MStdString result;
   if ( vbString != NULL )
   {
      unsigned len = ::SysStringLen(vbString);
      if ( len > 0 )
         result = MToStdString(vbString, len);
   }
   return result;
}

MVariant MAutomation::ToTimeOrTimeSpan(DATE date)
{
   MTime time;

   SYSTEMTIME sTime;
   if ( VariantTimeToSystemTime(date, &sTime) )
   {
      if ( sTime.wYear == 1899 && sTime.wMonth == 12 && (sTime.wDay == 31 || sTime.wDay == 30) )
      {  // in this case this is time span, not time
         MTimeSpan span(sTime.wSecond, sTime.wMinute, sTime.wHour);
         return MVariant(&span, MVariant::ACCEPT_OBJECT_EMBEDDED);
      }

      // will throw "bad time value" exception in case date is not in the MTime range
      time.Set(sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
      return MVariant(&time, MVariant::ACCEPT_OBJECT_EMBEDDED); // return valid time
   }
   else
      return MVariant(&time, MVariant::ACCEPT_OBJECT_EMBEDDED); // return null time
}

DATE MAutomation::ToOleDate(const MTime& time)
{
   double date;
   SYSTEMTIME sTime;
   struct tm tm;
   time.GetTM(&tm);
   sTime.wYear         = (WORD)tm.tm_year;
   sTime.wMonth        = (WORD)tm.tm_mon;
   sTime.wDay          = (WORD)tm.tm_mday;
   sTime.wHour         = (WORD)tm.tm_hour;
   sTime.wMinute       = (WORD)tm.tm_min;
   sTime.wSecond       = (WORD)tm.tm_sec;
   sTime.wDayOfWeek    = (WORD)tm.tm_wday;
   sTime.wMilliseconds = 0;
   if ( SystemTimeToVariantTime(&sTime, &date) )
      return date;
   else
      return 0.0; // invalid time
}

   struct SafeArrayAccessor
   {
      VARTYPE    m_vt;
      void*      m_data;
      int        m_arraySize;
      SAFEARRAY* m_safeArray;

      SafeArrayAccessor(const SAFEARRAY* safeArray, VARTYPE vt)
      :
         m_vt(VARTYPE(vt & ~VT_ARRAY)),
         m_data(NULL),
         m_arraySize(0),
         m_safeArray(const_cast<SAFEARRAY*>(safeArray))
      {
//         SafeArrayGetVartype(m_safeArray, &m_vt); not available on CE
         SafeArrayAccessData(m_safeArray, &m_data);
         if ( SafeArrayGetDim(m_safeArray) != 1 )
         {
            MException::ThrowUnsupportedType(m_vt);
            M_ENSURED_ASSERT(0);
         }

         long lBound;
         long uBound;
         MESystemError::VerifySystemError(SafeArrayGetLBound(m_safeArray, 1, &lBound));
         MESystemError::VerifySystemError(SafeArrayGetUBound(m_safeArray, 1, &uBound));
         m_arraySize = uBound - lBound + 1;
      }

      ~SafeArrayAccessor()
      {
         SafeArrayUnaccessData(m_safeArray); // no check, and we are in the destructor
      }
   };

#if !(M_OS & M_OS_WIN32_CE)
// there is no way to determine SAFEARRAY data type on CE - SafeArrayGetVartype() is not available
MVariant MAutomation::ToMVariant(const SAFEARRAY* sa)
{
   VARTYPE vt = VT_EMPTY;
   SafeArrayGetVartype(const_cast<SAFEARRAY*>(sa), &vt);
   return ToMVariant(sa, vt);
}
#endif

MVariant MAutomation::ToMVariant(const SAFEARRAY* sa, VARTYPE vt)
{
   M_COMPILED_ASSERT(sizeof(int)      == sizeof(INT)  && sizeof(INT)  == sizeof(Mint32));
   M_COMPILED_ASSERT(sizeof(unsigned) == sizeof(UINT) && sizeof(UINT) == sizeof(Muint32));

   MVariant variant(MVariant::VAR_VARIANT_COLLECTION);
   SafeArrayAccessor accessor(sa, vt);
   switch ( accessor.m_vt )
   {
   default:
      MException::ThrowUnsupportedType(accessor.m_vt);
      M_ENSURED_ASSERT(0);
   case VT_UI1:
   case VT_I1:
      variant.Assign((const char*)accessor.m_data, accessor.m_arraySize); // MByteString type
      break;
   case VT_UI2:
      {
         const Muint16* a = (const Muint16*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(unsigned(a[i]));
      }
      break;
   case VT_UINT:
   case VT_UI4:
      {
         const Muint32* a = (const Muint32*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(unsigned(a[i]));
      }
      break;
   case VT_UI8:
      {
         const Muint64* a = (const Muint64*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(double(a[i]));
      }
      break;
   case VT_I2:
      {
         const Mint16* a = (const Mint16*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(int(a[i]));
      }
      break;
   case VT_INT:
   case VT_I4:
      {
         const Mint32* a = (const Mint32*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(int(a[i]));
      }
      break;
   case VT_I8:
      {
         const Mint64* a = (const Mint64*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(double(a[i]));
      }
      break;
   case VT_R4:
      {
         const float* a = (const float*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(double(a[i]));
      }
      break;
   case VT_R8:
      {
         const double* a = (const double*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(a[i]);
      }
      break;
   case VT_BOOL:
      {
         const BOOL* a = (const BOOL*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += MVariant(a[i] != FALSE);
      }
      break;
   case VT_BSTR:
      {
         variant.SetToNull(MVariant::VAR_STRING_COLLECTION);
         const BSTR* a = (const BSTR*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += ToStdString(a[i]);
      }
      break;
   case VT_VARIANT:
      {
         const VARIANT* a = (const VARIANT*)accessor.m_data;
         for ( int i = 0; i < accessor.m_arraySize; ++i )
            variant += ToMVariant(a[i]);
      }
      break;
   }
   return variant;
}

CComVariant MAutomation::ToOleVariant(const MVariant& value, bool treatByteStringAsString)
{
   CComVariant var;
   switch ( value.GetType() )
   {
   default:
      MException::ThrowUnsupportedType(value.GetType());
      M_ENSURED_ASSERT(0);
   case MVariant::VAR_EMPTY:
      break; // return empty variant
   case MVariant::VAR_BOOL:
      var = value.DoInterpretAsBool();
      break;
   case MVariant::VAR_BYTE:
      var = (int)value.DoInterpretAsByte();
      break;
   case MVariant::VAR_INT:
      var = value.DoInterpretAsInt();
      break;
   case MVariant::VAR_UINT:
      {
         int val = (int)value.DoInterpretAsUInt();
         if ( val >= 0 ) // Automation supports only signed types
         {
            var = val;
            break;
         }
      }
      // No break here, drop into AsDouble if UInt does not fit
   case MVariant::VAR_DOUBLE:
      var = value.AsDouble();
      break;
   case MVariant::VAR_BYTE_STRING: // takes care of byte strings well...
      {
         MByteString str = value.DoInterpretAsByteString();
         if ( !treatByteStringAsString )
         {
            SAFEARRAYBOUND bounds[1];
            const unsigned strSize = M_64_CAST(unsigned, str.size());
            bounds[0].cElements = strSize;
            bounds[0].lLbound = 0;
            SAFEARRAY* sa = SafeArrayCreate(VT_VARIANT, 1, bounds);
            if ( sa == NULL )
               throw bad_alloc();
            for ( long i = 0; i < static_cast<long>(strSize); ++i )
            {
               CComVariant v((BYTE)str[i]);
               MESystemError::VerifySystemError(SafeArrayPutElement(sa, &i, reinterpret_cast<void*>(&v)));
            }
            var.vt = VT_ARRAY | VT_VARIANT;
            var.parray = sa;
         }
         else
         {
            var.vt = VT_BSTR;
            var.bstrVal = ToByteStringBSTR(str);
         }
         break;
      }
   case MVariant::VAR_CHAR:
   case MVariant::VAR_STRING:
      {
         var.vt = VT_BSTR;
         MWideString wideString = MToWideString(value.AsString());
         var.bstrVal = SysAllocStringLen(wideString.c_str(), M_64_CAST(unsigned, wideString.size()));
         if ( var.bstrVal == NULL )
            throw bad_alloc();
      }
      break;
   case MVariant::VAR_STRING_COLLECTION:
      {
         const MStdStringVector& vect = value.DoInterpretAsStringCollection();
         SAFEARRAYBOUND bounds[1];
         const unsigned vectSize = M_64_CAST(unsigned, vect.size());
         bounds[0].cElements = vectSize;
         bounds[0].lLbound = 1; // VBS array is one based
         SAFEARRAY* sa = SafeArrayCreate(VT_VARIANT, 1, bounds);
         if ( sa == NULL )
            throw bad_alloc();
         long uBound = long(vectSize + 1);
         for ( long i = 1; i < uBound; ++i )
         {
            CComVariant v = ToOleVariant(vect[i - 1]);
            MESystemError::VerifySystemError(SafeArrayPutElement(sa, &i, reinterpret_cast<void*>(&v)));
         }
         var.vt = VT_ARRAY | VT_VARIANT;
         var.parray = sa;
         break;
      }
   case MVariant::VAR_OBJECT:
      {
         const MObject* o = value.DoInterpretAsObject();
         if ( o != NULL ) // otherwise there is an empty variant
         {
            const MTime* t = M_DYNAMIC_CONST_CAST(MTime, o);
            if ( t != NULL )
            {
               var.vt = VT_DATE;
               var.date = ToOleDate(*t);
            }
            else
            {
               const MAutomation* a = M_DYNAMIC_CONST_CAST(MAutomation, o);
               if ( a != NULL )
                  var = a->GetDispatch(); // return appropriate object
               else
               {
                  MException::ThrowUnsupportedType(value.GetType());
                  M_ENSURED_ASSERT(0);
               }
            }
         }
         break;
      }
   case MVariant::VAR_VARIANT_COLLECTION: // takes care of byte strings well...
      {
         const MVariant::VariantVector& vect = value.DoInterpretAsVariantCollection();
         SAFEARRAYBOUND bounds[1];
         const unsigned vectSize = M_64_CAST(unsigned, vect.size());
         bounds[0].cElements = vectSize;
         bounds[0].lLbound = 0;
         SAFEARRAY* sa = SafeArrayCreate(VT_VARIANT, 1, bounds);
         if ( sa == NULL )
            throw bad_alloc();
         long uBound = vectSize;
         for ( long i = 0; i < uBound; ++i )
         {
            CComVariant v = ToOleVariant(vect[i]);
            MESystemError::VerifySystemError(SafeArrayPutElement(sa, &i, reinterpret_cast<void*>(&v)));
         }
         var.vt = VT_ARRAY | VT_VARIANT;
         var.parray = sa;
         break;
      }
   }
   return var;
}

MVariant MAutomation::ToMVariant(const VARIANT& value)
{
   CComVariant directValue(value);
   if ( (directValue.vt & VT_BYREF) != 0 )
      directValue.ChangeType(VARTYPE(directValue.vt & ~VT_BYREF));

   if ( (directValue.vt & VT_ARRAY) != 0 )
      return ToMVariant(directValue.parray, directValue.vt);

   switch ( directValue.vt )
   {
   default:
      MException::ThrowUnsupportedType(directValue.vt);
      M_ENSURED_ASSERT(0);
   case VT_DISPATCH:
      if ( directValue.pdispVal == NULL )
         return MVariant((MObject*)NULL);
      return MVariant(M_NEW MAutomation(directValue.pdispVal));
   case VT_EMPTY:
   case VT_NULL: // This is for NULL support in SQL
      return MVariant();
   case VT_UI1:
      return unsigned(directValue.bVal);
   case VT_UI2:
      return unsigned(directValue.uiVal);
   case VT_UI4:
      return unsigned(directValue.ulVal);
#if __REQUIRED_RPCNDR_H_VERSION__ > 440
   case VT_UI8:
      return double(directValue.ullVal);
#endif
   case VT_UINT:
      return unsigned(directValue.uintVal);
   case VT_I1:
      return int(directValue.cVal);
   case VT_I2:
      return int(directValue.iVal);
   case VT_I4:
      return int(directValue.lVal);
#if __REQUIRED_RPCNDR_H_VERSION__ > 440
   case VT_I8:
      return double(directValue.llVal);
#endif
   case VT_INT:
      return int(directValue.intVal);
   case VT_R4:
      return double(directValue.fltVal);
   case VT_R8:
      return double(directValue.dblVal);
   case VT_BOOL:
      return ToBool(directValue.boolVal); // comparison with FALSE is safer than one with TRUE
   case VT_BSTR:
      return ToStdString(directValue.bstrVal);
   case VT_DATE:
      return ToTimeOrTimeSpan(directValue.date);
   case VT_VARIANT:
      // A pointer to another VARIANTARG is passed in pvarVal.
      // This referenced VARIANTARG, pvarVal, cannot be another VT_VARIANT|VT_BYREF.
      // This value can be used to support languages that allow functions to change
      // the types of variables passed by reference.
      return ToMVariant(*directValue.pvarVal); // recurse
   }
}

void MAutomation::CheckParameterExists(void *o, MConstChars argumentName)
{
   if ( o == NULL )
   {
      MException::Throw(M_ERR_ARGUMENT_S1_IS_NOT_OPTIONAL, "Argument '%s' is not optional", argumentName);
      M_ENSURED_ASSERT(0);
   }
}

MAutomation* MAutomation::CreateObject(const MStdString& progId)
{
   s_initializer.EnsureInitialized();

   CLSID clsid;
   CComBSTR pid((int)progId.size() + 1, progId.c_str()); // size + 1 because the trailing zero needs to be included

   HRESULT hr;
   if ( pid[0] == L'{' )
      hr = CLSIDFromString(pid.m_str, &clsid);
   else
      hr = CLSIDFromProgID(pid.m_str, &clsid);

   MESystemError::VerifySystemError(hr);

   IDispatch* dispatch;
   hr = CoCreateInstance(clsid, NULL, (CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER), IID_IDispatch, (void**)&dispatch);
   if ( hr == E_NOINTERFACE ) // Search in the local server context for such case
      hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&dispatch);

   MESystemError::VerifySystemError(hr);
   return M_NEW MAutomation(dispatch);
}

MStdString MAutomation::GetProgId() const
{
   MStdString ret = "Automation";

   ITypeInfo* typeInfo;
   if ( !FAILED(m_dispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &typeInfo)) )
   {
      TYPEATTR*  typeAttr;
      if ( !FAILED(typeInfo->GetTypeAttr(&typeAttr)) )
      {
         if ( TKIND_COCLASS == typeAttr->typekind ) // Check if this is a CoClass
         {
            WCHAR* progId;
            if ( !FAILED(ProgIDFromCLSID(typeAttr->guid, &progId)) ) // typeAttr->guid contains the CLSID of the CoClass
            {
               ret = MToStdString(progId);
               CoTaskMemFree(progId);
            }
         }
         typeInfo->ReleaseTypeAttr(typeAttr);
      }
      typeInfo->Release();
   }
   return ret;
}

MVariant MAutomation::GetProperty(const MStdString& name) const
{
   #if !M_NO_REFLECTION
      if ( MObject::IsPropertyPresent(name) )
         return MObject::GetProperty(name);
   #endif // !M_NO_REFLECTION

   return GetAutomationProperty(name);
}

void MAutomation::SetProperty(const MStdString& name, const MVariant& value)
{
   #if !M_NO_REFLECTION
      if ( MObject::IsPropertyPresent(name) )
      {
         MObject::SetProperty(name, value);
         return;
      }
   #endif // !M_NO_REFLECTION

   SetAutomationProperty(name, value);
}

MVariant MAutomation::GetAutomationProperty(const MStdString& name) const
{
   CComVariant args;
   DISPPARAMS params;
   params.cNamedArgs = 0;
   params.rgdispidNamedArgs = 0;
   params.cArgs = 0;
   params.rgvarg = NULL;
   return DoInvoke(name, params, DISPATCH_PROPERTYGET);
}

void MAutomation::SetAutomationProperty(const MStdString& name, const MVariant& value)
{
   CComVariant args = ToOleVariant(value);
   DISPPARAMS params; // PROPERTYPUT requires one named parameter
   params.cNamedArgs = 1;
   DISPID dispidNamed = DISPID_PROPERTYPUT;
   params.rgdispidNamedArgs = &dispidNamed;
   params.cArgs = 1;
   params.rgvarg = &args;
   DoInvoke(name, params, DISPATCH_PROPERTYPUT);
}

MVariant MAutomation::CallV(const MStdString& name, const MVariant::VariantVector& p)
{
   #if !M_NO_REFLECTION
      if ( MObject::IsServicePresent(name) )
         return MObject::CallV(name, p);
   #endif

   vector<CComVariant> args;
   for ( int i = M_64_CAST(int, p.size())-1 ; i >= 0 ; --i )
      args.push_back(ToOleVariant(p[i]));

   DISPPARAMS params;
   params.cNamedArgs = 0;
   params.rgdispidNamedArgs = 0;
   params.cArgs = UINT(p.size());
   params.rgvarg = params.cArgs > 0 ? &(args[0]) : NULL;
   return DoInvoke(name, params, DISPATCH_METHOD|DISPATCH_PROPERTYGET);
}

bool MAutomation::IsServicePresent(const MStdString& name) const
{
   DISPID dispId;
   #if !M_NO_REFLECTION
      return MObject::IsServicePresent(name) || SUCCEEDED(DoGetDispId(name, &dispId));
   #else
      return SUCCEEDED(DoGetDispId(name, &dispId));
   #endif // !M_NO_REFLECTION
}

bool MAutomation::IsPropertyPresent(const MStdString& name) const
{
   DISPID dispId;
   #if !M_NO_REFLECTION
      return MObject::IsPropertyPresent(name) || SUCCEEDED(DoGetDispId(name, &dispId));
   #else
      return SUCCEEDED(DoGetDispId(name, &dispId));
   #endif // !M_NO_REFLECTION
}

HRESULT MAutomation::DoGetDispId(const MStdString& name, DISPID* dispId) const
{
   CComBSTR nameBStr((int)name.size() + 1, name.c_str()); // +1 because trailing zero needs to be included

   // This call can return DISPID_UNKNOWN
   //
   return m_dispatch->GetIDsOfNames(IID_NULL, (LPOLESTR*)&nameBStr.m_str, 1, LOCALE_USER_DEFAULT, dispId);
}

MVariant MAutomation::DoInvoke(const MStdString& name, DISPPARAMS& params, Muint16 invokeType) const
{
   EXCEPINFO ex;
   UINT      parError;
   VARIANT   result;
   DISPID    dispId = DISPID_UNKNOWN;
   HRESULT   hr = DoGetDispId(name, &dispId);
   if ( !SUCCEEDED(hr) )
   {
      M_ASSERT(dispId == DISPID_UNKNOWN);
      if ( invokeType == DISPATCH_METHOD )
         MClass::DoThrowUnknownServiceException(GetProgId().c_str(), name);
      else
         MClass::DoThrowUnknownPropertyException(GetProgId().c_str(), name);
      M_ENSURED_ASSERT(0);
   }

   result.vt = VT_EMPTY;
   hr = m_dispatch->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT, invokeType, &params, &result, &ex, &parError);
   switch ( hr )
   {
   case DISP_E_TYPEMISMATCH:
      MException::Throw(M_ERR_PARAMETER_D1_TYPE_MISMATCH, "Parameter %d type mismatch", params.cArgs - parError);
      M_ENSURED_ASSERT(0);
   case DISP_E_PARAMNOTFOUND:
      MException::Throw(M_ERR_PARAMETER_D1_NOT_FOUND, "Parameter %d not found", params.cArgs - parError);
      M_ENSURED_ASSERT(0);
   case DISP_E_EXCEPTION:
      if ( ex.bstrDescription == NULL )
      {
         MException::Throw(M_ERR_EXTERNAL_APPLICATION_EXCEPTION_CODE_D1, "External application exception, code %d", ex.wCode);
         M_ENSURED_ASSERT(0);
      }
      else
      {
         MException::Throw(M_ERR_EXTERNAL_APPLICATION_EXCEPTION_CODE_D1_MESSAGE_S2, "External application exception, code %d, message: %s", ex.wCode, MToStdString(ex.bstrDescription).c_str());
         M_ENSURED_ASSERT(0);
      }
   case S_OK:
      // success. nothing to do
      break;
   default:
      MESystemError::VerifySystemError(hr); // this will throw an exception, since hr is not ok.
      M_ENSURED_ASSERT(0);
   }
   return ToMVariant(result);
}

#endif
